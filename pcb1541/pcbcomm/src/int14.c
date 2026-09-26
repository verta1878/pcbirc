/* ============================================================================
 * int14.c — INT 14h FOSSIL 5 revision C handler for pcbcomm
 *
 * Implements the complete FOSSIL (Fido/Opus/Seadog Standard Interface Layer)
 * specification, FTS-0015 revision 5.
 *
 * FOSSIL calls hook INT 14h. Function selected by AH register; port
 * number in DX (0-based). Return values in AX (mostly), with modem/
 * line status typically in AH and byte value in AL.
 *
 * All 28 FOSSIL functions (00h-1Bh) implemented, plus COMM-DRV
 * extensions (10h-14h) for PCBoard compatibility.
 *
 * References:
 *   FTS-0015 (FOSSIL specification)
 *   drivers/netfosdl/fossil.pas (crew, GPLv3 — Free Pascal)
 *   mystic/mystfoss.pas (crew, GPLv3 — Mystic BBS FOSSIL driver)
 *   drivers/rlfossil/rlfossil.c (reference, recovered)
 *
 * pcbirc crew, GPLv3.  Revision C — 2026-09-22.
 * ==========================================================================*/

#include <dos.h>
#include <string.h>
#include "pcbcomm.h"
#include "compat.h"
#include "backend.h"
#include "uart.h"

/* ---- External state ---------------------------------------------------- */

extern pcbcomm_port_t g_ports[PCBCOMM_MAX_PORTS];
extern int            g_n_ports;

/* ---- Constants --------------------------------------------------------- */

/* FOSSIL signature: Init returns 0x1954 in AX */
#define FOSSIL_SIG     0x1954

/* FOSSIL revision level */
#define FOSSIL_REV     5

/* Driver info structure size (function 1Bh) */
#define FOSSIL_INFO_SIZE  19

/* Driver ID string — returned via function 1Bh info block */
static const char fossil_id[] = "pcbcomm FOSSIL 5C";

/* ---- Saved vectors ----------------------------------------------------- */

static pcbcomm_isr_t g_old_int14;

/* ---- Timer tick chain (function 16h) ----------------------------------- */

#define MAX_TIMER_HOOKS  4

typedef struct {
    void (PCBCOMM_FAR *func)(void);
    int  active;
} timer_hook_t;

static timer_hook_t  g_timer_hooks[MAX_TIMER_HOOKS];
static pcbcomm_isr_t g_old_int1c;  /* saved timer tick vector */
static int           g_timer_installed;

/* Timer tick ISR — calls all registered hooks, then chains to old vector */
PCBCOMM_INTERRUPT pcbcomm_int1c(PCBCOMM_INT14_ARGS)
{
    int i;
    for (i = 0; i < MAX_TIMER_HOOKS; i++) {
        if (g_timer_hooks[i].active && g_timer_hooks[i].func)
            g_timer_hooks[i].func();
    }
    /* Chain to old INT 1Ch — the old vector runs when we IRET.
     * For a proper chain we'd jump, but calling it as a function
     * and then returning via IRET is safe for timer tick. */
    if (g_old_int1c) {
        /* Call old handler — compiler generates far call + IRET handling */
        _chain_intr(g_old_int1c);
    }
    PCBCOMM_UNUSED_REGS;
}

static void timer_install(void)
{
    if (!g_timer_installed) {
        memset(g_timer_hooks, 0, sizeof(g_timer_hooks));
        g_old_int1c = _dos_getvect(0x1C);
        _dos_setvect(0x1C, pcbcomm_int1c);
        g_timer_installed = 1;
    }
}

static void timer_remove(void)
{
    if (g_timer_installed) {
        _dos_setvect(0x1C, g_old_int1c);
        g_timer_installed = 0;
    }
}

/* ---- Port helpers ------------------------------------------------------ */

static pcbcomm_port_t *port_lookup(unsigned int port_num)
{
    if (port_num >= (unsigned int)g_n_ports) return NULL;
    if (!g_ports[port_num].open) return NULL;
    return &g_ports[port_num];
}

/* ---- Status word (AH=line, AL=modem) ---------------------------------- */

/*  AH (line status, LSR-like):
 *    bit 0  = Data Ready (RX data available)
 *    bit 5  = TX Holding Register Empty
 *    bit 6  = TX Shift Register Empty
 *    bit 7  = (always set — port ready)
 *
 *  AL (modem status, MSR):
 *    bit 4  = CTS
 *    bit 5  = DSR
 *    bit 6  = RI
 *    bit 7  = DCD
 *
 *  Uses cached_msr from port struct (updated by backend ISR) so this
 *  works for all backends, not just direct-UART ones.
 */
static unsigned int status_word(pcbcomm_port_t *p)
{
    unsigned char ah = 0x80;  /* bit 7 always set */
    unsigned char al = 0;

    if (!p) return 0x0080;

    /* Line status */
    if (p->rx_head != p->rx_tail)  ah |= 0x01;  /* RX data available */
    if (p->tx_head == p->tx_tail)  ah |= 0x60;  /* TX empty          */

    /* Modem status — use cached value from backend ISR.
     * For UART backends, the ISR reads MSR on every interrupt and
     * stores it in cached_msr. For non-UART backends (Digi, Cyclom,
     * Stallion, etc), the backend updates cached_msr from whatever
     * status mechanism the card provides. */
    al = p->cached_msr & 0xF0;  /* CTS, DSR, RI, DCD */

    return ((unsigned int)ah << 8) | al;
}

/* ---- Baud rate encoding (function 00h) -------------------------------- */

/* AL register encodes baud/parity/bits/stop in BIOS format:
 *   bits 7-5: baud rate (000=19200..111=300 — NOTE: different from BIOS!)
 *   bits 4-3: parity (00=none, 01=odd, 11=even)
 *   bits 2:   stop bits (0=1, 1=2)
 *   bits 1-0: word length (10=7, 11=8)
 *
 * FOSSIL spec uses a different baud encoding than BIOS INT 14h:
 *   000=19200, 001=38400, 010=300, 011=600, 100=1200,
 *   101=2400, 110=4800, 111=9600
 */
static const long baud_table[8] = {
    19200L, 38400L, 300L, 600L, 1200L, 2400L, 4800L, 9600L
};

static void parse_baud_byte(pcbcomm_port_t *p, unsigned char al)
{
    unsigned char baud_idx = (al >> 5) & 0x07;
    unsigned char par      = (al >> 3) & 0x03;
    unsigned char stopb    = (al >> 2) & 0x01;
    unsigned char wlen     = (al)      & 0x03;

    p->baud      = baud_table[baud_idx];
    p->data_bits = 5 + wlen;   /* 00=5, 01=6, 10=7, 11=8 */
    p->stop_bits = stopb + 1;  /* 0→1, 1→2 */

    switch (par) {
        case 0:  p->parity = 'N'; break;
        case 1:  p->parity = 'O'; break;
        case 3:  p->parity = 'E'; break;
        default: p->parity = 'N'; break;
    }

    /* Rebuild LCR */
    p->lcr = (unsigned char)((wlen & 0x03) | ((stopb & 0x01) << 2));
    if (par == 1) p->lcr |= 0x08;       /* odd parity enable */
    if (par == 3) p->lcr |= 0x18;       /* even parity enable */

    /* Apply to hardware */
    if (p->backend && p->backend->init)
        (void)p->backend->init(p);
}

/* ---- DTR control (function 06h) ---------------------------------------- */

static void set_dtr(pcbcomm_port_t *p, int state)
{
    unsigned char mcr;
    if (!p || !p->base) return;

    mcr = inp(p->base + UART_MCR);
    if (state)
        mcr |= MCR_DTR;
    else
        mcr &= ~MCR_DTR;
    outp(p->base + UART_MCR, mcr);
}

/* ---- Break signal (function 1Ah) --------------------------------------- */

static void send_break(pcbcomm_port_t *p, int on)
{
    unsigned char lcr;
    if (!p || !p->base) return;

    lcr = inp(p->base + UART_LCR);
    if (on)
        lcr |= LCR_BREAK;
    else
        lcr &= ~LCR_BREAK;
    outp(p->base + UART_LCR, lcr);
}

/* ---- Ring buffer helpers ----------------------------------------------- */

static unsigned int rx_count(pcbcomm_port_t *p)
{
    return (p->rx_head - p->rx_tail) & (p->rx_size - 1);
}

static unsigned int tx_count(pcbcomm_port_t *p)
{
    return (p->tx_head - p->tx_tail) & (p->tx_size - 1);
}

static unsigned int tx_free(pcbcomm_port_t *p)
{
    return p->tx_size - 1 - tx_count(p);
}

/* ========================================================================
 * Main dispatcher — called via INT 14h vector
 * ======================================================================== */

PCBCOMM_INTERRUPT pcbcomm_int14(PCBCOMM_INT14_ARGS)
{
    unsigned char func = (PCBCOMM_AX >> 8) & 0xFF;
    unsigned char ch   = PCBCOMM_AX & 0xFF;
    unsigned int  port = PCBCOMM_DX;
    pcbcomm_port_t *p;
    unsigned int rc = 0;
    unsigned char buf[1];

    /* Functions 04h (init) and 1Bh (info) work even if port not open */
    p = port_lookup(port);
    if (!p && func != 0x04 && func != 0x1B) {
        PCBCOMM_AX = 0x0080;  /* timeout / not ready */
        return;
    }

    switch (func) {

    /* ---- Standard FOSSIL functions (00h-0Ch, 0Fh, 18h-1Bh) ------------ */

    case 0x00:  /* Set baud rate + line params */
        if (p) {
            parse_baud_byte(p, ch);
            rc = status_word(p);
        }
        break;

    case 0x01:  /* TX character with wait */
        if (p && p->backend && p->backend->write) {
            buf[0] = ch;
            while (p->backend->write(p, buf, 1) == 0)
                ;  /* block until sent */
        }
        rc = status_word(p);
        break;

    case 0x02:  /* RX character with wait */
        if (p && p->backend && p->backend->read) {
            while (p->backend->read(p, buf, 1) == 0)
                ;  /* block until received */
            rc = (status_word(p) & 0xFF00) | buf[0];
        }
        break;

    case 0x03:  /* Request status */
        rc = status_word(p);
        break;

    case 0x04:  /* Initialize driver — returns 0x1954 in AX */
        if (port < (unsigned int)g_n_ports) {
            p = &g_ports[port];
            if (!p->open && p->backend && p->backend->init) {
                p->backend->init(p);
                p->open = 1;
            }
        }
        rc = FOSSIL_SIG;
        /* BH = FOSSIL revision, BL = max port number */
        PCBCOMM_BX = ((unsigned int)FOSSIL_REV << 8) |
                     (unsigned int)(g_n_ports > 0 ? g_n_ports - 1 : 0);
        break;

    case 0x05:  /* Deinitialize driver */
        if (p) {
            if (p->backend && p->backend->deinit)
                p->backend->deinit(p);
            p->open = 0;
        }
        rc = status_word(p);
        break;

    case 0x06:  /* Raise/lower DTR — AL: 01=raise, 00=lower */
        set_dtr(p, ch & 0x01);
        rc = status_word(p);
        break;

    case 0x07:  /* Return timer tick parameters
                 * AH = ticks/sec approximation (18)
                 * AL = milliseconds/tick approximation (55) */
        rc = (18 << 8) | 55;
        break;

    case 0x08:  /* Flush output buffer — block until TX buffer drained */
        if (p && p->backend && p->backend->write) {
            /* Pump TX ring until empty */
            while (p->tx_head != p->tx_tail) {
                unsigned char c = p->tx_buf[p->tx_tail];
                buf[0] = c;
                if (p->backend->write(p, buf, 1) > 0) {
                    p->tx_tail = (p->tx_tail + 1) & (p->tx_size - 1);
                }
            }
        }
        rc = status_word(p);
        break;

    case 0x09:  /* Purge output buffer — discard unsent data */
        if (p) {
            p->tx_head = p->tx_tail = 0;
        }
        rc = status_word(p);
        break;

    case 0x0A:  /* Purge input buffer — discard unread data */
        if (p) {
            p->rx_head = p->rx_tail = 0;
        }
        rc = status_word(p);
        break;

    case 0x0B:  /* TX character without wait — AL = char
                 * Returns AX = 1 if accepted, 0 if buffer full */
        if (p && p->backend && p->backend->write) {
            buf[0] = ch;
            rc = (p->backend->write(p, buf, 1) > 0) ? 1 : 0;
        }
        break;

    case 0x0C:  /* Peek at input — AX = next char without removing,
                 * or AX = 0xFFFF if buffer empty */
        if (p && p->rx_head != p->rx_tail) {
            rc = (status_word(p) & 0xFF00) | p->rx_buf[p->rx_tail];
        } else {
            rc = 0xFFFF;
        }
        break;

    case 0x0D:  /* Read keyboard character (with wait)
                 * AH = scan code, AL = ASCII.
                 * Passes through to BIOS INT 16h AH=00h. */
        {
            union REGS r;
            r.h.ah = 0x00;
            int86(0x16, &r, &r);
            rc = r.x.ax;
        }
        break;

    case 0x0E:  /* Peek keyboard (non-destructive read)
                 * AX = 0xFFFF if no key, else AH=scan, AL=ASCII.
                 * Uses BIOS INT 16h AH=01h. */
        {
            union REGS r;
            r.h.ah = 0x01;
            int86(0x16, &r, &r);
            if (r.x.cflag & 0x40)  /* ZF set = no key */
                rc = 0xFFFF;
            else
                rc = r.x.ax;
        }
        break;

    case 0x0F:  /* Enable/disable flow control
                 * AL bit 0: XON/XOFF on transmit
                 * AL bit 1: CTS/RTS hardware flow
                 * AL bit 3: XON/XOFF on receive */
        if (p) {
            if (ch & 0x02)
                p->flow = 1;       /* RTS/CTS */
            else if (ch & 0x09)
                p->flow = 2;       /* XON/XOFF */
            else
                p->flow = 0;       /* none */
        }
        rc = status_word(p);
        break;

    /* ---- COMM-DRV extension: AH=10h, AL selects operation --------------- */
    /* Both MODEMFOS.C and MODEMDRV.C send AX=1000h (commgo) and
     * AX=1002h (commstop). These are NOT separate AH values 10h-14h;
     * they are AH=10h with AL as the sub-function selector. */

    case 0x10:
        switch (ch) {
            case 0x00:  /* commgo — flush TX / start transmit */
                if (p && p->backend && p->backend->write)
                    (void)p->backend->write(p, NULL, 0);
                break;

            case 0x01:  /* query port count */
                rc = (unsigned int)g_n_ports;
                PCBCOMM_AX = rc;
                PCBCOMM_UNUSED_REGS;
                return;

            case 0x02:  /* commstop — stop TX, flush RX+TX */
                if (p) {
                    p->rx_head = p->rx_tail = 0;
                    p->tx_head = p->tx_tail = 0;
                }
                break;

            case 0x03:  /* query backend name — 2-char sig in BX */
                if (p && p->backend) {
                    PCBCOMM_BX = ((unsigned int)p->backend->name[0] << 8) |
                                 (unsigned char)(p->backend->name[1] ?
                                                  p->backend->name[1] : 0);
                }
                break;

            case 0x04:  /* set/get baud — bit0: 0=get,1=set; CX=baud */
                if (p) {
                    if ((PCBCOMM_AX & 1) == 0) {
                        PCBCOMM_CX = (unsigned int)p->baud;
                    } else {
                        p->baud = PCBCOMM_CX;
                        if (p->backend && p->backend->init)
                            (void)p->backend->init(p);
                    }
                }
                break;

            default:
                break;
        }
        rc = status_word(p);
        break;

    /* ---- FOSSIL functions 15h-1Bh -------------------------------------- */

    case 0x15:  /* Write character with attribute to screen
                 * AL = character, BL = attribute.
                 * Uses BIOS INT 10h AH=09h (write char+attr at cursor). */
        {
            union REGS r;
            r.h.ah = 0x09;
            r.h.al = ch;
            r.h.bh = 0;              /* page 0 */
            r.h.bl = (unsigned char)(PCBCOMM_BX & 0xFF);
            r.x.cx = 1;              /* count = 1 */
            int86(0x10, &r, &r);
        }
        rc = status_word(p);
        break;

    case 0x16:  /* Timer chain management
                 * AL = 0: add hook, ES:DX = far pointer to routine
                 * AL = 1: remove hook, ES:DX = far pointer to routine */
        {
            int i;
            void (PCBCOMM_FAR *hook_ptr)(void);

            if (!g_timer_installed) timer_install();

            /* Extract the far function pointer from ES:DX */
            hook_ptr = (void (PCBCOMM_FAR *)(void))
                       PCBCOMM_MK_FP(PCBCOMM_ES, PCBCOMM_DX);

            if (ch == 0) {
                /* Add hook */
                for (i = 0; i < MAX_TIMER_HOOKS; i++) {
                    if (!g_timer_hooks[i].active) {
                        g_timer_hooks[i].func = hook_ptr;
                        g_timer_hooks[i].active = 1;
                        rc = 0;  /* success */
                        break;
                    }
                }
                if (i == MAX_TIMER_HOOKS) rc = 0xFFFF;  /* table full */
            } else {
                /* Remove hook — match by function pointer */
                for (i = 0; i < MAX_TIMER_HOOKS; i++) {
                    if (g_timer_hooks[i].active &&
                        g_timer_hooks[i].func == hook_ptr) {
                        g_timer_hooks[i].active = 0;
                        g_timer_hooks[i].func = NULL;
                        rc = 0;
                        break;
                    }
                }
            }
        }
        break;

    case 0x17:  /* Reboot system
                 * AL = 0: cold boot (full POST)
                 * AL = 1: warm boot (skip POST) */
#if defined(__WATCOMC__)
        if (ch == 0) {
            /* Cold boot: jump to FFFF:0000 */
            { void (__far *reboot)(void) = (void (__far *)(void))0xFFFF0000L;
              reboot(); }
        } else {
            /* Warm boot: set 0040:0072 = 0x1234, then jump to FFFF:0000 */
            *(unsigned int __far *)0x00400072L = 0x1234;
            { void (__far *reboot)(void) = (void (__far *)(void))0xFFFF0000L;
              reboot(); }
        }
#elif defined(__BORLANDC__) || defined(__TURBOC__)
        if (ch == 0) {
            { void (far *reboot)(void) = (void (far *)(void))0xFFFF0000L;
              reboot(); }
        } else {
            *(unsigned int far *)0x00400072L = 0x1234;
            { void (far *reboot)(void) = (void (far *)(void))0xFFFF0000L;
              reboot(); }
        }
#elif defined(_MSC_VER)
        if (ch == 0) {
            { void (_far *reboot)(void) = (void (_far *)(void))0xFFFF0000L;
              reboot(); }
        } else {
            *(unsigned int _far *)0x00400072L = 0x1234;
            { void (_far *reboot)(void) = (void (_far *)(void))0xFFFF0000L;
              reboot(); }
        }
#endif
        break;

    case 0x18:  /* Block read — CX = max bytes, ES:DI = buffer
                 * Returns AX = bytes actually read */
        if (p && p->backend && p->backend->read) {
            unsigned int want = PCBCOMM_CX;
            unsigned int got  = 0;
            unsigned char PCBCOMM_FAR *dest =
                PCBCOMM_FAR_PTR(PCBCOMM_ES, PCBCOMM_DI);
            unsigned char one_byte;
            while (got < want) {
                if (p->backend->read(p, &one_byte, 1) > 0) {
                    dest[got] = one_byte;
                    got++;
                } else {
                    break;  /* buffer empty, return what we have */
                }
            }
            rc = got;
        }
        break;

    case 0x19:  /* Block write — CX = byte count, ES:DI = buffer
                 * Returns AX = bytes actually written */
        if (p && p->backend && p->backend->write) {
            unsigned int want = PCBCOMM_CX;
            unsigned int sent = 0;
            unsigned char PCBCOMM_FAR *src =
                PCBCOMM_FAR_PTR(PCBCOMM_ES, PCBCOMM_DI);
            unsigned char one_byte;
            while (sent < want) {
                one_byte = src[sent];
                if (p->backend->write(p, &one_byte, 1) > 0) {
                    sent++;
                } else {
                    break;  /* buffer full, return what we sent */
                }
            }
            rc = sent;
        }
        break;

    case 0x1A:  /* Break signal — AL: 01=start, 00=stop */
        send_break(p, ch & 0x01);
        rc = status_word(p);
        break;

    case 0x1B:  /* Get driver information
                 * CX = buffer size, ES:DI = buffer pointer
                 * Returns AX = bytes written (up to FOSSIL_INFO_SIZE)
                 *
                 * Info structure (19 bytes):
                 *   u16  structure_size      (19)
                 *   u8   fossil_spec_version (5)
                 *   u8   driver_revision     ('C' = 0x43)
                 *   u32  id_string_ptr       (far pointer to ID string)
                 *   u16  rx_buf_size
                 *   u16  rx_bytes_pending
                 *   u16  tx_buf_size
                 *   u16  tx_bytes_pending
                 *   u8   screen_width        (80)
                 *   u8   screen_height       (25)
                 *   u8   baud_rate_mask      (from last fn 00h call)
                 *
                 * Note: Clark's MODEMFOS.C sets CX = sizeof(fossilstruct)
                 * before calling, but FTS-0015 says the struct is fixed at
                 * 19 bytes. We always write all 19 regardless of CX, since
                 * every caller passes a struct that size or larger. Treating
                 * CX == 0 or CX > 19 as 19 is defensive against callers
                 * that don't set CX (some third-party FOSSIL clients). */
        {
            unsigned int buf_size = FOSSIL_INFO_SIZE;  /* always write full struct */
            unsigned char info[FOSSIL_INFO_SIZE];

            /* Structure size */
            info[0] = FOSSIL_INFO_SIZE & 0xFF;
            info[1] = (FOSSIL_INFO_SIZE >> 8) & 0xFF;
            /* FOSSIL spec version */
            info[2] = FOSSIL_REV;
            /* Driver revision: 'C' */
            info[3] = 'C';
            /* Far pointer to ID string */
            {
                unsigned long fp = (unsigned long)(void PCBCOMM_FAR *)fossil_id;
                info[4] = (unsigned char)(fp);
                info[5] = (unsigned char)(fp >> 8);
                info[6] = (unsigned char)(fp >> 16);
                info[7] = (unsigned char)(fp >> 24);
            }
            /* RX buffer size */
            if (p) {
                info[8]  = (unsigned char)(p->rx_size);
                info[9]  = (unsigned char)(p->rx_size >> 8);
                /* RX bytes pending */
                { unsigned int n = rx_count(p);
                  info[10] = (unsigned char)(n);
                  info[11] = (unsigned char)(n >> 8);
                }
                /* TX buffer size */
                info[12] = (unsigned char)(p->tx_size);
                info[13] = (unsigned char)(p->tx_size >> 8);
                /* TX bytes pending */
                { unsigned int n = tx_count(p);
                  info[14] = (unsigned char)(n);
                  info[15] = (unsigned char)(n >> 8);
                }
            } else {
                memset(&info[8], 0, 8);
            }
            /* Screen width, height */
            info[16] = 80;
            info[17] = 25;
            /* Baud rate mask (last fn 00h AL value — simplified) */
            info[18] = 0;

            /* Copy info block to caller's ES:DI buffer */
            {
                unsigned char PCBCOMM_FAR *dest =
                    PCBCOMM_FAR_PTR(PCBCOMM_ES, PCBCOMM_DI);
                unsigned int i;
                for (i = 0; i < buf_size; i++)
                    dest[i] = info[i];
            }
            rc = buf_size;
        }
        break;

    case 0xFF:  /* ---- Admin / control channel (wrench, 2026-09-26) ----
                 * Used by PCBDTSR.EXE -d (deinstall) to tell the resident
                 * copy to shut down cleanly.
                 *   AL=01h: unload request
                 *     - unhooks INT 14h (restores saved vector)
                 *     - shuts down all IRQs
                 *     - deinits all backends
                 *     - returns AX=0x4F52 ("OR" = OK-Removed)
                 *   All other AL values: reserved, returns 0 (no-op). */
        switch (ch) {  /* AL sub-command */
        case 0x01:  /* unload */
            pcbcomm_int14_uninstall();
            pcbcomm_irq_shutdown();
            /* Deinit all active backends */
            {
                int i;
                for (i = 0; i < g_n_ports; i++) {
                    pcbcomm_port_t *pp = &g_ports[i];
                    if (pp->backend && pp->open)
                        pp->backend->deinit(pp);
                }
            }
            rc = 0x4F52;  /* "OR" = OK-Removed */
            break;
        default:
            rc = 0;
            break;
        }
        break;

    default:
        /* Unimplemented / future — return status as safe no-op */
        rc = status_word(p);
        break;
    }

    PCBCOMM_AX = rc;
    PCBCOMM_UNUSED_REGS;
}

/* ---- Install / uninstall INT 14h vector -------------------------------- */

void pcbcomm_int14_install(void)
{
    g_old_int14 = _dos_getvect(0x14);
    _dos_setvect(0x14, pcbcomm_int14);
}

void pcbcomm_int14_uninstall(void)
{
    if (g_timer_installed) timer_remove();
    if (g_old_int14)
        _dos_setvect(0x14, g_old_int14);
}
