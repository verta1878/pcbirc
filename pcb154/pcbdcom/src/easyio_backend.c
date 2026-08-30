/* ============================================================================
 * easyio_backend.c — pcbdcom Stallion EasyIO backend
 *
 * Cards supported (v1, ISA):
 *   EasyIO 4RS (4-port RS-232)   — EIO_4PORTRS  = 0x05
 *   EasyIO 8DI (8-port modular)  — EIO_8PORTDI  = 0x00
 *   EasyIO 8RS (8-port RS-232)   — EIO_8PORTRS  = 0x04
 *   EasyIO 8M  (8-port modem, 20MHz clock) — EIO_8PORTM = 0x06
 *   EasyIO MK3 (rev 3 revision)  — EIO_MK3      = 0x03
 *
 * Not supported here: EasyConnection 8/32 (ECH) — uses SC26198 UARTs
 * and different card architecture. Would be a separate ech_backend.c.
 *
 * Ported from Linux drivers/char/stallion.c (kernel 2.6.32), GPLv2.
 * Original authors: Stallion Technologies, Greg Ungerer.
 * DOS 16-bit adaptations: pcbirc crew (hexadecimal), GPLv3.
 *
 * Architecture:
 *   EasyIO uses Cirrus Logic CD1400 quad-UARTs (SAME chip family as
 *   Cyclades Cyclom-Y) but I/O-port-mapped instead of memory-mapped.
 *   Board has status byte at (ioaddr+2 or +3 for MK3) that reports
 *   card model and interrupt pending. CD1400 channels accessed at
 *   documented offsets from ioaddr.
 *
 * Adaptations from Linux stallion.c:
 *   - stl_eiointr() → easyio_backend_isr(): poll iostatus for
 *     EIO_INTRPEND, service each CD1400 channel via SVRR walk (same
 *     path as cyclom_backend.c but I/O-port instead of memory-mapped).
 *   - I/O port access: inp/outp instead of __far pointer.
 *   - Removed struct stlbrd/stlpanel/stlport plumbing; talks directly
 *     to pcbdcom_port_t via rx_buf/tx_buf rings.
 *   - CD1400 register bit constants same as cyclom_backend.c — the
 *     chip is identical, only bus access differs.
 * ==========================================================================*/

#include <conio.h>
#include "pcbdcom.h"
#include "backend.h"

#if defined(_MSC_VER)
# define EIO_IN(port)      (unsigned char)_inp((port))
# define EIO_OUT(port,val) _outp((port),(val))
#else
# define EIO_IN(port)      (unsigned char)inp((port))
# define EIO_OUT(port,val) outp((port),(val))
#endif

/* ----- EasyIO board detect + control registers ----- *
 * At ioaddr+1 = control (interrupt config); +2 = status (id + int pending);
 * MK3 revision moves control to +2 and status to +3. */
#define EIO_CTRL_OFF     1
#define EIO_STAT_OFF     2
#define EIO_CTRL_OFF_MK3 2
#define EIO_STAT_OFF_MK3 3

/* Status byte fields */
#define EIO_IDBITMASK    0x07
#define EIO_8PORTDI      0x00
#define EIO_MK3          0x03
#define EIO_8PORTRS      0x04
#define EIO_4PORTRS      0x05
#define EIO_8PORTM       0x06
#define EIO_INTRPEND     0x08

/* Control byte fields */
#define EIO_INTEDGE      0x00
#define EIO_INTLEVEL     0x08
#define EIO_0WS          0x10       /* zero wait states */

/* ----- CD1400 register offsets (I/O relative to ioaddr) ----- *
 * Stallion EasyIO uses linear I/O addressing, no *2 multiplier
 * (contrast with Cyclades memory-mapped which uses *2). */
#define CD_GFRCR    0x40    /* Global firmware revision */
#define CD_CAR      0x68    /* Channel access reg */
#define CD_SVRR     0x67    /* Service request reg */
#define CD_RIR      0x6B    /* RX interrupt reg */
#define CD_TIR      0x6A    /* TX interrupt reg */
#define CD_MIR      0x69    /* Modem interrupt reg */
#define CD_RIVR     0x43    /* RX interrupt vector */
#define CD_TIVR     0x42    /* TX interrupt vector */
#define CD_MIVR     0x41    /* Modem interrupt vector */
#define CD_RDSR     0x62    /* RX data/status */
#define CD_TDR      0x63    /* TX data */
#define CD_RDCR     0x0E    /* RX data count */
#define CD_MISR     0x4C    /* Modem interrupt status */
#define CD_CCR      0x05    /* Channel command */
#define CD_SRER     0x06    /* Service request enable */
#define CD_COR1     0x08    /* Channel option 1 */
#define CD_COR2     0x09
#define CD_COR3     0x0A
#define CD_TBPR     0x72    /* TX baud prescaler */
#define CD_RBPR     0x78    /* RX baud prescaler */
#define CD_TCOR     0x76    /* TX clock option */
#define CD_RCOR     0x7C    /* RX clock option */

/* Interrupt vector bit fields (same as cyclades.h) */
#define CD_IVRMask   0x07
#define CD_IVRRxOK   0x03
#define CD_IVRTxOK   0x02
#define CD_IVRMdmOK  0x01

/* SRER bits */
#define CD_MdmCh     0x80
#define CD_RxData    0x10
#define CD_TxRdy     0x04

/* CCR commands */
#define CD_CHAN_RESET     0x80
#define CD_CHAN_CTL       0x10
#define CD_ENB_RCVR       0x02
#define CD_ENB_XMTR       0x08

/* Line format */
#define CD_8_BITS         0x03
#define CD_1_STOP         0x00
#define CD_PARITY_NONE    0x00

/* ----- Per-card state ----- */
typedef struct {
    unsigned long  addr;           /* pool: ioaddr                           */
    unsigned char  in_use;         /* pool                                   */
    unsigned int   ioaddr;         /* Card I/O base                          */
    unsigned int   iostatus;       /* Status port (ioaddr + 2 or +3 for MK3) */
    unsigned int   ioctrl;         /* Control port                           */
    unsigned char  n_chans;        /* 4 or 8 depending on model              */
    unsigned char  clock_20mhz;    /* Non-zero for EIO_8PORTM (20 MHz)       */
    pcbdcom_port_t *chans[8];
} easyio_card_t;

#include "card_pool.h"
#define EASYIO_MAX_CARDS 4
static easyio_card_t g_easyio_cards[EASYIO_MAX_CARDS];

static void *easyio_card_get(unsigned long ioaddr)
{
    easyio_card_t *c = (easyio_card_t *)
        card_pool_get(g_easyio_cards, sizeof(easyio_card_t),
                      EASYIO_MAX_CARDS, ioaddr);
    if (c && c->ioaddr == 0)
        c->ioaddr = (unsigned int)ioaddr;
    return c;
}

/* Select channel on the CD1400: write chan# to CAR. All subsequent
 * register accesses refer to that channel. */
static void cd_select(unsigned int io, unsigned char chan)
{
    EIO_OUT(io + CD_CAR, chan & 0x03);
}

/* Baud programming: same tables as cyclom_backend.c (CD1400 25MHz clock) */
static void cd_set_baud(unsigned int io, long baud)
{
    unsigned char tcor, tbpr;
    switch (baud) {
        case 300:    tcor = 0x08; tbpr = 0xCF; break;
        case 1200:   tcor = 0x08; tbpr = 0x34; break;
        case 2400:   tcor = 0x08; tbpr = 0x1A; break;
        case 9600:   tcor = 0x03; tbpr = 0x82; break;
        case 19200:  tcor = 0x02; tbpr = 0x82; break;
        case 38400:  tcor = 0x02; tbpr = 0x41; break;
        case 57600:  tcor = 0x02; tbpr = 0x2C; break;
        case 115200: tcor = 0x02; tbpr = 0x16; break;
        default:     tcor = 0x02; tbpr = 0x41; break;
    }
    EIO_OUT(io + CD_TCOR, tcor);
    EIO_OUT(io + CD_TBPR, tbpr);
    EIO_OUT(io + CD_RCOR, tcor);
    EIO_OUT(io + CD_RBPR, tbpr);
}

/* ----- Backend hooks ----- */

int easyio_backend_probe(pcbdcom_port_t *p)
{
    easyio_card_t *card = (easyio_card_t *)p->backend_data;
    unsigned char status, gfrcr;
    unsigned char id;

    if (!card) return -1;

    /* Read board status; determine MK3 revision (moves ports up by 1) */
    status = EIO_IN(card->ioaddr + EIO_STAT_OFF);
    id = status & EIO_IDBITMASK;
    if (id == EIO_MK3) {
        card->iostatus = card->ioaddr + EIO_STAT_OFF_MK3;
        card->ioctrl   = card->ioaddr + EIO_CTRL_OFF_MK3;
        /* Re-read from MK3 offset */
        status = EIO_IN(card->iostatus);
        id = status & EIO_IDBITMASK;
    } else {
        card->iostatus = card->ioaddr + EIO_STAT_OFF;
        card->ioctrl   = card->ioaddr + EIO_CTRL_OFF;
    }

    /* Verify one of the recognized IDs */
    switch (id) {
        case EIO_4PORTRS: card->n_chans = 4; break;
        case EIO_8PORTDI:
        case EIO_8PORTRS: card->n_chans = 8; break;
        case EIO_8PORTM:  card->n_chans = 8; card->clock_20mhz = 1; break;
        default:          return -1;    /* not an EasyIO */
    }

    /* Verify CD1400 firmware revision valid (0x40..0x4F) */
    gfrcr = EIO_IN(card->ioaddr + CD_GFRCR);
    return (gfrcr >= 0x40 && gfrcr < 0x50) ? 0 : -1;
}

int easyio_backend_init(pcbdcom_port_t *p)
{
    easyio_card_t *card = (easyio_card_t *)p->backend_data;
    unsigned int io;
    unsigned char chan;

    if (easyio_backend_probe(p) < 0) return -1;
    io = card->ioaddr;
    chan = p->subport;   /* 0..7 for 8-port cards, 0..3 for 4-port */
    if (p->subport < 8) card->chans[p->subport] = p;

    /* Configure control register: level-triggered IRQ + 0 wait states */
    EIO_OUT(card->ioctrl, EIO_0WS | EIO_INTLEVEL);

    /* Per-channel init */
    cd_select(io, chan);
    EIO_OUT(io + CD_CCR, CD_CHAN_RESET);
    while (EIO_IN(io + CD_CCR) != 0) ;   /* wait for reset done */

    EIO_OUT(io + CD_COR1, CD_8_BITS | CD_1_STOP | CD_PARITY_NONE);
    EIO_OUT(io + CD_COR2, 0x00);
    EIO_OUT(io + CD_COR3, 0x01);

    cd_set_baud(io, p->baud);

    /* Enable RX + modem interrupts (TX enabled on demand) */
    EIO_OUT(io + CD_SRER, CD_RxData | CD_MdmCh);

    /* Enable channel */
    EIO_OUT(io + CD_CCR, CD_CHAN_CTL | CD_ENB_RCVR | CD_ENB_XMTR);

    p->open = 1;
    return 0;
}

void easyio_backend_deinit(pcbdcom_port_t *p)
{
    easyio_card_t *card = (easyio_card_t *)p->backend_data;
    if (!p->open || !card) return;
    cd_select(card->ioaddr, p->subport);
    EIO_OUT(card->ioaddr + CD_CCR, CD_CHAN_CTL);
    EIO_OUT(card->ioaddr + CD_SRER, 0);
    p->open = 0;
}

/* ----- ISR ----- *
 * Ported from stl_eiointr() + panelp->isr in stallion.c: poll board
 * status for EIO_INTRPEND, then walk SVRR on CD1400 to service. */
void easyio_backend_isr(pcbdcom_port_t *p)
{
    easyio_card_t *card = (easyio_card_t *)p->backend_data;
    unsigned int io;
    unsigned char svrr, save_car, save_xir, ivr, chan, ch, count, i, srer;
    unsigned int next;
    pcbdcom_port_t *pp;

    if (!card) return;
    io = card->ioaddr;

    /* Loop while board reports pending interrupt */
    while (EIO_IN(card->iostatus) & EIO_INTRPEND) {

        svrr = EIO_IN(io + CD_SVRR);
        if (svrr == 0) break;

        save_car = EIO_IN(io + CD_CAR);

        /* ---- RX service (bit 2) ---- */
        if (svrr & 0x04) {
            save_xir = EIO_IN(io + CD_RIR);
            chan = save_xir & 0x03;
            EIO_OUT(io + CD_CAR, chan);
            ivr = EIO_IN(io + CD_RIVR) & CD_IVRMask;

            if (ivr == CD_IVRRxOK) {
                pp = card->chans[chan];
                count = EIO_IN(io + CD_RDCR);
                for (i = 0; i < count && pp && pp->open; i++) {
                    ch = EIO_IN(io + CD_RDSR);
                    next = (pp->rx_head + 1) % pp->rx_size;
                    if (next != pp->rx_tail) {
                        pp->rx_buf[pp->rx_head] = ch;
                        pp->rx_head = next;
                    }
                }
            } else {
                /* RX exception — drain + discard for v1 */
                (void)EIO_IN(io + CD_RDSR);
            }
            EIO_OUT(io + CD_RIR, save_xir & 0x3F);
        }

        /* ---- TX service (bit 1) ---- */
        if (svrr & 0x02) {
            save_xir = EIO_IN(io + CD_TIR);
            chan = save_xir & 0x03;
            EIO_OUT(io + CD_CAR, chan);
            pp = card->chans[chan];
            for (i = 0; i < 12 && pp && pp->open &&
                        pp->tx_head != pp->tx_tail; i++) {
                EIO_OUT(io + CD_TDR, pp->tx_buf[pp->tx_tail]);
                pp->tx_tail = (pp->tx_tail + 1) % pp->tx_size;
            }
            if (pp && pp->tx_head == pp->tx_tail) {
                srer = EIO_IN(io + CD_SRER);
                EIO_OUT(io + CD_SRER, srer & ~CD_TxRdy);
            }
            EIO_OUT(io + CD_TIR, save_xir & 0x3F);
        }

        /* ---- Modem status (bit 0) ---- */
        if (svrr & 0x01) {
            save_xir = EIO_IN(io + CD_MIR);
            chan = save_xir & 0x03;
            EIO_OUT(io + CD_CAR, chan);
            (void)EIO_IN(io + CD_MISR);
            EIO_OUT(io + CD_MIR, save_xir & 0x3F);
        }

        /* Restore CAR */
        EIO_OUT(io + CD_CAR, save_car);
    }
}

int easyio_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int easyio_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    easyio_card_t *card = (easyio_card_t *)p->backend_data;
    const unsigned char *in = (const unsigned char *)buf;
    unsigned int next;
    int i;
    int was_empty = (p->tx_head == p->tx_tail);
    unsigned char srer;

    for (i = 0; i < n; i++) {
        next = (p->tx_head + 1) % p->tx_size;
        if (next == p->tx_tail) break;
        p->tx_buf[p->tx_head] = in[i];
        p->tx_head = next;
    }

    /* Kick TX interrupt if ring was empty */
    if (was_empty && i > 0 && card) {
        cd_select(card->ioaddr, p->subport);
        srer = EIO_IN(card->ioaddr + CD_SRER);
        EIO_OUT(card->ioaddr + CD_SRER, srer | CD_TxRdy);
    }
    return i;
}

const pcbdcom_backend_t pcbdcom_easyio_backend = {
    "EASYIO",
    easyio_card_get,
    easyio_backend_probe,
    easyio_backend_init,
    easyio_backend_deinit,
    easyio_backend_isr,
    easyio_backend_read,
    easyio_backend_write
};

/* ----- v1.1 TODO -----
 *  1. Multi-channel wiring: v1 assumes chan=0. parse_config passes
 *     ioaddr via backend_data; needs easyio_card_t* with chans[]
 *     table populated at load. Matches TODO for cyclom/digi/rocket.
 *  2. EIO_8PORTM 20 MHz clock: baud table needs alternate values
 *     when card->clock_20mhz is set (Linux uses different sc26198_
 *     baudtable[] for that variant — check stl_setport calls).
 *  3. RX error tagging (parity/framing/break) — v1 drops.
 *  4. ECH-family (EasyConnection 8/32) support — separate backend
 *     because ECH uses SC26198, not CD1400.
 * ---------------------------------------------------------------------- */
