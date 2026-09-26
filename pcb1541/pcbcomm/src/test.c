/* ============================================================================
 * test.c — pcbcomm port test / diagnostics utility
 *
 * Replaces Clark's TEST.EXE (WCSC COMM-DRV).
 * Probes a serial port, identifies the UART chip, runs a loopback
 * test, and reports results.  Used by sysops to verify hardware
 * before going live with PCBoard.
 *
 * Usage:  TEST <port> [options]
 *   port     Port number (1-24) as configured in PCBCOMM.CFG
 *   -l       Loopback test (TX→RX with MCR bit 4)
 *   -a       Auto-detect all ports (probe 1-24)
 *   -v       Verbose output
 *   -f file  Use alternate config file (default PCBCOMM.CFG)
 *
 * License: GPLv3 (pcbirc crew)
 * Authors: wrench (transport/FOSSIL)
 * ==========================================================================*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "pcbdcom.h"
#include "backend.h"

/* ---- UART register offsets (standard 8250/16550) ----------------------- */
#define UART_RBR    0   /* Receive Buffer (read) */
#define UART_THR    0   /* Transmit Holding (write) */
#define UART_IER    1   /* Interrupt Enable */
#define UART_IIR    2   /* Interrupt Ident (read) */
#define UART_FCR    2   /* FIFO Control (write) */
#define UART_LCR    3   /* Line Control */
#define UART_MCR    4   /* Modem Control */
#define UART_LSR    5   /* Line Status */
#define UART_MSR    6   /* Modem Status */
#define UART_SCR    7   /* Scratch Register */

/* ---- Chip identification ----------------------------------------------- */
static const char *identify_chip(unsigned int base)
{
    unsigned char scr_save, iir, fcr_test;

    /* Test 1: scratch register exists? (not on original 8250) */
    scr_save = inp(base + UART_SCR);
    outp(base + UART_SCR, 0xAA);
    if (inp(base + UART_SCR) != 0xAA) {
        outp(base + UART_SCR, scr_save);
        return "8250 (no scratch register)";
    }
    outp(base + UART_SCR, 0x55);
    if (inp(base + UART_SCR) != 0x55) {
        outp(base + UART_SCR, scr_save);
        return "8250A";
    }
    outp(base + UART_SCR, scr_save);

    /* Test 2: FIFO support? Enable FIFOs and check IIR bits 6-7 */
    outp(base + UART_FCR, 0xC1);  /* enable FIFOs, 14-byte trigger */
    iir = inp(base + UART_IIR);
    outp(base + UART_FCR, 0x00);  /* disable FIFOs */

    if ((iir & 0xC0) == 0xC0) {
        /* FIFOs work — 16550A or better */
        /* Test 3: 64-byte FIFO? (16750) */
        outp(base + UART_FCR, 0xE1);  /* try 64-byte mode */
        fcr_test = inp(base + UART_IIR);
        outp(base + UART_FCR, 0x00);
        if (fcr_test & 0x20)
            return "16750 (64-byte FIFO)";
        return "16550A (16-byte FIFO)";
    }
    if ((iir & 0xC0) == 0x80)
        return "16550 (broken FIFO)";

    return "16450 (no FIFO)";
}

/* ---- Port presence test ------------------------------------------------ */
static int port_present(unsigned int base)
{
    unsigned char mcr_save, msr;

    if (base == 0) return 0;

    /* Quick test: write to MCR, read back */
    mcr_save = inp(base + UART_MCR);
    outp(base + UART_MCR, 0x10);  /* loopback mode */
    outp(base + UART_THR, 0xAA);  /* send test byte */

    /* Small delay for slow hardware */
    { volatile int d; for (d = 0; d < 1000; d++); }

    msr = inp(base + UART_LSR);
    outp(base + UART_MCR, mcr_save);  /* restore */

    /* If LSR has any status at all, there's something at this address */
    return (msr != 0xFF);  /* 0xFF = no hardware (floating bus) */
}

/* ---- Loopback test ----------------------------------------------------- */
static int loopback_test(unsigned int base, int verbose)
{
    unsigned char mcr_save, lcr_save, ier_save;
    int i, errors = 0;
    unsigned char test_bytes[] = { 0x00, 0x55, 0xAA, 0xFF, 0x0F, 0xF0 };
    int n_tests = sizeof(test_bytes) / sizeof(test_bytes[0]);

    /* Save registers */
    mcr_save = inp(base + UART_MCR);
    lcr_save = inp(base + UART_LCR);
    ier_save = inp(base + UART_IER);

    /* Disable interrupts, set 8N1, enable internal loopback */
    outp(base + UART_IER, 0x00);
    outp(base + UART_LCR, 0x03);    /* 8N1 */
    outp(base + UART_MCR, 0x10);     /* loopback bit */

    /* Drain any pending data */
    while (inp(base + UART_LSR) & 0x01)
        inp(base + UART_RBR);

    /* Send and verify each test byte */
    for (i = 0; i < n_tests; i++) {
        unsigned char sent = test_bytes[i];
        unsigned char recv;
        int timeout = 10000;

        outp(base + UART_THR, sent);

        /* Wait for byte to come back through loopback */
        while (!(inp(base + UART_LSR) & 0x01) && --timeout > 0)
            ;

        if (timeout == 0) {
            if (verbose)
                printf("    byte 0x%02X: TIMEOUT (no loopback)\n", sent);
            errors++;
            continue;
        }

        recv = inp(base + UART_RBR);
        if (recv != sent) {
            if (verbose)
                printf("    byte 0x%02X: got 0x%02X — MISMATCH\n", sent, recv);
            errors++;
        } else if (verbose) {
            printf("    byte 0x%02X: OK\n", sent);
        }
    }

    /* Restore registers */
    outp(base + UART_IER, ier_save);
    outp(base + UART_LCR, lcr_save);
    outp(base + UART_MCR, mcr_save);

    return errors;
}

/* ---- Standard COM port base addresses (for auto-detect) ---------------- */
static const unsigned int std_bases[] = {
    0x3F8,  /* COM1 */
    0x2F8,  /* COM2 */
    0x3E8,  /* COM3 */
    0x2E8,  /* COM4 */
    0
};

/* ---- Auto-detect all standard COM ports -------------------------------- */
static void auto_detect(int verbose)
{
    int i;
    printf("Auto-detecting standard COM ports...\n\n");
    printf("  Port  Base   Chip                    Loopback\n");
    printf("  ----  ----   ----------------------  --------\n");

    for (i = 0; std_bases[i]; i++) {
        unsigned int base = std_bases[i];
        printf("  COM%d  0x%03X  ", i + 1, base);

        if (!port_present(base)) {
            printf("(not present)\n");
            continue;
        }

        printf("%-22s  ", identify_chip(base));

        /* Quick loopback */
        if (loopback_test(base, 0) == 0)
            printf("PASS\n");
        else
            printf("FAIL\n");
    }
    printf("\n");
}

/* ---- Test a specific configured port ----------------------------------- */
static void test_port(unsigned int port_num, const char *cfg, int verbose,
                      int do_loopback)
{
    /* Simple approach: test the standard base for the given port number.
     * A fuller version would parse PCBCOMM.CFG to find the actual base
     * address for multiport cards.  For now, if port <= 4 use the
     * standard base; otherwise read the config. */
    unsigned int base = 0;

    if (port_num >= 1 && port_num <= 4) {
        base = std_bases[port_num - 1];
    } else {
        /* Read config to find the base address for this port */
        FILE *f;
        char line[128], card[16];
        unsigned int p, sub, b, irq, seg;
        char fossil;

        f = fopen(cfg, "r");
        if (!f) {
            printf("Cannot open config: %s\n", cfg);
            return;
        }
        while (fgets(line, sizeof(line), f)) {
            if (line[0] == '#' || line[0] == '\n') continue;
            if (sscanf(line, "%u %15s %u %i %u %u %c",
                       &p, card, &sub, &b, &irq, &seg, &fossil) >= 4) {
                if (p == port_num) { base = b; break; }
            }
        }
        fclose(f);
    }

    if (base == 0) {
        printf("Port %u: base address unknown (not in %s)\n", port_num, cfg);
        return;
    }

    printf("Testing port %u at base 0x%03X...\n\n", port_num, base);

    /* Presence check */
    printf("  Presence: ");
    if (!port_present(base)) {
        printf("NOT DETECTED — no hardware at 0x%03X\n", base);
        return;
    }
    printf("OK\n");

    /* Chip identification */
    printf("  Chip:     %s\n", identify_chip(base));

    /* Modem status lines */
    {
        unsigned char msr = inp(base + UART_MSR);
        printf("  Signals:  CTS=%d  DSR=%d  DCD=%d  RI=%d\n",
               (msr >> 4) & 1, (msr >> 5) & 1,
               (msr >> 7) & 1, (msr >> 6) & 1);
    }

    /* Line status */
    {
        unsigned char lsr = inp(base + UART_LSR);
        printf("  Status:   DR=%d  OE=%d  PE=%d  FE=%d  BI=%d  THRE=%d  TEMT=%d\n",
               lsr & 1, (lsr >> 1) & 1, (lsr >> 2) & 1,
               (lsr >> 3) & 1, (lsr >> 4) & 1, (lsr >> 5) & 1,
               (lsr >> 6) & 1);
    }

    /* Loopback test */
    if (do_loopback) {
        int errs;
        printf("\n  Loopback test (%d patterns):\n", 6);
        errs = loopback_test(base, verbose);
        printf("  Result:   %s (%d error%s)\n",
               errs == 0 ? "PASS" : "FAIL", errs, errs == 1 ? "" : "s");
    }

    printf("\n");
}

/* ---- Usage ------------------------------------------------------------- */
static void usage(void)
{
    printf("pcbcomm TEST — port diagnostics utility\n");
    printf("the crew 4free — GPLv3\n\n");
    printf("Usage:  TEST <port> [options]\n");
    printf("        TEST -a        Auto-detect all standard COM ports\n\n");
    printf("Options:\n");
    printf("  -l       Run loopback test (TX->RX via MCR internal loop)\n");
    printf("  -a       Auto-detect COM1-COM4\n");
    printf("  -v       Verbose output\n");
    printf("  -f file  Alternate config file (default PCBCOMM.CFG)\n");
}

/* ---- Main -------------------------------------------------------------- */
int main(int argc, char **argv)
{
    int i, port_num = 0, do_loopback = 0, do_autodetect = 0, verbose = 0;
    const char *cfg = "PCBCOMM.CFG";

    printf("pcbcomm TEST — port diagnostics\n");
    printf("the crew 4free — GPLv3\n\n");

    if (argc < 2) {
        usage();
        return 0;
    }

    /* Parse args */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (argv[i][1]) {
            case 'l': case 'L': do_loopback = 1; break;
            case 'a': case 'A': do_autodetect = 1; break;
            case 'v': case 'V': verbose = 1; break;
            case 'f': case 'F':
                if (i + 1 < argc) cfg = argv[++i];
                break;
            case '?': case 'h': case 'H': usage(); return 0;
            default:
                printf("Unknown switch: %s\n", argv[i]);
                usage();
                return 1;
            }
        } else {
            port_num = atoi(argv[i]);
        }
    }

    if (do_autodetect) {
        auto_detect(verbose);
        return 0;
    }

    if (port_num < 1 || port_num > 24) {
        printf("Invalid port number (must be 1-24)\n");
        usage();
        return 1;
    }

    test_port((unsigned int)port_num, cfg, verbose, do_loopback);
    return 0;
}
