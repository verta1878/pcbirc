/* ============================================================================
 * digi_pcxe_backend.c — pcbcomm DigiBoard PC/Xe backend (COMMDV05)
 *
 * Cards supported: DigiBoard PC/Xe (4/8/16 ports, ISA).
 *
 * All FEP (Front End Processor) protocol code is shared with digi_accel
 * via digi_fep.c/h.  This file handles:
 *   - PC/Xe card probe (NPORT_OFF read to confirm FEP is booted)
 *   - Init wrapper (probe + FEP channel init + port registration)
 *   - Read from the port's software ring buffer (filled by digi_fep_isr)
 *   - Backend vtable registration
 *
 * The PC/Xe uses shared-memory mailbox I/O.  The on-card FEP processor
 * handles UART-level work (baud, flow control, interrupts); the host
 * driver talks to the FEP via command/event mailboxes in a memory window
 * at the card's segment address.  digi_fep_isr() drains the FEP event
 * queue and copies received bytes into p->rx_buf; our read() pulls from
 * that ring buffer.
 *
 * The FEP BIOS must be booted before the card will respond.  Clark's
 * COMMDRV relied on the DigiBoard DOS TSR (XIDOS.SYS) to download the
 * FEP firmware.  If the FEP is not booted (probe returns NPORT=0),
 * init prints a message directing the sysop to load XIDOS.SYS first.
 * A future version could embed the FEP download sequence directly
 * (see Linux epca.c pc_init / pcxe_memwinon / pcxe_memwinoff).
 *
 * Ported from Linux drivers/char/epca.c (kernel 2.6.32), GPLv2.
 * pcbirc crew (hexadecimal + wrench), GPLv3.
 * ==========================================================================*/

#include "pcbcomm.h"
#include "backend.h"
#include "digi_fep.h"

/* ---- Probe: confirm the FEP is alive ---------------------------------- */
int digi_pcxe_backend_probe(pcbcomm_port_t *p)
{
    unsigned int seg = ((digi_fep_card_t *)p->backend_data)->card_seg;
    unsigned char n  = digi_fep_readb(seg, NPORT_OFF);
    /* Live FEP reports port count 4..64.  0xFF or 0x00 means the card
     * is absent or the FEP BIOS hasn't been downloaded yet.
     * PC/Xe caps at 16 ports per card. */
    return (n >= 4 && n <= 16) ? 0 : -1;
}

/* ---- Init: probe, open channel, register in card table ---------------- */
int digi_pcxe_backend_init(pcbcomm_port_t *p)
{
    digi_fep_card_t *card = (digi_fep_card_t *)p->backend_data;

    if (digi_pcxe_backend_probe(p) < 0) {
        /* FEP not booted — most likely XIDOS.SYS wasn't loaded */
        printf("pcbcomm: DIGI_PCXE @ 0x%04X: FEP not responding.\n",
               card ? card->card_seg : 0);
        printf("  Load DigiBoard XIDOS.SYS before running PCBDTSR.\n");
        return -1;
    }

    if (digi_fep_init_channel(p, p->subport) < 0) return -1;

    /* Register this port in the card's port table so the ISR can
     * dispatch events to the right port_t by channel number. */
    if (card && p->subport < 64)
        card->ports[p->subport] = p;

    p->chip = 0;  /* no UART — FEP handles everything */
    p->open = 1;
    return 0;
}

/* ---- Deinit: close FEP channel --------------------------------------- */
void digi_pcxe_backend_deinit(pcbcomm_port_t *p)
{
    if (!p->open) return;
    digi_fep_deinit_channel(p, p->subport);
    p->open = 0;
}

/* ---- Read: drain the software ring buffer -----------------------------
 * digi_fep_isr() copies received bytes from the FEP's shared-memory
 * RX ring into p->rx_buf.  We read from that buffer here — same
 * pattern as every other backend.  No direct hardware access needed. */
int digi_pcxe_backend_read(pcbcomm_port_t *p, void *buf, int n)
{
    unsigned char *dst = (unsigned char *)buf;
    int count = 0;

    while (count < n && p->rx_head != p->rx_tail) {
        dst[count++] = p->rx_buf[p->rx_tail];
        p->rx_tail = (p->rx_tail + 1) % p->rx_size;
    }
    return count;
}

/* ---- Backend vtable --------------------------------------------------- */
const pcbcomm_backend_t pcbcomm_digi_pcxe_backend = {
    "DIGI_PCXE",
    digi_fep_card_get,
    digi_pcxe_backend_probe,
    digi_pcxe_backend_init,
    digi_pcxe_backend_deinit,
    digi_fep_isr,
    digi_pcxe_backend_read,
    digi_fep_write
};
