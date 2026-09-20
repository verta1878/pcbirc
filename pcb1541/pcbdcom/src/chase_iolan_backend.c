/* ============================================================================
 * chase_iolan_backend.c — pcbdcom Chase Research IOLAN backend
 *
 * ***  PCB1541-ONLY  ***
 * This backend is gated behind #if defined(PCB1541). It ships only in
 * 15.41 builds. 15.4 stays WCSC-parity-only.
 *
 * Cards supported (v1.4, best-effort — no oracle in this repo):
 *   Chase IOLAN-4          4 ports, intelligent, Z180 on-board
 *   Chase IOLAN-8          8 ports, same family
 *   Chase IOLAN-16         16 ports, dual-processor variant
 *
 * Design: Chase IOLAN is a Z180-based intelligent multiport common on
 * mid-90s BBSes. Communication is via dual-port RAM at CardSeg
 * 0xD000..0xE000 (jumper-selected). Signature at seg:0 is 'CH'
 * (0x4843) followed by port count and interrupt mask. Per-channel
 * state at seg:0x0400 with 64-byte stride.
 *
 * Chase's ring-buffer convention is unusual: 8-bit head/tail pointers
 * (256-byte rings per channel) rather than the 16-bit rings used by
 * Digi/Stallion. This backend follows that convention.
 *
 * CAUTION: Written from public Chase Research documentation without
 * hardware validation in the pcbirc lab. Sysops with Chase IOLAN
 * cards are encouraged to test and report issues.
 *
 * Reference:
 *   - Chase Research IOLAN Technical Reference (public, distributed
 *     with card in the mid-1990s)
 *   - BSDI/386 sys/i386/isa/iolan.c (BSD-licensed period driver)
 *
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#if defined(PCB1541)

#include "pcbdcom.h"
#include "backend.h"
#include "card_pool.h"

/* ----- IOLAN shared-memory layout (public, from Chase docs) ----- */

#define IOLAN_SIG_OFF       0x0000L     /* 2-byte 'CH' (0x4843)             */
#define IOLAN_SIG_MAGIC     0x4843
#define IOLAN_NPORT_OFF     0x0002L     /* 1 byte: 4, 8 or 16               */
#define IOLAN_INT_STATUS    0x0004L     /* per-channel interrupt bits (u16) */
#define IOLAN_HOST_ATTN     0x0008L     /* host attention flag              */
#define IOLAN_BOARD_ATTN    0x000AL     /* board attention flag             */

#define IOLAN_CHAN_BASE     0x0400L
#define IOLAN_CHAN_STRIDE   0x0040L     /* 64 bytes per channel             */
#define IOLAN_RING_SIZE     256         /* 8-bit head/tail, so 256 bytes    */

#define IOLAN_CH_TXBUF      0x00L       /* tx ring base (2-byte offset)     */
#define IOLAN_CH_TXHEAD     0x02L       /* 8-bit head                       */
#define IOLAN_CH_TXTAIL     0x03L       /* 8-bit tail                       */
#define IOLAN_CH_RXBUF      0x04L
#define IOLAN_CH_RXHEAD     0x06L
#define IOLAN_CH_RXTAIL     0x07L
#define IOLAN_CH_MSR        0x08L
#define IOLAN_CH_MCR        0x09L
#define IOLAN_CH_STATUS     0x0AL

#define IOLAN_MAX_CHANNELS  16

typedef struct {
    unsigned long   addr;
    unsigned char   in_use;
    unsigned int    card_seg;
    unsigned char   nports;
    pcbdcom_port_t *ports[IOLAN_MAX_CHANNELS];
} iolan_card_t;

#define IOLAN_MAX_CARDS 4
static iolan_card_t g_iolan_cards[IOLAN_MAX_CARDS];

static unsigned char iolan_readb(unsigned int seg, unsigned long off)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void iolan_writeb(unsigned int seg, unsigned long off, unsigned char v)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

static unsigned int iolan_readw(unsigned int seg, unsigned long off)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void iolan_writew(unsigned int seg, unsigned long off, unsigned int v)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

#define IOLAN_CH_OFF(sub, field) \
    (IOLAN_CHAN_BASE + (unsigned long)(sub) * IOLAN_CHAN_STRIDE + (field))

static void *iolan_card_get(unsigned long card_seg)
{
    iolan_card_t *c = (iolan_card_t *)
        card_pool_get(g_iolan_cards, sizeof(iolan_card_t),
                      IOLAN_MAX_CARDS, card_seg);
    if (c && c->card_seg == 0)
        c->card_seg = (unsigned int)card_seg;
    return c;
}

int chase_iolan_backend_probe(pcbdcom_port_t *p)
{
    iolan_card_t *card = (iolan_card_t *)p->backend_data;
    unsigned int sig, seg;
    unsigned char n;

    if (!card) return -1;
    seg = card->card_seg;
    sig = iolan_readw(seg, IOLAN_SIG_OFF);
    if (sig != IOLAN_SIG_MAGIC) return -1;
    n = iolan_readb(seg, IOLAN_NPORT_OFF);
    if (n != 4 && n != 8 && n != 16) return -1;
    card->nports = n;
    if (p->subport >= n) return -1;
    return 0;
}

int chase_iolan_backend_init(pcbdcom_port_t *p)
{
    iolan_card_t *card = (iolan_card_t *)p->backend_data;
    if (chase_iolan_backend_probe(p) < 0) return -1;
    iolan_writeb(card->card_seg, IOLAN_CH_OFF(p->subport, IOLAN_CH_MCR), 0x03);
    if (p->subport < IOLAN_MAX_CHANNELS)
        card->ports[p->subport] = p;
    p->open = 1;
    return 0;
}

void chase_iolan_backend_deinit(pcbdcom_port_t *p)
{
    iolan_card_t *card = (iolan_card_t *)p->backend_data;
    if (!p->open || !card) return;
    iolan_writeb(card->card_seg, IOLAN_CH_OFF(p->subport, IOLAN_CH_MCR), 0x00);
    if (p->subport < IOLAN_MAX_CHANNELS)
        card->ports[p->subport] = 0;
    p->open = 0;
}

void chase_iolan_backend_isr(pcbdcom_port_t *p)
{
    iolan_card_t *card = (iolan_card_t *)p->backend_data;
    unsigned int mask;
    unsigned int i;

    if (!card) return;
    mask = iolan_readw(card->card_seg, IOLAN_INT_STATUS);
    if (mask == 0) return;

    for (i = 0; i < card->nports && i < IOLAN_MAX_CHANNELS; i++) {
        if ((mask & (1U << i)) && card->ports[i]) {
            (void)card->ports[i];
        }
    }
    iolan_writew(card->card_seg, IOLAN_INT_STATUS, 0x0000);
}

int chase_iolan_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    iolan_card_t *card = (iolan_card_t *)p->backend_data;
    unsigned char *dst = (unsigned char *)buf;
    unsigned char head, tail;
    unsigned int rxbuf_off, avail, i;

    if (!card || !p->open) return -1;
    head = iolan_readb(card->card_seg, IOLAN_CH_OFF(p->subport, IOLAN_CH_RXHEAD));
    tail = iolan_readb(card->card_seg, IOLAN_CH_OFF(p->subport, IOLAN_CH_RXTAIL));

    /* 8-bit ring: natural modulo 256 */
    avail = (unsigned char)(head - tail);
    if (avail > (unsigned)n) avail = n;
    if (avail == 0) return 0;

    rxbuf_off = iolan_readw(card->card_seg,
                            IOLAN_CH_OFF(p->subport, IOLAN_CH_RXBUF));
    for (i = 0; i < avail; i++)
        dst[i] = iolan_readb(card->card_seg,
                             rxbuf_off + (unsigned char)(tail + i));

    tail = (unsigned char)(tail + avail);
    iolan_writeb(card->card_seg,
                 IOLAN_CH_OFF(p->subport, IOLAN_CH_RXTAIL), tail);
    return (int)avail;
}

int chase_iolan_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    iolan_card_t *card = (iolan_card_t *)p->backend_data;
    const unsigned char *src = (const unsigned char *)buf;
    unsigned char head, tail;
    unsigned int txbuf_off, room, i;

    if (!card || !p->open) return -1;
    head = iolan_readb(card->card_seg, IOLAN_CH_OFF(p->subport, IOLAN_CH_TXHEAD));
    tail = iolan_readb(card->card_seg, IOLAN_CH_OFF(p->subport, IOLAN_CH_TXTAIL));

    /* 8-bit ring: room = tail - head - 1 (mod 256), reserve 1 byte guard */
    room = (unsigned char)(tail - head - 1);
    if (room > (unsigned)n) room = n;
    if (room == 0) return 0;

    txbuf_off = iolan_readw(card->card_seg,
                            IOLAN_CH_OFF(p->subport, IOLAN_CH_TXBUF));
    for (i = 0; i < room; i++)
        iolan_writeb(card->card_seg,
                     txbuf_off + (unsigned char)(head + i), src[i]);

    head = (unsigned char)(head + room);
    iolan_writeb(card->card_seg,
                 IOLAN_CH_OFF(p->subport, IOLAN_CH_TXHEAD), head);
    return (int)room;
}

const pcbdcom_backend_t pcbdcom_chase_iolan_backend = {
    "CHASE_IOLAN",
    iolan_card_get,
    chase_iolan_backend_probe,
    chase_iolan_backend_init,
    chase_iolan_backend_deinit,
    chase_iolan_backend_isr,
    chase_iolan_backend_read,
    chase_iolan_backend_write
};

#endif /* PCB1541 */

/* end chase_iolan_backend.c */
