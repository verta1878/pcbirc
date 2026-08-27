/* ====================================================================
 * registry_compat.c — Runtime Platform Detection + Registry Path Selection
 * ====================================================================
 * Same pattern as win32compat.pas from fpc264irc: detect Windows
 * version at startup, select correct registry paths, one codebase
 * compiles for all platforms (Win98 through Win11).
 *
 * Detection:
 *   Win95/98/ME    → WF_PLATFORM_9X   → VxD registry path
 *   NT4/2000       → WF_PLATFORM_NT   → Woodruff registry path
 *   XP/Vista/7-11  → WF_PLATFORM_MOD  → Modern registry path
 *
 * The platform wrapper does NOT need to be compiled separately.
 * One binary handles all versions. Registry paths, security
 * features, and API surface adapt at runtime.
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "wf_core.h"

/* ---- Platform IDs ---- */

#define WF_PLATFORM_UNKNOWN  0
#define WF_PLATFORM_9X       1      /* Win95, Win98, WinME           */
#define WF_PLATFORM_NT       2      /* NT4, Windows 2000             */
#define WF_PLATFORM_MOD      3      /* XP, Vista, 7, 8, 10, 11      */

/* ---- Detected state (set once at init) ---- */

static int g_platform = WF_PLATFORM_UNKNOWN;
static int g_winver_major = 0;
static int g_winver_minor = 0;
static int g_is_64bit = 0;
static char g_reg_base[256] = "";
static char g_reg_security[256] = "";
static char g_reg_uninstall[256] = "";
static char g_platform_name[64] = "";


/* ================================================================
 * PLATFORM DETECTION
 * ================================================================
 * GetVersionEx is deprecated on Win8.1+ but still works.
 * For Win10+ version detection, we also check the registry
 * (CurrentVersion\CurrentBuild) since GetVersionEx lies.
 *
 * We need to detect:
 *   9x kernel (Win95/98/ME)  → VER_PLATFORM_WIN32_WINDOWS
 *   NT kernel < 5.1          → NT4 (4.0), Win2000 (5.0)
 *   NT kernel >= 5.1         → XP through Win11
 * ================================================================ */

static void detect_platform(void)
{
    OSVERSIONINFOA osvi;
    SYSTEM_INFO si;
    HKEY hKey;
    char build[32] = "";
    DWORD size;

    if (g_platform != WF_PLATFORM_UNKNOWN) return;

    /* Get OS version */
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    g_winver_major = (int)osvi.dwMajorVersion;
    g_winver_minor = (int)osvi.dwMinorVersion;

    /* Check 32/64 bit */
    GetNativeSystemInfo(&si);
    g_is_64bit = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
                  si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);

    /* Get real build number for Win10+ (GetVersionEx lies) */
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        size = sizeof(build);
        RegQueryValueExA(hKey, "CurrentBuild", NULL, NULL,
                        (BYTE *)build, &size);
        RegCloseKey(hKey);
    }

    /* Classify platform */
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        /* Win95 (4.0), Win98 (4.10), WinME (4.90) */
        g_platform = WF_PLATFORM_9X;

        if (g_winver_minor >= 90)
            snprintf(g_platform_name, sizeof(g_platform_name), "Windows ME");
        else if (g_winver_minor >= 10)
            snprintf(g_platform_name, sizeof(g_platform_name), "Windows 98");
        else
            snprintf(g_platform_name, sizeof(g_platform_name), "Windows 95");

        strncpy(g_reg_base, WF_REG_95, sizeof(g_reg_base) - 1);
        strncpy(g_reg_uninstall, WF_UNREG_95, sizeof(g_reg_uninstall) - 1);
        g_reg_security[0] = '\0'; /* No security on 9x */

    } else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        if (g_winver_major <= 4) {
            /* NT 4.0 */
            g_platform = WF_PLATFORM_NT;
            snprintf(g_platform_name, sizeof(g_platform_name),
                     "Windows NT %d.%d", g_winver_major, g_winver_minor);
            strncpy(g_reg_base, WF_REG_NT, sizeof(g_reg_base) - 1);
            strncpy(g_reg_uninstall, WF_UNREG_NT, sizeof(g_reg_uninstall) - 1);
            g_reg_security[0] = '\0'; /* No security on NT4 */

        } else if (g_winver_major == 5 && g_winver_minor == 0) {
            /* Windows 2000 */
            g_platform = WF_PLATFORM_NT;
            snprintf(g_platform_name, sizeof(g_platform_name), "Windows 2000");
            strncpy(g_reg_base, WF_REG_NT, sizeof(g_reg_base) - 1);
            strncpy(g_reg_uninstall, WF_UNREG_NT, sizeof(g_reg_uninstall) - 1);
            g_reg_security[0] = '\0';

        } else {
            /* XP (5.1), Vista (6.0), 7 (6.1), 8 (6.2), 8.1 (6.3), 10/11 (10.0) */
            g_platform = WF_PLATFORM_MOD;

            if (g_winver_major >= 10) {
                int buildnum = atoi(build);
                if (buildnum >= 22000)
                    snprintf(g_platform_name, sizeof(g_platform_name),
                             "Windows 11 (build %s)%s", build,
                             g_is_64bit ? " x64" : "");
                else
                    snprintf(g_platform_name, sizeof(g_platform_name),
                             "Windows 10 (build %s)%s", build,
                             g_is_64bit ? " x64" : "");
            } else if (g_winver_major == 6) {
                const char *names[] = { "Vista", "7", "8", "8.1" };
                int idx = g_winver_minor;
                if (idx > 3) idx = 3;
                snprintf(g_platform_name, sizeof(g_platform_name),
                         "Windows %s%s", names[idx],
                         g_is_64bit ? " x64" : "");
            } else {
                snprintf(g_platform_name, sizeof(g_platform_name),
                         "Windows XP%s", g_is_64bit ? " x64" : "");
            }

            strncpy(g_reg_base, WF_REG_MODERN, sizeof(g_reg_base) - 1);
            strncpy(g_reg_uninstall, WF_UNREG_MOD, sizeof(g_reg_uninstall) - 1);
            strncpy(g_reg_security, WF_REG_SECURITY, sizeof(g_reg_security) - 1);
        }
    }
}


/* ================================================================
 * PUBLIC API
 * ================================================================ */

/* Get detected platform ID */
int wf_compat_platform(void)
{
    detect_platform();
    return g_platform;
}

/* Get platform display name */
const char *wf_compat_platform_name(void)
{
    detect_platform();
    return g_platform_name;
}

/* Get major.minor version */
void wf_compat_version(int *major, int *minor)
{
    detect_platform();
    if (major) *major = g_winver_major;
    if (minor) *minor = g_winver_minor;
}

/* Is 64-bit OS? */
int wf_compat_is_64bit(void)
{
    detect_platform();
    return g_is_64bit;
}

/* Can run VxD? (Win95/98/ME only) */
int wf_compat_has_vxd(void)
{
    detect_platform();
    return (g_platform == WF_PLATFORM_9X);
}

/* Can run VDD/NTVDM? (NT4/2000/XP 32-bit only) */
int wf_compat_has_ntvdm(void)
{
    detect_platform();
    if (g_platform == WF_PLATFORM_9X) return 0;
    if (g_is_64bit) return 0;       /* No NTVDM on x64              */
    if (g_winver_major >= 10) return 0; /* Removed in Win10          */
    return 1;
}

/* Has security subsystem? (v2.0 modern only) */
int wf_compat_has_security(void)
{
    detect_platform();
    return (g_platform == WF_PLATFORM_MOD);
}

/* Has UAC? (Vista+) */
int wf_compat_has_uac(void)
{
    detect_platform();
    return (g_platform == WF_PLATFORM_MOD && g_winver_major >= 6);
}


/* ================================================================
 * PLATFORM CALLBACK: wfp_reg_base_key
 * ================================================================
 * Returns the correct registry base key for the detected platform.
 * Called by wf_reg_path() in wf_core.c.
 * ================================================================ */

const char *wfp_reg_base_key(void)
{
    detect_platform();
    return g_reg_base;
}

/* Security registry key (empty on 9x/NT4) */
const char *wf_compat_security_key(void)
{
    detect_platform();
    return g_reg_security;
}

/* Uninstall registry key */
const char *wf_compat_uninstall_key(void)
{
    detect_platform();
    return g_reg_uninstall;
}


/* ================================================================
 * REGISTRY HELPERS
 * ================================================================
 * Read/write registry values using the detected base key.
 * These implement the wfp_reg_* callbacks from wf_core.h.
 * ================================================================ */

int wfp_reg_read_port(int port_index, WfPortConfig *cfg)
{
    HKEY hKey, hPort;
    char subkey[32];
    DWORD val, size;

    detect_platform();

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, g_reg_base,
                      0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return -1;

    /* Per-port subkey: "Port0", "Port1", etc. (modern)
     * or flat values with port prefix (9x/NT) */
    if (g_platform == WF_PLATFORM_MOD) {
        snprintf(subkey, sizeof(subkey), "Port%d", port_index);
        if (RegOpenKeyExA(hKey, subkey, 0, KEY_READ, &hPort) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return -1;
        }
    } else {
        /* 9x and NT store values flat under the main key */
        hPort = hKey;
    }

    /* Read values */
    size = sizeof(DWORD);
    if (RegQueryValueExA(hPort, WF_RV_ENABLED, NULL, NULL,
                        (BYTE *)&val, &size) == ERROR_SUCCESS)
        cfg->enabled = val;

    size = sizeof(cfg->name);
    RegQueryValueExA(hPort, WF_RV_PORT_NAME, NULL, NULL,
                    (BYTE *)cfg->name, &size);

    size = sizeof(DWORD);
    if (RegQueryValueExA(hPort, WF_RV_LOCKED_BAUD, NULL, NULL,
                        (BYTE *)&val, &size) == ERROR_SUCCESS) {
        if (val > 0) {
            cfg->locked = 1;
            cfg->baud = val;
        }
    }

    size = sizeof(DWORD);
    if (RegQueryValueExA(hPort, WF_RV_RXBUF_SIZE, NULL, NULL,
                        (BYTE *)&val, &size) == ERROR_SUCCESS)
        cfg->rx_buf_size = val;

    size = sizeof(DWORD);
    if (RegQueryValueExA(hPort, WF_RV_TXBUF_SIZE, NULL, NULL,
                        (BYTE *)&val, &size) == ERROR_SUCCESS)
        cfg->tx_buf_size = val;

    /* Modern-only values */
    if (g_platform == WF_PLATFORM_MOD) {
        size = sizeof(DWORD);
        if (RegQueryValueExA(hPort, WF_RV_AUTO_OPEN, NULL, NULL,
                            (BYTE *)&val, &size) == ERROR_SUCCESS)
            cfg->auto_open = val;
        size = sizeof(DWORD);
        if (RegQueryValueExA(hPort, WF_RV_KEEP_OPEN, NULL, NULL,
                            (BYTE *)&val, &size) == ERROR_SUCCESS)
            cfg->keep_open = val;
        size = sizeof(DWORD);
        if (RegQueryValueExA(hPort, WF_RV_TIMESLICE, NULL, NULL,
                            (BYTE *)&val, &size) == ERROR_SUCCESS)
            cfg->timeslice = val;
        size = sizeof(DWORD);
        if (RegQueryValueExA(hPort, WF_RV_PERF_STATS, NULL, NULL,
                            (BYTE *)&val, &size) == ERROR_SUCCESS)
            cfg->perf_stats = val;
    }

    if (g_platform == WF_PLATFORM_MOD && hPort != hKey)
        RegCloseKey(hPort);
    RegCloseKey(hKey);
    return 0;
}

int wfp_reg_write_port(int port_index, const WfPortConfig *cfg)
{
    HKEY hKey, hPort;
    char subkey[32];
    DWORD val, disp;

    detect_platform();

    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, g_reg_base,
                        0, NULL, 0, KEY_WRITE, NULL, &hKey, &disp) != ERROR_SUCCESS)
        return -1;

    if (g_platform == WF_PLATFORM_MOD) {
        snprintf(subkey, sizeof(subkey), "Port%d", port_index);
        if (RegCreateKeyExA(hKey, subkey, 0, NULL, 0, KEY_WRITE,
                            NULL, &hPort, &disp) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return -1;
        }
    } else {
        hPort = hKey;
    }

    val = cfg->enabled;
    RegSetValueExA(hPort, WF_RV_ENABLED, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));

    RegSetValueExA(hPort, WF_RV_PORT_NAME, 0, REG_SZ,
                  (BYTE *)cfg->name, (DWORD)strlen(cfg->name) + 1);

    val = cfg->locked ? cfg->baud : 0;
    RegSetValueExA(hPort, WF_RV_LOCKED_BAUD, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));

    val = cfg->rx_buf_size;
    RegSetValueExA(hPort, WF_RV_RXBUF_SIZE, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));

    val = cfg->tx_buf_size;
    RegSetValueExA(hPort, WF_RV_TXBUF_SIZE, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));

    if (g_platform == WF_PLATFORM_MOD) {
        val = cfg->auto_open;
        RegSetValueExA(hPort, WF_RV_AUTO_OPEN, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
        val = cfg->keep_open;
        RegSetValueExA(hPort, WF_RV_KEEP_OPEN, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
        val = cfg->timeslice;
        RegSetValueExA(hPort, WF_RV_TIMESLICE, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
        val = cfg->perf_stats;
        RegSetValueExA(hPort, WF_RV_PERF_STATS, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
    }

    /* v1.12 VxD-specific: write StaticVxD and Start */
    if (g_platform == WF_PLATFORM_9X) {
        RegSetValueExA(hKey, WF_RV_STATIC_VXD, 0, REG_SZ,
                      (BYTE *)"fossil.vxd", 11);
        val = 0;
        RegSetValueExA(hKey, WF_RV_START, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
    }

    /* v1.0 NT-specific: write Installed flag */
    if (g_platform == WF_PLATFORM_NT) {
        val = 1;
        RegSetValueExA(hKey, WF_RV_INSTALLED, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
    }

    if (g_platform == WF_PLATFORM_MOD && hPort != hKey)
        RegCloseKey(hPort);
    RegCloseKey(hKey);
    return 0;
}

int wfp_reg_read_global(const char *name, uint32_t *val)
{
    HKEY hKey;
    DWORD size = sizeof(DWORD);
    const char *key;

    detect_platform();

    /* Security values go under security key (modern only) */
    if (strcmp(name, WF_RV_REQUIRE_ADMIN) == 0 ||
        strcmp(name, WF_RV_LOG_ACCESS) == 0 ||
        strcmp(name, WF_RV_MAX_CONN) == 0) {
        if (g_platform != WF_PLATFORM_MOD) return -1;
        key = g_reg_security;
    } else {
        key = g_reg_base;
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return -1;

    if (RegQueryValueExA(hKey, name, NULL, NULL, (BYTE *)val, &size) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return -1;
    }

    RegCloseKey(hKey);
    return 0;
}

int wfp_reg_write_global(const char *name, uint32_t val)
{
    HKEY hKey;
    DWORD disp;
    const char *key;

    detect_platform();

    if (strcmp(name, WF_RV_REQUIRE_ADMIN) == 0 ||
        strcmp(name, WF_RV_LOG_ACCESS) == 0 ||
        strcmp(name, WF_RV_MAX_CONN) == 0) {
        if (g_platform != WF_PLATFORM_MOD) return -1;
        key = g_reg_security;
    } else {
        key = g_reg_base;
    }

    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, key, 0, NULL, 0,
                        KEY_WRITE, NULL, &hKey, &disp) != ERROR_SUCCESS)
        return -1;

    RegSetValueExA(hKey, name, 0, REG_DWORD, (BYTE *)&val, sizeof(DWORD));
    RegCloseKey(hKey);
    return 0;
}


/* ================================================================
 * STARTUP BANNER
 * ================================================================ */

void wf_compat_print_banner(void)
{
    detect_platform();

    fprintf(stderr, "WinFOSSIL v%s — Modern FOSSIL Driver\n", WF_VERSION_STR);
    fprintf(stderr, "Detected: %s\n", g_platform_name);
    fprintf(stderr, "Registry: %s\n", g_reg_base);

    if (g_platform == WF_PLATFORM_9X)
        fprintf(stderr, "Mode: VxD (FOSSIL.VXD)\n");
    else if (g_platform == WF_PLATFORM_NT && !g_is_64bit)
        fprintf(stderr, "Mode: VDD/NTVDM (FOSSIL.DLL)\n");
    else
        fprintf(stderr, "Mode: Native Win32%s\n", g_is_64bit ? "/x64" : "");

    if (wf_compat_has_security())
        fprintf(stderr, "Security: enabled (%s)\n", g_reg_security);
    if (wf_compat_has_uac())
        fprintf(stderr, "UAC: available\n");
    if (!wf_compat_has_ntvdm() && g_platform != WF_PLATFORM_9X)
        fprintf(stderr, "Note: NTVDM not available — DOS apps need DOSBox\n");
}

#endif /* _WIN32 */
