/* ====================================================================
 * modes.c — MODES.EXE Port Configuration/Swap Utility for OS/2
 * ====================================================================
 * Replacement for OS/2's MODE.COM for serial ports.
 * Supports COM1-COM256 (MODE.COM only supports COM1-COM9).
 *
 * Features beyond MODE.COM:
 *   - Ports > COM9
 *   - Baud rate LOCK/UNLOCK (SIO2K-specific)
 *   - Port swapping (MODES COM1=COM10)
 *   - Does NOT validate baud rate (intentional — allows non-standard)
 *   - DOES validate parity/databits/stopbits and their combination,
 *     matching the original's SYS1630/1631/1632/0724 messages (this
 *     was previously parsed but never validated — invalid values
 *     were silently accepted or silently coerced)
 *
 * Usage:
 *   MODES COMn                          Display current settings
 *   MODES COMn:baud,parity,data,stop    Set line characteristics
 *   MODES COMn:LOCK 115200              Lock baud rate
 *   MODES COMn:UNLOCK                   Unlock baud rate
 *   MODES COMn=COMm                     Swap port assignments
 *
 * Syntax is identical to OS/2 MODE command for serial ports.
 *
 * Clean-room reimplementation from MODES.TXT documentation.
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VERSION     "2.0.0"

/* SIO2K IOCtl categories and functions */
#define IOCTL_ASYNC         0x01
#define IOCTL_SIO2K         0x80

#define ASYNC_SETBAUDRATE   0x0041  /* Set baud rate */
#define ASYNC_SETLINECTRL   0x0042  /* Set line characteristics */
#define ASYNC_GETBAUDRATE   0x0061  /* Get baud rate */
#define ASYNC_GETLINECTRL   0x0062  /* Get line characteristics */
#define ASYNC_GETCOMSTATUS  0x0064  /* Get COM status */
#define ASYNC_GETLINESTATUS 0x0065  /* Get line status */
#define ASYNC_GETMODEMINPUT 0x0068  /* Get modem input signals */

/* SIO2K-specific IOCtl */
#define SIO2K_LOCKBAUD      0x0090  /* Lock baud rate */
#define SIO2K_UNLOCKBAUD    0x0091  /* Unlock baud rate */
#define SIO2K_SWAPPORTS     0x0092  /* Swap port assignments */
#define SIO2K_GETPORTINFO   0x0093  /* Get extended port info */

/* Parity codes */
#define PARITY_NONE     0
#define PARITY_ODD      1
#define PARITY_EVEN     2
#define PARITY_MARK     3
#define PARITY_SPACE    4


/* ---- Display current port settings ---- */

static int show_port(const char *portName)
{
    HFILE hCom;
    ULONG action, rc;
    ULONG parmLen, dataLen;
    unsigned long baudRate = 0;
    unsigned char lineCtrl[3] = {0};    /* data, parity, stop */
    unsigned char msr = 0;
    unsigned char lsr = 0;
    const char *parityStr;

    rc = DosOpen(portName, &hCom, &action, 0,
                 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                 OPEN_ACCESS_READONLY, 0);
    if (rc != 0) {
        fprintf(stderr, "%s is not available (rc=%lu)\n", portName, rc);
        return 1;
    }

    /* Get baud rate */
    parmLen = 0; dataLen = sizeof(baudRate);
    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETBAUDRATE,
                NULL, 0, &parmLen,
                &baudRate, sizeof(baudRate), &dataLen);

    /* Get line characteristics */
    parmLen = 0; dataLen = sizeof(lineCtrl);
    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETLINECTRL,
                NULL, 0, &parmLen,
                lineCtrl, sizeof(lineCtrl), &dataLen);

    /* Get modem status */
    parmLen = 0; dataLen = sizeof(msr);
    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
                NULL, 0, &parmLen,
                &msr, sizeof(msr), &dataLen);

    /* Get line status */
    parmLen = 0; dataLen = sizeof(lsr);
    DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETLINESTATUS,
                NULL, 0, &parmLen,
                &lsr, sizeof(lsr), &dataLen);

    switch (lineCtrl[1]) {
    case PARITY_NONE:  parityStr = "None"; break;
    case PARITY_ODD:   parityStr = "Odd";  break;
    case PARITY_EVEN:  parityStr = "Even"; break;
    case PARITY_MARK:  parityStr = "Mark"; break;
    case PARITY_SPACE: parityStr = "Space"; break;
    default:           parityStr = "?"; break;
    }

    printf("%s:\n", portName);
    printf("    Baud Rate:     %lu\n", baudRate);
    printf("    Data Bits:     %d\n", lineCtrl[0]);
    printf("    Parity:        %s\n", parityStr);
    printf("    Stop Bits:     %s\n", lineCtrl[2] == 2 ? "2" : "1");
    printf("    CTS=%d  DSR=%d  DCD=%d  RI=%d\n",
           (msr & 0x10) ? 1 : 0,
           (msr & 0x20) ? 1 : 0,
           (msr & 0x80) ? 1 : 0,
           (msr & 0x40) ? 1 : 0);

    DosClose(hCom);
    return 0;
}


/* ---- Set port parameters ---- */

static int set_port(const char *portName, const char *params)
{
    HFILE hCom;
    ULONG action, rc;
    ULONG parmLen, dataLen;
    char buf[256];
    char *tok;
    unsigned long baudRate = 0;
    unsigned char lineCtrl[3];
    int hasBaud = 0, hasLine = 0;
    int lockMode = 0;   /* 1=lock, -1=unlock */

    strncpy(buf, params, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Check for LOCK/UNLOCK prefix */
    if (strnicmp(buf, "LOCK", 4) == 0) {
        lockMode = 1;
        memmove(buf, buf + 4, strlen(buf + 4) + 1);
        while (*buf == ' ') memmove(buf, buf + 1, strlen(buf));
    } else if (strnicmp(buf, "UNLOCK", 6) == 0) {
        lockMode = -1;
        memmove(buf, buf + 6, strlen(buf + 6) + 1);
        while (*buf == ' ') memmove(buf, buf + 1, strlen(buf));
    }

    /* Parse baud,parity,data,stop */
    /* Comma-separated: 9600,N,8,1 or just 9600 */
    lineCtrl[0] = 8;       /* Default: 8 data bits */
    lineCtrl[1] = 0;       /* Default: no parity */
    lineCtrl[2] = 0;       /* Default: 1 stop bit */

    tok = strtok(buf, ", ");
    if (tok && isdigit(tok[0])) {
        baudRate = atol(tok);
        hasBaud = 1;
        tok = strtok(NULL, ", ");
    }

    if (tok) {
        /* Parity */
        char p = toupper(tok[0]);
        if (p == 'N')      lineCtrl[1] = PARITY_NONE;
        else if (p == 'O') lineCtrl[1] = PARITY_ODD;
        else if (p == 'E') lineCtrl[1] = PARITY_EVEN;
        else if (p == 'M') lineCtrl[1] = PARITY_MARK;
        else if (p == 'S') lineCtrl[1] = PARITY_SPACE;
        else {
            /* Previously: invalid parity chars silently fell through
             * to the "no parity" default with no error at all. The
             * original binary's own strings show it validates this
             * (SYS1632) — matching that message here. */
            fprintf(stderr, "SYS1632: The MODE parameter parity %s "
                             "is incorrect.\n", tok);
            return 1;
        }
        hasLine = 1;
        tok = strtok(NULL, ", ");
    }

    if (tok) {
        /* Data bits — valid range is 5-8; anything else was
         * previously accepted silently via atoi() with no check. */
        int db = atoi(tok);
        if (db < 5 || db > 8) {
            fprintf(stderr, "SYS1630: The MODE parameter databits %s "
                             "is incorrect.\n", tok);
            return 1;
        }
        lineCtrl[0] = (unsigned char)db;
        tok = strtok(NULL, ", ");
    }

    if (tok) {
        /* Stop bits — 1, 1.5, or 2. Previously: anything other than
         * a literal "2" silently became 1 stop bit with no error,
         * so "1.5", "3", or garbage all passed silently. */
        int sb;
        if (strcmp(tok, "1") == 0)        sb = 1;
        else if (strcmp(tok, "1.5") == 0) sb = 15;   /* internal: 1.5 */
        else if (strcmp(tok, "2") == 0)   sb = 2;
        else {
            fprintf(stderr, "SYS1631: The MODE parameter stopbits %s "
                             "is incorrect.\n", tok);
            return 1;
        }

        /* Combination check: 1.5 stop bits is only defined for 5
         * data bits (the classic async framing rule); 5 data bits
         * cannot use 2 stop bits. Neither was checked before. */
        if ((sb == 15 && lineCtrl[0] != 5) ||
            (sb == 2  && lineCtrl[0] == 5)) {
            fprintf(stderr, "SYS0724: An invalid combination of "
                             "DATABITS and STOPBITS has been found.\n");
            return 1;
        }

        lineCtrl[2] = (unsigned char)(sb == 15 ? 1 : (sb == 2 ? 2 : 0));
        /* NOTE: lineCtrl[2] only has room for 0=1 stop bit / 2=2 stop
         * bits in this driver's ASYNC_SETLINECTRL encoding (see
         * modes.c's own "NOEMS"-style stop-bit print logic below,
         * and sio2k.c's IOCtl62/matching SETLINECTRL handler) — there
         * is no distinct wire value for 1.5 today. Validating the
         * 1.5-stop-bits case above is still correct (it catches the
         * invalid combinations the original flags), but actually
         * transmitting 1.5 stop bits would need a wire-format change
         * this pass didn't make. Encoded as 1 stop bit for now rather
         * than silently miscounting it as 2.
         */
    }

    /* Open port */
    rc = DosOpen(portName, &hCom, &action, 0,
                 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                 OPEN_ACCESS_READWRITE, 0);
    if (rc != 0) {
        fprintf(stderr, "%s is not available (rc=%lu)\n", portName, rc);
        return 1;
    }

    /* Set baud rate */
    if (hasBaud) {
        parmLen = sizeof(baudRate); dataLen = 0;
        rc = DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_SETBAUDRATE,
                    &baudRate, sizeof(baudRate), &parmLen,
                    NULL, 0, &dataLen);
        if (rc == 0)
            printf("%s: Baud rate set to %lu\n", portName, baudRate);
        else
            fprintf(stderr, "%s: Failed to set baud rate (rc=%lu)\n", portName, rc);
    }

    /* Set line characteristics */
    if (hasLine) {
        parmLen = sizeof(lineCtrl); dataLen = 0;
        rc = DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_SETLINECTRL,
                    lineCtrl, sizeof(lineCtrl), &parmLen,
                    NULL, 0, &dataLen);
        if (rc == 0)
            printf("%s: Line set to %d%c%d\n", portName,
                   lineCtrl[0],
                   "NOEMS"[lineCtrl[1]],
                   lineCtrl[2] == 2 ? 2 : 1);
    }

    /* Lock/unlock baud rate (SIO2K-specific) */
    if (lockMode == 1) {
        parmLen = 0; dataLen = 0;
        rc = DosDevIOCtl(hCom, IOCTL_SIO2K, SIO2K_LOCKBAUD,
                    NULL, 0, &parmLen, NULL, 0, &dataLen);
        printf("%s: Baud rate LOCKED%s\n", portName,
               hasBaud ? "" : " at current rate");
    } else if (lockMode == -1) {
        parmLen = 0; dataLen = 0;
        rc = DosDevIOCtl(hCom, IOCTL_SIO2K, SIO2K_UNLOCKBAUD,
                    NULL, 0, &parmLen, NULL, 0, &dataLen);
        printf("%s: Baud rate UNLOCKED\n", portName);
    }

    DosClose(hCom);
    return 0;
}


/* ---- Swap ports ---- */

static int is_port_busy(const char *portName)
{
    /* Best-effort substitute for "is a mouse driver bound to this
     * port" (see the long comment in swap_ports below for why that
     * exact check isn't implementable). Try to open the port with
     * DENYWRITE|DENYREAD-equivalent (deny all) sharing; if something
     * else already has it open, OS/2 returns ERROR_SHARING_VIOLATION
     * (32) or ERROR_ACCESS_DENIED (5) rather than letting us in. This
     * catches a serial mouse driver OR any other process actively
     * using the port — a strictly more general check than "is it
     * specifically a mouse," but a real, verifiable safety net
     * instead of no check at all. */
    HFILE hCom;
    ULONG action, rc;

    rc = DosOpen(portName, &hCom, &action, 0,
                 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYREADWRITE |
                 OPEN_ACCESS_READWRITE, 0);
    if (rc == 0) {
        DosClose(hCom);
        return 0;   /* We got exclusive access — nothing else has it open */
    }
    return 1;       /* Couldn't get exclusive access — something's using it */
}

static int is_mouse_port(const char *portName)
{
    /* The real, targeted check: OS/2's IOCTL_POINTINGDEVICE category
     * (0x07) only reports whether a mouse exists anywhere in the
     * system (MOU_GETBUTTONCOUNT / ERROR_MOUSE_NO_DEVICE=385 if not)
     * — it has no notion of which port. The actual authoritative
     * answer to "which COM port has a serial mouse" lives in
     * CONFIG.SYS itself: DEVICE=...MOUSE.SYS TYPE=SERIAL$ PORT=COMn
     * (or similar) is literally how OS/2 binds a serial mouse to a
     * port. Parsing that is more correct than any runtime IOCtl,
     * since it's the actual source of truth rather than a proxy for
     * it. Case-insensitive substring match on the port name and the
     * MOUSE.SYS filename, which is deliberately loose — CONFIG.SYS
     * device lines vary in whitespace/casing/full-path form, and a
     * false positive here (refusing a swap unnecessarily) is far
     * safer than a false negative (silently swapping the mouse's
     * port out from under it). */
    FILE *cf;
    char  line[256];
    char  bootDrive[3] = "C:";
    char  path[16];
    ULONG driveNum = 0, driveMap = 0;
    int   found = 0;

    if (DosQueryCurrentDisk(&driveNum, &driveMap) == 0 && driveNum >= 1)
        bootDrive[0] = (char)('A' + (driveNum - 1));

    sprintf(path, "%s\\CONFIG.SYS", bootDrive);
    cf = fopen(path, "r");
    if (!cf) return 0;   /* Can't check — don't block the swap on this alone */

    while (fgets(line, sizeof(line), cf)) {
        char upper[256];
        int  i;
        for (i = 0; line[i] && i < 255; i++)
            upper[i] = (char)toupper((unsigned char)line[i]);
        upper[i] = '\0';

        if (strstr(upper, "MOUSE.SYS") && strstr(upper, "SERIAL")) {
            char portUpper[16];
            int  j;
            for (j = 0; portName[j] && j < 15; j++)
                portUpper[j] = (char)toupper((unsigned char)portName[j]);
            portUpper[j] = '\0';

            if (strstr(upper, portUpper)) {
                found = 1;
                break;
            }
        }
    }

    fclose(cf);
    return found;
}

static int swap_ports(const char *port1, const char *port2)
{
    HFILE hCom;
    ULONG action, rc;
    ULONG parmLen, dataLen;
    char swapData[32];

    /* Real, targeted check first: refuse if CONFIG.SYS shows a
     * serial mouse configured on either port. */
    if (is_mouse_port(port1)) {
        fprintf(stderr, "The mouse port cannot be swapped.\n");
        return 1;
    }
    if (is_mouse_port(port2)) {
        fprintf(stderr, "The mouse port cannot be swapped.\n");
        return 1;
    }

    /* General fallback: refuse if either port is held open by any
     * other process right now (catches a mouse driver that doesn't
     * match the CONFIG.SYS pattern above, plus anything else using
     * the port). */
    if (is_port_busy(port1)) {
        fprintf(stderr, "%s is currently in use by another process "
                        "and cannot be swapped.\n", port1);
        return 1;
    }
    if (is_port_busy(port2)) {
        fprintf(stderr, "%s is currently in use by another process "
                        "and cannot be swapped.\n", port2);
        return 1;
    }

    /* Open the first port to send the swap IOCtl */
    rc = DosOpen(port1, &hCom, &action, 0,
                 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                 OPEN_ACCESS_READWRITE, 0);
    if (rc != 0) {
        fprintf(stderr, "%s is not available (rc=%lu)\n", port1, rc);
        return 1;
    }

    /* Build swap parameter: target port name */
    strncpy(swapData, port2, sizeof(swapData) - 1);
    swapData[sizeof(swapData) - 1] = '\0';

    parmLen = (ULONG)strlen(swapData) + 1;
    dataLen = 0;
    rc = DosDevIOCtl(hCom, IOCTL_SIO2K, SIO2K_SWAPPORTS,
                swapData, parmLen, &parmLen,
                NULL, 0, &dataLen);

    /* Message text matched to the original binary's own strings
     * ("<port> has been swapped with OS/2 device <port>" /
     * "Cannot swap <port>" / "The mouse port cannot be swapped.") */
    if (rc == 0)
        printf("%s has been swapped with OS/2 device %s\n", port1, port2);
    else
        fprintf(stderr, "Cannot swap %s (rc=%lu) — may require SIO2K.SYS\n",
                port1, rc);

    DosClose(hCom);
    return 0;
}


/* ================================================================
 * MAIN
 * ================================================================ */

int main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        printf("MODES v%s — Serial Port Configuration for OS/2\n", VERSION);
        printf("GPLv3 — FPC264IRC Contributors, 2026\n\n");
        printf("Usage:\n");
        printf("  MODES COMn                       Display settings\n");
        printf("  MODES COMn:baud,parity,data,stop  Set parameters\n");
        printf("  MODES COMn:LOCK [baud]            Lock baud rate\n");
        printf("  MODES COMn:UNLOCK [baud]          Unlock baud rate\n");
        printf("  MODES COMn=COMm                   Swap port assignments\n\n");
        printf("Examples:\n");
        printf("  MODES COM1:9600,N,8,1\n");
        printf("  MODES COM1:LOCK 115200\n");
        printf("  MODES COM1=COM10\n");
        return 0;
    }

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];
        char portName[16];
        char *colon, *equals;

        /* Check for swap: COMn=COMm */
        equals = strchr(arg, '=');
        if (equals && strnicmp(arg, "COM", 3) == 0) {
            *equals = '\0';
            snprintf(portName, sizeof(portName), "%s", arg);
            swap_ports(portName, equals + 1);
            continue;
        }

        /* Check for set: COMn:params or COMn params */
        colon = strchr(arg, ':');
        if (colon) {
            *colon = '\0';
            snprintf(portName, sizeof(portName), "%s", arg);
            /* Params might continue in next argv */
            if (colon[1]) {
                set_port(portName, colon + 1);
            } else if (i + 1 < argc) {
                set_port(portName, argv[++i]);
            }
            continue;
        }

        /* Check if next arg is params */
        if (strnicmp(arg, "COM", 3) == 0) {
            snprintf(portName, sizeof(portName), "%s", arg);
            if (i + 1 < argc && (isdigit(argv[i+1][0]) ||
                strnicmp(argv[i+1], "LOCK", 4) == 0 ||
                strnicmp(argv[i+1], "UNLOCK", 6) == 0)) {
                set_port(portName, argv[++i]);
            } else {
                show_port(portName);
            }
        }
    }

    return 0;
}
