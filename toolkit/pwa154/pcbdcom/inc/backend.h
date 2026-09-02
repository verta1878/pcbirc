/* ============================================================================
 * backend.h — pluggable backend interface
 *
 * Every card family (uart, boca, cyclom, digi_pcxe, digi_accel, rocket,
 * easyio) implements pcbdcom_backend_t. See ../src/uart_backend.c for
 * the reference implementation.
 *
 * License: GPLv3
 * ==========================================================================*/
#ifndef PCBDCOM_BACKEND_H
#define PCBDCOM_BACKEND_H

#include "pcbdcom.h"

typedef struct pcbdcom_backend {
    const char *name;                                 /* "8250", "BOCA16", ... */
    /* card_get: return per-card state (allocated on first call per addr).
     * NULL for backends with no per-card state (uart). Called by
     * parse_config() BEFORE first port on card is init'd. */
    void *(*card_get)(unsigned long card_addr);
    int  (*probe) (pcbdcom_port_t *p);                /* detect presence      */
    int  (*init)  (pcbdcom_port_t *p);                /* configure hardware   */
    void (*deinit)(pcbdcom_port_t *p);                /* release hardware     */
    void (*isr)   (pcbdcom_port_t *p);                /* IRQ dispatch entry   */
    int  (*read)  (pcbdcom_port_t *p, void *buf, int n);
    int  (*write) (pcbdcom_port_t *p, const void *buf, int n);
} pcbdcom_backend_t;

/* Concrete backends (defined in src/<backend>_backend.c) */
extern const pcbdcom_backend_t pcbdcom_uart_backend;       /* 8250/16550        */
extern const pcbdcom_backend_t pcbdcom_boca_backend;       /* Boca dumb multi   */
extern const pcbdcom_backend_t pcbdcom_cyclom_backend;     /* Cyclades Cyclom-Y */
extern const pcbdcom_backend_t pcbdcom_digi_pcxe_backend;  /* DigiBoard PC/Xe   */
extern const pcbdcom_backend_t pcbdcom_digi_accel_backend; /* DigiBoard Accel   */
extern const pcbdcom_backend_t pcbdcom_rocket_backend;     /* Comtrol RocketPrt */
extern const pcbdcom_backend_t pcbdcom_easyio_backend;     /* Stallion EasyIO   */
extern const pcbdcom_backend_t pcbdcom_arnet_backend;      /* Arnet SmartPort   */
/* Deferred: TCP_SOCKET (15.41 only) */

#endif /* PCBDCOM_BACKEND_H */
