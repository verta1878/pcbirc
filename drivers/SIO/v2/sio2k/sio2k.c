/* ====================================================================
 * SIO2K.SYS — Logical Layer Serial I/O Driver for OS/2
 * ====================================================================
 * Clean-room V2 reimplementation.
 *
 * SIO2K handles the OS/2 application interface:
 *   - Device headers (COMn or custom names)
 *   - Strategy entry point (INIT/OPEN/CLOSE/READ/WRITE/GENIO)
 *   - IOCtl dispatch — Category 01h (funcs 41h-74h, per-port async
 *     control) and Category 80h (funcs 90h-A3h, SIO2K-private:
 *     baud lock/unlock, port swap, modem/count/type queries used
 *     by MODES.EXE, LOGGER.EXE, PMLM.EXE)
 *   - DCB management
 *   - Per-port RX/TX ring buffers, Read()/Write() with DCB-timeout-
 *     bounded polling
 *   - Error/Event tracking
 *
 * All hardware interaction is delegated to physical layer drivers
 * (UART.SYS, ESP.SYS, VMODEM.SYS) via the IDC interface.
 *
 * Key V2 behaviors:
 *   - No UART touch at boot (hardware probed on first Open)
 *   - Config file (SIO2K.CFG) for advanced users
 *   - Custom device names (not limited to COMn)
 *   - 256 port limit
 *   - Coexistence with COM.SYS
 *   - Log file generation
 * ====================================================================
 */

typedef unsigned char   BYTE;
typedef unsigned short  USHORT;
typedef unsigned long   ULONG;
typedef void           *PVOID;
typedef int             BOOL;
#define TRUE  1
#define FALSE 0

/* Interrupt protection (hex W-01).
 * Real driver (OpenWatcom build): _disable()/_enable() are Watcom's
 * standard 16-bit intrinsics mapping directly to CLI/STI, declared
 * in <conio.h>. Guarded behind __WATCOMC__ (Watcom's predefined
 * compiler macro) so the gcc cross-compile used elsewhere in this
 * project purely for syntax-checking still builds — _disable()/
 * _enable() aren't available under gcc, and gcc can't process
 * Watcom's `_asm` blocks either (see spin_lock()/spin_unlock() below,
 * same reasoning). Not independently verified against a real Watcom
 * compiler — no toolchain available in this environment. */
#ifdef __WATCOMC__
#include <conio.h>
static void int_disable(void) { _disable(); }
static void int_enable(void)  { _enable(); }
#else
static void int_disable(void) { /* no-op under gcc syntax-check build */ }
static void int_enable(void)  { /* no-op under gcc syntax-check build */ }
#endif

/* ---------------------------------------------------------------------
 * SMP-safe locking (hex M04 — previously unimplemented anywhere in
 * either v1 or v2, despite v1's own tracking table marking it "V2
 * scope" as if v2 had already handled it)
 * ---------------------------------------------------------------------
 * int_disable()/int_enable() alone are NOT SMP-safe: on a multi-CPU
 * OS/2 kernel (e.g. an MP-enabled Warp Server build), CLI/STI only
 * affects the *current* CPU. A second CPU concurrently running this
 * same driver code — e.g. SIO2KCallback firing for one port on CPU 0
 * while a GENIO Read() request for another port runs on CPU 1 — is
 * not blocked by CPU 0's CLI. True mutual exclusion across CPUs needs
 * an actual spinlock (a shared flag other CPUs busy-wait on) in
 * addition to locally disabling interrupts, so the current CPU's own
 * ISR can't deadlock against itself while the lock is held.
 *
 * Uses the classic x86 XCHG-with-memory idiom: XCHG is *always*
 * atomic when one operand is memory (the CPU asserts the bus lock
 * automatically — no explicit LOCK prefix needed, per the Intel SDM).
 * Only compiled in for a real Watcom build (__WATCOMC__), since
 * inline `_asm` blocks are Watcom-specific syntax; gcc builds fall
 * back to a no-op, consistent with int_disable()/int_enable() above
 * and fine for single-CPU testing.
 *
 * NOT independently verified against a real OpenWatcom compiler or
 * real SMP hardware — no toolchain or multi-CPU test rig available
 * in this environment. Treat as a reviewed-but-unbuilt draft; a real
 * build should single-step this under a debugger before trusting it
 * on real MP hardware. */
static volatile BYTE g_spinlock = 0;

static void spin_lock(void)
{
#ifdef __WATCOMC__
    _asm {
        push    ax
    retry:
        mov     al, 1
        xchg    al, g_spinlock
        test    al, al
        jnz     retry
        pop     ax
    }
#endif
    /* gcc syntax-check build: no-op (single-CPU test context) */
}

static void spin_unlock(void)
{
#ifdef __WATCOMC__
    _asm {
        mov     g_spinlock, 0
    }
#endif
}

#include "sio2k_idc.h"
#include "cfgparse.h"
#include "driver.h"
#include "vsio.h"

/* driver.h assumes OS2_INCLUDED is defined by <os2.h> for FAR/PASCAL/
 * EXPENTRY; this file is built standalone for cross-compilation, so
 * supply plain fallbacks when the OS/2 toolchain headers aren't present. */
#ifndef FAR
#define FAR
#endif
#ifndef PASCAL
#define PASCAL
#endif

/* Publishes a PDD's entry point under a name so a VDD can retrieve it
 * (VSIO2K.SYS does this via VDHOpenPDD/PDDCMD_REGISTER — see
 * vsio2k.c). The real OS/2 kernel DevHelp routine for this varies by
 * toolkit version; declared here as a stub for the same reason the
 * other cross-compile fallbacks above exist. */
extern void DevHelp_RegisterPDD(const char *pszName, void FAR *pfnEntry);

/* -------------------------------------------------------------------- */
/* Constants                                                            */
/* -------------------------------------------------------------------- */

#define IOCTL_ASYNC         0x01
#define IOCTL_SIO2K         0x80
#define MAX_SIO2K_PORTS     256
#define DCB_SIZE            11
#define LOG_FILENAME        "\\SIO2K.LOG"

/* Logical-layer RX/TX ring buffers (used by Read()/Write()). Size
 * must stay a power of two: wrap uses a bitmask (SIZE-1), never
 * modulo — see hex W-03. Compile-time configurable via this define. */
#define SIO2K_RING_SIZE     1024
#define SIO2K_RING_MASK     (SIO2K_RING_SIZE - 1)

/* -------------------------------------------------------------------- */
/* DCB (Device Control Block) — per OS/2 ASYNC spec                     */
/* -------------------------------------------------------------------- */

typedef struct _SIO_DCB {
    USHORT  writeTimeout;       /* 1/100 sec, 0-based               */
    USHORT  readTimeout;        /* 1/100 sec, 0-based               */
    BYTE    flags1;             /* DTR mode, CTS/DSR enables        */
    BYTE    flags2;             /* XON/XOFF, RTS mode               */
    BYTE    flags3;             /* Timeout modes, FIFO, trigger     */
    BYTE    errorRepl;          /* Error replacement byte            */
    BYTE    breakRepl;          /* Break replacement byte            */
    BYTE    xonChar;            /* XON character (default 11h)       */
    BYTE    xoffChar;           /* XOFF character (default 13h)     */
} SIO_DCB;

/* DCB Flags */
#define F1_DTR_MASK     0x03
#define F1_DTR_DIS      0x00
#define F1_DTR_ENA      0x01
#define F1_CTS_ENA      0x08
#define F1_DSR_RX       0x40

#define F2_XON_RX       0x01
#define F2_XON_TX       0x02
#define F2_RTS_MASK     0xC0
#define F2_RTS_DIS      0x00
#define F2_RTS_ENA      0x40
#define F2_RTS_HS       0x80

#define F3_FORCED       0xD0    /* SIO forced: FIFO=ena + trig8 + txload */
#define F3_RTO_MASK     0x06
#define F3_RTO_NORM     0x02
#define F3_RTO_WAIT     0x04
#define F3_RTO_NOW      0x06

/* -------------------------------------------------------------------- */
/* Per-Port Logical State                                               */
/* -------------------------------------------------------------------- */

typedef struct _SIO2K_PORT {
    char            devName[9];     /* OS/2 device name                 */
    USHORT          openCount;      /* Number of active opens           */
    BOOL            isConfigured;   /* Port defined in config           */

    /* Physical driver binding */
    char            physDriverName[9];  /* IDC name (e.g., "UART$")     */
    USHORT          physPortIndex;      /* Port index within phys driver */
    PFN_PHYS_IDC    pfnPhys;           /* Physical driver entry point   */

    /* Logical state */
    SIO_DCB         dcb;
    USHORT          errorWord;      /* IOCtl 6Dh — resets on read       */
    USHORT          eventWord;      /* IOCtl 72h — resets on read       */
    ULONG           baudRate;
    BYTE            lineCtrl;       /* LCR shadow                       */
    BYTE            mcrShadow;      /* MCR shadow (DTR/RTS)             */
    BOOL            txHeld;         /* IOCtl 47h hold                   */
    BOOL            breakActive;    /* Break signal active               */
    BYTE            txImmChar;      /* Transmit immediate byte           */
    BOOL            txImmPending;   /* Immediate byte pending            */

    /* Config options */
    ULONG           lockedBaud;
    BOOL            baudLocked;
    BOOL            os2Shares;

    /* State tracking */
    BOOL            hwProbed;       /* Hardware probed on first open     */
    PHYS_PORT_INFO  hwInfo;         /* Cached from physical driver       */

    /* Logical-layer ring buffers for Read()/Write(). Pointers are set
     * from the fixed per-port pool in PortOpen() the first time the
     * port is opened; NULL beforehand. */
    BYTE           *rxRing;
    USHORT          rxHead, rxTail, rxCount;
    BYTE           *txRing;
    USHORT          txHead, txTail, txCount;
} SIO2K_PORT;

/* Backing storage for the ring buffers above — one slot per port,
 * indexed the same way as g_ports[]. Kept as a separate pool (rather
 * than embedded arrays in SIO2K_PORT) so SIO2K_PORT stays cheap to
 * copy/swap wholesale in IOCtl92 (SWAPPORTS). */
static BYTE g_rxRingPool[MAX_SIO2K_PORTS][SIO2K_RING_SIZE];
static BYTE g_txRingPool[MAX_SIO2K_PORTS][SIO2K_RING_SIZE];

/* -------------------------------------------------------------------- */
/* Globals                                                              */
/* -------------------------------------------------------------------- */

static SIO2K_PORT   g_ports[MAX_SIO2K_PORTS];
static USHORT       g_numPorts = 0;
static SIO2K_CONFIG g_config;
static ULONG        g_devHlp = 0;
static USHORT       g_logHandle = 0xFFFF;


/* -------------------------------------------------------------------- */
/* Logging                                                              */
/* -------------------------------------------------------------------- */

static void LogOpen(void)
{
    /* Would use DosOpen or INT 21h during INIT */
    /* Simplified for cross-compilation */
}

static void LogMsg(const char *msg)
{
    /* Write to log file if open */
    (void)msg;
}

static void LogClose(void)
{
    if (g_logHandle != 0xFFFF) {
        g_logHandle = 0xFFFF;
    }
}


/* -------------------------------------------------------------------- */
/* DCB Defaults                                                         */
/* -------------------------------------------------------------------- */

static void InitDCB(SIO_DCB *dcb)
{
    dcb->writeTimeout = 100;    /* 1 second */
    dcb->readTimeout  = 100;
    dcb->flags1       = F1_DTR_ENA;
    dcb->flags2       = F2_RTS_ENA;
    dcb->flags3       = F3_FORCED | F3_RTO_NORM;
    dcb->errorRepl    = 0;
    dcb->breakRepl    = 0;
    dcb->xonChar      = 0x11;
    dcb->xoffChar     = 0x13;
}


/* -------------------------------------------------------------------- */
/* Physical Driver Communication                                        */
/* -------------------------------------------------------------------- */

static USHORT PhysCall(SIO2K_PORT *port, USHORT cmd,
                       void *pData, USHORT cbData)
{
    if (!port->pfnPhys) return 0xFFFF;
    return port->pfnPhys(cmd, port->physPortIndex, pData, cbData);
}


/* -------------------------------------------------------------------- */
/* IOCtl Implementation                                                 */
/* -------------------------------------------------------------------- */

/* 41h: Set Baud Rate */
static USHORT IOCtl41(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    ULONG baud;
    if (port->baudLocked) return 0; /* Silently ignore */
    baud = (ULONG)(*(USHORT *)parm);
    if (baud < 50) return 1;
    port->baudRate = baud;
    return PhysCall(port, PHYS_CMD_SETBAUD, &baud, 4);
}

/* 42h: Set Line Control */
static USHORT IOCtl42(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE lcr = 0;
    BYTE dbits = parm[0], parity = parm[1], stop = parm[2];

    if (dbits < 5 || dbits > 8) return 1;
    lcr = dbits - 5;

    if (parity > 0) {
        lcr |= 0x08;   /* PEN */
        if (parity == 2) lcr |= 0x10;          /* Even: PEN+EPS */
        else if (parity == 3) lcr |= 0x20;     /* Mark: PEN+STICK */
        else if (parity == 4) lcr |= 0x30;     /* Space: PEN+EPS+STICK */
        /* Odd: just PEN */
    }
    if (stop > 0) lcr |= 0x04;

    port->lineCtrl = lcr;
    return PhysCall(port, PHYS_CMD_SETLINE, &lcr, 1);
}

/* 43h: Extended Set Baud */
static USHORT IOCtl43(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    ULONG baud;
    if (port->baudLocked) return 0;
    baud = *(ULONG *)parm;
    if (baud < 50) return 1;
    port->baudRate = baud;
    return PhysCall(port, PHYS_CMD_SETBAUD, &baud, 4);
}

/* 44h: Transmit Immediate */
static USHORT IOCtl44(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    return PhysCall(port, PHYS_CMD_TXBYTE, parm, 1);
}

/* 45h: Set Break Off */
static USHORT IOCtl45(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    port->breakActive = FALSE;
    PhysCall(port, PHYS_CMD_BREAKOFF, NULL, 0);
    if (data) *(USHORT *)data = port->errorWord;
    return 0;
}

/* 46h: Set Modem Control */
static USHORT IOCtl46(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE mcr = port->mcrShadow;
    BYTE onMask = parm[0], offMask = parm[1];

    /* DTR */
    if ((port->dcb.flags1 & F1_DTR_MASK) != 0x02) { /* Not HS */
        if (onMask & 0x01) mcr |= 0x01;
        else if (!(offMask & 0x01)) mcr &= ~0x01;
    }
    /* RTS */
    if ((port->dcb.flags2 & F2_RTS_MASK) != F2_RTS_HS) {
        if (onMask & 0x02) mcr |= 0x02;
        else if (!(offMask & 0x02)) mcr &= ~0x02;
    }

    port->mcrShadow = mcr;
    PhysCall(port, PHYS_CMD_SETMCR, &mcr, 1);
    if (data) *(USHORT *)data = port->errorWord;
    return 0;
}

/* 47h: Hold Transmit */
static USHORT IOCtl47(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    port->txHeld = TRUE;
    PhysCall(port, PHYS_CMD_TXSTOP, NULL, 0);
    return 0;
}

/* 48h: Start Transmit */
static USHORT IOCtl48(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    port->txHeld = FALSE;
    PhysCall(port, PHYS_CMD_TXSTART, NULL, 0);
    return 0;
}

/* 4Bh: Set Break On */
static USHORT IOCtl4B(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    port->breakActive = TRUE;
    PhysCall(port, PHYS_CMD_BREAKON, NULL, 0);
    if (data) *(USHORT *)data = port->errorWord;
    return 0;
}

/* 53h: Write DCB */
static USHORT IOCtl53(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE *src = parm;
    BYTE *dst = (BYTE *)&port->dcb;
    int i;
    for (i = 0; i < DCB_SIZE; i++) dst[i] = src[i];
    /* Force SIO bits in flags3 */
    port->dcb.flags3 = (port->dcb.flags3 & 0x07) | F3_FORCED;
    /* Apply flow control to physical driver */
    {
        BYTE flow = 0;
        if (port->dcb.flags2 & F2_XON_RX) flow |= FLOW_XONXOFF_TX;
        if (port->dcb.flags2 & F2_XON_TX) flow |= FLOW_XONXOFF_RX;
        if (port->dcb.flags1 & F1_CTS_ENA) flow |= FLOW_CTS;
        if ((port->dcb.flags2 & F2_RTS_MASK) == F2_RTS_HS) flow |= FLOW_RTS;
        PhysCall(port, PHYS_CMD_SETFLOW, &flow, 1);
    }
    return 0;
}

/* 54h: Write Enhanced (ignored) */
static USHORT IOCtl54(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{ return 0; }

/* 61h: Query Baud Rate */
static USHORT IOCtl61(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    *(USHORT *)data = (USHORT)port->baudRate;
    return 0;
}

/* 62h: Query Line Control */
static USHORT IOCtl62(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE lcr = port->lineCtrl;
    data[0] = (lcr & 0x03) + 5;    /* Data bits */
    /* Parity */
    if (!(lcr & 0x08)) data[1] = 0;
    else if (lcr & 0x20) data[1] = (lcr & 0x10) ? 4 : 3;
    else data[1] = (lcr & 0x10) ? 2 : 1;
    data[2] = (lcr & 0x04) ? 2 : 0;
    data[3] = port->breakActive ? 1 : 0;
    return 0;
}

/* 63h: Extended Query Baud */
static USHORT IOCtl63(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    *(ULONG *)&data[0] = port->baudRate;
    data[4] = 0;
    *(ULONG *)&data[5] = 50;
    data[9] = 0;
    *(ULONG *)&data[10] = port->hwInfo.maxBaud ? port->hwInfo.maxBaud : 115200;
    data[14] = 0;
    return 0;
}

/* 64h: Query Status */
static USHORT IOCtl64(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE status = 0;
    BYTE msr;
    PhysCall(port, PHYS_CMD_GETMSR, &msr, 1);
    if ((port->dcb.flags1 & F1_CTS_ENA) && !(msr & 0x10)) status |= 0x01;
    if (port->txHeld) status |= 0x08;
    if (port->breakActive) status |= 0x20;
    if (port->txImmPending) status |= 0x40;
    data[0] = status;
    return 0;
}

/* 65h: Query TX Status */
static USHORT IOCtl65(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE status = 0;
    USHORT txInfo[2] = {0};
    PhysCall(port, PHYS_CMD_TXCOUNT, txInfo, 4);
    if (txInfo[0] > 0) status |= 0x02;
    if (port->txImmPending) status |= 0x08;
    data[0] = status;
    return 0;
}

/* 66h: Query Modem Output */
static USHORT IOCtl66(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    data[0] = port->mcrShadow & 0x03;
    return 0;
}

/* 67h: Query Modem Input */
static USHORT IOCtl67(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE msr;
    PhysCall(port, PHYS_CMD_GETMSR, &msr, 1);
    data[0] = msr & 0xF0;
    return 0;
}

/* 68h: Query RX Count */
static USHORT IOCtl68(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    return PhysCall(port, PHYS_CMD_RXCOUNT, data, 4);
}

/* 69h: Query TX Count */
static USHORT IOCtl69(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    return PhysCall(port, PHYS_CMD_TXCOUNT, data, 4);
}

/* 6Dh: Query Error (resets) */
static USHORT IOCtl6D(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    *(USHORT *)data = port->errorWord;
    port->errorWord = 0;
    return 0;
}

/* 72h: Query Event (resets) */
static USHORT IOCtl72(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    *(USHORT *)data = port->eventWord;
    port->eventWord = 0;
    return 0;
}

/* 73h: Read DCB */
static USHORT IOCtl73(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE *src = (BYTE *)&port->dcb;
    int i;
    for (i = 0; i < DCB_SIZE; i++) data[i] = src[i];
    return 0;
}

/* 74h: Read Enhanced */
static USHORT IOCtl74(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    data[0] = 0;
    *(ULONG *)&data[1] = 0;
    return 0;
}


/* -------------------------------------------------------------------- */
/* IOCtl Dispatcher (Category 01h — async device IOCtls)                */
/* -------------------------------------------------------------------- */

typedef USHORT (*PFN_IOCTL)(SIO2K_PORT *, BYTE *, BYTE *);

static const struct { BYTE code; PFN_IOCTL fn; } g_ioctlTable[] = {
    { 0x41, IOCtl41 }, { 0x42, IOCtl42 }, { 0x43, IOCtl43 },
    { 0x44, IOCtl44 }, { 0x45, IOCtl45 }, { 0x46, IOCtl46 },
    { 0x47, IOCtl47 }, { 0x48, IOCtl48 }, { 0x4B, IOCtl4B },
    { 0x53, IOCtl53 }, { 0x54, IOCtl54 },
    { 0x61, IOCtl61 }, { 0x62, IOCtl62 }, { 0x63, IOCtl63 },
    { 0x64, IOCtl64 }, { 0x65, IOCtl65 }, { 0x66, IOCtl66 },
    { 0x67, IOCtl67 }, { 0x68, IOCtl68 }, { 0x69, IOCtl69 },
    { 0x6D, IOCtl6D }, { 0x72, IOCtl72 }, { 0x73, IOCtl73 },
    { 0x74, IOCtl74 },
    { 0, 0 }
};

static USHORT DispatchIOCtl(SIO2K_PORT *port, BYTE func,
                            BYTE *parm, BYTE *data)
{
    int i;
    for (i = 0; g_ioctlTable[i].fn; i++) {
        if (g_ioctlTable[i].code == func)
            return g_ioctlTable[i].fn(port, parm, data);
    }
    return 0x8103;  /* Invalid command */
}


/* -------------------------------------------------------------------- */
/* IOCtl Implementation — Category 80h (SIO2K-private, used by          */
/* MODES.EXE, LOGGER.EXE, PMLM.EXE)                                     */
/* -------------------------------------------------------------------- */

/* Case-insensitive-free, NUL/space-padded 8-char device name compare,
 * matching the fixed-width name copy done in InitDriver(). No libc
 * string.h dependency — this file uses manual byte loops throughout. */
static BOOL NameEq(const char *a, const char *b)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (a[i] != b[i]) return FALSE;
        if (a[i] == '\0') return TRUE;
    }
    return TRUE;
}

static SIO2K_PORT *FindPortByName(const char *name)
{
    int i;
    for (i = 0; i < g_numPorts; i++) {
        if (NameEq(g_ports[i].devName, name)) return &g_ports[i];
    }
    return (SIO2K_PORT *)0;
}

/* 90h: Lock Baud Rate at current setting */
static USHORT IOCtl90(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    port->lockedBaud = port->baudRate;
    port->baudLocked = TRUE;
    return 0;
}

/* 91h: Unlock Baud Rate */
static USHORT IOCtl91(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    port->baudLocked = FALSE;
    return 0;
}

/* 92h: Swap Port Assignments — parm is a NUL-terminated target device
 * name (as sent by MODES.EXE). Swaps the physical-driver bindings of
 * the two logical ports, so each OS/2 device name now talks to the
 * other's hardware. */
static USHORT IOCtl92(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    SIO2K_PORT *target;
    char          tmpPhys[9];
    USHORT        tmpIdx;
    PFN_PHYS_IDC  tmpFn;
    BOOL          tmpProbed;
    PHYS_PORT_INFO tmpInfo;
    int i;

    target = FindPortByName((const char *)parm);
    if (!target || target == port) return 0x8103;  /* Invalid command */

    for (i = 0; i < 9; i++) tmpPhys[i] = port->physDriverName[i];
    tmpIdx    = port->physPortIndex;
    tmpFn     = port->pfnPhys;
    tmpProbed = port->hwProbed;
    tmpInfo   = port->hwInfo;

    for (i = 0; i < 9; i++) port->physDriverName[i] = target->physDriverName[i];
    port->physPortIndex = target->physPortIndex;
    port->pfnPhys       = target->pfnPhys;
    port->hwProbed      = target->hwProbed;
    port->hwInfo        = target->hwInfo;

    for (i = 0; i < 9; i++) target->physDriverName[i] = tmpPhys[i];
    target->physPortIndex = tmpIdx;
    target->pfnPhys       = tmpFn;
    target->hwProbed      = tmpProbed;
    target->hwInfo        = tmpInfo;

    return 0;
}

/* A1h: Get Modem Signals (combined output+input byte for monitor tools).
 * Bits match IOCtl66 (Query Modem Output: bit0=DTR, bit1=RTS) combined
 * with IOCtl67 (Query Modem Input: bit4=CTS, bit5=DSR, bit6=RI, bit7=DCD)
 * so callers can use either the individual queries or this combined one. */
static USHORT IOCtlA1(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    BYTE msr = 0;
    PhysCall(port, PHYS_CMD_GETMSR, &msr, 1);
    data[0] = (port->mcrShadow & 0x03) | (msr & 0xF0);
    return 0;
}

/* A2h: Get Byte Counts — two ULONGs: { rxCount, txCount } */
static USHORT IOCtlA2(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    PhysCall(port, PHYS_CMD_RXCOUNT, &data[0], 4);
    PhysCall(port, PHYS_CMD_TXCOUNT, &data[4], 4);
    return 0;
}

/* A3h: Get UART/physical-layer type (PHYS_UART_* from sio2k_idc.h) —
 * lets monitor tools (PMLM) tell real vs. virtual (VMODEM) vs. ESP
 * ports apart without guessing from the port number. */
static USHORT IOCtlA3(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    data[0] = port->hwInfo.uartType;
    return 0;
}

/* A4h: Re-probe port hardware (hex M05 — PCMCIA hot-plug, partial).
 *
 * Once a port is probed (PortOpen's first call), hwInfo is cached
 * for the rest of the driver's lifetime — there was previously no
 * way to ask SIO2K to look again. That's a real gap for a PCMCIA
 * serial card: unlike a fixed ISA/PCI UART, a PCMCIA card can be
 * physically removed and a different one inserted while the driver
 * stays loaded, and the cached hwInfo would silently go stale.
 *
 * This is a MANUAL on-demand re-probe an admin utility can trigger
 * (e.g. after swapping a card) — it is NOT automatic hot-plug
 * detection. Genuine automatic detection needs the port driver to
 * register for OS/2 Card Services insertion/removal events, which
 * is a whole separate subsystem this clean-room build doesn't
 * integrate with (no verified Card Services interface available in
 * this environment to build against). Documented as a partial step
 * rather than claiming full hot-plug support it doesn't have. */
static USHORT IOCtlA4(SIO2K_PORT *port, BYTE *parm, BYTE *data)
{
    USHORT rc;

    if (port->openCount == 0) return 0x8103;  /* Invalid command — not open */

    rc = PhysCall(port, PHYS_CMD_OPEN, NULL, 0);
    if (rc != 0) {
        /* Card is physically gone — don't keep serving stale info
         * from before it was removed. */
        port->hwProbed = FALSE;
        return 0x8110;  /* Not ready */
    }

    PhysCall(port, PHYS_CMD_GETINFO, &port->hwInfo, sizeof(PHYS_PORT_INFO));
    port->hwProbed = TRUE;
    return 0;
}

static const struct { BYTE code; PFN_IOCTL fn; } g_ioctl80Table[] = {
    { 0x90, IOCtl90 }, { 0x91, IOCtl91 }, { 0x92, IOCtl92 },
    { 0xA1, IOCtlA1 }, { 0xA2, IOCtlA2 }, { 0xA3, IOCtlA3 },
    { 0xA4, IOCtlA4 },
    { 0, 0 }
};

static USHORT DispatchIOCtl80(SIO2K_PORT *port, BYTE func,
                              BYTE *parm, BYTE *data)
{
    int i;
    for (i = 0; g_ioctl80Table[i].fn; i++) {
        if (g_ioctl80Table[i].code == func)
            return g_ioctl80Table[i].fn(port, parm, data);
    }
    return 0x8103;  /* Invalid command */
}


/* -------------------------------------------------------------------- */
/* Open / Close                                                         */
/* -------------------------------------------------------------------- */

static USHORT PortOpen(SIO2K_PORT *port)
{
    if (port->openCount > 0) {
        /* Additional (shared) open */
        port->openCount++;
        return 0;
    }

    /* Initial open — probe hardware via physical driver */
    if (!port->hwProbed) {
        USHORT rc = PhysCall(port, PHYS_CMD_OPEN, NULL, 0);
        if (rc != 0) return 0x8110;     /* Not ready */

        /* Cache hardware info */
        PhysCall(port, PHYS_CMD_GETINFO, &port->hwInfo,
                 sizeof(PHYS_PORT_INFO));
        port->hwProbed = TRUE;

        LogMsg("Port opened, hardware probed");
    }

    /* Bind this port's ring buffers from the fixed pool (hex W-05
     * follow-up: Read()/Write() need somewhere to buffer through). */
    if (!port->rxRing) {
        USHORT idx = (USHORT)(port - g_ports);
        port->rxRing = g_rxRingPool[idx];
        port->txRing = g_txRingPool[idx];
        port->rxHead = port->rxTail = port->rxCount = 0;
        port->txHead = port->txTail = port->txCount = 0;
    }

    /* Set DCB defaults */
    InitDCB(&port->dcb);

    /* Set DTR per DCB */
    if ((port->dcb.flags1 & F1_DTR_MASK) == F1_DTR_ENA) {
        port->mcrShadow |= 0x01;
        PhysCall(port, PHYS_CMD_SETMCR, &port->mcrShadow, 1);
    }

    /* Set RTS per DCB */
    if ((port->dcb.flags2 & F2_RTS_MASK) == F2_RTS_ENA) {
        port->mcrShadow |= 0x02;
        PhysCall(port, PHYS_CMD_SETMCR, &port->mcrShadow, 1);
    }

    /* Apply locked baud if configured */
    if (port->baudLocked) {
        port->baudRate = port->lockedBaud;
        PhysCall(port, PHYS_CMD_SETBAUD, &port->lockedBaud, 4);
    }

    /* Clear error/event */
    port->errorWord = 0;
    port->eventWord = 0;

    /* Flush buffers */
    PhysCall(port, PHYS_CMD_RXFLUSH, NULL, 0);
    PhysCall(port, PHYS_CMD_TXFLUSH, NULL, 0);

    port->openCount = 1;
    return 0;
}

static USHORT PortClose(SIO2K_PORT *port)
{
    if (port->openCount == 0) return 0x8106;

    port->openCount--;
    if (port->openCount > 0) return 0;  /* Intermediate close */

    /* Last close */
    /* Drop DTR if enabled mode */
    if ((port->dcb.flags1 & F1_DTR_MASK) == F1_DTR_ENA) {
        port->mcrShadow &= ~0x01;
        PhysCall(port, PHYS_CMD_SETMCR, &port->mcrShadow, 1);
    }

    /* Drop RTS if enabled mode */
    if ((port->dcb.flags2 & F2_RTS_MASK) == F2_RTS_ENA) {
        port->mcrShadow &= ~0x02;
        PhysCall(port, PHYS_CMD_SETMCR, &port->mcrShadow, 1);
    }

    PhysCall(port, PHYS_CMD_CLOSE, NULL, 0);
    return 0;
}


/* -------------------------------------------------------------------- */
/* Ring buffer primitives — wrap via bitmask, not modulo (hex W-03)     */
/* -------------------------------------------------------------------- */

static BOOL RingPush(BYTE *ring, USHORT *tail, USHORT *count, BYTE b)
{
    if (*count >= SIO2K_RING_SIZE) return FALSE;   /* full */
    ring[*tail] = b;
    *tail = (USHORT)((*tail + 1) & SIO2K_RING_MASK);
    (*count)++;
    return TRUE;
}

static BOOL RingPop(BYTE *ring, USHORT *head, USHORT *count, BYTE *out)
{
    if (*count == 0) return FALSE;   /* empty */
    *out = ring[*head];
    *head = (USHORT)((*head + 1) & SIO2K_RING_MASK);
    (*count)--;
    return TRUE;
}


/* -------------------------------------------------------------------- */
/* Read / Write — logical-layer buffering with DCB timeout             */
/* -------------------------------------------------------------------- */

/* Length/actual-count pair used for READ and WRITE requests. The
 * simplified Request_packet in driver.h has no dedicated transfer
 * fields, so (matching the GENIO convention already used elsewhere
 * in this file) GioParams points at one of these and GioData points
 * at the caller's data buffer. */
typedef struct _RWREQ {
    ULONG length;    /* [in]  bytes requested */
    ULONG actual;    /* [out] bytes actually transferred */
} RWREQ;

static USHORT PortRead(SIO2K_PORT *port, RWREQ *req, BYTE *buf)
{
    ULONG  got = 0;
    USHORT retries;

    if (!port->rxRing) return 0x8110;  /* Not ready — never opened */

    /* Drain whatever's already buffered locally. Locked (hex M04):
     * rxHead/rxCount are also touched by SIO2KCallback's RXDATA
     * handler, which can fire on another CPU concurrently. */
    int_disable();
    spin_lock();
    while (got < req->length &&
           RingPop(port->rxRing, &port->rxHead, &port->rxCount, &buf[got]))
        got++;
    spin_unlock();
    int_enable();

    /* Top up from the physical layer's own buffer for whatever is
     * still short. dcb.readTimeout (1/100 sec) bounds the number of
     * refill attempts: an honest polling approximation of a blocking
     * Read. A true blocking wait would need DevHelp_ProcBlock, which
     * this clean-room build doesn't hook up — but the timeout value
     * is at least used rather than silently ignored. */
    retries = port->dcb.readTimeout ? port->dcb.readTimeout : 1;
    while (got < req->length && retries-- > 0) {
        BYTE   tmp[64];
        USHORT want = (USHORT)((req->length - got) > sizeof(tmp) ?
                                sizeof(tmp) : (req->length - got));
        USHORT n = (USHORT)PhysCall(port, PHYS_CMD_RXBLOCK, tmp, want);
        USHORT i;
        for (i = 0; i < n && got < req->length; i++) buf[got++] = tmp[i];
    }

    req->actual = got;
    return 0;
}

static USHORT PortWrite(SIO2K_PORT *port, RWREQ *req, BYTE *buf)
{
    ULONG queued = 0;

    if (!port->txRing) return 0x8110;  /* Not ready — never opened */
    if (port->txHeld) { req->actual = 0; return 0; }

    /* Queue as much as the ring holds, then drain it down to the
     * physical layer. Locked as one unit (hex M04): txTail/txHead/
     * txCount could otherwise be touched by a concurrent PortWrite()
     * or SioPddEntry() call on another CPU mid-operation. */
    int_disable();
    spin_lock();
    while (queued < req->length &&
           RingPush(port->txRing, &port->txTail, &port->txCount, buf[queued]))
        queued++;

    {
        BYTE   tmp[64];
        USHORT n = 0;
        BYTE   b;
        while (port->txCount > 0 && n < sizeof(tmp) &&
               RingPop(port->txRing, &port->txHead, &port->txCount, &b))
            tmp[n++] = b;
        spin_unlock();
        int_enable();
        if (n > 0) PhysCall(port, PHYS_CMD_TXBLOCK, tmp, n);
    }

    req->actual = queued;
    return 0;
}


/* -------------------------------------------------------------------- */
/* PDD Entry Point for VSIO2K.SYS (DOS-session/VDM virtualization)      */
/* -------------------------------------------------------------------- */
/* Implements the SIOCMD_* contract from inc/vsio.h. VSIO2K.SYS obtains
 * a far pointer to this function during PDD registration (see
 * vsio2k.c's PDDCMD_REGISTER handling in VsioDevReq) and calls it
 * whenever a DOS program running in a VDM touches a virtualized COM
 * port, so SIO2K can service it the same way a real OS/2 Open()/
 * Read()/Write() would. */

static ULONG SioPddEntry(ULONG ulFunc, ULONG ul1, ULONG ul2)
{
    USHORT      idx = (USHORT)ul1;
    SIO2K_PORT *port;

    if (ulFunc == SIOCMD_GETPORTINFO) {
        PSIOPORT_INFO info = (PSIOPORT_INFO)ul2;
        if (!info) return 0;
        if (idx >= g_numPorts || !g_ports[idx].isConfigured) return 0;

        port = &g_ports[idx];
        info->ioBase     = port->hwInfo.ioBase;
        info->ioBaseDOS  = port->hwInfo.ioBase;  /* No remap table yet — identity map */
        info->irq        = port->hwInfo.irq;
        info->irqDOS     = port->hwInfo.irq;
        info->uartType   = port->hwInfo.uartType;
        info->isInternet = (port->hwInfo.uartType == PHYS_UART_VMODEM) ? 1 : 0;
        info->flags      = SPIF_EXISTS |
                            (port->openCount ? SPIF_OPEN : 0) |
                            (port->hwInfo.fifoSize > 1 ? SPIF_FIFO : 0);
        return 1;
    }

    if (idx >= g_numPorts) return 0;
    port = &g_ports[idx];

    switch (ulFunc) {

    case SIOCMD_OPENPORT:
        return (PortOpen(port) == 0) ? 1 : 0;

    case SIOCMD_CLOSEPORT:
        return (PortClose(port) == 0) ? 1 : 0;

    case SIOCMD_READBYTE:
        if (!ul2 || !port->rxRing) return 0;
        {
            USHORT ok;
            int_disable();
            spin_lock();    /* hex M04: same rxRing as PortRead()/RXDATA */
            ok = RingPop(port->rxRing, &port->rxHead, &port->rxCount,
                         (BYTE *)ul2) ? 1 : 0;
            spin_unlock();
            int_enable();
            return ok;
        }

    case SIOCMD_WRITEBYTE:
        if (!port->txRing) return 0;
        {
            BYTE   b;
            USHORT havebyte;
            int_disable();
            spin_lock();    /* hex M04: same txRing as PortWrite() */
            if (!RingPush(port->txRing, &port->txTail, &port->txCount, (BYTE)ul2)) {
                spin_unlock();
                int_enable();
                return 0;
            }
            havebyte = RingPop(port->txRing, &port->txHead, &port->txCount, &b) ? 1 : 0;
            spin_unlock();
            int_enable();
            if (havebyte) PhysCall(port, PHYS_CMD_TXBLOCK, &b, 1);
            return 1;
        }

    case SIOCMD_GETMSR:
        if (!ul2) return 0;
        PhysCall(port, PHYS_CMD_GETMSR, (BYTE *)ul2, 1);
        return 1;

    case SIOCMD_GETLSR:
        if (!ul2) return 0;
        PhysCall(port, PHYS_CMD_GETLSR, (BYTE *)ul2, 1);
        return 1;

    case SIOCMD_SETMCR:
        port->mcrShadow = (BYTE)ul2;
        PhysCall(port, PHYS_CMD_SETMCR, &port->mcrShadow, 1);
        return 1;

    case SIOCMD_SETBAUD:
        port->baudRate = (ULONG)ul2;
        PhysCall(port, PHYS_CMD_SETBAUD, &port->baudRate, 4);
        return 1;

    case SIOCMD_SETLCR:
        port->lineCtrl = (BYTE)ul2;
        PhysCall(port, PHYS_CMD_SETLINE, &port->lineCtrl, 1);
        return 1;

    case SIOCMD_SETFCR:
        {
            BYTE fcr = (BYTE)ul2;
            PhysCall(port, PHYS_CMD_SETFIFO, &fcr, 1);
        }
        return 1;

    case SIOCMD_SETIER:
        /* Which virtual interrupts a DOS program wants enabled is
         * VSIO2K's own concern for deciding when to raise a VIRQ.
         * SIO2K's SIOCMD_NOTIFY isn't filtered by IER, so this is
         * acknowledged only. */
        return 1;

    case SIOCMD_TXREADY:
        return (port->txCount < SIO2K_RING_SIZE) ? 1 : 0;

    case SIOCMD_RXREADY:
        return (port->rxCount > 0) ? 1 : 0;

    default:
        return 0;
    }
}


/* -------------------------------------------------------------------- */
/* SIO2K Callback — called by physical driver on events                 */
/* -------------------------------------------------------------------- */

static USHORT SIO2KCallback(USHORT event, USHORT portHandle,
                            void *pData, USHORT cbData)
{
    SIO2K_PORT *port;
    if (portHandle >= g_numPorts) return 1;
    port = &g_ports[portHandle];

    switch (event) {
    case SIO2K_CB_RXDATA:
        int_disable();
        spin_lock();    /* hex M04: rxTail/rxCount are touched from
                         * PortRead()/SioPddEntry() too, which can run
                         * on another CPU while this callback fires */
        port->eventWord |= 0x0001;
        /* Copy the newly-arrived bytes into the logical-layer ring
         * buffer, if one has been bound (i.e. the port is open), so
         * Read() has something to drain without a physical-layer
         * round trip. Overflow -> Data Overrun (matches errorWord
         * bit already used elsewhere in this file for LSR overrun). */
        if (port->rxRing && pData && cbData > 0) {
            USHORT i;
            BYTE *src = (BYTE *)pData;
            for (i = 0; i < cbData; i++) {
                if (!RingPush(port->rxRing, &port->rxTail, &port->rxCount, src[i])) {
                    port->errorWord |= 0x0002;  /* Overrun */
                    break;
                }
            }
        }
        spin_unlock();
        int_enable();
        break;
    case SIO2K_CB_TXEMPTY:
        int_disable();
        port->eventWord |= 0x0004;
        int_enable();
        break;
    case SIO2K_CB_MSRCHANGE:
        int_disable();
        port->eventWord |= 0x0008;
        int_enable();
        break;
    case SIO2K_CB_LSREVENT:
        int_disable();  /* hex W-01: protect vs IOCtl read */
        port->eventWord |= 0x0080;
        if (pData && cbData >= 1) {
            BYTE lsr = *(BYTE *)pData;
            if (lsr & 0x02) port->errorWord |= 0x0002;
            if (lsr & 0x04) port->errorWord |= 0x0004;
            if (lsr & 0x08) port->errorWord |= 0x0008;
        }
        int_enable();
        break;
    case SIO2K_CB_BREAK:
        int_disable();
        port->eventWord |= 0x0040;
        int_enable();
        break;
    }
    return 0;
}


/* -------------------------------------------------------------------- */
/* Driver Initialization                                                */
/* -------------------------------------------------------------------- */

static void InitDriver(void)
{
    int i;

    LogOpen();
    LogMsg("SIO2K v2.00 initializing");

    /* Try to load config file */
    if (CfgParse("\\SIO2K.CFG", &g_config) != 0) {
        /* No config file — use defaults */
        CfgDefault(&g_config);
        LogMsg("No config file, using defaults");
    } else {
        LogMsg("Config file loaded");
    }

    /* Set up logical ports from config */
    g_numPorts = (USHORT)g_config.numOs2Dev;

    for (i = 0; i < g_numPorts && i < MAX_SIO2K_PORTS; i++) {
        SIO2K_PORT *port = &g_ports[i];
        OS2DEV_CFG *cfg = &g_config.os2dev[i];

        /* Copy device name */
        {
            int j;
            for (j = 0; j < 8 && cfg->name[j]; j++)
                port->devName[j] = cfg->name[j];
            port->devName[j] = '\0';
        }

        /* Set physical driver binding */
        {
            int j;
            for (j = 0; j < 8 && cfg->altDriverName[j]; j++)
                port->physDriverName[j] = cfg->altDriverName[j];
            port->physDriverName[j] = '\0';
        }
        port->physPortIndex = cfg->altDriverPort;

        /* Apply config options */
        port->lockedBaud = cfg->lockedBaud;
        port->baudLocked = cfg->baudLocked;
        port->os2Shares  = cfg->os2Shares;

        /* Initialize state */
        port->openCount = 0;
        port->hwProbed  = FALSE;
        port->baudRate  = 9600;
        port->lineCtrl  = 0x03;    /* 8N1 */
        port->mcrShadow = 0;
        port->errorWord = 0;
        port->eventWord = 0;
        port->txHeld    = FALSE;
        port->breakActive = FALSE;
        port->isConfigured = TRUE;

        InitDCB(&port->dcb);

        /* Physical driver entry point will be resolved via IDC
         * during InitComplete when all drivers are loaded */
        port->pfnPhys = (PFN_PHYS_IDC)0;
    }

    LogMsg("Initialization complete");
    LogClose();
}


/* -------------------------------------------------------------------- */
/* Strategy Routine — OS/2 device driver entry point                    */
/* -------------------------------------------------------------------- */
/* Every request from OS/2 (DosOpen/DosClose/DosDevIOCtl/driver INIT)
 * arrives here as a Request_packet. This is the piece that was missing
 * before: without it, PortOpen/PortClose/DispatchIOCtl/DispatchIOCtl80
 * above are never actually called by anything. */

void FAR PASCAL Strategy(lpRequest rp)
{
    SIO2K_PORT *port;
    USHORT rc = 0;

    switch (rp->reqcommand) {

    case INIT:
        InitDriver();
        rp->reqstatus = DONE_BIT;
        return;

    case INIT_COMPLETE:
        /* All drivers are loaded by now — publish SioPddEntry so
         * VSIO2K.SYS can retrieve it via VDHOpenPDD/PDDCMD_REGISTER
         * and route DOS-session (VDM) port access through us. */
        DevHelp_RegisterPDD("SIO2K$", (void FAR *)SioPddEntry);
        rp->reqstatus = DONE_BIT;
        return;

    case OPEN:
        if (rp->devunit >= g_numPorts) {
            rp->reqstatus = DONE_BIT | GEN_ERR | ERR_UNKNOWNCOMMAND;
            return;
        }
        rc = PortOpen(&g_ports[rp->devunit]);
        break;

    case CLOSE:
        if (rp->devunit >= g_numPorts) {
            rp->reqstatus = DONE_BIT | GEN_ERR | ERR_UNKNOWNCOMMAND;
            return;
        }
        rc = PortClose(&g_ports[rp->devunit]);
        break;

    case READ:
        if (rp->devunit >= g_numPorts) {
            rp->reqstatus = DONE_BIT | GEN_ERR | ERR_UNKNOWNCOMMAND;
            return;
        }
        port = &g_ports[rp->devunit];
        rc = PortRead(port, (RWREQ *)rp->GioParams, (BYTE *)rp->GioData);
        break;

    case WRITE:
        if (rp->devunit >= g_numPorts) {
            rp->reqstatus = DONE_BIT | GEN_ERR | ERR_UNKNOWNCOMMAND;
            return;
        }
        port = &g_ports[rp->devunit];
        rc = PortWrite(port, (RWREQ *)rp->GioParams, (BYTE *)rp->GioData);
        break;

    case GENIO:
        if (rp->devunit >= g_numPorts) {
            rp->reqstatus = DONE_BIT | GEN_ERR | ERR_UNKNOWNCOMMAND;
            return;
        }
        port = &g_ports[rp->devunit];

        if (rp->fcategory == IOCTL_ASYNC)
            rc = DispatchIOCtl(port, rp->fcode,
                                (BYTE *)rp->GioParams, (BYTE *)rp->GioData);
        else if (rp->fcategory == IOCTL_SIO2K)
            rc = DispatchIOCtl80(port, rp->fcode,
                                  (BYTE *)rp->GioParams, (BYTE *)rp->GioData);
        else
            rc = 0x8103;  /* Invalid command / category */
        break;

    default:
        rp->reqstatus = DONE_BIT | GEN_ERR | ERR_UNKNOWNCOMMAND;
        return;
    }

    rp->reqstatus = (USHORT)(DONE_BIT | (rc ? (GEN_ERR | (rc & 0xFF)) : 0));
}
