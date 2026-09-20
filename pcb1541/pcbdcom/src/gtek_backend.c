/* ============================================================================
 * gtek_backend.c — pcbdcom GTEK BBS-550 / 8Fx dumb-multiport backend
 *
 * Cards supported (v1):
 *   GTEK BBS-550        8 ports, 16550 UARTs, 32-byte stride, shared IRQ
 *   GTEK PCSS-8         8 ports, same layout (rebranded/pre-BBS-550)
 *
 * Design: GTEK 8-port cards are 8 standard 16550 UARTs on one ISA
 * board with a shared interrupt. Each sub-port occupies 32 I/O bytes
 * (0x20 stride) starting at the card base — unusual compared to Boca
 * (8-byte stride) or HUB-6 (8-byte stride). The wider stride lets each
 * UART's decoding be simpler (fewer address-decode lines) and matches
 * GTEK's design for standard 74LS138-family address decoders.
 *
 * IRQ dispatch scans each UART's IIR to find which fired — same as
 * boca_backend.c and hub6_backend.c.
 *
 * Historical note: identified as COMMDV06.DRV in WCSC's COMMDRV.RED
 * (embedded string "GTEK(8Fx)   1.00"). GTEK was a popular 1990s
 * multiport vendor especially in the shareware/BBS world; their
 * cards competed on price against Boca and Digi.
 *
 * Reference: GTEK BBS-550 datasheet (public, distributed with card);
 * GTEK PCSS-8 pinout notes from period BBS documentation.
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include <conio.h>
#include "uart.h"
#include "pcbdcom.h"
#include "backend.h"
#include "card_pool.h"

#if defined(_MSC_VER)
# define GTEK_IN(port)  (unsigned char)_inp((port))
#else
# define GTEK_IN(port)  (unsigned char)inp((port))
#endif

/* GTEK layout constants (all public, from GTEK datasheets) */
#define GTEK_PORT_STRIDE  0x20      /* each sub-UART occupies 32 I/O bytes  */
#define GTEK_MAX_PORTS    8         /* 8 UARTs per card                     */
#define GTEK_IIR          0x02      /* UART Interrupt Identification Reg    */
#define GTEK_IIR_NONE     0x01      /* bit 0 set = "no interrupt pending"   */

/* Per-card state, keyed by card base I/O address */
typedef struct {
    unsigned long   addr;                    /* pool header: card base I/O   */
    unsigned char   in_use;                  /* pool header                  */
    unsigned int    base;                    /* I/O base (typ. 0x100/180..)  */
    pcbdcom_port_t *sub[GTEK_MAX_PORTS];     /* 8 sub-port slots             */
} gtek_card_t;

#define GTEK_MAX_CARDS 4
static gtek_card_t g_gtek_cards[GTEK_MAX_CARDS];

/* card_get: called by parse_config() before first port on card is init'd */
static void *gtek_card_get(unsigned long card_addr)
{
    gtek_card_t *c = (gtek_card_t *)
        card_pool_get(g_gtek_cards, sizeof(gtek_card_t),
                      GTEK_MAX_CARDS, card_addr);
    if (c && c->base == 0)
        c->base = (unsigned int)card_addr;
    return c;
}

/* ----- Backend hooks ----- */

int gtek_backend_probe(pcbdcom_port_t *p)
{
    extern int uart_backend_probe(pcbdcom_port_t *);
    return uart_backend_probe(p);
}

int gtek_backend_init(pcbdcom_port_t *p)
{
    extern int uart_backend_init(pcbdcom_port_t *);
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned int slot;

    if (card) {
        slot = (p->base - card->base) / GTEK_PORT_STRIDE;
        if (slot < GTEK_MAX_PORTS)
            card->sub[slot] = p;
    }
    return uart_backend_init(p);
}

void gtek_backend_deinit(pcbdcom_port_t *p)
{
    extern void uart_backend_deinit(pcbdcom_port_t *);
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned int slot;

    if (card) {
        slot = (p->base - card->base) / GTEK_PORT_STRIDE;
        if (slot < GTEK_MAX_PORTS)
            card->sub[slot] = 0;
    }
    uart_backend_deinit(p);
}

/* Shared-IRQ dispatch — same do/while(serviced) pattern as boca/hub6 */
void gtek_backend_isr(pcbdcom_port_t *p)
{
    extern void uart_backend_isr(pcbdcom_port_t *);
    gtek_card_t *card = (gtek_card_t *)p->backend_data;
    unsigned int i, sub_base;
    unsigned char iir;
    int serviced;

    if (!card) return;

    do {
        serviced = 0;
        for (i = 0; i < GTEK_MAX_PORTS; i++) {
            pcbdcom_port_t *sp = card->sub[i];
            if (!sp) continue;
            sub_base = card->base + i * GTEK_PORT_STRIDE;
            iir = GTEK_IN(sub_base + GTEK_IIR);
            if ((iir & GTEK_IIR_NONE) == 0) {
                uart_backend_isr(sp);
                serviced = 1;
            }
        }
    } while (serviced);
}

int gtek_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int gtek_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbdcom_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_gtek_backend = {
    "GTEK",
    gtek_card_get,
    gtek_backend_probe,
    gtek_backend_init,
    gtek_backend_deinit,
    gtek_backend_isr,
    gtek_backend_read,
    gtek_backend_write
};

/* end gtek_backend.c */
