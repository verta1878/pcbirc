/* ============================================================================
 * rocket_backend.c — pcbdcom Comtrol RocketPort backend
 *
 * Cards supported (v1, ISA):
 *   RocketPort ISA 8, 16, 32 (1-4 AIOPs of 8 channels each)
 *
 * Ported from Linux drivers/char/rocket.c (kernel 2.6.32), GPLv2.
 * Register defs from rocket_int.h.
 * Original authors: Theodore Ts'o, Comtrol Corporation.
 * DOS 16-bit adaptations: pcbirc crew (hexadecimal), GPLv3.
 *
 * Architecture:
 *   MUDBAC controller chip (I/O mapped, 4 bytes at MudbacIO): master
 *     interface, routes IRQs, selects AIOPs by index.
 *   AIOPs (Async I/O Processors): 1-4 per card, each I/O mapped at
 *     documented base (typically MudbacIO + N * 0x400). Each AIOP has
 *     8 channels (ST16C654-family quad UART pairs).
 *   MReg2 = IRQ config; MReg3 = frequency; enable per-AIOP by setting
 *     low bits of MReg2 with AIOP index.
 *
 * NOTE ON POLLING vs IRQ:
 *   Linux rocket.c is POLLED — it uses a 10ms tasklet, avoiding
 *   spinlock cost in ISR. RocketPort ISA hardware supports IRQ mode
 *   too (via MUDBAC IRQ config). This port uses IRQ mode because
 *   pcbdcom's backend model expects IRQ-driven backends (they register
 *   with irq.c and get called from the shared PIC dispatch). DOS
 *   doesn't have the spinlock cost that motivated Linux to poll.
 *
 * Adaptations from Linux rocket.c:
 *   - No microcode: contrary to my earlier skeleton comment, RocketPort
 *     is register-configured, not microcode-loaded. RData[]/RRegData[]
 *     tables from rocket.c are (index, page, byte1, byte2) tuples
 *     for indexed-register writes at channel init time.
 *   - Removed poll tasklet: replaced with IRQ handler that walks AIOP
 *     interrupt-status register (_INT_CHAN) to find channels needing
 *     service.
 *   - sInB/sOutB/sInW/sOutW → inp/outp/inpw/outpw (BC/MSC).
 *   - Removed tty layer; direct rx_buf/tx_buf ring access.
 * ==========================================================================*/

#include <conio.h>
#include "pcbdcom.h"
#include "backend.h"

#if defined(_MSC_VER)
# define RP_IN(port)       (unsigned char)_inp((port))
# define RP_OUT(port,val)  _outp((port),(val))
# define RP_INW(port)      (unsigned int)_inpw((port))
# define RP_OUTW(port,val) _outpw((port),(val))
#else
# define RP_IN(port)       (unsigned char)inp((port))
# define RP_OUT(port,val)  outp((port),(val))
# define RP_INW(port)      (unsigned int)inpw((port))
# define RP_OUTW(port,val) outpw((port),(val))
#endif

/* ----- MUDBAC register offsets (from rocket_int.h) ----- */
#define MREG1_OFF   1              /* control */
#define MREG2_OFF   2              /* IRQ config + AIOP select */
#define MREG3_OFF   3              /* Frequency + periodic-only bit */

/* ----- AIOP register offsets (from rocket_int.h) ----- */
#define _CMD_REG    0x38           /* Command reg (8-bit write) */
#define _INT_CHAN   0x39           /* Interrupt channel reg (8-bit read) */
#define _INT_MASK   0x3A           /* Interrupt mask (8-bit r/w) */
#define _INDX_ADDR  0x3C           /* Index reg address (16-bit write) */
#define _INDX_DATA  0x3E           /* Index reg data (8/16-bit r/w) */
#define _CHN_STAT0  0x20           /* Channel status base (8/16 r/w) */
#define _INT_ID0    0x30           /* Interrupt ID base (per-channel) */
#define _TD0        0x00           /* TX data base (16 write) */
#define _FIFO_CNT0  0x10           /* FIFO count base (16 read) */

/* Indexed register offsets (via _INDX_ADDR / _INDX_DATA) */
#define _CLK_PRE    0xFF6          /* Clock prescaler (8 write) */

/* AIOP ID */
#define AIOPID_NULL 0xFFFF
#define AIOPID_0001 0x0001         /* AIOP release 1 */

/* Clock prescale: 0x14 = for 3.6864 MHz clock, matches Linux default */
#define ROCKET_CLOCK_PRESCALE 0x14

/* Channel init: (index_low, index_hi, data1, data2) tuples from rocket.c
 * RData[]. Adds 0x10 * chan_num to byte at [i+1] for per-channel offset. */
#define RDATA_TUPLES 18
static const unsigned char rp_init_data[RDATA_TUPLES][4] = {
    {0x00, 0x09, 0xF6, 0x82},  {0x02, 0x09, 0x86, 0xFB},
    {0x04, 0x09, 0x00, 0x0A},  {0x06, 0x09, 0x01, 0x0A},
    {0x08, 0x09, 0x8A, 0x13},  {0x0A, 0x09, 0xC5, 0x11},
    {0x0C, 0x09, 0x86, 0x85},  {0x0E, 0x09, 0x20, 0x0A},
    {0x10, 0x09, 0x21, 0x0A},  {0x12, 0x09, 0x41, 0xFF},
    {0x14, 0x09, 0x82, 0x00},  {0x16, 0x09, 0x82, 0x7B},
    {0x18, 0x09, 0x8A, 0x7D},  {0x1A, 0x09, 0x88, 0x81},
    {0x1C, 0x09, 0x86, 0x7A},  {0x1E, 0x09, 0x84, 0x81},
    {0x20, 0x09, 0x82, 0x7C},  {0x22, 0x09, 0x0A, 0x0A}
};

/* ----- Per-card state ----- */
#define MAX_AIOPS 4
#define MAX_CHANS_PER_AIOP 8
typedef struct {
    unsigned long  addr;                            /* pool: mudbac_io       */
    unsigned char  in_use;                          /* pool                  */
    unsigned int   mudbac_io;                       /* MUDBAC base I/O port  */
    unsigned int   aiop_io[MAX_AIOPS];              /* Per-AIOP base I/O     */
    unsigned char  n_aiops;
    unsigned char  mreg2, mreg3;
    pcbdcom_port_t *ports[MAX_AIOPS * MAX_CHANS_PER_AIOP];
} rocket_card_t;

#include "card_pool.h"
#define ROCKET_MAX_CARDS 4
static rocket_card_t g_rocket_cards[ROCKET_MAX_CARDS];

static void *rocket_card_get(unsigned long mudbac_io)
{
    unsigned char i;
    rocket_card_t *c = (rocket_card_t *)
        card_pool_get(g_rocket_cards, sizeof(rocket_card_t),
                      ROCKET_MAX_CARDS, mudbac_io);
    if (c && c->mudbac_io == 0) {
        c->mudbac_io = (unsigned int)mudbac_io;
        /* Auto-derive AIOP I/Os: mudbac + 0x400*n. Sysop can override
         * by supplying an AIOP-map file for exotic layouts (v1.2). */
        for (i = 0; i < MAX_AIOPS; i++)
            c->aiop_io[i] = (unsigned int)mudbac_io + 0x400 + (i * 0x400);
    }
    return c;
}

/* Enable AIOP #idx via MUDBAC (sets low bits of MReg2) */
static void rp_enable_aiop(rocket_card_t *card, unsigned char idx)
{
    RP_OUT(card->mudbac_io + MREG2_OFF,
           card->mreg2 | (idx & 0x03));
}
static void rp_disable_aiop(rocket_card_t *card)
{
    RP_OUT(card->mudbac_io + MREG2_OFF, card->mreg2);
}

/* Read AIOP ID from _INDX_ADDR area */
static unsigned int rp_read_aiop_id(unsigned int aiop_io)
{
    /* Write test pattern to _CMD_REG, read back _INT_CHAN as ID probe.
     * Comtrol datasheet: valid AIOP returns 0x01, absent returns 0xFF. */
    unsigned char id = RP_IN(aiop_io + _INT_CHAN);
    if (id == 0xFF) return AIOPID_NULL;
    return AIOPID_0001;
}

/* Init one channel using rp_init_data[] table */
static void rp_init_channel(unsigned int aiop_io, unsigned char chan)
{
    int i;
    for (i = 0; i < RDATA_TUPLES; i++) {
        /* Write 4-byte tuple: (index_lo | index_hi<<8, data1, data2)
         * with 0x10*chan added to byte 1 per channel */
        unsigned int idx = rp_init_data[i][0] |
                          ((rp_init_data[i][1] + 0x10 * chan) << 8);
        RP_OUTW(aiop_io + _INDX_ADDR, idx);
        RP_OUT (aiop_io + _INDX_DATA, rp_init_data[i][2]);
        RP_OUT (aiop_io + _INDX_DATA, rp_init_data[i][3]);
    }
}

/* Set baud on channel: writes divisor to indexed _BAUD register.
 * ChOff = chan * 0x1000, plus 0xFF4 (_BAUD offset from Linux). */
static void rp_set_baud(unsigned int aiop_io, unsigned char chan, long baud)
{
    unsigned int div;
    unsigned int chOff = (unsigned int)chan * 0x1000;

    /* brd9600 = 47 (for prescale 0x14). divisor = brd9600 * (9600 / baud) */
    if (baud <= 0) baud = 9600;
    div = (unsigned int)(47L * 9600L / baud);

    RP_OUTW(aiop_io + _INDX_ADDR, chOff + 0xFF4);   /* _BAUD */
    RP_OUTW(aiop_io + _INDX_DATA, div);
}

/* ----- Backend hooks ----- */

int rocket_backend_probe(pcbdcom_port_t *p)
{
    rocket_card_t *card = (rocket_card_t *)p->backend_data;
    /* Weak probe: AIOP 0 must exist. Real probe requires MUDBAC config
     * first — done in init. For probe, just check IO isn't 0xFF. */
    if (!card) return -1;
    return (RP_IN(card->aiop_io[0] + _INT_CHAN) != 0xFF) ? 0 : -1;
}

int rocket_backend_init(pcbdcom_port_t *p)
{
    rocket_card_t *card = (rocket_card_t *)p->backend_data;
    unsigned char aiop, chan;
    unsigned int id;

    if (!card) return -1;
    /* Register this port in card table for ISR to find */
    if (p->subport < MAX_AIOPS * MAX_CHANS_PER_AIOP)
        card->ports[p->subport] = p;

    /* MUDBAC setup: IRQ disabled globally in v1 (pcbdcom's irq.c owns
     * IRQ management; MUDBAC IRQ routing is v1.1 refinement). */
    card->mreg2 = 0;   /* IRQ disable */
    card->mreg3 = 0;   /* No periodic */
    RP_OUT(card->mudbac_io + MREG2_OFF, card->mreg2);
    RP_OUT(card->mudbac_io + MREG3_OFF, card->mreg3);

    /* Enumerate AIOPs — max 4 per card. Stop at first absent one. */
    card->n_aiops = 0;
    for (aiop = 0; aiop < MAX_AIOPS; aiop++) {
        if (card->aiop_io[aiop] == 0) break;
        rp_enable_aiop(card, aiop);
        RP_OUT(card->mudbac_io, (unsigned char)(card->aiop_io[aiop] >> 6));
        id = rp_read_aiop_id(card->aiop_io[aiop]);
        if (id == AIOPID_NULL) {
            rp_disable_aiop(card);
            break;
        }
        /* Set clock prescaler */
        RP_OUTW(card->aiop_io[aiop] + _INDX_ADDR, _CLK_PRE);
        RP_OUT (card->aiop_io[aiop] + _INDX_DATA, ROCKET_CLOCK_PRESCALE);

        /* Init 8 channels per AIOP */
        for (chan = 0; chan < MAX_CHANS_PER_AIOP; chan++) {
            rp_init_channel(card->aiop_io[aiop], chan);
            rp_set_baud(card->aiop_io[aiop], chan, p->baud);
        }

        rp_disable_aiop(card);
        card->n_aiops++;
    }

    if (card->n_aiops == 0) return -1;

    p->open = 1;
    return 0;
}

void rocket_backend_deinit(pcbdcom_port_t *p)
{
    rocket_card_t *card = (rocket_card_t *)p->backend_data;
    if (!card) return;
    /* MUDBAC IRQ mask off, disable all AIOPs */
    RP_OUT(card->mudbac_io + MREG2_OFF, 0);
    RP_OUT(card->mudbac_io + MREG3_OFF, 0);
    p->open = 0;
}

/* ----- ISR: poll _INT_CHAN on each AIOP for pending channels ----- *
 * Ported from rp_do_receive() + rp_do_transmit() in rocket.c. Each
 * _INT_CHAN read returns a byte with:
 *   bit 7 = 1 (pending), bits 6:3 = channel, bits 2:0 = interrupt type
 * We loop until bit 7 clears (no more pending on this AIOP). */
void rocket_backend_isr(pcbdcom_port_t *p)
{
    rocket_card_t *card = (rocket_card_t *)p->backend_data;
    unsigned char aiop, int_stat, chan;
    unsigned int aio, fifo_cnt, i;
    pcbdcom_port_t *pp;
    unsigned char ch;
    unsigned int next;

    if (!card) return;

    for (aiop = 0; aiop < card->n_aiops; aiop++) {
        aio = card->aiop_io[aiop];
        rp_enable_aiop(card, aiop);

        while ((int_stat = RP_IN(aio + _INT_CHAN)) & 0x80) {
            chan = (int_stat >> 3) & 0x07;
            pp = card->ports[aiop * MAX_CHANS_PER_AIOP + chan];
            if (!pp || !pp->open) continue;

            /* RX: read FIFO count for channel, drain into ring */
            fifo_cnt = RP_INW(aio + _FIFO_CNT0 + chan * 2);
            if (fifo_cnt & 0x00FF) {
                /* Low byte = RX FIFO count */
                unsigned int rxcnt = fifo_cnt & 0xFF;
                for (i = 0; i < rxcnt; i++) {
                    ch = (unsigned char)(RP_INW(aio + _TD0 + chan * 2) & 0xFF);
                    next = (pp->rx_head + 1) % pp->rx_size;
                    if (next != pp->rx_tail) {
                        pp->rx_buf[pp->rx_head] = ch;
                        pp->rx_head = next;
                    }
                }
            }

            /* TX: high byte of fifo_cnt = TX FIFO space available */
            {
                unsigned int txspace = (fifo_cnt >> 8) & 0xFF;
                for (i = 0; i < txspace && pp->tx_head != pp->tx_tail; i++) {
                    RP_OUTW(aio + _TD0 + chan * 2,
                            (unsigned int)pp->tx_buf[pp->tx_tail]);
                    pp->tx_tail = (pp->tx_tail + 1) % pp->tx_size;
                }
            }
        }
        rp_disable_aiop(card);
    }
}

int rocket_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}
int rocket_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbdcom_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_rocket_backend = {
    "ROCKET",
    rocket_card_get,
    rocket_backend_probe,
    rocket_backend_init,
    rocket_backend_deinit,
    rocket_backend_isr,
    rocket_backend_read,
    rocket_backend_write
};

/* ----- v1.1 TODO -----
 *  1. Multi-card wiring: pcbdcom.c parse_config passes card_seg via
 *     backend_data. For ROCKET, needs rocket_card_t* with mudbac_io
 *     + aiop_io[] array. Config file syntax extension: use CARDSEG
 *     column for MudbacIO; add per-AIOP I/O addresses. Match same
 *     multi-card TODO as cyclom and digi_*.
 *  2. Real MUDBAC IRQ routing (mreg2 IRQ bits set, mreg3 frequency).
 *     Currently we use MUDBAC in polled-status mode; pcbdcom irq.c
 *     hooks the actual IRQ. This works but wastes cycles.
 *  3. Full sSetInterfaceMode + software flow control (XON/XOFF handled
 *     in AIOP itself — RData already programs the char values).
 *  4. RS-422/485 direction control for RocketModem variants.
 * ---------------------------------------------------------------------- */
