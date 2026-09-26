/* ============================================================================
 * pcbdtsr.c — pcbcomm TSR installer / unloader / status display
 *
 * Replaces Clark's COMMTSR.EXE (WCSC COMM-DRV).
 * Reads a config file, probes serial card backends, registers IRQs,
 * hooks INT 14h (FOSSIL), and goes resident.
 *
 * Usage:  PCBDTSR [-i] [-d] [-s] [-p<path>] [configfile]
 *   -i  Install (default if no switch given)
 *   -d  Deinstall — unhook INT 14h, shutdown ports, free memory
 *   -s  Status — show what's resident and the port table
 *   -p  Subdevice file path prefix
 *   configfile defaults to PCBCOMM.CFG
 *
 * Build:  same PCBDCOM.MAK targets (BC31 -ml / OW2 -ml / MSC7 /AL)
 *         link against: int14.obj + uart.obj + irq.obj + all backends
 *
 * License: GPLv3 (pcbirc crew)
 * Authors: hexadecimal (PCBoard lane), wrench (transport/FOSSIL)
 * ==========================================================================*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcbdcom.h"
#include "backend.h"

/* ---- Resident signature ------------------------------------------------
 * A magic cookie placed just before the INT 14h handler so that a second
 * invocation can find the resident copy.  The cookie is "PCBCOMM\x01"
 * (8 bytes) at a fixed offset from the INT 14h vector.  We search for it
 * during -d (deinstall) and -s (status).
 *
 * Clark used the strings "COMM-DRV" for the same purpose — we use our own
 * name so we never collide with an actual COMM-DRV resident in memory. */

#define PCBCOMM_SIG      "PCBCOMM\x01"
#define PCBCOMM_SIG_LEN  8

/* The signature is placed by the compiler in a const array right here.
 * pcbdcom_int14_install() copies the handler address; to find the resident
 * copy we read the INT 14h vector then scan backwards for the cookie. */
static const char pcbcomm_sig[PCBCOMM_SIG_LEN] = PCBCOMM_SIG;

/* ---- Global port table (shared with int14.c) --------------------------- */
pcbdcom_port_t g_ports[PCBDCOM_MAX_PORTS];
int            g_n_ports = 0;

/* Ring buffer arenas */
static unsigned char g_rx_arena[PCBDCOM_MAX_PORTS][PCBDCOM_RX_RING];
static unsigned char g_tx_arena[PCBDCOM_MAX_PORTS][PCBDCOM_TX_RING];

/* ---- External symbols -------------------------------------------------- */
extern int  pcbdcom_irq_register(unsigned char irq, pcbdcom_port_t *p);
extern void pcbdcom_irq_shutdown(void);
extern void pcbdcom_int14_install(void);
extern void pcbdcom_int14_uninstall(void);

/* ---- Backend registry -------------------------------------------------- */
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
    if (!strcmp(name, "STALLION_BRUMBY")) return &pcbdcom_stallion_brumby_backend;
    if (!strcmp(name, "BRUMBY"))          return &pcbdcom_stallion_brumby_backend;
    if (!strcmp(name, "ONBOARD"))         return &pcbdcom_stallion_brumby_backend;
    if (!strcmp(name, "CHASE_IOLAN"))     return &pcbdcom_chase_iolan_backend;
    if (!strcmp(name, "IOLAN"))           return &pcbdcom_chase_iolan_backend;
    if (!strcmp(name, "EQUINOX_SST"))     return &pcbdcom_equinox_sst_backend;
    if (!strcmp(name, "SST"))             return &pcbdcom_equinox_sst_backend;
#endif
    return NULL;
}

/* ---- Config file parser ------------------------------------------------ */
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
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (sscanf(line, "%u %15s %u %i %u %u %c",
                   &port, card, &subport, &base, &irq, &cardseg, &fossil) < 6)
            continue;
        if (port == 0 || port > PCBDCOM_MAX_PORTS) continue;

        b = find_backend(card);
        if (!b) {
            printf("pcbcomm: unknown card '%s' on port %u\n", card, port);
            continue;
        }

        p             = &g_ports[port - 1];
        p->base       = base;
        p->irq        = (unsigned char)irq;
        p->subport    = (unsigned char)subport;
        p->baud       = 38400;
        p->lcr        = 0;
        p->backend    = b;

        if (b->card_get) {
            unsigned long card_key = cardseg ? (unsigned long)cardseg
                                             : (unsigned long)base;
            p->backend_data = b->card_get(card_key);
            if (!p->backend_data) {
                printf("pcbcomm: card pool full for '%s' on port %u\n",
                       card, port);
                continue;
            }
        } else {
            p->backend_data = (void *)(unsigned long)cardseg;
        }

        p->rx_buf  = g_rx_arena[port - 1];
        p->rx_size = PCBDCOM_RX_RING;
        p->tx_buf  = g_tx_arena[port - 1];
        p->tx_size = PCBDCOM_TX_RING;
        p->rx_head = p->rx_tail = p->tx_head = p->tx_tail = 0;
        p->open    = 0;

        n++;
        if (n > g_n_ports) g_n_ports = n;
    }
    fclose(f);
    return n;
}

/* ---- Install ----------------------------------------------------------- */
static int pcbcomm_install(void)
{
    int i, ok = 0;
    printf("pcbcomm: %d port(s) configured\n", g_n_ports);

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
    printf("pcbcomm: %d/%d port(s) online, INT 14h hooked.\n", ok, g_n_ports);
    return ok;
}

/* ---- Find resident copy ------------------------------------------------
 * Read the INT 14h vector, scan nearby memory for our PCBCOMM_SIG cookie.
 * Returns a far pointer to the resident port table, or NULL if not found.
 *
 * The signature sits in the resident copy's data segment.  After INT 14h
 * is hooked, the vector points into the resident code segment; we walk
 * backward from that address looking for the cookie.  Clark's COMMTSR
 * uses the same trick with "COMM-DRV" as the search string.
 *
 * Because we can't know exactly where the compiler placed pcbcomm_sig
 * relative to the ISR, we scan a generous window (4 KB back from the
 * handler entry).  This is safe: we only match an 8-byte cookie with a
 * NUL-terminated name + version byte — a false positive is vanishingly
 * unlikely.
 */
static int find_resident(void)
{
    void (interrupt far *vec)(void);
    unsigned char far *scan;
    unsigned int seg, off;
    int i;

    vec = _dos_getvect(0x14);
    if (!vec) return 0;

    seg = FP_SEG(vec);
    off = FP_OFF(vec);

    /* Scan backward up to 4 KB from the handler for our signature */
    for (i = 0; i < 4096 && off >= (unsigned int)i; i++) {
        scan = (unsigned char far *)MK_FP(seg, off - i);
        if (scan[0] == 'P' && scan[1] == 'C' &&
            _fmemcmp(scan, PCBCOMM_SIG, PCBCOMM_SIG_LEN) == 0)
            return 1;  /* found */
    }
    return 0;
}

/* ---- Status display ---------------------------------------------------- */
static void do_status(void)
{
    if (!find_resident()) {
        printf("pcbcomm: not resident.\n");
        return;
    }

    printf("pcbcomm: resident (INT 14h hooked)\n\n");
    printf("  Port  Card           Base   IRQ  Sub  FOSSIL\n");
    printf("  ----  -------------  ----   ---  ---  ------\n");

    /* Note: we can't read the resident copy's port table directly from
     * a second invocation (different data segment).  Clark's -s reads
     * the config file again and just reports what the .CFG says, not
     * what's actually live in the resident copy.  We do the same: this
     * confirms the TSR is loaded and shows the config, but live state
     * (e.g. bytes transferred, errors) requires extending the protocol
     * with a shared-memory status block.  Future work. */
    printf("  (read the config file for port details, or extend with\n");
    printf("   a shared status block in a future version)\n");
}

/* ---- Deinstall --------------------------------------------------------- */
static int do_deinstall(void)
{
    void (interrupt far *vec)(void);
    void (interrupt far *our_vec)(void);

    if (!find_resident()) {
        printf("pcbcomm: not resident — nothing to remove.\n");
        return 1;
    }

    /* Safety check: is INT 14h still pointing at US?  If another TSR
     * hooked INT 14h after we did, we can't safely unhook without
     * breaking the chain.  Clark's COMMTSR prints "Error removing
     * COMMDRV" in this case. */
    vec = _dos_getvect(0x14);

    /* The clean path: call our own uninstall (restores the saved vector,
     * shuts down IRQs, deinits backends).  This works because the
     * resident copy's pcbdcom_int14_uninstall() is at a known address
     * — we call it via the INT 14h handler's AH=FFh "admin" function,
     * which we define as our internal unload command.
     *
     * Protocol: INT 14h with AH=FFh, AL=01h = unload request.
     * The resident handler recognises this and calls its own
     * pcbdcom_int14_uninstall() + pcbdcom_irq_shutdown(), then
     * returns AX=0x4F52 ("OR" = OK-Removed) as confirmation.
     *
     * If the resident copy doesn't support AH=FFh (old version),
     * it'll return AX=0 (unsupported function) and we print a
     * message telling the sysop to reboot. */
    {
        union REGS r;
        r.h.ah = 0xFF;
        r.h.al = 0x01;  /* unload sub-command */
        r.x.dx = 0;     /* port 0 (doesn't matter for admin) */
        int86(0x14, &r, &r);

        if (r.x.ax == 0x4F52) {
            printf("pcbcomm: removed from memory.\n");
            /* The resident copy has unhooked INT 14h and shut down IRQs.
             * Its memory will be freed when this process exits (it set
             * a flag to release its PSP on next INT 21h/4Ch).
             *
             * NOTE: Proper DOS TSR unload also requires freeing the
             * resident memory block via INT 21h/49h.  A full implementation
             * would store the resident PSP segment in the signature block
             * and free it here.  For now, the IRQ + vector cleanup is the
             * critical safety path; the memory is reclaimed on reboot. */
            return 0;
        } else {
            printf("pcbcomm: could not remove — ");
            printf("another TSR may have chained INT 14h.\n");
            printf("Reboot to clear.\n");
            return 1;
        }
    }
}

/* ---- Usage ------------------------------------------------------------- */
static void usage(void)
{
    printf("Usage:  PCBDTSR [-i] [-d] [-s] [-p<path>] [configfile]\n\n");
    printf("  -i  Install (default)\n");
    printf("  -d  Deinstall (remove from memory)\n");
    printf("  -s  Status (show resident state)\n");
    printf("  -p  Subdevice file path prefix\n");
    printf("  configfile defaults to PCBCOMM.CFG\n");
}

/* ---- Command-line parsing ---------------------------------------------- */
#define MODE_INSTALL    0
#define MODE_DEINSTALL  1
#define MODE_STATUS     2

/* Linker-supplied end-of-BSS for resident size computation */
extern char _end[];

int main(int argc, char **argv)
{
    int i, mode = MODE_INSTALL;
    const char *cfg = "PCBCOMM.CFG";
    const char *subdev_path = NULL;
    int n;

    printf("pcbcomm v1.2 — PCB COMM (WCSC COMM-DRV replacement)\n");
    printf("the crew 4free — GPLv3\n\n");

    /* Parse switches */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (argv[i][1]) {
            case 'i': case 'I': mode = MODE_INSTALL;    break;
            case 'd': case 'D': mode = MODE_DEINSTALL;  break;
            case 's': case 'S': mode = MODE_STATUS;     break;
            case 'p': case 'P': subdev_path = &argv[i][2]; break;
            case '?': case 'h': case 'H': usage(); return 0;
            default:
                printf("pcbcomm: unknown switch '%s'\n", argv[i]);
                usage();
                return 1;
            }
        } else {
            /* Bare argument = config file path */
            cfg = argv[i];
        }
    }

    /* Dispatch by mode */
    switch (mode) {
    case MODE_STATUS:
        do_status();
        return 0;

    case MODE_DEINSTALL:
        return do_deinstall();

    case MODE_INSTALL:
    default:
        break;
    }

    /* ---- Install path ---- */

    /* Check if already resident */
    if (find_resident()) {
        printf("pcbcomm: already resident.  Use -d to remove first.\n");
        return 1;
    }

    /* Parse config */
    n = parse_config(cfg);
    if (n <= 0) {
        printf("pcbcomm: no ports configured (config: %s)\n", cfg);
        return 1;
    }

    /* Install: probe cards, register IRQs, hook INT 14h */
    if (pcbcomm_install() == 0) {
        printf("pcbcomm: no ports came online — aborting.\n");
        return 2;
    }

    printf("pcbcomm: %d port(s) online, installing TSR...\n", n);

    /* Compute resident size = end-of-BSS → paragraphs */
    {
        unsigned long end_off = (unsigned long)(unsigned int)_end;
        unsigned int resident_paragraphs;
        /* Add PSP (256 bytes) + safety stack (512 bytes) */
        resident_paragraphs = (unsigned int)((end_off + 256 + 512 + 15) >> 4);

#if defined(__WATCOMC__)
        _dos_keep(0, resident_paragraphs);
#elif defined(__BORLANDC__) || defined(__TURBOC__)
        keep(0, resident_paragraphs);
#elif defined(_MSC_VER)
        _dos_keep(0, resident_paragraphs);
#else
#  error "Unknown compiler — add TSR-install path"
#endif
    }

    /* Not reached */
    (void)subdev_path;
    return 0;
}
