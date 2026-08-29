/* ====================================================================
 * cyinstall.c — Cyclades Driver Installer / Uninstaller
 * ====================================================================
 * A standalone console utility that installs or removes the Cyclades
 * Cyclom-Y driver package on Windows 2000 through Windows 11.
 *
 * This is NOT a kernel component — it's a user-mode application
 * built with the standard Windows SDK.
 *
 * CRITICAL: The uninstaller is MORE IMPORTANT than the installer.
 * A driver that can't be uninstalled can brick a machine. This tool
 * provides multiple fallback removal methods including registry
 * cleanup that works even when the driver won't load.
 *
 * Usage:
 *   cyinstall /install [/testsign] [/silent] [/verbose]
 *   cyinstall /uninstall [/force] [/removecert] [/silent]
 *   cyinstall /status
 *
 * Build:
 *   cl cyinstall.c /Fe:cyinstall.exe setupapi.lib newdev.lib
 *   advapi32.lib cfgmgr32.lib
 *
 * License: GPLv3
 * ====================================================================
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <newdev.h>         /* UpdateDriverForPlugAndPlayDevices     */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ====================================================================
 * Constants
 * ==================================================================== */

/* Version — displayed in header and /status */
#define CY_VERSION          "1.0.0"

/* Our driver's service name — must match the INF [Services] section */
#define CY_SERVICE_NAME     "cyport"

/* Display name shown in Device Manager and Services */
#define CY_DISPLAY_NAME     "Cyclades Cyclom-Y Serial Port Driver"

/* PCI hardware ID — must match the INF */
#define CY_HARDWARE_ID      "PCI\\VEN_120E&DEV_0101"

/* Driver binary filename */
#define CY_DRIVER_FILE      "cyport.sys"

/* INF filename */
#define CY_INF_FILE         "cyclom-y.inf"

/* Test certificate filename */
#define CY_CERT_FILE        "cytest.cer"

/* Certificate store names */
#define CY_CERT_STORE_ROOT  "Root"
#define CY_CERT_STORE_PUB   "TrustedPublisher"

/* ====================================================================
 * Debug Output
 * ====================================================================
 * CyPrint always outputs. CyDebug outputs only when /verbose.
 * CyError prefixes with "ERROR: " for visibility.
 * ==================================================================== */

static int g_verbose = 0;
static int g_silent  = 0;

/* GNU C (MinGW cross-compiler) requires ##__VA_ARGS__ to handle
 * zero variadic arguments. MSVC accepts __VA_ARGS__ with zero args
 * but GCC does not. The ## prefix removes the trailing comma when
 * the variadic part is empty. Both compilers accept ##__VA_ARGS__. */

#define CyPrint(fmt, ...)                                              \
    do {                                                               \
        if (!g_silent)                                                 \
            printf("[CYINSTALL] " fmt, ##__VA_ARGS__);                 \
    } while (0)

#define CyDebug(fmt, ...)                                              \
    do {                                                               \
        if (g_verbose && !g_silent)                                    \
            printf("[CYINSTALL DBG] " fmt, ##__VA_ARGS__);             \
    } while (0)

#define CyError(fmt, ...)                                              \
    do {                                                               \
        fprintf(stderr, "[CYINSTALL ERROR] " fmt, ##__VA_ARGS__);      \
    } while (0)


/* ====================================================================
 * IsElevated — Check if running as Administrator
 * ====================================================================
 * Driver installation requires admin privileges. We check before
 * doing anything so the user gets a clear error message instead
 * of random access-denied failures midway through install. */

static BOOL IsElevated(void)
{
    BOOL isAdmin = FALSE;
    HANDLE token = NULL;

    /* On Vista+, UAC means "Administrator" isn't necessarily elevated.
     * We check TokenElevation. On Win2K/XP, there's no UAC — any
     * Administrator account is fully privileged. We check group
     * membership instead.
     *
     * Strategy: Try TokenElevation first. If it fails (Win2K/XP
     * doesn't support it), fall back to checking if the user is
     * in the Administrators group. (Audit B1 fix) */

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        /* Try Vista+ TokenElevation first */
        TOKEN_ELEVATION elev;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation,
                                &elev, sizeof(elev), &size)) {
            /* Vista+ — TokenElevation worked */
            isAdmin = elev.TokenIsElevated;
        } else {
            /* Win2K/XP — TokenElevation not supported.
             * Check if user is in the Administrators group.
             * SID for BUILTIN\Administrators: S-1-5-32-544 */
            SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
            PSID adminSid = NULL;

            if (AllocateAndInitializeSid(&ntAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0, &adminSid)) {
                /* CheckTokenMembership is available on Win2K+ */
                if (!CheckTokenMembership(NULL, adminSid, &isAdmin)) {
                    isAdmin = FALSE;
                }
                FreeSid(adminSid);
            }
        }
        CloseHandle(token);
    }

    return isAdmin;
}


/* ====================================================================
 * StopDriverService — Stop the cyport service if running
 * ====================================================================
 * Sends SERVICE_CONTROL_STOP to the service. Waits up to 10 seconds
 * for it to stop. Returns TRUE if stopped or already stopped. */

static BOOL StopDriverService(void)
{
    SC_HANDLE scm, svc;
    SERVICE_STATUS svcStatus;
    BOOL result = FALSE;
    int retries;

    CyDebug("Opening Service Control Manager...\n");

    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        CyDebug("OpenSCManager failed: %lu\n", GetLastError());
        return FALSE;
    }

    svc = OpenServiceA(scm, CY_SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!svc) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
            CyDebug("Service does not exist — nothing to stop\n");
            CloseServiceHandle(scm);
            return TRUE;    /* Not installed = already stopped      */
        }
        CyError("OpenService failed: %lu\n", err);
        CloseServiceHandle(scm);
        return FALSE;
    }

    /* Send stop command */
    CyPrint("Stopping %s service...\n", CY_SERVICE_NAME);
    if (!ControlService(svc, SERVICE_CONTROL_STOP, &svcStatus)) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_NOT_ACTIVE) {
            CyDebug("Service already stopped\n");
            result = TRUE;
        } else {
            CyError("ControlService STOP failed: %lu\n", err);
        }
    } else {
        /* Wait for service to actually stop — poll every 500ms */
        retries = 20;  /* 20 × 500ms = 10 seconds max */
        while (retries-- > 0) {
            if (QueryServiceStatus(svc, &svcStatus)) {
                if (svcStatus.dwCurrentState == SERVICE_STOPPED) {
                    CyPrint("Service stopped successfully\n");
                    result = TRUE;
                    break;
                }
                CyDebug("Service state: %lu, waiting...\n",
                         svcStatus.dwCurrentState);
            }
            Sleep(500);
        }
        if (!result) {
            CyError("Service did not stop within 10 seconds\n");
        }
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return result;
}


/* ====================================================================
 * DeleteDriverService — Remove the service registration
 * ==================================================================== */

static BOOL DeleteDriverService(void)
{
    SC_HANDLE scm, svc;
    BOOL result = FALSE;

    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) return FALSE;

    svc = OpenServiceA(scm, CY_SERVICE_NAME, DELETE);
    if (!svc) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
            CyDebug("Service does not exist — nothing to delete\n");
            CloseServiceHandle(scm);
            return TRUE;
        }
        CyError("OpenService for delete failed: %lu\n", err);
        CloseServiceHandle(scm);
        return FALSE;
    }

    CyPrint("Deleting %s service...\n", CY_SERVICE_NAME);
    if (DeleteService(svc)) {
        CyPrint("Service deleted (will be removed after reboot)\n");
        result = TRUE;
    } else {
        CyError("DeleteService failed: %lu\n", GetLastError());
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return result;
}


/* ====================================================================
 * DeleteDriverFile — Remove cyport.sys from system32\drivers
 * ==================================================================== */

static BOOL DeleteDriverFile(void)
{
    char path[MAX_PATH];
    UINT sysLen;

    sysLen = GetSystemDirectoryA(path, sizeof(path));
    if (sysLen == 0 || sysLen >= sizeof(path) - 30) {
        CyError("GetSystemDirectory failed\n");
        return FALSE;
    }

    strcat(path, "\\Drivers\\");
    strcat(path, CY_DRIVER_FILE);

    CyDebug("Deleting %s\n", path);

    if (DeleteFileA(path)) {
        CyPrint("Deleted %s\n", path);
        return TRUE;
    } else {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            CyDebug("File not found — already removed\n");
            return TRUE;
        }
        /* File may be locked if driver is still loaded.
         * Mark for deletion on next reboot. */
        if (MoveFileExA(path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
            CyPrint("File locked — marked for deletion on reboot\n");
            return TRUE;
        }
        CyError("Cannot delete %s: error %lu\n", path, err);
        return FALSE;
    }
}


/* ====================================================================
 * CleanRegistry — Remove driver registry entries
 * ==================================================================== */

static void CleanRegistry(void)
{
    LONG result;

    CyPrint("Cleaning registry...\n");

    /* Delete the service key and ALL subkeys recursively.
     * RegDeleteKeyA on Win2K/XP cannot delete keys with subkeys,
     * so we delete known subkeys first, then try the parent.
     *
     * Order: children first, then parent. (Audit B5 fix)
     * If unknown subkeys exist, the parent delete will fail —
     * this is acceptable. DeleteService + reboot will clean up
     * anything we miss. */

    /* Delete known subkeys first */
    result = RegDeleteKeyA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\cyport\\Parameters");
    CyDebug("Delete Parameters: %s\n",
             result == ERROR_SUCCESS ? "OK" :
             result == ERROR_FILE_NOT_FOUND ? "not found" : "failed");

    result = RegDeleteKeyA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\cyport\\Enum");
    CyDebug("Delete Enum: %s\n",
             result == ERROR_SUCCESS ? "OK" :
             result == ERROR_FILE_NOT_FOUND ? "not found" : "failed");

    result = RegDeleteKeyA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\cyport\\Security");
    CyDebug("Delete Security: %s\n",
             result == ERROR_SUCCESS ? "OK" :
             result == ERROR_FILE_NOT_FOUND ? "not found" : "failed");

    /* Now try the parent key — will succeed if all subkeys are gone */
    result = RegDeleteKeyA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\cyport");
    if (result == ERROR_SUCCESS) {
        CyPrint("Service registry key deleted\n");
    } else if (result == ERROR_FILE_NOT_FOUND) {
        CyDebug("Service key not found — already clean\n");
    } else {
        /* Subkeys remain — DeleteService + reboot will handle it */
        CyDebug("Service key has remaining subkeys — reboot will clean\n");
    }
}


/* ====================================================================
 * DoInstall — Install the driver package
 * ==================================================================== */

static int DoInstall(int testSign)
{
    char infPath[MAX_PATH];
    BOOL rebootNeeded = FALSE;  /* Track if reboot is required.
                                 * Kernel driver installs should ALWAYS
                                 * recommend a reboot — the driver may
                                 * not load until the next boot cycle,
                                 * especially if replacing an existing
                                 * driver. (Audit P2-B1 fix) */

    CyPrint("=== Cyclades Cyclom-Y Driver Installation ===\n");
    CyPrint("\n");

    /* Step 1: Check admin privileges */
    if (!IsElevated()) {
        CyError("Administrator privileges required.\n");
        CyError("Right-click and select 'Run as Administrator'.\n");
        return 1;
    }
    CyPrint("Running as Administrator — OK\n");

    /* Step 2: Find the INF file and verify it exists */
    if (!GetFullPathNameA(CY_INF_FILE, sizeof(infPath),
                          infPath, NULL)) {
        CyError("Cannot resolve path for %s\n", CY_INF_FILE);
        return 1;
    }

    /* GetFullPathNameA succeeds even if the file doesn't exist —
     * it just constructs the path. We must check that the file
     * actually exists, or SetupCopyOEMInf will give a confusing
     * error. (Audit B3 fix) */
    if (GetFileAttributesA(infPath) == INVALID_FILE_ATTRIBUTES) {
        CyError("INF file not found: %s\n", infPath);
        CyError("Make sure %s is in the current directory.\n", CY_INF_FILE);
        return 1;
    }
    CyPrint("INF file: %s\n", infPath);

    /* Verify that cyport.sys and cyyport.inf also exist.
     * SetupCopyOEMInf only preloads the INF — the .sys is copied
     * later by PnP. But if the .sys is missing, the device will
     * fail to start with a confusing error. Check now and fail
     * early with a clear message. (Audit P2-B3 fix) */
    if (GetFileAttributesA(CY_DRIVER_FILE) == INVALID_FILE_ATTRIBUTES) {
        CyError("Driver binary not found: %s\n", CY_DRIVER_FILE);
        CyError("Make sure %s is in the same directory as %s.\n",
                CY_DRIVER_FILE, CY_INF_FILE);
        return 1;
    }
    CyDebug("Driver binary: %s — found\n", CY_DRIVER_FILE);

    if (GetFileAttributesA("cyyport.inf") == INVALID_FILE_ATTRIBUTES) {
        CyError("Port INF not found: cyyport.inf\n");
        CyError("Make sure cyyport.inf is in the same directory.\n");
        return 1;
    }
    CyDebug("Port INF: cyyport.inf — found\n");

    /* Step 3: Install test certificate if requested */
    if (testSign) {
        char cmd[MAX_PATH + 64];

        CyPrint("\n--- Installing test certificate ---\n");

        /* Verify certificate file exists */
        if (GetFileAttributesA(CY_CERT_FILE) == INVALID_FILE_ATTRIBUTES) {
            CyError("Certificate file not found: %s\n", CY_CERT_FILE);
            CyError("Run MakeCert.exe first to create it.\n");
            return 1;
        }

        /* Add to TrustedPublisher store — allows driver loading */
        _snprintf(cmd, sizeof(cmd), "certutil -addstore TrustedPublisher \"%s\"", CY_CERT_FILE);
        CyPrint("Running: %s\n", cmd);
        if (system(cmd) != 0)
            CyError("certutil TrustedPublisher failed (may need manual install)\n");

        /* Add to Root store — establishes trust chain */
        _snprintf(cmd, sizeof(cmd), "certutil -addstore Root \"%s\"", CY_CERT_FILE);
        CyPrint("Running: %s\n", cmd);
        if (system(cmd) != 0)
            CyError("certutil Root failed (may need manual install)\n");

        /* Enable TESTSIGNING — only on Vista+ (bcdedit doesn't exist on XP) */
        CyPrint("Enabling TESTSIGNING mode...\n");
        if (system("bcdedit -set TESTSIGNING ON") != 0)
            CyPrint("bcdedit not available (Win2K/XP don't need it)\n");

        CyPrint("WARNING: Reboot required after enabling TESTSIGNING\n");
        rebootNeeded = TRUE;
    }

    /* Step 4: Copy INF to driver store.
     * SetupCopyOEMInf copies the INF (and any referenced files
     * via CopyFiles directives) to the driver store
     * (%SystemRoot%\INF\oem*.inf). This makes the driver
     * available for PnP matching. */
    CyPrint("\nCopying INF to driver store...\n");
    if (!SetupCopyOEMInfA(infPath, NULL, SPOST_PATH, 0,
                           NULL, 0, NULL, NULL)) {
        CyError("SetupCopyOEMInf failed: %lu\n", GetLastError());
        CyError("Make sure %s and %s are in the same directory.\n",
                CY_INF_FILE, CY_DRIVER_FILE);
        return 1;
    }
    CyPrint("INF installed to driver store\n");

    /* Step 5: Trigger PnP rescan.
     * If a Cyclom-Y card is present, PnP will detect it and
     * load our driver automatically. */
    CyPrint("\nScanning for Cyclom-Y hardware...\n");
    {
        DEVINST rootDev;
        CONFIGRET cr;

        cr = CM_Locate_DevNodeA(&rootDev, NULL, CM_LOCATE_DEVNODE_NORMAL);
        if (cr == CR_SUCCESS) {
            cr = CM_Reenumerate_DevNode(rootDev, CM_REENUMERATE_NORMAL);
            if (cr == CR_SUCCESS) {
                CyPrint("PnP rescan triggered\n");
            } else {
                CyError("CM_Reenumerate failed: %lu\n", cr);
            }
        }
    }

    /* A kernel driver was installed. Recommend reboot.
     * The driver may not load until PnP rescans at boot time,
     * especially if this is the first install or if an older
     * version was previously loaded. (Audit P2-B1 fix) */
    rebootNeeded = TRUE;

    CyPrint("\n=== Installation complete ===\n");
    CyPrint("If a Cyclom-Y card is installed, check Device Manager\n");
    CyPrint("for new COM ports under 'Ports (COM & LPT)'.\n");

    if (testSign || rebootNeeded) {
        CyPrint("\n*** REBOOT RECOMMENDED ***\n");
        CyPrint("The driver may not load until the system is restarted.\n");
        if (testSign)
            CyPrint("Test signing requires a reboot to take effect.\n");
    }

    return 0;
}


/* ====================================================================
 * DoUninstall — Remove the driver completely
 * ==================================================================== */

static int DoUninstall(int force, int removeCert)
{
    CyPrint("=== Cyclades Cyclom-Y Driver Removal ===\n");
    CyPrint("\n");

    /* Step 1: Check admin privileges */
    if (!IsElevated()) {
        CyError("Administrator privileges required.\n");
        return 1;
    }
    CyPrint("Running as Administrator — OK\n");

    /* Step 2: Stop the driver service */
    CyPrint("\n--- Step 1: Stop driver service ---\n");
    if (!StopDriverService() && !force) {
        CyError("Cannot stop service. Use /force to continue anyway.\n");
        return 1;
    }

    /* Step 3: Remove devices via SetupAPI.
     * This tells PnP to remove the device, which sends
     * IRP_MN_REMOVE_DEVICE to our driver.
     *
     * We scan TWICE: once for PCI bus devices (VEN_120E) and once
     * for child PDOs (Cyclom-Y\Port). Child devices must be removed
     * too, or their COM port numbers stay claimed in SERIALCOMM and
     * the port INF stays in the driver store. (Audit B4 fix) */
    CyPrint("\n--- Step 2: Remove devices ---\n");
    {
        HDEVINFO devInfo;
        SP_DEVINFO_DATA devData;
        DWORD idx;

        /* ---- Pass 1: Remove child port devices ----
         * These have hardware ID "Cyclom-Y\Port" and are NOT on the
         * PCI enumerator. We scan all devices (enumerator = NULL). */
        CyDebug("Scanning for child port devices...\n");
        devInfo = SetupDiGetClassDevsA(NULL, NULL, NULL,
                                        DIGCF_ALLCLASSES);
        if (devInfo != INVALID_HANDLE_VALUE) {
            devData.cbSize = sizeof(SP_DEVINFO_DATA);
            for (idx = 0;
                 SetupDiEnumDeviceInfo(devInfo, idx, &devData);
                 idx++) {
                char hwId[256];
                if (SetupDiGetDeviceRegistryPropertyA(
                        devInfo, &devData, SPDRP_HARDWAREID,
                        NULL, (PBYTE)hwId, sizeof(hwId), NULL)) {
                    if (strstr(hwId, "Cyclom-Y\\Port")) {
                        CyPrint("Found port device: %s\n", hwId);
                        if (SetupDiCallClassInstaller(
                                DIF_REMOVE, devInfo, &devData)) {
                            CyPrint("Port device removed\n");
                        } else {
                            CyDebug("Port removal failed: %lu\n",
                                     GetLastError());
                        }
                    }
                }
            }
            SetupDiDestroyDeviceInfoList(devInfo);
        }

        /* ---- Pass 2: Remove PCI bus devices ---- */
        CyDebug("Scanning for PCI bus devices...\n");
        devInfo = SetupDiGetClassDevsA(NULL, "PCI", NULL,
                                        DIGCF_ALLCLASSES);
        if (devInfo != INVALID_HANDLE_VALUE) {
            devData.cbSize = sizeof(SP_DEVINFO_DATA);
            for (idx = 0;
                 SetupDiEnumDeviceInfo(devInfo, idx, &devData);
                 idx++) {
                char hwId[256];
                if (SetupDiGetDeviceRegistryPropertyA(
                        devInfo, &devData, SPDRP_HARDWAREID,
                        NULL, (PBYTE)hwId, sizeof(hwId), NULL)) {
                    if (strstr(hwId, "VEN_120E")) {
                        CyPrint("Found Cyclades bus device: %s\n", hwId);
                        if (SetupDiCallClassInstaller(
                                DIF_REMOVE, devInfo, &devData)) {
                            CyPrint("Bus device removed\n");
                        } else {
                            CyDebug("Bus removal failed: %lu\n",
                                     GetLastError());
                        }
                    }
                }
            }
            SetupDiDestroyDeviceInfoList(devInfo);
        }
    }

    /* Step 3: Delete the service */
    CyPrint("\n--- Step 3: Delete service registration ---\n");
    DeleteDriverService();

    /* Step 4: Delete the driver file */
    CyPrint("\n--- Step 4: Delete driver binary ---\n");
    DeleteDriverFile();

    /* Step 5: Clean registry */
    CyPrint("\n--- Step 5: Clean registry ---\n");
    CleanRegistry();

    /* Step 6: Remove OEM INF from driver store */
    /* Step 6: Remove OEM INF from driver store.
     * SetupUninstallOEMInf is available on XP SP2+. On Win2K/XP RTM,
     * we fall back to pnputil instructions. We dynamically load the
     * function to maintain Win2K compatibility. (Audit P2-B5 fix) */
    CyPrint("\n--- Step 6: Remove INF from driver store ---\n");
    {
        typedef BOOL (WINAPI *PFN_SetupUninstallOEMInfA)(
            PCSTR InfFileName, DWORD Flags, PVOID Reserved);
        PFN_SetupUninstallOEMInfA pfnUninstall;
        HMODULE hSetupApi;
        BOOL uninstalled = FALSE;

        hSetupApi = GetModuleHandleA("setupapi.dll");
        if (hSetupApi) {
            pfnUninstall = (PFN_SetupUninstallOEMInfA)
                GetProcAddress(hSetupApi, "SetupUninstallOEMInfA");
            if (pfnUninstall) {
                /* Try to uninstall our OEM INF.
                 * We need to find the oem*.inf name first.
                 * For now, try the known INF name — SetupAPI may
                 * accept the original name. */
                if (pfnUninstall("cyclom-y.inf", 0x0001, NULL)) {
                    CyPrint("OEM INF removed from driver store\n");
                    uninstalled = TRUE;
                }
            }
        }
        if (!uninstalled) {
            CyPrint("Auto-removal not available.\n");
            CyPrint("To manually remove: pnputil -e (list), pnputil -f -d oem*.inf\n");
        }
    }

    /* Step 7: Remove test certificate if requested */
    if (removeCert) {
        CyPrint("\n--- Step 7: Remove test certificate ---\n");

        CyPrint("Removing test certificate from stores...\n");
        system("certutil -delstore TrustedPublisher \"Cyclades Test\"");
        system("certutil -delstore Root \"Cyclades Test\"");

        CyPrint("Disabling TESTSIGNING mode...\n");
        if (system("bcdedit -set TESTSIGNING OFF") != 0)
            CyPrint("bcdedit not available (Win2K/XP don't need it)\n");
    }

    CyPrint("\n=== Removal complete ===\n");
    CyPrint("REBOOT RECOMMENDED to fully release driver resources.\n");

    return 0;
}


/* ====================================================================
 * DoStatus — Show current installation status
 * ==================================================================== */

static int DoStatus(void)
{
    SC_HANDLE scm, svc;
    SERVICE_STATUS svcStatus;

    CyPrint("=== Cyclades Cyclom-Y Driver Status (v%s) ===\n\n", CY_VERSION);

    /* Check service */
    scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm) {
        svc = OpenServiceA(scm, CY_SERVICE_NAME, SERVICE_QUERY_STATUS);
        if (svc) {
            if (QueryServiceStatus(svc, &svcStatus)) {
                CyPrint("Service '%s': ", CY_SERVICE_NAME);
                switch (svcStatus.dwCurrentState) {
                case SERVICE_STOPPED:         printf("STOPPED\n"); break;
                case SERVICE_RUNNING:         printf("RUNNING\n"); break;
                case SERVICE_START_PENDING:   printf("STARTING\n"); break;
                case SERVICE_STOP_PENDING:    printf("STOPPING\n"); break;
                default: printf("state %lu\n", svcStatus.dwCurrentState);
                }
            }
            CloseServiceHandle(svc);
        } else {
            CyPrint("Service '%s': NOT INSTALLED\n", CY_SERVICE_NAME);
        }
        CloseServiceHandle(scm);
    }

    /* Check driver file */
    {
        char path[MAX_PATH];
        UINT sysLen = GetSystemDirectoryA(path, sizeof(path));
        if (sysLen > 0) {
            strcat(path, "\\Drivers\\" CY_DRIVER_FILE);
            if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
                CyPrint("Driver file: %s — EXISTS\n", path);
            } else {
                CyPrint("Driver file: %s — NOT FOUND\n", path);
            }
        }
    }

    /* Check for Cyclades devices */
    CyPrint("\nScanning for Cyclades PCI devices...\n");
    {
        HDEVINFO devInfo;
        SP_DEVINFO_DATA devData;
        DWORD idx;
        int found = 0;

        devInfo = SetupDiGetClassDevsA(NULL, "PCI", NULL,
                                        DIGCF_ALLCLASSES);
        if (devInfo != INVALID_HANDLE_VALUE) {
            devData.cbSize = sizeof(SP_DEVINFO_DATA);
            for (idx = 0;
                 SetupDiEnumDeviceInfo(devInfo, idx, &devData);
                 idx++) {
                char hwId[256];
                char desc[256];
                if (SetupDiGetDeviceRegistryPropertyA(
                        devInfo, &devData, SPDRP_HARDWAREID,
                        NULL, (PBYTE)hwId, sizeof(hwId), NULL)) {
                    if (strstr(hwId, "VEN_120E")) {
                        SetupDiGetDeviceRegistryPropertyA(
                            devInfo, &devData, SPDRP_DEVICEDESC,
                            NULL, (PBYTE)desc, sizeof(desc), NULL);
                        CyPrint("  Device: %s\n", desc);
                        CyPrint("  HW ID:  %s\n", hwId);
                        found++;
                    }
                }
            }
            SetupDiDestroyDeviceInfoList(devInfo);
        }

        if (found == 0) {
            CyPrint("  No Cyclades PCI devices found.\n");
        } else {
            CyPrint("  %d device(s) found.\n", found);
        }
    }

    /* Check test signing */
    CyPrint("\nTest signing: ");
    {
        /* Query TESTSIGNING via bcdedit — simplistic check */
        FILE *pipe = _popen("bcdedit /enum {current} 2>NUL", "r");
        if (pipe) {
            char line[256];
            int foundTestSign = 0;
            while (fgets(line, sizeof(line), pipe)) {
                if (strstr(line, "testsigning") && strstr(line, "Yes")) {
                    foundTestSign = 1;
                }
            }
            _pclose(pipe);
            printf("%s\n", foundTestSign ? "ENABLED" : "disabled");
        } else {
            printf("unable to check\n");
        }
    }

    return 0;
}


/* ====================================================================
 * main — Parse arguments and dispatch
 * ==================================================================== */

int main(int argc, char *argv[])
{
    int doInstall   = 0;
    int doUninstall = 0;
    int doStatus    = 0;
    int testSign    = 0;
    int removeCert  = 0;
    int force       = 0;
    int i;

    printf("CYINSTALL v%s — Cyclades Cyclom-Y Driver Installer/Uninstaller\n", CY_VERSION);
    printf("============================================================\n\n");

    if (argc < 2) {
        printf("Usage:\n");
        printf("  cyinstall /install [/testsign] [/silent] [/verbose]\n");
        printf("  cyinstall /uninstall [/force] [/removecert] [/silent]\n");
        printf("  cyinstall /status\n");
        printf("\nOptions:\n");
        printf("  /install     Install driver package\n");
        printf("  /uninstall   Remove driver completely\n");
        printf("  /status      Show current installation status\n");
        printf("  /testsign    Install test cert + enable TESTSIGNING\n");
        printf("  /removecert  Remove test cert + disable TESTSIGNING\n");
        printf("  /force       Force uninstall even if device is busy\n");
        printf("  /silent      No prompts or status output\n");
        printf("  /verbose     Show detailed debug output\n");
        return 1;
    }

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (_stricmp(argv[i], "/install") == 0)     doInstall = 1;
        else if (_stricmp(argv[i], "/uninstall") == 0) doUninstall = 1;
        else if (_stricmp(argv[i], "/status") == 0)    doStatus = 1;
        else if (_stricmp(argv[i], "/testsign") == 0)  testSign = 1;
        else if (_stricmp(argv[i], "/removecert") == 0) removeCert = 1;
        else if (_stricmp(argv[i], "/force") == 0)     force = 1;
        else if (_stricmp(argv[i], "/silent") == 0)    g_silent = 1;
        else if (_stricmp(argv[i], "/verbose") == 0)   g_verbose = 1;
        else {
            printf("Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (doInstall)
        return DoInstall(testSign);
    else if (doUninstall)
        return DoUninstall(force, removeCert);
    else if (doStatus)
        return DoStatus();

    printf("No action specified. Use /install, /uninstall, or /status.\n");
    return 1;
}
