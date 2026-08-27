/* wf_tray.c — WinFOSSIL System Tray (WNFOSSIL.EXE for v2.0)
 * GPLv3 — FPC264IRC Contributors, 2026. */
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include "wf_core.h"

#define WM_TRAYICON (WM_USER + 1)
#define IDM_STATUS 40001
#define IDM_CONFIG 40002
#define IDM_COM1   40011
#define IDM_COM4   40014
#define IDM_VMODEM 40020
#define IDM_ABOUT  40030
#define IDM_EXIT   40099

extern void wf_compat_print_banner(void);

static WfPort g_ports[WF_MAX_PORTS];
static NOTIFYICONDATAA g_nid;
static HINSTANCE g_hInst;
static HWND g_hWnd;
static int g_enabled[WF_MAX_PORTS];

static void tray_add(HWND hw)
{
    memset(&g_nid, 0, sizeof(g_nid));
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hw;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(100));
    if (!g_nid.hIcon) g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strncpy(g_nid.szTip, "WinFOSSIL v" WF_VERSION_STR, 63);
    Shell_NotifyIconA(NIM_ADD, &g_nid);
}

static void toggle_port(int p)
{
    if (g_enabled[p]) {
        wf_deinit(&g_ports[p]);
        g_enabled[p] = 0;
    } else {
        wf_init(&g_ports[p], p);
        wf_open_com(&g_ports[p]);
        g_enabled[p] = 1;
    }
}

static LRESULT CALLBACK WndProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) { tray_add(hw); return 0; }
    if (msg == WM_DESTROY) { Shell_NotifyIconA(NIM_DELETE, &g_nid); PostQuitMessage(0); return 0; }
    if (msg == WM_TRAYICON && (lp == WM_RBUTTONUP || lp == WM_CONTEXTMENU)) {
        HMENU hm = CreatePopupMenu();
        POINT pt;
        int i;
        AppendMenuA(hm, MF_STRING, IDM_STATUS, "&Status...");
        AppendMenuA(hm, MF_SEPARATOR, 0, NULL);
        for (i = 0; i < WF_MAX_PORTS; i++) {
            char buf[16]; snprintf(buf, 16, "COM&%d", i+1);
            AppendMenuA(hm, MF_STRING | (g_enabled[i] ? MF_CHECKED : 0), IDM_COM1+i, buf);
        }
        AppendMenuA(hm, MF_SEPARATOR, 0, NULL);
        AppendMenuA(hm, MF_STRING, IDM_ABOUT, "&About");
        AppendMenuA(hm, MF_STRING, IDM_EXIT, "E&xit");
        GetCursorPos(&pt);
        SetForegroundWindow(hw);
        TrackPopupMenu(hm, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hw, NULL);
        DestroyMenu(hm);
        return 0;
    }
    if (msg == WM_COMMAND) {
        int id = LOWORD(wp);
        if (id >= IDM_COM1 && id <= IDM_COM4) toggle_port(id - IDM_COM1);
        else if (id == IDM_STATUS) {
            char buf[512]; int len = 0, i;
            len += snprintf(buf+len, sizeof(buf)-len, "WinFOSSIL v" WF_VERSION_STR "\n\n");
            for (i = 0; i < WF_MAX_PORTS; i++)
                len += snprintf(buf+len, sizeof(buf)-len, "COM%d: %s\n", i+1,
                    g_enabled[i] ? "active" : "disabled");
            MessageBoxA(hw, buf, "WinFOSSIL Status", MB_OK);
        }
        else if (id == IDM_ABOUT)
            MessageBoxA(hw, "WinFOSSIL v" WF_VERSION_STR "\nModern FOSSIL Driver\n"
                "Recreated from Bryan Woodruff's WinFOSSIL (1996)\nGPLv3", "About", MB_OK);
        else if (id == IDM_EXIT) DestroyWindow(hw);
        return 0;
    }
    return DefWindowProc(hw, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
    WNDCLASSA wc; MSG msg; int i;
    (void)hPrev; (void)cmd; (void)show;
    g_hInst = hInst;
    if (FindWindowA("WinFOSSIL_Tray", NULL)) { MessageBoxA(NULL, "Already running.", "WinFOSSIL", MB_OK); return 0; }
    for (i = 0; i < WF_MAX_PORTS; i++) { wf_init(&g_ports[i], i); g_enabled[i] = g_ports[i].cfg.enabled; }
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = "WinFOSSIL_Tray";
    RegisterClassA(&wc);
    g_hWnd = CreateWindowA("WinFOSSIL_Tray", "WinFOSSIL", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, hInst, NULL);
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    for (i = 0; i < WF_MAX_PORTS; i++) if (g_enabled[i]) wf_deinit(&g_ports[i]);
    return (int)msg.wParam;
}
#endif
