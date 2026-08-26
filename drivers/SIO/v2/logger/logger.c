/* ====================================================================
 * logger.c — LOGGER.EXE Port Activity Logger for OS/2
 * ====================================================================
 * Monitors and logs all activity on a COM port via DosDevIOCtl.
 * Displays real-time hex/ASCII dump of TX/RX data, modem line
 * changes, errors, and port open/close events.
 *
 * Usage:
 *   LOGGER DEV <TraceFileName>
 *
 *   DEV is a Device Name or a comm port number (1 relative).
 *   Examples:
 *     LOGGER 1 LOGCOM1.LOG          Monitor COM1, log to file
 *     LOGGER COM1 LOGCOM1.LOG       Same (accepts COMn too)
 *     LOGGER \dev\$sio$ LOG.DAT     Direct SIO device access
 *
 * Note: Do NOT use names like COM1.LOG — use LOGCOM1.LOG
 *       to avoid confusing OS/2's device name resolution.
 *
 * Output format:
 *   HH:MM:SS.mmm  DIR  DATA
 *   14:23:01.456  TX>  41 54 5A 0D                    ATZ.
 *   14:23:01.512  <RX  4F 4B 0D 0A                    OK..
 *   14:23:01.600  MSR  DCD=1 DSR=1 CTS=1 RI=0
 *   14:23:02.100  LSR  OE=0 PE=0 FE=0 BI=0
 *
 * Designed to be re-entrant — multiple instances can run on
 * different ports simultaneously.
 *
 * Clean-room reimplementation from LOGGER.TXT documentation.
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VERSION     "2.0.0"
#define POLL_MS     10          /* Poll interval (milliseconds) */
#define BUF_SIZE    512         /* Read buffer size */

/* SIO2K IOCtl categories */
#define IOCTL_ASYNC     0x01    /* Standard async IOCtl */
#define IOCTL_SIO2K     0x80    /* SIO2K private IOCtl */

/* IOCTL_ASYNC functions */
#define ASYNC_GETLINESTATUS     0x0065  /* Get line status (LSR) */
#define ASYNC_GETMODEMSTATUS    0x0067  /* Get modem output signals */
#define ASYNC_GETMODEMINPUT     0x0068  /* Get modem input signals */
#define ASYNC_GETLINECTRL       0x0062  /* Get line characteristics */
#define ASYNC_GETDCBINFO        0x0073  /* Get DCB info */
#define ASYNC_GETCOMSTATUS      0x0064  /* Get COM status */
#define ASYNC_GETINQUECOUNT     0x0068  /* Get input queue count */
#define ASYNC_GETOUTQUECOUNT    0x0069  /* Get output queue count */

/* Modem status bits */
#define MS_CTS      0x10
#define MS_DSR      0x20
#define MS_RI       0x40
#define MS_DCD      0x80
#define MS_DCTS     0x01
#define MS_DDSR     0x02
#define MS_TERI     0x04
#define MS_DDCD     0x08

/* Line status bits */
#define LS_RXRDY    0x01
#define LS_OVERRUN  0x02
#define LS_PARITY   0x04
#define LS_FRAMING  0x08
#define LS_BREAK    0x10
#define LS_THRE     0x20
#define LS_TSRE     0x40

static HFILE   g_hCom = 0;
static FILE   *g_logFile = NULL;
static int     g_running = 1;
static unsigned char g_lastMSR = 0;
static unsigned char g_lastLSR = 0;


/* ---- Timestamp ---- */

static void get_timestamp(char *buf, int bufsize)
{
    DATETIME dt;
    DosGetDateTime(&dt);
    snprintf(buf, bufsize, "%02d:%02d:%02d.%03d",
             dt.hours, dt.minutes, dt.seconds, dt.hundredths * 10);
}


/* ---- Output (screen + optional file) ---- */

static void logprintf(const char *fmt, ...)
{
    va_list ap;
    char buf[1024];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    fputs(buf, stdout);
    fflush(stdout);

    if (g_logFile) {
        fputs(buf, g_logFile);
        fflush(g_logFile);
    }
}


/* ---- Hex + ASCII dump ---- */

static void log_data(const char *dir, const unsigned char *data, int len)
{
    char ts[20];
    int i, j, offset;

    get_timestamp(ts, sizeof(ts));

    offset = 0;
    while (offset < len) {
        int chunk = len - offset;
        if (chunk > 16) chunk = 16;

        logprintf("%s  %s  ", ts, dir);

        /* Hex */
        for (i = 0; i < chunk; i++)
            logprintf("%02X ", data[offset + i]);
        for (i = chunk; i < 16; i++)
            logprintf("   ");

        logprintf("  ");

        /* ASCII */
        for (i = 0; i < chunk; i++) {
            unsigned char ch = data[offset + i];
            logprintf("%c", (ch >= 0x20 && ch < 0x7F) ? ch : '.');
        }

        logprintf("\n");
        offset += chunk;
    }
}


/* ---- Modem status change ---- */

static void log_msr(unsigned char msr)
{
    char ts[20];

    if (msr == g_lastMSR) return;

    get_timestamp(ts, sizeof(ts));
    logprintf("%s  MSR  DCD=%d DSR=%d CTS=%d RI=%d",
              ts,
              (msr & MS_DCD) ? 1 : 0,
              (msr & MS_DSR) ? 1 : 0,
              (msr & MS_CTS) ? 1 : 0,
              (msr & MS_RI)  ? 1 : 0);

    /* Show deltas */
    if (msr != g_lastMSR) {
        logprintf("  (");
        if ((msr ^ g_lastMSR) & MS_DCD) logprintf(" dDCD");
        if ((msr ^ g_lastMSR) & MS_DSR) logprintf(" dDSR");
        if ((msr ^ g_lastMSR) & MS_CTS) logprintf(" dCTS");
        if ((msr ^ g_lastMSR) & MS_RI)  logprintf(" dRI");
        logprintf(" )");
    }
    logprintf("\n");

    g_lastMSR = msr;
}


/* ---- Line status change ---- */

static void log_lsr(unsigned char lsr)
{
    char ts[20];

    /* Only log if error bits are set */
    if (!(lsr & (LS_OVERRUN | LS_PARITY | LS_FRAMING | LS_BREAK)))
        return;
    if (lsr == g_lastLSR) return;

    get_timestamp(ts, sizeof(ts));
    logprintf("%s  LSR  OE=%d PE=%d FE=%d BI=%d\n",
              ts,
              (lsr & LS_OVERRUN) ? 1 : 0,
              (lsr & LS_PARITY)  ? 1 : 0,
              (lsr & LS_FRAMING) ? 1 : 0,
              (lsr & LS_BREAK)   ? 1 : 0);

    g_lastLSR = lsr;
}


/* ---- Query port status via IOCtl ---- */

static unsigned char query_msr(HFILE hCom)
{
    unsigned char msr = 0;
    ULONG parmLen = 0, dataLen = sizeof(msr);

    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
                NULL, 0, &parmLen,
                &msr, sizeof(msr), &dataLen);
    return msr;
}

static unsigned char query_lsr(HFILE hCom)
{
    unsigned char lsr = 0;
    ULONG parmLen = 0, dataLen = sizeof(lsr);

    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETLINESTATUS,
                NULL, 0, &parmLen,
                &lsr, sizeof(lsr), &dataLen);
    return lsr;
}

static unsigned short query_rxcount(HFILE hCom)
{
    unsigned short count = 0;
    ULONG parmLen = 0, dataLen = sizeof(count);

    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
                NULL, 0, &parmLen,
                &count, sizeof(count), &dataLen);
    return count;
}


/* ---- Signal handler (Ctrl-C) ---- */

static void __cdecl sig_handler(int sig)
{
    (void)sig;
    g_running = 0;
}


/* ================================================================
 * MAIN
 * ================================================================ */

int main(int argc, char *argv[])
{
    char    *portName;
    char    *logName = NULL;
    ULONG   action, rc;
    char    ts[20];

    printf("SIO2K Port Activity Logger v%s\n", VERSION);
    printf("GPLv3 — FPC264IRC Contributors, 2026\n\n");

    if (argc < 2) {
        printf("Usage: LOGGER DEV <TraceFileName>\n\n");
        printf("  DEV is a Device Name or a comm port number (1 relative).\n\n");
        printf("Examples:\n");
        printf("  LOGGER 1 LOGCOM1.LOG     Monitor COM1, log to file\n");
        printf("  LOGGER COM1              Monitor COM1, screen only\n\n");
        printf("Note: Do NOT use names like COM1.LOG — use LOGCOM1.LOG\n");
        printf("      to avoid confusing OS/2's device name resolution.\n");
        return 1;
    }

    /* Accept port number (1-relative) or device name.
     * Original LOGGER opens \dev\$sio$ for direct SIO access.
     * If user gives a number, build the SIO device path.
     * If user gives COMn, convert to number.
     * If user gives a full path, use it directly. */
    {
        static char devPath[64];
        char *arg = argv[1];
        int portNum = 0;

        if (arg[0] >= '1' && arg[0] <= '9' && strlen(arg) <= 3) {
            /* Bare number: "1", "2", "12" */
            portNum = atoi(arg);
        } else if (strnicmp(arg, "COM", 3) == 0 && isdigit(arg[3])) {
            /* COMn format */
            portNum = atoi(arg + 3);
        }

        if (portNum > 0) {
            /* Open SIO device directly — sees all events.
             * Format: \dev\$sio$ with port IOCtl parameter */
            snprintf(devPath, sizeof(devPath), "\\dev\\$sio$");
            portName = devPath;
            printf("Opening SIO device for COM%d...\n", portNum);
        } else {
            /* Full device path given */
            portName = arg;
        }
    }
    if (argc > 2) logName = argv[2];

    /* Open log file */
    if (logName) {
        g_logFile = fopen(logName, "a");
        if (!g_logFile) {
            fprintf(stderr, "ERROR: Cannot open log file: %s\n", logName);
            return 1;
        }
    }

    /* Open COM port in shared mode (don't interfere with app) */
    rc = DosOpen(portName, &g_hCom, &action, 0,
                 FILE_NORMAL,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                 OPEN_ACCESS_READONLY,
                 0);

    if (rc != 0) {
        fprintf(stderr, "ERROR: Cannot open %s (rc=%lu)\n", portName, rc);
        fprintf(stderr, "Port may not exist or SIO2K.SYS not loaded.\n");
        if (g_logFile) fclose(g_logFile);
        return 1;
    }

    /* Install Ctrl-C handler */
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    get_timestamp(ts, sizeof(ts));
    logprintf("%s  ---  Logger started on %s\n", ts, portName);

    /* Initial modem status */
    g_lastMSR = query_msr(g_hCom);
    log_msr(g_lastMSR);

    /* Main poll loop */
    while (g_running) {
        unsigned char msr, lsr;
        unsigned short rxCount;

        /* Check modem status changes */
        msr = query_msr(g_hCom);
        log_msr(msr);

        /* Check line status errors */
        lsr = query_lsr(g_hCom);
        log_lsr(lsr);

        /* Check for RX data (read-only — we don't consume it) */
        rxCount = query_rxcount(g_hCom);
        if (rxCount > 0) {
            get_timestamp(ts, sizeof(ts));
            logprintf("%s  INF  %u bytes in RX queue\n", ts, rxCount);
        }

        /* Sleep between polls */
        DosSleep(POLL_MS);
    }

    get_timestamp(ts, sizeof(ts));
    logprintf("%s  ---  Logger stopped\n", ts);

    DosClose(g_hCom);
    if (g_logFile) fclose(g_logFile);

    return 0;
}
