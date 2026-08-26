/* ====================================================================
 * esp.c — ESP.SYS Hayes Enhanced Serial Port Physical Driver for OS/2
 * ====================================================================
 * Physical layer driver for the Hayes ESP card (ComBic UART).
 * Registers with SIO2K.SYS via IDC as a physical driver.
 *
 * The ComBic is a 16550-compatible UART with:
 *   - 1024-byte hardware FIFOs (TX and RX)
 *   - On-chip RTS/CTS and XON/XOFF flow control
 *   - Enhanced mode with exact FIFO byte count access
 *   - DMA transfers for block I/O
 *   - Maximum bit rate: 921,600 bps
 *   - Two modes: compatibility (acts as 16550) and enhanced
 *
 * ESP register layout:
 *   Base+0x000..0x007: Standard 16550 registers (compatibility mode)
 *   Base+0x400..0x407: Enhanced mode registers
 *   Base+0x480..0x481: DMA channel registers
 *
 * ESP V1 (original) is NOT supported — only ESP V2+ (ComBic).
 * SIO2K config: DEVICE=ESP.SYS (COMn,base_enhanced,IRQn)
 *   Master port: must specify IRQ for baud > 180000
 *   Slave port: IRQ ignored (shared with master)
 *
 * Clean-room reimplementation from Hayes ESP Technical Reference.
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#include "driver.h"
#include "sio2k_idc.h"

/* ---- ESP Enhanced Register Offsets (from base+0x400) ---- */

#define ESP_STATUS1     0x400   /* Status register 1 (read) */
#define ESP_STATUS2     0x401   /* Status register 2 (read) */
#define ESP_CMD1        0x402   /* Command register 1 (write) */
#define ESP_CMD2        0x403   /* Command register 2 (write) */
#define ESP_SERVCHG     0x404   /* Service change mask */
#define ESP_RXTHRESH    0x405   /* RX FIFO trigger threshold */
#define ESP_TXTHRESH    0x406   /* TX FIFO trigger threshold */
#define ESP_ERRSTATUS   0x407   /* Error status register */
#define ESP_DMARX       0x480   /* DMA RX channel select */
#define ESP_DMATX       0x481   /* DMA TX channel select */

/* ---- ESP Status Register 1 Bits ---- */

#define ESP_ST1_RXRDY   0x01    /* RX data available */
#define ESP_ST1_TXRDY   0x02    /* TX FIFO has space */
#define ESP_ST1_RXFULL  0x04    /* RX FIFO full */
#define ESP_ST1_TXEMPTY 0x08    /* TX FIFO empty */
#define ESP_ST1_DCD     0x10    /* DCD asserted */
#define ESP_ST1_CTS     0x20    /* CTS asserted */
#define ESP_ST1_DSR     0x40    /* DSR asserted */
#define ESP_ST1_RI      0x80    /* Ring indicator */

/* ---- ESP Command Register 1 Commands ---- */

#define ESP_CMDRX_COUNT 0x01    /* Read RX FIFO byte count */
#define ESP_CMDTX_COUNT 0x02    /* Read TX FIFO byte count */
#define ESP_CMD_RESET   0x04    /* Reset ESP */
#define ESP_CMD_RXFLUSH 0x08    /* Flush RX FIFO */
#define ESP_CMD_TXFLUSH 0x10    /* Flush TX FIFO */
#define ESP_CMD_ENHENH  0x20    /* Enable enhanced mode */
#define ESP_CMD_DISENH  0x40    /* Disable enhanced mode (→ compatibility) */
#define ESP_CMD_SETFLOW 0x80    /* Set on-chip flow control */

/* ---- ESP Flow Control Bits (Command Register 2 when CMD_SETFLOW) ---- */

#define ESP_FLOW_RTSCTS 0x01    /* RTS/CTS hardware flow control */
#define ESP_FLOW_XONOFF 0x02    /* XON/XOFF software flow control */
#define ESP_FLOW_DTRCTS 0x04    /* DTR/CTS variant (rare) */

/* ---- Constants ---- */

#define ESP_FIFO_SIZE   1024    /* Hardware FIFO depth */
#define ESP_MAX_BAUD    921600  /* Maximum bit rate */
#define ESP_MAX_PORTS   8       /* Max ESP ports (4 cards × 2 ports) */
#define ESP_CRYSTAL     1843200 /* Standard crystal (1.8432 MHz) */

/* OS/2 device registration names (from original binary) */
#define ESP_DEV_NAME    "767ESP$    "   /* 8-char padded device name */
#define ESP_SIO_REF     "$SIO$   "      /* Reference to SIO2K parent */
#define ESP_BANNER      "Ray Gwinn's Hayes ESP Device Driver"

/* ---- Per-port state ---- */

typedef struct {
    int             active;
    unsigned short  ioBase;         /* Compatibility mode base (0x3F8 etc.) */
    unsigned short  ioEnhanced;     /* Enhanced mode base (0x180 etc.) */
    unsigned char   irq;            /* IRQ number */
    int             isMaster;       /* Master port (owns IRQ) */
    int             enhancedMode;   /* Currently in enhanced mode */
    unsigned short  portHandle;     /* SIO2K port handle */

    /* Cached register state */
    unsigned char   ier;            /* Interrupt enable */
    unsigned char   lcr;            /* Line control */
    unsigned char   mcr;            /* Modem control */
    unsigned char   fcr;            /* FIFO control */
    unsigned char   dll, dlh;       /* Divisor latch */
    unsigned short  rxThreshold;    /* RX trigger level */
    unsigned short  txThreshold;    /* TX trigger level */
    unsigned char   flowMode;       /* On-chip flow control */

    /* DMA state */
    int             dmaEnabled;
    unsigned char   dmaRxChan;
    unsigned char   dmaTxChan;
} ESP_PORT;

static ESP_PORT g_ports[ESP_MAX_PORTS];
static int      g_numPorts = 0;


/* ================================================================
 * LOW-LEVEL I/O (inp/outp wrappers)
 * ================================================================ */

/* OS/2 DevHelp or direct I/O — in real driver, these go through
 * DevHelp_PhysToVirt or are inlined with IN/OUT instructions.
 * For portability, use compiler intrinsics. */

#ifdef __WATCOMC__
#include <conio.h>
#define port_in(p)    inp(p)
#define port_out(p,v) outp(p,v)
#else
static unsigned char port_in(unsigned short p) { (void)p; return 0; }
static void port_out(unsigned short p, unsigned char v) { (void)p; (void)v; }
#endif


/* ================================================================
 * ESP CHIP ACCESS
 * ================================================================ */

/* Enter enhanced mode */
static void esp_enter_enhanced(ESP_PORT *p)
{
    if (p->enhancedMode) return;
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMD_ENHENH);
    p->enhancedMode = 1;
}

/* Exit to compatibility mode */
static void esp_exit_enhanced(ESP_PORT *p)
{
    if (!p->enhancedMode) return;
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMD_DISENH);
    p->enhancedMode = 0;
}

/* Get exact RX FIFO byte count (enhanced mode only) */
static unsigned short esp_rx_count(ESP_PORT *p)
{
    unsigned short count;
    esp_enter_enhanced(p);
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMDRX_COUNT);
    count = port_in(p->ioEnhanced + (ESP_STATUS1 - 0x400));
    count |= (unsigned short)port_in(p->ioEnhanced + (ESP_STATUS2 - 0x400)) << 8;
    return count;
}

/* Get exact TX FIFO free space (enhanced mode only) */
static unsigned short esp_tx_free(ESP_PORT *p)
{
    unsigned short count;
    esp_enter_enhanced(p);
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMDTX_COUNT);
    count = port_in(p->ioEnhanced + (ESP_STATUS1 - 0x400));
    count |= (unsigned short)port_in(p->ioEnhanced + (ESP_STATUS2 - 0x400)) << 8;
    return ESP_FIFO_SIZE - count;
}

/* Read block from RX FIFO (enhanced mode — direct block read) */
static unsigned short esp_rx_block(ESP_PORT *p, unsigned char *buf, unsigned short max)
{
    unsigned short avail, i;
    avail = esp_rx_count(p);
    if (avail > max) avail = max;
    for (i = 0; i < avail; i++)
        buf[i] = port_in(p->ioBase);   /* Read from RBR */
    return avail;
}

/* Write block to TX FIFO (enhanced mode — direct block write) */
static unsigned short esp_tx_block(ESP_PORT *p, const unsigned char *buf, unsigned short len)
{
    unsigned short space, i;
    space = esp_tx_free(p);
    if (len > space) len = space;
    for (i = 0; i < len; i++)
        port_out(p->ioBase, buf[i]);    /* Write to THR */
    return len;
}

/* Flush FIFOs */
static void esp_flush_rx(ESP_PORT *p)
{
    esp_enter_enhanced(p);
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMD_RXFLUSH);
}

static void esp_flush_tx(ESP_PORT *p)
{
    esp_enter_enhanced(p);
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMD_TXFLUSH);
}

/* Set on-chip flow control */
static void esp_set_flow(ESP_PORT *p, unsigned char mode)
{
    esp_enter_enhanced(p);
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMD_SETFLOW);
    port_out(p->ioEnhanced + (ESP_CMD2 - 0x400), mode);
    p->flowMode = mode;
}

/* Set FIFO trigger thresholds */
static void esp_set_thresholds(ESP_PORT *p, unsigned short rx, unsigned short tx)
{
    esp_enter_enhanced(p);
    port_out(p->ioEnhanced + (ESP_RXTHRESH - 0x400), rx & 0xFF);
    port_out(p->ioEnhanced + (ESP_TXTHRESH - 0x400), tx & 0xFF);
    p->rxThreshold = rx;
    p->txThreshold = tx;
}

/* Reset ESP chip */
static void esp_reset(ESP_PORT *p)
{
    port_out(p->ioEnhanced + (ESP_CMD1 - 0x400), ESP_CMD_RESET);
    p->enhancedMode = 0;
    p->flowMode = 0;
    p->rxThreshold = 768;   /* Default: trigger at 75% full */
    p->txThreshold = 256;   /* Default: trigger at 25% full */
}

/* Detect ESP chip — write scratch register, check enhanced port */
static int esp_detect(unsigned short ioBase, unsigned short ioEnhanced)
{
    unsigned char save, test;

    /* Check if standard UART responds */
    save = port_in(ioBase + 7);     /* Save scratch register */
    port_out(ioBase + 7, 0xA5);
    test = port_in(ioBase + 7);
    port_out(ioBase + 7, save);     /* Restore */
    if (test != 0xA5) return 0;

    /* Check if enhanced port responds with ESP signature */
    test = port_in(ioEnhanced + (ESP_STATUS1 - 0x400));
    /* ESP status register always has a valid pattern */
    /* A real ESP will respond; an empty port returns 0xFF */
    if (test == 0xFF) return 0;

    return 1;   /* ESP detected */
}


/* ================================================================
 * IDC ENTRY — called by SIO2K.SYS
 * ================================================================ */

static unsigned short esp_idc(unsigned short cmd, unsigned short portIndex,
                               void *pData, unsigned short cbData)
{
    ESP_PORT *p;
    if (portIndex >= g_numPorts) return 0xFFFF;
    p = &g_ports[portIndex];

    switch (cmd) {

    case PHYS_CMD_INIT:
        esp_reset(p);
        esp_enter_enhanced(p);
        esp_set_thresholds(p, 768, 256);
        return 0;

    case PHYS_CMD_OPEN:
        p->active = 1;
        /* Raise DTR + RTS */
        p->mcr = 0x0B;     /* DTR + RTS + OUT2 (enables IRQ) */
        port_out(p->ioBase + 4, p->mcr);
        /* Enable receive interrupt */
        p->ier = 0x01;
        port_out(p->ioBase + 1, p->ier);
        return 0;

    case PHYS_CMD_CLOSE:
        /* Drop DTR, disable interrupts */
        p->mcr = 0;
        port_out(p->ioBase + 4, p->mcr);
        p->ier = 0;
        port_out(p->ioBase + 1, p->ier);
        p->active = 0;
        return 0;

    case PHYS_CMD_SETBAUD:
        if (pData && cbData >= sizeof(unsigned long)) {
            unsigned long baud = *(unsigned long *)pData;
            unsigned short div;
            if (baud == 0) baud = 9600;
            div = (unsigned short)(ESP_CRYSTAL / (baud * 16));
            if (div == 0) div = 1;
            /* Set DLAB, write divisor, clear DLAB */
            port_out(p->ioBase + 3, p->lcr | 0x80);
            port_out(p->ioBase + 0, div & 0xFF);
            port_out(p->ioBase + 1, (div >> 8) & 0xFF);
            port_out(p->ioBase + 3, p->lcr);
            p->dll = div & 0xFF;
            p->dlh = (div >> 8) & 0xFF;
        }
        return 0;

    case PHYS_CMD_SETLINE:
        if (pData && cbData >= 1) {
            p->lcr = *(unsigned char *)pData;
            port_out(p->ioBase + 3, p->lcr);
        }
        return 0;

    case PHYS_CMD_SETMCR:
        if (pData && cbData >= 1) {
            p->mcr = *(unsigned char *)pData | 0x08;  /* Keep OUT2 set */
            port_out(p->ioBase + 4, p->mcr);
        }
        return 0;

    case PHYS_CMD_GETMSR:
        if (pData && cbData >= 1)
            *(unsigned char *)pData = port_in(p->ioBase + 6);
        return 0;

    case PHYS_CMD_GETLSR:
        if (pData && cbData >= 1)
            *(unsigned char *)pData = port_in(p->ioBase + 5);
        return 0;

    case PHYS_CMD_TXBYTE:
        if (pData && cbData >= 1) {
            /* Wait for THR empty, then write */
            int retries = 0;
            while (!(port_in(p->ioBase + 5) & 0x20) && retries < 1000) retries++;
            port_out(p->ioBase, *(unsigned char *)pData);
        }
        return 0;

    case PHYS_CMD_TXBLOCK:
        return esp_tx_block(p, (const unsigned char *)pData, cbData);

    case PHYS_CMD_RXBLOCK:
        return esp_rx_block(p, (unsigned char *)pData, cbData);

    case PHYS_CMD_RXCOUNT:
        if (pData && cbData >= sizeof(unsigned short))
            *(unsigned short *)pData = esp_rx_count(p);
        return 0;

    case PHYS_CMD_TXCOUNT:
        if (pData && cbData >= sizeof(unsigned short))
            *(unsigned short *)pData = esp_tx_free(p);
        return 0;

    case PHYS_CMD_TXFLUSH:
        esp_flush_tx(p);
        return 0;

    case PHYS_CMD_RXFLUSH:
        esp_flush_rx(p);
        return 0;

    case PHYS_CMD_BREAKON:
        p->lcr |= 0x40;
        port_out(p->ioBase + 3, p->lcr);
        return 0;

    case PHYS_CMD_BREAKOFF:
        p->lcr &= ~0x40;
        port_out(p->ioBase + 3, p->lcr);
        return 0;

    case PHYS_CMD_SETFLOW:
        if (pData && cbData >= sizeof(unsigned short)) {
            unsigned short flow = *(unsigned short *)pData;
            unsigned char espFlow = 0;
            if (flow & FLOW_CTS)      espFlow |= ESP_FLOW_RTSCTS;
            if (flow & FLOW_XONXOFF_TX) espFlow |= ESP_FLOW_XONOFF;
            esp_set_flow(p, espFlow);
        }
        return 0;

    case PHYS_CMD_SETFIFO:
        if (pData && cbData >= sizeof(unsigned short))
            esp_set_thresholds(p, *(unsigned short *)pData, p->txThreshold);
        return 0;

    case PHYS_CMD_GETINFO:
        if (pData && cbData >= sizeof(PHYS_PORT_INFO)) {
            PHYS_PORT_INFO *pi = (PHYS_PORT_INFO *)pData;
            pi->ioBase = p->ioBase;
            pi->irq = p->irq;
            pi->uartType = PHYS_UART_ESP;
            pi->fifoSize = ESP_FIFO_SIZE;
            pi->crystalFreq = ESP_CRYSTAL;
            pi->maxBaud = ESP_MAX_BAUD;
            pi->flags = PHYS_FLAG_EXISTS | PHYS_FLAG_FIFO | PHYS_FLAG_ENHANCED;
        }
        return 0;

    case PHYS_CMD_AUTOIRQ:
    case PHYS_CMD_AUTOFIFO:
    case PHYS_CMD_AUTOCRYSTAL:
        /* ESP has fixed values — auto-detect not needed */
        return 0;

    default:
        return 0;
    }
}


/* ================================================================
 * ISR — Interrupt Service Routine
 * ================================================================
 * Called when the ESP fires an IRQ. Reads IIR to determine cause,
 * drains RX FIFO into SIO2K's buffer via IDC callback.
 * ================================================================ */

static void esp_isr(ESP_PORT *p, PFN_SIO2K_CB pfnCallback)
{
    unsigned char iir;
    int maxLoops = 32;

    while (maxLoops-- > 0) {
        iir = port_in(p->ioBase + 2);
        if (iir & 0x01) break;     /* No interrupt pending */

        switch (iir & 0x0E) {
        case 0x04:  /* RX data available */
        case 0x0C:  /* RX timeout (character timeout) */
        {
            /* Drain RX FIFO using enhanced mode block read */
            unsigned char buf[256];
            unsigned short n = esp_rx_block(p, buf, sizeof(buf));
            if (n > 0 && pfnCallback)
                pfnCallback(SIOEVT_RXDATA, p->portHandle, buf, n);
            break;
        }
        case 0x02:  /* TX holding register empty */
            if (pfnCallback)
                pfnCallback(SIOEVT_TXEMPTY, p->portHandle, 0, 0);
            break;
        case 0x06:  /* Line status change (error/break) */
        {
            unsigned char lsr = port_in(p->ioBase + 5);
            if (pfnCallback)
                pfnCallback(SIOEVT_LSR, p->portHandle, &lsr, 1);
            break;
        }
        case 0x00:  /* Modem status change */
        {
            unsigned char msr = port_in(p->ioBase + 6);
            if (pfnCallback)
                pfnCallback(SIOEVT_MSR, p->portHandle, &msr, 1);
            break;
        }
        }
    }
}
