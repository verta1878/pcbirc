/* ============================================================================
 * pcbdcom.c — PCB DOS COM main entry
 *
 * PCBDCOM is a hardware serial layer for DOS, providing:
 *   - Direct 8250/16450/16550 UART access on COM1-COM4 (and beyond)
 *   - FOSSIL-compliant INT 14h interrupt API
 *   - IRQ-driven TX/RX ring buffers
 *   - Modem control (DTR/RTS/DCD/CTS/DSR/RI monitoring)
 *
 * Dual-mode loader: can be loaded as either
 *   1. CONFIG.SYS device driver: DEVICE=C:\PCBDCOM\PCBDCOM.DRV /IRQ=4 ...
 *   2. AUTOEXEC.BAT TSR:         PCBDCOM /IRQ=4 ...
 *
 * The same binary handles both — entry point sniffs load mode from
 * DS:SI (command-line pointer on CONFIG.SYS, PSP address on AUTOEXEC).
 *
 * Reference sources (all crew-owned or free):
 *   drivers/netfosdl/*.pas       — Free Pascal DOS FOSSIL (closest match,
 *                                  ~70% of v1 covered; port Pascal→C)
 *   drivers/SIO/v2/uart/         — OS/2 UART code (register-level ref)
 *   Linux drivers/tty/serial/8250/  (GPL, for chip probe idioms)
 *   Linux drivers/char/pcxx.c     (GPL, DigiBoard PC/Xe firmware embedded)
 *   Linux drivers/char/epca.c     (GPL, DigiBoard AccelePort firmware embedded)
 *   Linux drivers/char/rocket.c   (GPL, Comtrol RocketPort firmware embedded)
 *   Linux drivers/char/istallion.c (GPL, Stallion EasyIO firmware embedded)
 *   NetBSD/OpenBSD sys/dev/isa/boca.c (BSD, dumb Boca BB-100x/BB-2016)
 *
 * v1 card coverage (per LINUX_BSD_HUNT.md analysis, crew-scratch):
 *   ✓ Standard COM1-4 (8250/16450/16550)  — native from datasheets
 *   ✓ Boca BB-1004/1008/2016 (dumb multi) — extend 8250 + IRQ-mux
 *   ✓ Cyclades Cyclom-Y                    — drivers/cyclades/ (in repo)
 *   ✓ DigiBoard PC/Xe, PC/Xi, PC/Xeve      — port pcxx.c
 *   ✓ DigiBoard AccelePort                 — port epca.c
 *   ✓ Comtrol RocketPort                   — port rocket.c
 *   ✓ Stallion EasyIO                      — port istallion.c
 *   ✗ Arnet SmartPort Plus                 — SKIP v1 (no free driver;
 *                                            firmware XABIOS/XACOOK/XACOMX
 *                                            proprietary Digi legacy;
 *                                            user supplies from vendor disk)
 *
 * License: GPLv3
 * ==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uart.h"

/* Build config */
#define PCBDCOM_VERSION "0.1-scaffold"
#define PCBDCOM_MAX_PORTS 8  /* COM1..COM8; extends further with smart cards */

/* Per-port state */
typedef struct {
    unsigned int base;          /* I/O base address                       */
    unsigned char irq;          /* IRQ number                             */
    long baud;                  /* Configured baud rate                   */
    unsigned char lcr;          /* Line control (bits/parity/stop)        */
    uart_type_t chip;           /* Detected chip type                     */
    unsigned char *rx_buf;      /* Receive ring buffer                    */
    unsigned int rx_head;
    unsigned int rx_tail;
    unsigned int rx_size;
    unsigned char *tx_buf;      /* Transmit ring buffer                   */
    unsigned int tx_head;
    unsigned int tx_tail;
    unsigned int tx_size;
    int open;                   /* Non-zero when in use                   */
} pcbdcom_port_t;

static pcbdcom_port_t g_ports[PCBDCOM_MAX_PORTS];

/* Load-mode enum */
typedef enum {
    LOAD_MODE_UNKNOWN,
    LOAD_MODE_DEVICE,   /* Loaded from CONFIG.SYS DEVICE= line            */
    LOAD_MODE_TSR       /* Loaded from AUTOEXEC.BAT / command line        */
} pcbdcom_load_mode_t;

/* --------------------------------------------------------------------------
 * Detect load mode by inspecting DS:SI and PSP layout.
 * When loaded via CONFIG.SYS DEVICE=, DS:SI points to the command-line
 * arguments after the DEVICE= line (space-delimited args). When loaded
 * via AUTOEXEC.BAT, we're a normal COM/EXE and DS:SI points to the PSP.
 * -------------------------------------------------------------------------- */
static pcbdcom_load_mode_t detect_load_mode(void) {
    /* TODO: real detection via checking for driver header signature
     * in the load segment vs standard PSP layout. Stub for now. */
    return LOAD_MODE_UNKNOWN;
}

/* --------------------------------------------------------------------------
 * FOSSIL INT 14h dispatch (implemented in fossil.c — not yet written).
 * -------------------------------------------------------------------------- */
extern void fossil_install(void);
extern void fossil_uninstall(void);

/* --------------------------------------------------------------------------
 * Command-line parsing
 * -------------------------------------------------------------------------- */
static int parse_args(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "/IRQ=", 5) == 0) {
            /* TODO: parse IRQ number */
        } else if (strncmp(argv[i], "/PORT=", 6) == 0) {
            /* TODO: parse I/O port base */
        } else if (strncmp(argv[i], "/BAUD=", 6) == 0) {
            /* TODO: parse baud rate */
        } else if (strcmp(argv[i], "/U") == 0) {
            /* Uninstall the TSR */
            fossil_uninstall();
            return 0;
        } else if (strcmp(argv[i], "/?") == 0 || strcmp(argv[i], "-?") == 0) {
            printf("pcbdcom " PCBDCOM_VERSION " - PCB DOS COM driver\n");
            printf("Usage: PCBDCOM [/IRQ=n] [/PORT=xxx] [/BAUD=n] [/U]\n");
            printf("  /IRQ=n     hardware IRQ (default: 4 for COM1)\n");
            printf("  /PORT=xxx  I/O base in hex (default: 3F8 for COM1)\n");
            printf("  /BAUD=n    initial baud rate (default: 38400)\n");
            printf("  /U         uninstall resident driver\n");
            return 0;
        }
    }
    return 1;
}

/* --------------------------------------------------------------------------
 * Main entry
 * -------------------------------------------------------------------------- */
int main(int argc, char **argv) {
    pcbdcom_load_mode_t mode = detect_load_mode();

    (void)mode;  /* TODO: dispatch on mode */

    memset(g_ports, 0, sizeof(g_ports));

    if (!parse_args(argc, argv)) {
        return 0;
    }

    /* Probe COM1 by default */
    g_ports[0].chip = uart_probe(COM1_BASE);
    if (g_ports[0].chip == UART_TYPE_NONE) {
        fprintf(stderr, "pcbdcom: no UART detected at 0x%X\n", COM1_BASE);
        return 1;
    }
    printf("pcbdcom: detected UART type %d at 0x%X\n",
           (int)g_ports[0].chip, COM1_BASE);

    /* Install FOSSIL INT 14h vector */
    fossil_install();

    printf("pcbdcom " PCBDCOM_VERSION " installed.\n");
    return 0;
}
