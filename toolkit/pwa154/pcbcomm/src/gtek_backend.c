/* ============================================================================
 * gtek_backend.c — pcbcomm GTEK 8Fx backend
 *
 * Cards supported: GTEK BBS-550, BlackBoard-8A, and 8Fx variants.
 * These are dumb multi-port ISA cards with 16C550 UARTs and shared IRQ
 * (same architecture as Boca BB-1004/BB-2016).
 *
 * NOT the GTEK PCSS-8FX intelligent card (8032 processor) — that's
 * a different product. COMM-DRV's COMMDV06.DRV (1,662 bytes) is a
 * dumb-UART driver, not a smart-card protocol driver.
 *
 * GTEK 8Fx I/O layout:
 *   8 UARTs at contiguous I/O addresses (base + N*8)
 *   Shared IRQ with status register at base + 0x1F
 *   Status register: bit N set = port N has pending interrupt
 *
 * Reference: GTEK BBS-550 with Linux mini-HOWTO (Wajihuddin Ahmed, 1997),
 * setserial shared-IRQ documentation, Linux 8250 shared-IRQ support.
 * pcbirc crew (wrench), GPLv3.
 * ==========================================================================*/

#include <conio.h>
#include "pcbcomm.h"
#include "backend.h"
#include "uart.h"

#if defined(_MSC_VER)
# define GK_OUT(port, val) _outp((port), (val))
# define GK_IN(port)       (unsigned char)_inp((port))
#else
# define GK_OUT(port, val) outp((port), (val))
# define GK_IN(port)       (unsigned char)inp((port))
#endif

/* GTEK shared-IRQ status register: base + 0x1F for BBS-550.
 * Some variants use base + 0x40. Configurable at init. */
#define GTEK_IRQ_STATUS_OFF  0x1F

/* Max 8 ports per card, contiguous at base + N*8 */
#define GTEK_MAX_PORTS  8
#define GTEK_PORT_STEP  8

/* ----- Per-card state ----- */
typedef struct {
    unsigned long  addr;               /* pool: card iobase             */
    unsigned char  in_use;             /* pool                          */
    unsigned int   iobase;             /* Card base I/O                 */
    unsigned int   irq_status_port;    /* Shared IRQ status register    */
    unsigned char  n_ports;            /* 4 or 8                        */
    pcbcomm_port_t *ports[GTEK_MAX_PORTS];
} gtek_card_t;

#include "card_pool.h"
#define GTEK_MAX_CARDS 4
static gtek_card_t g_gtek_cards[GTEK_MAX_CARDS];

static void *gtek_card_get(unsigned long iobase)
{
    gtek_card_t *c = (gtek_card_t *)
        card_pool_get(g_gtek_cards, sizeof(gtek_card_t),
                      GTEK_MAX_CARDS, iobase);
    if (c && c->iobase == 0) {
        c->iobase = (unsigned int)iobase;
        c->irq_status_port = (unsigned int)iobase + GTEK_IRQ_STATUS_OFF;
        c->n_ports = GTEK_MAX_PORTS;
    }
    return c;
}

/* Per-port I/O base: card base + port_index * 8 */
static unsigned int gtek_port_io(gtek_card_t *card, unsigned char port)
{
    return card->iobase + (unsigned int)port * GTEK_PORT_STEP;
}

/* ----- Backend hooks ----- */

int gtek_backend_probe(pcbcomm_port_t *p)
{
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned int pio;
    unsigned char scratch;

    if (!card) return -1;
    pio = gtek_port_io(card, p->subport);

    /* Standard UART scratch register probe */
    GK_OUT(pio + UART_SCR, 0xA5);
    scratch = GK_IN(pio + UART_SCR);
    if (scratch != 0xA5) return -1;

    GK_OUT(pio + UART_SCR, 0x5A);
    scratch = GK_IN(pio + UART_SCR);
    return (scratch == 0x5A) ? 0 : -1;
}

int gtek_backend_init(pcbcomm_port_t *p)
{
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned int pio;
    uart_type_t t;

    if (!card || gtek_backend_probe(p) < 0) return -1;
    pio = gtek_port_io(card, p->subport);

    if (p->subport < GTEK_MAX_PORTS)
        card->ports[p->subport] = p;

    /* Detect UART type (8250/16450/16550/16550A) */
    t = uart_probe(pio);
    p->chip = t;

    /* 8N1 */
    uart_set_line(pio, LCR_8BITS | LCR_STOP1 | LCR_PAR_N);
    uart_set_baud(pio, p->baud);

    /* Enable FIFO on 16550A+ */
    if (t >= UART_TYPE_16550A)
        uart_set_fifo(pio, FCR_ENABLE | FCR_RXCLR | FCR_TXCLR | FCR_TRIG_8);

    /* Enable RX + line-status IRQ */
    GK_OUT(pio + UART_IER, IER_RDA | IER_LSR);

    /* DTR + RTS + OUT2 (shared IRQ) */
    GK_OUT(pio + UART_MCR, MCR_DTR | MCR_RTS | MCR_OUT2);

    p->open = 1;
    return 0;
}

void gtek_backend_deinit(pcbcomm_port_t *p)
{
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned int pio;

    if (!card) return;
    pio = gtek_port_io(card, p->subport);
    GK_OUT(pio + UART_IER, 0);
    GK_OUT(pio + UART_MCR, 0);
    p->open = 0;
}

/* ISR: read shared IRQ status register, service each pending port.
 * Bit N = port N has pending interrupt. Walk set bits. */
void gtek_backend_isr(pcbcomm_port_t *p)
{
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned char status, port_bit;
    unsigned int pio;
    unsigned char iir, lsr, ch, i;
    unsigned int next;
    pcbcomm_port_t *pp;

    if (!card) return;

    status = GK_IN(card->irq_status_port);
    if (status == 0) return;

    for (i = 0; i < card->n_ports; i++) {
        port_bit = 1 << i;
        if (!(status & port_bit)) continue;

        pp = card->ports[i];
        if (!pp || !pp->open) continue;
        pio = gtek_port_io(card, i);

        /* Service this port's UART — same loop as uart_backend */
        for (;;) {
            iir = GK_IN(pio + UART_IIR);
            if (iir & IIR_NONE) break;

            switch (iir & IIR_MASK) {
                case IIR_RDA:
                case IIR_TIMO:
                    while (GK_IN(pio + UART_LSR) & LSR_DR) {
                        ch = GK_IN(pio + UART_RBR);
                        next = (pp->rx_head + 1) % pp->rx_size;
                        if (next != pp->rx_tail) {
                            pp->rx_buf[pp->rx_head] = ch;
                            pp->rx_head = next;
                        }
                    }
                    break;

                case IIR_THRE:
                    while ((GK_IN(pio + UART_LSR) & LSR_THRE) &&
                           pp->tx_head != pp->tx_tail) {
                        GK_OUT(pio + UART_THR, pp->tx_buf[pp->tx_tail]);
                        pp->tx_tail = (pp->tx_tail + 1) % pp->tx_size;
                    }
                    if (pp->tx_head == pp->tx_tail) {
                        unsigned char ier = GK_IN(pio + UART_IER);
                        GK_OUT(pio + UART_IER, ier & ~IER_THRE);
                    }
                    break;

                case IIR_LSR:
                    lsr = GK_IN(pio + UART_LSR);
                    (void)lsr;
                    break;

                case IIR_MSR:
                    (void)GK_IN(pio + UART_MSR);
                    break;
            }
        }
    }
}

int gtek_backend_read(pcbcomm_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbcomm_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int gtek_backend_write(pcbcomm_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbcomm_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

const pcbcomm_backend_t pcbcomm_gtek_backend = {
    "GTEK",
    gtek_card_get,
    gtek_backend_probe,
    gtek_backend_init,
    gtek_backend_deinit,
    gtek_backend_isr,
    gtek_backend_read,
    gtek_backend_write
};
