/* ============================================================================
 * pcbdcom.c — main entry point, config parser, backend registry
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
int main(int argc, char **argv)
{
    const char *cfg;
    int n;

    printf("pcbdcom v1.0 — PCB DOS COM (WCSC COMM-DRV replacement)\n");
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

    /* TSR — keep in memory. Size in paragraphs computed by DOS from PSP.
     * Real code would use dos_keep_tsr(psp_size / 16, 0). For v1
     * we just exit — CONFIG.SYS DEVICE path is the primary target. */
    printf("pcbdcom: use CONFIG.SYS DEVICE= line for full TSR install.\n");
    return 0;
}

/* ----- .SYS / DEVICE entry ----- *
 * Request-header dispatch per DOS device driver spec. Only INIT (0)
 * is meaningful for a character-mode driver like pcbdcom — after
 * INIT returns, DOS never calls us again; we live via INT 14h hook. */
struct device_request {
    unsigned char length;
    unsigned char unit;
    unsigned char command;
    unsigned int  status;
    unsigned char reserved[8];
    /* INIT-specific: */
    unsigned char n_units;
    void __far   *break_addr;
    char __far   *cmdline;
};

void __far device_entry(struct device_request __far *req)
{
    if (req->command != 0x00) {         /* INIT only */
        req->status = 0x8003;           /* error, unknown command */
        return;
    }

    /* INIT: parse config, install. cmdline points at args after
     * DEVICE=PCBDCOM.SYS in CONFIG.SYS. */
    (void)parse_config("PCBDCOM.CFG");
    (void)pcbdcom_install();

    /* Report resident break address (end of driver code + BSS) —
     * everything from break_addr onward is released back to DOS. */
    req->status = 0x0100;               /* done */
    req->n_units = 0;                   /* character device */
    /* break_addr set by linker/startup code */
}
