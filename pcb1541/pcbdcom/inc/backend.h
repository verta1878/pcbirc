/* ============================================================================
 * backend.h — pluggable backend interface
 *
 * Every card family (uart, boca, cyclom, digi_pcxe, digi_accel, rocket,
 * easyio) implements pcbcomm_backend_t. See ../src/uart_backend.c for
 * the reference implementation.
 *
 * License: GPLv3
 * ==========================================================================*/
#ifndef PCBCOMM_BACKEND_H
#define PCBCOMM_BACKEND_H

#include "pcbcomm.h"

typedef struct pcbcomm_backend {
    const char *name;                                 /* "8250", "BOCA16", ... */
    /* card_get: return per-card state (allocated on first call per addr).
     * NULL for backends with no per-card state (uart). Called by
     * parse_config() BEFORE first port on card is init'd. */
    void *(*card_get)(unsigned long card_addr);
    int  (*probe) (pcbcomm_port_t *p);                /* detect presence      */
    int  (*init)  (pcbcomm_port_t *p);                /* configure hardware   */
    void (*deinit)(pcbcomm_port_t *p);                /* release hardware     */
    void (*isr)   (pcbcomm_port_t *p);                /* IRQ dispatch entry   */
    int  (*read)  (pcbcomm_port_t *p, void *buf, int n);
    int  (*write) (pcbcomm_port_t *p, const void *buf, int n);
} pcbcomm_backend_t;

/* Concrete backends (defined in src/<backend>_backend.c) */
extern const pcbcomm_backend_t pcbcomm_uart_backend;       /* 8250/16550        */
extern const pcbcomm_backend_t pcbcomm_boca_backend;       /* Boca dumb multi   */
extern const pcbcomm_backend_t pcbcomm_cyclom_backend;     /* Cyclades Cyclom-Y */
extern const pcbcomm_backend_t pcbcomm_digi_pcxe_backend;  /* DigiBoard PC/Xe   */
extern const pcbcomm_backend_t pcbcomm_digi_accel_backend; /* DigiBoard Accel   */
extern const pcbcomm_backend_t pcbcomm_rocket_backend;     /* Comtrol RocketPrt */
extern const pcbcomm_backend_t pcbcomm_easyio_backend;     /* Stallion EasyIO   */
extern const pcbcomm_backend_t pcbcomm_arnet_backend;      /* Arnet SmartPort   */
extern const pcbcomm_backend_t pcbcomm_hub6_backend;       /* Intel HUB-6       */
extern const pcbcomm_backend_t pcbcomm_digi_comxi_backend; /* Digi COM/Xi 80186 */
extern const pcbcomm_backend_t pcbcomm_gtek_backend;       /* GTEK BBS-550/8Fx  */
#if defined(PCB1541)
/* ----- v1.4 extended backends — PCB1541-only ----- *
 * Written from public documentation without hardware validation in the
 * pcbirc lab. Sysops with these cards should test + report issues. */
extern const pcbcomm_backend_t pcbcomm_stallion_brumby_backend; /* Stallion Brumby/ONboard */
extern const pcbcomm_backend_t pcbcomm_chase_iolan_backend;     /* Chase Research IOLAN    */
extern const pcbcomm_backend_t pcbcomm_equinox_sst_backend;     /* Equinox SST (untested)  */
#endif /* PCB1541 */
/* Deferred: TCP_SOCKET (15.41 only) */

#endif /* PCBCOMM_BACKEND_H */
