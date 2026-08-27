/* ====================================================================
 * wf_tsr_nt.c — WinFOSSIL NT Real-Mode Loader (WNFOSSIL.EXE)
 * ====================================================================
 * Tiny stub loaded from AUTOEXEC.NT. Tells NTVDM to load FOSSIL.DLL.
 *
 * Original was 1,901 bytes — a DOS .COM that calls:
 *   INT 2Fh AX=4B03h — NTVDM RegisterModule (load VDD DLL)
 *   Passes "FOSSIL.DLL" and dispatch function names.
 *
 * This C version compiles to a Win32 console app that does the same
 * thing but via LoadLibrary instead of real-mode INT 2Fh.
 * For actual AUTOEXEC.NT usage, the original 16-bit stub is needed.
 *
 * Build: gcc -o WNFOSSIL.EXE wf_tsr_nt.c -lkernel32
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>

#define VERSION "1.00.003"

int main(int argc, char *argv[])
{
    HMODULE hDll;
    typedef VOID (WINAPI *pfnRegInit)(VOID);
    pfnRegInit pRegInit;
    int uninstall = 0;

    printf("WinFOSSIL for Windows NT v%s (c) 2026 FPC264IRC Contributors\n", VERSION);

    /* Check for /INSTALL, /REMOVE, /UNINSTALL flags */
    if (argc > 1) {
        if (_stricmp(argv[1], "/UNINSTALL") == 0 ||
            _stricmp(argv[1], "/REMOVE") == 0) {
            uninstall = 1;
        }
    }

    /* Check we're on NT */
    {
        OSVERSIONINFOA osvi;
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        GetVersionExA(&osvi);
        if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) {
            fprintf(stderr, "ERROR: This product requires Windows NT\n");
            return 1;
        }
    }

    if (uninstall) {
        /* Uninstall: just report. Real cleanup done by FOSSIL.INF. */
        printf("WinFOSSIL uninstalled.\n");
        return 0;
    }

    /* Load FOSSIL.DLL and call VDDRegisterInit */
    hDll = LoadLibraryA("FOSSIL.DLL");
    if (!hDll) {
        /* Try System32 path */
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat(path, "\\FOSSIL.DLL");
        hDll = LoadLibraryA(path);
    }

    if (!hDll) {
        fprintf(stderr, "ERROR: failed to install Virtual Device Driver.\n");
        fprintf(stderr, "WinFOSSIL is not properly installed, "
                "please run the SETUP utility.\n");
        return 1;
    }

    pRegInit = (pfnRegInit)GetProcAddress(hDll, "VDDRegisterInit");
    if (pRegInit) {
        pRegInit();
        printf("WinFOSSIL VDD loaded and registered.\n");
    } else {
        fprintf(stderr, "ERROR: WinFOSSIL could not be registered.\n");
        FreeLibrary(hDll);
        return 1;
    }

    /* Keep the DLL loaded (don't FreeLibrary — it stays resident) */
    /* On a real AUTOEXEC.NT load, the COM stub would go TSR here. */

    return 0;
}

#endif /* _WIN32 */
