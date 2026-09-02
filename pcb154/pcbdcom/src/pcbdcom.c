/* ============================================================================
 * pcbdcom.c — main entry point (TSR install), config parser, backend registry
 *
 * Dual-mode loader:
 *   - CONFIG.SYS DEVICE=PCBDCOM.SYS  → device_entry() called by DOS
 *   - AUTOEXEC.BAT LH PCBDCOM.EXE    → main() called normally, TSR install
 *
 * The same source file compiles into either variant. .SYS build uses
 * device_entry() as its request-header dispatch; .EXE build uses main()
 * and calls dos_keep_tsr() after installation.
 *
 * License: GPLv3 (pcbirc crew, hexadecimal)
 * ==========================================================================*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcbdcom.h"
#include "backend.h"

/* Global port table (referenced from int14.c) */
pcbdcom_port_t g_ports[PCBDCOM_MAX_PORTS];
int            g_n_ports = 0;

/* Ring buffer arenas — statically allocated to keep resident image small */
static unsigned char g_rx_arena[PCBDCOM_MAX_PORTS][PCBDCOM_RX_RING];
static unsigned char g_tx_arena[PCBDCOM_MAX_PORTS][PCBDCOM_TX_RING];

/* External IRQ + INT14 install/uninstall */
extern int  pcbdcom_irq_register(unsigned char irq, pcbdcom_port_t *p);
extern void pcbdcom_irq_shutdown(void);
extern void pcbdcom_int14_install(void);
extern void pcbdcom_int14_uninstall(void);

/* Backend name -> pointer lookup */
static const pcbdcom_backend_t *find_backend(const char *name)
{
    if (!strcmp(name, "8250"))       return &pcbdcom_uart_backend;
    if (!strcmp(name, "BOCA"))       return &pcbdcom_boca_backend;
    if (!strcmp(name, "BOCA16"))     return &pcbdcom_boca_backend;
    if (!strcmp(name, "CYCLOM"))     return &pcbdcom_cyclom_backend;
    if (!strcmp(name, "DIGI_PCXE"))  return &pcbdcom_digi_pcxe_backend;
    if (!strcmp(name, "DIGI_ACCEL")) return &pcbdcom_digi_accel_backend;
    if (!strcmp(name, "ROCKET"))     return &pcbdcom_rocket_backend;
    if (!strcmp(name, "EASYIO"))     return &pcbdcom_easyio_backend;
    if (!strcmp(name, "ARNETSPP"))   return &pcbdcom_arnet_backend;
    if (!strcmp(name, "ARNET"))      return &pcbdcom_arnet_backend;
    if (!strcmp(name, "HUB6"))       return &pcbdcom_hub6_backend;
    if (!strcmp(name, "IBM8"))       return &pcbdcom_hub6_backend;
    if (!strcmp(name, "DIGI_COMXI")) return &pcbdcom_digi_comxi_backend;
    if (!strcmp(name, "COMXI"))      return &pcbdcom_digi_comxi_backend;
    if (!strcmp(name, "GTEK"))       return &pcbdcom_gtek_backend;
    if (!strcmp(name, "GTEK8"))      return &pcbdcom_gtek_backend;
#if defined(PCB1541)
    /* ----- v1.4 extended card families (15.41-only) ----- */
    if (!strcmp(name, "STALLION_BRUMBY")) return &pcbdcom_stallion_brumby_backend;
    if (!strcmp(name, "BRUMBY"))          return &pcbdcom_stallion_brumby_backend;
    if (!strcmp(name, "ONBOARD"))         return &pcbdcom_stallion_brumby_backend;
    if (!strcmp(name, "CHASE_IOLAN"))     return &pcbdcom_chase_iolan_backend;
    if (!strcmp(name, "IOLAN"))           return &pcbdcom_chase_iolan_backend;
    if (!strcmp(name, "EQUINOX_SST"))     return &pcbdcom_equinox_sst_backend;
    if (!strcmp(name, "SST"))             return &pcbdcom_equinox_sst_backend;
#endif /* PCB1541 */
    return NULL;
}

/* ----- Config file parser ----- *
 * PCBDCOM.CFG format (see SPEC.md):
 *   # comment
 *   PORT CARD SUBPORT BASE IRQ CARDSEG FOSSIL
 *   1    8250 0       0x3F8 4  0       Y
 *   ...
 * Returns number of ports configured, or -1 on error. */
static int parse_config(const char *path)
{
    FILE *f;
    char line[128], card[16];
    unsigned int port, subport, base, irq, cardseg;
    char fossil;
    pcbdcom_port_t *p;
    const pcbdcom_backend_t *b;
    int n = 0;

    f = fopen(path, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        unsigned long card_key;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (sscanf(line, "%u %15s %u %i %u %u %c",
                   &port, card, &subport, &base, &irq, &cardseg, &fossil) < 6)
            continue;
        if (port == 0 || port > PCBDCOM_MAX_PORTS) continue;

        b = find_backend(card);
        if (!b) {
            printf("pcbdcom: unknown card '%s' on port %u\n", card, port);
            continue;
        }

        p             = &g_ports[port - 1];
        p->base       = base;
        p->irq        = (unsigned char)irq;
        p->subport    = (unsigned char)subport;   /* v1.1: sub-port index    */
        p->baud       = 38400;
        p->lcr        = 0;
        p->backend    = b;

        /* v1.1: per-card state via backend->card_get() hook.
         * card_key: memory-mapped cards → cardseg; I/O-mapped cards →
         * base (Boca / EasyIO) or a mudbac derived from base. If backend
         * has no card_get (uart), backend_data stays as the raw seg. */
        if (b->card_get) {
            card_key = cardseg ? (unsigned long)cardseg : (unsigned long)base;
            p->backend_data = b->card_get(card_key);
            if (!p->backend_data) {
                printf("pcbdcom: card pool full for '%s' on port %u\n",
                       card, port);
                continue;
            }
        } else {
            p->backend_data = (void *)(unsigned long)cardseg;
        }

        p->rx_buf     = g_rx_arena[port - 1];
        p->rx_size    = PCBDCOM_RX_RING;
        p->tx_buf     = g_tx_arena[port - 1];
        p->tx_size    = PCBDCOM_TX_RING;
        p->rx_head = p->rx_tail = p->tx_head = p->tx_tail = 0;
        p->open       = 0;

        n++;
        if (n > g_n_ports) g_n_ports = n;
    }
    fclose(f);
    return n;
}

/* Bring up every configured port + register IRQs + install INT 14h */
static int pcbdcom_install(void)
{
    int i, ok = 0;
    printf("pcbdcom v1 — %d ports configured\n", g_n_ports);

    for (i = 0; i < g_n_ports; i++) {
        pcbdcom_port_t *p = &g_ports[i];
        if (!p->backend) continue;
        if (p->backend->init(p) < 0) {
            printf("  port %d (%s @ 0x%X): probe/init FAILED\n",
                   i + 1, p->backend->name, p->base);
            continue;
        }
        if (pcbdcom_irq_register(p->irq, p) < 0) {
            printf("  port %d IRQ %u: register FAILED\n", i + 1, p->irq);
            p->backend->deinit(p);
            continue;
        }
        printf("  port %d: %s @ 0x%X IRQ %u  chip=%d\n",
               i + 1, p->backend->name, p->base, p->irq, (int)p->chip);
        ok++;
    }

    pcbdcom_int14_install();
    printf("pcbdcom: %d/%d ports online, INT 14h hooked.\n", ok, g_n_ports);
    return ok;
}

/* Command-line arg: /CFG=path (default PCBDCOM.CFG) */
static const char *find_cfg_arg(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "/CFG=", 5)) return argv[i] + 5;
        if (!strncmp(argv[i], "/cfg=", 5)) return argv[i] + 5;
    }
    return "PCBDCOM.CFG";
}

/* ----- .EXE / TSR entry ----- */
/* Symbol at end of BSS — linker-supplied. Used to compute resident size.
 * All compilers we target expose _end or __end (BC31: _end; OpenWatcom: end;
 * MSC: _end). Fall back to a conservative constant if unavailable. */
extern char _end[];

int main(int argc, char **argv)
{
    const char *cfg;
    int n;
    unsigned int resident_paragraphs;
    unsigned int psp_seg;

    printf("pcbdcom v1.2 — PCB DOS COM (WCSC COMM-DRV replacement)\n");
    printf("the crew 4free — GPLv3\n\n");

    cfg = find_cfg_arg(argc, argv);
    n = parse_config(cfg);
    if (n <= 0) {
        printf("pcbdcom: no ports configured (config: %s)\n", cfg);
        return 1;
    }

    if (pcbdcom_install() == 0) {
        printf("pcbdcom: no ports came online — aborting.\n");
        return 2;
    }

    printf("pcbdcom: %d port(s) online, installing TSR...\n", n);

    /* TSR install: compute resident size = end-of-BSS - PSP + safety margin.
     * PSP is 256 bytes below the loaded image on DOS EXE (CS = PSP + 0x10).
     * _end gives us the top of BSS relative to DS. Convert to paragraphs. */
    {
        unsigned long end_off = (unsigned long)(unsigned int)_end;
        /* Add PSP (256 bytes) + safety stack (256 bytes) */
        resident_paragraphs = (unsigned int)((end_off + 256 + 256 + 15) >> 4);
    }

    /* Get PSP segment for _dos_keep. OpenWatcom/BC/MSC all have this. */
    psp_seg = 0;
#if defined(__WATCOMC__)
    /* OpenWatcom: _psp declared in stdlib.h */
    psp_seg = _psp;
    _dos_keep(0, resident_paragraphs);
#elif defined(__BORLANDC__) || defined(__TURBOC__)
    /* BC31: keep() function */
    keep(0, resident_paragraphs);
#elif defined(_MSC_VER)
    /* MSC: _dos_keep — same as OpenWatcom */
    _dos_keep(0, resident_paragraphs);
#else
#  error "Unknown compiler — add TSR-install path"
#endif

    /* Not reached */
    (void)psp_seg;
    return 0;
}

/* ----- .SYS / DEVICE entry -----
 *
 * DROPPED in v1.2 for WCSC parity: original COMM-DRV shipped ONLY as
 * COMMTSR.EXE (a TSR), never as a .SYS device driver. Sysops load us
 * from AUTOEXEC.BAT with `PCBDTSR.EXE /F=PCBDCOM.CFG` — same as they
 * used to load COMMTSR.EXE.
 *
 * If .SYS-mode is needed later, resurrect device_entry() with correct
 * CONFIG.SYS request-header dispatch. Old skeleton in commit history.
 */
