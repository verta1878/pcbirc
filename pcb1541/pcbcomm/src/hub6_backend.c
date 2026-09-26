/* ============================================================================
 * hub6_backend.c — pcbdcom Intel HUB-6 / IBM 8-port async adapter backend
 *
 * Cards supported (v1):
 *   Intel HUB-6         8 ports, base + N*8, one shared IRQ
 *   IBM 8-Port Async    8 ports, same register layout (Intel-designed)
 *
 * Design: HUB-6 is 8 standard 8250/16450/16550 UARTs on one ISA board
 * sharing a single interrupt. Each sub-port occupies 8 I/O bytes
 * starting at the card base. Interrupt dispatch scans each UART's IIR
 * to find which one fired — same pattern as boca_backend.c.
 *
 * Historical note: identified as COMMDV01.DRV in WCSC's COMMDRV.RED
 * (embedded string "INTEL HUB6  1.00"). Common on early BBSes because
 * Intel's design was well-documented and multiple vendors shipped
 * compatible boards.
 *
 * Reference: Intel HUB-6 datasheet (public since 1988); IBM 8-Port
 * Asynchronous Adapter/A Hardware Technical Reference (public);
 * NetBSD sys/dev/isa/com_multi.c layout notes.
 *
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include <conio.h>
#include "uart.h"
#include "pcbdcom.h"
#include "backend.h"
#include "card_pool.h"

#if defined(_MSC_VER)
# define HUB6_IN(port)  (unsigned char)_inp((port))
#else
# define HUB6_IN(port)  (unsigned char)inp((port))
#endif

/* HUB-6 layout constants (all public, from Intel/IBM datasheets) */
#define HUB6_PORT_STRIDE  0x08      /* each sub-UART occupies 8 I/O bytes */
#define HUB6_MAX_PORTS    8         /* 8 UARTs per card, always           */
#define HUB6_IIR          0x02      /* UART Interrupt Identification Reg  */
#define HUB6_IIR_NONE     0x01      /* bit 0 set = "no interrupt pending" */

/* Per-card state, keyed by card base I/O address */
typedef struct {
    unsigned long   addr;                    /* pool header: card base I/O   */
    unsigned char   in_use;                  /* pool header                  */
    unsigned int    base;                    /* I/O base (typ. 0x100/140..)  */
    pcbdcom_port_t *sub[HUB6_MAX_PORTS];     /* 8 sub-port slots             */
} hub6_card_t;

#define HUB6_MAX_CARDS 4
static hub6_card_t g_hub6_cards[HUB6_MAX_CARDS];

/* card_get: called by parse_config() before first port on card is init'd */
static void *hub6_card_get(unsigned long card_addr)
{
    hub6_card_t *c = (hub6_card_t *)
        card_pool_get(g_hub6_cards, sizeof(hub6_card_t),
                      HUB6_MAX_CARDS, card_addr);
    if (c && c->base == 0)
        c->base = (unsigned int)card_addr;
    return c;
}

/* ----- Backend hooks ----- */

int hub6_backend_probe(pcbdcom_port_t *p)
{
    extern int uart_backend_probe(pcbdcom_port_t *);
    return uart_backend_probe(p);
}

int hub6_backend_init(pcbdcom_port_t *p)
{
    extern int uart_backend_init(pcbdcom_port_t *);
    hub6_card_t *card = (hub6_card_t *)p->backend_data;
    unsigned int slot;

    if (card) {
        slot = (p->base - card->base) / HUB6_PORT_STRIDE;
        if (slot < HUB6_MAX_PORTS)
            card->sub[slot] = p;
    }
    return uart_backend_init(p);
}

void hub6_backend_deinit(pcbdcom_port_t *p)
{
    extern void uart_backend_deinit(pcbdcom_port_t *);
    hub6_card_t *card = (hub6_card_t *)p->backend_data;
    unsigned int slot;

    if (card) {
        slot = (p->base - card->base) / HUB6_PORT_STRIDE;
        if (slot < HUB6_MAX_PORTS)
            card->sub[slot] = 0;
    }
    uart_backend_deinit(p);
}

/* Shared-IRQ dispatch: scan all 8 sub-UARTs, service any pending.
 * Matches boca_backend_isr()'s do/while(serviced) pattern to handle
 * back-to-back interrupts that fire during service. */
void hub6_backend_isr(pcbdcom_port_t *p)
{
    extern void uart_backend_isr(pcbdcom_port_t *);
    hub6_card_t *card = (hub6_card_t *)p->backend_data;
    unsigned int i, sub_base;
    unsigned char iir;
    int serviced;

    if (!card) return;

    do {
        serviced = 0;
        for (i = 0; i < HUB6_MAX_PORTS; i++) {
            pcbdcom_port_t *sp = card->sub[i];
            if (!sp) continue;
            sub_base = card->base + i * HUB6_PORT_STRIDE;
            iir = HUB6_IN(sub_base + HUB6_IIR);
            if ((iir & HUB6_IIR_NONE) == 0) {
                uart_backend_isr(sp);
                serviced = 1;
            }
        }
    } while (serviced);
}

int hub6_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int hub6_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbdcom_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_hub6_backend = {
    "HUB6",
    hub6_card_get,
    hub6_backend_probe,
    hub6_backend_init,
    hub6_backend_deinit,
    hub6_backend_isr,
    hub6_backend_read,
    hub6_backend_write
};

/* end hub6_backend.c */
