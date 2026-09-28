/* ====================================================================
 * SU.EXE — SIO Status Utility for OS/2
 * ====================================================================
 * Clean-room reimplementation from SIOREF.TXT.
 * Provides port status, IRQ info, and modem signal control.
 *
 * Usage: SU [command] [port]
 *   SU              — show available commands
 *   SU STATUS n     — show status for COM port n
 *   SU IRQS         — show free IRQs
 *   SU DTR n ON|OFF — control DTR on port n
 *   SU RTS n ON|OFF — control RTS on port n
 *   SU SIGNALS n    — show modem signals on port n
 *   SU BAUD n rate  — set baud rate on port n
 * ====================================================================
 */

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ASYNC IOCtl codes from OS/2 bsedev.h */
#include <bsedev.h>

/* ====================================================================
 * DOS$ device reboot IOCtl — undocumented, but well established.
 * There's no documented OS/2 API in the official SDK headers (checked
 * against os2tk45) for rebooting the system; the only known method is
 * this undocumented IOCTL on the DOS$ device driver, the same one
 * SETBOOT.EXE itself uses. Category/function codes and calling
 * sequence per the EDM/2 writeup on rebooting OS/2 (Roman Stangl) —
 * the same values are echoed in more than one independent source, so
 * treated as reliable despite being undocumented.
 * DOS.SYS must be loaded (DEVICE=...\DOS.SYS in CONFIG.SYS) for
 * \DEV\DOS$ to exist — it does not ship loaded by default on every
 * OS/2 configuration (e.g. the IBM EWS BOOTOS2 package omits it).
 * ==================================================================== */
#define IOCTL_DOS       0x00D5
#define DOS_REBOOT      0x00AB

static HFILE OpenPort(int portNum);
static void  ClosePort(HFILE hf);
static void  ShowUsage(void);
static void  ShowStatus(int port);
static void  ShowIRQs(void);
static void  ShowSignals(int port);
static void  ShowSignalsH(HFILE hf);  /* With pre-opened handle */
static void  SetDTR(int port, int on);
static void  SetRTS(int port, int on);
static void  SetBaud(int port, long rate);
static int   QueryDCD(int port);
static int   DoReboot(void);
static void  ShowEnhanced(int port);

int main(int argc, char *argv[])
{
    printf("SU — SIO Status Utility v1.00\n\n");

    if (argc < 2) {
        ShowUsage();
        return 0;
    }

    if (stricmp(argv[1], "IRQS") == 0) {
        ShowIRQs();
    }
    else if (stricmp(argv[1], "STATUS") == 0 && argc >= 3) {
        ShowStatus(atoi(argv[2]));
    }
    else if (stricmp(argv[1], "SIGNALS") == 0 && argc >= 3) {
        ShowSignals(atoi(argv[2]));
    }
    else if (stricmp(argv[1], "DTR") == 0 && argc >= 4) {
        SetDTR(atoi(argv[2]), stricmp(argv[3], "ON") == 0);
    }
    else if (stricmp(argv[1], "RTS") == 0 && argc >= 4) {
        SetRTS(atoi(argv[2]), stricmp(argv[3], "ON") == 0);
    }
    else if (stricmp(argv[1], "BAUD") == 0 && argc >= 4) {
        SetBaud(atoi(argv[2]), atol(argv[3]));
    }
    else if (stricmp(argv[1], "DCD") == 0 && argc >= 3) {
        /* "Exits with ErrorLevel 1 if DCD is on" — per the original
         * binary's own help text. Previously unimplemented; the bit
         * itself was already being queried (ShowSignals), just never
         * exposed as its own subcommand/exit code. */
        return QueryDCD(atoi(argv[2]));
    }
    else if (stricmp(argv[1], "BOOT") == 0) {
        return DoReboot();
    }
    else if (stricmp(argv[1], "ENHANCED") == 0 && argc >= 3) {
        ShowEnhanced(atoi(argv[2]));
    }
    else {
        ShowUsage();
    }

    return 0;
}

static void ShowUsage(void)
{
    printf("Commands:\n");
    printf("  SU STATUS n      Show port status\n");
    printf("  SU IRQS           Show available IRQs\n");
    printf("  SU SIGNALS n     Show modem signals\n");
    printf("  SU DTR n ON|OFF  Control DTR\n");
    printf("  SU RTS n ON|OFF  Control RTS\n");
    printf("  SU BAUD n rate   Set baud rate\n");
    printf("  SU DCD n         Exit code 1 if DCD is on, else 0\n");
    printf("  SU BOOT          Reboot the system\n");
    printf("  SU ENHANCED n    Display enhanced mode settings\n");
}

static HFILE OpenPort(int portNum)
{
    HFILE   hf;
    ULONG   action;
    APIRET  rc;
    char    name[16];

    sprintf(name, "COM%d", portNum);
    rc = DosOpen(name, &hf, &action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL);
    if (rc != 0) {
        printf("Error: Cannot open %s (rc=%lu)\n", name, rc);
        return (HFILE)-1;
    }
    return hf;
}

static void ClosePort(HFILE hf)
{
    if (hf != (HFILE)-1)
        DosClose(hf);
}

static void ShowStatus(int port)
{
    HFILE hf = OpenPort(port);
    if (hf == (HFILE)-1) return;

    printf("COM%d Status:\n", port);

    /* Extended baud rate (63h) */
    {
        ULONG parmLen = 0, dataLen = 15;
        BYTE  data[15];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_EXTGETBAUDRATE,
                                NULL, 0, &parmLen, data, 15, &dataLen);
        if (rc == 0) {
            ULONG curBaud = *(ULONG *)&data[0];
            ULONG minBaud = *(ULONG *)&data[5];
            ULONG maxBaud = *(ULONG *)&data[10];
            printf("  Baud rate: %lu (min %lu, max %lu)\n",
                   curBaud, minBaud, maxBaud);
        }
    }

    /* Line control (62h) */
    {
        ULONG parmLen = 0, dataLen = 4;
        BYTE  data[4];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETLINECTRL,
                                NULL, 0, &parmLen, data, 4, &dataLen);
        if (rc == 0) {
            const char *parStr[] = {"None","Odd","Even","Mark","Space"};
            printf("  Line: %d data bits, %s parity, %s stop\n",
                   data[0],
                   data[1] < 5 ? parStr[data[1]] : "?",
                   data[2] == 0 ? "1" : data[2] == 1 ? "1.5" : "2");
            if (data[3]) printf("  ** BREAK active **\n");
        }
    }

    /* Buffer counts (68h, 69h) */
    {
        ULONG parmLen = 0, dataLen = 4;
        BYTE  data[4];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
                                NULL, 0, &parmLen, data, 4, &dataLen);
        if (rc == 0) {
            printf("  RX buffer: %u / %u bytes\n",
                   *(USHORT *)&data[0], *(USHORT *)&data[2]);
        }
        dataLen = 4;
        rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT,
                         NULL, 0, &parmLen, data, 4, &dataLen);
        if (rc == 0) {
            printf("  TX buffer: %u / %u bytes\n",
                   *(USHORT *)&data[0], *(USHORT *)&data[2]);
        }
    }

    /* Comm status (64h) */
    {
        ULONG parmLen = 0, dataLen = 1;
        BYTE  data[1];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETCOMMSTATUS,
                                NULL, 0, &parmLen, data, 1, &dataLen);
        if (rc == 0) {
            printf("  Status: ");
            if (data[0] & 0x01) printf("Wait-CTS ");
            if (data[0] & 0x02) printf("Wait-DSR ");
            if (data[0] & 0x04) printf("Wait-DCD ");
            if (data[0] & 0x08) printf("XOFF/Hold ");
            if (data[0] & 0x20) printf("BREAK ");
            if (data[0] & 0x40) printf("TxImm-pending ");
            if (data[0] == 0)   printf("OK");
            printf("\n");
        }
    }

    /* Error word (6Dh) */
    {
        ULONG parmLen = 0, dataLen = 2;
        BYTE  data[2];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETCOMMERROR,
                                NULL, 0, &parmLen, data, 2, &dataLen);
        if (rc == 0) {
            USHORT err = *(USHORT *)&data[0];
            if (err) {
                printf("  Errors: ");
                if (err & 0x01) printf("BufOverrun ");
                if (err & 0x02) printf("UARTOverrun ");
                if (err & 0x04) printf("Parity ");
                if (err & 0x08) printf("Framing ");
                printf("\n");
            }
        }
    }

    ShowSignalsH(hf);
    ClosePort(hf);
}

static void ShowSignals(int port)
{
    HFILE hf = OpenPort(port);
    if (hf == (HFILE)-1) return;

    /* Modem output (66h) */
    {
        ULONG parmLen = 0, dataLen = 1;
        BYTE  data[1];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMOUTPUT,
                                NULL, 0, &parmLen, data, 1, &dataLen);
        if (rc == 0) {
            printf("  Output: DTR=%s  RTS=%s\n",
                   (data[0] & 0x01) ? "ON " : "OFF",
                   (data[0] & 0x02) ? "ON " : "OFF");
        }
    }

    /* Modem input (67h) */
    {
        ULONG parmLen = 0, dataLen = 1;
        BYTE  data[1];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
                                NULL, 0, &parmLen, data, 1, &dataLen);
        if (rc == 0) {
            printf("  Input:  CTS=%s  DSR=%s  RI=%s  DCD=%s\n",
                   (data[0] & 0x10) ? "ON " : "OFF",
                   (data[0] & 0x20) ? "ON " : "OFF",
                   (data[0] & 0x40) ? "ON " : "OFF",
                   (data[0] & 0x80) ? "ON " : "OFF");
        }
    }

    ClosePort(hf);
}

static void ShowSignalsH(HFILE hf)
{

    /* Modem output (66h) */
    {
        ULONG parmLen = 0, dataLen = 1;
        BYTE  data[1];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMOUTPUT,
                                NULL, 0, &parmLen, data, 1, &dataLen);
        if (rc == 0) {
            printf("  Output: DTR=%s  RTS=%s\n",
                   (data[0] & 0x01) ? "ON " : "OFF",
                   (data[0] & 0x02) ? "ON " : "OFF");
        }
    }

    /* Modem input (67h) */
    {
        ULONG parmLen = 0, dataLen = 1;
        BYTE  data[1];
        APIRET rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
                                NULL, 0, &parmLen, data, 1, &dataLen);
        if (rc == 0) {
            printf("  Input:  CTS=%s  DSR=%s  RI=%s  DCD=%s\n",
                   (data[0] & 0x10) ? "ON " : "OFF",
                   (data[0] & 0x20) ? "ON " : "OFF",
                   (data[0] & 0x40) ? "ON " : "OFF",
                   (data[0] & 0x80) ? "ON " : "OFF");
        }
    }

}

static int QueryDCD(int port)
{
    /* "SU DCD n — Exits with ErrorLevel 1 if DCD is on" (per the
     * original binary's own help text, confirmed via `strings`).
     * Silent — no printf — since this is meant for use in a CMD
     * batch file's IF ERRORLEVEL check, not interactive output. */
    HFILE hf = OpenPort(port);
    ULONG parmLen, dataLen;
    BYTE  data[1];
    APIRET rc;

    if (hf == (HFILE)-1) return 0;

    parmLen = 0; dataLen = 1;
    rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
                      NULL, 0, &parmLen, data, 1, &dataLen);
    ClosePort(hf);

    if (rc != 0) return 0;
    return (data[0] & 0x80) ? 1 : 0;
}

static int DoReboot(void)
{
    HFILE  hDosDD;
    ULONG  action;
    APIRET rc;

    printf("Rebooting the system...\n");
    fflush(stdout);

    rc = DosOpen("\\DEV\\DOS$", &hDosDD, &action, 0,
                 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, 0);
    if (rc != 0) {
        fprintf(stderr, "SU: Cannot open \\DEV\\DOS$ (rc=%lu).\n", rc);
        fprintf(stderr, "    Is DOS.SYS loaded in CONFIG.SYS?\n");
        fprintf(stderr, "    (DEVICE=...\\OS2\\BOOT\\DOS.SYS on Warp,\n");
        fprintf(stderr, "     DEVICE=...\\OS2\\DOS.SYS on 2.11 and earlier)\n");
        return 1;
    }

    /* Shut down the file system. DosShutdown() doesn't stop running
     * processes, but file system calls stop working afterward —
     * that's why \DEV\DOS$ had to be opened first, since DosOpen()
     * is itself a file system call. */
    DosShutdown(0);

    /* Jump to the reboot vector via the undocumented DOS$ IOCtl. If
     * this call succeeds, it does not return — the machine resets. */
    rc = DosDevIOCtl(hDosDD, IOCTL_DOS, DOS_REBOOT,
                      NULL, 0, NULL, NULL, 0, NULL);

    /* Only reached if the reboot IOCtl itself failed. The file
     * system is already shut down at this point, so there isn't
     * much more SU can do about it than report the failure. */
    fprintf(stderr, "SU: Reboot IOCtl failed (rc=%lu).\n", rc);
    DosClose(hDosDD);
    return 1;
}

static void ShowEnhanced(int port)
{
    /* "SU ENHANCED n — Display enhanced mode settings" (per the
     * original binary's own strings). Three of its four lines
     * ("Error Replacement Character", "Null Stripping", "Break
     * Replacement Character") are always "not supported by SIO" in
     * the original itself — these are 16550-class UART features SIO
     * never implements regardless of the actual hardware, so
     * reporting them as fixed/disabled here is faithful, not a
     * shortcut. Requested explicitly even though there's no real
     * functional gap to close: these are informational strings for
     * output parity with the original, not a live hardware query.
     *
     * The XON/XOFF line *is* queryable state, but only as a
     * currently-held-for-XOFF flag (ASYNC_GETCOMMSTATUS bit 0x08),
     * not as an enabled/disabled software-flow-control setting —
     * there's no IOCtl for that in bsedev.h. Labeled accordingly
     * rather than presenting it as the same kind of toggle. */
    HFILE hf = OpenPort(port);
    ULONG parmLen, dataLen;
    BYTE  data[1];
    APIRET rc;

    if (hf == (HFILE)-1) return;

    printf("COM%d Enhanced Mode Settings:\n", port);

    parmLen = 0; dataLen = 1;
    rc = DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_GETCOMMSTATUS,
                      NULL, 0, &parmLen, data, 1, &dataLen);
    if (rc == 0 && (data[0] & 0x08))
        printf("  Transmitted XON/XOFF: currently held (XOFF received)\n");
    else
        printf("  Transmitted XON/XOFF: not currently held\n");

    printf("  Error Replacement Character is disabled "
           "(not supported by SIO).\n");
    printf("  Null Stripping is disabled "
           "(not supported by SIO).\n");
    printf("  Break Replacement Character is disabled "
           "(not supported by SIO).\n");

    ClosePort(hf);
}

static void ShowIRQs(void)
{
    printf("IRQ availability:\n");
    printf("  (Query requires SIO-specific IOCtl — not in standard ASYNC spec)\n");
    printf("  Standard assignments:\n");
    printf("    IRQ3: COM2, COM4\n");
    printf("    IRQ4: COM1, COM3\n");
    printf("  SIO supports IRQ sharing — multiple ports may use the same IRQ.\n");
}

static void SetDTR(int port, int on)
{
    HFILE hf = OpenPort(port);
    BYTE  parm[2];
    ULONG parmLen = 2, dataLen = 2;
    BYTE  data[2] = {0, 0};

    if (hf == (HFILE)-1) return;

    if (on) {
        parm[0] = 0x01;    /* ON mask: DTR */
        parm[1] = 0xFF;    /* OFF mask: don't turn anything off */
    } else {
        parm[0] = 0x00;    /* ON mask: nothing */
        parm[1] = 0xFE;    /* OFF mask: clear DTR */
    }

    DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                parm, 2, &parmLen, data, 2, &dataLen);

    printf("COM%d DTR %s\n", port, on ? "ON" : "OFF");
    ClosePort(hf);
}

static void SetRTS(int port, int on)
{
    HFILE hf = OpenPort(port);
    BYTE  parm[2];
    ULONG parmLen = 2, dataLen = 2;
    BYTE  data[2] = {0, 0};

    if (hf == (HFILE)-1) return;

    if (on) {
        parm[0] = 0x02;    /* ON mask: RTS */
        parm[1] = 0xFF;
    } else {
        parm[0] = 0x00;
        parm[1] = 0xFD;    /* OFF mask: clear RTS */
    }

    DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                parm, 2, &parmLen, data, 2, &dataLen);

    printf("COM%d RTS %s\n", port, on ? "ON" : "OFF");
    ClosePort(hf);
}

static void SetBaud(int port, long rate)
{
    HFILE hf = OpenPort(port);
    ULONG parmLen, dataLen;

    if (hf == (HFILE)-1) return;

    if (rate <= 65535) {
        USHORT baud = (USHORT)rate;
        parmLen = 2; dataLen = 0;
        DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_SETBAUDRATE,
                    &baud, 2, &parmLen, NULL, 0, &dataLen);
    } else {
        BYTE parm[5];
        *(ULONG *)&parm[0] = (ULONG)rate;
        parm[4] = 0;        /* Fraction */
        parmLen = 5; dataLen = 0;
        DosDevIOCtl(hf, IOCTL_ASYNC, ASYNC_EXTSETBAUDRATE,
                    parm, 5, &parmLen, NULL, 0, &dataLen);
    }

    printf("COM%d baud rate set to %ld\n", port, rate);
    ClosePort(hf);
}
