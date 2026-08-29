/* ====================================================================
 * cyloopback.c — Serial Port Loopback Test
 * ====================================================================
 * Sends data out the TX pin and verifies it comes back on the RX
 * pin via a loopback cable (TX→RX, RTS→CTS, DTR→DSR).
 *
 * This tests the ENTIRE driver stack:
 *   - WriteFile → IRP_MJ_WRITE → TxBuf → ISR → TDR → wire
 *   - wire → RDSR → ISR → RxBuf → IRP_MJ_READ → ReadFile
 *   - Baud rate accuracy (both sides must match)
 *   - FIFO operation (12-byte CD1400 FIFO)
 *   - Ring buffer management (4096-byte software buffers)
 *   - Interrupt delivery and DPC completion
 *
 * Test patterns:
 *   1. Single byte — verify basic TX→RX works
 *   2. Counting pattern — 0x00 through 0xFF, check order
 *   3. Block transfer — 1KB blocks, verify content
 *   4. Baud rate sweep — test at all supported baud rates
 *   5. Modem signals — DTR→DSR, RTS→CTS (with loopback cable)
 *
 * Requirements:
 *   - Loopback cable: TX(pin 3) → RX(pin 2)
 *                     RTS(pin 7) → CTS(pin 8)
 *                     DTR(pin 4) → DSR(pin 6)
 *   - Or: null modem cable connecting two ports on the same card
 *
 * Usage:
 *   cyloopback COM3            — run all tests at 9600 baud
 *   cyloopback COM3 /B:115200  — run at 115200 baud
 *   cyloopback COM3 /A         — sweep all baud rates
 *   cyloopback COM3 /V         — verbose (show every byte)
 *
 * Build:
 *   cl cyloopback.c /Fe:cyloopback.exe
 *
 * License: GPLv3
 * ====================================================================
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ====================================================================
 * Test Infrastructure
 * ==================================================================== */

static int g_verbose  = 0;
static int g_passed   = 0;
static int g_failed   = 0;
static int g_total    = 0;

#define TEST_START(name) \
    do { \
        g_total++; \
        printf("  TEST %2d: %-40s ", g_total, (name)); \
    } while (0)

#define TEST_PASS() \
    do { g_passed++; printf("PASS\n"); } while (0)

#define TEST_FAIL(reason) \
    do { \
        g_failed++; \
        printf("FAIL — %s\n", (reason)); \
    } while (0)


/* ====================================================================
 * OpenPort — Open and configure a COM port
 * ==================================================================== */

static HANDLE OpenPort(const char *portName, DWORD baud)
{
    HANDLE hCom;
    DCB dcb;
    COMMTIMEOUTS cto;
    char fullName[32];

    /* Prepend \\.\\ for COM port numbers > 9.
     * CreateFile("COM3") works, but CreateFile("COM10") doesn't
     * unless you use the \\.\\ prefix. Always use it for safety. */
    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);

    hCom = CreateFileA(fullName,
                       GENERIC_READ | GENERIC_WRITE,
                       0,              /* Exclusive access             */
                       NULL,
                       OPEN_EXISTING,
                       0,              /* Synchronous I/O              */
                       NULL);

    if (hCom == INVALID_HANDLE_VALUE) {
        printf("ERROR: Cannot open %s (error %lu)\n",
               portName, GetLastError());
        printf("  Is the port installed? Check Device Manager.\n");
        return INVALID_HANDLE_VALUE;
    }

    /* Configure the port */
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hCom, &dcb);
    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary  = TRUE;
    /* Disable all flow control for loopback testing —
     * we want raw byte-for-byte transfer. */
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl  = DTR_CONTROL_ENABLE;
    dcb.fRtsControl  = RTS_CONTROL_ENABLE;
    dcb.fOutX        = FALSE;
    dcb.fInX         = FALSE;

    if (!SetCommState(hCom, &dcb)) {
        printf("ERROR: SetCommState failed (error %lu)\n",
               GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    /* Set timeouts: wait up to 2 seconds for data.
     * Loopback should be near-instant, but we allow slack for
     * slow baud rates and system load. */
    cto.ReadIntervalTimeout         = 100;
    cto.ReadTotalTimeoutMultiplier  = 10;
    cto.ReadTotalTimeoutConstant    = 2000;
    cto.WriteTotalTimeoutMultiplier = 10;
    cto.WriteTotalTimeoutConstant   = 2000;
    SetCommTimeouts(hCom, &cto);

    /* Purge any stale data in buffers */
    PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return hCom;
}


/* ====================================================================
 * Test 1: Single Byte Loopback
 * ==================================================================== */

static void TestSingleByte(HANDLE hCom)
{
    UCHAR txByte = 0xA5;       /* Alternating bit pattern           */
    UCHAR rxByte = 0;
    DWORD written, bytesRead;

    TEST_START("Single byte loopback (0xA5)");

    if (!WriteFile(hCom, &txByte, 1, &written, NULL) || written != 1) {
        TEST_FAIL("WriteFile failed");
        return;
    }

    if (!ReadFile(hCom, &rxByte, 1, &bytesRead, NULL) || bytesRead != 1) {
        TEST_FAIL("ReadFile timeout — no data received");
        return;
    }

    if (rxByte != txByte) {
        char msg[64];
        _snprintf(msg, sizeof(msg), "data mismatch: sent 0x%02X, got 0x%02X",
                txByte, rxByte);
        TEST_FAIL(msg);
        return;
    }

    TEST_PASS();
}


/* ====================================================================
 * Test 2: Counting Pattern (0x00 through 0xFF)
 * ==================================================================== */

static void TestCountingPattern(HANDLE hCom)
{
    UCHAR txBuf[256];
    UCHAR rxBuf[256];
    DWORD written, bytesRead;
    int i;

    TEST_START("Counting pattern (0x00-0xFF, 256 bytes)");

    /* Build the pattern */
    for (i = 0; i < 256; i++)
        txBuf[i] = (UCHAR)i;

    /* Send all 256 bytes */
    if (!WriteFile(hCom, txBuf, 256, &written, NULL) || written != 256) {
        char msg[64];
        _snprintf(msg, sizeof(msg), "WriteFile: only %lu of 256 bytes written", written);
        TEST_FAIL(msg);
        return;
    }

    /* Receive — may come in multiple reads due to FIFO chunking */
    bytesRead = 0;
    memset(rxBuf, 0xFF, sizeof(rxBuf));
    while (bytesRead < 256) {
        DWORD got = 0;
        if (!ReadFile(hCom, rxBuf + bytesRead, 256 - bytesRead,
                      &got, NULL) || got == 0) {
            char msg[64];
            _snprintf(msg, sizeof(msg), "ReadFile: only %lu of 256 bytes received",
                    bytesRead);
            TEST_FAIL(msg);
            return;
        }
        bytesRead += got;
    }

    /* Verify the pattern */
    for (i = 0; i < 256; i++) {
        if (rxBuf[i] != txBuf[i]) {
            char msg[80];
            _snprintf(msg, sizeof(msg), "mismatch at byte %d: sent 0x%02X, got 0x%02X",
                    i, txBuf[i], rxBuf[i]);
            TEST_FAIL(msg);
            return;
        }
    }

    TEST_PASS();
}


/* ====================================================================
 * Test 3: Block Transfer (1KB blocks)
 * ==================================================================== */

static void TestBlockTransfer(HANDLE hCom)
{
    UCHAR txBuf[1024];
    UCHAR rxBuf[1024];
    DWORD written, bytesRead;
    int i;

    TEST_START("Block transfer (1024 bytes, pseudo-random)");

    /* Generate pseudo-random data.
     * XOR-shift pattern ensures good bit coverage. */
    for (i = 0; i < 1024; i++)
        txBuf[i] = (UCHAR)((i * 7 + 0x5A) ^ (i >> 3));

    if (!WriteFile(hCom, txBuf, 1024, &written, NULL) || written != 1024) {
        char msg[64];
        _snprintf(msg, sizeof(msg), "WriteFile: only %lu of 1024 bytes", written);
        TEST_FAIL(msg);
        return;
    }

    /* Read back — may require multiple reads */
    bytesRead = 0;
    while (bytesRead < 1024) {
        DWORD got = 0;
        if (!ReadFile(hCom, rxBuf + bytesRead, 1024 - bytesRead,
                      &got, NULL) || got == 0) {
            char msg[64];
            _snprintf(msg, sizeof(msg), "ReadFile: only %lu of 1024 bytes", bytesRead);
            TEST_FAIL(msg);
            return;
        }
        bytesRead += got;
    }

    /* Verify */
    for (i = 0; i < 1024; i++) {
        if (rxBuf[i] != txBuf[i]) {
            char msg[80];
            _snprintf(msg, sizeof(msg), "mismatch at byte %d: sent 0x%02X, got 0x%02X",
                    i, txBuf[i], rxBuf[i]);
            TEST_FAIL(msg);
            return;
        }
    }

    TEST_PASS();
}


/* ====================================================================
 * Test 4: Modem Signals (DTR→DSR, RTS→CTS via loopback cable)
 * ==================================================================== */

static void TestModemSignals(HANDLE hCom)
{
    DWORD modemStatus;

    /* ---- DTR → DSR ---- */
    TEST_START("Modem: DTR high → DSR high");

    EscapeCommFunction(hCom, SETDTR);
    Sleep(50);      /* Allow signal to propagate                    */
    GetCommModemStatus(hCom, &modemStatus);

    if (modemStatus & MS_DSR_ON) {
        TEST_PASS();
    } else {
        TEST_FAIL("DSR not asserted (check loopback cable DTR→DSR)");
    }

    TEST_START("Modem: DTR low → DSR low");

    EscapeCommFunction(hCom, CLRDTR);
    Sleep(50);
    GetCommModemStatus(hCom, &modemStatus);

    if (!(modemStatus & MS_DSR_ON)) {
        TEST_PASS();
    } else {
        TEST_FAIL("DSR still asserted after clearing DTR");
    }

    /* ---- RTS → CTS ---- */
    TEST_START("Modem: RTS high → CTS high");

    EscapeCommFunction(hCom, SETRTS);
    Sleep(50);
    GetCommModemStatus(hCom, &modemStatus);

    if (modemStatus & MS_CTS_ON) {
        TEST_PASS();
    } else {
        TEST_FAIL("CTS not asserted (check loopback cable RTS→CTS)");
    }

    TEST_START("Modem: RTS low → CTS low");

    EscapeCommFunction(hCom, CLRRTS);
    Sleep(50);
    GetCommModemStatus(hCom, &modemStatus);

    if (!(modemStatus & MS_CTS_ON)) {
        TEST_PASS();
    } else {
        TEST_FAIL("CTS still asserted after clearing RTS");
    }

    /* Restore DTR + RTS for subsequent tests */
    EscapeCommFunction(hCom, SETDTR);
    EscapeCommFunction(hCom, SETRTS);
}


/* ====================================================================
 * Test 5: IOCTL Coverage — verify key IOCTLs work
 * ==================================================================== */

static void TestIoctls(HANDLE hCom)
{
    DCB dcb;
    COMMPROP props;
    COMSTAT comStat;
    DWORD errors;

    /* ---- GET_PROPERTIES ---- */
    TEST_START("IOCTL: GetCommProperties");

    memset(&props, 0, sizeof(props));
    if (GetCommProperties(hCom, &props)) {
        if (g_verbose) {
            printf("\n    MaxBaud=%lu MaxTxQueue=%lu MaxRxQueue=%lu\n",
                   props.dwMaxBaud, props.dwMaxTxQueue, props.dwMaxRxQueue);
        }
        TEST_PASS();
    } else {
        TEST_FAIL("GetCommProperties failed");
    }

    /* ---- SET/GET line control ---- */
    TEST_START("IOCTL: Set 7E1, read back");

    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hCom, &dcb);
    dcb.ByteSize = 7;
    dcb.Parity   = EVENPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hCom, &dcb);

    /* Read back and verify */
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hCom, &dcb);
    if (dcb.ByteSize == 7 && dcb.Parity == EVENPARITY) {
        TEST_PASS();
    } else {
        char msg[64];
        _snprintf(msg, sizeof(msg), "readback: ByteSize=%u Parity=%u (expected 7, 2)",
                dcb.ByteSize, dcb.Parity);
        TEST_FAIL(msg);
    }

    /* Restore 8N1 */
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hCom, &dcb);

    /* ---- PURGE ---- */
    TEST_START("IOCTL: PurgeComm (all flags)");

    if (PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT |
                        PURGE_TXCLEAR | PURGE_RXCLEAR)) {
        TEST_PASS();
    } else {
        TEST_FAIL("PurgeComm failed");
    }

    /* ---- COMMSTATUS ---- */
    TEST_START("IOCTL: ClearCommError (GET_COMMSTATUS)");

    if (ClearCommError(hCom, &errors, &comStat)) {
        if (g_verbose) {
            printf("\n    InQueue=%lu OutQueue=%lu Errors=0x%lX\n",
                   comStat.cbInQue, comStat.cbOutQue, errors);
        }
        TEST_PASS();
    } else {
        TEST_FAIL("ClearCommError failed");
    }
}


/* ====================================================================
 * Test 6: Baud Rate Sweep
 * ==================================================================== */

static void TestBaudSweep(HANDLE hCom)
{
    static const DWORD rates[] = {
        300, 1200, 2400, 4800, 9600, 19200, 38400,
        57600, 115200, 0
    };
    int i;
    UCHAR txByte, rxByte;
    DWORD written, bytesRead;

    for (i = 0; rates[i] != 0; i++) {
        DCB dcb;
        char name[48];

        _snprintf(name, sizeof(name), "Baud sweep: %lu baud loopback", rates[i]);
        TEST_START(name);

        /* Change baud rate */
        memset(&dcb, 0, sizeof(dcb));
        dcb.DCBlength = sizeof(dcb);
        GetCommState(hCom, &dcb);
        dcb.BaudRate = rates[i];
        if (!SetCommState(hCom, &dcb)) {
            TEST_FAIL("SetCommState failed for this baud rate");
            continue;
        }

        /* Purge any leftover data from previous baud */
        PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
        Sleep(10);

        /* Send and receive one byte */
        txByte = (UCHAR)(rates[i] & 0xFF);
        rxByte = 0;

        if (!WriteFile(hCom, &txByte, 1, &written, NULL) || written != 1) {
            TEST_FAIL("WriteFile failed");
            continue;
        }

        if (!ReadFile(hCom, &rxByte, 1, &bytesRead, NULL) || bytesRead != 1) {
            TEST_FAIL("ReadFile timeout");
            continue;
        }

        if (rxByte != txByte) {
            char msg[64];
            _snprintf(msg, sizeof(msg), "data mismatch: sent 0x%02X, got 0x%02X",
                    txByte, rxByte);
            TEST_FAIL(msg);
            continue;
        }

        TEST_PASS();
    }
}


/* ====================================================================
 * main
 * ==================================================================== */

int main(int argc, char *argv[])
{
    HANDLE hCom;
    DWORD baud = 9600;
    int sweepAll = 0;
    const char *portName = NULL;
    int i;

    printf("CYLOOPBACK — Cyclades Serial Port Loopback Test\n");
    printf("================================================\n\n");

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "/B:", 3) == 0)
            baud = strtoul(argv[i]+3, NULL, 10);
        else if (_stricmp(argv[i], "/A") == 0)
            sweepAll = 1;
        else if (_stricmp(argv[i], "/V") == 0)
            g_verbose = 1;
        else if (argv[i][0] != '/')
            portName = argv[i];
    }

    if (!portName) {
        printf("Usage: cyloopback COMn [/B:baud] [/A] [/V]\n");
        printf("  COMn    COM port to test (e.g., COM3)\n");
        printf("  /B:N    Baud rate (default 9600)\n");
        printf("  /A      Sweep all standard baud rates\n");
        printf("  /V      Verbose output\n");
        printf("\nRequires loopback cable: TX→RX, RTS→CTS, DTR→DSR\n");
        return 1;
    }

    /* Open the port */
    printf("Opening %s at %lu baud...\n\n", portName, baud);
    hCom = OpenPort(portName, baud);
    if (hCom == INVALID_HANDLE_VALUE)
        return 1;

    /* Run tests */
    printf("--- Data Tests ---\n");
    TestSingleByte(hCom);
    TestCountingPattern(hCom);
    TestBlockTransfer(hCom);

    printf("\n--- Modem Signal Tests ---\n");
    TestModemSignals(hCom);

    printf("\n--- IOCTL Tests ---\n");
    TestIoctls(hCom);

    if (sweepAll) {
        printf("\n--- Baud Rate Sweep ---\n");
        TestBaudSweep(hCom);
    }

    /* Summary */
    printf("\n================================================\n");
    printf("RESULTS: %d passed, %d failed, %d total\n",
           g_passed, g_failed, g_total);
    printf("================================================\n");

    CloseHandle(hCom);
    return g_failed > 0 ? 1 : 0;
}
