/* ============================================================================
 * digi_accel_backend.c — pcbdcom DigiBoard AccelePort backend
 *
 * Cards supported: AccelePort Xe, Xr, Xem (2..64 ports; ISA/PCI).
 *
 * Same FEP protocol as PC/Xe — code shared via digi_fep.c/h.
 * Differences:
 *   1. Probe validates CARDTYPE_OFF byte == ACCELE_ID / PCXEM_ID etc.,
 *      not just port count (some non-Accel Digi cards would also pass
 *      a plain port-count probe).
 *   2. Port cap: 64 (concentrator model), vs PC/Xe's 16.
 *   3. Concentrator enumeration (Xem base + concentrator hubs) — v1
 *      treats concentrator ports as regular ports; hub topology
 *      discovery is v1.1 work.
 *
 * Ported from Linux drivers/char/epca.c (kernel 2.6.32), GPLv2.
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include "pcbdcom.h"
#include "backend.h"
#include "digi_fep.h"

/* Card-type IDs from epca.h (accepted by AccelePort family) */
#define PCXEM_ID    0x02
#define EISAXEM_ID  0x03
#define PCIXEM_ID   0x04
#define PCIXR_ID    0x05
#define ACCELE_ID   0x06

int digi_accel_backend_probe(pcbdcom_port_t *p)
{
    unsigned int seg = ((digi_fep_card_t *)p->backend_data)->card_seg;
    unsigned char card_type = digi_fep_readb(seg, CARDTYPE_OFF);
    unsigned char n_ports;

    /* Card-type byte identifies AccelePort family */
    if (card_type != ACCELE_ID  && card_type != PCXEM_ID  &&
        card_type != EISAXEM_ID && card_type != PCIXEM_ID &&
        card_type != PCIXR_ID)
        return -1;

    n_ports = digi_fep_readb(seg, NPORT_OFF);
    /* AccelePort with concentrators: 2..64 ports */
    return (n_ports >= 2 && n_ports <= 64) ? 0 : -1;
}

int digi_accel_backend_init(pcbdcom_port_t *p)
{
    digi_fep_card_t *card = (digi_fep_card_t *)p->backend_data;
    if (digi_accel_backend_probe(p) < 0) return -1;
    if (digi_fep_init_channel(p, p->subport) < 0) return -1;
    if (card && p->subport < 64) card->ports[p->subport] = p;
    p->open = 1;
    return 0;
}

void digi_accel_backend_deinit(pcbdcom_port_t *p)
{
    if (!p->open) return;
    digi_fep_deinit_channel(p, p->subport);
    p->open = 0;
}

int digi_accel_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_digi_accel_backend = {
    "DIGI_ACCEL",
    digi_fep_card_get,
    digi_accel_backend_probe,
    digi_accel_backend_init,
    digi_accel_backend_deinit,
    digi_fep_isr,      /* shared */
    digi_accel_backend_read,
    digi_fep_write     /* shared */
};

/* ----- v1.1 TODO -----
 *  1. Concentrator topology: Xem cards have a base + external hubs
 *     (PORTS/8 concentrators of 8-16 ports each). Linux epca_setup
 *     enumerates these — port to a helper called at load time.
 *  2. PCIXR firmware-download (PCI variant) — needs bus scan.
 *  3. Otherwise identical to PC/Xe: shared code in digi_fep.c handles
 *     everything from init onward.
 * ---------------------------------------------------------------------- */
