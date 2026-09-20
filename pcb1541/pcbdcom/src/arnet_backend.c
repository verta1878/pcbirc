/* ============================================================================
 * arnet_backend.c — pcbdcom Arnet SmartPort / SmartPort Plus backend
 *
 * Cards supported:
 *   Arnet SmartPort 4/8/16 (ISA, dumb multi-port with 82530 UART chips)
 *   Arnet SmartPort Plus 4/8 (ISA, on-card 8032 firmware for buffering)
 *
 * Public references:
 *   - Arnet SmartPort register layout (from published Arnet datasheets and
 *     the ARNETSP4.DAT / ARNETSP8.DAT config files shipped in PCBoard's
 *     INSTALL.zip, which are plain-text register descriptions)
 *   - Linux 8250_port.c for the underlying UART programming model
 *   - PCBoard MODEMDRV.C for the observed behavior expected by callers
 *
 * Architecture:
 *   SmartPort:      Base I/O + N * 8 bytes per port (16550-compatible UART
 *                    with card-level IRQ multiplexer at Base + 0x40)
 *   SmartPort Plus: Base I/O with 8032 firmware; access via mailbox
 *                    protocol at Base + 0x00..0x07; buffered per-port I/O
 *                    at Base + 0x10..0x1F
 *
 * Original MODEMDRV.C notes report a specific behavior for old SmartPort
 * (non-Plus): interrupts don't reliably update inbuf_count, so callers had
 * to force a getpacket() to refresh. We handle this transparently in the
 * read() path.
 *
 * License: GPLv3
 * ==========================================================================*/
#include "pcbdcom.h"
#include "backend.h"
#include "uart.h"
#include "card_pool.h"
#include <stddef.h>
#include "compat.h"
#include <conio.h>

/* I/O primitives — same style as other backends */
#if defined(_MSC_VER)
# define ARNET_OUT(port, val) _outp((port), (val))
# define ARNET_IN(port)       (unsigned char)_inp((port))
#else
# define ARNET_OUT(port, val) outp((port), (val))
# define ARNET_IN(port)       (unsigned char)inp((port))
#endif


/* Arnet register offsets (from ARNETSP*.DAT).
 * Base I/O per card is configured in PCBDCOM.CFG. */
#define ARNET_UART_STRIDE   8       /* bytes between per-port UART registers  */
#define ARNET_IRQ_ID_REG    0x40    /* card-level IRQ identify (read-clear)   */
#define ARNET_IRQ_EN_REG    0x41    /* card-level per-port IRQ enable mask    */
#define ARNET_CARD_CTL      0x42    /* card control (bit 0: enable, bit 7: reset) */

/* SmartPort Plus mailbox — offset from Base I/O */
#define ARNETPLUS_MBOX_CMD  0x00    /* write command byte                     */
#define ARNETPLUS_MBOX_STAT 0x01    /* read status byte                       */
#define ARNETPLUS_MBOX_DATA 0x02    /* 2-byte data word                       */
#define ARNETPLUS_PORT_BASE 0x10    /* per-port I/O offset base               */
#define ARNETPLUS_PORT_STRIDE 8

/* Per-card state */
typedef struct arnet_card {
    unsigned int io_base;           /* card I/O base                          */
    unsigned char n_ports;          /* number of ports on this card           */
    unsigned char is_plus;          /* 0 = SmartPort, 1 = SmartPort Plus      */
    unsigned char irq;              /* card IRQ (shared across ports)         */
    unsigned char irq_mask;         /* per-port IRQ enable bitmap             */
} arnet_card_t;

static arnet_card_t arnet_cards[PCBDCOM_MAX_CARDS];
static unsigned char n_arnet_cards = 0;

/* Look up per-card state by base I/O */
static arnet_card_t *arnet_find_card(unsigned long io_base)
{
    unsigned char i;
    for (i = 0; i < n_arnet_cards; i++) {
        if (arnet_cards[i].io_base == (unsigned int)io_base)
            return &arnet_cards[i];
    }
    return NULL;
}

/* card_get: called by parse_config() before first port init on this card.
 * Allocates card state and configures card-level registers. */
static void *arnet_card_get(unsigned long card_addr)
{
    arnet_card_t *c = arnet_find_card(card_addr);
    if (c) return c;

    if (n_arnet_cards >= PCBDCOM_MAX_CARDS)
        return NULL;

    c = &arnet_cards[n_arnet_cards++];
    c->io_base = (unsigned int)card_addr;
    c->n_ports = 0;
    c->is_plus = 0;  /* Auto-detected in probe() */
    c->irq = 0;
    c->irq_mask = 0;

    /* Reset card: bit 7 of CARD_CTL */
    ARNET_OUT(c->io_base + ARNET_CARD_CTL, 0x80);
    /* Small delay via port read (~4us on ISA) */
    (void)ARNET_IN(c->io_base + ARNET_CARD_CTL);
    ARNET_OUT(c->io_base + ARNET_CARD_CTL, 0x01);  /* enable, no reset */

    return c;
}

/* Detect SmartPort vs SmartPort Plus by mailbox probe */
static int arnet_is_plus(unsigned int io_base)
{
    /* Plus firmware responds to command 0x00 (get version) with 0xAA in status.
     * Non-Plus (dumb) leaves the register floating; read returns 0xFF. */
    ARNET_OUT(io_base + ARNETPLUS_MBOX_CMD, 0x00);
    /* Give firmware time to respond (typical 200us) */
    {
        int i;
        for (i = 0; i < 100; i++) (void)ARNET_IN(io_base + ARNETPLUS_MBOX_STAT);
    }
    return (ARNET_IN(io_base + ARNETPLUS_MBOX_STAT) == 0xAA);
}

static int arnet_probe(pcbdcom_port_t *p)
{
    arnet_card_t *c = (arnet_card_t *)p->card_state;
    unsigned int port_io;

    if (!c) return 0;

    /* First-port probe: detect card variant */
    if (c->n_ports == 0)
        c->is_plus = (unsigned char)arnet_is_plus(c->io_base);

    if (c->is_plus)
        port_io = c->io_base + ARNETPLUS_PORT_BASE + p->subport * ARNETPLUS_PORT_STRIDE;
    else
        port_io = c->io_base + p->subport * ARNET_UART_STRIDE;

    /* Poke UART scratch register (offset 7) to verify presence */
    ARNET_OUT(port_io + 7, 0x55);
    if (ARNET_IN(port_io + 7) != 0x55) return 0;
    ARNET_OUT(port_io + 7, 0xAA);
    if (ARNET_IN(port_io + 7) != 0xAA) return 0;
    return 1;
}

static int arnet_init(pcbdcom_port_t *p)
{
    arnet_card_t *c = (arnet_card_t *)p->card_state;
    unsigned int port_io;

    if (!c) return -1;

    if (c->is_plus)
        port_io = c->io_base + ARNETPLUS_PORT_BASE + p->subport * ARNETPLUS_PORT_STRIDE;
    else
        port_io = c->io_base + p->subport * ARNET_UART_STRIDE;

    p->base = port_io;
    p->irq = c->irq;

    /* Standard 8250/16550 init: 9600 8N1, FIFO on */
    uart_backend_init(p);

    /* Enable this port's IRQ at the card level */
    c->irq_mask |= (1 << p->subport);
    ARNET_OUT(c->io_base + ARNET_IRQ_EN_REG, c->irq_mask);
    c->n_ports++;

    return 0;
}

static void arnet_deinit(pcbdcom_port_t *p)
{
    arnet_card_t *c = (arnet_card_t *)p->card_state;
    if (!c) return;

    /* Disable this port at card level */
    c->irq_mask &= ~(1 << p->subport);
    ARNET_OUT(c->io_base + ARNET_IRQ_EN_REG, c->irq_mask);
    if (c->irq_mask == 0)
        ARNET_OUT(c->io_base + ARNET_CARD_CTL, 0x00);  /* disable card */
}

static void arnet_isr(pcbdcom_port_t *p)
{
    arnet_card_t *c = (arnet_card_t *)p->card_state;
    unsigned char pending;
    unsigned int port_io;
    unsigned char i;

    if (!c) return;

    /* Read card-level IRQ identify — one bit per port that has activity */
    pending = ARNET_IN(c->io_base + ARNET_IRQ_ID_REG);
    while (pending) {
        for (i = 0; i < 8; i++) {
            if (!(pending & (1 << i))) continue;
            if (c->is_plus)
                port_io = c->io_base + ARNETPLUS_PORT_BASE + i * ARNETPLUS_PORT_STRIDE;
            else
                port_io = c->io_base + i * ARNET_UART_STRIDE;
            {
                pcbdcom_port_t tmp;
                tmp.base = port_io;
                tmp.subport = i;
                tmp.card_state = c;
                uart_backend_isr(&tmp);
            }
        }
        pending = ARNET_IN(c->io_base + ARNET_IRQ_ID_REG);
    }
}

static int arnet_read(pcbdcom_port_t *p, void *buf, int n)
{
    /* Old (non-Plus) SmartPort quirk from MODEMDRV.C: interrupts don't
     * always update the buffered count reliably. Force a hardware read
     * to refresh before returning. */
    arnet_card_t *c = (arnet_card_t *)p->card_state;
    if (c && !c->is_plus) {
        /* Poll UART LSR to refresh state */
        (void)ARNET_IN(p->base + 5);  /* LSR = base + 5 */
    }
    return uart_backend_read(p, buf, n);
}

static int arnet_write(pcbdcom_port_t *p, const void *buf, int n)
{
    return uart_backend_write(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_arnet_backend = {
    "ARNETSPP",
    arnet_card_get,
    arnet_probe,
    arnet_init,
    arnet_deinit,
    arnet_isr,
    arnet_read,
    arnet_write
};
