/* ====================================================================
 * viewpmlm.c — VIEWPMLM.EXE Trace File Viewer for OS/2 PM
 * ====================================================================
 * Replays PMLM trace files (.TRC) with timeline display.
 * Shows signal changes, data dumps, and statistics from saved traces.
 *
 * Usage: VIEWPMLM filename.TRC
 *
 * Original: 48,139 bytes NE (16-bit OS/2 PM), v2.00.
 * Clean-room reimplementation. GPLv3.
 * ==================================================================== */

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION     "2.00"
#define MAX_EVENTS  10000
#define EVENT_LEN   128

/* ---- Trace event ---- */

typedef struct {
    char    text[EVENT_LEN];
    int     type;           /* 0=signal, 1=data, 2=status, 3=error */
} TRACE_EVENT;

static TRACE_EVENT g_events[MAX_EVENTS];
static int         g_eventCount = 0;
static int         g_topEvent = 0;
static HAB         g_hab;
static HWND        g_hwndClient;
static char        g_filename[260] = "";


/* ---- Load trace file ---- */

static int load_trace(const char *filename)
{
    FILE *f;
    char line[256];

    f = fopen(filename, "r");
    if (!f) return -1;

    g_eventCount = 0;
    while (fgets(line, sizeof(line), f) && g_eventCount < MAX_EVENTS) {
        TRACE_EVENT *e = &g_events[g_eventCount];
        int len = (int)strlen(line);

        /* Strip trailing newline */
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;

        strncpy(e->text, line, EVENT_LEN - 1);
        e->text[EVENT_LEN - 1] = '\0';

        /* Classify event type by content */
        if (strstr(line, "MSR") || strstr(line, "LSR"))
            e->type = 0;       /* Signal change */
        else if (strstr(line, "TX>") || strstr(line, "<RX"))
            e->type = 1;       /* Data */
        else if (strstr(line, "---") || strstr(line, "INF"))
            e->type = 2;       /* Status */
        else
            e->type = 3;       /* Other */

        g_eventCount++;
    }

    fclose(f);
    return g_eventCount;
}


/* ---- Keep the frame's vertical scrollbar in sync with g_topEvent ---- */

static void update_scrollbar(HWND hwnd)
{
    HWND hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
    HWND hwndVScroll = WinWindowFromID(hwndFrame, FID_VERTSCROLL);
    if (!hwndVScroll) return;

    WinSendMsg(hwndVScroll, SBM_SETSCROLLBAR,
               MPFROMSHORT((SHORT)g_topEvent),
               MPFROM2SHORT(0, (SHORT)(g_eventCount > 0 ? g_eventCount - 1 : 0)));
}


/* ---- PM Window Procedure ---- */

static MRESULT EXPENTRY WndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {
    case WM_CREATE:
        update_scrollbar(hwnd);
        return 0;

    case WM_VSCROLL:
    {
        USHORT cmd = SHORT2FROMMP(mp2);
        int pageSize = 20;

        switch (cmd) {
        case SB_LINEUP:
            if (g_topEvent > 0) g_topEvent--;
            break;
        case SB_LINEDOWN:
            if (g_topEvent < g_eventCount - 1) g_topEvent++;
            break;
        case SB_PAGEUP:
            g_topEvent = (g_topEvent - pageSize > 0) ? g_topEvent - pageSize : 0;
            break;
        case SB_PAGEDOWN:
            g_topEvent = (g_topEvent + pageSize < g_eventCount) ?
                          g_topEvent + pageSize : g_eventCount - 1;
            break;
        case SB_SLIDERTRACK:
        case SB_SLIDERPOSITION:
            g_topEvent = (int)SHORT1FROMMP(mp2);
            break;
        }
        if (g_topEvent < 0) g_topEvent = 0;

        update_scrollbar(hwnd);
        WinInvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }

    case WM_PAINT:
    {
        HPS hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
        RECTL rcl;
        int i, y;
        LONG colors[] = { CLR_BLUE, CLR_BLACK, CLR_DARKGREEN, CLR_RED };

        WinQueryWindowRect(hwnd, &rcl);
        WinFillRect(hps, &rcl, CLR_WHITE);

        /* Header */
        {
            char hdr[128];
            POINTL pt = { 10, (int)rcl.yTop - 20 };
            snprintf(hdr, sizeof(hdr), "VIEWPMLM — %s  (%d events)", g_filename, g_eventCount);
            GpiSetColor(hps, CLR_BLACK);
            GpiCharStringAt(hps, &pt, (LONG)strlen(hdr), (PCH)hdr);
        }

        /* Event list */
        y = (int)rcl.yTop - 40;
        for (i = g_topEvent; i < g_eventCount && y > 5; i++) {
            POINTL pt = { 10, y };
            TRACE_EVENT *e = &g_events[i];

            GpiSetColor(hps, colors[e->type & 3]);
            GpiCharStringAt(hps, &pt, (LONG)strlen(e->text), (PCH)e->text);
            y -= 14;
        }

        WinEndPaint(hps);
        return 0;
    }

    case WM_CHAR:
    {
        USHORT flags = SHORT1FROMMP(mp1);
        USHORT vk = SHORT2FROMMP(mp2);
        if (flags & KC_VIRTUALKEY) {
            int pageSize = 20;
            if (vk == VK_DOWN && g_topEvent < g_eventCount - 1) g_topEvent++;
            if (vk == VK_UP && g_topEvent > 0) g_topEvent--;
            if (vk == VK_PAGEDOWN) g_topEvent = (g_topEvent + pageSize < g_eventCount) ?
                                                 g_topEvent + pageSize : g_eventCount - 1;
            if (vk == VK_PAGEUP) g_topEvent = (g_topEvent - pageSize > 0) ?
                                               g_topEvent - pageSize : 0;
            if (vk == VK_HOME) g_topEvent = 0;
            if (vk == VK_END) g_topEvent = (g_eventCount > pageSize) ?
                                            g_eventCount - pageSize : 0;
            update_scrollbar(hwnd);
            WinInvalidateRect(hwnd, NULL, FALSE);
        }
        if ((flags & KC_CHAR) && (SHORT1FROMMP(mp2) == 'x' || SHORT1FROMMP(mp2) == 'X'))
            WinPostMsg(hwnd, WM_CLOSE, 0, 0);
        return 0;
    }

    case WM_CLOSE:
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
                FCF_MINMAX | FCF_TASKLIST | FCF_VERTSCROLL;
    char title[128];
    int count;

    printf("VIEWPMLM, Version V%s\n", VERSION);
    printf("GPLv3 — FPC264IRC Contributors, 2026\n");

    if (argc < 2) {
        printf("\nViewPMLM filename\n");
        printf("Where 'filename' is the name of a trace file created by PMLM.\n");
        return 1;
    }

    strncpy(g_filename, argv[1], sizeof(g_filename) - 1);

    count = load_trace(g_filename);
    if (count < 0) {
        printf("A problem occured when opening the disk file.\n");
        return 1;
    }
    printf("Loaded %d events from %s\n", count, g_filename);

    /* Check for DOS session */
    {
        PTIB ptib; PPIB ppib;
        DosGetInfoBlocks(&ptib, &ppib);
        if (ppib->pib_ultype != 3) {
            printf("!This program cannot be run in a DOS session.\n");
            return 1;
        }
    }

    g_hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(g_hab, 0);

    WinRegisterClass(g_hab, "VIEWPMLM", WndProc, CS_SIZEREDRAW, 0);

    snprintf(title, sizeof(title), "VIEWPMLM — %s", g_filename);
    hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
                                    &fcf, "VIEWPMLM", title,
                                    0, NULLHANDLE, 100,
                                    &g_hwndClient);

    WinSetWindowPos(hwndFrame, NULLHANDLE, 80, 80, 700, 500,
                    SWP_SIZE | SWP_MOVE | SWP_SHOW);

    while (WinGetMsg(g_hab, &qmsg, NULLHANDLE, 0, 0))
        WinDispatchMsg(g_hab, &qmsg);

    WinDestroyWindow(hwndFrame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(g_hab);

    return 0;
}
