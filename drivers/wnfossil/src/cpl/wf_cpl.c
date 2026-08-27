/* wf_cpl.c — WinFOSSIL Control Panel Applet (WNFOSSIL.CPL)
 * Exports CPlApplet. Property Sheet with port config.
 * GPLv3 — FPC264IRC Contributors, 2026. */

#ifdef _WIN32
#include <windows.h>
#include <cpl.h>
#include <prsht.h>
#include <stdio.h>
#include "wf_core.h"

#define IDI_CPL_ICON 100
#define CPL_VERSION  "2.0.0"

extern int wfp_reg_read_port(int port_index, WfPortConfig *cfg);
extern int wfp_reg_write_port(int port_index, const WfPortConfig *cfg);

static HINSTANCE g_hCplInst;

static INT_PTR CALLBACK PortDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static WfPortConfig cfg[WF_MAX_PORTS];
    static int cur = 0;

    switch (msg) {
    case WM_INITDIALOG:
    {
        int i;
        char buf[32];
        for (i = 0; i < WF_MAX_PORTS; i++) {
            memset(&cfg[i], 0, sizeof(WfPortConfig));
            cfg[i].baud = 9600;
            cfg[i].rx_buf_size = WF_BUF_SIZE;
            cfg[i].tx_buf_size = WF_BUF_SIZE;
            wfp_reg_read_port(i, &cfg[i]);
            snprintf(buf, sizeof(buf), "COM%d", i + 1);
            SendDlgItemMessageA(hDlg, 1001, CB_ADDSTRING, 0, (LPARAM)buf);
        }
        SendDlgItemMessageA(hDlg, 1001, CB_SETCURSEL, 0, 0);

        { const char *bauds[] = {"300","1200","2400","4800","9600","19200","38400","57600","115200",NULL};
          int b; for (b = 0; bauds[b]; b++)
            SendDlgItemMessageA(hDlg, 1002, CB_ADDSTRING, 0, (LPARAM)bauds[b]);
        }
        SendDlgItemMessageA(hDlg, 1002, CB_SETCURSEL, 4, 0);

        CheckDlgButton(hDlg, 1010, cfg[0].auto_open ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, 1011, cfg[0].keep_open ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, 1012, cfg[0].timeslice ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, 1013, cfg[0].perf_stats ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, 1003, cfg[0].locked ? BST_CHECKED : BST_UNCHECKED);
        SetDlgItemInt(hDlg, 1004, cfg[0].rx_buf_size, FALSE);
        SetDlgItemInt(hDlg, 1005, cfg[0].tx_buf_size, FALSE);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == 1099) {
            cfg[cur].locked = IsDlgButtonChecked(hDlg, 1003) ? 1 : 0;
            cfg[cur].auto_open = IsDlgButtonChecked(hDlg, 1010) ? 1 : 0;
            cfg[cur].keep_open = IsDlgButtonChecked(hDlg, 1011) ? 1 : 0;
            cfg[cur].timeslice = IsDlgButtonChecked(hDlg, 1012) ? 1 : 0;
            cfg[cur].perf_stats = IsDlgButtonChecked(hDlg, 1013) ? 1 : 0;
            cfg[cur].rx_buf_size = GetDlgItemInt(hDlg, 1004, NULL, FALSE);
            cfg[cur].tx_buf_size = GetDlgItemInt(hDlg, 1005, NULL, FALSE);
            { int i; for (i = 0; i < WF_MAX_PORTS; i++) wfp_reg_write_port(i, &cfg[i]); }
            if (LOWORD(wParam) == IDOK) EndDialog(hDlg, IDOK);
        } else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
        } else if (LOWORD(wParam) == 1001 && HIWORD(wParam) == CBN_SELCHANGE) {
            cur = (int)SendDlgItemMessageA(hDlg, 1001, CB_GETCURSEL, 0, 0);
            CheckDlgButton(hDlg, 1003, cfg[cur].locked ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, 1010, cfg[cur].auto_open ? BST_CHECKED : BST_UNCHECKED);
            SetDlgItemInt(hDlg, 1004, cfg[cur].rx_buf_size, FALSE);
            SetDlgItemInt(hDlg, 1005, cfg[cur].tx_buf_size, FALSE);
        }
        return TRUE;
    }
    return FALSE;
}

LONG APIENTRY CPlApplet(HWND hWnd, UINT msg, LPARAM lp1, LPARAM lp2)
{
    switch (msg) {
    case CPL_INIT: return TRUE;
    case CPL_GETCOUNT: return 1;
    case CPL_NEWINQUIRE:
    {
        NEWCPLINFOA *info = (NEWCPLINFOA *)lp2;
        info->dwSize = sizeof(NEWCPLINFOA);
        info->dwFlags = 0;
        info->dwHelpContext = 0;
        info->lData = 0;
        info->hIcon = LoadIcon(g_hCplInst, MAKEINTRESOURCE(IDI_CPL_ICON));
        if (!info->hIcon) info->hIcon = LoadIcon(NULL, IDI_APPLICATION);
        strncpy(info->szName, "WinFOSSIL", sizeof(info->szName));
        strncpy(info->szInfo, "Configure WinFOSSIL FOSSIL driver ports", sizeof(info->szInfo));
        info->szHelpFile[0] = '\0';
        return 0;
    }
    case CPL_DBLCLK:
        DialogBoxA(g_hCplInst, "IDD_PORTCFG", hWnd, PortDlgProc);
        return 0;
    case CPL_STOP: return 0;
    case CPL_EXIT: return 0;
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) g_hCplInst = hInst;
    return TRUE;
}
#endif
