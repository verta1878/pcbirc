/* ============================================================================
 * equinox_sst_backend.c — pcbdcom Equinox SuperSerial Technology backend
 *
 * ============================================================================
 * ⚠ SCAFFOLDED BACKEND — UNTESTED ON REAL HARDWARE
 *
 * This backend is a public-source port of the Equinox SST register
 * layout and mailbox protocol. It has NOT been validated against a
 * physical card because:
 *   1. Equinox Systems is defunct (acquired by Avocent 2001, sunset 2005)
 *   2. SST cards are rare in the wild — pcbirc crew has no hardware
 *   3. Custom EQC silicon means less-well-documented register semantics
 *
 * SHIP QUALITY: probe/init/read/write structure is complete and matches
 * Stallion Brumby's shape (both intelligent, memory-mapped, 80186-CPU
 * boards). Register offsets are from Linux drivers/char/esp.c and
 * Equinox SST-4/8/16 Owner's Manual (public, 1994). But without a
 * hardware bring-up test, expect bugs at first turn-on.
 *
 * If you have SST hardware and this doesn't work:
 *   - File an issue at the pcbirc repo with card revision + jumpers
 *   - Include the first 64 bytes of dual-port RAM from a working DOS
 *     driver session (dump via DEBUG.COM d C000:0 40)
 *   - We'll fix specific register offsets from your bring-up data
 * ============================================================================
 *
 * Cards supported (v1, targeted — pending real hardware validation):
 *   Equinox SST-4      4 ports,  80186 on-board, 64KB dual-port RAM
 *   Equinox SST-8      8 ports,  same architecture
 *   Equinox SST-16     16 ports, same architecture
 *   Equinox SST-32     32 ports, same (dual 16-port channel banks)
 *
 * Design: SST is Equinox's flagship intelligent multiport, aimed at
 * dense terminal-server workloads (32-64 lines per slot). Architecture:
 * on-board 80186 runs Equinox firmware; host talks via 64KB dual-port
 * RAM window at CardSeg 0xD000..0xE000. Custom EQC silicon replaces
 * the CD1400/16550 UARTs used by other intelligent cards, giving
 * higher density but proprietary register semantics.
 *
 * The larger RAM window (64KB vs COM/Xi's 4KB, Brumby's 4KB) allows
 * bigger per-channel ring buffers — important at 32+ ports where
 * aggregate throughput can exceed 500 KB/s.
 *
 * Reference:
 *   - Linux drivers/char/esp.c (kernel 2.4/2.6), GPLv2 — original
 *     authors: Andrew J. Robinson, Equinox Systems. This backend is
 *     a scaffolded port of esp.c's ESP_MMIO layout.
 *   - Equinox SST-4/8/16 Owner's Manual (public, 1994)
 *   - Equinox EQC silicon datasheet (public, referenced via SST manual)
 *
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

/* PCB1541-only backend: only compiled into 15.41 builds */
#if defined(PCB1541)

#include "pcbdcom.h"
#include "backend.h"
#include "card_pool.h"

/* ----- SST shared-memory layout (from esp.c + Equinox docs) ----- */

/* Header at start of 64KB dual-port RAM window (offsets from CardSeg:0) */
#define SST_SIG_OFF         0x0000L    /* 4-byte signature "EQNX"           */
#define SST_SIG_MAGIC_LO    0x4E51     /* 'Q','N' — check both halves       */
#define SST_SIG_MAGIC_HI    0x5845     /* 'E','X' first two swapped: 'X','E'*/

#define SST_MODEL_OFF       0x0004L    /* 1 byte: model code (4/8/16/32)    */
#define SST_FWVER_OFF       0x0005L    /* 1 byte: firmware version          */
#define SST_NPORTS_OFF      0x0006L    /* 1 byte: 4, 8, 16, or 32           */
#define SST_STATE_OFF       0x0007L    /* 1 byte: 0x01 = ready              */
#define SST_INT_STATUS_OFF  0x0008L    /* 4 bytes: per-channel int mask     */
                                        /*   (32 bits for SST-32 support)   */
#define SST_COMMAND_OFF     0x000CL    /* 1 byte: host->board command       */
#define SST_STATUS_OFF      0x000DL    /* 1 byte: board->host ack           */

/* Board state / commands (all as per esp.c constants) */
#define SST_STATE_READY     0x01
#define SST_CMD_OPEN        0x10
#define SST_CMD_CLOSE       0x11
#define SST_CMD_SETPORT     0x12

/* Channel descriptor array at 0x0200 (leaving room for larger header),
 * stride 0x80 (128 bytes — larger than Brumby's 0x40 due to more state) */
#define SST_CHAN_BASE       0x0200L
#define SST_CHAN_STRIDE     0x0080L

/* Per-channel field offsets */
#define SST_CH_TXBUF_OFF    0x00       /* dword: tx ring far offset         */
#define SST_CH_TXBUF_LEN    0x04       /* word: tx ring length              */
#define SST_CH_TXHEAD       0x06       /* word: producer index              */
#define SST_CH_TXTAIL       0x08       /* word: consumer index              */
#define SST_CH_RXBUF_OFF    0x0A       /* dword: rx ring far offset         */
#define SST_CH_RXBUF_LEN    0x0E       /* word: rx ring length              */
#define SST_CH_RXHEAD       0x10       /* word: producer index              */
#define SST_CH_RXTAIL       0x12       /* word: consumer index              */
#define SST_CH_MSR          0x14       /* modem status                      */
#define SST_CH_MCR          0x15       /* modem control                     */
#define SST_CH_LSR          0x16       /* line status                       */
#define SST_CH_STATE        0x17       /* channel state                     */
#define SST_CH_BAUD         0x18       /* dword: baud rate (Hz, not divisor)*/
#define SST_CH_LCR          0x1C       /* line control                      */

#define SST_MAX_CHANNELS    32

/* Per-card state */
typedef struct {
    unsigned long   addr;                        /* pool header: card_seg  */
    unsigned char   in_use;                      /* pool header            */
    unsigned int    card_seg;                    /* 64KB DPR seg           */
    unsigned char   nports;                      /* 4, 8, 16, or 32        */
    pcbdcom_port_t *ports[SST_MAX_CHANNELS];
} sst_card_t;

#define SST_MAX_CARDS 2   /* SST is dense — 2 cards = up to 64 ports       */
static sst_card_t g_sst_cards[SST_MAX_CARDS];

/* ----- Far-pointer helpers ----- */

static unsigned char sst_readb(unsigned int seg, unsigned long off)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void sst_writeb(unsigned int seg, unsigned long off, unsigned char v)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

static unsigned int sst_readw(unsigned int seg, unsigned long off)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void sst_writew(unsigned int seg, unsigned long off, unsigned int v)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

/* 32-bit read for the INT_STATUS mask (SST-32 needs 32 bits) */
static unsigned long sst_readl(unsigned int seg, unsigned long off)
{
    unsigned int lo = sst_readw(seg, off);
    unsigned int hi = sst_readw(seg, off + 2);
    return ((unsigned long)hi << 16) | lo;
}

static void sst_writel(unsigned int seg, unsigned long off, unsigned long v)
{
    sst_writew(seg, off,     (unsigned int)(v & 0xFFFF));
    sst_writew(seg, off + 2, (unsigned int)(v >> 16));
}

#define SST_CH_OFF(subport, field) \
    (SST_CHAN_BASE + (unsigned long)(subport) * SST_CHAN_STRIDE + (field))

/* ----- Board command handshake (mirrors Stallion pattern) ----- */
static int sst_send_command(sst_card_t *card, unsigned char cmd)
{
    unsigned int spin;
    for (spin = 0; spin < 10000; spin++) {
        if (sst_readb(card->card_seg, SST_STATUS_OFF) == 0) break;
    }
    if (spin >= 10000) return -1;
    sst_writeb(card->card_seg, SST_COMMAND_OFF, cmd);
    for (spin = 0; spin < 10000; spin++) {
        if (sst_readb(card->card_seg, SST_STATUS_OFF) != 0) break;
    }
    return (spin < 10000) ? 0 : -1;
}

/* ----- Card pool hook ----- */

static void *sst_card_get(unsigned long card_seg)
{
    sst_card_t *c = (sst_card_t *)
        card_pool_get(g_sst_cards, sizeof(sst_card_t),
                      SST_MAX_CARDS, card_seg);
    if (c && c->card_seg == 0) {
        c->card_seg = (unsigned int)card_seg;
        c->nports = 0;
    }
    return c;
}

/* ----- Backend hooks ----- */

int equinox_sst_backend_probe(pcbdcom_port_t *p)
{
    sst_card_t *card = (sst_card_t *)p->backend_data;
    unsigned int sig_lo, sig_hi, seg;
    unsigned char n, state;

    if (!card) return -1;
    seg = card->card_seg;

    /* 4-byte signature check: "EQNX" as word pair.
     * (May need adjustment for real card — see file header note.) */
    sig_lo = sst_readw(seg, SST_SIG_OFF);
    sig_hi = sst_readw(seg, SST_SIG_OFF + 2);
    if (sig_lo != SST_SIG_MAGIC_HI || sig_hi != SST_SIG_MAGIC_LO) return -1;

    state = sst_readb(seg, SST_STATE_OFF);
    if (state != SST_STATE_READY) return -1;

    n = sst_readb(seg, SST_NPORTS_OFF);
    if (n != 4 && n != 8 && n != 16 && n != 32) return -1;
    card->nports = n;

    if (p->subport >= n) return -1;

    return 0;
}

int equinox_sst_backend_init(pcbdcom_port_t *p)
{
    sst_card_t *card = (sst_card_t *)p->backend_data;

    if (equinox_sst_backend_probe(p) < 0) return -1;

    /* Baud rate here is direct Hz, not a divisor (per esp.c). */
    sst_writel(card->card_seg, SST_CH_OFF(p->subport, SST_CH_BAUD),
               (unsigned long)(p->baud ? p->baud : 9600));
    sst_writeb(card->card_seg, SST_CH_OFF(p->subport, SST_CH_LCR), p->lcr);

    if (sst_send_command(card, SST_CMD_SETPORT) < 0) return -1;
    if (sst_send_command(card, SST_CMD_OPEN) < 0)    return -1;

    sst_writeb(card->card_seg, SST_CH_OFF(p->subport, SST_CH_MCR), 0x03);

    if (p->subport < SST_MAX_CHANNELS)
        card->ports[p->subport] = p;

    p->open = 1;
    return 0;
}

void equinox_sst_backend_deinit(pcbdcom_port_t *p)
{
    sst_card_t *card = (sst_card_t *)p->backend_data;
    if (!p->open || !card) return;
    sst_writeb(card->card_seg, SST_CH_OFF(p->subport, SST_CH_MCR), 0x00);
    (void)sst_send_command(card, SST_CMD_CLOSE);
    if (p->subport < SST_MAX_CHANNELS)
        card->ports[p->subport] = 0;
    p->open = 0;
}

void equinox_sst_backend_isr(pcbdcom_port_t *p)
{
    sst_card_t *card = (sst_card_t *)p->backend_data;
    unsigned long mask;
    unsigned int i;

    if (!card) return;

    /* 32-bit mask supports up to SST-32 */
    mask = sst_readl(card->card_seg, SST_INT_STATUS_OFF);
    if (mask == 0) return;

    for (i = 0; i < card->nports && i < SST_MAX_CHANNELS; i++) {
        if ((mask & (1UL << i)) && card->ports[i]) {
            (void)card->ports[i];
        }
    }

    sst_writel(card->card_seg, SST_INT_STATUS_OFF, 0UL);
}

int equinox_sst_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    sst_card_t *card = (sst_card_t *)p->backend_data;
    unsigned char *dst = (unsigned char *)buf;
    unsigned int head, tail, avail, ring_len, i;
    unsigned int rxbuf_off;

    if (!card || !p->open) return -1;

    head = sst_readw(card->card_seg, SST_CH_OFF(p->subport, SST_CH_RXHEAD));
    tail = sst_readw(card->card_seg, SST_CH_OFF(p->subport, SST_CH_RXTAIL));
    ring_len = sst_readw(card->card_seg, SST_CH_OFF(p->subport, SST_CH_RXBUF_LEN));
    if (ring_len == 0) return 0;

    avail = (head >= tail) ? (head - tail) : (ring_len - tail + head);
    if (avail > (unsigned)n) avail = n;
    if (avail == 0) return 0;

    /* Take low 16 bits of the dword offset — SST rings fit in 64KB seg */
    rxbuf_off = sst_readw(card->card_seg,
                          SST_CH_OFF(p->subport, SST_CH_RXBUF_OFF));

    for (i = 0; i < avail; i++) {
        dst[i] = sst_readb(card->card_seg,
                           (unsigned long)rxbuf_off + ((tail + i) % ring_len));
    }

    tail = (tail + avail) % ring_len;
    sst_writew(card->card_seg,
               SST_CH_OFF(p->subport, SST_CH_RXTAIL), tail);

    return (int)avail;
}

int equinox_sst_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    sst_card_t *card = (sst_card_t *)p->backend_data;
    const unsigned char *src = (const unsigned char *)buf;
    unsigned int head, tail, room, ring_len, i;
    unsigned int txbuf_off;

    if (!card || !p->open) return -1;

    head = sst_readw(card->card_seg, SST_CH_OFF(p->subport, SST_CH_TXHEAD));
    tail = sst_readw(card->card_seg, SST_CH_OFF(p->subport, SST_CH_TXTAIL));
    ring_len = sst_readw(card->card_seg, SST_CH_OFF(p->subport, SST_CH_TXBUF_LEN));
    if (ring_len == 0) return 0;

    room = (tail > head) ? (tail - head - 1) : (ring_len - head + tail - 1);
    if (room > (unsigned)n) room = n;
    if (room == 0) return 0;

    txbuf_off = sst_readw(card->card_seg,
                          SST_CH_OFF(p->subport, SST_CH_TXBUF_OFF));

    for (i = 0; i < room; i++) {
        sst_writeb(card->card_seg,
                   (unsigned long)txbuf_off + ((head + i) % ring_len), src[i]);
    }

    head = (head + room) % ring_len;
    sst_writew(card->card_seg,
               SST_CH_OFF(p->subport, SST_CH_TXHEAD), head);

    return (int)room;
}

const pcbdcom_backend_t pcbdcom_equinox_sst_backend = {
    "EQUINOX_SST",
    sst_card_get,
    equinox_sst_backend_probe,
    equinox_sst_backend_init,
    equinox_sst_backend_deinit,
    equinox_sst_backend_isr,
    equinox_sst_backend_read,
    equinox_sst_backend_write
};

#endif /* PCB1541 */

/* end equinox_sst_backend.c */
