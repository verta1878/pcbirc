/* ============================================================================
 * int14.c — INT 14h FOSSIL API handler for pcbdcom
 *
 * Implements the FOSSIL (Fido/Opus/Seadog Standard Interface Layer)
 * specification, revision 5. Spec: http://www.textfiles.com/programming/fossil.txt
 *
 * FOSSIL calls hook INT 14h. Function selected by AH register; port
 * number in DX (0-based). Return values in AX (mostly), with modem/
 * line status typically in AH and byte value in AL.
 *
 * Reference: drivers/netfosdl/fossil.pas (crew, GPLv3 — Free Pascal
 * DOS FOSSIL, native implementation). This file is a C port of the
 * dispatch logic.
 *
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include <dos.h>
#include <string.h>
#include "pcbdcom.h"
#include "backend.h"

/* External port table — defined in pcbdcom.c */
extern pcbdcom_port_t g_ports[PCBDCOM_MAX_PORTS];
extern int            g_n_ports;

/* Saved original INT 14h vector (installed BIOS/prior driver) */
static void (__interrupt __far *g_old_int14)();

/* FOSSIL signature: after Init(00h), returns 0x1954 in AX (magic). */
#define FOSSIL_SIG_LO  0x54
#define FOSSIL_SIG_HI  0x19

/* Convert AH FOSSIL func + DX port to a pcbdcom_port_t*, or NULL */
static pcbdcom_port_t *port_lookup(unsigned int port_num)
{
    if (port_num >= (unsigned int)g_n_ports) return NULL;
    if (!g_ports[port_num].open) return NULL;
    return &g_ports[port_num];
}

/* Line + modem status bytes for FOSSIL status returns.
 * AH = line status (LSR-like), AL = modem status (MSR-like) — differs
 * per function, see FOSSIL spec. */
static unsigned int status_word(pcbdcom_port_t *p)
{
    unsigned char ah = 0;
    unsigned char al = 0;

    /* AH: bit7=1 always (always ready), bit5=THRE, bit6=TEMT, bit0=DR */
    if (p->rx_head != p->rx_tail) ah |= 0x01;   /* RX data available */
    if (p->tx_head == p->tx_tail) ah |= 0x60;   /* TX buffer empty   */
    ah |= 0x80;                                 /* Port ready        */

    /* AL: modem status — extension. FOSSIL func 03h returns MSR here. */
    (void)al;

    return ((unsigned int)ah << 8);
}

/* Main dispatcher. Called via INT 14h vector. */
void __interrupt __far pcbdcom_int14(unsigned _es, unsigned _ds,
                                     unsigned _di, unsigned _si,
                                     unsigned _bp, unsigned _sp,
                                     unsigned _bx, unsigned _dx,
                                     unsigned _cx, unsigned _ax,
                                     unsigned _ip, unsigned _cs,
                                     unsigned _flags)
{
    unsigned char func = (_ax >> 8) & 0xFF;
    unsigned char ch   = _ax & 0xFF;
    pcbdcom_port_t *p  = port_lookup(_dx);
    unsigned int rc    = 0;
    unsigned char buf[1];

    if (!p && func != 0x00 && func != 0x04) {
        /* Unknown port — pass through to old handler (may be BIOS INT 14h
         * serving COM1..4 for callers who don't know about pcbdcom).
         * Simplest: return "not ready" so caller falls back. */
        _ax = 0x0080;   /* AH=timeout, AL=0 */
        return;
    }

    switch (func) {
        case 0x00: /* Initialize port — legacy BIOS INT 14h func 00h.
                    * AL = params packed (baud/parity/bits/stop). We
                    * honor as a set-baud-and-lcr call; init happens
                    * at load time so hardware is already up. */
            rc = status_word(p);
            break;

        case 0x01: /* Send character (blocking) — AL = char */
            if (p && p->backend && p->backend->write) {
                buf[0] = ch;
                while (p->backend->write(p, buf, 1) == 0)
                    ; /* spin — real code would yield */
            }
            rc = status_word(p);
            break;

        case 0x02: /* Receive character (blocking) — returns char in AL */
            if (p && p->backend && p->backend->read) {
                while (p->backend->read(p, buf, 1) == 0)
                    ; /* spin */
                rc = ((unsigned int)status_word(p) & 0xFF00) | buf[0];
            }
            break;

        case 0x03: /* Get port status */
            rc = status_word(p);
            break;

        case 0x04: /* FOSSIL Init — returns 0x1954 magic + port count */
            rc = (FOSSIL_SIG_HI << 8) | FOSSIL_SIG_LO;
            /* BL = maximum port number, BH = revision (5) */
            _bx = (5 << 8) | (g_n_ports - 1);
            break;

        case 0x05: /* FOSSIL Deinit */
            rc = status_word(p);
            break;

        case 0x0B: /* Non-blocking send — AL = char */
            if (p && p->backend && p->backend->write) {
                buf[0] = ch;
                rc = (p->backend->write(p, buf, 1) > 0) ? 1 : 0;
            }
            break;

        case 0x0C: /* Non-blocking receive — 0xFFFF if empty, else char in AL */
            if (p && p->backend && p->backend->read) {
                if (p->backend->read(p, buf, 1) > 0)
                    rc = ((unsigned int)status_word(p) & 0xFF00) | buf[0];
                else
                    rc = 0xFFFF;
            }
            break;

        case 0x0D: /* Peek RX buffer — 0xFFFF if empty, else char without consuming */
            if (p && p->rx_head != p->rx_tail)
                rc = ((unsigned int)status_word(p) & 0xFF00) | p->rx_buf[p->rx_tail];
            else
                rc = 0xFFFF;
            break;

        case 0x0E: /* Keyboard read (fossil ext — skip in v1) */
        case 0x0F: /* Flow control config */
            rc = status_word(p);
            break;

        default:
            /* Unimplemented function — chain to old handler? For safety
             * just return "OK" with status word. */
            rc = status_word(p);
            break;
    }

    _ax = rc;
    /* Preserve other regs by not writing them */
    (void)_bp; (void)_si; (void)_di; (void)_es; (void)_ds;
    (void)_cx; (void)_sp; (void)_ip; (void)_cs; (void)_flags;
}

/* Install / uninstall INT 14h vector */
void pcbdcom_int14_install(void)
{
    g_old_int14 = _dos_getvect(0x14);
    _dos_setvect(0x14, pcbdcom_int14);
}

void pcbdcom_int14_uninstall(void)
{
    if (g_old_int14)
        _dos_setvect(0x14, g_old_int14);
}
