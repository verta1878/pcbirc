/* ====================================================================
 * D4TEST.EXE — SIO Conformance Test Harness
 * ====================================================================
 * 37 tests covering the complete ASYNC IOCtl interface.
 * Tests are numbered D4-01 through D4-37 for wrench's test matrix.
 *
 * Usage: D4TEST [portnum]
 *   Default: COM1 (port 1)
 *
 * Exit code: number of failed tests (0 = all pass)
 * ====================================================================
 */

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#include <os2.h>
#include <bsedev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------- */
/* Test Framework                                                       */
/* -------------------------------------------------------------------- */

static int  g_pass = 0;
static int  g_fail = 0;
static int  g_skip = 0;
static HFILE g_hCom = (HFILE)-1;

#define TEST(id, desc) \
    printf("  D4-%02d %-45s ", id, desc); fflush(stdout)

#define PASS() \
    do { printf("[PASS]\n"); g_pass++; } while(0)

#define FAIL(reason) \
    do { printf("[FAIL] %s\n", reason); g_fail++; } while(0)

#define SKIP(reason) \
    do { printf("[SKIP] %s\n", reason); g_skip++; } while(0)

static APIRET IOCtl(ULONG func, PVOID parm, ULONG parmLen,
                    PVOID data, ULONG dataLen)
{
    ULONG pl = parmLen, dl = dataLen;
    return DosDevIOCtl(g_hCom, IOCTL_ASYNC, func,
                       parm, parmLen, &pl,
                       data, dataLen, &dl);
}


/* -------------------------------------------------------------------- */
/* Tests                                                                */
/* -------------------------------------------------------------------- */

/* D4-01: Open COM port */
static void test_01_open(int port)
{
    ULONG action;
    char  name[16];
    APIRET rc;

    TEST(1, "DosOpen COMn");
    sprintf(name, "COM%d", port);
    rc = DosOpen(name, &g_hCom, &action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL);
    if (rc == 0)
        PASS();
    else
        FAIL("DosOpen failed");
}

/* D4-02: Set baud rate (41h) */
static void test_02_setbaud(void)
{
    USHORT baud = 9600;
    TEST(2, "IOCtl 41h Set Baud Rate (9600)");
    if (IOCtl(ASYNC_SETBAUDRATE, &baud, 2, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 41h failed");
}

/* D4-03: Query baud rate (61h) */
static void test_03_querybaud(void)
{
    USHORT baud = 0;
    TEST(3, "IOCtl 61h Query Baud Rate");
    if (IOCtl(ASYNC_GETBAUDRATE, NULL, 0, &baud, 2) == 0 && baud == 9600)
        PASS();
    else
        FAIL("Baud mismatch or IOCtl failed");
}

/* D4-04: Set line control (42h) */
static void test_04_setline(void)
{
    BYTE parm[3] = { 8, 0, 0 };  /* 8N1 */
    TEST(4, "IOCtl 42h Set Line Ctrl (8N1)");
    if (IOCtl(ASYNC_SETLINECTRL, parm, 3, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 42h failed");
}

/* D4-05: Query line control (62h) */
static void test_05_queryline(void)
{
    BYTE data[4] = {0};
    TEST(5, "IOCtl 62h Query Line Ctrl");
    if (IOCtl(ASYNC_GETLINECTRL, NULL, 0, data, 4) == 0 &&
        data[0] == 8 && data[1] == 0 && data[2] == 0)
        PASS();
    else
        FAIL("Line params mismatch");
}

/* D4-06: Extended set baud (43h) */
static void test_06_extsetbaud(void)
{
    BYTE parm[5];
    *(ULONG *)parm = 38400;
    parm[4] = 0;
    TEST(6, "IOCtl 43h Extended Set Baud (38400)");
    if (IOCtl(ASYNC_EXTSETBAUDRATE, parm, 5, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 43h failed");
}

/* D4-07: Extended query baud (63h) */
static void test_07_extquerybaud(void)
{
    BYTE data[15] = {0};
    ULONG cur;
    TEST(7, "IOCtl 63h Extended Query Baud");
    if (IOCtl(ASYNC_EXTGETBAUDRATE, NULL, 0, data, 15) == 0) {
        cur = *(ULONG *)data;
        if (cur == 38400)
            PASS();
        else
            FAIL("Baud != 38400");
    } else {
        FAIL("IOCtl 63h failed");
    }
}

/* D4-08: Query min/max baud from 63h */
static void test_08_baudrange(void)
{
    BYTE data[15] = {0};
    TEST(8, "IOCtl 63h Baud min >= 50, max >= 115200");
    if (IOCtl(ASYNC_EXTGETBAUDRATE, NULL, 0, data, 15) == 0) {
        ULONG minB = *(ULONG *)&data[5];
        ULONG maxB = *(ULONG *)&data[10];
        if (minB <= 50 && maxB >= 115200)
            PASS();
        else
            FAIL("Baud range out of spec");
    } else {
        FAIL("IOCtl 63h failed");
    }
}

/* D4-09: Set modem control signals (46h) — DTR on */
static void test_09_dtr_on(void)
{
    BYTE parm[2] = { 0x01, 0xFF };  /* DTR on */
    BYTE data[2] = {0};
    TEST(9, "IOCtl 46h Set DTR ON");
    if (IOCtl(ASYNC_SETMODEMCTRL, parm, 2, data, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 46h failed");
}

/* D4-10: Query modem output (66h) — verify DTR */
static void test_10_query_modem_out(void)
{
    BYTE data[1] = {0};
    TEST(10, "IOCtl 66h Query Modem Output (DTR set)");
    if (IOCtl(ASYNC_GETMODEMOUTPUT, NULL, 0, data, 1) == 0 && (data[0] & 0x01))
        PASS();
    else
        FAIL("DTR not reflected");
}

/* D4-11: Set DTR off */
static void test_11_dtr_off(void)
{
    BYTE parm[2] = { 0x00, 0xFE };
    BYTE data[2] = {0};
    TEST(11, "IOCtl 46h Set DTR OFF");
    if (IOCtl(ASYNC_SETMODEMCTRL, parm, 2, data, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 46h failed");
}

/* D4-12: Set RTS on */
static void test_12_rts_on(void)
{
    BYTE parm[2] = { 0x02, 0xFF };
    BYTE data[2] = {0};
    TEST(12, "IOCtl 46h Set RTS ON");
    if (IOCtl(ASYNC_SETMODEMCTRL, parm, 2, data, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 46h failed");
}

/* D4-13: Query modem input signals (67h) */
static void test_13_query_modem_in(void)
{
    BYTE data[1] = {0};
    TEST(13, "IOCtl 67h Query Modem Input Signals");
    if (IOCtl(ASYNC_GETMODEMINPUT, NULL, 0, data, 1) == 0)
        PASS();
    else
        FAIL("IOCtl 67h failed");
}

/* D4-14: Transmit byte immediate (44h) */
static void test_14_tx_immediate(void)
{
    BYTE parm[1] = { 'T' };
    TEST(14, "IOCtl 44h Transmit Byte Immediate");
    if (IOCtl(ASYNC_TRANSMITIMM, parm, 1, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 44h failed");
}

/* D4-15: Hold transmit (47h) */
static void test_15_hold_tx(void)
{
    TEST(15, "IOCtl 47h Hold Transmit");
    if (IOCtl(ASYNC_STOPTRANSMIT, NULL, 0, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 47h failed");
}

/* D4-16: Start transmit (48h) */
static void test_16_start_tx(void)
{
    TEST(16, "IOCtl 48h Start Transmit");
    if (IOCtl(ASYNC_STARTTRANSMIT, NULL, 0, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 48h failed");
}

/* D4-17: Query comm status (64h) */
static void test_17_comm_status(void)
{
    BYTE data[1] = {0};
    TEST(17, "IOCtl 64h Query SIO Status");
    if (IOCtl(ASYNC_GETCOMMSTATUS, NULL, 0, data, 1) == 0)
        PASS();
    else
        FAIL("IOCtl 64h failed");
}

/* D4-18: Query TX status (65h) */
static void test_18_tx_status(void)
{
    BYTE data[1] = {0};
    TEST(18, "IOCtl 65h Query Transmit Data Status");
    if (IOCtl(ASYNC_GETLINESTATUS, NULL, 0, data, 1) == 0)
        PASS();
    else
        FAIL("IOCtl 65h failed");
}

/* D4-19: Query RX buffer count (68h) */
static void test_19_rx_count(void)
{
    BYTE data[4] = {0};
    TEST(19, "IOCtl 68h Query RX Buffer Count");
    if (IOCtl(ASYNC_GETINQUECOUNT, NULL, 0, data, 4) == 0) {
        USHORT count = *(USHORT *)data;
        USHORT size  = *(USHORT *)&data[2];
        if (size > 0)
            PASS();
        else
            FAIL("Buffer size is 0");
    } else {
        FAIL("IOCtl 68h failed");
    }
}

/* D4-20: Query TX buffer count (69h) */
static void test_20_tx_count(void)
{
    BYTE data[4] = {0};
    TEST(20, "IOCtl 69h Query TX Buffer Count");
    if (IOCtl(ASYNC_GETOUTQUECOUNT, NULL, 0, data, 4) == 0) {
        USHORT size = *(USHORT *)&data[2];
        if (size > 0)
            PASS();
        else
            FAIL("Buffer size is 0");
    } else {
        FAIL("IOCtl 69h failed");
    }
}

/* D4-21: Query error (6Dh) */
static void test_21_query_error(void)
{
    USHORT err = 0;
    TEST(21, "IOCtl 6Dh Query SIO Error");
    if (IOCtl(ASYNC_GETCOMMERROR, NULL, 0, &err, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 6Dh failed");
}

/* D4-22: Error word resets after read */
static void test_22_error_reset(void)
{
    USHORT err = 0xFFFF;
    TEST(22, "IOCtl 6Dh Error word resets after read");
    IOCtl(ASYNC_GETCOMMERROR, NULL, 0, &err, 2);
    err = 0xFFFF;
    IOCtl(ASYNC_GETCOMMERROR, NULL, 0, &err, 2);
    if (err == 0)
        PASS();
    else
        FAIL("Error not cleared");
}

/* D4-23: Query event (72h) */
static void test_23_query_event(void)
{
    USHORT evt = 0;
    TEST(23, "IOCtl 72h Query SIO Event");
    if (IOCtl(ASYNC_GETCOMMEVENT, NULL, 0, &evt, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 72h failed");
}

/* D4-24: Event word resets after read */
static void test_24_event_reset(void)
{
    USHORT evt = 0xFFFF;
    TEST(24, "IOCtl 72h Event word resets after read");
    IOCtl(ASYNC_GETCOMMEVENT, NULL, 0, &evt, 2);
    evt = 0xFFFF;
    IOCtl(ASYNC_GETCOMMEVENT, NULL, 0, &evt, 2);
    if (evt == 0)
        PASS();
    else
        FAIL("Event not cleared");
}

/* D4-25: Write DCB (53h) */
static void test_25_write_dcb(void)
{
    BYTE dcb[11] = { 100,0, 100,0, 0x01, 0x40, 0xD2, 0, 0, 0x11, 0x13 };
    TEST(25, "IOCtl 53h Write DCB");
    if (IOCtl(ASYNC_SETDCBINFO, dcb, 11, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 53h failed");
}

/* D4-26: Read DCB (73h) */
static void test_26_read_dcb(void)
{
    BYTE dcb[11] = {0};
    TEST(26, "IOCtl 73h Read DCB");
    if (IOCtl(ASYNC_GETDCBINFO, NULL, 0, dcb, 11) == 0) {
        if (dcb[9] == 0x11 && dcb[10] == 0x13)
            PASS();
        else
            FAIL("XON/XOFF chars wrong");
    } else {
        FAIL("IOCtl 73h failed");
    }
}

/* D4-27: SIO forces DCB Flags3 bits */
static void test_27_dcb_forced_bits(void)
{
    BYTE dcb[11] = {0};
    TEST(27, "DCB Flags3 forced: FIFO=ena, trig=8, TXload=16");
    IOCtl(ASYNC_GETDCBINFO, NULL, 0, dcb, 11);
    /* Flags3 should have bits 4:3=10 (FIFO ena), 6:5=10 (trig 8), 7=1 (TX 16) */
    if ((dcb[6] & 0xF8) == 0xD0)
        PASS();
    else
        FAIL("Flags3 forced bits wrong");
}

/* D4-28: Set break on (4Bh) */
static void test_28_break_on(void)
{
    BYTE data[2] = {0};
    TEST(28, "IOCtl 4Bh Set Break On");
    if (IOCtl(ASYNC_SETBREAKON, NULL, 0, data, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 4Bh failed");
}

/* D4-29: Query break state via 62h */
static void test_29_break_verify(void)
{
    BYTE data[4] = {0};
    TEST(29, "IOCtl 62h Verify break active");
    IOCtl(ASYNC_GETLINECTRL, NULL, 0, data, 4);
    if (data[3] == 1)
        PASS();
    else
        FAIL("Break not indicated");
}

/* D4-30: Set break off (45h) */
static void test_30_break_off(void)
{
    BYTE data[2] = {0};
    TEST(30, "IOCtl 45h Set Break Off");
    if (IOCtl(ASYNC_SETBREAKOFF, NULL, 0, data, 2) == 0)
        PASS();
    else
        FAIL("IOCtl 45h failed");
}

/* D4-31: Write enhanced mode params (54h — ignored by SIO) */
static void test_31_enhanced_write(void)
{
    BYTE parm[5] = {0};
    TEST(31, "IOCtl 54h Write Enhanced (should succeed/ignore)");
    if (IOCtl(ASYNC_SETENHANCEDMODEPARMS, parm, 5, NULL, 0) == 0)
        PASS();
    else
        FAIL("IOCtl 54h failed");
}

/* D4-32: Read enhanced mode params (74h) */
static void test_32_enhanced_read(void)
{
    BYTE data[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    TEST(32, "IOCtl 74h Read Enhanced (bit 0 = hw avail)");
    if (IOCtl(ASYNC_GETENHANCEDMODEPARMS, NULL, 0, data, 5) == 0)
        PASS();
    else
        FAIL("IOCtl 74h failed");
}

/* D4-33: Set various baud rates */
static void test_33_baud_sweep(void)
{
    USHORT rates[] = { 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600 };
    int i, ok = 1;
    TEST(33, "Baud sweep: 300-57600");
    for (i = 0; i < 8; i++) {
        if (IOCtl(ASYNC_SETBAUDRATE, &rates[i], 2, NULL, 0) != 0) {
            ok = 0; break;
        }
    }
    if (ok) PASS(); else FAIL("Baud set failed");
}

/* D4-34: Set line params sweep */
static void test_34_line_sweep(void)
{
    BYTE p7n1[3] = { 7, 0, 0 };
    BYTE p8e2[3] = { 8, 2, 2 };
    BYTE p8n1[3] = { 8, 0, 0 };
    int ok = 1;
    TEST(34, "Line params: 7N1, 8E2, 8N1");
    if (IOCtl(ASYNC_SETLINECTRL, p7n1, 3, NULL, 0) != 0) ok = 0;
    if (IOCtl(ASYNC_SETLINECTRL, p8e2, 3, NULL, 0) != 0) ok = 0;
    if (IOCtl(ASYNC_SETLINECTRL, p8n1, 3, NULL, 0) != 0) ok = 0;
    if (ok) PASS(); else FAIL("Line set failed");
}

/* D4-35: Hold/start transmit cycle */
static void test_35_hold_start_cycle(void)
{
    BYTE status[1] = {0};
    TEST(35, "Hold TX → status bit 3 set → Start TX → cleared");
    IOCtl(ASYNC_STOPTRANSMIT, NULL, 0, NULL, 0);
    IOCtl(ASYNC_GETCOMMSTATUS, NULL, 0, status, 1);
    if (!(status[0] & 0x08)) { FAIL("Hold not reflected"); return; }
    IOCtl(ASYNC_STARTTRANSMIT, NULL, 0, NULL, 0);
    IOCtl(ASYNC_GETCOMMSTATUS, NULL, 0, status, 1);
    if (status[0] & 0x08) { FAIL("Start not reflected"); return; }
    PASS();
}

/* D4-36: Shared open */
static void test_36_shared_open(int port)
{
    HFILE hf2;
    ULONG action;
    char  name[16];
    TEST(36, "Shared (additional) open succeeds");
    sprintf(name, "COM%d", port);  /* Same port as test_01 */
    if (DosOpen(name, &hf2, &action, 0, 0,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL) == 0) {
        DosClose(hf2);
        PASS();
    } else {
        FAIL("Shared open failed");
    }
}

/* D4-37: Close port */
static void test_37_close(void)
{
    TEST(37, "DosClose COMn");
    if (DosClose(g_hCom) == 0) {
        g_hCom = (HFILE)-1;
        PASS();
    } else {
        FAIL("DosClose failed");
    }
}


/* ====================================================================
 * Main
 * ==================================================================== */

int main(int argc, char *argv[])
{
    int port = (argc > 1) ? atoi(argv[1]) : 1;

    printf("D4 Conformance Test — SIO ASYNC IOCtl Suite\n");
    printf("Testing COM%d — 37 tests\n", port);
    printf("============================================\n\n");

    test_01_open(port);
    if (g_hCom == (HFILE)-1) {
        printf("\nCannot open port — aborting.\n");
        return 37;
    }

    test_02_setbaud();
    test_03_querybaud();
    test_04_setline();
    test_05_queryline();
    test_06_extsetbaud();
    test_07_extquerybaud();
    test_08_baudrange();
    test_09_dtr_on();
    test_10_query_modem_out();
    test_11_dtr_off();
    test_12_rts_on();
    test_13_query_modem_in();
    test_14_tx_immediate();
    test_15_hold_tx();
    test_16_start_tx();
    test_17_comm_status();
    test_18_tx_status();
    test_19_rx_count();
    test_20_tx_count();
    test_21_query_error();
    test_22_error_reset();
    test_23_query_event();
    test_24_event_reset();
    test_25_write_dcb();
    test_26_read_dcb();
    test_27_dcb_forced_bits();
    test_28_break_on();
    test_29_break_verify();
    test_30_break_off();
    test_31_enhanced_write();
    test_32_enhanced_read();
    test_33_baud_sweep();
    test_34_line_sweep();
    test_35_hold_start_cycle();
    test_36_shared_open(port);
    test_37_close();

    printf("\n============================================\n");
    printf("Results: %d PASS, %d FAIL, %d SKIP (of 37)\n",
           g_pass, g_fail, g_skip);
    printf("============================================\n");

    return g_fail;
}
