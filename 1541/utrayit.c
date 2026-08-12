/*
 * utrayit.c — Cross-platform console tray/minimize for pcbis
 * Converted to C from utrayit.pas by sysop/0 (fpc264irc, GPL v3.0)
 *
 * Original: sysop/0 — fpc264irc/libs/utrayit/utrayit.pas
 * C port:   pcbrevival project
 *
 * Windows: system tray icon with background thread message pump.
 *          Win2000 through Win11 (XP-safe, no dynamic loading).
 * Unix:    XTWINOPS escape sequences (CSI 2 t / CSI 1 t).
 * DOS:     graceful stubs.
 */

#include "utrayit.h"
#include <string.h>

/* ================================================================== */
#if defined(_WIN32) || defined(_WIN64)
/* ================================================================== */

#include <windows.h>
#include <shellapi.h>

#define WM_TRAYCB (WM_APP + 1)

static int g_in_tray = 0;
static int g_minimized = 0;
static HWND g_helper_wnd = 0;
static HANDLE g_thread = 0;
static DWORD g_thread_id = 0;
static wchar_t g_tip[128];
static volatile int g_clicked = 0;
static int g_class_done = 0;
static UINT g_taskbar_msg = 0;

static HICON get_console_icon(void) {
    HICON ico;
    HWND cw;
    ico = LoadIconA(GetModuleHandle(NULL), "MAINICON");
    if (ico) return ico;
    cw = GetConsoleWindow();
    if (cw) {
        ico = (HICON)(LONG_PTR)GetClassLongPtrW(cw, GCLP_HICONSM);
        if (!ico) ico = (HICON)(LONG_PTR)GetClassLongPtrW(cw, GCLP_HICON);
    }
    if (!ico) ico = LoadIconA(NULL, IDI_APPLICATION);
    return ico;
}

static void helper_add_icon(void) {
    NOTIFYICONDATAW nid;
    if (!g_helper_wnd) return;
    memset(&nid, 0, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = g_helper_wnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYCB;
    nid.hIcon = get_console_icon();
    wcsncpy(nid.szTip, g_tip, sizeof(nid.szTip) / sizeof(nid.szTip[0]) - 1);
    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        Sleep(150);
        Shell_NotifyIconW(NIM_ADD, &nid);
    }
}

static LRESULT CALLBACK helper_wnd_proc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    NOTIFYICONDATAW nid;
    (void)wp;
    if (g_taskbar_msg && msg == g_taskbar_msg) {
        helper_add_icon();
        return 0;
    }
    switch (msg) {
        case WM_TRAYCB:
            if (LOWORD(lp) == WM_LBUTTONUP || LOWORD(lp) == WM_LBUTTONDBLCLK) {
                g_clicked = 1;
                PostMessage(h, WM_CLOSE, 0, 0);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(h);
            return 0;
        case WM_DESTROY:
            memset(&nid, 0, sizeof(nid));
            nid.cbSize = sizeof(NOTIFYICONDATAW);
            nid.hWnd = h;
            nid.uID = 1;
            Shell_NotifyIconW(NIM_DELETE, &nid);
            g_helper_wnd = 0;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

static DWORD WINAPI helper_thread(LPVOID param) {
    WNDCLASSEXW wc;
    MSG m;
    (void)param;
    if (!g_class_done) {
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = helper_wnd_proc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"UTrayItHelper";
        RegisterClassExW(&wc);
        g_class_done = 1;
    }
    g_taskbar_msg = RegisterWindowMessageW(L"TaskbarCreated");
    g_helper_wnd = CreateWindowExW(0, L"UTrayItHelper", L"UTrayItHelper",
                     WS_OVERLAPPED, 0, 0, 0, 0, 0, 0,
                     GetModuleHandle(NULL), NULL);
    if (!g_helper_wnd) return 0;
    helper_add_icon();
    while (GetMessageW(&m, 0, 0, 0)) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    if (g_clicked) {
        HWND cw = GetConsoleWindow();
        if (cw) {
            ShowWindow(cw, SW_SHOW);
            ShowWindow(cw, SW_RESTORE);
            SetForegroundWindow(cw);
        }
    }
    return 0;
}

int tray_console_supported(void) {
    return GetConsoleWindow() != 0;
}

int tray_supported(void) {
    return 1;
}

int tray_minimize(void) {
    HWND cw = GetConsoleWindow();
    if (!cw) return 0;
    ShowWindow(cw, SW_MINIMIZE);
    g_minimized = 1;
    return 1;
}

int tray_restore(void) {
    HWND cw = GetConsoleWindow();
    if (!cw) return 0;
    ShowWindow(cw, SW_SHOW);
    ShowWindow(cw, SW_RESTORE);
    SetForegroundWindow(cw);
    g_minimized = 0;
    return 1;
}

int tray_to_tray(const char *tip) {
    HWND cw;
    int i, len;
    if (!tip) return 0;
    if (g_in_tray) return 0;
    cw = GetConsoleWindow();
    if (!cw) return 0;
    memset(g_tip, 0, sizeof(g_tip));
    len = (int)strlen(tip);
    if (len > 127) len = 127;
    for (i = 0; i < len; i++)
        g_tip[i] = (wchar_t)(unsigned char)tip[i];
    g_clicked = 0;
    g_thread = CreateThread(NULL, 0, helper_thread, NULL, 0, &g_thread_id);
    if (!g_thread) return 0;
    ShowWindow(cw, SW_HIDE);
    g_in_tray = 1;
    return 1;
}

int tray_from_tray(void) {
    if (!g_in_tray) return 0;
    g_clicked = 0;
    if (g_helper_wnd)
        PostMessage(g_helper_wnd, WM_CLOSE, 0, 0);
    if (g_thread) {
        WaitForSingleObject(g_thread, 3000);
        CloseHandle(g_thread);
        g_thread = 0;
    }
    g_in_tray = 0;
    return tray_restore();
}

void tray_cleanup(void) {
    if (g_in_tray) tray_from_tray();
}

/* ================================================================== */
#elif defined(__unix__) || defined(__linux__) || defined(__APPLE__)
/* ================================================================== */

#include <unistd.h>
#include <fcntl.h>

static int g_in_tray = 0;
static int g_minimized = 0;

static int write_to_tty(const char *s) {
    int fd;
    ssize_t len, written;
    len = (ssize_t)strlen(s);
    fd = open("/dev/tty", O_WRONLY);
    if (fd >= 0) {
        written = write(fd, s, (size_t)len);
        close(fd);
        return written == len;
    }
    if (isatty(1)) {
        written = write(1, s, (size_t)len);
        return written == len;
    }
    return 0;
}

int tray_console_supported(void) {
    if (isatty(1)) return 1;
    return access("/dev/tty", W_OK) == 0;
}

int tray_supported(void) {
    return 0;
}

int tray_minimize(void) {
    if (!write_to_tty("\033[2t")) return 0;
    g_minimized = 1;
    return 1;
}

int tray_restore(void) {
    if (!write_to_tty("\033[1t")) return 0;
    g_minimized = 0;
    return 1;
}

int tray_to_tray(const char *tip) {
    (void)tip;
    if (g_in_tray) return 0;
    if (!tray_minimize()) return 0;
    g_in_tray = 1;
    return 1;
}

int tray_from_tray(void) {
    if (!g_in_tray) return 0;
    g_in_tray = 0;
    return tray_restore();
}

void tray_cleanup(void) {
    if (g_in_tray) tray_from_tray();
}

/* ================================================================== */
#else
/* DOS / go32v2 / OS2 / other — graceful stubs                        */
/* ================================================================== */

int tray_console_supported(void) { return 0; }
int tray_supported(void) { return 0; }
int tray_minimize(void) { return 0; }
int tray_restore(void) { return 0; }
int tray_to_tray(const char *tip) { (void)tip; return 0; }
int tray_from_tray(void) { return 0; }
void tray_cleanup(void) {}

#endif
