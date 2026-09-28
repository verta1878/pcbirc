/* ====================================================================
 * vx00.c — VX00.SYS FOSSIL Driver for OS/2 DOS Sessions
 * ====================================================================
 * DOS TSR (MZ EXE) that hooks INT 14h and provides FOSSIL Level 5
 * (FTS-0017) access. Communicates with VSIO2K.SYS in the OS/2
 * kernel via the VSIO IDC protocol.
 *
 * Original: 3,711 bytes MZ EXE by Ray Gwinn, v1.60f.
 * Built from: dispatch.obj + int14api.obj + vx00init.obj +
 *             ticker.obj + segend.obj
 *
 * Architecture:
 *   DOS app → INT 14h → VX00 (resident) → VSIO2K.SYS → SIO2K.SYS
 *
 * VX00 has NO buffers — it uses SIO2K's buffers via VSIO2K.
 * All data passes through the VDD layer transparently.
 *
 * Load: DEVICE=path\VX00.SYS in CONFIG.SYS or DOS_DEVICE=
 * Remove: VX00 /R from command line
 *
 * Clean-room reimplementation. GPLv3.
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#ifdef __WATCOMC__
#include <conio.h>
#include <i86.h>
#define getvect     _dos_getvect
#define setvect     _dos_setvect
#define MK_FP(s,o) ((void far *)((unsigned long)(s) << 16 | (o)))
#define FP_SEG(p)  ((unsigned short)((unsigned long)(p) >> 16))
#define FP_OFF(p)  ((unsigned short)((unsigned long)(p)))
#define keep        _dos_keep
#endif

#define VERSION_STR     "2.00"
#define VERSION_NUM     0x0200
#define FOSSIL_SIG      0x1954
#define FOSSIL_REV      5
#define FOSSIL_MAXFN    0x1B
#define MAX_PORTS       8
#define ID_STRING       "VX00/SIO2K v2.0 FOSSIL (GPLv3)\r\n"
#define MAGIC_WORD      0x5630      /* 'V0' — version check with VSIO2K */

/* ---- Embedded device driver header ----
 * This structure is placed in the resident data segment.
 * OS/2's DOS session loader finds it and recognizes VX00
 * as a FOSSIL provider. The device name "VX00$   " is
 * what VSIO2K.SYS searches for to establish the link. */

static const char g_devName[8] = "VX00$   ";

/* ---- Port state ----
 * VX00 has NO buffers. All I/O goes through VSIO2K via
 * INT 2Fh AX=FD00h multiplex interface. VSIO2K passes
 * the calls to SIO2K.SYS via IDC. */

typedef struct {
    int     active;                 /* FOSSIL initialized */
    int     dtr;                    /* DTR state */
    int     rts;                    /* RTS state */
    int     flow;                   /* Flow control flags */
} VX00_PORT;

static VX00_PORT g_ports[MAX_PORTS];
static int g_numPorts = 4;
static int g_installed = 0;

/* Old interrupt vectors (saved for unhook) */
static void (interrupt far *g_oldInt14)() = NULL;
static void (interrupt far *g_oldInt2F)() = NULL;
static void (interrupt far *g_oldTick)() = NULL;

/* ================================================================
 * VSIO2K COMMUNICATION (INT 2Fh multiplex)
 * ================================================================
 * VX00 talks to VSIO2K.SYS using INT 2Fh:
 *   AX = FD00h     Check if VSIO2K present
 *   AX = FD01h     Send byte (DL = port, DH = byte)
 *   AX = FD02h     Receive byte (DL = port, returns AL)
 *   AX = FD03h     Get status (DL = port, returns AX)
 *   AX = FD04h     Set baud (DL = port, BX = divisor)
 *   AX = FD05h     Set MCR (DL = port, DH = mcr)
 *   AX = FD06h     Set LCR (DL = port, DH = lcr)
 *   AX = FD07h     Flush TX (DL = port)
 *   AX = FD08h     Purge RX (DL = port)
 *   AX = FD09h     Purge TX (DL = port)
 *   AX = FD0Ah     Get RX count (DL = port, returns CX)
 *   AX = FD0Bh     Get TX free (DL = port, returns CX)
 *   AX = FD0Ch     Block read (DL=port, ES:DI=buf, CX=max, returns CX)
 *   AX = FD0Dh     Block write (DL=port, ES:DI=buf, CX=len, returns CX)
 *   AX = FD0Eh     Set flow (DL = port, DH = flags)
 *   AX = FD0Fh     Break on (DL = port)
 *   AX = FD10h     Break off (DL = port)
 *   AX = FD11h     Get MSR (DL = port, returns AL)
 *   AX = FD12h     Get LSR (DL = port, returns AL)
 *   AX = FDFFh     Get port count (returns CX)
 * ================================================================ */

static int vsio_present(void)
{
    union REGS r;
    r.x.ax = 0xFD00;
    int86(0x2F, &r, &r);
    return (r.x.ax == MAGIC_WORD) ? 1 : 0;
}

static void vsio_tx(int port, unsigned char ch)
{
    union REGS r;
    r.x.ax = 0xFD01;
    r.h.dl = (unsigned char)port;
    r.h.dh = ch;
    int86(0x2F, &r, &r);
}

static int vsio_rx(int port)
{
    union REGS r;
    r.x.ax = 0xFD02;
    r.h.dl = (unsigned char)port;
    int86(0x2F, &r, &r);
    return (r.x.ax == 0xFFFF) ? -1 : (r.h.al & 0xFF);
}

static unsigned short vsio_status(int port)
{
    union REGS r;
    r.x.ax = 0xFD03;
    r.h.dl = (unsigned char)port;
    int86(0x2F, &r, &r);
    return r.x.ax;
}

static void vsio_set_baud(int port, unsigned short divisor)
{
    union REGS r;
    r.x.ax = 0xFD04;
    r.h.dl = (unsigned char)port;
    r.x.bx = divisor;
    int86(0x2F, &r, &r);
}

static void vsio_set_mcr(int port, unsigned char mcr)
{
    union REGS r;
    r.x.ax = 0xFD05;
    r.h.dl = (unsigned char)port;
    r.h.dh = mcr;
    int86(0x2F, &r, &r);
}

static void vsio_set_lcr(int port, unsigned char lcr)
{
    union REGS r;
    r.x.ax = 0xFD06;
    r.h.dl = (unsigned char)port;
    r.h.dh = lcr;
    int86(0x2F, &r, &r);
}

static void vsio_flush_tx(int port) { union REGS r; r.x.ax = 0xFD07; r.h.dl = (unsigned char)port; int86(0x2F, &r, &r); }
static void vsio_purge_rx(int port) { union REGS r; r.x.ax = 0xFD08; r.h.dl = (unsigned char)port; int86(0x2F, &r, &r); }
static void vsio_purge_tx(int port) { union REGS r; r.x.ax = 0xFD09; r.h.dl = (unsigned char)port; int86(0x2F, &r, &r); }

static unsigned short vsio_rx_count(int port)
{
    union REGS r;
    r.x.ax = 0xFD0A;
    r.h.dl = (unsigned char)port;
    int86(0x2F, &r, &r);
    return r.x.cx;
}

static unsigned short vsio_tx_free(int port)
{
    union REGS r;
    r.x.ax = 0xFD0B;
    r.h.dl = (unsigned char)port;
    int86(0x2F, &r, &r);
    return r.x.cx;
}

static unsigned char vsio_get_msr(int port)
{
    union REGS r;
    r.x.ax = 0xFD11;
    r.h.dl = (unsigned char)port;
    int86(0x2F, &r, &r);
    return r.h.al;
}

static unsigned char vsio_get_lsr(int port)
{
    union REGS r;
    r.x.ax = 0xFD12;
    r.h.dl = (unsigned char)port;
    int86(0x2F, &r, &r);
    return r.h.al;
}

static void vsio_set_flow(int port, unsigned char flags)
{
    union REGS r;
    r.x.ax = 0xFD0E;
    r.h.dl = (unsigned char)port;
    r.h.dh = flags;
    int86(0x2F, &r, &r);
}

static void vsio_break_on(int port)  { union REGS r; r.x.ax = 0xFD0F; r.h.dl = (unsigned char)port; int86(0x2F, &r, &r); }
static void vsio_break_off(int port) { union REGS r; r.x.ax = 0xFD10; r.h.dl = (unsigned char)port; int86(0x2F, &r, &r); }


/* ================================================================
 * BAUD RATE DECODE (same table as all FOSSIL drivers)
 * ================================================================ */

static const unsigned short baud_divisors[] = {
    /* 300=384, 600=192, 1200=96, 2400=48, 4800=24, 9600=12, 19200=6, 38400=3 */
    6, 3, 384, 192, 96, 48, 24, 12
};


/* ================================================================
 * INT 14h HANDLER
 * ================================================================ */

static void interrupt far int14_handler(
    unsigned es, unsigned ds, unsigned di, unsigned si,
    unsigned bp, unsigned sp, unsigned bx, unsigned dx,
    unsigned cx, unsigned ax, unsigned ip, unsigned cs, unsigned flags)
{
    unsigned char ah = (unsigned char)(ax >> 8);
    unsigned char al = (unsigned char)(ax & 0xFF);
    int port = dx & 0xFF;
    VX00_PORT *p;

    if (port >= g_numPorts) {
        /* Not our port — chain to old handler */
        _chain_intr(g_oldInt14);
        return;
    }

    p = &g_ports[port];

    switch (ah) {
    case 0x00:  /* Set baud/line params */
    {
        unsigned short div = baud_divisors[(al >> 5) & 7];
        unsigned char lcr = al & 0x1F;
        vsio_set_baud(port, div);
        vsio_set_lcr(port, lcr);
        ax = vsio_status(port);
        break;
    }
    case 0x01:  /* Write char wait */
    {
        int retries = 0;
        while (vsio_tx_free(port) == 0 && retries < 5000) retries++;
        vsio_tx(port, al);
        ax = vsio_status(port);
        break;
    }
    case 0x02:  /* Read char wait */
    {
        int ch, retries = 0;
        while (vsio_rx_count(port) == 0 && retries < 5000) retries++;
        ch = vsio_rx(port);
        if (ch >= 0) ax = (vsio_status(port) & 0xFF00) | (ch & 0xFF);
        else ax = vsio_status(port) | 0x8000;
        break;
    }
    case 0x03:  /* Status */
        ax = vsio_status(port);
        break;

    case 0x04:  /* Init FOSSIL */
        p->active = 1;
        p->dtr = p->rts = 1;
        vsio_set_mcr(port, 0x03);  /* DTR + RTS */
        vsio_purge_rx(port);
        vsio_purge_tx(port);
        ax = FOSSIL_SIG;
        bx = (FOSSIL_REV << 8) | FOSSIL_MAXFN;
        break;

    case 0x05:  /* Deinit */
        vsio_set_mcr(port, 0x00);
        p->active = 0;
        break;

    case 0x06:  /* DTR */
        p->dtr = al ? 1 : 0;
        vsio_set_mcr(port, (p->dtr ? 0x01 : 0) | (p->rts ? 0x02 : 0));
        break;

    case 0x07:  /* Timer tick — yield */
        break;

    case 0x08:  /* Flush TX */
        vsio_flush_tx(port);
        break;

    case 0x09:  /* Purge TX */
        vsio_purge_tx(port);
        break;

    case 0x0A:  /* Purge RX */
        vsio_purge_rx(port);
        break;

    case 0x0B:  /* Write no wait */
        if (vsio_tx_free(port) > 0) { vsio_tx(port, al); ax = 1; }
        else ax = 0;
        break;

    case 0x0C:  /* Peek RX */
        if (vsio_rx_count(port) > 0) {
            int ch = vsio_rx(port);
            ax = (vsio_status(port) & 0xFF00) | (ch & 0xFF);
        } else ax = 0xFFFF;
        break;

    case 0x0D:  /* Keyboard peek — BIOS INT 16h AH=01h */
    {
        union REGS kr;
        kr.h.ah = 0x01;
        int86(0x16, &kr, &kr);
        ax = (kr.x.flags & 0x40) ? 0xFFFF : kr.x.ax; /* ZF = no key */
        break;
    }

    case 0x0E:  /* Keyboard read — BIOS INT 16h AH=00h */
    {
        union REGS kr;
        kr.h.ah = 0x00;
        int86(0x16, &kr, &kr);
        ax = kr.x.ax;
        break;
    }

    case 0x0F:  /* Flow control */
        p->flow = al;
        vsio_set_flow(port, al);
        break;

    case 0x10:  /* Ctrl-C/K check */
        ax = 0;
        break;

    case 0x18:  /* Block read */
    {
        unsigned char far *buf = MK_FP(es, di);
        unsigned short count = 0, max = cx;
        while (count < max && vsio_rx_count(port) > 0) {
            int ch = vsio_rx(port);
            if (ch >= 0) buf[count++] = (unsigned char)ch;
            else break;
        }
        ax = count;
        break;
    }

    case 0x19:  /* Block write */
    {
        unsigned char far *buf = MK_FP(es, di);
        unsigned short count = 0, max = cx;
        while (count < max && vsio_tx_free(port) > 0) {
            vsio_tx(port, buf[count++]);
        }
        ax = count;
        break;
    }

    case 0x1A:  /* Break */
        if (al) vsio_break_on(port);
        else vsio_break_off(port);
        break;

    case 0x1B:  /* Get info */
    {
        unsigned char far *buf = MK_FP(es, di);
        unsigned short max = cx, rxfree, txfree;
        if (max > 19) max = 19;
        rxfree = 4096 - vsio_rx_count(port);
        txfree = vsio_tx_free(port);
        buf[0] = 19; buf[1] = 0;
        buf[2] = FOSSIL_REV;
        buf[3] = (VERSION_NUM >> 8) & 0xFF;
        buf[4] = buf[5] = buf[6] = buf[7] = 0;
        buf[8] = 0x00; buf[9] = 0x10;
        buf[10] = rxfree & 0xFF; buf[11] = (rxfree >> 8) & 0xFF;
        buf[12] = 0x00; buf[13] = 0x10;
        buf[14] = txfree & 0xFF; buf[15] = (txfree >> 8) & 0xFF;
        buf[16] = 80; buf[17] = 25; buf[18] = 0xFF;
        ax = 19;
        break;
    }

    default:
        /* Chain to old handler for unknown functions */
        _chain_intr(g_oldInt14);
        return;
    }
}


/* ================================================================
 * INT 2Fh HANDLER (FOSSIL DETECT + VX00 CONTROL)
 * ================================================================ */

static void interrupt far int2f_handler(
    unsigned es, unsigned ds, unsigned di, unsigned si,
    unsigned bp, unsigned sp, unsigned bx, unsigned dx,
    unsigned cx, unsigned ax, unsigned ip, unsigned cs, unsigned flags)
{
    unsigned short func = ax;

    if (func == 0x1600) {
        /* FOSSIL detect: return AX=00FFh = FOSSIL installed */
        ax = 0x00FF;
        return;
    }

    /* Chain to previous handler */
    _chain_intr(g_oldInt2F);
}


/* ================================================================
 * INSTALL / REMOVE
 * ================================================================ */

static void beep(void)
{
    /* Short beep on install (matches original) */
    outp(0x43, 0xB6);
    outp(0x42, 0x33);   /* ~1000 Hz */
    outp(0x42, 0x05);
    outp(0x61, inp(0x61) | 0x03);
    { unsigned long i; for (i = 0; i < 50000UL; i++) ; }
    outp(0x61, inp(0x61) & ~0x03);
}

int main(int argc, char *argv[])
{
    int i;

    printf("Ray Gwinn's double aught buckshot driver, VX00  %s \n", VERSION_STR);
    printf("Gwinn's Communications Controller, VX00.SYS Version  %s, ", VERSION_STR);

    /* Check for /R = remove */
    for (i = 1; i < argc; i++) {
        if (stricmp(argv[i], "/R") == 0 || stricmp(argv[i], "-R") == 0) {
            /* Unhook and exit */
            if (g_oldInt14) setvect(0x14, g_oldInt14);
            if (g_oldInt2F) setvect(0x2F, g_oldInt2F);
            printf("removed.\n");
            return 0;
        }
    }

    /* Check VSIO2K is present */
    if (!vsio_present()) {
        printf("\nVX00/VSIO version mismatch or VSIO is not installed.\n");
        printf("Hit any key to continue.$");
        getch();
        return 1;
    }

    /* Save old vectors */
    g_oldInt14 = getvect(0x14);
    g_oldInt2F = getvect(0x2F);

    /* Initialize ports */
    for (i = 0; i < MAX_PORTS; i++) {
        g_ports[i].active = 0;
        g_ports[i].dtr = 0;
        g_ports[i].rts = 0;
        g_ports[i].flow = 0;
    }

    /* Hook INT 14h and INT 2Fh */
    setvect(0x14, int14_handler);
    setvect(0x2F, int2f_handler);
    g_installed = 1;

    beep();
    printf("installed.\n");

    /* Go resident — keep code + data, free environment */
    keep(0, (_CS + (_DS - _CS) + 0x100) >> 4);

    return 0;  /* Never reached */
}
