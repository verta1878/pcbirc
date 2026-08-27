/* ====================================================================
 * cyport.h — CYPORT Driver Internal Definitions
 * ====================================================================
 * Master header for the Cyclades CD1400 serial port driver.
 * Includes data structures, function prototypes with SAL annotations,
 * and configuration constants.
 *
 * SAL (Standard Annotation Language) annotations on every pointer
 * parameter and return value. These enable:
 *   - PREfast static analysis (catches null deref, buffer overrun)
 *   - Code Analysis in Visual Studio
 *   - Self-documenting parameter contracts
 * ====================================================================
 */

#ifndef CYPORT_H
#define CYPORT_H

#include <ntddk.h>
#include <ntddser.h>
#include <wdmsec.h>

/* Our headers */
#include "cd1400.h"
#include "cydebug.h"
#include "cytrace.h"

/* ====================================================================
 * CONFIGURATION CONSTANTS
 * ==================================================================== */

#define CYPORT_MAX_PORTS        32      /* Max ports per card           */
#define CYPORT_RX_BUFFER_SIZE   4096    /* Per-port RX buffer           */
#define CYPORT_TX_BUFFER_SIZE   4096    /* Per-port TX buffer           */
#define CYPORT_DEFAULT_BAUD     9600
#define CYPORT_DEFAULT_MEMBASE  0xD4000UL
#define CYPORT_ISA_WINDOW_SIZE  0x2000
#define CYPORT_DRIVER_VERSION   0x00010000  /* 1.0.0.0                 */

/* How long to wait for CCR to clear after a command (microseconds).
 * The CD1400 datasheet says CCR executes in a few clock cycles,
 * but we add margin for slow ISA bus and clock variants. */
#define CYPORT_CCR_TIMEOUT_US   200

/* Maximum iterations for any drain/poll loop.
 * Prevents infinite loops on hardware faults (hexadecimal W-02). */
#define CYPORT_MAX_DRAIN_ITER   256


/* ====================================================================
 * DEVICE EXTENSION — per-port state
 * ====================================================================
 * One of these exists for each COM port device object. Contains
 * all port-specific state, hardware pointers, buffers, and locks.
 *
 * Layout is ordered for cache efficiency:
 *   1. Frequently accessed fields (ISR path) first
 *   2. Configuration fields (set once, read often) second
 *   3. Infrequently accessed fields last
 * ==================================================================== */

typedef struct _CYPORT_EXTENSION {

    /* ---- ISR-hot path (must be in same cache line) ---- */

    _Field_range_(0, CY_MAX_CHIPS-1)
    UCHAR           ChipIndex;      /* Which CD1400 chip (0-7)         */

    _Field_range_(0, CY_PORTS_PER_CHIP-1)
    UCHAR           Channel;        /* Which channel within chip (0-3) */

    UCHAR           ChipRev;        /* CD1400 revision code            */
    BOOLEAN         Is60MHz;        /* TRUE = Rev J (60 MHz clock)     */

    ULONG           DeviceState;    /* CYPORT_STATE_xxx                */

    _Field_size_(CYPORT_RX_BUFFER_SIZE)
    PUCHAR          RxBuffer;       /* RX ring buffer (pool alloc)     */
    ULONG           RxHead;         /* Consumer index (read path)      */
    ULONG           RxTail;         /* Producer index (ISR/DPC)        */
    ULONG           RxCount;        /* Bytes in buffer                 */

    _Field_size_(CYPORT_TX_BUFFER_SIZE)
    PUCHAR          TxBuffer;       /* TX ring buffer (pool alloc)     */
    ULONG           TxHead;         /* Consumer index (ISR/DPC)        */
    ULONG           TxTail;         /* Producer index (write path)     */
    ULONG           TxCount;        /* Bytes in buffer                 */

    /* ---- Hardware pointers ---- */

    PUCHAR          ChipBase;       /* Memory-mapped CD1400 chip base  */
    PUCHAR          CardBase;       /* Memory-mapped card base         */

    /* ---- Synchronization ---- */

    KSPIN_LOCK      PortLock;       /* Protects ring buffers + state   */
    PKINTERRUPT     InterruptObject;/* From IoConnectInterrupt         */

    /* ---- Current settings ---- */

    ULONG           BaudRate;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_HANDFLOW HandFlow;
    SERIAL_CHARS    SpecialChars;
    SERIAL_TIMEOUTS Timeouts;
    ULONG           ModemControl;   /* SERIAL_DTR_STATE | RTS_STATE    */
    ULONG           WaitMask;       /* IOCTL_SERIAL_SET_WAIT_MASK      */

    /* ---- Modem shadow ---- */

    UCHAR           MsvrShadow;     /* Last MSVR1 reading              */
    UCHAR           LsrShadow;      /* Accumulated LSR-like errors     */

    /* ---- Flow control state ---- */

    BOOLEAN         TxHeld;         /* Transmitter held by software    */
    BOOLEAN         XoffReceived;   /* Remote sent XOFF                */
    UCHAR           XonChar;
    UCHAR           XoffChar;

    /* ---- Statistics ---- */

    SERIALPERF_STATS PerfStats;
    SERIAL_COMMPROP  CommProp;      /* Capabilities for this port      */

    /* ---- Device identity ---- */

    PDEVICE_OBJECT  Self;           /* Back-pointer to our device obj  */
    PDEVICE_OBJECT  Pdo;            /* Physical device object (PnP)    */
    PDEVICE_OBJECT  NextLowerDevice;/* Next in device stack            */
    UNICODE_STRING  SymbolicLinkName;
    UNICODE_STRING  DeviceName;
    ULONG           ComPortNumber;  /* e.g., 3 for COM3                */

    /* ---- IRP queues (cancel-safe) ---- */

    IO_CSQ          ReadQueue;      /* Pending read IRPs               */
    IO_CSQ          WriteQueue;     /* Pending write IRPs              */
    IO_CSQ          WaitQueue;      /* Pending wait-on-mask IRPs       */
    LIST_ENTRY      ReadQueueHead;
    LIST_ENTRY      WriteQueueHead;
    LIST_ENTRY      WaitQueueHead;

} CYPORT_EXTENSION, *PCYPORT_EXTENSION;


/* ====================================================================
 * FUNCTION PROTOTYPES — with SAL annotations
 * ====================================================================
 * Every function gets:
 *   _IRQL_requires_max_(level) — max IRQL at entry
 *   _In_, _Out_, _Inout_ — parameter direction
 *   _Must_inspect_result_ — caller must check return value
 * ==================================================================== */

/* ---- cyenum.c: Driver entry, AddDevice, FDO PnP ---- */

DRIVER_INITIALIZE   DriverEntry;
DRIVER_UNLOAD       CyUnload;

/* ---- cyhw.c: Hardware abstraction ---- */

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
UCHAR
CyHwReadReg(
    _In_ PCYPORT_EXTENSION Ext,
    _In_ ULONG             RegOffset
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
CyHwWriteReg(
    _In_ PCYPORT_EXTENSION Ext,
    _In_ ULONG             RegOffset,
    _In_ UCHAR             Value
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
CyHwSelectChannel(
    _In_ PCYPORT_EXTENSION Ext
    );

_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSTATUS
CyHwDetectChip(
    _In_  PUCHAR    ChipBase,
    _Out_ PUCHAR    Revision,
    _Out_ PBOOLEAN  Is60MHz
    );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
CyHwInitChannel(
    _In_ PCYPORT_EXTENSION Ext
    );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
CyHwShutdownChannel(
    _In_ PCYPORT_EXTENSION Ext
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
const char *
CyRegName(
    _In_ ULONG Offset
    );

/* ---- cybaud.c: Baud rate ---- */

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
NTSTATUS
CyBaudSet(
    _In_ PCYPORT_EXTENSION Ext,
    _In_ ULONG             BaudRate
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
ULONG
CyBaudGet(
    _In_ PCYPORT_EXTENSION Ext
    );

/* ---- cyisr.c: Interrupt handling ---- */

KSERVICE_ROUTINE    CyIsr;
IO_DPC_ROUTINE      CyDpcForIsr;

/* ---- cyread.c: Read path ---- */

_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSTATUS
CyReadDispatch(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
    );

/* ---- cywrite.c: Write path ---- */

_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSTATUS
CyWriteDispatch(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
    );

/* ---- cyioctl.c: IOCTL dispatch ---- */

_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSTATUS
CyIoCtlDispatch(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP           Irp
    );

/* ---- cyflow.c: Flow control ---- */

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
CyFlowApply(
    _In_ PCYPORT_EXTENSION Ext
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
CyFlowCheckXonXoff(
    _In_ PCYPORT_EXTENSION Ext,
    _In_ UCHAR             ReceivedByte
    );


#endif /* CYPORT_H */
