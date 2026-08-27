/* ====================================================================
 * cycoinstall.c — Co-installer for Cyclades Cyclom-Y COM Port Assignment
 * ====================================================================
 * A device co-installer that:
 *   1. Queries the next available COM port number from ComDB
 *   2. Assigns it to the port
 *   3. Shows the assigned COM number in Device Manager
 *
 * Build (WDK):
 *   cl /LD /I$(DDK_INC_PATH) cycoinstall.c setupapi.lib msports.lib
 *
 * INF reference:
 *   [Port.CoInstallers]
 *   AddReg=Port.CoInstallers.AddReg
 *   [Port.CoInstallers.AddReg]
 *   HKR,,CoInstallers32,0x00010000,"cycoinstall.dll,CyclomCoInstaller"
 *
 * License: GPLv3
 * ==================================================================== */

#include <windows.h>
#include <setupapi.h>

/* ComDB functions — msports.lib */
typedef LONG (WINAPI *PFN_ComDBOpen)(PHCOMDB);
typedef LONG (WINAPI *PFN_ComDBGetCurrentPortUsage)(HCOMDB, PBYTE, DWORD, ULONG, LPDWORD);
typedef LONG (WINAPI *PFN_ComDBClaimPort)(HCOMDB, DWORD, BOOL, PBOOL);
typedef LONG (WINAPI *PFN_ComDBReleasePort)(HCOMDB, DWORD);
typedef LONG (WINAPI *PFN_ComDBClose)(HCOMDB);

/* ====================================================================
 * CyclomCoInstaller — Main co-installer entry point
 * ====================================================================
 * Called by the PnP manager during device installation.
 * DIF_INSTALLDEVICE: Assign COM port number.
 * DIF_REMOVE: Release COM port number.
 * ==================================================================== */

DWORD WINAPI CyclomCoInstaller(
    DI_FUNCTION                 InstallFunction,
    HDEVINFO                    DeviceInfoSet,
    PSP_DEVINFO_DATA            DeviceInfoData,
    PCOINSTALLER_CONTEXT_DATA   Context)
{
    if (InstallFunction == DIF_INSTALLDEVICE && !Context->PostProcessing) {
        /* Pre-install: claim next available COM port from ComDB */
        HMODULE hMsports;
        HCOMDB hComDB;

        hMsports = LoadLibraryA("msports.dll");
        if (hMsports) {
            PFN_ComDBOpen pOpen;
            PFN_ComDBClaimPort pClaim;
            PFN_ComDBClose pClose;

            pOpen = (PFN_ComDBOpen)GetProcAddress(hMsports, "ComDBOpen");
            pClaim = (PFN_ComDBClaimPort)GetProcAddress(hMsports, "ComDBClaimNextFreePort");
            pClose = (PFN_ComDBClose)GetProcAddress(hMsports, "ComDBClose");

            if (pOpen && pClaim && pClose) {
                if (pOpen(&hComDB) == ERROR_SUCCESS) {
                    DWORD portNum = 0;
                    BOOL  claimed = FALSE;

                    /* Claim next free COM port number */
                    if (pClaim(hComDB, &portNum, &claimed) == ERROR_SUCCESS && claimed) {
                        /* Store port number in device registry key */
                        HKEY hKey;
                        hKey = SetupDiCreateDevRegKeyA(DeviceInfoSet, DeviceInfoData,
                                DICS_FLAG_GLOBAL, 0, DIREG_DEV, NULL, NULL);
                        if (hKey != INVALID_HANDLE_VALUE) {
                            CHAR portName[16];
                            _snprintf(portName, sizeof(portName), "COM%lu", portNum);
                            RegSetValueExA(hKey, "PortName", 0, REG_SZ,
                                          (LPBYTE)portName, (DWORD)strlen(portName) + 1);
                            RegCloseKey(hKey);
                        }

                        /* Write to SERIALCOMM */
                        {
                            HKEY hSerialComm;
                            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                "HARDWARE\\DEVICEMAP\\SERIALCOMM",
                                0, KEY_SET_VALUE, &hSerialComm) == ERROR_SUCCESS) {
                                CHAR deviceName[64];
                                CHAR portVal[16];
                                _snprintf(deviceName, sizeof(deviceName),
                                         "\\Device\\CycladesCOM%lu", portNum);
                                _snprintf(portVal, sizeof(portVal), "COM%lu", portNum);
                                RegSetValueExA(hSerialComm, deviceName, 0, REG_SZ,
                                             (LPBYTE)portVal, (DWORD)strlen(portVal) + 1);
                                RegCloseKey(hSerialComm);
                            }
                        }
                    }
                    pClose(hComDB);
                }
            }
            FreeLibrary(hMsports);
        }

        return ERROR_DI_POSTPROCESSING_REQUIRED;
    }

    return NO_ERROR;
}
