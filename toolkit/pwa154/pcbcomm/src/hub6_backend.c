/* ============================================================================
 * hub6_backend.c — pcbcomm Intel HUB6 backend
 *
 * Cards supported: Intel HUB6 ISA (6-port, shared IRQ).
 *
 * The HUB6 is a standard 8250/16550 UART card with a port-select
 * multiplexer. All 6 ports share a single I/O base (0x302) and IRQ (3).
 * Access method: write (card<<6 | port<<3 | 1) to iobase, then read/write
 * the UART register at iobase+1.
 *
 * Ported from Linux drivers/tty/serial/8250/8250_hub6.c (GPLv2,
 * Russell King) and hub6_serial_in/out in 8250.c.
 * DOS adaptations: pcbirc crew (wrench), GPLv3.
 * ==========================================================================*/

#include <conio.h>
#include "pcbcomm.h"
#include "backend.h"
#include "uart.h"

#if defined(_MSC_VER)
# define HUB_OUT(port, val) _outp((port), (val))
# define HUB_IN(port)       (unsigned char)_inp((port))
#else
# define HUB_OUT(port, val) outp((port), (val))
# define HUB_IN(port)       (unsigned char)inp((port))
#endif

/* Default HUB6 base I/O and IRQ (from Linux 8250_hub6.c) */
#define HUB6_IOBASE  0x302
#define HUB6_IRQ     3
#define HUB6_CLOCK   1843200

/* HUB6 port-select: (card << 6 | port << 3 | 1) written to iobase.
 * Then UART registers at iobase+1. */
#define HUB6_SELECT(card, port) (((card) << 6) | ((port) << 3) | 1)

/* Max: 2 cards x 6 ports = 12 ports */
#define HUB6_MAX_CARDS   2
#define HUB6_PORTS_PER   6

/* Read/write a UART register through the HUB6 multiplexer */
static unsigned char hub6_read(unsigned int iobase, unsigned char hub6_sel,
                               unsigned char reg)
{
    HUB_OUT(iobase, hub6_sel - 1 + reg);
    return HUB_IN(iobase + 1);
}

static void hub6_write(unsigned int iobase, unsigned char hub6_sel,
                       unsigned char reg, unsigned char val)
{
    HUB_OUT(iobase, hub6_sel - 1 + reg);
    HUB_OUT(iobase + 1, val);
}

/* ----- Backend hooks ----- */

int hub6_backend_probe(pcbcomm_port_t *p)
{
    unsigned char card = p->subport / HUB6_PORTS_PER;
    unsigned char port = p->subport % HUB6_PORTS_PER;
    unsigned char sel = HUB6_SELECT(card, port);
    unsigned char scratch;

    /* Probe: write scratch register, read back. Standard UART detect. */
    hub6_write(p->base, sel, UART_SCR, 0xA5);
    scratch = hub6_read(p->base, sel, UART_SCR);
    if (scratch != 0xA5) return -1;

    hub6_write(p->base, sel, UART_SCR, 0x5A);
    scratch = hub6_read(p->base, sel, UART_SCR);
    if (scratch != 0x5A) return -1;

    return 0;
}

int hub6_backend_init(pcbcomm_port_t *p)
{
    unsigned char card = p->subport / HUB6_PORTS_PER;
    unsigned char port = p->subport % HUB6_PORTS_PER;
    unsigned char sel = HUB6_SELECT(card, port);
    unsigned int div;

    if (hub6_backend_probe(p) < 0) return -1;

    if (p->base == 0) p->base = HUB6_IOBASE;

    /* 8N1 */
    hub6_write(p->base, sel, UART_LCR, LCR_8BITS | LCR_STOP1 | LCR_PAR_N);

    /* Baud divisor: clock / (16 * baud) */
    if (p->baud <= 0) p->baud = 9600;
    div = (unsigned int)(HUB6_CLOCK / (16L * p->baud));
    hub6_write(p->base, sel, UART_LCR, 0x80);  /* DLAB on */
    hub6_write(p->base, sel, UART_DLL, (unsigned char)(div & 0xFF));
    hub6_write(p->base, sel, UART_DLH, (unsigned char)(div >> 8));
    hub6_write(p->base, sel, UART_LCR, LCR_8BITS | LCR_STOP1 | LCR_PAR_N);

    /* Enable RX + line-status IRQ */
    hub6_write(p->base, sel, UART_IER, IER_RDA | IER_LSR);

    /* DTR + RTS + OUT2 */
    hub6_write(p->base, sel, UART_MCR, MCR_DTR | MCR_RTS | MCR_OUT2);

    p->open = 1;
    return 0;
}

void hub6_backend_deinit(pcbcomm_port_t *p)
{
    unsigned char card = p->subport / HUB6_PORTS_PER;
    unsigned char port = p->subport % HUB6_PORTS_PER;
    unsigned char sel = HUB6_SELECT(card, port);

    hub6_write(p->base, sel, UART_IER, 0);
    hub6_write(p->base, sel, UART_MCR, 0);
    p->open = 0;
}

void hub6_backend_isr(pcbcomm_port_t *p)
{
    unsigned char card = p->subport / HUB6_PORTS_PER;
    unsigned char port = p->subport % HUB6_PORTS_PER;
    unsigned char sel = HUB6_SELECT(card, port);
    unsigned char iir, lsr, ch;
    unsigned int next;

    for (;;) {
        iir = hub6_read(p->base, sel, UART_IIR);
        if (iir & IIR_NONE) return;

        switch (iir & IIR_MASK) {
            case IIR_RDA:
            case IIR_TIMO:
                while (hub6_read(p->base, sel, UART_LSR) & LSR_DR) {
                    ch = hub6_read(p->base, sel, UART_RBR);
                    next = (p->rx_head + 1) % p->rx_size;
                    if (next != p->rx_tail) {
                        p->rx_buf[p->rx_head] = ch;
                        p->rx_head = next;
                    }
                }
                break;

            case IIR_THRE:
                while ((hub6_read(p->base, sel, UART_LSR) & LSR_THRE) &&
                       p->tx_head != p->tx_tail) {
                    hub6_write(p->base, sel, UART_THR,
                               p->tx_buf[p->tx_tail]);
                    p->tx_tail = (p->tx_tail + 1) % p->tx_size;
                }
                if (p->tx_head == p->tx_tail) {
                    unsigned char ier = hub6_read(p->base, sel, UART_IER);
                    hub6_write(p->base, sel, UART_IER, ier & ~IER_THRE);
                }
                break;

            case IIR_LSR:
                lsr = hub6_read(p->base, sel, UART_LSR);
                (void)lsr;
                break;

            case IIR_MSR:
                (void)hub6_read(p->base, sel, UART_MSR);
                break;
        }
    }
}

int hub6_backend_read(pcbcomm_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbcomm_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

int hub6_backend_write(pcbcomm_port_t *p, const void *buf, int n)
{
    extern int uart_backend_write(pcbcomm_port_t *, const void *, int);
    return uart_backend_write(p, buf, n);
}

const pcbcomm_backend_t pcbcomm_hub6_backend = {
    "HUB6",
    0,                         /* no per-card state — select via subport */
    hub6_backend_probe,
    hub6_backend_init,
    hub6_backend_deinit,
    hub6_backend_isr,
    hub6_backend_read,
    hub6_backend_write
};
