/* ============================================================================
 * uart.c — pcbdcom 8250/16450/16550/16550A UART backend
 *
 * Ported from Linux drivers/tty/serial/8250/8250_port.c (kernel v6.6), GPLv2.
 * Original authors: Ted Ts'o, Russell King, Alan Cox, Theodore Y. Ts'o.
 * DOS 16-bit adaptations: pcbirc crew (hexadecimal), GPLv3.
 *
 * Adaptations from Linux 8250_port.c:
 *   - autoconfig() → uart_probe(). Same SCR + IIR/FCR probe sequence.
 *   - serial_in/serial_out (memory or port I/O) → UART_IN/UART_OUT
 *     (I/O port only — DOS 16-bit target, no memory-mapped serial).
 *   - Removed tty_struct/uart_port plumbing; talks to caller-supplied
 *     base I/O address; higher-level backend passes pcbdcom_port_t.
 *   - Removed platform_device/OF; static port table (see pcbdcom.c).
 *   - 16650/16750 detection deferred to v2 (EFR probe).
 *
 * Chip probe reference: Linux 8250_port.c autoconfig(), lines ~1500-1700.
 * ==========================================================================*/

#include <conio.h>
#include "uart.h"

/* Compiler shims: BC uses outp/inp, MSC7 uses _outp/_inp, Watcom outp/inp */
#if defined(_MSC_VER)
# define UART_OUT(port, val) _outp((port), (val))
# define UART_IN(port)       (unsigned char)_inp((port))
#else
# define UART_OUT(port, val) outp((port), (val))
# define UART_IN(port)       (unsigned char)inp((port))
#endif

/* ----- Chip detection: SCR then FCR probe (Linux autoconfig() logic) ----- */

uart_type_t uart_probe(unsigned int base)
{
    unsigned char scr_orig, scr1, scr2, iir;

    /* SCR test: 8250 has no scratch register. If two distinct writes stick,
     * we have at least a 16450. */
    scr_orig = UART_IN(base + UART_SCR);
    UART_OUT(base + UART_SCR, 0xA5);
    scr1 = UART_IN(base + UART_SCR);
    UART_OUT(base + UART_SCR, 0x5A);
    scr2 = UART_IN(base + UART_SCR);
    UART_OUT(base + UART_SCR, scr_orig);
    if (scr1 != 0xA5 || scr2 != 0x5A)
        return UART_TYPE_8250;  /* or nothing there — caller may verify */

    /* FCR test: enable FIFO, read IIR bits 7:6.
     *  11 → 16550A (FIFO works)
     *  10 → 16550  (FIFO buggy)
     *  0x → 16450  (no FIFO but has SCR) */
    UART_OUT(base + UART_FCR, FCR_ENABLE);
    iir = UART_IN(base + UART_IIR);
    UART_OUT(base + UART_FCR, 0);
    switch (iir & IIR_FIFO) {
        case 0xC0: return UART_TYPE_16550A;
        case 0x80: return UART_TYPE_16550;
        default:   return UART_TYPE_16450;
    }
}

/* ----- Baud rate ----- */

void uart_set_baud(unsigned int base, long baud)
{
    unsigned int div;
    unsigned char lcr;

    if (baud <= 0) return;
    div = (unsigned int) UART_BAUD_DIVISOR(baud);
    if (div == 0) div = 1;
    lcr = UART_IN(base + UART_LCR);
    UART_OUT(base + UART_LCR, lcr | LCR_DLAB);
    UART_OUT(base + UART_DLL, div & 0xFF);
    UART_OUT(base + UART_DLM, (div >> 8) & 0xFF);
    UART_OUT(base + UART_LCR, lcr & ~LCR_DLAB);
}

/* ----- Line configuration (bits/parity/stop packed as LCR value) ----- */

void uart_set_line(unsigned int base, unsigned char lcr)
{
    /* Preserve DLAB=0 */
    UART_OUT(base + UART_LCR, lcr & ~LCR_DLAB);
}

/* ----- FIFO configuration (16550+) ----- */

void uart_set_fifo(unsigned int base, unsigned char fcr)
{
    UART_OUT(base + UART_FCR, fcr);
}

/* ----- Poll-style status + I/O (used by INT 14h synchronous path) ----- */

int uart_tx_ready(unsigned int base)
{
    return (UART_IN(base + UART_LSR) & LSR_THRE) ? 1 : 0;
}

int uart_rx_ready(unsigned int base)
{
    return (UART_IN(base + UART_LSR) & LSR_DR) ? 1 : 0;
}

void uart_tx(unsigned int base, unsigned char byte)
{
    UART_OUT(base + UART_THR, byte);
}

unsigned char uart_rx(unsigned int base)
{
    return UART_IN(base + UART_RBR);
}

/* ----- Modem control ----- */

void uart_set_dtr(unsigned int base, int on)
{
    unsigned char mcr = UART_IN(base + UART_MCR);
    if (on) mcr |=  MCR_DTR; else mcr &= ~MCR_DTR;
    UART_OUT(base + UART_MCR, mcr);
}

void uart_set_rts(unsigned int base, int on)
{
    unsigned char mcr = UART_IN(base + UART_MCR);
    if (on) mcr |=  MCR_RTS; else mcr &= ~MCR_RTS;
    UART_OUT(base + UART_MCR, mcr);
}

int uart_get_modem_status(unsigned int base)
{
    return UART_IN(base + UART_MSR);
}
