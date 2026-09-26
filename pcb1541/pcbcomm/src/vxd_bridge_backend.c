/* ============================================================================
 * vxd_bridge_backend.c — pcbcomm VxD bridge backend (COMMDV08)
 *
 * Provides serial I/O under Windows 9x DOS boxes by calling into a
 * Win9x VxD (Virtual Device Driver) that owns the physical port.
 *
 * When PCBoard runs in a Win9x DOS box, it can't touch UART hardware
 * directly — Windows virtualizes I/O ports.  This backend bridges
 * from the DOS-side pcbcomm TSR to a VxD that provides real serial
 * access.  Two VxD targets:
 *
 *   1. VCOMM.VXD (Windows built-in) — the standard Win9x serial port
 *      virtualizer.  Always present on Win9x.  DOS programs see a
 *      virtualized COM port; we use INT 14h (BIOS serial) which VCOMM
 *      already intercepts and routes to real hardware.
 *
 *   2. FOSSIL.VXD (our own, GPLv3) — hooks INT 14h from the VxD side
 *      with full FOSSIL semantics.  Already built by wrench (2026-08).
 *      When loaded, it replaces VCOMM's INT 14h handling with the
 *      FOSSIL protocol (AH=00..1Bh, FSC-0015 rev-5).
 *
 * The bridge works by issuing INT 14h calls (BIOS serial services)
 * from the DOS side.  Under Win9x, the VxD (either VCOMM or FOSSIL)
 * intercepts these and does the actual hardware work in ring 0.
 * This is the same mechanism Clark's COMMDV08 used — it's the only
 * way a DOS box can do serial I/O under Windows 9x.
 *
 * Clean-room implementation.  No Clark code was used or examined.
 * References: Win9x DDK documentation (VxD programming), Microsoft
 * INT 14h specification, FSC-0015 rev-5 FOSSIL spec, our FOSSIL.VXD
 * source (GPLv3, pcbirc crew).
 *
 * License: GPLv3 (pcbirc crew)
 * Author: wrench (transport/FOSSIL)
 * ==========================================================================*/

#include "pcbcomm.h"
#include "backend.h"

#include <dos.h>

/* ---- INT 14h function codes (BIOS serial + FOSSIL extensions) ---------- */
#define INT14_INIT          0x00    /* Initialize port                       */
#define INT14_WRITE_CHAR    0x01    /* Write character (with timeout)        */
#define INT14_READ_CHAR     0x02    /* Read character (with timeout)         */
#define INT14_STATUS        0x03    /* Get port status                       */
#define INT14_FOSSIL_INIT   0x04    /* FOSSIL: extended init (returns 1954h) */
#define INT14_FOSSIL_DEINIT 0x05    /* FOSSIL: deinit port                  */
#define INT14_FOSSIL_DTR    0x06    /* FOSSIL: raise/lower DTR              */
#define INT14_FOSSIL_BLOCK_READ  0x18  /* FOSSIL: block read from RX buffer */
#define INT14_FOSSIL_BLOCK_WRITE 0x19  /* FOSSIL: block write to TX buffer  */

/* ---- Baud rate encoding for INT 14h AH=00h (same as BIOS) ------------- */
static unsigned char baud_to_init_byte(unsigned long baud)
{
    /* INT 14h AH=00h init parameter: bits 7-5 = baud, 4-3 = parity,
     * bit 2 = stop bits, bits 1-0 = word length.
     * We set 8N1 (parity=00, stop=0, word=11) and vary the baud. */
    unsigned char base = 0x03;  /* 8N1 = 0000_0011 */
    switch (baud) {
        case 110:    return base | (0x00 << 5);
        case 150:    return base | (0x01 << 5);
        case 300:    return base | (0x02 << 5);
        case 600:    return base | (0x03 << 5);
        case 1200:   return base | (0x04 << 5);
        case 2400:   return base | (0x05 << 5);
        case 4800:   return base | (0x06 << 5);
        case 9600:   return base | (0x07 << 5);
        default:     return base | (0x07 << 5);  /* default 9600 */
    }
}

/* ---- Detect whether we're running under Windows 9x -------------------- */
static int detect_win9x(void)
{
    union REGS r;
    /* INT 2Fh AX=1600h — Get Windows Version.
     * Returns AL=major (3=Win3, 4=Win95/98, etc).
     * AL=0 or AL=80h means no Windows (bare DOS). */
    r.x.ax = 0x1600;
    int86(0x2F, &r, &r);
    return (r.h.al >= 3);  /* Win3.x or later */
}

/* ---- Detect whether FOSSIL is active on a port ------------------------ */
static int detect_fossil(unsigned int port)
{
    union REGS r;
    /* FOSSIL init (AH=04h) returns AX=1954h if a FOSSIL driver is
     * active on the port.  This works whether it's a DOS TSR FOSSIL
     * or our VxD FOSSIL — both respond the same way. */
    r.h.ah = INT14_FOSSIL_INIT;
    r.x.dx = port;
    int86(0x14, &r, &r);
    return (r.x.ax == 0x1954);
}

/* ---- Probe: check that Win9x is running and INT 14h works ------------- */
int vxd_bridge_probe(pcbcomm_port_t *p)
{
    if (!detect_win9x()) {
        /* Not running under Windows — this backend is only for DOS boxes */
        return -1;
    }
    /* Under Win9x, INT 14h is always available (VCOMM intercepts it).
     * If FOSSIL.VXD is also loaded, we get extended FOSSIL services.
     * Either way, basic INT 14h works. */
    return 0;
}

/* ---- Init: set baud rate + 8N1 via INT 14h AH=00h -------------------- */
int vxd_bridge_init(pcbcomm_port_t *p)
{
    union REGS r;
    int has_fossil;

    if (vxd_bridge_probe(p) < 0) {
        printf("pcbcomm: VXD_BRIDGE: not running under Windows 9x.\n");
        return -1;
    }

    /* Try FOSSIL init first — if FOSSIL.VXD is loaded, we get the
     * full FOSSIL protocol (block transfers, flow control, etc.) */
    has_fossil = detect_fossil(p->subport);

    /* Initialize port: AH=00h, AL=baud|parity|stop|bits, DX=port */
    r.h.ah = INT14_INIT;
    r.h.al = baud_to_init_byte(p->baud);
    r.x.dx = p->subport;   /* COM port number (0=COM1, 1=COM2, ...) */
    int86(0x14, &r, &r);

    /* Store FOSSIL availability for read/write optimization */
    p->chip = has_fossil ? 1 : 0;  /* 1 = FOSSIL available */

    p->open = 1;
    printf("pcbcomm: VXD_BRIDGE COM%u: %s, %lu baud\n",
           p->subport + 1,
           has_fossil ? "FOSSIL.VXD detected" : "VCOMM (BIOS INT 14h)",
           p->baud);
    return 0;
}

/* ---- Deinit: if FOSSIL, call deinit; otherwise nothing needed --------- */
void vxd_bridge_deinit(pcbcomm_port_t *p)
{
    if (!p->open) return;

    if (p->chip == 1) {
        /* FOSSIL deinit: AH=05h */
        union REGS r;
        r.h.ah = INT14_FOSSIL_DEINIT;
        r.x.dx = p->subport;
        int86(0x14, &r, &r);
    }
    p->open = 0;
}

/* ---- ISR: not used — the VxD handles interrupts in ring 0 ------------- */
void vxd_bridge_isr(pcbcomm_port_t *p)
{
    /* Under Win9x, the VxD (VCOMM or FOSSIL) owns the IRQ entirely.
     * The DOS side never sees hardware interrupts.  This function
     * exists only to satisfy the backend vtable. */
    (void)p;
}

/* ---- Read: pull bytes via INT 14h ------------------------------------- */
int vxd_bridge_read(pcbcomm_port_t *p, void *buf, int n)
{
    unsigned char *dst = (unsigned char *)buf;
    int count = 0;

    if (p->chip == 1 && n > 1) {
        /* FOSSIL block read: AH=18h, CX=count, ES:DI=buffer, DX=port.
         * Returns AX = bytes actually read. Much faster than one-at-a-time. */
        union REGS r;
        struct SREGS s;
        r.h.ah = INT14_FOSSIL_BLOCK_READ;
        r.x.cx = (unsigned int)n;
        r.x.dx = p->subport;
        r.x.di = FP_OFF(dst);
        s.es   = FP_SEG(dst);
        int86x(0x14, &r, &r, &s);
        return r.x.ax;  /* bytes read */
    }

    /* Fallback: one byte at a time via AH=02h.
     * Check status first (AH=03h) to avoid blocking. */
    while (count < n) {
        union REGS r;

        /* Status check: AH=03h, DX=port. AH returns LSR, AL returns MSR.
         * Bit 0 of AH (returned) = Data Ready. */
        r.h.ah = INT14_STATUS;
        r.x.dx = p->subport;
        int86(0x14, &r, &r);
        if (!(r.h.ah & 0x01)) break;  /* no data ready */

        /* Read one byte: AH=02h, DX=port. Returns AL=char, AH=status. */
        r.h.ah = INT14_READ_CHAR;
        r.x.dx = p->subport;
        int86(0x14, &r, &r);
        if (r.h.ah & 0x80) break;  /* timeout/error */
        dst[count++] = r.h.al;
    }
    return count;
}

/* ---- Write: push bytes via INT 14h ------------------------------------ */
int vxd_bridge_write(pcbcomm_port_t *p, const void *buf, int n)
{
    const unsigned char *src = (const unsigned char *)buf;
    int count = 0;

    if (p->chip == 1 && n > 1) {
        /* FOSSIL block write: AH=19h, CX=count, ES:DI=buffer, DX=port.
         * Returns AX = bytes actually written. */
        union REGS r;
        struct SREGS s;
        r.h.ah = INT14_FOSSIL_BLOCK_WRITE;
        r.x.cx = (unsigned int)n;
        r.x.dx = p->subport;
        r.x.di = FP_OFF(src);
        s.es   = FP_SEG(src);
        int86x(0x14, &r, &r, &s);
        return r.x.ax;  /* bytes written */
    }

    /* Fallback: one byte at a time via AH=01h. */
    while (count < n) {
        union REGS r;
        r.h.ah = INT14_WRITE_CHAR;
        r.h.al = src[count];
        r.x.dx = p->subport;
        int86(0x14, &r, &r);
        if (r.h.ah & 0x80) break;  /* timeout/error (bit 7 = timeout) */
        count++;
    }
    return count;
}

/* ---- Backend vtable --------------------------------------------------- */
const pcbcomm_backend_t pcbcomm_vxd_bridge_backend = {
    "VXD_BRIDGE",
    NULL,                   /* no per-card state needed */
    vxd_bridge_probe,
    vxd_bridge_init,
    vxd_bridge_deinit,
    vxd_bridge_isr,
    vxd_bridge_read,
    vxd_bridge_write
};
