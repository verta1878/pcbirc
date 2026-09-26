/* ============================================================================
 * pcbcomm.h — public types for the pcbcomm serial layer
 *
 * License: GPLv3
 * ==========================================================================*/
#ifndef PCBCOMM_H
#define PCBCOMM_H

#include "uart.h"

#define PCBCOMM_MAX_PORTS 16   /* covers COM1..COM8 + multiport cards      */
#define PCBCOMM_RX_RING   512  /* per-port RX ring (bytes, DOS memory)     */
#define PCBCOMM_TX_RING   512  /* per-port TX ring                         */

typedef struct pcbcomm_port {
    unsigned int   base;       /* I/O base address                          */
    unsigned char  irq;        /* IRQ number                                */
    unsigned char  subport;    /* Sub-port index within multi-port card 0..N-1 */
    long           baud;       /* Configured baud rate                      */
    unsigned char  lcr;        /* Line control (bits/parity/stop, packed)   */
    uart_type_t    chip;       /* Detected chip type                        */
    /* Ring buffers */
    unsigned char *rx_buf;
    unsigned int   rx_head;
    unsigned int   rx_tail;
    unsigned int   rx_size;
    unsigned char *tx_buf;
    unsigned int   tx_head;
    unsigned int   tx_tail;
    unsigned int   tx_size;
    /* Backend hook */
    const struct pcbcomm_backend *backend;
    void          *backend_data;
    /* State */
    int            open;
    /* v1.2 additions */
    unsigned char  parity;      /* 'N','E','O','M','S' */
    unsigned char  data_bits;   /* 5..8 */
    unsigned char  stop_bits;   /* 1 or 2 */
    unsigned char  flow;        /* 0=none, 1=RTS/CTS, 2=XON/XOFF */
    void          *card_state;  /* per-card state (owned by backend) */
    /* FOSSIL 5C addition */
    unsigned char  cached_msr;  /* modem status register, updated by backend ISR.
                                 * Used by status_word() in int14.c so it works
                                 * for all backends, not just direct-UART ones.
                                 * Backend ISR sets: p->cached_msr = inp(base + UART_MSR);
                                 * Non-UART backends update from their own status mechanism. */
} pcbcomm_port_t;

#define PCBCOMM_MAX_CARDS  8
#define PCBCOMM_BUF_SIZE   PCBCOMM_RX_RING

#endif /* PCBCOMM_H */
