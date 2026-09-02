/* ============================================================================
 * pcbdcom.h — public types for the pcbdcom serial layer
 *
 * License: GPLv3
 * ==========================================================================*/
#ifndef PCBDCOM_H
#define PCBDCOM_H

#include "uart.h"

#define PCBDCOM_MAX_PORTS 16   /* covers COM1..COM8 + multiport cards      */
#define PCBDCOM_RX_RING   512  /* per-port RX ring (bytes, DOS memory)     */
#define PCBDCOM_TX_RING   512  /* per-port TX ring                         */

typedef struct pcbdcom_port {
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
    const struct pcbdcom_backend *backend;
    void          *backend_data;
    /* State */
    int            open;
    /* v1.2 additions */
    unsigned char  parity;      /* 'N','E','O','M','S' */
    unsigned char  data_bits;   /* 5..8 */
    unsigned char  stop_bits;   /* 1 or 2 */
    unsigned char  flow;        /* 0=none, 1=RTS/CTS, 2=XON/XOFF */
    void          *card_state;  /* per-card state (owned by backend) */
} pcbdcom_port_t;

#define PCBDCOM_MAX_CARDS  8
#define PCBDCOM_BUF_SIZE   PCBDCOM_RX_RING

#endif /* PCBDCOM_H */
