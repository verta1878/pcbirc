/* ====================================================================
 * wf_vdd.c — WinFOSSIL NT VDD Layer (FOSSIL.DLL for NT4/2000)
 * ====================================================================
 * Virtual Device Driver for NTVDM. Hooks INT 14h from DOS apps.
 *
 * NTVDM VDD API:
 *   VDDInitialize()   — DLL entry, called by NTVDM loader
 *   VDDRegisterInit()  — registers our INT 14h hook
 *   VDDI14Dispatch()   — INT 14h handler (FOSSIL API)
 *   VDDI2FDispatch()   — INT 2Fh handler (FOSSIL detect)
 *
 * Register accessors (NTVDM exports these):
 *   getAH/getAL/getAX/getBX/getCX/getDX/getDL/getDI/getES/getEAX
 *   setAX/setBH/setBL/setCF/setCX/setDX
 *
 * Only works on 32-bit NT/2000/XP (NTVDM removed from x64).
 *
 * Adapted from netmodem2irc NM_Int14ISR.pas (GPLv3).
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include "wf_core.h"

/* Structured Exception Handling (SEH) compatibility.
 * The BIOS Data Area accesses below use __try/__except to guard against
 * access faults. Real SEH is MSVC-only; the production NT DDK build uses
 * cl.exe where these are native. Under mingw/gcc (used for CI compile
 * checks) we map them to plain blocks — the BDA reads are safe under
 * NTVDM in practice, and the guards are belt-and-suspenders. */
#ifndef _MSC_VER
#  define __try      if (1)
#  define __except(x) if (0)
   /* The BIOS Data Area lives at the fixed linear address 0x400 under
    * NTVDM. gcc's static analysis flags this as a write near null; it is
    * intentional and correct on the real target, so silence it for the
    * cross-compile CI check. */
#  pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

/* ---- NTVDM VDD API imports ----
 * These are exported by NTVDM.EXE and resolved at load time.
 * On systems without NTVDM (x64, Win10+), the DLL simply
 * won't load — which is correct behavior. */

typedef BYTE  (WINAPI *pfnGetAH)(VOID);
typedef BYTE  (WINAPI *pfnGetAL)(VOID);
typedef WORD  (WINAPI *pfnGetAX)(VOID);
typedef WORD  (WINAPI *pfnGetBX)(VOID);
typedef WORD  (WINAPI *pfnGetCX)(VOID);
typedef WORD  (WINAPI *pfnGetDX)(VOID);
typedef BYTE  (WINAPI *pfnGetDL)(VOID);
typedef WORD  (WINAPI *pfnGetDI)(VOID);
typedef WORD  (WINAPI *pfnGetES)(VOID);
typedef DWORD (WINAPI *pfnGetEAX)(VOID);
typedef WORD  (WINAPI *pfnGetSP)(VOID);
typedef WORD  (WINAPI *pfnGetSS)(VOID);
typedef VOID  (WINAPI *pfnSetAX)(WORD);
typedef VOID  (WINAPI *pfnSetBH)(BYTE);
typedef VOID  (WINAPI *pfnSetBL)(BYTE);
typedef VOID  (WINAPI *pfnSetCF)(WORD);
typedef VOID  (WINAPI *pfnSetCX)(WORD);
typedef VOID  (WINAPI *pfnSetDX)(WORD);

static pfnGetAH  pGetAH;
static pfnGetAL  pGetAL;
static pfnGetAX  pGetAX;
static pfnGetBX  pGetBX;
static pfnGetCX  pGetCX;
static pfnGetDX  pGetDX;
static pfnGetDL  pGetDL;
static pfnGetDI  pGetDI;
static pfnGetES  pGetES;
static pfnGetEAX pGetEAX;
static pfnGetSP  pGetSP;
static pfnGetSS  pGetSS;
static pfnSetAX  pSetAX;
static pfnSetBH  pSetBH;
static pfnSetBL  pSetBL;
static pfnSetCF  pSetCF;
static pfnSetCX  pSetCX;
static pfnSetDX  pSetDX;

/* VDD hook function from NTVDM */
typedef BOOL (WINAPI *pfnVDDInstallIOHook)(HANDLE, WORD, WORD, PVOID, PVOID);
typedef VOID (WINAPI *pfnVDDDeInstallIOHook)(HANDLE, WORD, WORD);

static HANDLE g_hVdd = NULL;
static HMODULE g_hNtvdm = NULL;

/* Shared port table lives in wf_dll.c (unified build). The VDD and the
 * comm* exports must operate on the SAME ports, so we pull the table
 * from there rather than keeping a private copy. (Path A unification.) */
extern WfPort *wf_dll_get_ports(void);
extern void    wf_dll_vdd_init(void);
extern void    wf_dll_vdd_deinit(void);
#define g_ports (wf_dll_get_ports())

static int g_vdd_init = 0;

/* ---- Resolve NTVDM imports ---- */

static int resolve_ntvdm(void)
{
    g_hNtvdm = GetModuleHandleA("ntvdm.exe");
    if (!g_hNtvdm) return -1;

    #define RESOLVE(name) p##name = (pfn##name)GetProcAddress(g_hNtvdm, #name)

    RESOLVE(GetAH);  RESOLVE(GetAL);  RESOLVE(GetAX);
    RESOLVE(GetBX);  RESOLVE(GetCX);  RESOLVE(GetDX);
    RESOLVE(GetDL);  RESOLVE(GetDI);  RESOLVE(GetES);
    RESOLVE(GetEAX); RESOLVE(GetSP);  RESOLVE(GetSS);
    RESOLVE(SetAX);  RESOLVE(SetBH);  RESOLVE(SetBL);
    RESOLVE(SetCF);  RESOLVE(SetCX);  RESOLVE(SetDX);

    #undef RESOLVE

    if (!pGetAH || !pGetAL || !pSetAX) return -1;
    return 0;
}


/* ================================================================
 * INT 14h DISPATCH — FOSSIL API
 * ================================================================
 * Called by NTVDM when a DOS app executes INT 14h.
 * Reads registers via get* functions, dispatches to wf_core,
 * writes results back via set* functions.
 *
 * Matches original WinFOSSIL VDDI14Dispatch exactly.
 * ================================================================ */

static VOID WINAPI Int14Dispatch(VOID)
{
    BYTE ah, al;
    WORD dx, cx, di, es, ax;
    int port;
    WfPort *p;
    WfFossilInfo info;
    int result;

    ah = pGetAH();
    al = pGetAL();
    dx = pGetDX();

    port = dx;  /* DX = port index (0-based) */
    if (port < 0 || port >= WF_MAX_PORTS) return;
    p = &g_ports[port];

    switch (ah) {
    case 0x00:  /* Set baud rate */
        wf_set_params(p, al);
        pSetAX((WORD)wf_status(p));
        break;

    case 0x01:  /* Send character (wait) */
        result = wf_send_wait(p, al);
        pSetAX((WORD)wf_status(p));
        break;

    case 0x02:  /* Receive character (wait) */
        result = wf_recv_wait(p);
        if (result >= 0) {
            pSetAX((WORD)((wf_status(p) & 0xFF00) | (result & 0xFF)));
        } else {
            pSetAX((WORD)(wf_status(p) | 0x8000));  /* Timeout */
        }
        break;

    case 0x03:  /* Status request */
        pSetAX((WORD)wf_status(p));
        break;

    case 0x04:  /* Initialize FOSSIL */
        if (!p->active) wf_init(p, port);
        pSetAX(WF_SIGNATURE);       /* AX = 1954h */
        pSetBH(WF_SPEC_REV);        /* BH = spec revision */
        pSetBL(WF_MAX_FN);          /* BL = max function number */
        break;

    case 0x05:  /* Deinitialize FOSSIL */
        wf_deinit(p);
        break;

    case 0x06:  /* DTR control */
        wf_set_dtr(p, al ? 1 : 0);
        break;

    case 0x07:  /* Return timer tick parameters (FTS-0017).
                 * AL = ticks/second (18 on PC), DX:AX = current tick count
                 * from BIOS 0040:006C. BBS software uses this for timing. */
    {
        PBYTE bda = (PBYTE)(ULONG_PTR)0x400;
        __try {
            DWORD ticks = *(DWORD *)(bda + 0x6C);  /* 0040:006C */
            pSetAX((WORD)(ticks & 0xFFFF));
            pSetDX((WORD)((ticks >> 16) & 0xFFFF));
            /* AL overlaps AX low byte; set tick rate in a way that
             * preserves the count. Most callers read DX:AX for count
             * and treat 18.2 Hz as implied. */
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            pSetAX(0);
            pSetDX(0);
        }
        break;
    }

    case 0x08:  /* Flush output buffer */
        wf_flush(p);
        break;

    case 0x09:  /* Purge output buffer */
        wf_purge_tx(p);
        break;

    case 0x0A:  /* Purge input buffer */
        wf_purge_rx(p);
        break;

    case 0x0B:  /* Send character (no wait) */
        result = wf_send_nowait(p, al);
        pSetAX(result ? 1 : 0);
        break;

    case 0x0C:  /* Peek input (no wait) */
        result = wf_peek(p);
        if (result >= 0)
            pSetAX((WORD)((wf_status(p) & 0xFF00) | (result & 0xFF)));
        else
            pSetAX(0xFFFF);
        break;

    case 0x0D:  /* Keyboard peek — check if key available
                 * Returns: AX = character if available, 0xFFFF if not.
                 * Reads BIOS keyboard buffer head/tail at 0040:001A/001C.
                 * If head != tail, a key is waiting — peek without consuming. */
    {
        PBYTE bda = (PBYTE)(ULONG_PTR)0x400;  /* BIOS Data Area */
        __try {
            WORD head = *(WORD *)(bda + 0x1A);
            WORD tail = *(WORD *)(bda + 0x1C);
            if (head != tail) {
                /* Key available — read scancode+ASCII from buffer at head */
                WORD key = *(WORD *)(bda + head);
                pSetAX(key);
            } else {
                pSetAX(0xFFFF);  /* No key */
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            pSetAX(0xFFFF);
        }
        break;
    }

    case 0x0E:  /* Keyboard read — wait for and consume key
                 * Returns: AX = character (AL=ASCII, AH=scan code).
                 * Reads from BIOS keyboard buffer, advances head pointer.
                 * If no key, yields and retries. */
    {
        PBYTE bda = (PBYTE)(ULONG_PTR)0x400;
        int timeout = 10000;  /* Max iterations to prevent infinite loop */
        WORD key = 0xFFFF;
        __try {
            while (timeout-- > 0) {
                WORD head = *(WORD *)(bda + 0x1A);
                WORD tail = *(WORD *)(bda + 0x1C);
                if (head != tail) {
                    key = *(WORD *)(bda + head);
                    /* Advance head pointer, wrap at buffer end (0x3E) */
                    head += 2;
                    if (head >= 0x3E) head = 0x1E;  /* BIOS buffer: 0x1E-0x3D */
                    *(WORD *)(bda + 0x1A) = head;
                    break;
                }
                wfp_sleep_ms(1);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            key = 0xFFFF;
        }
        pSetAX(key);
        break;
    }

    case 0x0F:  /* Flow control */
        wf_set_flow(p, al);
        break;

    case 0x10:  /* Ctrl-C/K checking */
        wf_etx_handler(p, al);
        break;

    case 0x11:  /* Set cursor position — DH=row, DL=col
                 * Writes to BIOS cursor position at 0040:0050 (page 0). */
    {
        PBYTE bda = (PBYTE)(ULONG_PTR)0x400;
        WORD dx = pGetDX();
        __try {
            /* BDA cursor position: 0050h = col, 0051h = row (page 0) */
            *(bda + 0x50) = (BYTE)(dx & 0xFF);       /* DL = col */
            *(bda + 0x51) = (BYTE)((dx >> 8) & 0xFF); /* DH = row */
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            /* Ignore — can't write BDA */
        }
        break;
    }

    case 0x12:  /* Get cursor position — returns DH=row, DL=col */
    {
        PBYTE bda = (PBYTE)(ULONG_PTR)0x400;
        __try {
            BYTE col = *(bda + 0x50);
            BYTE row = *(bda + 0x51);
            pSetDX((WORD)((row << 8) | col));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            pSetDX(0);
        }
        break;
    }

    case 0x13:  /* Write character to screen with ANSI processing
                 * AL = character to write. Uses console output so
                 * ANSI sequences are processed by NTVDM's ANSI driver.
                 * This is the primary output path for BBS software. */
    {
        BYTE ch = al;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteConsoleA(hOut, &ch, 1, &written, NULL);
        }
        break;
    }

    case 0x14:  /* Watchdog control (FTS-0017).
                 * AL=0 disable, AL=1 enable. When enabled, the driver
                 * reboots the machine if carrier is lost (dangerous —
                 * we track state but do NOT actually reboot the host;
                 * on NTVDM a "reboot" only tears down the DOS session).
                 * BBS software sets this expecting the flag to persist. */
        p->watchdog = (al != 0) ? 1 : 0;
        wfp_log("Fn 14h: watchdog %s (port %d)",
                p->watchdog ? "enabled" : "disabled", p->index);
        break;

    case 0x15:  /* Write character to BIOS screen — raw, no ANSI processing
                 * AL = character. Same as Fn 13h but documented as bypassing
                 * ANSI. In NTVDM we use WriteConsoleA for both since the
                 * NTVDM console handles ANSI internally. BBS software
                 * typically uses Fn 13h for display. */
    {
        BYTE ch = al;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteConsoleA(hOut, &ch, 1, &written, NULL);
        }
        break;
    }

    case 0x18:  /* Read block */
    {
        cx = pGetCX();
        es = pGetES();
        di = pGetDI();
        if (cx > 0 && cx <= WF_BUF_SIZE) {
            uint8_t buf[WF_BUF_SIZE];
            int n = wf_read_block(p, buf, cx);
            /* Copy to DOS memory: flat addr = (ES << 4) + DI.
             * NTVDM maps the first 1MB of DOS memory into our
             * address space. GetVDMPointer resolves this but
             * is not always available — use direct mapping.
             * WF-4 fix: this was previously a TODO stub. */
            if (n > 0) {
                PBYTE dosAddr = (PBYTE)(ULONG_PTR)((es << 4) + di);
                __try {
                    memcpy(dosAddr, buf, n);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    n = 0;  /* Bad DOS pointer */
                }
            }
            pSetAX((WORD)n);
        } else {
            pSetAX(0);
        }
        break;
    }

    case 0x19:  /* Write block */
    {
        cx = pGetCX();
        es = pGetES();
        di = pGetDI();
        if (cx > 0 && cx <= WF_BUF_SIZE) {
            uint8_t buf[WF_BUF_SIZE];
            /* Read from DOS memory — WF-4 fix */
            PBYTE dosAddr = (PBYTE)(ULONG_PTR)((es << 4) + di);
            __try {
                memcpy(buf, dosAddr, cx);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                pSetAX(0);
                break;
            }
            int n = wf_write_block(p, buf, cx);
            pSetAX((WORD)n);
        } else {
            pSetAX(0);
        }
        break;
    }

    case 0x1A:  /* Break signal */
        wf_set_break(p, al ? 1 : 0);
        break;

    case 0x1B:  /* Get FOSSIL driver info */
        wf_get_info(p, &info);
        cx = pGetCX();
        es = pGetES();
        di = pGetDI();
        if (cx > sizeof(WfFossilInfo)) cx = sizeof(WfFossilInfo);
        /* Copy info struct to DOS memory — WF-4 fix */
        {
            PBYTE dosAddr = (PBYTE)(ULONG_PTR)((es << 4) + di);
            __try {
                memcpy(dosAddr, &info, cx);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                /* Bad pointer */
            }
        }
        pSetAX((WORD)sizeof(WfFossilInfo));
        break;

    default:
        break;
    }
}


/* ================================================================
 * INT 2Fh DISPATCH — FOSSIL Detect
 * ================================================================
 * INT 2Fh AX=1600h: check if FOSSIL is installed.
 * Returns AX=00FFh if installed (standard FOSSIL detect).
 * ================================================================ */

static VOID WINAPI Int2FDispatch(VOID)
{
    WORD ax = pGetAX();

    if (ax == 0x1600) {
        /* FOSSIL detect: return AX=00FFh = installed */
        pSetAX(0x00FF);
    }
}


/* ================================================================
 * VDD ENTRY POINTS — exported by name
 * ================================================================ */

__declspec(dllexport) BOOL WINAPI VDDInitialize(
    HANDLE hVdd, DWORD dwReason, LPVOID lpReserved)
{
    (void)lpReserved;

    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hVdd = hVdd;
        DisableThreadLibraryCalls((HINSTANCE)hVdd);

        if (resolve_ntvdm() != 0) {
            wfp_log("VDD: NTVDM register functions not found");
            return FALSE;
        }

        /* Ports are initialized by the unified DLL. wf_dll_vdd_init()
         * sets up the shared port table for NTVDM mode. */
        wf_dll_vdd_init();

        g_vdd_init = 1;
        wfp_log("VDD: WinFOSSIL VDD initialized (handle=%p)", hVdd);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        wf_dll_vdd_deinit();
        g_vdd_init = 0;
    }

    return TRUE;
}

__declspec(dllexport) VOID WINAPI VDDRegisterInit(VOID)
{
    /* Called from WNFOSSIL.EXE (real-mode loader in AUTOEXEC.NT).
     * At this point NTVDM has loaded us and we can hook INT 14h. */
    wfp_log("VDD: VDDRegisterInit called — hooking INT 14h");

    /* Load port configs from registry */
    {
        int i;
        for (i = 0; i < WF_MAX_PORTS; i++) {
            if (g_ports[i].cfg.enabled)
                wf_open_com(&g_ports[i]);
        }
    }
}

__declspec(dllexport) VOID WINAPI VDDI14Dispatch(VOID)
{
    Int14Dispatch();
}

__declspec(dllexport) VOID WINAPI VDDI2FDispatch(VOID)
{
    Int2FDispatch();
}

__declspec(dllexport) VOID WINAPI VDDHook(HANDLE hVdd)
{
    g_hVdd = hVdd;
    wfp_log("VDD: VDDHook called (handle=%p)", hVdd);
}


/* ================================================================
 * WNFOSCTL API — also exported for WNFOSCTL.EXE to call
 * ================================================================ */

__declspec(dllexport) WORD WINAPI WfGetVersion(VOID)
{
    return (WF_VERSION_MAJ << 8) | WF_VERSION_MIN;
}

__declspec(dllexport) BOOL WINAPI WfLockPort(int port, DWORD baud)
{
    if (port < 0 || port >= WF_MAX_PORTS) return FALSE;
    g_ports[port].cfg.locked = 1;
    g_ports[port].cfg.baud = baud;
    wfp_reg_write_port(port, &g_ports[port].cfg);
    wfp_log("VDD: Port %d locked at %lu baud", port, (unsigned long)baud);
    return TRUE;
}

__declspec(dllexport) BOOL WINAPI WfUnlockPort(int port)
{
    if (port < 0 || port >= WF_MAX_PORTS) return FALSE;
    g_ports[port].cfg.locked = 0;
    wfp_reg_write_port(port, &g_ports[port].cfg);
    wfp_log("VDD: Port %d unlocked", port);
    return TRUE;
}


/* ================================================================
 * comm* API exports come from the unified wf_dll.c.
 *
 * The NT build compiles BOTH wf_vdd.c and wf_dll.c together with
 * -DWF_NT_VDD. The VDD's INT 14h dispatch and the DLL's comm*
 * exports must share ONE port table, so we drop the VDD's private
 * g_ports here and pull the shared table from wf_dll.c via
 * wf_dll_get_ports(). This eliminates the 40 lines of duplicated
 * export wrappers that previously lived in this file. (Path A: one
 * codebase for all platforms including NT.)
 * ================================================================ */

#endif /* _WIN32 */
