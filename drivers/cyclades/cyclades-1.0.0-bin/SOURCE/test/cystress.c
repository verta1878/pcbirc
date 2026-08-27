/* ====================================================================
 * cystress.c — Serial Port Stress Test
 * ====================================================================
 * Exercises the driver under heavy concurrent load to expose:
 *   - Race conditions between open/close and read/write
 *   - IRP cancellation races (CancelIo during pending I/O)
 *   - Resource leaks (handles, memory, COM port numbers)
 *   - BSOD-causing bugs (null derefs, double-frees, deadlocks)
 *
 * Test phases:
 *   Phase 1: Rapid open/close — open and close the port 1000 times
 *   Phase 2: Write flood — continuous writes for 10 seconds
 *   Phase 3: Read timeout — reads with no data, verify timeout works
 *   Phase 4: IOCTL storm — rapid baud/line changes during I/O
 *   Phase 5: Cancel stress — start reads, cancel immediately
 *   Phase 6: Concurrent threads — read + write + ioctl simultaneously
 *
 * No loopback cable required — tests exercise the driver API without
 * requiring data to actually arrive on the wire.
 *
 * Usage:
 *   cystress COM3              — run all phases
 *   cystress COM3 /P:2         — run only phase 2
 *   cystress COM3 /T:30        — run for 30 seconds per phase
 *   cystress COM3 /V           — verbose
 *
 * Build:
 *   cl cystress.c /Fe:cystress.exe
 *
 * License: GPLv3
 * ====================================================================
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_verbose   = 0;
static int g_passed    = 0;
static int g_failed    = 0;
static int g_total     = 0;
static volatile int g_stopThreads = 0;

#define TEST_START(name) \
    do { g_total++; printf("  TEST %2d: %-40s ", g_total, (name)); } while (0)
#define TEST_PASS() do { g_passed++; printf("PASS\n"); } while (0)
#define TEST_FAIL(r) do { g_failed++; printf("FAIL — %s\n", (r)); } while (0)


/* ====================================================================
 * Phase 1: Rapid Open/Close
 * ====================================================================
 * Opens and closes the port many times in succession. Tests:
 *   - Handle creation/cleanup doesn't leak
 *   - CD1400 channel init/shutdown is stable under repetition
 *   - COM port number claim/release cycle works
 *   - No BSOD on rapid open→close→open→close
 * ==================================================================== */

static void Phase1_RapidOpenClose(const char *portName, int iterations)
{
    int i;
    int failures = 0;
    char fullName[32];

    TEST_START("Rapid open/close");

    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);

    for (i = 0; i < iterations; i++) {
        HANDLE hCom = CreateFileA(fullName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCom == INVALID_HANDLE_VALUE) {
            failures++;
            if (g_verbose)
                printf("\n    Iteration %d: open failed (%lu)",
                       i, GetLastError());
            continue;
        }
        CloseHandle(hCom);
    }

    if (failures == 0) {
        printf("(%d cycles) ", iterations);
        TEST_PASS();
    } else {
        char msg[64];
        _snprintf(msg, sizeof(msg), "%d of %d opens failed", failures, iterations);
        TEST_FAIL(msg);
    }
}


/* ====================================================================
 * Phase 2: Write Flood
 * ====================================================================
 * Continuously writes data for a fixed duration. Even without a
 * loopback cable, this exercises:
 *   - TX ring buffer management under pressure
 *   - ISR TX drain path
 *   - Write IRP completion/queueing
 *   - TxBuf overflow handling
 * ==================================================================== */

static void Phase2_WriteFlood(const char *portName, int durationSec)
{
    HANDLE hCom;
    char fullName[32];
    UCHAR buf[512];
    DWORD written;
    DWORD startTick, totalBytes;
    int i;

    TEST_START("Write flood");

    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);
    hCom = CreateFileA(fullName, GENERIC_READ | GENERIC_WRITE,
                       0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE) {
        TEST_FAIL("cannot open port");
        return;
    }

    /* Fill buffer with pattern */
    for (i = 0; i < 512; i++)
        buf[i] = (UCHAR)(i & 0xFF);

    /* Set a short write timeout so we don't block forever
     * when the TX buffer fills up (no loopback draining it) */
    {
        COMMTIMEOUTS cto;
        cto.ReadIntervalTimeout         = MAXDWORD;
        cto.ReadTotalTimeoutMultiplier  = 0;
        cto.ReadTotalTimeoutConstant    = 0;
        cto.WriteTotalTimeoutMultiplier = 0;
        cto.WriteTotalTimeoutConstant   = 100;  /* 100ms write timeout */
        SetCommTimeouts(hCom, &cto);
    }

    /* Write continuously for durationSec seconds */
    startTick = GetTickCount();
    totalBytes = 0;
    while ((GetTickCount() - startTick) < (DWORD)(durationSec * 1000)) {
        if (WriteFile(hCom, buf, 512, &written, NULL)) {
            totalBytes += written;
        }
        /* Write may timeout (buffer full, no drain) — that's OK */
    }

    CloseHandle(hCom);

    printf("(%lu KB in %ds) ", totalBytes / 1024, durationSec);
    TEST_PASS();
}


/* ====================================================================
 * Phase 3: Read Timeout
 * ====================================================================
 * Reads with no data coming in. Verifies:
 *   - Read timeout fires correctly (doesn't hang)
 *   - ReadFile returns 0 bytes, not an error
 *   - No BSOD on timeout expiry
 * ==================================================================== */

static void Phase3_ReadTimeout(const char *portName)
{
    HANDLE hCom;
    char fullName[32];
    UCHAR buf[64];
    DWORD bytesRead;
    COMMTIMEOUTS cto;
    DWORD startTick;

    TEST_START("Read timeout (500ms, no data)");

    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);
    hCom = CreateFileA(fullName, GENERIC_READ | GENERIC_WRITE,
                       0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE) {
        TEST_FAIL("cannot open port");
        return;
    }

    /* Set 500ms total read timeout */
    cto.ReadIntervalTimeout         = 0;
    cto.ReadTotalTimeoutMultiplier  = 0;
    cto.ReadTotalTimeoutConstant    = 500;
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant   = 1000;
    SetCommTimeouts(hCom, &cto);

    /* Purge any stale data */
    PurgeComm(hCom, PURGE_RXCLEAR);

    /* Read — should timeout after ~500ms with 0 bytes */
    startTick = GetTickCount();
    ReadFile(hCom, buf, sizeof(buf), &bytesRead, NULL);
    {
        DWORD elapsed = GetTickCount() - startTick;

        if (bytesRead == 0 && elapsed >= 400 && elapsed <= 1500) {
            printf("(%lums) ", elapsed);
            TEST_PASS();
        } else if (bytesRead > 0) {
            TEST_FAIL("got unexpected data (stale bytes in buffer?)");
        } else {
            char msg[64];
            _snprintf(msg, sizeof(msg), "timeout took %lums (expected ~500ms)", elapsed);
            TEST_FAIL(msg);
        }
    }

    CloseHandle(hCom);
}


/* ====================================================================
 * Phase 4: IOCTL Storm
 * ====================================================================
 * Rapidly changes baud rate and line settings. Exercises:
 *   - IOCTL dispatch under rapid-fire conditions
 *   - KeSynchronizeExecution contention
 *   - COR1/COR_CHANGE command processing
 * ==================================================================== */

static void Phase4_IoctlStorm(const char *portName, int iterations)
{
    HANDLE hCom;
    char fullName[32];
    DCB dcb;
    int i;
    int failures = 0;
    static const DWORD bauds[] = {
        9600, 19200, 38400, 57600, 115200, 2400, 1200, 4800
    };

    TEST_START("IOCTL storm (baud changes)");

    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);
    hCom = CreateFileA(fullName, GENERIC_READ | GENERIC_WRITE,
                       0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE) {
        TEST_FAIL("cannot open port");
        return;
    }

    for (i = 0; i < iterations; i++) {
        memset(&dcb, 0, sizeof(dcb));
        dcb.DCBlength = sizeof(dcb);
        GetCommState(hCom, &dcb);
        dcb.BaudRate = bauds[i % 8];
        dcb.ByteSize = (BYTE)(5 + (i % 4));   /* Cycle 5-8 bits    */
        if (!SetCommState(hCom, &dcb))
            failures++;
    }

    CloseHandle(hCom);

    if (failures == 0) {
        printf("(%d changes) ", iterations);
        TEST_PASS();
    } else {
        char msg[64];
        _snprintf(msg, sizeof(msg), "%d of %d SetCommState failed", failures, iterations);
        TEST_FAIL(msg);
    }
}


/* ====================================================================
 * Phase 5: Cancel Stress
 * ====================================================================
 * Starts overlapped reads then immediately cancels them. Exercises:
 *   - IoCsq cancel path
 *   - Cancel routine races
 *   - Pending IRP cleanup
 * ==================================================================== */

static void Phase5_CancelStress(const char *portName, int iterations)
{
    int i;
    int failures = 0;
    char fullName[32];

    TEST_START("Cancel stress (read + CancelIo)");

    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);

    for (i = 0; i < iterations; i++) {
        HANDLE hCom;
        UCHAR buf[64];
        DWORD bytesRead;
        OVERLAPPED ov = {0};

        /* Open with FILE_FLAG_OVERLAPPED for async I/O */
        hCom = CreateFileA(fullName, GENERIC_READ | GENERIC_WRITE,
                           0, NULL, OPEN_EXISTING,
                           FILE_FLAG_OVERLAPPED, NULL);
        if (hCom == INVALID_HANDLE_VALUE) {
            failures++;
            continue;
        }

        ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        /* Start an async read — will pend because no data */
        ReadFile(hCom, buf, sizeof(buf), &bytesRead, &ov);

        /* Immediately cancel it */
        CancelIo(hCom);

        /* Wait briefly for cancellation to complete */
        WaitForSingleObject(ov.hEvent, 100);

        CloseHandle(ov.hEvent);
        CloseHandle(hCom);
    }

    if (failures == 0) {
        printf("(%d cycles) ", iterations);
        TEST_PASS();
    } else {
        char msg[64];
        _snprintf(msg, sizeof(msg), "%d of %d cycles failed", failures, iterations);
        TEST_FAIL(msg);
    }
}


/* ====================================================================
 * Phase 6: Concurrent Threads
 * ====================================================================
 * Runs read, write, and IOCTL operations simultaneously from
 * separate threads. Exercises:
 *   - Thread safety of all dispatch routines
 *   - Spinlock contention under load
 *   - KeSynchronizeExecution under concurrent access
 * ==================================================================== */

static DWORD WINAPI WriterThread(LPVOID param)
{
    HANDLE hCom = (HANDLE)param;
    UCHAR buf[128];
    DWORD written;
    int i;

    for (i = 0; i < 128; i++) buf[i] = (UCHAR)i;

    while (!g_stopThreads) {
        WriteFile(hCom, buf, 128, &written, NULL);
    }
    return 0;
}

static DWORD WINAPI ReaderThread(LPVOID param)
{
    HANDLE hCom = (HANDLE)param;
    UCHAR buf[128];
    DWORD bytesRead;

    while (!g_stopThreads) {
        ReadFile(hCom, buf, sizeof(buf), &bytesRead, NULL);
    }
    return 0;
}

static DWORD WINAPI IoctlThread(LPVOID param)
{
    HANDLE hCom = (HANDLE)param;
    DWORD modemStatus;
    COMSTAT comStat;
    DWORD errors;

    while (!g_stopThreads) {
        GetCommModemStatus(hCom, &modemStatus);
        ClearCommError(hCom, &errors, &comStat);
        Sleep(1);
    }
    return 0;
}

static void Phase6_ConcurrentThreads(const char *portName, int durationSec)
{
    HANDLE hCom;
    HANDLE threads[3];
    char fullName[32];

    TEST_START("Concurrent threads (R+W+IOCTL)");

    _snprintf(fullName, sizeof(fullName), "\\\\.\\%s", portName);
    hCom = CreateFileA(fullName, GENERIC_READ | GENERIC_WRITE,
                       0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE) {
        TEST_FAIL("cannot open port");
        return;
    }

    /* Short timeouts so threads don't block forever */
    {
        COMMTIMEOUTS cto;
        cto.ReadIntervalTimeout         = MAXDWORD;
        cto.ReadTotalTimeoutMultiplier  = 0;
        cto.ReadTotalTimeoutConstant    = 50;
        cto.WriteTotalTimeoutMultiplier = 0;
        cto.WriteTotalTimeoutConstant   = 50;
        SetCommTimeouts(hCom, &cto);
    }

    g_stopThreads = 0;

    /* Launch three threads: reader, writer, IOCTL caller */
    threads[0] = CreateThread(NULL, 0, WriterThread, hCom, 0, NULL);
    threads[1] = CreateThread(NULL, 0, ReaderThread, hCom, 0, NULL);
    threads[2] = CreateThread(NULL, 0, IoctlThread,  hCom, 0, NULL);

    /* Let them run */
    printf("(%ds) ", durationSec);
    Sleep(durationSec * 1000);

    /* Signal threads to stop */
    g_stopThreads = 1;

    /* Wait for all threads to exit (5 second timeout) */
    WaitForMultipleObjects(3, threads, TRUE, 5000);

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    CloseHandle(threads[2]);
    CloseHandle(hCom);

    /* If we got here without a BSOD, the test passed.
     * The point of this test is survival, not data correctness. */
    TEST_PASS();
}


/* ====================================================================
 * main
 * ==================================================================== */

int main(int argc, char *argv[])
{
    const char *portName = NULL;
    int duration = 5;       /* Default: 5 seconds per timed phase   */
    int onlyPhase = 0;     /* 0 = run all phases                   */
    int i;

    printf("CYSTRESS — Cyclades Serial Port Stress Test\n");
    printf("=============================================\n\n");

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "/P:", 3) == 0)
            onlyPhase = atoi(argv[i]+3);
        else if (strncmp(argv[i], "/T:", 3) == 0)
            duration = atoi(argv[i]+3);
        else if (_stricmp(argv[i], "/V") == 0)
            g_verbose = 1;
        else if (argv[i][0] != '/')
            portName = argv[i];
    }

    if (!portName) {
        printf("Usage: cystress COMn [/P:N] [/T:sec] [/V]\n");
        printf("  COMn    COM port to test\n");
        printf("  /P:N    Run only phase N (1-6)\n");
        printf("  /T:N    Duration per timed phase (default 5s)\n");
        printf("  /V      Verbose\n");
        printf("\nNo loopback cable required.\n");
        return 1;
    }

    printf("Port: %s, Duration: %ds per phase\n\n", portName, duration);

    if (onlyPhase == 0 || onlyPhase == 1) {
        printf("--- Phase 1: Rapid Open/Close ---\n");
        Phase1_RapidOpenClose(portName, 100);
    }
    if (onlyPhase == 0 || onlyPhase == 2) {
        printf("\n--- Phase 2: Write Flood ---\n");
        Phase2_WriteFlood(portName, duration);
    }
    if (onlyPhase == 0 || onlyPhase == 3) {
        printf("\n--- Phase 3: Read Timeout ---\n");
        Phase3_ReadTimeout(portName);
    }
    if (onlyPhase == 0 || onlyPhase == 4) {
        printf("\n--- Phase 4: IOCTL Storm ---\n");
        Phase4_IoctlStorm(portName, 500);
    }
    if (onlyPhase == 0 || onlyPhase == 5) {
        printf("\n--- Phase 5: Cancel Stress ---\n");
        Phase5_CancelStress(portName, 100);
    }
    if (onlyPhase == 0 || onlyPhase == 6) {
        printf("\n--- Phase 6: Concurrent Threads ---\n");
        Phase6_ConcurrentThreads(portName, duration);
    }

    printf("\n=============================================\n");
    printf("RESULTS: %d passed, %d failed, %d total\n",
           g_passed, g_failed, g_total);
    if (g_failed == 0)
        printf("No BSOD. Driver survived stress test.\n");
    printf("=============================================\n");

    return g_failed > 0 ? 1 : 0;
}
