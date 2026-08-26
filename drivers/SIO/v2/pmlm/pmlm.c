/* ====================================================================
 * pmlm.c — PMLM.EXE Port/Modem Line Monitor for OS/2 PM
 * ====================================================================
 * Presentation Manager application. Monitors serial port activity
 * in real-time via \dev\$sio$ IOCtl interface.
 *
 * Displays:
 *   Real UART signals:  RTS CTS DTR DSR DCD RI
 *   Virt UART signals:  RTS CTS DTR DSR
 *   Data hex/ASCII dump with timestamps
 *   Statistics: RX/TX byte counts, throughput (bps)
 *
 * Menu: Clear  Start/Stop  eXit  Disk-save  Lm font  Hex font  Ascii font
 *
 * Usage: PMLM DEV <TraceFileName>
 *   DEV = device name or comm port number (1 relative)
 *
 * Original: 22,411 bytes NE (16-bit OS/2 PM), v2.00.
 * Clean-room reimplementation. GPLv3.
 * ==================================================================== */

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION         "2.00"
#define TIMER_ID        1
#define TIMER_MS        50          /* 50ms refresh (20 Hz) */
#define MAX_LINES       500         /* Scrollback buffer */
#define LINE_LEN        80

/* IOCtl for SIO2K monitoring */
#define IOCTL_SIO2K     0x80
#define SIO2K_GETMODEM  0x00A1      /* Get modem signals */
#define SIO2K_GETCOUNT  0x00A2      /* Get byte counts */
#define SIO2K_GETTYPE   0x00A3      /* Get UART/physical-layer type */

/* Category 01h (async device) IOCtl used just to read the current
 * line control, so throughput can be reported at the actual bits-
 * per-frame instead of assuming a fixed 8N1 frame. */
#define IOCTL_ASYNC          0x01
#define ASYNC_GETLINECTRL    0x0062

/* PHYS_UART_* values from inc/sio2k_idc.h (not included here since
 * this is a user-mode app, not the driver — mirrored to avoid a
 * driver-internal header dependency). */
#define PHYS_UART_ESP       20
#define PHYS_UART_VMODEM    30

/* Window IDs */
#define ID_FRAME        100
#define IDM_CLEAR       200
#define IDM_STARTSTOP   201
#define IDM_EXIT        202
#define IDM_DISKSAVE    203
#define IDM_LMFONT      204
#define IDM_HEXFONT     205
#define IDM_ASCIIFONT   206

/* Signal indicator states */
#define SIG_OFF         0
#define SIG_ON          1
#define SIG_NA          2           /* Not available (virtual UART) */

/* ---- Per-port monitor state ---- */

typedef struct {
    HFILE       hDev;               /* \dev\$sio$ handle */
    FILE       *fTrace;             /* Disk trace file */
    int         monitoring;         /* Active flag */
    int         portNum;            /* Port number (1-relative) */
    int         isVirtual;          /* Virtual UART (VMODEM) */
    int         isESP;              /* ComBic / ESP card */

    /* Current signal states */
    unsigned char rts, cts, dtr, dsr, dcd, ri;
    unsigned char prev_rts, prev_cts, prev_dtr, prev_dsr, prev_dcd, prev_ri;

    /* Statistics */
    unsigned long rxChars;
    unsigned long txChars;
    unsigned long rxBps;            /* Current throughput */
    unsigned long txBps;
    unsigned long lastRx, lastTx;   /* Previous counts for delta */
    unsigned long lastTick;
    int         bitsPerFrame;       /* start+data+parity+stop, for bps calc */

    /* Display buffer */
    char lines[MAX_LINES][LINE_LEN];
    int  lineCount;
    int  topLine;                   /* Scroll position */
} PMLM_STATE;

static PMLM_STATE g_state;
static HAB g_hab;
static HWND g_hwndClient;


/* ---- Signal LED drawing ---- */

static void draw_led(HPS hps, int x, int y, int state, const char *label)
{
    POINTL pt;
    LONG color;

    switch (state) {
    case SIG_ON:  color = CLR_GREEN; break;
    case SIG_OFF: color = CLR_DARKGRAY; break;
    default:      color = CLR_PALEGRAY; break;
    }

    /* Draw filled circle */
    pt.x = x; pt.y = y;
    GpiSetColor(hps, color);
    GpiMove(hps, &pt);
    {
        ARCPARAMS ap = { 5, 5, 0, 0 };
        GpiSetArcParams(hps, &ap);
        GpiFullArc(hps, DRO_FILL, MAKEFIXED(1, 0));
    }

    /* Draw label below */
    pt.x = x - 6; pt.y = y - 16;
    GpiSetColor(hps, CLR_BLACK);
    GpiCharStringAt(hps, &pt, (LONG)strlen(label), (PCH)label);
}

static void draw_signals(HPS hps, PMLM_STATE *s)
{
    int y = 30, x = 20, gap = 45;

    /* Real UART signals */
    GpiCharStringAt(hps, &(POINTL){10, 50}, 9, "Real UART");
    draw_led(hps, x,        y, s->rts, "RTS");
    draw_led(hps, x+gap,    y, s->cts, "CTS");
    draw_led(hps, x+gap*2,  y, s->dtr, "DTR");
    draw_led(hps, x+gap*3,  y, s->dsr, "DSR");
    draw_led(hps, x+gap*4,  y, s->dcd, "DCD");
    draw_led(hps, x+gap*5,  y, s->ri,  "RI");

    if (s->isVirtual) {
        /* Virtual UART (no DCD/RI) */
        GpiCharStringAt(hps, &(POINTL){320, 50}, 9, "Virt UART");
        draw_led(hps, 330,       y, s->rts, "RTS");
        draw_led(hps, 330+gap,   y, s->cts, "CTS");
        draw_led(hps, 330+gap*2, y, s->dtr, "DTR");
        draw_led(hps, 330+gap*3, y, s->dsr, "DSR");
    }
}


/* ---- Status bar ---- */

static void draw_status(HPS hps, PMLM_STATE *s)
{
    char buf[128];
    POINTL pt = { 10, 5 };

    if (s->monitoring)
        snprintf(buf, sizeof(buf), "Monitoring Enabled           Disk saving %s  ",
                s->fTrace ? "Enabled" : "Disabled");
    else
        snprintf(buf, sizeof(buf), "Closed. Rx Chars = %5lu, Tx Chars = %5lu.  %10lu bps",
                s->rxChars, s->txChars, s->rxBps);

    GpiCharStringAt(hps, &pt, (LONG)strlen(buf), (PCH)buf);
}


/* ---- Add line to display buffer ---- */

static void add_line(PMLM_STATE *s, const char *line)
{
    if (s->lineCount < MAX_LINES) {
        strncpy(s->lines[s->lineCount], line, LINE_LEN - 1);
        s->lines[s->lineCount][LINE_LEN - 1] = '\0';
        s->lineCount++;
    } else {
        /* Scroll: shift up, add at bottom */
        memmove(s->lines[0], s->lines[1], (MAX_LINES - 1) * LINE_LEN);
        strncpy(s->lines[MAX_LINES - 1], line, LINE_LEN - 1);
    }

    /* Write to trace file */
    if (s->fTrace)
        fprintf(s->fTrace, "%s\n", line);
}


/* ---- Query line control to compute actual bits-per-frame ---- */

static void refresh_frame_bits(PMLM_STATE *s)
{
    ULONG parmLen, dataLen;
    unsigned char lineCtrl[4] = {0};   /* databits, parity, stopbits, break */

    parmLen = 0; dataLen = sizeof(lineCtrl);
    if (DosDevIOCtl(s->hDev, IOCTL_ASYNC, ASYNC_GETLINECTRL,
                    NULL, 0, &parmLen, lineCtrl, sizeof(lineCtrl), &dataLen) != 0) {
        s->bitsPerFrame = 10;  /* Fallback: assume 8N1 if the query fails */
        return;
    }

    /* 1 start bit + data bits + (1 parity bit if any) + stop bits */
    s->bitsPerFrame = 1 + lineCtrl[0] + (lineCtrl[1] != 0 ? 1 : 0) +
                      (lineCtrl[2] == 2 ? 2 : 1);
}


/* ---- Query port state via IOCtl ---- */

static void poll_port(PMLM_STATE *s)
{
    ULONG parmLen, dataLen;
    unsigned char modem[4] = {0};
    unsigned long counts[2] = {0};
    DATETIME dt;

    if (!s->monitoring || s->hDev == 0) return;

    /* Get modem signals */
    parmLen = 0; dataLen = sizeof(modem);
    DosDevIOCtl(s->hDev, IOCTL_SIO2K, SIO2K_GETMODEM,
                NULL, 0, &parmLen, modem, sizeof(modem), &dataLen);

    s->prev_rts = s->rts; s->prev_cts = s->cts;
    s->prev_dtr = s->dtr; s->prev_dsr = s->dsr;
    s->prev_dcd = s->dcd; s->prev_ri  = s->ri;

    s->rts = (modem[0] & 0x02) ? SIG_ON : SIG_OFF;
    s->cts = (modem[0] & 0x10) ? SIG_ON : SIG_OFF;
    s->dtr = (modem[0] & 0x01) ? SIG_ON : SIG_OFF;
    s->dsr = (modem[0] & 0x20) ? SIG_ON : SIG_OFF;
    s->dcd = (modem[0] & 0x80) ? SIG_ON : SIG_OFF;
    s->ri  = (modem[0] & 0x40) ? SIG_ON : SIG_OFF;

    /* Log signal changes */
    if (s->rts != s->prev_rts || s->cts != s->prev_cts ||
        s->dtr != s->prev_dtr || s->dsr != s->prev_dsr ||
        s->dcd != s->prev_dcd || s->ri  != s->prev_ri) {
        char buf[80];
        DosGetDateTime(&dt);
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d  MSR  RTS=%d CTS=%d DTR=%d DSR=%d DCD=%d RI=%d",
                dt.hours, dt.minutes, dt.seconds,
                s->rts, s->cts, s->dtr, s->dsr, s->dcd, s->ri);
        add_line(s, buf);
    }

    /* Get byte counts */
    parmLen = 0; dataLen = sizeof(counts);
    DosDevIOCtl(s->hDev, IOCTL_SIO2K, SIO2K_GETCOUNT,
                NULL, 0, &parmLen, counts, sizeof(counts), &dataLen);

    s->rxChars = counts[0];
    s->txChars = counts[1];

    /* Calculate throughput */
    {
        unsigned long now;
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &now, sizeof(now));
        if (s->lastTick > 0 && (now - s->lastTick) >= 1000) {
            unsigned long elapsed = now - s->lastTick;
            unsigned long frameBits;
            refresh_frame_bits(s);
            frameBits = (unsigned long)s->bitsPerFrame;
            /* bits/sec = bytes/sec * bits-per-frame
             *          = (delta_bytes * 1000 / elapsed_ms) * bitsPerFrame */
            s->rxBps = ((s->rxChars - s->lastRx) * 1000 * frameBits) / elapsed;
            s->txBps = ((s->txChars - s->lastTx) * 1000 * frameBits) / elapsed;
            s->lastRx = s->rxChars;
            s->lastTx = s->txChars;
            s->lastTick = now;
        }
        if (s->lastTick == 0) {
            s->lastTick = now;
            s->lastRx = s->rxChars;
            s->lastTx = s->txChars;
        }
    }
}


/* ---- PM Window Procedure ---- */

static MRESULT EXPENTRY WndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {
    case WM_CREATE:
        WinStartTimer(g_hab, hwnd, TIMER_ID, TIMER_MS);
        return 0;

    case WM_TIMER:
        if (SHORT1FROMMP(mp1) == TIMER_ID) {
            poll_port(&g_state);
            WinInvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_PAINT:
    {
        HPS hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
        RECTL rcl;
        int i, y;

        WinQueryWindowRect(hwnd, &rcl);
        WinFillRect(hps, &rcl, CLR_WHITE);

        draw_signals(hps, &g_state);
        draw_status(hps, &g_state);

        /* Draw scrollback text */
        y = (int)rcl.yTop - 80;
        for (i = g_state.topLine; i < g_state.lineCount && y > 20; i++) {
            POINTL pt = { 10, y };
            GpiCharStringAt(hps, &pt, (LONG)strlen(g_state.lines[i]),
                           (PCH)g_state.lines[i]);
            y -= 14;
        }

        WinEndPaint(hps);
        return 0;
    }

    case WM_COMMAND:
        switch (SHORT1FROMMP(mp1)) {
        case IDM_CLEAR:
            g_state.lineCount = 0;
            g_state.topLine = 0;
            WinInvalidateRect(hwnd, NULL, FALSE);
            break;
        case IDM_STARTSTOP:
            g_state.monitoring = !g_state.monitoring;
            break;
        case IDM_DISKSAVE:
            if (!g_state.fTrace) {
                char fname[260];
                snprintf(fname, sizeof(fname), "PMLM_COM%d.TRC", g_state.portNum);
                g_state.fTrace = fopen(fname, "a");
            } else {
                fclose(g_state.fTrace);
                g_state.fTrace = NULL;
            }
            break;
        case IDM_EXIT:
            WinPostMsg(hwnd, WM_CLOSE, 0, 0);
            break;
        }
        return 0;

    case WM_CHAR:
    {
        USHORT flags = SHORT1FROMMP(mp1);
        USHORT ch = SHORT1FROMMP(mp2);
        USHORT vk = SHORT2FROMMP(mp2);
        int pageSize = 20;

        if (flags & KC_CHAR) {
            switch (ch) {
            case 'c': case 'C': WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_CLEAR), 0); break;
            case 's': case 'S': WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_STARTSTOP), 0); break;
            case 'x': case 'X': WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_EXIT), 0); break;
            case 'd': case 'D': WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_DISKSAVE), 0); break;
            }
        }

        /* Scrollback navigation — topLine was previously write-only
         * (reset on Clear, never advanced), so anything beyond one
         * screenful was unreachable. Wire it to the standard keys. */
        if (flags & KC_VIRTUALKEY) {
            switch (vk) {
            case VK_UP:
                if (g_state.topLine > 0) g_state.topLine--;
                break;
            case VK_DOWN:
                if (g_state.topLine < g_state.lineCount - 1) g_state.topLine++;
                break;
            case VK_PAGEUP:
                g_state.topLine = (g_state.topLine - pageSize > 0) ?
                                   g_state.topLine - pageSize : 0;
                break;
            case VK_PAGEDOWN:
                g_state.topLine = (g_state.topLine + pageSize < g_state.lineCount) ?
                                   g_state.topLine + pageSize : g_state.lineCount - 1;
                if (g_state.topLine < 0) g_state.topLine = 0;
                break;
            case VK_HOME:
                g_state.topLine = 0;
                break;
            case VK_END:
                g_state.topLine = (g_state.lineCount > pageSize) ?
                                   g_state.lineCount - pageSize : 0;
                break;
            }
            WinInvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_CLOSE:
        WinStopTimer(g_hab, hwnd, TIMER_ID);
        if (g_state.hDev) DosClose(g_state.hDev);
        if (g_state.fTrace) fclose(g_state.fTrace);
        WinPostMsg(hwnd, WM_QUIT, 0, 0);
        return 0;
    }

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


/* ================================================================
 * MAIN
 * ================================================================ */

int main(int argc, char *argv[])
{
    HMQ hmq;
    HWND hwndFrame;
    QMSG qmsg;
    ULONG fcf = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
                FCF_MINMAX | FCF_TASKLIST;
    ULONG action;
    APIRET rc;
    char title[64];

    printf("PMLM, Version V%s\n", VERSION);
    printf("GPLv3 — FPC264IRC Contributors, 2026\n");

    if (argc < 2) {
        printf("\n       Usage:   PMLM DEV <TraceFileName>\n");
        printf("       DEV is a Device Name or a comm port number (1 relative).\n");
        return 1;
    }

    /* Initialize state */
    memset(&g_state, 0, sizeof(g_state));

    /* Parse port argument */
    if (argv[1][0] >= '1' && argv[1][0] <= '9')
        g_state.portNum = atoi(argv[1]);
    else if (strnicmp(argv[1], "COM", 3) == 0)
        g_state.portNum = atoi(argv[1] + 3);
    else
        g_state.portNum = 1;

    /* Open trace file if specified */
    if (argc > 2) {
        g_state.fTrace = fopen(argv[2], "a");
        if (!g_state.fTrace) {
            printf("A problem occured when opening the disk file.\n");
            return 1;
        }
    }

    /* Open SIO device */
    rc = DosOpen("\\dev\\$sio$", &g_state.hDev, &action, 0,
                 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                 OPEN_ACCESS_READONLY, 0);
    if (rc != 0) {
        printf("SIO2K.SYS does not seem to be installed.\n");
        printf("The error code is %lu\n", rc);
        if (g_state.fTrace) fclose(g_state.fTrace);
        return 1;
    }

    g_state.monitoring = 1;

    /* Determine real vs. virtual (VMODEM) vs. ESP UART so the "Virt
     * UART" signal row and status text reflect the actual port type
     * instead of never being set. */
    {
        unsigned char uartType = 0;
        ULONG parmLen = 0, dataLen = sizeof(uartType);
        DosDevIOCtl(g_state.hDev, IOCTL_SIO2K, SIO2K_GETTYPE,
                    NULL, 0, &parmLen, &uartType, sizeof(uartType), &dataLen);
        g_state.isVirtual = (uartType == PHYS_UART_VMODEM) ? 1 : 0;
        g_state.isESP     = (uartType == PHYS_UART_ESP)    ? 1 : 0;
    }

    /* Check for DOS session */
    {
        PTIB ptib; PPIB ppib;
        DosGetInfoBlocks(&ptib, &ppib);
        if (ppib->pib_ultype != 3) { /* Not PM */
            printf("!This program cannot be run in a DOS session.\n");
            DosClose(g_state.hDev);
            if (g_state.fTrace) fclose(g_state.fTrace);
            return 1;
        }
    }

    /* Initialize PM */
    g_hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(g_hab, 0);

    WinRegisterClass(g_hab, "PMLM", WndProc, CS_SIZEREDRAW, 0);

    snprintf(title, sizeof(title), "PMLM — Port %d Monitor", g_state.portNum);
    hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
                                    &fcf, "PMLM", title,
                                    0, NULLHANDLE, ID_FRAME,
                                    &g_hwndClient);

    WinSetWindowPos(hwndFrame, NULLHANDLE, 50, 50, 600, 400,
                    SWP_SIZE | SWP_MOVE | SWP_SHOW);

    /* Message loop */
    while (WinGetMsg(g_hab, &qmsg, NULLHANDLE, 0, 0))
        WinDispatchMsg(g_hab, &qmsg);

    WinDestroyWindow(hwndFrame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(g_hab);

    return 0;
}
