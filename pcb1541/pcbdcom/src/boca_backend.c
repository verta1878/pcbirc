/* ============================================================================
 * boca_backend.c — pcbdcom Boca dumb multiport backend
 *
 * Cards supported (v1):
 *   BB-1004    4 ports,  base+0x00..0x18 step 0x08, one shared IRQ
 *   BB-1008    8 ports,  base+0x00..0x38 step 0x08, one shared IRQ
 *   BB-2016   16 ports,  base+0x00..0x78 step 0x08, two IRQs (8 ports each)
 *
 * Design: Boca cards are just multiple 16550A UARTs at documented I/O
 * offsets sharing an IRQ. This backend registers N sub-ports as
 * standard UARTs (see uart_backend.c) and installs a shared-IRQ
 * dispatcher that polls each port's IIR to find which fired.
 *
 * Reference: NetBSD sys/dev/isa/boca.c (BSD-licensed, describes register
 * layout); crew hardware notes on BB-2016 grouping.
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include <conio.h>
#include "uart.h"
#include "pcbdcom.h"
#include "backend.h"

#if defined(_MSC_VER)
# define BOCA_IN(port)  (unsigned char)_inp((port))
#else
# define BOCA_IN(port)  (unsigned char)inp((port))
#endif

/* Boca sub-port I/O offset from card base. Same 8-byte stride for
 * every documented Boca model. */
#define BOCA_PORT_STRIDE  0x08

/* Card models: number of sub-ports per shared-IRQ group */
typedef enum {
    BOCA_BB_1004 = 4,
    BOCA_BB_1008 = 8,
    BOCA_BB_2016 = 16   /* two 8-port groups, second group has its own IRQ */
} boca_model_t;

/* Per-card state: which sub-ports exist and their pcbdcom_port_t
 * pointers, for IRQ dispatch. Attached via backend_data. */
typedef struct {
    unsigned long  addr;                    /* pool header: card base I/O    */
    unsigned char  in_use;                  /* pool header                   */
    boca_model_t   model;
    unsigned int   base;
    unsigned char  group_size;              /* 4 or 8 */
    pcbdcom_port_t *sub[8];                 /* max 8 per IRQ group */
} boca_card_t;

#include "card_pool.h"
#define BOCA_MAX_CARDS 4
static boca_card_t g_boca_cards[BOCA_MAX_CARDS];

static void *boca_card_get(unsigned long card_addr)
{
    boca_card_t *c = (boca_card_t *)
        card_pool_get(g_boca_cards, sizeof(boca_card_t),
                      BOCA_MAX_CARDS, card_addr);
    if (c && c->group_size == 0) {
        /* First-time init: address = card base; assume BB-1008 (8-port)
         * unless a later port on same card overrides. Boca has no card-
         * type register — sysop config picks the model. */
        c->base = (unsigned int)card_addr;
        c->model = BOCA_BB_1008;
        c->group_size = 8;
    }
    return c;
}

/* ----- Backend hooks — each sub-port looks like a plain UART ----- */

int boca_backend_probe(pcbdcom_port_t *p)
{
    /* Each sub-port is a plain 16550. Delegate to the UART probe. */
    uart_type_t t = uart_probe(p->base);
    p->chip = t;
    /* Boca BB-* ships 16550A across all documented models. If we don't
     * see one, either the card isn't installed or the base is wrong. */
    return (t >= UART_TYPE_16550A) ? 0 : -1;
}

int boca_backend_init(pcbdcom_port_t *p)
{
    boca_card_t *card = (boca_card_t *)p->backend_data;
    extern int uart_backend_init(pcbdcom_port_t *);
    /* Register in card's sub-port table */
    if (card && p->subport < card->group_size)
        card->sub[p->subport] = p;
    return uart_backend_init(p);
}

void boca_backend_deinit(pcbdcom_port_t *p)
{
    extern void uart_backend_deinit(pcbdcom_port_t *);
    uart_backend_deinit(p);
}

/* Shared-IRQ dispatcher — called once per interrupt for the card's
 * IRQ group. Walks the sub-ports, dispatches to the standard UART ISR
 * on any port whose IIR reports pending. Boca hardware does NOT
 * provide a card-level "which port fired" register — poll is the way. */
void boca_backend_isr(pcbdcom_port_t *p)
{
    boca_card_t *card = (boca_card_t *)p->backend_data;
    extern void uart_backend_isr(pcbdcom_port_t *);
    int i, serviced;

    if (!card) {
        /* Fallback: no card record — treat as single UART */
        uart_backend_isr(p);
        return;
    }

    /* Loop until no port in the group has pending — shared IRQ may
     * re-fire during service if a second port raises mid-loop. */
    do {
        serviced = 0;
        for (i = 0; i < card->group_size; i++) {
            pcbdcom_port_t *sp = card->sub[i];
            if (!sp || !sp->open) continue;
            if ((BOCA_IN(sp->base + UART_IIR) & IIR_NONE) == 0) {
                uart_backend_isr(sp);
                serviced = 1;
            }
        }
    } while (serviced);
}

int boca_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int boca_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbdcom_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_boca_backend = {
    "BOCA",
    boca_card_get,
    boca_backend_probe,
    boca_backend_init,
    boca_backend_deinit,
    boca_backend_isr,
    boca_backend_read,
    boca_backend_write
};

/* ----- Helper: build a Boca card record for the PCBDCOM.CFG parser ----- *
 * Called from src/pcbdcom.c when config file names a BOCA_* card. Fills
 * a boca_card_t and points each sub-port's backend_data at the card so
 * the shared-IRQ ISR can find its peers. */
int boca_card_init(boca_card_t *card, boca_model_t model, unsigned int base,
                   pcbdcom_port_t *sub_ports)
{
    int i;
    card->model = model;
    card->base = base;
    card->group_size = (model == BOCA_BB_2016) ? 8 : (unsigned char)model;

    for (i = 0; i < card->group_size; i++) {
        sub_ports[i].base = base + (i * BOCA_PORT_STRIDE);
        sub_ports[i].backend = &pcbdcom_boca_backend;
        sub_ports[i].backend_data = card;
        card->sub[i] = &sub_ports[i];
    }
    return 0;
}
