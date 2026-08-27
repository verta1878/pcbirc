/* ====================================================================
 * wf_dll_unified.c — WinFOSSIL Unified DLL Entry Point
 * ====================================================================
 * Single source file builds FOSSIL.DLL for all platforms:
 *
 *   WF_TARGET_MODERN  (default)  Win32/x64 DLL for XP through Win11
 *   WF_TARGET_WIN98              Win98/ME DLL (ANSI only, no NTVDM)
 *   WF_TARGET_VDD               NT4/2000 VDD (NTVDM INT 14h hook)
 *
 * Build:
 *   Modern i386:
 *     gcc -DWF_TARGET_MODERN -shared -o FOSSIL.DLL wf_dll_unified.c
 *         wf_core.c registry_compat.c comport_compat.c tcp_compat.c
 *         thread_compat.c -lws2_32 -ladvapi32
 *
 *   Modern x64:
 *     gcc -DWF_TARGET_MODERN -shared -m64 -o FOSSIL.DLL ...
 *
 *   Win98:
 *     gcc -DWF_TARGET_WIN98 -shared -o FOSSIL.DLL wf_dll_unified.c
 *         wf_core.c registry_compat.c comport_compat.c tcp_compat.c
 *         thread_compat.c -lws2_32 -ladvapi32
 *
 *   NT VDD:
 *     cl -DWF_TARGET_VDD -LD wf_dll_unified.c wf_core.c ...
 *         /link ntvdm.lib
 *
 * All three targets link the same wf_core.c engine. The only
 * differences are the entry points and platform-specific init.
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include "wf_core.h"

/* Default to modern if no target specified */
#if !defined(WF_TARGET_MODERN) && !defined(WF_TARGET_WIN98) && \
    !defined(WF_TARGET_VDD)
#define WF_TARGET_MODERN
#endif

/* Human-readable platform name, returned by commGetPlatform(). */
#if defined(WF_TARGET_WIN98)
#  define WF_PLATFORM_NAME "Windows 9x"
#elif defined(WF_TARGET_VDD)
#  define WF_PLATFORM_NAME "NT VDD"
#elif defined(_WIN64)
#  define WF_PLATFORM_NAME "Win64"
#else
#  define WF_PLATFORM_NAME "Win32"
#endif

/* ====================================================================
 * Shared port array and initialization — used by all targets
 * ==================================================================== */

static WfPort g_ports[WF_MAX_PORTS];
static int g_init_done = 0;

static void wf_dll_init_ports(void)
{
    int i;
    if (g_init_done) return;
    for (i = 0; i < WF_MAX_PORTS; i++)
        wf_init(&g_ports[i], i);
    g_init_done = 1;
}

static void wf_dll_deinit_ports(void)
{
    int i;
    for (i = 0; i < WF_MAX_PORTS; i++)
        if (g_ports[i].active)
            wf_deinit(&g_ports[i]);
}

/* ====================================================================
 * SECTION 1: DLL Main — entry point for all targets
 * ==================================================================== */

#if defined(WF_TARGET_MODERN) || defined(WF_TARGET_WIN98)

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    (void)hInst; (void)reserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        wf_dll_init_ports();
#ifdef WF_TARGET_WIN98
        wfp_log("WinFOSSIL v2.0.0 loaded (Win98 mode)");
#else
        wfp_log("WinFOSSIL v2.0.0 loaded (Modern mode)");
#endif
        break;
    case DLL_PROCESS_DETACH:
        wf_dll_deinit_ports();
        { extern void wfp_tcp_cleanup(void); wfp_tcp_cleanup(); }
        break;
    }
    return TRUE;
}

#endif /* MODERN || WIN98 */

/* ====================================================================
 * SECTION 2: Exported FOSSIL comm* API — Modern + Win98
 * ==================================================================== */

#if defined(WF_TARGET_MODERN) || defined(WF_TARGET_WIN98) || defined(WF_TARGET_VDD)

#define FOSSIL_EXPORT __declspec(dllexport) __stdcall

/* ---- Core FOSSIL functions ---- */

int FOSSIL_EXPORT commOpenPort(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    wf_dll_init_ports();
    return wf_open_com(&g_ports[port]);
}

void FOSSIL_EXPORT commClosePort(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_deinit(&g_ports[port]);
}

int FOSSIL_EXPORT commReadCharWait(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    return wf_recv_wait(&g_ports[port]);
}

int FOSSIL_EXPORT commWriteCharWait(int port, int ch)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    return wf_send_wait(&g_ports[port], (uint8_t)ch);
}

int FOSSIL_EXPORT commWriteCharNoWait(int port, int ch)
{
    if (port < 0 || port >= WF_MAX_PORTS) return 0;
    return wf_send_nowait(&g_ports[port], (uint8_t)ch);
}

int FOSSIL_EXPORT commPeekRecv(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    return wf_peek(&g_ports[port]);
}

int FOSSIL_EXPORT commReadBlock(int port, void *buf, int len)
{
    if (port < 0 || port >= WF_MAX_PORTS) return 0;
    return wf_read_block(&g_ports[port], buf, len);
}

int FOSSIL_EXPORT commWriteBlock(int port, const void *buf, int len)
{
    if (port < 0 || port >= WF_MAX_PORTS) return 0;
    return wf_write_block(&g_ports[port], buf, len);
}

void FOSSIL_EXPORT commFlushXmit(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_flush(&g_ports[port]);
}

int FOSSIL_EXPORT commGetStatus(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return 0;
    return wf_status(&g_ports[port]);
}

void FOSSIL_EXPORT commSetParams(int port, int params)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_set_params(&g_ports[port], (uint8_t)params);
}

void FOSSIL_EXPORT commSetParamsEx(int port, uint32_t baud,
                                    int parity, int data, int stop)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_set_params_ex(&g_ports[port], baud,
                     (uint8_t)parity, (uint8_t)data, (uint8_t)stop);
}

void FOSSIL_EXPORT commSetDTRState(int port, int on)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_set_dtr(&g_ports[port], on);
}

void FOSSIL_EXPORT commSetBreakState(int port, int on)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_set_break(&g_ports[port], on);
}

void FOSSIL_EXPORT commSetFlowCtrl(int port, int flags)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_set_flow(&g_ports[port], flags);
}

void FOSSIL_EXPORT commEtxHandler(int port, int flags)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_etx_handler(&g_ports[port], flags);
}

void FOSSIL_EXPORT commSetupDCB(int port, void *dcb)
{
    (void)port; (void)dcb;
}

int FOSSIL_EXPORT commIsPortEnabled(int port, int vm, int flags)
{
    (void)vm; (void)flags;
    if (port < 0 || port >= WF_MAX_PORTS) return 0;
    return g_ports[port].active;
}

void FOSSIL_EXPORT commReactivatePort(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    if (!g_ports[port].active) {
        wf_init(&g_ports[port], port);
        wf_open_com(&g_ports[port]);
    }
}

void FOSSIL_EXPORT commCleanupProcess(void)
{
    wf_dll_deinit_ports();
}

/* ---- Keyboard and screen (Fn 0Dh-0Eh, 11h-13h, 15h) ----
 * For DLL callers (modern + Win98), these use the console API
 * directly. The VDD path uses BIOS data area instead. */

int FOSSIL_EXPORT commKeyboardPeek(int port)
{
    INPUT_RECORD rec;
    DWORD avail;
    HANDLE hIn;
    (void)port;
    hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return -1;
    if (PeekConsoleInputA(hIn, &rec, 1, &avail) && avail > 0) {
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
            return (rec.Event.KeyEvent.wVirtualScanCode << 8) |
                    rec.Event.KeyEvent.uChar.AsciiChar;
    }
    return -1;  /* No key */
}

int FOSSIL_EXPORT commKeyboardRead(int port)
{
    INPUT_RECORD rec;
    DWORD read_count;
    HANDLE hIn;
    (void)port;
    hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return -1;
    while (1) {
        if (ReadConsoleInputA(hIn, &rec, 1, &read_count) && read_count > 0) {
            if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
                return (rec.Event.KeyEvent.wVirtualScanCode << 8) |
                        rec.Event.KeyEvent.uChar.AsciiChar;
        }
    }
}

void FOSSIL_EXPORT commSetCursorPos(int port, int row, int col)
{
    COORD pos;
    HANDLE hOut;
    (void)port;
    pos.X = (SHORT)col;
    pos.Y = (SHORT)row;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
        SetConsoleCursorPosition(hOut, pos);
}

int FOSSIL_EXPORT commGetCursorPos(int port)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut;
    (void)port;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(hOut, &csbi))
        return (csbi.dwCursorPosition.Y << 8) | csbi.dwCursorPosition.X;
    return 0;
}

void FOSSIL_EXPORT commWriteCharScreen(int port, int ch)
{
    HANDLE hOut;
    DWORD written;
    char c = (char)ch;
    (void)port;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
        WriteConsoleA(hOut, &c, 1, &written, NULL);
}

/* ---- VMODEM exports ---- */

int FOSSIL_EXPORT commVmodemEngine(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    wf_vm_engine(&g_ports[port]);
    return 0;
}

int FOSSIL_EXPORT commVmodemParseCmdStr(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    return wf_vm_parse_cmd(&g_ports[port]);
}

int FOSSIL_EXPORT commVmodemDial(int port, const char *addr)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    return wf_vm_dial(&g_ports[port], addr);
}

void FOSSIL_EXPORT commVmodemHangup(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_vm_hangup(&g_ports[port]);
}

void FOSSIL_EXPORT commVmodemConnectMsg(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_vm_connect_msg(&g_ports[port]);
}

int FOSSIL_EXPORT commVmodemFilter(int port, void *buf, int len)
{
    if (port < 0 || port >= WF_MAX_PORTS) return len;
    wf_vm_filter_telnet(&g_ports[port], (uint8_t *)buf, &len);
    return len;
}

void FOSSIL_EXPORT commVmodemEchoChar(int port, int ch)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_vm_echo(&g_ports[port], (uint8_t)ch);
}

void FOSSIL_EXPORT commVmodemSetState(int port, int state)
{
    if (port < 0 || port >= WF_MAX_PORTS) return;
    wf_vm_set_state(&g_ports[port], state);
}

int FOSSIL_EXPORT commVmodemStuffReadQ(int port, const void *buf, int len)
{
    if (port < 0 || port >= WF_MAX_PORTS) return 0;
    wf_vm_stuff_rx(&g_ports[port], buf, len);
    return len;
}

int FOSSIL_EXPORT commVmodemWaitForState(int port, int state, int ms)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;
    return wf_vm_wait_state(&g_ports[port], state, ms);
}

/* ---- Driver info ---- */
const char * FOSSIL_EXPORT commGetPlatform(void)
{
    return WF_PLATFORM_NAME;
}

#endif /* MODERN || WIN98 || VDD */


/* ====================================================================
 * SECTION 3: NT VDD (NTVDM) — INT 14h hook
 * ====================================================================
 * This section is compiled ONLY for WF_TARGET_VDD. It provides
 * the NTVDM entry points (VDDInitialize, VDDRegisterInit, etc.)
 * and the INT 14h dispatch that maps FOSSIL API calls to wf_core.
 *
 * The VDD path uses BIOS Data Area for keyboard/screen functions
 * because the DOS app accesses these through INT 14h, not through
 * Win32 console APIs.
 *
 * The full VDD implementation is in wf_vdd.c — it's kept separate
 * because it requires NTVDM headers and link libraries that are
 * not available on modern Windows.
 * ==================================================================== */

#ifdef WF_TARGET_VDD

/* VDD implementation lives in wf_vdd.c.
 * This section just provides the shared g_ports array.
 * wf_vdd.c extern-references g_ports and calls wf_core functions.
 *
 * Build: link wf_dll_unified.c + wf_vdd.c + wf_core.c together.
 * wf_vdd.c provides VDDInitialize/VDDRegisterInit/VDDI14Dispatch. */

/* Export g_ports for wf_vdd.c */
WfPort *wf_dll_get_ports(void) { return g_ports; }
int wf_dll_get_max_ports(void) { return WF_MAX_PORTS; }

/* VDD calls this on load */
void wf_dll_vdd_init(void)
{
    wf_dll_init_ports();
    wfp_log("WinFOSSIL v2.0.0 loaded (NT VDD mode)");
}

void wf_dll_vdd_deinit(void)
{
    wf_dll_deinit_ports();
    { extern void wfp_tcp_cleanup(void); wfp_tcp_cleanup(); }
}

#endif /* WF_TARGET_VDD */


/* ====================================================================
 * SECTION 4: Win98 VxD companion (Option B)
 * ====================================================================
 * When WF_TARGET_WIN98 is defined AND the VxD is loaded, the DLL
 * acts as a ring-3 companion. The VxD handles INT 14h and routes
 * VMODEM/TCP calls to this DLL via a DeviceIoControl interface.
 *
 * This provides the best of both worlds on Win98:
 *   - VxD does direct COM port I/O at ring-0 (fast, no API overhead)
 *   - DLL provides VMODEM/TCP/security at ring-3 (full wf_core engine)
 *
 * The VxD calls the DLL through a named pipe or shared memory region.
 * If the VxD is not loaded (no FOSSIL.VXD), the DLL works standalone
 * using Win32 COM APIs (same as Modern mode but ANSI-only).
 * ==================================================================== */

#ifdef WF_TARGET_WIN98

static BOOL g_vxd_present = FALSE;
static HANDLE g_hVxD = INVALID_HANDLE_VALUE;

/* Check if FOSSIL.VXD is loaded */
int FOSSIL_EXPORT commVxDDetect(void)
{
    g_hVxD = CreateFileA("\\\\.\\FOSSIL.VXD", 0, 0, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_hVxD != INVALID_HANDLE_VALUE) {
        g_vxd_present = TRUE;
        wfp_log("VxD companion: FOSSIL.VXD detected");
        return 1;
    }
    wfp_log("VxD companion: FOSSIL.VXD not found, standalone mode");
    return 0;
}

/* VxD forwards VMODEM commands to DLL via DeviceIoControl.
 * IOCTL codes for the pipe: */
#define WF_IOCTL_VMODEM_ENGINE   0x00010001
#define WF_IOCTL_VMODEM_DIAL     0x00010002
#define WF_IOCTL_VMODEM_HANGUP   0x00010003
#define WF_IOCTL_VMODEM_FILTER   0x00010004
#define WF_IOCTL_TCP_CONNECT     0x00010010
#define WF_IOCTL_TCP_DISCONNECT  0x00010011
#define WF_IOCTL_TCP_SEND        0x00010012
#define WF_IOCTL_TCP_RECV        0x00010013

/* Process IOCTL from VxD — called by VxD via W32_DeviceIOControl */
int FOSSIL_EXPORT commVxDIoCtl(DWORD ioctl, int port,
                                void *inbuf, int inlen,
                                void *outbuf, int outlen)
{
    if (port < 0 || port >= WF_MAX_PORTS) return -1;

    switch (ioctl) {
    case WF_IOCTL_VMODEM_ENGINE:
        wf_vm_engine(&g_ports[port]);
        return 0;

    case WF_IOCTL_VMODEM_DIAL:
        return wf_vm_dial(&g_ports[port], (const char *)inbuf);

    case WF_IOCTL_VMODEM_HANGUP:
        wf_vm_hangup(&g_ports[port]);
        return 0;

    case WF_IOCTL_VMODEM_FILTER:
        if (inbuf && inlen > 0) {
            int len = inlen;
            wf_vm_filter_telnet(&g_ports[port], (uint8_t *)inbuf, &len);
            return len;
        }
        return 0;

    case WF_IOCTL_TCP_CONNECT:
        return wf_vm_dial(&g_ports[port], (const char *)inbuf);

    case WF_IOCTL_TCP_DISCONNECT:
        wf_vm_hangup(&g_ports[port]);
        return 0;

    case WF_IOCTL_TCP_SEND:
        return wf_write_block(&g_ports[port], inbuf, inlen);

    case WF_IOCTL_TCP_RECV:
        return wf_read_block(&g_ports[port], outbuf, outlen);
    }

    return -1;
}

void FOSSIL_EXPORT commVxDCleanup(void)
{
    if (g_hVxD != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hVxD);
        g_hVxD = INVALID_HANDLE_VALUE;
    }
    g_vxd_present = FALSE;
}

#endif /* WF_TARGET_WIN98 */

#endif /* _WIN32 */
