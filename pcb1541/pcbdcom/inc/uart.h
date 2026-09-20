/* ============================================================================
 * uart.h — 8250 / 16450 / 16550 UART register definitions for pcbdcom
 *
 * Reference sources (all free/open):
 *   drivers/netfosdl/serial.pas — Free Pascal DOS FOSSIL driver (crew)
 *   PC16550D datasheet (National Semiconductor, public)
 *   Linux drivers/tty/serial/8250/*.c (GPL)
 *
 * License: GPLv3
 * ==========================================================================*/

#ifndef PCBDCOM_UART_H
#define PCBDCOM_UART_H

/* Standard COM port base I/O addresses (IBM PC/AT convention) */
#define COM1_BASE 0x3F8
#define COM2_BASE 0x2F8
#define COM3_BASE 0x3E8
#define COM4_BASE 0x2E8

/* Standard COM port IRQs (IBM PC/AT convention).
 * COM1/COM3 share IRQ 4; COM2/COM4 share IRQ 3. Sharing is legal at
 * hardware level with some care in the interrupt handler. */
#define COM1_IRQ 4
#define COM2_IRQ 3
#define COM3_IRQ 4
#define COM4_IRQ 3

/* UART register offsets (from base). Same for 8250/16450/16550/16650. */
#define UART_RBR  0  /* R:   Receive Buffer Register (DLAB=0)      */
#define UART_THR  0  /* W:   Transmit Holding Register (DLAB=0)    */
#define UART_DLL  0  /* R/W: Divisor Latch Low Byte (DLAB=1)       */
#define UART_IER  1  /* R/W: Interrupt Enable Register (DLAB=0)    */
#define UART_DLM  1  /* R/W: Divisor Latch High Byte (DLAB=1)      */
#define UART_IIR  2  /* R:   Interrupt Identification Register     */
#define UART_FCR  2  /* W:   FIFO Control Register (16550+)        */
#define UART_LCR  3  /* R/W: Line Control Register                 */
#define UART_MCR  4  /* R/W: Modem Control Register                */
#define UART_LSR  5  /* R:   Line Status Register                  */
#define UART_MSR  6  /* R:   Modem Status Register                 */
#define UART_SCR  7  /* R/W: Scratch Register                      */

/* IER — Interrupt Enable Register bits */
#define IER_RDA   0x01  /* Received data available                 */
#define IER_THRE  0x02  /* Transmit Holding Register Empty         */
#define IER_LSR   0x04  /* Receiver Line Status change             */
#define IER_MSR   0x08  /* Modem Status change                     */

/* IIR — Interrupt Identification Register bits */
#define IIR_NONE  0x01  /* No interrupt pending (when bit0=1)      */
#define IIR_MASK  0x0E  /* Which interrupt (bits 3:1)              */
#define IIR_MSR   0x00  /* Modem status change                     */
#define IIR_THRE  0x02  /* Transmitter empty                       */
#define IIR_RDA   0x04  /* Received data available                 */
#define IIR_LSR   0x06  /* Line status change                      */
#define IIR_TIMO  0x0C  /* Character timeout (16550+ FIFO mode)    */
#define IIR_FIFO  0xC0  /* Bits 7:6 set = 16550 FIFO active        */

/* FCR — FIFO Control (16550+) */
#define FCR_ENABLE   0x01  /* Enable FIFOs                         */
#define FCR_RXCLR    0x02  /* Clear receive FIFO                   */
#define FCR_TXCLR    0x04  /* Clear transmit FIFO                  */
#define FCR_DMA      0x08  /* DMA mode                             */
#define FCR_TRIG_1   0x00  /* RX trigger 1 byte                    */
#define FCR_TRIG_4   0x40  /* RX trigger 4 bytes                   */
#define FCR_TRIG_8   0x80  /* RX trigger 8 bytes                   */
#define FCR_TRIG_14  0xC0  /* RX trigger 14 bytes                  */

/* LCR — Line Control Register */
#define LCR_5BITS  0x00
#define LCR_6BITS  0x01
#define LCR_7BITS  0x02
#define LCR_8BITS  0x03
#define LCR_STOP1  0x00
#define LCR_STOP2  0x04
#define LCR_PAR_N  0x00  /* No parity   */
#define LCR_PAR_O  0x08  /* Odd parity  */
#define LCR_PAR_E  0x18  /* Even parity */
#define LCR_PAR_M  0x28  /* Mark        */
#define LCR_PAR_S  0x38  /* Space       */
#define LCR_BREAK  0x40  /* Set break line high                    */
#define LCR_DLAB   0x80  /* Divisor Latch Access Bit               */

/* MCR — Modem Control Register */
#define MCR_DTR    0x01  /* Data Terminal Ready                    */
#define MCR_RTS    0x02  /* Request To Send                        */
#define MCR_OUT1   0x04  /* General-purpose output 1               */
#define MCR_OUT2   0x08  /* General-purpose output 2 (IRQ enable)  */
#define MCR_LOOP   0x10  /* Loopback test mode                     */

/* LSR — Line Status Register */
#define LSR_DR     0x01  /* Data Ready                             */
#define LSR_OE     0x02  /* Overrun Error                          */
#define LSR_PE     0x04  /* Parity Error                           */
#define LSR_FE     0x08  /* Framing Error                          */
#define LSR_BI     0x10  /* Break Interrupt                        */
#define LSR_THRE   0x20  /* Transmit Holding Register Empty        */
#define LSR_TEMT   0x40  /* Transmit shift register Empty          */
#define LSR_FE_RX  0x80  /* Error in RX FIFO (16550+)              */

/* MSR — Modem Status Register */
#define MSR_DCTS   0x01  /* Delta CTS                              */
#define MSR_DDSR   0x02  /* Delta DSR                              */
#define MSR_TERI   0x04  /* Trailing edge RI                       */
#define MSR_DDCD   0x08  /* Delta DCD                              */
#define MSR_CTS    0x10  /* Clear To Send                          */
#define MSR_DSR    0x20  /* Data Set Ready                         */
#define MSR_RI     0x40  /* Ring Indicator                         */
#define MSR_DCD    0x80  /* Data Carrier Detect                    */

/* Divisor calculation: divisor = 115200 / baud_rate */
#define UART_CLOCK_HZ 1843200L
#define UART_BAUD_DIVISOR(baud) ((int)(UART_CLOCK_HZ / (16L * (baud))))

/* Detected UART chip type — returned from uart_probe() */
typedef enum {
    UART_TYPE_NONE   = 0,  /* No UART at that base                 */
    UART_TYPE_8250   = 1,  /* Original 8250 (no scratch, no FIFO)  */
    UART_TYPE_16450  = 2,  /* Scratch register present, no FIFO    */
    UART_TYPE_16550  = 3,  /* FIFO 16 bytes, may have bugs         */
    UART_TYPE_16550A = 4,  /* FIFO 16 bytes, working               */
    UART_TYPE_16650  = 5,  /* FIFO 32 bytes                        */
    UART_TYPE_16750  = 6   /* FIFO 64 bytes, auto-flow-ctl         */
} uart_type_t;

/* Public API — implemented in uart.c */
uart_type_t uart_probe(unsigned int base);
void        uart_set_baud(unsigned int base, long baud);
void        uart_set_line(unsigned int base, unsigned char lcr);
void        uart_set_fifo(unsigned int base, unsigned char fcr);
int         uart_tx_ready(unsigned int base);
int         uart_rx_ready(unsigned int base);
void        uart_tx(unsigned int base, unsigned char byte);
unsigned char uart_rx(unsigned int base);
void        uart_set_dtr(unsigned int base, int on);
void        uart_set_rts(unsigned int base, int on);
int         uart_get_modem_status(unsigned int base);

#endif /* PCBDCOM_UART_H */
