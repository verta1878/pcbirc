/* ============================================================================
 * digi_accel_backend.c — pcbcomm DigiBoard AccelePort backend
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

#include "pcbcomm.h"
#include "backend.h"
#include "digi_fep.h"

/* Card-type IDs from epca.h (accepted by AccelePort family) */
#define PCXEM_ID    0x02
#define EISAXEM_ID  0x03
#define PCIXEM_ID   0x04
#define PCIXR_ID    0x05
#define ACCELE_ID   0x06

int digi_accel_backend_probe(pcbcomm_port_t *p)
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

int digi_accel_backend_init(pcbcomm_port_t *p)
{
    digi_fep_card_t *card = (digi_fep_card_t *)p->backend_data;
    if (digi_accel_backend_probe(p) < 0) return -1;
    if (digi_fep_init_channel(p, p->subport) < 0) return -1;
    if (card && p->subport < 64) card->ports[p->subport] = p;
    p->open = 1;
    return 0;
}

void digi_accel_backend_deinit(pcbcomm_port_t *p)
{
    if (!p->open) return;
    digi_fep_deinit_channel(p, p->subport);
    p->open = 0;
}

int digi_accel_backend_read(pcbcomm_port_t *p, void *buf, int n)
{
    extern int uart_backend_read(pcbcomm_port_t *, void *, int);
    return uart_backend_read(p, buf, n);
}

const pcbcomm_backend_t pcbcomm_digi_accel_backend = {
    "DIGI_ACCEL",
    digi_fep_card_get,
    digi_accel_backend_probe,
    digi_accel_backend_init,
    digi_accel_backend_deinit,
    digi_fep_isr,      /* shared */
    digi_accel_backend_read,
    digi_fep_write     /* shared */
};

/* ----- Concentrator topology ----- *
 * Xem cards have a base board + external concentrator hubs.
 * Each hub adds 8-16 ports. The FEP firmware reports the total
 * port count via NPORT_OFF. We don't need to enumerate hubs
 * ourselves — the FEP does that after firmware boot. We just
 * trust the port count it reports and map subports 0..n_ports-1
 * to FEP channels. Linux epca.c did the same for non-PCI cards. */

/* ----- Firmware download for PCI variants ----- *
 * ISA cards have onboard BIOS ROM — they boot FEP automatically.
 * PCI cards (PCIXR, PCIXEM) need host-loaded firmware:
 *   1. Find card via PCI BIOS (INT 1Ah)
 *   2. Read BAR to get memory window
 *   3. Reset card (write FEPRST to I/O port)
 *   4. Copy BIOS to card memory at offset 0x1000
 *   5. Write boot magic (0x0bf00401) to offset 0
 *   6. Wait for BIOS POST ("GD" at POSTAREA)
 *   7. Copy FEP to card memory at offset 0x1000
 *   8. Write FEP boot magic
 *   9. Wait for FEP ready
 *
 * Firmware files ship with COMM-DRV at:
 *   pcb1541/install/dist/target/COMMDRV/XABIOS.BIN  (2K BIOS)
 *   pcb1541/install/dist/target/COMMDRV/XACOMX.BIN  (6K FEP comms mode)
 *   pcb1541/install/dist/target/COMMDRV/XACOOK.BIN  (6K FEP cooked mode)
 */

#include <stdio.h>
#include <stdlib.h>

#define FEPRST       0x0E
#define POSTAREA     0x0C00
#define FW_OFFSET    0x1000
#define BOOT_MAGIC   0x0bf00401UL
#define POST_OK_HI   'G'
#define POST_OK_LO   'D'

/* PCI BIOS: INT 1Ah, AH=B1h — find device by vendor/device ID */
#define DIGI_VENDOR_ID  0x114F
#define PCIXR_DEV_ID    0x0004
#define PCIXEM_DEV_ID   0x0005

/* Load a firmware file from disk into a malloc'd buffer.
 * Returns size, or 0 on failure. Caller frees. */
static unsigned int digi_load_fw_file(const char *path,
                                      unsigned char **buf_out)
{
    FILE *f;
    long sz;
    unsigned char *buf;

    f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0 || sz > 65536L) { fclose(f); return 0; }
    fseek(f, 0, SEEK_SET);
    buf = (unsigned char *)malloc((unsigned int)sz);
    if (!buf) { fclose(f); return 0; }
    if (fread(buf, 1, (unsigned int)sz, f) != (unsigned int)sz) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    *buf_out = buf;
    return (unsigned int)sz;
}

/* Download BIOS + FEP to a DigiBoard card.
 * seg = card memory segment (ISA) or mapped base (PCI).
 * Returns 0 on success. */
int digi_accel_firmware_download(unsigned int seg,
                                const char *bios_path,
                                const char *fep_path)
{
    unsigned char *bios_buf = 0, *fep_buf = 0;
    unsigned int bios_sz, fep_sz;
    unsigned int i;

    /* Load BIOS */
    bios_sz = digi_load_fw_file(bios_path, &bios_buf);
    if (bios_sz == 0) return -1;

    /* Clear POST area */
    for (i = 0; i < 16; i++)
        digi_fep_writeb(seg, POSTAREA + i, 0);

    /* Copy BIOS at offset 0x1000 */
    for (i = 0; i < bios_sz; i++)
        digi_fep_writeb(seg, FW_OFFSET + i, bios_buf[i]);
    free(bios_buf);

    /* Write boot magic */
    digi_fep_writew(seg, 0, (unsigned int)(BOOT_MAGIC & 0xFFFF));
    digi_fep_writew(seg, 2, (unsigned int)(BOOT_MAGIC >> 16));

    /* Wait for BIOS POST — "GD" at POSTAREA (up to 10 seconds) */
    for (i = 0; i < 10000; i++) {
        unsigned char hi = digi_fep_readb(seg, POSTAREA);
        unsigned char lo = digi_fep_readb(seg, POSTAREA + 1);
        if (hi == POST_OK_HI && lo == POST_OK_LO) break;
        /* ~1ms delay — rough busy-wait */
        { volatile int d; for (d = 0; d < 1000; d++) ; }
    }
    if (digi_fep_readb(seg, POSTAREA) != POST_OK_HI)
        return -2;  /* BIOS POST failed */

    /* Load FEP */
    fep_sz = digi_load_fw_file(fep_path, &fep_buf);
    if (fep_sz == 0) return -3;

    /* Copy FEP at offset 0x1000 (overwrites BIOS — normal) */
    for (i = 0; i < fep_sz; i++)
        digi_fep_writeb(seg, FW_OFFSET + i, fep_buf[i]);
    free(fep_buf);

    /* FEP boot: write 0x0002 to trigger FEP start */
    digi_fep_writew(seg, POSTAREA, 0);
    digi_fep_writew(seg, 2, 0x0002);

    /* Wait for FEP to report ready — NPORT_OFF should become non-zero */
    for (i = 0; i < 10000; i++) {
        if (digi_fep_readb(seg, NPORT_OFF) != 0) break;
        { volatile int d; for (d = 0; d < 1000; d++) ; }
    }
    if (digi_fep_readb(seg, NPORT_OFF) == 0)
        return -4;  /* FEP did not start */

    return 0;
}

/* ----- v1.1 status -----
 *  1. (DONE) Concentrator topology: FEP firmware handles hub
 *     enumeration automatically. Port count from NPORT_OFF covers
 *     all ports including concentrator-attached ones. We map
 *     subports 0..n_ports-1 to FEP channels — same as Linux epca.c.
 *  2. (DONE) Firmware download: digi_accel_firmware_download() loads
 *     BIOS + FEP from disk, copies to card memory, waits for POST
 *     and FEP ready. Firmware at pcb1541/install/dist/target/COMMDRV/.
 *     ISA cards don't need this (onboard BIOS ROM boots automatically).
 *  3. Otherwise identical to PC/Xe: shared code in digi_fep.c handles
 *     everything from init onward.
 * ---------------------------------------------------------------------- */
