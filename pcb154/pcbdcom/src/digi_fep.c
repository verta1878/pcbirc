/* ============================================================================
 * digi_fep.c — Shared FEP protocol implementation for Digi PC/Xe + AccelePort
 *
 * Ported from Linux drivers/char/epca.c (kernel 2.6.32), GPLv2.
 * Original authors: Digi International, Daniel Taylor, Chris Elston.
 * DOS 16-bit adaptations: pcbirc crew (hexadecimal), GPLv3.
 *
 * See digi_fep.h for card window / mailbox / board_chan layout.
 * See digi_pcxe_backend.c + digi_accel_backend.c for the card-specific
 * probe and init wrappers.
 * ==========================================================================*/

#include "pcbdcom.h"
#include "digi_fep.h"

/* ----- Register access via seg:offset far pointer ----- */

unsigned char digi_fep_readb(unsigned int seg, unsigned long off)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}
void digi_fep_writeb(unsigned int seg, unsigned long off, unsigned char v)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}
unsigned int digi_fep_readw(unsigned int seg, unsigned long off)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}
void digi_fep_writew(unsigned int seg, unsigned long off, unsigned int v)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

/* ----- fepcmd — send command, wait for FEP acknowledgment ----- *
 * Ported from epca.c fepcmd() (line 1561). */
int digi_fep_cmd(unsigned int seg, unsigned char chan, unsigned char op,
                 unsigned char arg1, unsigned char arg2)
{
    unsigned int cin, cout, cmax;
    long timeout;

    cmax = digi_fep_readw(seg, GLOBAL + MB_CMAX);
    cin  = digi_fep_readw(seg, GLOBAL + MB_CIN);

    digi_fep_writeb(seg, CSTART + cin + 0, op);
    digi_fep_writeb(seg, CSTART + cin + 1, chan);
    digi_fep_writeb(seg, CSTART + cin + 2, arg1);
    digi_fep_writeb(seg, CSTART + cin + 3, arg2);

    cin = (cin + 4) & (cmax - 4);
    digi_fep_writew(seg, GLOBAL + MB_CIN, cin);

    for (timeout = FEPTIMEOUT; timeout > 0; timeout--) {
        cout = digi_fep_readw(seg, GLOBAL + MB_COUT);
        if (cout == cin) return 0;
    }
    return -1;
}

/* ----- Per-channel init: baud + line + open ----- */
int digi_fep_init_channel(pcbdcom_port_t *p, unsigned char chan)
{
    unsigned int seg;
    unsigned int baud_code;

    seg = ((digi_fep_card_t *)p->backend_data)->card_seg;

    switch (p->baud) {
        case 300:    baud_code = 0x07;   break;
        case 1200:   baud_code = 0x09;   break;
        case 2400:   baud_code = 0x0B;   break;
        case 9600:   baud_code = 0x0D;   break;
        case 19200:  baud_code = 0x0E;   break;
        case 38400:  baud_code = 0x0F;   break;
        case 57600:  baud_code = 0x1001; break;
        case 115200: baud_code = 0x1002; break;
        default:     baud_code = 0x0F;   break;
    }
    digi_fep_writew(seg, BC_BASE(chan) + BC_CFLAG, baud_code | 0x0300);

    if (digi_fep_cmd(seg, chan, RESUMERX, 0, 0)     < 0) return -1;
    if (digi_fep_cmd(seg, chan, RESUMETX, 0, 0)     < 0) return -1;
    if (digi_fep_cmd(seg, chan, SETMODEM, 0x82, 0)  < 0) return -1;
    return 0;
}

void digi_fep_deinit_channel(pcbdcom_port_t *p, unsigned char chan)
{
    unsigned int seg = ((digi_fep_card_t *)p->backend_data)->card_seg;
    (void)digi_fep_cmd(seg, chan, SETMODEM, 0, 0x82);
    (void)digi_fep_cmd(seg, chan, PAUSERX, 0, 0);
    (void)digi_fep_cmd(seg, chan, PAUSETX, 0, 0);
}

/* ----- Shared ISR — event queue drain ----- *
 * Ported from doevent() in epca.c (line 1464). */
void digi_fep_isr(pcbdcom_port_t *p)
{
    digi_fep_card_t *card = (digi_fep_card_t *)p->backend_data;
    unsigned int seg, ein, eout, imax;
    unsigned char chan, event;
    pcbdcom_port_t *pp;

    if (!card) return;
    seg = card->card_seg;
    imax = digi_fep_readw(seg, GLOBAL + MB_IMAX);

    while ((eout = digi_fep_readw(seg, GLOBAL + MB_EOUT)) !=
           (ein  = digi_fep_readw(seg, GLOBAL + MB_EIN))) {

        chan  = digi_fep_readb(seg, ISTART + eout + 0);
        event = digi_fep_readb(seg, ISTART + eout + 1);

        if (chan < card->n_ports) {
            pp = card->ports[chan];

            if ((event & DATA_IND) && pp) {
                unsigned int rseg = digi_fep_readw(seg, BC_BASE(chan) + BC_RSEG);
                unsigned int rin  = digi_fep_readw(seg, BC_BASE(chan) + BC_RIN);
                unsigned int rout = digi_fep_readw(seg, BC_BASE(chan) + BC_ROUT);
                unsigned int rmax = digi_fep_readw(seg, BC_BASE(chan) + BC_RMAX);
                unsigned int next;
                unsigned char ch;

                while (rout != rin) {
                    ch = digi_fep_readb(seg, RXTXBUF + rseg + rout);
                    next = (pp->rx_head + 1) % pp->rx_size;
                    if (next != pp->rx_tail) {
                        pp->rx_buf[pp->rx_head] = ch;
                        pp->rx_head = next;
                    }
                    rout = (rout + 1) & (rmax - 1);
                }
                digi_fep_writew(seg, BC_BASE(chan) + BC_ROUT, rout);
            }

            if ((event & LOWTX_IND) && pp && pp->tx_head != pp->tx_tail) {
                unsigned int tseg = digi_fep_readw(seg, BC_BASE(chan) + BC_TSEG);
                unsigned int tin  = digi_fep_readw(seg, BC_BASE(chan) + BC_TIN);
                unsigned int tout = digi_fep_readw(seg, BC_BASE(chan) + BC_TOUT);
                unsigned int tmax = digi_fep_readw(seg, BC_BASE(chan) + BC_TMAX);
                unsigned int nextt;
                while (pp->tx_head != pp->tx_tail) {
                    nextt = (tin + 1) & (tmax - 1);
                    if (nextt == tout) break;
                    digi_fep_writeb(seg, RXTXBUF + tseg + tin,
                                    pp->tx_buf[pp->tx_tail]);
                    pp->tx_tail = (pp->tx_tail + 1) % pp->tx_size;
                    tin = nextt;
                }
                digi_fep_writew(seg, BC_BASE(chan) + BC_TIN, tin);
            }
        }
        eout = (eout + 4) & (imax - 4);
        digi_fep_writew(seg, GLOBAL + MB_EOUT, eout);
    }
}

/* Write to pcbdcom tx ring; LOWTX_IND flushes to FEP */
int digi_fep_write(pcbdcom_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbdcom_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

/* ----- Shared card pool for PC/Xe + AccelePort ----- */
#include "card_pool.h"
#define DIGI_MAX_CARDS 4
static digi_fep_card_t g_digi_cards[DIGI_MAX_CARDS];

void *digi_fep_card_get(unsigned long card_seg)
{
    digi_fep_card_t *c = (digi_fep_card_t *)
        card_pool_get(g_digi_cards, sizeof(digi_fep_card_t),
                      DIGI_MAX_CARDS, card_seg);
    if (c && c->card_seg == 0) {
        c->card_seg = (unsigned int)card_seg;
        c->n_ports = 16;     /* trimmed by probe */
    }
    return c;
}
