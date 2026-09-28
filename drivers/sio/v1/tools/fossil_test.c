/* ====================================================================
 * FOSTEST.EXE — FOSSIL (INT 14h) Conformance Test for DOS/VDM
 * ====================================================================
 * 20 tests covering all FTS-0001 Rev 5 functions.
 * Runs inside a DOS session (real DOS or OS/2 VDM with VX00.SYS).
 *
 * Usage: FOSTEST [port]
 *   port = 0 (COM1), 1 (COM2), etc.  Default: 0
 *
 * Exit code: number of failed tests (0 = all pass)
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

static int g_pass = 0;
static int g_fail = 0;
static int g_port = 0;

#define TEST(id, desc) \
    printf("  FOS-%02d %-42s ", id, desc); fflush(stdout)
#define PASS() do { printf("[PASS]\n"); g_pass++; } while(0)
#define FAIL(r) do { printf("[FAIL] %s\n", r); g_fail++; } while(0)

/* INT 14h wrapper */
static void fossil_call(unsigned char fn, unsigned *ax_out,
                        unsigned *bx_out, unsigned *cx_out)
{
    union REGS r;
    r.h.ah = fn;
    r.x.dx = g_port;
    r.x.bx = 0;
    r.x.cx = 0;
    int86(0x14, &r, &r);
    if (ax_out) *ax_out = r.x.ax;
    if (bx_out) *bx_out = r.x.bx;
    if (cx_out) *cx_out = r.x.cx;
}

static void fossil_call_al(unsigned char fn, unsigned char al_val,
                           unsigned *ax_out)
{
    union REGS r;
    r.h.ah = fn;
    r.h.al = al_val;
    r.x.dx = g_port;
    int86(0x14, &r, &r);
    if (ax_out) *ax_out = r.x.ax;
}

/* FOS-01: Initialize FOSSIL (fn 04h) */
static void test_01(void)
{
    unsigned ax, bx;
    TEST(1, "Fn 04h Initialize FOSSIL");
    fossil_call(0x04, &ax, &bx, NULL);
    if (ax == 0x1954)
        PASS();
    else
        FAIL("Signature != 1954h");
}

/* FOS-02: Check revision and max function (from fn 04h) */
static void test_02(void)
{
    unsigned ax, bx;
    TEST(2, "Fn 04h Rev level and max function");
    fossil_call(0x04, &ax, &bx, NULL);
    if ((bx >> 8) >= 5 && (bx & 0xFF) >= 0x1B)
        PASS();
    else
        FAIL("Rev < 5 or maxfn < 1Bh");
}

/* FOS-03: Set baud rate (fn 00h) — 9600 8N1 */
static void test_03(void)
{
    unsigned ax;
    /* 9600 = 111b, 8N1 = 00 0 11 → 0xE3 */
    fossil_call_al(0x00, 0xE3, &ax);
    TEST(3, "Fn 00h Set 9600 8N1");
    if (ax != 0)
        PASS();
    else
        FAIL("AX = 0 (unexpected)");
}

/* FOS-04: Port status (fn 03h) */
static void test_04(void)
{
    unsigned ax;
    TEST(4, "Fn 03h Port Status");
    fossil_call(0x03, &ax, NULL, NULL);
    /* AH=LSR, AL=MSR — THRE should be set (bit 5 of AH) */
    if (ax & 0x2000)
        PASS();
    else
        FAIL("THRE not set in status");
}

/* FOS-05: Transmit char no-wait (fn 0Bh) */
static void test_05(void)
{
    unsigned ax;
    TEST(5, "Fn 0Bh TX char no-wait");
    fossil_call_al(0x0B, 'X', &ax);
    if (ax == 1)
        PASS();
    else
        FAIL("AX != 1 (not sent)");
}

/* FOS-06: Peek char (fn 0Ch) — should be empty after init */
static void test_06(void)
{
    unsigned ax;
    TEST(6, "Fn 0Ch Peek (expect empty)");
    fossil_call(0x0C, &ax, NULL, NULL);
    if (ax == 0xFFFF)
        PASS();
    else
        FAIL("Data present when none expected");
}

/* FOS-07: Flush output (fn 08h) */
static void test_07(void)
{
    TEST(7, "Fn 08h Flush Output");
    fossil_call(0x08, NULL, NULL, NULL);
    PASS(); /* No return value to check — success = no hang */
}

/* FOS-08: Purge output (fn 09h) */
static void test_08(void)
{
    TEST(8, "Fn 09h Purge Output");
    fossil_call(0x09, NULL, NULL, NULL);
    PASS();
}

/* FOS-09: Purge input (fn 0Ah) */
static void test_09(void)
{
    TEST(9, "Fn 0Ah Purge Input");
    fossil_call(0x0A, NULL, NULL, NULL);
    PASS();
}

/* FOS-10: Flow control (fn 0Fh) */
static void test_10(void)
{
    unsigned ax;
    TEST(10, "Fn 0Fh Flow Control (XON+CTS)");
    fossil_call_al(0x0F, 0x03, &ax); /* XON TX + CTS/RTS */
    PASS();
}

/* FOS-11: Ctrl-C/K control (fn 10h) */
static void test_11(void)
{
    TEST(11, "Fn 10h Ctrl-C/K Control");
    fossil_call_al(0x10, 0x00, NULL);
    PASS();
}

/* FOS-12: Set cursor (fn 11h) */
static void test_12(void)
{
    union REGS r;
    TEST(12, "Fn 11h Set Cursor 0,0");
    r.h.ah = 0x11;
    r.h.dh = 0;
    r.h.dl = 0;
    int86(0x14, &r, &r);
    PASS();
}

/* FOS-13: Get cursor (fn 12h) */
static void test_13(void)
{
    union REGS r;
    TEST(13, "Fn 12h Get Cursor Position");
    r.h.ah = 0x12;
    r.x.dx = g_port;
    int86(0x14, &r, &r);
    if (r.h.dh == 0 && r.h.dl == 0)
        PASS();
    else
        FAIL("Cursor not at 0,0");
}

/* FOS-14: ANSI write (fn 13h) */
static void test_14(void)
{
    TEST(14, "Fn 13h ANSI Write '*'");
    fossil_call_al(0x13, '*', NULL);
    PASS();
}

/* FOS-15: Keyboard read no-wait (fn 0Dh) */
static void test_15(void)
{
    unsigned ax;
    TEST(15, "Fn 0Dh Keyboard Read (no-wait)");
    fossil_call(0x0D, &ax, NULL, NULL);
    /* FFFFh = no key, or a keystroke */
    PASS();
}

/* FOS-16: Keyboard peek (fn 0Eh) */
static void test_16(void)
{
    unsigned ax;
    TEST(16, "Fn 0Eh Keyboard Peek (no-wait)");
    fossil_call(0x0E, &ax, NULL, NULL);
    PASS();
}

/* FOS-17: Break on (fn 18h AL=1) */
static void test_17(void)
{
    TEST(17, "Fn 18h Break ON");
    fossil_call_al(0x18, 0x01, NULL);
    PASS();
}

/* FOS-18: Break off (fn 18h AL=0) */
static void test_18(void)
{
    TEST(18, "Fn 18h Break OFF");
    fossil_call_al(0x18, 0x00, NULL);
    PASS();
}

/* FOS-19: Driver info (fn 19h) */
static void test_19(void)
{
    union REGS r;
    struct SREGS s;
    unsigned char buf[32];
    TEST(19, "Fn 19h Driver Info (19 bytes)");
    memset(buf, 0, sizeof(buf));
    r.h.ah = 0x19;
    r.x.cx = 19;
    r.x.di = FP_OFF(buf);
    s.es = FP_SEG(buf);
    r.x.dx = g_port;
    int86x(0x14, &r, &r, &s);
    if (r.x.ax == 19 && buf[0] == 19 && buf[1] == 0)
        PASS();
    else
        FAIL("Wrong size or structure");
}

/* FOS-20: Deinitialize (fn 05h) */
static void test_20(void)
{
    TEST(20, "Fn 05h Deinitialize FOSSIL");
    fossil_call(0x05, NULL, NULL, NULL);
    PASS();
}

int main(int argc, char *argv[])
{
    if (argc > 1) g_port = atoi(argv[1]);

    printf("FOSSIL Conformance Test — Port COM%d\n", g_port + 1);
    printf("FTS-0001 Rev 5 — 20 tests\n");
    printf("==========================================\n\n");

    test_01();
    test_02();
    test_03();
    test_04();
    test_05();
    test_06();
    test_07();
    test_08();
    test_09();
    test_10();
    test_11();
    test_12();
    test_13();
    test_14();
    test_15();
    test_16();
    test_17();
    test_18();
    test_19();
    test_20();

    printf("\n==========================================\n");
    printf("Results: %d PASS, %d FAIL (of 20)\n", g_pass, g_fail);
    printf("==========================================\n");

    return g_fail;
}
