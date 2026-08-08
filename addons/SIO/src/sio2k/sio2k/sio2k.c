/* ====================================================================
 * SIO2K.SYS — Logical Layer Serial I/O Driver for OS/2
 * ====================================================================
 * Clean-room V2 reimplementation.
 *
 * SIO2K handles the OS/2 application interface:
 *   - Device headers (COMn or custom names)
 *   - IOCtl dispatch (Category 01h, functions 41h-74h)
 *   - DCB management
 *   - Read/Write with timeout and blocking
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

/* Interrupt protection (hex W-01) */
static void int_disable(void) { /* _disable() or CLI in real driver */ }
static void int_enable(void) { /* _enable() or STI in real driver */ }

#include "sio2k_idc.h"
#include "cfgparse.h"

/* -------------------------------------------------------------------- */
/* Constants                                                            */
/* -------------------------------------------------------------------- */

#define IOCTL_ASYNC         0x01
#define MAX_SIO2K_PORTS     256
#define DCB_SIZE            11
#define LOG_FILENAME        "\\SIO2K.LOG"

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
} SIO2K_PORT;

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
/* IOCtl Dispatcher                                                     */
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
        port->eventWord |= 0x0001;
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
