/* ====================================================================
 * cycommon.h — Shared Device Extension Definitions
 * ====================================================================
 * Both the bus enumerator (FDO) and port driver (child PDO) live in
 * one .sys file. The PnP dispatch examines the IsFDO flag in the
 * common header to route IRPs to the correct handler.
 *
 * Memory layout:
 *   FDO DeviceExtension → CY_FDO_EXT (starts with CY_COMMON_EXT)
 *   PDO DeviceExtension → CY_PDO_EXT (starts with CY_COMMON_EXT)
 *
 * The IsFDO flag MUST be the first meaningful field in both
 * extensions so that the dispatcher can read it from either type.
 *
 * Architecture derived from:
 *   - Microsoft serenum sample (WDK)
 *   - Original Cyclades cyclom-y.sys import analysis
 *   - A2_A3_ARCHITECTURE.md research document
 *
 * License: GPLv3
 * ====================================================================
 */

#ifndef CYCOMMON_H
#define CYCOMMON_H

#include <ntddk.h>
#include <ntddser.h>        /* Serial port IOCTL definitions          */
#include <wdm.h>            /* IO_CSQ types (OW puts them here)       */

/* OpenWatcom DDK compatibility — fills gaps in OW headers */
#ifdef __WATCOMC__
#include "owcompat.h"
#endif

/* Pull in our CD1400 register definitions */
#include "cd1400.h"

/* Pull in debug infrastructure — CyDbgPrint, CyError, CyInfo, CyTrace,
 * CYPORT_ASSERT, IRQL validation macros. All source files get these
 * automatically through cycommon.h. 
 *
 * Debug levels: 0=silent, 1=error, 2=warn, 3=info, 4=trace, 5=verbose.
 * Checked (debug) build: default level 3, all asserts active.
 * Free (release) build: default level 1, asserts compiled out. */
#include "cydebug.h"
#include "cytrace.h"        /* WPP tracing (or DbgPrint fallback)     */

/* ====================================================================
 * Pool Tags
 * ====================================================================
 * All pool allocations use a 4-byte tag for tracking in WinDbg.
 * Tag is stored little-endian, so 'CyPt' appears as 'tPyC' in raw.
 * ==================================================================== */

#define CY_POOL_TAG         'tPyC'      /* General allocations          */
#define CY_PDO_TAG          'dPyC'      /* PDO-related allocations      */


/* ====================================================================
 * PnP State Machine
 * ====================================================================
 * Tracks the current PnP state of each device object. Used to
 * determine which operations are legal at any given time.
 * ==================================================================== */

typedef enum _CY_PNP_STATE {
    CyNotStarted = 0,       /* Initial state after AddDevice            */
    CyStarted,              /* After successful IRP_MN_START_DEVICE     */
    CyStopPending,          /* IRP_MN_QUERY_STOP received              */
    CyStopped,              /* IRP_MN_STOP_DEVICE received             */
    CyRemovePending,        /* IRP_MN_QUERY_REMOVE received            */
    CySurpriseRemoved,      /* IRP_MN_SURPRISE_REMOVAL received        */
    CyRemoved               /* IRP_MN_REMOVE_DEVICE received           */
} CY_PNP_STATE;


/* ====================================================================
 * Common Device Extension Header
 * ====================================================================
 * MUST be the first member of both CY_FDO_EXT and CY_PDO_EXT.
 * The PnP dispatch routine casts DeviceExtension to this type
 * and reads IsFDO to route the IRP.
 *
 * Pattern from serenum sample and MSDN WDM bus driver documentation.
 * ==================================================================== */

typedef struct _CY_COMMON_EXT {

    /* ---- Routing flag (MUST BE FIRST) ---- */
    BOOLEAN         IsFDO;          /* TRUE = bus FDO, FALSE = child PDO*/

    /* ---- Device identity ---- */
    PDEVICE_OBJECT  Self;           /* Points to our own device object  */

    /* ---- PnP state ---- */
    CY_PNP_STATE    PnPState;       /* Current PnP state               */
    CY_PNP_STATE    PreviousPnPState; /* State before last transition   */

    /* ---- Remove lock ----
     * Acquired at the top of every IRP dispatch routine.
     * Released when done processing.
     * IoReleaseRemoveLockAndWait in REMOVE handler drains all
     * outstanding IRPs before we delete the device object.
     *
     * This fixes audit bug #8 (no remove lock on any IRP). */
    IO_REMOVE_LOCK  RemoveLock;

    /* ---- Power state ---- */
    SYSTEM_POWER_STATE  SystemPowerState;
    DEVICE_POWER_STATE  DevicePowerState;

} CY_COMMON_EXT, *PCY_COMMON_EXT;


/* ====================================================================
 * FDO Device Extension — Bus Enumerator
 * ====================================================================
 * One FDO per Cyclom-Y card. Created in AddDevice when the PCI bus
 * enumerates our PCI device (VEN_120E).
 *
 * The FDO owns:
 *   - The mapped memory window (CardBase)
 *   - The chip array (how many CD1400s, their revisions)
 *   - The child PDO array (one PDO per serial port)
 *
 * This fixes audit bug #1 (CYDRIVER_EXT was stack local).
 * ==================================================================== */

typedef struct _CY_FDO_EXT {
    /* ---- Common header (MUST BE FIRST) ---- */
    CY_COMMON_EXT   Common;

    /* ---- Bus driver topology ---- */
    PDEVICE_OBJECT  UnderlyingPDO;  /* PCI's PDO for our card          */
    PDEVICE_OBJECT  LowerDevice;    /* IoAttachDeviceToDeviceStack result*/

    /* ---- Hardware resources ----
     * CardBase is the MmMapIoSpace result for the entire memory window.
     * It persists for the lifetime of the FDO and is unmapped in
     * IRP_MN_REMOVE_DEVICE. This fixes audit bug #1 (CardBase was lost
     * because CYDRIVER_EXT was on the stack). */
    PHYSICAL_ADDRESS PhysicalBase;  /* Physical address from PCI BAR   */
    PUCHAR          CardBase;       /* MmMapIoSpace result             */
    ULONG           MappedLength;   /* Length of mapped region         */

    /* ---- Chip inventory ---- */
    ULONG           NumChips;       /* How many CD1400s detected       */

    /* ---- PCI bus index (register spacing) ----
     * Linux uses a bus_index multiplier for register addressing:
     *   ISA:  readb(base + (reg << 0))  = reg at offset*1
     *   PCI:  readb(base + (reg << 1))  = reg at offset*2
     *
     * Our cd1400.h defines registers as (raw_offset * 2), which is
     * the ISA convention. For PCI cards, the Linux driver shifts
     * an additional time, making PCI registers at raw_offset * 4.
     *
     * The newer Linux driver (cyy_readb) abstracts this into a
     * precomputed offset stored per-card. We store the shift value
     * here and apply it in CyReadReg/CyWriteReg.
     *
     * Default: 0 (ISA, registers at raw*2 as defined in cd1400.h).
     * Set to 1 for PCI (registers at raw*4).
     * Auto-detected from PCI BAR address in START_DEVICE.
     * (Audit B3 platform-difference note) */
    ULONG           BusIndex;       /* 0=ISA, 1=PCI register shift    */

    /* ---- Interrupt resources ----
     * Populated from CmResourceTypeInterrupt in START_DEVICE.
     * Used by IoConnectInterrupt to connect the ISR.
     * (Fix for known issue 5.5 — PCI resource parsing) */
    KIRQL           InterruptLevel;     /* DIRQL for this interrupt    */
    ULONG           InterruptVector;    /* Interrupt vector            */
    KAFFINITY       InterruptAffinity;  /* Processor affinity mask     */
    KINTERRUPT_MODE InterruptMode;      /* Latched(ISA) or Level(PCI)  */
    PKINTERRUPT     InterruptObject;    /* IoConnectInterrupt result   */
    BOOLEAN         InterruptConnected; /* TRUE if ISR is connected    */
    struct {
        PUCHAR      Base;           /* Chip base = CardBase + n*0x400  */
        UCHAR       Revision;       /* GFRCR value                    */
        BOOLEAN     Is60MHz;        /* TRUE if Rev J (60 MHz clock)   */
    } Chips[CY_MAX_CHIPS];

    /* ---- Child PDO management ----
     * One PDO per port. Created during START_DEVICE when we scan
     * for chips. Reported to PnP via IRP_MN_QUERY_DEVICE_RELATIONS.
     * Each PDO is ObReferenceObject'd when returned in BusRelations
     * and ObDereferenceObject'd by the PnP manager when done.
     *
     * NumPDOs tracks how many children exist. */
    ULONG           NumPDOs;
    PDEVICE_OBJECT  ChildPDOs[CY_MAX_CHIPS * CY_PORTS_PER_CHIP];

    /* ---- Enumeration sync ---- */
    KSPIN_LOCK      EnumLock;       /* Protects child PDO list         */
    BOOLEAN         Started;        /* TRUE after START_DEVICE succeeds*/

} CY_FDO_EXT, *PCY_FDO_EXT;


/* ====================================================================
 * PDO Device Extension — Per-Port Serial State
 * ====================================================================
 * One PDO per CD1400 channel (serial port). Created by the bus
 * enumerator when it scans for chips.
 *
 * The PDO owns:
 *   - The serial port state (baud, line control, modem signals)
 *   - The ring buffers (RX and TX)
 *   - The interrupt object (shared with other ports on same chip)
 *   - The symbolic link (\DosDevices\COMn)
 *
 * The PDO does NOT forward IRPs — it's the bottom of the device
 * stack. It completes all IRPs directly.
 *
 * Ring buffer access:
 *   ISR writes to RxBuf and reads from TxBuf at DIRQL.
 *   Dispatch routines read from RxBuf and write to TxBuf at DISPATCH.
 *   All ring buffer access is protected by KeSynchronizeExecution
 *   (which runs a callback at DIRQL, synchronized with the ISR).
 *   This fixes audit bugs #3, #4, #5, #7, #9.
 * ==================================================================== */

#define CY_RING_BUF_SIZE    4096    /* Ring buffer size in bytes             */

typedef struct _CY_PDO_EXT {
    /* ---- Common header (MUST BE FIRST) ---- */
    CY_COMMON_EXT   Common;

    /* ---- Parent reference ---- */
    PDEVICE_OBJECT  ParentFdo;      /* Back-pointer to bus FDO         */

    /* ---- Port identity ---- */
    ULONG           PortIndex;      /* Global port number (0-31)       */
    UCHAR           ChipIndex;      /* Which chip on the card (0-7)    */
    UCHAR           Channel;        /* Channel within chip (0-3)       */

    /* ---- Hardware access ----
     * ChipBase points into the FDO's mapped memory window.
     * It is valid as long as the FDO's CardBase is mapped.
     * The PDO does NOT own the mapping — the FDO does. */
    PUCHAR          ChipBase;       /* Memory-mapped chip base         */
    UCHAR           ChipRev;        /* CD1400 revision code            */
    BOOLEAN         Is60MHz;        /* TRUE if Rev J (60 MHz clock)    */

    /* ---- Port state ---- */
    BOOLEAN         IsOpen;         /* TRUE between CREATE and CLOSE   */
    ULONG           BaudRate;       /* Current baud rate               */
    SERIAL_LINE_CONTROL LineControl; /* Data/stop/parity settings      */
    SERIAL_HANDFLOW HandFlow;       /* Flow control settings           */
    SERIAL_CHARS    SpecialChars;   /* XON/XOFF/error/break chars      */
    SERIAL_TIMEOUTS Timeouts;       /* Read/write timeout values       */
    ULONG           ModemControl;   /* DTR/RTS state flags             */
    ULONG           WaitMask;       /* EV_* mask for WaitCommEvent     */

    /* ---- DTR/RTS pin inversion flag ----
     * Some Cyclom-Y boards (rev 6.00+) have DTR and RTS pins
     * physically swapped. When rtsdtr_inv is TRUE:
     *   CyDTR bit → RTS pin on the wire
     *   CyRTS bit → DTR pin on the wire
     * The Linux driver detects this from the board revision and
     * cable type. We default to FALSE (normal mapping).
     * (Fix for known issue 5.2) */
    BOOLEAN         RtsDtrInv;

    /* ---- Modem signal shadow ----
     * Updated by ISR on modem status change. Read by IOCTLs.
     * Protected by KeSynchronizeExecution. */
    UCHAR           ShadowMSVR;     /* Last MSVR1 reading from ISR     */

    /* ---- Statistics ---- */
    SERIALPERF_STATS PerfStats;     /* Performance counters            */

    /* ---- Interrupt ---- */
    PKINTERRUPT     Interrupt;      /* IoConnectInterrupt result       */
    BOOLEAN         InterruptConnected; /* TRUE if ISR is connected    */

    /* ---- DPC for ISR bottom half ---- */
    KDPC            ReadDpc;        /* Fires when RX data available    */
    KDPC            WriteDpc;       /* Fires when TX space available   */
    KDPC            ModemDpc;       /* Fires on modem signal change    */

    /* ---- Timer for serial timeouts ---- */
    KTIMER          ReadTimer;      /* Read timeout timer              */
    KDPC            ReadTimerDpc;   /* DPC for read timeout            */

    /* ---- Ring buffers ----
     * RxBuf: ISR writes (from RDSR), read dispatch reads.
     * TxBuf: write dispatch writes, ISR reads (to TDR).
     * Head = next position to write.
     * Tail = next position to read.
     * Count = number of bytes in buffer.
     *
     * Access from dispatch level: use KeSynchronizeExecution.
     * Access from ISR: direct (ISR runs at DIRQL).
     *
     * This fixes audit bugs #3 (CyRead from FIFO) and #4 (CyWrite
     * to TDR). Now dispatch reads from RxBuf and writes to TxBuf.
     * Only the ISR touches the hardware FIFO registers. */
    UCHAR           RxBuf[CY_RING_BUF_SIZE];
    ULONG           RxHead;         /* ISR writes here                 */
    ULONG           RxTail;         /* Dispatch reads from here        */
    ULONG           RxCount;        /* Bytes in RX buffer              */

    UCHAR           TxBuf[CY_RING_BUF_SIZE];
    ULONG           TxHead;         /* Dispatch writes here            */
    ULONG           TxTail;         /* ISR reads from here             */
    ULONG           TxCount;        /* Bytes in TX buffer              */

    /* ---- Cancel-safe IRP queues ----
     * Pending read and write IRPs are queued here.
     * Uses IoCsq via Csq.lib for Win2K backward compatibility.
     * DPC dequeues and completes IRPs when data arrives (RX)
     * or space opens (TX). */
    IO_CSQ          ReadQueue;      /* Cancel-safe read IRP queue      */
    LIST_ENTRY      ReadQueueHead;  /* Linked list of pending reads    */
    KSPIN_LOCK      ReadQueueLock;  /* Protects ReadQueueHead          */

    IO_CSQ          WriteQueue;     /* Cancel-safe write IRP queue     */
    LIST_ENTRY      WriteQueueHead; /* Linked list of pending writes   */
    KSPIN_LOCK      WriteQueueLock; /* Protects WriteQueueHead         */

    /* ---- WaitCommEvent support ---- */
    PIRP            WaitIrp;        /* Pending WAIT_ON_MASK IRP        */
    ULONG           EventHistory;   /* Accumulated events since last wait */
    KSPIN_LOCK      EventLock;      /* Protects WaitIrp + EventHistory */

    /* ---- Symbolic link ----
     * Allocated from pool, NOT a stack buffer.
     * This fixes audit bug #2 (SymLinkName pointed to stack). */
    UNICODE_STRING  SymLinkName;    /* \DosDevices\COMn                */
    UNICODE_STRING  DeviceName;     /* \Device\CycladesCOMn            */
    ULONG           ComPortNumber;  /* COM port number assigned        */

    /* ---- Port lock ----
     * General-purpose spinlock for port state that isn't covered
     * by the ISR sync (KeSynchronizeExecution) or queue locks.
     * Protects: LineControl, HandFlow, BaudRate, ModemControl,
     *           Timeouts, SpecialChars, WaitMask */
    KSPIN_LOCK      PortLock;

    /* ---- Saved power state ----
     * Persists between D0→D3 (save) and D3→D0 (restore) power
     * transitions. Must be in the PDO extension because power
     * IRPs are separate dispatches — no stack frame survives
     * between them. (Audit B1 fix — was local to cypower.c) */
    struct {
        BOOLEAN     Valid;          /* TRUE if state has been saved   */
        UCHAR       COR1;          /* Channel Option Register 1      */
        UCHAR       COR2;          /* Channel Option Register 2      */
        UCHAR       COR3;          /* Channel Option Register 3      */
        UCHAR       TCOR;          /* Transmit Clock Option           */
        UCHAR       TBPR;          /* Transmit Baud Period            */
        UCHAR       RCOR;          /* Receive Clock Option            */
        UCHAR       RBPR;          /* Receive Baud Period             */
        UCHAR       SRER;          /* Service Request Enable Register */
        UCHAR       MSVR;          /* Modem Signal Value (DTR/RTS)    */
        UCHAR       SCHR1;         /* Special Character 1 (XON)      */
        UCHAR       SCHR2;         /* Special Character 2 (XOFF)     */
    } SavedPowerState;

} CY_PDO_EXT, *PCY_PDO_EXT;


/* ====================================================================
 * Inline Helpers
 * ==================================================================== */

/* Get the common extension from any device object */
static __inline PCY_COMMON_EXT CyGetCommon(_In_ PDEVICE_OBJECT DevObj)
{
    return (PCY_COMMON_EXT)DevObj->DeviceExtension;
}

/* Get the FDO extension (caller must verify IsFDO first) */
static __inline PCY_FDO_EXT CyGetFdo(_In_ PDEVICE_OBJECT DevObj)
{
    return (PCY_FDO_EXT)DevObj->DeviceExtension;
}

/* Get the PDO extension (caller must verify !IsFDO first) */
static __inline PCY_PDO_EXT CyGetPdo(_In_ PDEVICE_OBJECT DevObj)
{
    return (PCY_PDO_EXT)DevObj->DeviceExtension;
}

/* Update PnP state with history tracking */
static __inline VOID CySetPnPState(
    PCY_COMMON_EXT Common, CY_PNP_STATE NewState)
{
    Common->PreviousPnPState = Common->PnPState;
    Common->PnPState = NewState;
}

/* Restore PnP state to previous (for cancel-stop, cancel-remove) */
static __inline VOID CyRestorePnPState(PCY_COMMON_EXT Common)
{
    Common->PnPState = Common->PreviousPnPState;
}


/* ====================================================================
 * Register Access
 * ====================================================================
 * Memory-mapped I/O through the card's shared memory window.
 * Uses READ/WRITE_REGISTER_UCHAR which provides the correct
 * memory barrier semantics on all architectures.
 *
 * The reg parameter uses cd1400.h offsets (raw_offset × 2, ISA
 * convention). For PCI cards, registers are at raw_offset × 4.
 * The BusIndex shift handles this:
 *   ISA (BusIndex=0): chipBase + (reg << 0) = reg as-is
 *   PCI (BusIndex=1): chipBase + (reg << 1) = reg × 2
 *
 * Since cd1400.h defines reg as (raw × 2), the final address is:
 *   ISA: chipBase + raw×2       (correct)
 *   PCI: chipBase + raw×2×2 = raw×4  (correct — matches Linux)
 *
 * The BusIndex is set during START_DEVICE resource parsing based
 * on whether the physical address is above 1MB (PCI) or below (ISA).
 * (Fix for known issue 5.1 — PCI register spacing)
 *
 * NOTE: The shift is applied per-access. For ISR performance,
 * chipBase could be pre-adjusted, but the shift is a single
 * instruction and correctness is more important than saving
 * one cycle per register access in the ISR.
 * ==================================================================== */

/* BusIndex is stored in the FDO extension but register access
 * functions receive chipBase (a pointer into the FDO's mapped window).
 * We store a file-scope variable set during START_DEVICE so the
 * inline functions don't need the FDO pointer on every call.
 * This is safe because all cards in one driver instance share
 * the same BusIndex (all PCI or all ISA — you can't mix). */
extern ULONG g_CyBusIndex;

static __inline UCHAR CyReadReg(PUCHAR chipBase, ULONG reg)
{
    return READ_REGISTER_UCHAR(chipBase + (reg << g_CyBusIndex));
}

static __inline VOID CyWriteReg(PUCHAR chipBase, ULONG reg, UCHAR val)
{
    WRITE_REGISTER_UCHAR(chipBase + (reg << g_CyBusIndex), val);
}

/* ====================================================================
 * Channel Selection
 * ====================================================================
 * The CD1400 multiplexes 4 channels through one register set.
 * Writing the channel number (0-3) to the CAR register selects
 * which channel subsequent per-channel register accesses apply to.
 *
 * CRITICAL: This write + subsequent register access MUST be atomic.
 * In the ISR, the hardware auto-selects CAR. Outside the ISR, ALL
 * code that touches per-channel registers must use
 * KeSynchronizeExecution to prevent the ISR from changing CAR
 * between our CAR write and our register access.
 *
 * This fixes audit bugs #4, #5, #7 (CySelectChannel without sync).
 *
 * The original cyyport.sys imports KeSynchronizeExecution for
 * exactly this purpose (confirmed by import analysis).
 * ==================================================================== */

static __inline VOID CySelectChannel(PUCHAR chipBase, UCHAR chan)
{
    CyWriteReg(chipBase, CyCAR, chan & 0x03);
}


/* ====================================================================
 * KeSynchronizeExecution Context
 * ====================================================================
 * Passed to KeSynchronizeExecution callbacks as a PVOID.
 * Contains everything the callback needs to access the port's
 * CD1400 registers and return results.
 *
 * Usage pattern:
 *   CY_SYNC_CONTEXT ctx;
 *   ctx.Extension = pdoExt;
 *   ctx.Data = &myData;
 *   KeSynchronizeExecution(pdoExt->Interrupt, CySyncMyFunc, &ctx);
 *   status = ctx.Status;
 *
 * The callback runs at DIRQL, synchronized with the ISR. No other
 * ISR or synchronized callback can run while it executes. This
 * makes CAR writes + register accesses atomic.
 * ==================================================================== */

typedef struct _CY_SYNC_CONTEXT {
    PCY_PDO_EXT Extension;      /* Port extension                   */
    PVOID       Data;           /* Operation-specific data pointer  */
    NTSTATUS    Status;         /* Result status from callback      */
} CY_SYNC_CONTEXT, *PCY_SYNC_CONTEXT;


/* ====================================================================
 * Forward Declarations — Dispatch Routines
 * ==================================================================== */

/* DriverEntry and unload */
DRIVER_INITIALIZE   DriverEntry;

/* AddDevice — called by PnP manager when our PCI device is found */
DRIVER_ADD_DEVICE   CyAddDevice;

/* IRP dispatch routines */
DRIVER_DISPATCH     CyDispatchPnP;      /* IRP_MJ_PNP routing         */
DRIVER_DISPATCH     CyDispatchPower;    /* IRP_MJ_POWER               */
DRIVER_DISPATCH     CyDispatchCreate;   /* IRP_MJ_CREATE              */
DRIVER_DISPATCH     CyDispatchClose;    /* IRP_MJ_CLOSE               */
DRIVER_DISPATCH     CyDispatchRead;     /* IRP_MJ_READ                */
DRIVER_DISPATCH     CyDispatchWrite;    /* IRP_MJ_WRITE               */
DRIVER_DISPATCH     CyDispatchIoCtl;    /* IRP_MJ_DEVICE_CONTROL      */
DRIVER_DISPATCH     CyDispatchCleanup;  /* IRP_MJ_CLEANUP             */

/* ISR */
KSERVICE_ROUTINE    CyInterruptService;

/* FDO PnP handlers (cyenum.c) */
NTSTATUS CyFdoPnP(_In_ PDEVICE_OBJECT DevObj, _Inout_ PIRP Irp);

/* PDO PnP handlers (cypdo.c) */
NTSTATUS CyPdoPnP(_In_ PDEVICE_OBJECT DevObj, _Inout_ PIRP Irp);

/* Serial port core (cyserial.c) */
NTSTATUS CyInitCancelSafeQueues(_In_ PCY_PDO_EXT pdoExt);
BOOLEAN NTAPI CyInitChannelSync(_In_ PVOID Context);
BOOLEAN NTAPI CyShutdownChannelSync(_In_ PVOID Context);

/* Read/Write DPC completions (cyread.c, cywrite.c) */
VOID     CyReadDpcComplete(_In_ PCY_PDO_EXT pdoExt);
VOID     CyWriteDpcComplete(_In_ PCY_PDO_EXT pdoExt);

/* ISR DPC routines (cyisr.c) — used by KeInitializeDpc */
VOID NTAPI CyReadDpcRoutine(PKDPC Dpc, PVOID Ctx, PVOID A1, PVOID A2);
VOID NTAPI CyWriteDpcRoutine(PKDPC Dpc, PVOID Ctx, PVOID A1, PVOID A2);
VOID NTAPI CyModemDpcRoutine(PKDPC Dpc, PVOID Ctx, PVOID A1, PVOID A2);

/* KeSynchronizeExecution wrappers (cyisr.c) */
BOOLEAN NTAPI CySyncReadMSVR(_In_ PVOID Context);
BOOLEAN NTAPI CySyncEnableTx(_In_ PVOID Context);
BOOLEAN NTAPI CySyncPurgeBuffers(_In_ PVOID Context);

/* Interrupt lifecycle (cyisr.c) */
NTSTATUS CyConnectInterrupt(_In_ PCY_FDO_EXT fdoExt, _In_ PCY_PDO_EXT pdoExt);
VOID     CyDisconnectInterrupt(_In_ PCY_PDO_EXT pdoExt);

/* Event logging (cylog.c) — writes to Windows Event Log.
 * These also emit CyDbgPrint output in checked builds so
 * events are visible with a debugger attached. */
VOID CyLogEvent(_In_ PDEVICE_OBJECT DevObj, NTSTATUS ErrorCode,
                ULONG UniqueId, PULONG DumpData,
                ULONG DumpCount, NTSTATUS NtStatus);
VOID CyLogDriverLoaded(_In_ PDEVICE_OBJECT DevObj,
                       ULONG NumChips, ULONG NumPorts);
VOID CyLogDriverUnloaded(_In_ PDEVICE_OBJECT DevObj);
VOID CyLogPortOpened(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex,
                     ULONG ComNumber, ULONG BaudRate);
VOID CyLogPortClosed(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex,
                     ULONG ComNumber, ULONG TxTotal, ULONG RxTotal);
VOID CyLogBufferOverrun(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex,
                        ULONG BytesLost);
VOID CyLogFifoOverrun(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex);
VOID CyLogParityErrors(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex,
                       ULONG ErrorCount);
VOID CyLogFramingErrors(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex,
                        ULONG ErrorCount);
VOID CyLogNoChipsFound(_In_ PDEVICE_OBJECT DevObj,
                       PHYSICAL_ADDRESS MemAddress);
VOID CyLogMemoryMapFailed(_In_ PDEVICE_OBJECT DevObj,
                          PHYSICAL_ADDRESS MemAddress, ULONG Length);
VOID CyLogInterruptConnectFailed(_In_ PDEVICE_OBJECT DevObj,
                                 NTSTATUS NtStatus);
VOID CyLogPowerSuspend(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex);
VOID CyLogPowerResume(_In_ PDEVICE_OBJECT DevObj, ULONG PortIndex);


/* ====================================================================
 * Hardware IDs for Child PDOs
 * ====================================================================
 * When the PnP manager sends IRP_MN_QUERY_ID to our child PDOs,
 * we return these strings. They must match cyyport.inf so the PnP
 * manager loads the correct port driver (which is us, since we're
 * a single .sys with dual role).
 *
 * Confirmed from strings in original cyclom-y.sys:
 *   "Cyclom-Y\Port"     — hardware ID
 *   "Cyclom-Y Port %2u" — device description format
 * ==================================================================== */

#define CY_CHILD_HARDWARE_ID    L"Cyclom-Y\\Port"
#define CY_CHILD_DEVICE_DESC    L"Cyclades Cyclom-Y Serial Port"


#endif /* CYCOMMON_H */
