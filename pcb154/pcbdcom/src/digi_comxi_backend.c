/* ============================================================================
 * digi_comxi_backend.c — pcbdcom Digi COM/Xi (early 80186-based) backend
 *
 * Cards supported (v1):
 *   Digi COM/Xi 4       4 ports, 80186 on-board, dual-port RAM
 *   Digi COM/Xi 8       8 ports, same architecture
 *   PC/8i, PC/16i       compatible-family (same mailbox protocol)
 *
 * Design: COM/Xi is Digi's original intelligent multiport card from
 * 1988. It carries an on-board 80186 and 128-256 KB of dual-port RAM
 * mapped into the host's UMB address space (jumper-selected segment
 * 0xC000..0xE000). Host-to-board communication is entirely via
 * shared-memory mailbox structures — no I/O port programming.
 *
 * Unlike Digi's later PC/Xe / AccelePort cards (see digi_pcxe_backend.c
 * and digi_accel_backend.c), COM/Xi predates the FEP protocol. Its
 * mailbox layout is simpler: fixed offsets, no channel-descriptor
 * array indirection. That's why this backend is standalone rather than
 * sharing digi_fep.c with the newer Digi backends.
 *
 * Historical note: identified as COMMDV02.DRV in WCSC's COMMDRV.RED
 * (embedded string "DIGI-COMXI  1.00"). Very common on high-volume
 * BBSes in the late-'80s / early-'90s window before PC/Xe eclipsed it.
 *
 * Reference:
 *   - Digi International COM/Xi Owner's Manual (public, 1989)
 *   - Linux kernel history: drivers/char/digi/ prior to epca merge
 *   - NetBSD sys/dev/isa/dgb.c (BSD-licensed COM/Xi driver)
 *
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include "pcbdcom.h"
#include "backend.h"
#include "card_pool.h"

/* ----- COM/Xi shared-memory layout (all public, from Digi docs) ----- */

#define COMXI_SIG_OFF       0x0000L    /* 2-byte signature "DS" (0x53 0x44) */
#define COMXI_SIG_MAGIC     0x4453     /* little-endian 'D','S' as uint16   */

#define COMXI_NPORT_OFF     0x0002L    /* 1 byte: 4 or 8                    */
#define COMXI_BOARD_TYPE    0x0003L    /* 1 byte: 0x01=COMXi/4, 0x02=/8    */
#define COMXI_INT_STATUS    0x0004L    /* 1 byte: per-channel interrupt mask*/
#define COMXI_HOST_CMD      0x0006L    /* command mailbox (host -> board)   */
#define COMXI_BOARD_STAT    0x0008L    /* status mailbox   (board -> host)  */

/* Per-channel structure: 32 bytes starting at 0x0100, stride 0x20 */
#define COMXI_CHAN_BASE     0x0100L
#define COMXI_CHAN_STRIDE   0x0020L

/* Channel offsets (within one channel's 32-byte block) */
#define COMXI_CH_TXBUF      0x00       /* tx ring pointer (word)            */
#define COMXI_CH_TXHEAD     0x02
#define COMXI_CH_TXTAIL     0x04
#define COMXI_CH_RXBUF      0x08
#define COMXI_CH_RXHEAD     0x0A
#define COMXI_CH_RXTAIL     0x0C
#define COMXI_CH_MSR        0x10       /* modem status (CD/DSR/CTS/RI)      */
#define COMXI_CH_MCR        0x11       /* modem control (DTR/RTS)           */
#define COMXI_CH_STATUS     0x12       /* channel status bits               */

#define COMXI_MAX_CHANNELS  8

/* Per-card state (one card = one CardSeg dual-port RAM window). Multiple
 * ports on the same card share one comxi_card_t; parse_config() calls
 * comxi_card_get() with the card_seg address before wiring subports. */
typedef struct {
    unsigned long   addr;                        /* pool header: card_seg  */
    unsigned char   in_use;                      /* pool header            */
    unsigned int    card_seg;                    /* dual-port RAM segment  */
    unsigned char   nports;                      /* 4 or 8                 */
    pcbdcom_port_t *ports[COMXI_MAX_CHANNELS];   /* subport -> port slot   */
} comxi_card_t;

#define COMXI_MAX_CARDS 4
static comxi_card_t g_comxi_cards[COMXI_MAX_CARDS];

/* ----- Far-pointer helpers into dual-port RAM window ----- */

static unsigned char comxi_readb(unsigned int seg, unsigned long off)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void comxi_writeb(unsigned int seg, unsigned long off, unsigned char v)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

static unsigned int comxi_readw(unsigned int seg, unsigned long off)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void comxi_writew(unsigned int seg, unsigned long off, unsigned int v)
{
    unsigned int __far *pt = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *pt = v;
}

/* Compute far offset for a channel field */
#define COMXI_CH_OFF(subport, field) \
    (COMXI_CHAN_BASE + (unsigned long)(subport) * COMXI_CHAN_STRIDE + (field))

/* ----- Card pool hook ----- */

static void *comxi_card_get(unsigned long card_seg)
{
    comxi_card_t *c = (comxi_card_t *)
        card_pool_get(g_comxi_cards, sizeof(comxi_card_t),
                      COMXI_MAX_CARDS, card_seg);
    if (c && c->card_seg == 0) {
        c->card_seg = (unsigned int)card_seg;
        c->nports = 0;      /* filled in on first probe */
    }
    return c;
}

/* ----- Backend hooks ----- */

int digi_comxi_backend_probe(pcbdcom_port_t *p)
{
    comxi_card_t *card = (comxi_card_t *)p->backend_data;
    unsigned int sig, seg;
    unsigned char n;

    if (!card) return -1;
    seg = card->card_seg;

    /* Signature check: word at offset 0 must equal 'DS' */
    sig = comxi_readw(seg, COMXI_SIG_OFF);
    if (sig != COMXI_SIG_MAGIC) return -1;

    /* Port count sanity: 4 or 8 */
    n = comxi_readb(seg, COMXI_NPORT_OFF);
    if (n != 4 && n != 8) return -1;
    card->nports = n;

    /* Subport must be within reported channel count */
    if (p->subport >= n) return -1;

    return 0;
}

int digi_comxi_backend_init(pcbdcom_port_t *p)
{
    comxi_card_t *card = (comxi_card_t *)p->backend_data;

    if (digi_comxi_backend_probe(p) < 0) return -1;

    /* Assert DTR + RTS on channel open (standard behavior) */
    comxi_writeb(card->card_seg, COMXI_CH_OFF(p->subport, COMXI_CH_MCR), 0x03);

    /* Register subport for ISR dispatch */
    if (p->subport < COMXI_MAX_CHANNELS)
        card->ports[p->subport] = p;

    p->open = 1;
    return 0;
}

void digi_comxi_backend_deinit(pcbdcom_port_t *p)
{
    comxi_card_t *card = (comxi_card_t *)p->backend_data;

    if (!p->open || !card) return;

    /* Drop DTR/RTS on close */
    comxi_writeb(card->card_seg, COMXI_CH_OFF(p->subport, COMXI_CH_MCR), 0x00);

    if (p->subport < COMXI_MAX_CHANNELS)
        card->ports[p->subport] = 0;

    p->open = 0;
}

/* ISR: card raises host IRQ, board writes per-channel bits in INT_STATUS.
 * We read the mask, walk it, service each affected channel, then clear. */
void digi_comxi_backend_isr(pcbdcom_port_t *p)
{
    comxi_card_t *card = (comxi_card_t *)p->backend_data;
    unsigned char mask;
    unsigned int i;

    if (!card) return;

    mask = comxi_readb(card->card_seg, COMXI_INT_STATUS);
    if (mask == 0) return;

    /* Channel bits in `mask` indicate which sub-ports have activity.
     * COM/Xi's dual-port RAM makes ISR trivial: the ring pointers on
     * the board side advance autonomously, so we just acknowledge the
     * interrupt. The upper layer picks up new data on the next call to
     * digi_comxi_backend_read() which reads the ring pointers directly.
     *
     * Traverse `mask` for symmetry with other multi-channel backends;
     * a future optimization could push data into per-port rings here to
     * reduce read-side latency. */
    for (i = 0; i < card->nports && i < COMXI_MAX_CHANNELS; i++) {
        if ((mask & (1 << i)) && card->ports[i]) {
            /* Channel i has pending activity — no per-port state to
             * update; ring pointers on the card side already reflect
             * the new data. */
            (void)card->ports[i];
        }
    }

    /* Clear serviced bits by writing them back */
    comxi_writeb(card->card_seg, COMXI_INT_STATUS, 0x00);
}

int digi_comxi_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    comxi_card_t *card = (comxi_card_t *)p->backend_data;
    unsigned char *dst = (unsigned char *)buf;
    unsigned int head, tail, avail, i;
    unsigned long rxbuf_off;

    if (!card || !p->open) return -1;

    head = comxi_readw(card->card_seg, COMXI_CH_OFF(p->subport, COMXI_CH_RXHEAD));
    tail = comxi_readw(card->card_seg, COMXI_CH_OFF(p->subport, COMXI_CH_RXTAIL));

    /* Bytes available in ring */
    avail = (head >= tail) ? (head - tail) : (0x1000 - tail + head);
    if (avail > (unsigned)n) avail = n;
    if (avail == 0) return 0;

    rxbuf_off = comxi_readw(card->card_seg,
                            COMXI_CH_OFF(p->subport, COMXI_CH_RXBUF));

    /* Copy from ring, respecting wrap */
    for (i = 0; i < avail; i++) {
        dst[i] = comxi_readb(card->card_seg,
                             rxbuf_off + ((tail + i) & 0x0FFF));
    }

    /* Advance tail */
    tail = (tail + avail) & 0x0FFF;
    comxi_writew(card->card_seg,
                 COMXI_CH_OFF(p->subport, COMXI_CH_RXTAIL), tail);

    return (int)avail;
}
int digi_comxi_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    comxi_card_t *card = (comxi_card_t *)p->backend_data;
    const unsigned char *src = (const unsigned char *)buf;
    unsigned int head, tail, room, i;
    unsigned long txbuf_off;

    if (!card || !p->open) return -1;

    head = comxi_readw(card->card_seg, COMXI_CH_OFF(p->subport, COMXI_CH_TXHEAD));
    tail = comxi_readw(card->card_seg, COMXI_CH_OFF(p->subport, COMXI_CH_TXTAIL));

    /* Free space in ring (leave 1 byte guard) */
    room = (tail > head) ? (tail - head - 1) : (0x1000 - head + tail - 1);
    if (room > (unsigned)n) room = n;
    if (room == 0) return 0;

    txbuf_off = comxi_readw(card->card_seg,
                            COMXI_CH_OFF(p->subport, COMXI_CH_TXBUF));

    for (i = 0; i < room; i++) {
        comxi_writeb(card->card_seg,
                     txbuf_off + ((head + i) & 0x0FFF), src[i]);
    }

    head = (head + room) & 0x0FFF;
    comxi_writew(card->card_seg,
                 COMXI_CH_OFF(p->subport, COMXI_CH_TXHEAD), head);

    return (int)room;
}

/* ----- Backend descriptor exported to dispatcher ----- */

const pcbdcom_backend_t pcbdcom_digi_comxi_backend = {
    "DIGI_COMXI",
    comxi_card_get,
    digi_comxi_backend_probe,
    digi_comxi_backend_init,
    digi_comxi_backend_deinit,
    digi_comxi_backend_isr,
    digi_comxi_backend_read,
    digi_comxi_backend_write
};

/* end digi_comxi_backend.c */
