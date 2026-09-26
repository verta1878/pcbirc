/* ============================================================================
 * digi_pcxe_backend.c — pcbdcom DigiBoard PC/Xe backend
 *
 * Cards supported: DigiBoard PC/Xe (4/8/16 ports, ISA).
 *
 * All FEP protocol code shared with digi_accel via digi_fep.c/h.
 * This file: card probe + init wrapper + backend registration.
 *
 * Ported from Linux drivers/char/epca.c (kernel 2.6.32), GPLv2.
 * pcbirc crew (hexadecimal), GPLv3.
 * ==========================================================================*/

#include "pcbdcom.h"
#include "backend.h"
#include "digi_fep.h"

int digi_pcxe_backend_probe(pcbdcom_port_t *p)
{
    unsigned int seg = ((digi_fep_card_t *)p->backend_data)->card_seg;
    unsigned char n  = digi_fep_readb(seg, NPORT_OFF);
    /* Live FEP reports port count 4..64. 0xFF/0x00 = card absent or
     * FEP not booted. PC/Xe caps at 16 ports per card. */
    return (n >= 4 && n <= 16) ? 0 : -1;
}

int digi_pcxe_backend_init(pcbdcom_port_t *p)
{
    digi_fep_card_t *card = (digi_fep_card_t *)p->backend_data;
    if (digi_pcxe_backend_probe(p) < 0) return -1;
    if (digi_fep_init_channel(p, p->subport) < 0) return -1;
    if (card && p->subport < 64) card->ports[p->subport] = p;
    p->open = 1;
    return 0;
}

void digi_pcxe_backend_deinit(pcbdcom_port_t *p)
{
    if (!p->open) return;
    digi_fep_deinit_channel(p, p->subport);
    p->open = 0;
}

int digi_pcxe_backend_read(pcbdcom_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbdcom_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

const pcbdcom_backend_t pcbdcom_digi_pcxe_backend = {
    "DIGI_PCXE",
    digi_fep_card_get,
    digi_pcxe_backend_probe,
    digi_pcxe_backend_init,
    digi_pcxe_backend_deinit,
    digi_fep_isr,
    digi_pcxe_backend_read,
    digi_fep_write
};
