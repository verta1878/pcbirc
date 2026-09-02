/* ============================================================================
 * stallion_brumby_backend.c — pcbdcom Stallion Brumby / ONboard backend
 *
 * Cards supported (v1):
 *   Stallion Brumby/4     4 ports, 80186 on-board, dual-port RAM
 *   Stallion Brumby/8     8 ports, same architecture
 *   Stallion ONboard/4    4 ports, later revision (compatible mailbox)
 *   Stallion ONboard/8    8 ports, same
 *
 * Design: Brumby/ONboard is Stallion's intelligent multiport line from
 * 1991-1993, the primary competitor to Digi's PC/8i and PC/Xe in that
 * era. Architecture: 80186 CPU on-board runs Stallion firmware; host
 * talks to the board entirely via a 4KB dual-port RAM window at a
 * jumper-selected CardSeg (0xC000..0xE000).
 *
 * The board firmware drives Cirrus Logic CD1400 quad-UARTs directly;
 * the host never touches CD1400 registers. This is DIFFERENT from
 * easyio_backend.c (Stallion's later dumb multiport line) which does
 * host-driven CD1400 I/O.
 *
 * Compared to Digi COM/Xi (digi_comxi_backend.c):
 *   - Same intelligent-card shape (host <-> mailbox <-> board CPU)
 *   - Different signature word ('SL' vs Digi's 'DS')
 *   - Larger channel struct (Stallion firmware exposes more state)
 *   - Explicit host->board command protocol (COMMAND word poll)
 *
 * Historical note: Stallion Technologies was an Australian company;
 * "Brumby" is Australian for wild horse. Stallion was Digi's #2
 * competitor in intelligent multiport before both were eclipsed by
 * ethernet-attached terminal servers in the late '90s. Stallion later
 * acquired by Emulex, then sunset entirely by 2005.
 *
 * Reference:
 *   - Linux drivers/char/istallion.c (kernel 2.6.32), GPLv2 — original
 *     authors: Greg Ungerer, Stallion Technologies. This backend is a
 *     port of the ONboard/Brumby mailbox layout and command protocol.
 *   - Stallion ONboard Technical Manual (public, 1992)
 *   - Stallion Brumby Hardware Reference (public, 1991)
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

/* PCB1541-only backend: only compiled into 15.41 builds */
#if defined(PCB1541)

#include "pcbdcom.h"
#include "backend.h"
#include "card_pool.h"

/* ----- Brumby/ONboard shared-memory layout (from istallion.c + Stallion docs) */

/* Header at start of dual-port RAM window (offsets from CardSeg:0) */
#define BR_SIG_OFF          0x0000L    /* 2-byte signature 'SL' (0x4C 0x53)  */
#define BR_SIG_MAGIC        0x4C53     /* little-endian 'S','L' as uint16    */

#define BR_MEMSIZE_OFF      0x0002L    /* 1 byte: RAM size code (0=256KB..)  */
#define BR_HWCFG_OFF        0x0003L    /* 1 byte: hw config bits             */
#define BR_NPORTS_OFF       0x0004L    /* 1 byte: 4 or 8                     */
#define BR_STATE_OFF        0x0005L    /* 1 byte: board state                */
#define BR_COMMAND_OFF      0x0006L    /* 1 byte: host->board command reg    */
#define BR_STATUS_OFF       0x0007L    /* 1 byte: board->host status reg     */
#define BR_INT_STATUS_OFF   0x0008L    /* 2 bytes: per-channel interrupt bits*/

/* Board state values */
#define BR_STATE_READY      0x02       /* board firmware ready               */

/* Command register values (host writes) */
#define BR_CMD_OPEN         0x01       /* open channel                       */
#define BR_CMD_CLOSE        0x02       /* close channel                      */
#define BR_CMD_SETPORT      0x03       /* set channel parameters             */

/* Channel descriptor array at 0x0100, stride 0x40 (larger than COM/Xi's 0x20) */
#define BR_CHAN_BASE        0x0100L
#define BR_CHAN_STRIDE      0x0040L

/* Per-channel field offsets */
#define BR_CH_TXBUF_OFF     0x00       /* word: tx ring far offset           */
#define BR_CH_TXBUF_LEN     0x02       /* word: tx ring length               */
#define BR_CH_TXHEAD        0x04       /* word: producer index (host writes) */
#define BR_CH_TXTAIL        0x06       /* word: consumer index (board reads) */
#define BR_CH_RXBUF_OFF     0x08       /* word: rx ring far offset           */
#define BR_CH_RXBUF_LEN     0x0A       /* word: rx ring length               */
#define BR_CH_RXHEAD        0x0C       /* word: producer index (board writes)*/
#define BR_CH_RXTAIL        0x0E       /* word: consumer index (host reads)  */
#define BR_CH_MSR           0x10       /* modem status (CD/DSR/CTS/RI)       */
#define BR_CH_MCR           0x11       /* modem control (DTR/RTS)            */
#define BR_CH_LSR           0x12       /* line status (breaks/errors)        */
#define BR_CH_STATE         0x13       /* channel state (open/closed/..)    */
#define BR_CH_BAUD          0x14       /* word: baud rate divisor            */
#define BR_CH_LCR           0x16       /* line control (bits/parity/stop)    */

#define BR_MAX_CHANNELS     8

/* Per-card state */
typedef struct {
    unsigned long   addr;                        /* pool header: card_seg  */
    unsigned char   in_use;                      /* pool header            */
    unsigned int    card_seg;                    /* dual-port RAM segment  */
    unsigned char   nports;                      /* 4 or 8                 */
    pcbdcom_port_t *ports[BR_MAX_CHANNELS];      /* subport -> port slot   */
} brumby_card_t;

#define BR_MAX_CARDS 4
static brumby_card_t g_brumby_cards[BR_MAX_CARDS];

/* ----- Far-pointer helpers into dual-port RAM window ----- */

static unsigned char br_readb(unsigned int seg, unsigned long off)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void br_writeb(unsigned int seg, unsigned long off, unsigned char v)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

static unsigned int br_readw(unsigned int seg, unsigned long off)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    return *p;
}

static void br_writew(unsigned int seg, unsigned long off, unsigned int v)
{
    unsigned int __far *p = (unsigned int __far *)
        (((unsigned long)seg << 16) | (off & 0xFFFFL));
    *p = v;
}

#define BR_CH_OFF(subport, field) \
    (BR_CHAN_BASE + (unsigned long)(subport) * BR_CHAN_STRIDE + (field))

/* ----- Board command handshake -----
 * Host writes command byte, board firmware reads it, executes, writes
 * status. Standard poll pattern from istallion.c. */
static int br_send_command(brumby_card_t *card, unsigned char cmd)
{
    unsigned int spin;

    /* Wait for previous command to complete (status = 0) */
    for (spin = 0; spin < 10000; spin++) {
        if (br_readb(card->card_seg, BR_STATUS_OFF) == 0) break;
    }
    if (spin >= 10000) return -1;

    br_writeb(card->card_seg, BR_COMMAND_OFF, cmd);

    /* Wait for board to ack (status != 0) */
    for (spin = 0; spin < 10000; spin++) {
        if (br_readb(card->card_seg, BR_STATUS_OFF) != 0) break;
    }
    return (spin < 10000) ? 0 : -1;
}

/* ----- Card pool hook ----- */

static void *brumby_card_get(unsigned long card_seg)
{
    brumby_card_t *c = (brumby_card_t *)
        card_pool_get(g_brumby_cards, sizeof(brumby_card_t),
                      BR_MAX_CARDS, card_seg);
    if (c && c->card_seg == 0) {
        c->card_seg = (unsigned int)card_seg;
        c->nports = 0;   /* filled on first probe */
    }
    return c;
}

/* ----- Backend hooks ----- */

int stallion_brumby_backend_probe(pcbdcom_port_t *p)
{
    brumby_card_t *card = (brumby_card_t *)p->backend_data;
    unsigned int sig, seg;
    unsigned char n, state;

    if (!card) return -1;
    seg = card->card_seg;

    /* Signature check: word at offset 0 must equal 'SL' */
    sig = br_readw(seg, BR_SIG_OFF);
    if (sig != BR_SIG_MAGIC) return -1;

    /* Board must be in READY state (firmware booted) */
    state = br_readb(seg, BR_STATE_OFF);
    if (state != BR_STATE_READY) return -1;

    /* Port count sanity: 4 or 8 */
    n = br_readb(seg, BR_NPORTS_OFF);
    if (n != 4 && n != 8) return -1;
    card->nports = n;

    if (p->subport >= n) return -1;

    return 0;
}

int stallion_brumby_backend_init(pcbdcom_port_t *p)
{
    brumby_card_t *card = (brumby_card_t *)p->backend_data;

    if (stallion_brumby_backend_probe(p) < 0) return -1;

    /* Set baud rate + line params in channel struct, then command board
     * to apply them. Board firmware handles CD1400 register programming. */
    br_writew(card->card_seg, BR_CH_OFF(p->subport, BR_CH_BAUD),
              (unsigned int)(115200L / (p->baud ? p->baud : 9600)));
    br_writeb(card->card_seg, BR_CH_OFF(p->subport, BR_CH_LCR), p->lcr);

    if (br_send_command(card, BR_CMD_SETPORT) < 0) return -1;
    if (br_send_command(card, BR_CMD_OPEN) < 0)    return -1;

    /* Assert DTR+RTS */
    br_writeb(card->card_seg, BR_CH_OFF(p->subport, BR_CH_MCR), 0x03);

    if (p->subport < BR_MAX_CHANNELS)
        card->ports[p->subport] = p;

    p->open = 1;
    return 0;
}

void stallion_brumby_backend_deinit(pcbdcom_port_t *p)
{
    brumby_card_t *card = (brumby_card_t *)p->backend_data;

    if (!p->open || !card) return;

    br_writeb(card->card_seg, BR_CH_OFF(p->subport, BR_CH_MCR), 0x00);
    (void)br_send_command(card, BR_CMD_CLOSE);

    if (p->subport < BR_MAX_CHANNELS)
        card->ports[p->subport] = 0;

    p->open = 0;
}

/* ISR: read 16-bit interrupt-status mask, walk affected channels, ack. */
void stallion_brumby_backend_isr(pcbdcom_port_t *p)
{
    brumby_card_t *card = (brumby_card_t *)p->backend_data;
    unsigned int mask, i;

    if (!card) return;

    mask = br_readw(card->card_seg, BR_INT_STATUS_OFF);
    if (mask == 0) return;

    /* Board firmware advances ring pointers autonomously; here we just
     * acknowledge. Upper layer reads current pointers on next read/write. */
    for (i = 0; i < card->nports && i < BR_MAX_CHANNELS; i++) {
        if (mask & (1 << i)) {
            (void)card->ports[i];
        }
    }

    br_writew(card->card_seg, BR_INT_STATUS_OFF, 0x0000);
}

int stallion_brumby_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    brumby_card_t *card = (brumby_card_t *)p->backend_data;
    unsigned char *dst = (unsigned char *)buf;
    unsigned int head, tail, avail, ring_len, i;
    unsigned long rxbuf_off;

    if (!card || !p->open) return -1;

    head = br_readw(card->card_seg, BR_CH_OFF(p->subport, BR_CH_RXHEAD));
    tail = br_readw(card->card_seg, BR_CH_OFF(p->subport, BR_CH_RXTAIL));
    ring_len = br_readw(card->card_seg, BR_CH_OFF(p->subport, BR_CH_RXBUF_LEN));
    if (ring_len == 0) return 0;

    avail = (head >= tail) ? (head - tail) : (ring_len - tail + head);
    if (avail > (unsigned)n) avail = n;
    if (avail == 0) return 0;

    rxbuf_off = br_readw(card->card_seg,
                         BR_CH_OFF(p->subport, BR_CH_RXBUF_OFF));

    for (i = 0; i < avail; i++) {
        dst[i] = br_readb(card->card_seg,
                          rxbuf_off + ((tail + i) % ring_len));
    }

    tail = (tail + avail) % ring_len;
    br_writew(card->card_seg,
              BR_CH_OFF(p->subport, BR_CH_RXTAIL), tail);

    return (int)avail;
}

int stallion_brumby_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    brumby_card_t *card = (brumby_card_t *)p->backend_data;
    const unsigned char *src = (const unsigned char *)buf;
    unsigned int head, tail, room, ring_len, i;
    unsigned long txbuf_off;

    if (!card || !p->open) return -1;

    head = br_readw(card->card_seg, BR_CH_OFF(p->subport, BR_CH_TXHEAD));
    tail = br_readw(card->card_seg, BR_CH_OFF(p->subport, BR_CH_TXTAIL));
    ring_len = br_readw(card->card_seg, BR_CH_OFF(p->subport, BR_CH_TXBUF_LEN));
    if (ring_len == 0) return 0;

    /* Free space, leave 1-byte guard between head and tail */
    room = (tail > head) ? (tail - head - 1) : (ring_len - head + tail - 1);
    if (room > (unsigned)n) room = n;
    if (room == 0) return 0;

    txbuf_off = br_readw(card->card_seg,
                         BR_CH_OFF(p->subport, BR_CH_TXBUF_OFF));

    for (i = 0; i < room; i++) {
        br_writeb(card->card_seg,
                  txbuf_off + ((head + i) % ring_len), src[i]);
    }

    head = (head + room) % ring_len;
    br_writew(card->card_seg,
              BR_CH_OFF(p->subport, BR_CH_TXHEAD), head);

    return (int)room;
}

const pcbdcom_backend_t pcbdcom_stallion_brumby_backend = {
    "STALLION_BRUMBY",
    brumby_card_get,
    stallion_brumby_backend_probe,
    stallion_brumby_backend_init,
    stallion_brumby_backend_deinit,
    stallion_brumby_backend_isr,
    stallion_brumby_backend_read,
    stallion_brumby_backend_write
};

#endif /* PCB1541 */

/* end stallion_brumby_backend.c */
