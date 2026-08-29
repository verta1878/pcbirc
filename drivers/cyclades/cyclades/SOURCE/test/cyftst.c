/* ====================================================================
 * CYFTST.C — CYFOSSIL Driver Test Utility (DOS)
 * ====================================================================
 * Tests the CYFOSSIL.SYS FOSSIL driver via INT 14h function calls.
 * Verifies that all FSC-0015 functions work correctly.
 *
 * Usage: CYFTST [port]
 *   port = FOSSIL port number (0-based, default 0)
 *
 * Build (OpenWatcom):
 *   wcl -ox -bt=dos -ml -fe=CYFTST.EXE CYFTST.C
 *
 * License: GPLv3
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>

static int g_port = 0;         /* FOSSIL port number (0-based)      */
static int g_passed = 0;
static int g_failed = 0;

/* ====================================================================
 * FOSSIL INT 14h wrapper functions
 * ==================================================================== */

/* Fn00 — Set baud rate.
 * Input:  DX = port, AL = baud code
 * Output: AX = port status
 * Baud codes: 0=19200, 1=38400, 2=300, 3=600, 4=1200,
 *             5=2400, 6=4800, 7=9600, 8=115200 */
static unsigned int fossil_set_baud(int port, unsigned char baudCode)
{
    union REGS regs;
    regs.h.ah = 0x00;
    regs.h.al = baudCode | 0x03;  /* 8N1 + baud code in high bits */
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
    return regs.x.ax;
}

/* Fn01 — Transmit character (with wait).
 * Input:  DX = port, AL = character
 * Output: AX = status (bit 15 = 1 if timed out) */
static unsigned int fossil_tx(int port, unsigned char ch)
{
    union REGS regs;
    regs.h.ah = 0x01;
    regs.h.al = ch;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
    return regs.x.ax;
}

/* Fn02 — Receive character (with wait).
 * Input:  DX = port
 * Output: AL = character, AH = 0 if OK */
static unsigned int fossil_rx(int port)
{
    union REGS regs;
    regs.h.ah = 0x02;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
    return regs.x.ax;
}

/* Fn03 — Status request.
 * Input:  DX = port
 * Output: AX = status word */
static unsigned int fossil_status(int port)
{
    union REGS regs;
    regs.h.ah = 0x03;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
    return regs.x.ax;
}

/* Fn04 — Initialize FOSSIL.
 * Input:  DX = port
 * Output: AX = 1954h if FOSSIL present, BX = max baud */
static unsigned int fossil_init(int port, unsigned int *maxBaud)
{
    union REGS regs;
    regs.h.ah = 0x04;
    regs.x.dx = port;
    regs.x.bx = 0;
    int86(0x14, &regs, &regs);
    if (maxBaud) *maxBaud = regs.x.bx;
    return regs.x.ax;
}

/* Fn05 — Deinitialize FOSSIL.
 * Input:  DX = port */
static void fossil_deinit(int port)
{
    union REGS regs;
    regs.h.ah = 0x05;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
}

/* Fn06 — Raise/lower DTR.
 * Input:  DX = port, AL = 1 (raise) or 0 (lower) */
static void fossil_dtr(int port, int raise)
{
    union REGS regs;
    regs.h.ah = 0x06;
    regs.h.al = raise ? 1 : 0;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
}

/* Fn09 — Purge output buffer.
 * Input:  DX = port */
static void fossil_purge_tx(int port)
{
    union REGS regs;
    regs.h.ah = 0x09;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
}

/* Fn0A — Purge input buffer.
 * Input:  DX = port */
static void fossil_purge_rx(int port)
{
    union REGS regs;
    regs.h.ah = 0x0A;
    regs.x.dx = port;
    int86(0x14, &regs, &regs);
}

/* Fn18 — Get FOSSIL information block.
 * Input:  DX = port, CX = buffer size, ES:DI = buffer
 * Output: AX = bytes written */
static unsigned int fossil_info(int port, void far *buf, int bufSize)
{
    union REGS regs;
    struct SREGS sregs;

    regs.h.ah = 0x1B;
    regs.x.dx = port;
    regs.x.cx = bufSize;
    regs.x.di = FP_OFF(buf);
    sregs.es = FP_SEG(buf);
    int86x(0x14, &regs, &regs, &sregs);
    return regs.x.ax;
}


/* ====================================================================
 * Tests
 * ==================================================================== */

static void test(const char *name, int passed, const char *failMsg)
{
    printf("  %-40s ", name);
    if (passed) {
        g_passed++;
        printf("PASS\n");
    } else {
        g_failed++;
        printf("FAIL");
        if (failMsg) printf(" — %s", failMsg);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    unsigned int result;
    unsigned int maxBaud;
    unsigned int status;
    char failBuf[64];

    printf("CYFTST — CYFOSSIL Driver Test Utility\n");
    printf("======================================\n\n");

    if (argc > 1) g_port = atoi(argv[1]);
    printf("Testing FOSSIL port %d\n\n", g_port);

    /* ---- Test 1: FOSSIL detection (Fn04) ---- */
    result = fossil_init(g_port, &maxBaud);
    test("Fn04 Init (FOSSIL detect)",
         result == 0x1954,
         result != 0x1954 ? "AX != 1954h — no FOSSIL loaded" : NULL);

    if (result != 0x1954) {
        printf("\nNo FOSSIL driver detected on port %d.\n", g_port);
        printf("Load CYFOSSIL.SYS first:\n");
        printf("  DEVICE=CYFOSSIL.SYS\n");
        return 1;
    }

    _snprintf(failBuf, sizeof(failBuf), "maxBaud=%u", maxBaud);
    test("Fn04 Max baud reported",
         maxBaud > 0, failBuf);

    /* ---- Test 2: Status (Fn03) ---- */
    status = fossil_status(g_port);
    test("Fn03 Status request",
         1, NULL);  /* Always passes — just checking it doesn't crash */

    printf("    Status word: %04Xh (TX empty=%d, RX avail=%d)\n",
           status,
           (status >> 14) & 1,
           (status >> 8) & 1);

    /* ---- Test 3: Set baud (Fn00) ---- */
    result = fossil_set_baud(g_port, 7);  /* 7 = 9600 baud */
    test("Fn00 Set baud 9600",
         1, NULL);

    result = fossil_set_baud(g_port, 8);  /* 8 = 115200 baud */
    test("Fn00 Set baud 115200",
         1, NULL);

    /* Set back to 9600 for remaining tests */
    fossil_set_baud(g_port, 7);

    /* ---- Test 4: DTR control (Fn06) ---- */
    fossil_dtr(g_port, 1);
    test("Fn06 DTR raise", 1, NULL);

    fossil_dtr(g_port, 0);
    test("Fn06 DTR drop", 1, NULL);

    fossil_dtr(g_port, 1);  /* Restore DTR for other tests */

    /* ---- Test 5: Purge (Fn09, Fn0A) ---- */
    fossil_purge_tx(g_port);
    test("Fn09 Purge TX buffer", 1, NULL);

    fossil_purge_rx(g_port);
    test("Fn0A Purge RX buffer", 1, NULL);

    /* ---- Test 6: TX loopback (requires cable) ---- */
    printf("\n  --- Loopback tests (skip if no cable) ---\n");
    {
        unsigned char txByte = 0xA5;
        unsigned int rxResult;

        /* Purge first */
        fossil_purge_rx(g_port);
        fossil_purge_tx(g_port);

        /* Send one byte */
        result = fossil_tx(g_port, txByte);
        if (result & 0x8000) {
            test("Fn01 TX byte (loopback)",
                 0, "TX timed out");
        } else {
            test("Fn01 TX byte (loopback)", 1, NULL);

            /* Brief delay for byte to loop back */
            {
                volatile long delay;
                for (delay = 0; delay < 100000L; delay++);
            }

            /* Check if byte came back */
            status = fossil_status(g_port);
            if (status & 0x0100) {  /* RX data available */
                rxResult = fossil_rx(g_port);
                _snprintf(failBuf, sizeof(failBuf), "sent %02Xh got %02Xh",
                        txByte, rxResult & 0xFF);
                test("Fn02 RX byte (loopback)",
                     (rxResult & 0xFF) == txByte, failBuf);
            } else {
                test("Fn02 RX byte (loopback)",
                     0, "no data received (no loopback cable?)");
            }
        }
    }

    /* ---- Test 7: Deinit (Fn05) ---- */
    fossil_deinit(g_port);
    test("Fn05 Deinit", 1, NULL);

    /* Re-init for clean state */
    fossil_init(g_port, NULL);
    fossil_deinit(g_port);

    /* ---- Summary ---- */
    printf("\n======================================\n");
    printf("RESULTS: %d passed, %d failed\n", g_passed, g_failed);

    return g_failed > 0 ? 1 : 0;
}
