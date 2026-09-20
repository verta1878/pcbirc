/* ============================================================================
 * uart_backend.c — pcbdcom backend wrapper for standard 8250-family UARTs
 *
 * Ties uart.c (low-level register access, from Linux 8250_port.c) to
 * the pcbdcom_backend_t interface (see inc/backend.h).
 *
 * Ported from Linux drivers/tty/serial/8250/8250_core.c handling of
 * uart_port callbacks, GPLv2 (Russell King, Alan Cox, Ted Ts'o).
 * DOS 16-bit adaptations: pcbirc crew, GPLv3.
 *
 * IRQ handler design:
 *   - Read IIR in a loop, dispatch by type until IIR reports no pending
 *   - RX path drains RBR into rx ring, drops on overrun
 *   - TX path fills THR from tx ring, masks THRE IRQ when ring empty
 *   - Ring buffers safe for single-producer/single-consumer with
 *     16-bit head/tail indices (atomic on 16-bit target)
 * ==========================================================================*/

#include <conio.h>
#include "uart.h"
#include "pcbdcom.h"

#if defined(_MSC_VER)
# define UART_OUT(port, val) _outp((port), (val))
# define UART_IN(port)       (unsigned char)_inp((port))
#else
# define UART_OUT(port, val) outp((port), (val))
# define UART_IN(port)       (unsigned char)inp((port))
#endif

int uart_backend_probe(pcbdcom_port_t *p)
{
    uart_type_t t = uart_probe(p->base);
    p->chip = t;
    return (t != UART_TYPE_NONE) ? 0 : -1;
}

int uart_backend_init(pcbdcom_port_t *p)
{
    if (uart_backend_probe(p) < 0) return -1;

    uart_set_line(p->base, p->lcr ? p->lcr : (LCR_8BITS | LCR_STOP1 | LCR_PAR_N));
    uart_set_baud(p->base, p->baud);

    if (p->chip >= UART_TYPE_16550A)
        uart_set_fifo(p->base,
            FCR_ENABLE | FCR_RXCLR | FCR_TXCLR | FCR_TRIG_8);

    /* Enable RX + line-status IRQs; THRE enabled on demand when TX ring nonempty */
    UART_OUT(p->base + UART_IER, IER_RDA | IER_LSR);

    /* DTR + RTS + OUT2 (IRQ line enable on PC/AT) */
    UART_OUT(p->base + UART_MCR, MCR_DTR | MCR_RTS | MCR_OUT2);

    p->open = 1;
    return 0;
}

void uart_backend_deinit(pcbdcom_port_t *p)
{
    UART_OUT(p->base + UART_IER, 0);
    UART_OUT(p->base + UART_MCR, 0);
    p->open = 0;
}

/* IRQ handler — called from irq.c dispatch for this port's IRQ line */
void uart_backend_isr(pcbdcom_port_t *p)
{
    unsigned char iir, lsr, ch;
    unsigned int next;

    for (;;) {
        iir = UART_IN(p->base + UART_IIR);
        if (iir & IIR_NONE) return;

        switch (iir & IIR_MASK) {
            case IIR_RDA:
            case IIR_TIMO:
                while (uart_rx_ready(p->base)) {
                    ch = uart_rx(p->base);
                    next = (p->rx_head + 1) % p->rx_size;
                    if (next != p->rx_tail) {
                        p->rx_buf[p->rx_head] = ch;
                        p->rx_head = next;
                    }
                }
                break;

            case IIR_THRE:
                while (uart_tx_ready(p->base) && p->tx_head != p->tx_tail) {
                    uart_tx(p->base, p->tx_buf[p->tx_tail]);
                    p->tx_tail = (p->tx_tail + 1) % p->tx_size;
                }
                if (p->tx_head == p->tx_tail) {
                    unsigned char ier = UART_IN(p->base + UART_IER);
                    UART_OUT(p->base + UART_IER, ier & ~IER_THRE);
                }
                break;

            case IIR_LSR:
                lsr = UART_IN(p->base + UART_LSR);
                (void)lsr;  /* v1: log-and-continue */
                break;

            case IIR_MSR:
                (void)UART_IN(p->base + UART_MSR);
                break;
        }
    }
}

/* Non-blocking read: drain from rx ring into caller buffer, return count */
int uart_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    unsigned char *out = (unsigned char *)buf;
    int i;
    for (i = 0; i < n && p->rx_head != p->rx_tail; i++) {
        out[i] = p->rx_buf[p->rx_tail];
        p->rx_tail = (p->rx_tail + 1) % p->rx_size;
    }
    return i;
}

/* Non-blocking write: fill tx ring, kick THRE IRQ on first byte, return count */
int uart_backend_write(pcbdcom_port_t *p, const void *buf, int n)
{
    const unsigned char *in = (const unsigned char *)buf;
    unsigned int next;
    int i;
    int was_empty = (p->tx_head == p->tx_tail);
    for (i = 0; i < n; i++) {
        next = (p->tx_head + 1) % p->tx_size;
        if (next == p->tx_tail) break;  /* ring full */
        p->tx_buf[p->tx_head] = in[i];
        p->tx_head = next;
    }
    if (was_empty && i > 0) {
        /* Wake TX path — enable THRE IRQ */
        unsigned char ier = UART_IN(p->base + UART_IER);
        UART_OUT(p->base + UART_IER, ier | IER_THRE);
    }
    return i;
}

/* ----- Backend registration ----- */

#include "backend.h"

const pcbdcom_backend_t pcbdcom_uart_backend = {
    "8250",
    0,                        /* no per-card state — every UART is standalone */
    uart_backend_probe,
    uart_backend_init,
    uart_backend_deinit,
    uart_backend_isr,
    uart_backend_read,
    uart_backend_write
};
