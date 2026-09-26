/* ============================================================================
 * commdrbl.c — COMM-DRV Borland Library replacement
 *
 * Clean-room implementation of the 13 ser_rs232_* functions that PCBoard's
 * MODEMDRV.C links against. Compiles to commdrbl.lib (Borland large model)
 * as a drop-in replacement for the proprietary WCSC COMM-DRV/DOS SDK library.
 *
 * This is the link-time library. It talks to the pcbdcom TSR (or any
 * COMM-DRV-compatible TSR) through shared memory — the TSR allocates the
 * opcb and auxpcb blocks in its resident segment, and this library accesses
 * them via far pointers set during ser_rs232_setup().
 *
 * For the FOSSIL-only path (option a), this library is NOT needed — PCBoard
 * uses MODEMFOS.C which calls INT 14h directly. This library enables option
 * (c): the full COMM-DRV replacement path via MODEMDRV.C.
 *
 * Build:
 *   bcc -ml -c -DCOMMDRV_DRIVER commdrbl.c
 *   tlib commdrbl.lib +commdrbl.obj
 *
 * pcbirc crew (hexadecimal + sysop/0), GPLv3.
 * ==========================================================================*/

#include "comm.h"
#include <dos.h>
#include <string.h>

/* ---- TSR detection ----------------------------------------------------- */
/* The pcbdcom TSR (or COMM-DRV) installs on INT 14h. We detect it by
 * calling function 04h (FOSSIL init) and checking for the 0x1954 signature.
 * If present, the TSR also stores a far pointer to its port table at a
 * known location (the COMM-DRV "info block" returned by function 1Bh). */

#define MAX_PORTS  16

/* Per-port state — mirrors the TSR's resident data */
static struct {
    int                configured;
    struct opcb_block  opcb;        /* local copy, refreshed on each getpacket(n=0) */
    struct aux_pcb     auxpcb;      /* local copy, refreshed on read */
    unsigned int       rx_buf_total; /* total RX buffer size from FOSSIL info */
    unsigned int       tx_buf_total; /* total TX buffer size from FOSSIL info */
} ports[MAX_PORTS];

static int driver_initialized = 0;

/* ---- INT 14h helpers --------------------------------------------------- */

static unsigned int call_int14(unsigned char ah, unsigned char al, int port)
{
    union REGS r;
    r.h.ah = ah;
    r.h.al = al;
    r.x.dx = port;
    int86(0x14, &r, &r);
    return r.x.ax;
}

static unsigned int call_int14_bx(unsigned char ah, unsigned char al, int port,
                                   unsigned int *bx_out)
{
    union REGS r;
    r.h.ah = ah;
    r.h.al = al;
    r.x.dx = port;
    int86(0x14, &r, &r);
    if (bx_out) *bx_out = r.x.bx;
    return r.x.ax;
}

/* Block read via INT 14h function 18h — reads n bytes into buf via ES:DI */
static int block_read(int port, int n, unsigned char *buf)
{
    union REGS r;
    struct SREGS sr;

    if (n <= 0 || !buf) return 0;

    r.h.ah = 0x18;
    r.x.cx = n;
    r.x.dx = port;
    sr.es  = FP_SEG((void far *)buf);
    r.x.di = FP_OFF((void far *)buf);
    int86x(0x14, &r, &r, &sr);
    return r.x.ax;  /* bytes actually read */
}

/* Block write via INT 14h function 19h */
static int block_write(int port, int n, unsigned char *buf)
{
    union REGS r;
    struct SREGS sr;

    if (n <= 0 || !buf) return 0;

    r.h.ah = 0x19;
    r.x.cx = n;
    r.x.dx = port;
    sr.es  = FP_SEG((void far *)buf);
    r.x.di = FP_OFF((void far *)buf);
    int86x(0x14, &r, &r, &sr);
    return r.x.ax;  /* bytes actually written */
}

/* Get driver info via INT 14h function 1Bh — fills local opcb */
static void refresh_info(int port)
{
    union REGS r;
    struct SREGS sr;
    unsigned char info[19];

    if (port < 0 || port >= MAX_PORTS) return;

    r.h.ah = 0x1B;
    r.x.cx = sizeof(info);
    r.x.dx = port;
    sr.es  = FP_SEG((void far *)info);
    r.x.di = FP_OFF((void far *)info);
    int86x(0x14, &r, &r, &sr);

    /* Parse the 19-byte FOSSIL info block into our opcb */
    {
        unsigned int rx_total = info[8]  | ((unsigned int)info[9]  << 8);
        unsigned int rx_pend  = info[10] | ((unsigned int)info[11] << 8);
        unsigned int tx_total = info[12] | ((unsigned int)info[13] << 8);
        unsigned int tx_pend  = info[14] | ((unsigned int)info[15] << 8);
        ports[port].opcb.inbuf_count  = rx_pend;
        ports[port].opcb.outbuf_count = tx_pend;
        ports[port].rx_buf_total = rx_total;
        ports[port].tx_buf_total = tx_total;
    }

    /* MSR from status word */
    {
        unsigned int st = call_int14(0x03, 0, port);
        ports[port].opcb.msr_reg = (unsigned char)(st & 0xFF);
    }
}

/* ---- Public API (13 functions) ----------------------------------------- */

int LIBENTRY ser_rs232_init(void)
{
    unsigned int ax;
    unsigned int bx;

    memset(ports, 0, sizeof(ports));

    /* Probe for FOSSIL/pcbdcom on port 0 */
    ax = call_int14_bx(0x04, 0, 0, &bx);
    if (ax != 0x1954)
        return RS232ERR_INIT;  /* no FOSSIL driver loaded */

    driver_initialized = 1;
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_setup(int port, struct port_param *pp)
{
    unsigned char baud_byte;

    if (!driver_initialized) return RS232ERR_INIT;
    if (port < 0 || port >= MAX_PORTS) return RS232ERR_PARAM;
    if (!pp) return RS232ERR_PARAM;

    /* Initialize the port via FOSSIL init (fn 04h) */
    {
        unsigned int ax = call_int14(0x04, 0, port);
        if (ax != 0x1954)
            return RS232ERR_NOPORT;
    }

    /* Set baud rate via function 00h */
    /* Build the FOSSIL baud byte from pp->baud and pp->lngth/parity */
    baud_byte = 0;
    /* Baud bits 7-5 — pp->baud is a divisor index, pass through */
    baud_byte = (unsigned char)((pp->baud & 0x07) << 5);
    /* Parity bits 4-3 */
    switch (pp->parity) {
        case PARITY_NONE: baud_byte |= 0x00; break;
        case PARITY_ODD:  baud_byte |= 0x08; break;
        case PARITY_EVEN: baud_byte |= 0x18; break;
    }
    /* Data bits 1-0: LENGTH_7 = 0x02, LENGTH_8 = 0x03 */
    baud_byte |= (pp->lngth - 5) & 0x03;
    call_int14(0x00, baud_byte, port);

    /* Set flow control via function 0Fh */
    switch (pp->protocol) {
        case PROT_RTSRTS:  call_int14(0x0F, 0x02, port); break;
        case PROT_XONXOFF: call_int14(0x0F, 0x09, port); break;
        default:           call_int14(0x0F, 0x00, port); break;
    }

    /* Point caller's opcb/auxpcb at our per-port blocks */
    pp->opcb   = &ports[port].opcb;
    pp->auxpcb = &ports[port].auxpcb;

    /* Save card type */
    ports[port].opcb.cardtype = pp->cardtype;
    ports[port].configured = 1;

    /* Initial refresh */
    refresh_info(port);
    pp->outbuf_len = ports[port].tx_buf_total ? ports[port].tx_buf_total : 4096;

    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_getport(int port, struct port_param *pp)
{
    if (port < 0 || port >= MAX_PORTS) return RS232ERR_PARAM;
    if (!pp) return RS232ERR_PARAM;

    pp->opcb   = &ports[port].opcb;
    pp->auxpcb = &ports[port].auxpcb;
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_getbyte(int port, unsigned char *b)
{
    unsigned int ax;

    if (!b) return RS232ERR_PARAM;

    /* FOSSIL function 02h — RX char with wait */
    ax = call_int14(0x02, 0, port);
    *b = (unsigned char)(ax & 0xFF);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_putbyte(int port, unsigned char *b)
{
    if (!b) return RS232ERR_PARAM;

    /* FOSSIL function 01h — TX char with wait */
    call_int14(0x01, *b, port);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_getpacket(int port, int n, unsigned char *buf)
{
    if (port < 0 || port >= MAX_PORTS) return RS232ERR_PARAM;

    /* n == 0: refresh state (MODEMDRV convention) */
    if (n == 0) {
        refresh_info(port);
        return RS232ERR_NONE;
    }

    /* n == 32767 with NULL buf: flush RX buffer */
    if (n == 32767 && buf == NULL) {
        call_int14(0x0A, 0, port);  /* purge input */
        refresh_info(port);
        return RS232ERR_NONE;
    }

    if (!buf) return RS232ERR_PARAM;

    /* FOSSIL function 18h — block read */
    block_read(port, n, buf);
    refresh_info(port);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_putpacket(int port, int n, unsigned char *buf)
{
    if (port < 0 || port >= MAX_PORTS) return RS232ERR_PARAM;

    /* NULL buf: flush TX (kick transmit) */
    if (buf == NULL) {
        /* COMM-DRV commgo: AX=1000h */
        call_int14(0x10, 0x00, port);
        return RS232ERR_NONE;
    }

    if (n <= 0) return RS232ERR_PARAM;

    /* FOSSIL function 19h — block write */
    block_write(port, n, buf);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_viewpacket(int port, int n, unsigned char *buf)
{
    /* Peek RX buffer without consuming. Use FOSSIL function 0Ch (peek)
     * for the first byte, and refresh opcb for the count. If buf is
     * provided and n > 0, fill the first byte from the peek. */
    union REGS r;

    if (port < 0 || port >= MAX_PORTS) return RS232ERR_PARAM;
    refresh_info(port);

    if (buf != NULL && n > 0) {
        /* FOSSIL function 0Ch — non-destructive read */
        r.h.ah = 0x0C;
        r.x.dx = port;
        int86(0x14, &r, &r);
        if (r.x.ax != 0xFFFF)
            buf[0] = (unsigned char)(r.x.ax & 0xFF);
    }
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_flush(int port, int which)
{
    if (port < 0 || port >= MAX_PORTS) return RS232ERR_PARAM;

    switch (which) {
        case 0: call_int14(0x0A, 0, port); break;  /* purge input */
        case 1: call_int14(0x09, 0, port); break;  /* purge output */
        case 2:
            call_int14(0x0A, 0, port);              /* purge input */
            call_int14(0x09, 0, port);              /* purge output */
            break;
    }
    refresh_info(port);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_dtr_on(int port)
{
    call_int14(0x06, 0x01, port);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_dtr_off(int port)
{
    call_int14(0x06, 0x00, port);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_rts_on(int port)
{
    /* No standard FOSSIL function for RTS — but the COMM-DRV extension
     * at AH=10h might carry it. For now, use flow control function 0Fh
     * with CTS/RTS enabled. */
    call_int14(0x0F, 0x02, port);
    return RS232ERR_NONE;
}

int LIBENTRY ser_rs232_rts_off(int port)
{
    call_int14(0x0F, 0x00, port);
    return RS232ERR_NONE;
}
