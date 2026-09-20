/* ============================================================================
 * cyclom_backend.c — pcbdcom Cyclades Cyclom-Y backend
 *
 * Cards supported: Cyclom-4Y, Cyclom-8Y, Cyclom-16Y, Cyclom-32Y (ISA).
 * All use Cirrus Logic CD1400 quad-UART chips memory-mapped at Card
 * Segment (typically 0xD000..0xE000, 16 KB window).
 *
 * Ported from Linux drivers/char/cyclades.c (kernel 2.6.32), GPLv2.
 * Original authors: Cyclades Corp; Randolph Bentson (major rewrite,
 * 1996-2004); Ivan Passos, Marcio Saito.
 * Register/bit constants from linux/include/linux/cyclades.h.
 * DOS 16-bit adaptations: pcbirc crew (hexadecimal), GPLv3.
 *
 * Adaptations from Linux cyclades.c:
 *   - cyy_writeb/cyy_readb → direct __far pointer access via card_seg.
 *   - cyy_interrupt() → cyclom_backend_isr(): same SVRR walk, same
 *     RIVR/TIVR/MIVR vector decode. Simplified: no character tagging,
 *     RX errors dropped silently (v1). Break/parity handling deferred.
 *   - Ring buffers use pcbdcom's 512B (Linux used 4KB per port).
 *   - Removed tty_flip_buffer / tty_insert_flip_char; write directly
 *     to pcbdcom_port_t's rx_buf ring.
 *   - Removed spinlocks: DOS is single-threaded; ISR must complete
 *     without being re-entered (guaranteed by 8259 PIC masking).
 *   - ISA-only: register offsets use *2 multiplier (Linux "index=1").
 * ==========================================================================*/

#include "pcbdcom.h"
#include "backend.h"

/* ----- CD1400 register offsets (ISA: byte offset = *2 from datasheet) ----- */
#define CyGFRCR   (0x40*2)   /* Global firmware revision code             */
#define CyCAR     (0x68*2)   /* Channel access register                   */
#define CyGCR     (0x4B*2)   /* Global config register                    */
#define CySVRR    (0x67*2)   /* Service request register (per chip)       */
#define CyRIR     (0x6B*2)   /* Receive interrupt register                */
#define CyTIR     (0x6A*2)   /* Transmit interrupt register               */
#define CyMIR     (0x69*2)   /* Modem interrupt register                  */
#define CyRIVR    (0x43*2)   /* Receive interrupt vector register         */
#define CyTIVR    (0x42*2)   /* Transmit interrupt vector register        */
#define CyMIVR    (0x41*2)   /* Modem interrupt vector register           */
#define CyRDSR    (0x62*2)   /* Receive data / status register            */
#define CyTDR     (0x63*2)   /* Transmit data register                    */
#define CyRDCR    (0x0E*2)   /* Receive data count (channel)              */
#define CyMISR    (0x4C*2)   /* Modem interrupt status                    */
#define CyEOSRR   (0x60*2)   /* End of service request                    */
#define CyLIVR    (0x18*2)   /* Local interrupt vector                    */
#define CyCCR     (0x05*2)   /* Channel command register                  */
#define CySRER    (0x06*2)   /* Service request enable                    */
#define CyCOR1    (0x08*2)   /* Channel option 1 (parity/bits)            */
#define CyCOR2    (0x09*2)   /* Channel option 2 (flow control)           */
#define CyCOR3    (0x0A*2)   /* Channel option 3 (RX threshold)           */
#define CyTBPR    (0x72*2)   /* Transmit baud prescaler                   */
#define CyRBPR    (0x78*2)   /* Receive baud prescaler                    */
#define CyTCOR    (0x76*2)   /* Transmit clock option                     */
#define CyRCOR    (0x7C*2)   /* Receive clock option                      */

/* Interrupt vector bit fields (RIVR/TIVR/MIVR) */
#define CyIVRMask  0x07      /* Low 3 bits = interrupt type              */
#define CyIVRRxEx  0x07      /* RX exception (parity, framing, timeout)  */
#define CyIVRRxOK  0x03      /* RX normal                                */
#define CyIVRTxOK  0x02      /* TX ready                                 */
#define CyIVRMdmOK 0x01      /* Modem status change                      */

/* SRER (Service Request Enable) bits */
#define CyMdmCh   0x80       /* Enable modem interrupts                  */
#define CyRxData  0x10       /* Enable RX interrupts                     */
#define CyTxRdy   0x04       /* Enable TX ready interrupts               */
#define CyTxMpty  0x02       /* Enable TX empty (drain) interrupts       */

/* CCR (Channel Command Register) commands */
#define CyCHAN_RESET     0x80
#define CyCHIP_RESET     0x81
#define CyFlushTransFIFO 0x82
#define CyCHAN_CTL       0x10
#define CyENB_RCVR       0x02
#define CyENB_XMTR       0x08

/* Line format */
#define Cy_8_BITS        0x03
#define Cy_1_STOP        0x00
#define CyPARITY_NONE    0x00

/* Card window offsets */
#define Cy_ClrIntr       0x1800   /* Write to clear card-level interrupt */
#define CyRegSize        0x0400   /* Bytes per CD1400 chip window        */

/* Max chips per card and channels per chip */
#define CyMAX_CHIPS_PER_CARD 8
#define CyPORTS_PER_CHIP     4

/* ----- Per-card state (attached via backend_data) ----- */
typedef struct {
    unsigned long  addr;                           /* pool: card_seg (as ul) */
    unsigned char  in_use;                         /* pool                   */
    unsigned int   card_seg;                       /* Card memory segment    */
    unsigned char  n_chips;                        /* 1..8                   */
    unsigned char  n_channels;                     /* n_chips * 4            */
    pcbdcom_port_t *channels[CyMAX_CHIPS_PER_CARD * CyPORTS_PER_CHIP];
} cyclom_card_t;

#include "card_pool.h"
#define CYCLOM_MAX_CARDS 4
static cyclom_card_t g_cyclom_cards[CYCLOM_MAX_CARDS];

static void *cyclom_card_get(unsigned long card_seg)
{
    cyclom_card_t *c = (cyclom_card_t *)
        card_pool_get(g_cyclom_cards, sizeof(cyclom_card_t),
                      CYCLOM_MAX_CARDS, card_seg);
    if (c && c->card_seg == 0) {
        c->card_seg = (unsigned int)card_seg;
        c->n_chips = 8;              /* max; probe will trim */
        c->n_channels = 32;
    }
    return c;
}

/* Register access: card_seg:offset far pointer.
 * offset is card-window-relative (0 <= offset < 0x4000). */
static unsigned char cy_read(unsigned int seg, unsigned int off)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | off);
    return *p;
}
static void cy_write(unsigned int seg, unsigned int off, unsigned char v)
{
    unsigned char __far *p = (unsigned char __far *)
        (((unsigned long)seg << 16) | off);
    *p = v;
}

/* Chip base within card window: chip N starts at offset N*CyRegSize */
#define CHIP_BASE(chip) ((chip) * CyRegSize)

/* Select channel on a chip (writes CAR). All subsequent register
 * accesses on that chip refer to the selected channel. */
static void cy_select(unsigned int seg, unsigned char chip, unsigned char chan)
{
    cy_write(seg, CHIP_BASE(chip) + CyCAR, chan & 0x03);
}

/* ----- Baud rate: CD1400 uses prescaler + clock option ----- *
 * Simplified table for common rates. Full computation in Linux
 * cy_baud_table[] — deferred, this covers PCBoard's normal set. */
static void cy_set_baud(unsigned int seg, unsigned char chip, long baud)
{
    unsigned char tcor, tbpr;
    /* CD1400 prescaler values for 25 MHz clock */
    switch (baud) {
        case 300:    tcor = 0x08; tbpr = 0xCF; break;
        case 1200:   tcor = 0x08; tbpr = 0x34; break;
        case 2400:   tcor = 0x08; tbpr = 0x1A; break;
        case 9600:   tcor = 0x03; tbpr = 0x82; break;
        case 19200:  tcor = 0x02; tbpr = 0x82; break;
        case 38400:  tcor = 0x02; tbpr = 0x41; break;
        case 57600:  tcor = 0x02; tbpr = 0x2C; break;
        case 115200: tcor = 0x02; tbpr = 0x16; break;
        default:     tcor = 0x02; tbpr = 0x41; break;  /* fall back 38400 */
    }
    cy_write(seg, CHIP_BASE(chip) + CyTCOR, tcor);
    cy_write(seg, CHIP_BASE(chip) + CyTBPR, tbpr);
    cy_write(seg, CHIP_BASE(chip) + CyRCOR, tcor);
    cy_write(seg, CHIP_BASE(chip) + CyRBPR, tbpr);
}

/* ----- Backend hooks ----- */

int cyclom_backend_probe(pcbdcom_port_t *p)
{
    cyclom_card_t *card = (cyclom_card_t *)p->backend_data;
    unsigned char gfrcr;
    if (!card) return -1;
    gfrcr = cy_read(card->card_seg, CHIP_BASE(0) + CyGFRCR);
    return (gfrcr >= 0x40 && gfrcr < 0x50) ? 0 : -1;
}

int cyclom_backend_init(pcbdcom_port_t *p)
{
    cyclom_card_t *card = (cyclom_card_t *)p->backend_data;
    unsigned int seg;
    unsigned char chip, chan;

    if (cyclom_backend_probe(p) < 0) return -1;
    seg = card->card_seg;

    /* subport 0..31 → chip 0..7, chan 0..3 */
    chip = p->subport / CyPORTS_PER_CHIP;
    chan = p->subport % CyPORTS_PER_CHIP;
    /* Register in card's channel table for ISR dispatch */
    if (p->subport < 32) card->channels[p->subport] = p;

    /* Reset channel */
    cy_select(seg, chip, chan);
    cy_write(seg, CHIP_BASE(chip) + CyCCR, CyCHAN_RESET);
    /* Spin: CCR reads back 0 when reset done */
    while (cy_read(seg, CHIP_BASE(chip) + CyCCR) != 0)
        ; /* datasheet: <10µs */

    /* Line: 8N1, no flow, RX threshold = 1 */
    cy_write(seg, CHIP_BASE(chip) + CyCOR1, Cy_8_BITS | Cy_1_STOP | CyPARITY_NONE);
    cy_write(seg, CHIP_BASE(chip) + CyCOR2, 0x00);
    cy_write(seg, CHIP_BASE(chip) + CyCOR3, 0x01);

    cy_set_baud(seg, chip, p->baud);

    /* Local interrupt vector: encode port number (for our ISR to find port) */
    cy_write(seg, CHIP_BASE(chip) + CyLIVR, (unsigned char)(p - &p[0]) & 0xFF);

    /* Enable RX + modem, TX enabled on demand */
    cy_write(seg, CHIP_BASE(chip) + CySRER, CyRxData | CyMdmCh);

    /* Enable channel */
    cy_write(seg, CHIP_BASE(chip) + CyCCR, CyCHAN_CTL | CyENB_RCVR | CyENB_XMTR);

    p->open = 1;
    return 0;
}

void cyclom_backend_deinit(pcbdcom_port_t *p)
{
    cyclom_card_t *card = (cyclom_card_t *)p->backend_data;
    unsigned char chip, chan;
    if (!p->open || !card) return;
    chip = p->subport / CyPORTS_PER_CHIP;
    chan = p->subport % CyPORTS_PER_CHIP;
    cy_select(card->card_seg, chip, chan);
    cy_write(card->card_seg, CHIP_BASE(chip) + CyCCR, CyCHAN_CTL);
    cy_write(card->card_seg, CHIP_BASE(chip) + CySRER, 0);
    p->open = 0;
}

/* ----- ISR: per-card entry, walks all chips + services pending channels
 * Ported from cy_interrupt() / cyy_chip_rx / cyy_chip_tx / cyy_chip_modem
 * in Linux cyclades.c (~line 400-800). ----- */
void cyclom_backend_isr(pcbdcom_port_t *p)
{
    cyclom_card_t *card = (cyclom_card_t *)p->backend_data;
    unsigned int seg;
    unsigned char chip, save_car, save_xir, ivr, chan, ch, count, i;
    unsigned int next;

    if (!card) return;
    seg = card->card_seg;

    /* Walk each CD1400 chip. SVRR bits: 0=modem, 1=TX, 2=RX. */
    for (chip = 0; chip < card->n_chips; chip++) {
        unsigned char svrr = cy_read(seg, CHIP_BASE(chip) + CySVRR);
        if (svrr == 0) continue;  /* nothing pending on this chip */

        /* Save current CAR (channel select) so we restore before exit */
        save_car = cy_read(seg, CHIP_BASE(chip) + CyCAR);

        /* ---- RX service ---- */
        if (svrr & 0x04) {
            save_xir = cy_read(seg, CHIP_BASE(chip) + CyRIR);
            chan = save_xir & 0x03;
            cy_write(seg, CHIP_BASE(chip) + CyCAR, chan);
            ivr = cy_read(seg, CHIP_BASE(chip) + CyRIVR) & CyIVRMask;

            if (ivr == CyIVRRxOK) {
                pcbdcom_port_t *pp = card->channels[chip * 4 + chan];
                count = cy_read(seg, CHIP_BASE(chip) + CyRDCR);
                for (i = 0; i < count && pp && pp->open; i++) {
                    ch = cy_read(seg, CHIP_BASE(chip) + CyRDSR);
                    next = (pp->rx_head + 1) % pp->rx_size;
                    if (next != pp->rx_tail) {
                        pp->rx_buf[pp->rx_head] = ch;
                        pp->rx_head = next;
                    }
                }
            } else if (ivr == CyIVRRxEx) {
                /* RX exception — drain and discard for v1 */
                (void)cy_read(seg, CHIP_BASE(chip) + CyRDSR);
            }
            /* End-of-service: write RIR to acknowledge */
            cy_write(seg, CHIP_BASE(chip) + CyRIR, save_xir & 0x3F);
        }

        /* ---- TX service ---- */
        if (svrr & 0x02) {
            save_xir = cy_read(seg, CHIP_BASE(chip) + CyTIR);
            chan = save_xir & 0x03;
            cy_write(seg, CHIP_BASE(chip) + CyCAR, chan);
            {
                pcbdcom_port_t *pp = card->channels[chip * 4 + chan];
                /* Feed up to CyMAX_CHAR_FIFO (12) chars per TX interrupt */
                for (i = 0; i < 12 && pp && pp->open &&
                            pp->tx_head != pp->tx_tail; i++) {
                    cy_write(seg, CHIP_BASE(chip) + CyTDR,
                             pp->tx_buf[pp->tx_tail]);
                    pp->tx_tail = (pp->tx_tail + 1) % pp->tx_size;
                }
                /* If ring drained, mask TX interrupt */
                if (pp && pp->tx_head == pp->tx_tail) {
                    unsigned char srer = cy_read(seg, CHIP_BASE(chip) + CySRER);
                    cy_write(seg, CHIP_BASE(chip) + CySRER, srer & ~CyTxRdy);
                }
            }
            cy_write(seg, CHIP_BASE(chip) + CyTIR, save_xir & 0x3F);
        }

        /* ---- Modem status service ---- */
        if (svrr & 0x01) {
            save_xir = cy_read(seg, CHIP_BASE(chip) + CyMIR);
            chan = save_xir & 0x03;
            cy_write(seg, CHIP_BASE(chip) + CyCAR, chan);
            (void)cy_read(seg, CHIP_BASE(chip) + CyMISR);  /* clear */
            cy_write(seg, CHIP_BASE(chip) + CyMIR, save_xir & 0x3F);
        }

        /* Restore CAR */
        cy_write(seg, CHIP_BASE(chip) + CyCAR, save_car);
    }

    /* Card-level interrupt clear (write any value to Cy_ClrIntr) */
    cy_write(seg, Cy_ClrIntr, 0);
}

int cyclom_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int cyclom_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    cyclom_card_t *card = (cyclom_card_t *)p->backend_data;
    unsigned char chip = p->subport / CyPORTS_PER_CHIP;
    unsigned char chan = p->subport % CyPORTS_PER_CHIP;
    unsigned char srer;
    const unsigned char *in = (const unsigned char *)buf;
    unsigned int next;
    int i;
    int was_empty = (p->tx_head == p->tx_tail);

    for (i = 0; i < n; i++) {
        next = (p->tx_head + 1) % p->tx_size;
        if (next == p->tx_tail) break;
        p->tx_buf[p->tx_head] = in[i];
        p->tx_head = next;
    }
    if (was_empty && i > 0 && card) {
        cy_select(card->card_seg, chip, chan);
        srer = cy_read(card->card_seg, CHIP_BASE(chip) + CySRER);
        cy_write(card->card_seg, CHIP_BASE(chip) + CySRER, srer | CyTxRdy);
    }
    return i;
}

const pcbdcom_backend_t pcbdcom_cyclom_backend = {
    "CYCLOM",
    cyclom_card_get,
    cyclom_backend_probe,
    cyclom_backend_init,
    cyclom_backend_deinit,
    cyclom_backend_isr,
    cyclom_backend_read,
    cyclom_backend_write
};

/* ----- TODO for v1.1 -----
 *  1. Multi-chip card: init flow currently assumes chip=0, chan=0 per
 *     port. Config parser needs to pass sub-port index; then chip =
 *     subport / 4, chan = subport % 4.
 *  2. cyclom_card_init() helper: allocate cyclom_card_t, wire card
 *     -> channels[] array so ISR can find pcbdcom_port_t for each
 *     (chip, chan). Currently ISR assumes it exists (via backend_data
 *     = card_t*), but pcbdcom.c parse_config sets backend_data = card_seg.
 *     Mismatch to reconcile in v1.1 wiring.
 *  3. Baud table beyond common rates (need cy_baud_table[] from Linux).
 *  4. RX error tagging (parity/framing/break) — current v1 drops them.
 * ---------------------------------------------------------------------- */
