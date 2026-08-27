# Driver Installation & Uninstallation Guide

## IMPORTANT: Always Know How to Remove Before You Install

A kernel driver that can't be uninstalled can brick a machine. If the
driver crashes during boot, Windows may blue-screen in a loop with no
way to recover except Safe Mode or a recovery disk. The uninstaller
must work even when the driver is broken.

---

## Installation Methods

### Method 1: Device Manager (Interactive)
```
1. Open Device Manager (devmgmt.msc)
2. Right-click the Cyclom-Y device (under "Other Devices" or "Multiport Serial")
3. Select "Update Driver..."
4. Choose "Browse my computer for driver software"
5. Point to the directory containing cyclom-y.inf
6. Click Next — Windows installs cyclom-y.sys + cyyport.inf
7. PnP manager detects child ports and installs cyport.sys for each
```

### Method 2: PnPUtil (Command Line, XP SP2+)
```bat
REM Install the driver package into the driver store
pnputil -a cyclom-y.inf

REM Force install (skip "Have Disk" dialog)
pnputil -i -a cyclom-y.inf
```

### Method 3: DevCon (Command Line, WDK Tool)
```bat
REM Install for a specific hardware ID
devcon install cyclom-y.inf "PCI\VEN_120E&DEV_0101"

REM Scan for new hardware after install
devcon rescan
```

### Method 4: CYINSTALL.EXE (Our Custom Installer)
```bat
REM Install driver + certificate
cyinstall /install

REM Install with test signing certificate
cyinstall /install /testsign

REM Silent install (no prompts)
cyinstall /install /silent
```

---

## Uninstallation Methods

### Method 1: Device Manager (Interactive)
```
1. Open Device Manager (devmgmt.msc)
2. Find the Cyclom-Y device under "Multiport Serial Adapters"
3. Right-click → "Uninstall"
4. CHECK the box "Delete the driver software for this device"
   (if you don't check this, the driver stays in the store
    and reinstalls automatically on next boot!)
5. Click OK
6. Reboot
```

### Method 2: PnPUtil (Command Line)
```bat
REM List installed driver packages — find our OEM INF name
pnputil -e
REM Look for "Cyclades" or "cyclom-y" in the output
REM Note the published name (e.g., oem5.inf)

REM Delete the driver package from the store
pnputil -d oem5.inf

REM Force delete (even if device is present)
pnputil -f -d oem5.inf
```

### Method 3: DevCon (WDK Tool)
```bat
REM Remove the device
devcon remove "PCI\VEN_120E&DEV_0101"

REM Remove all Cyclom-Y devices
devcon remove *VEN_120E*
```

### Method 4: CYINSTALL.EXE (Our Custom Uninstaller)
```bat
REM Uninstall driver + remove from driver store
cyinstall /uninstall

REM Force uninstall (even if device is in use)
cyinstall /uninstall /force

REM Uninstall + remove test certificate
cyinstall /uninstall /removecert
```

### Method 5: Safe Mode Recovery (When Driver Causes BSOD)
```
1. Boot into Safe Mode:
   - Win2K/XP: Press F8 during boot → "Safe Mode"
   - Win7:     Press F8 during boot → "Safe Mode"
   - Win10:    Hold Shift + click Restart → Troubleshoot →
               Advanced → Startup Settings → Safe Mode

2. In Safe Mode, the driver doesn't load (PnP skips it)

3. Open an admin command prompt:
   sc delete cyport
   del %SystemRoot%\System32\Drivers\cyport.sys
   
4. Or use Device Manager (works in Safe Mode):
   Right-click device → Uninstall → check "Delete driver software"

5. Reboot normally
```

### Method 6: Recovery Console / WinRE (Last Resort)
```
If Safe Mode also crashes (unlikely but possible):

Win2K/XP Recovery Console:
  Boot from CD → press R for Recovery Console
  cd \windows\system32\drivers
  del cyport.sys
  exit

Win7+ Recovery Environment:
  Boot from install media → "Repair your computer"
  → Command Prompt
  del X:\Windows\System32\Drivers\cyport.sys
  (X: = your Windows drive, may not be C: in WinRE)
```

---

## Registry Cleanup

The driver creates registry entries that should be removed on uninstall:

```bat
REM Service registration
reg delete HKLM\SYSTEM\CurrentControlSet\Services\cyport /f

REM Driver parameters
reg delete HKLM\SYSTEM\CurrentControlSet\Services\cyport\Parameters /f

REM COM port assignments (SERIALCOMM entries)
REM These are cleaned up by ComDBReleasePort in the uninstaller

REM Device instance entries (cleaned by PnP on device removal)
```

---

## CYINSTALL.EXE — Custom Installer/Uninstaller

### Why a custom installer?
- Handles test certificate installation automatically
- Enables TESTSIGNING on Win7 x64 (with user confirmation)
- Proper COM port number cleanup via MSPORTS ComDB
- Single command for install and uninstall
- Works on Win2K through Win11

### Command Line Options

| Option | What it does |
|--------|-------------|
| `/install` | Install driver package + INF |
| `/uninstall` | Remove driver + clean registry + release COM ports |
| `/testsign` | Install test certificate + enable TESTSIGNING mode |
| `/removecert` | Remove test certificate from certificate store |
| `/silent` | No prompts or dialogs |
| `/force` | Force operation even if device is in use |
| `/status` | Show current installation status |
| `/list` | List all Cyclom-Y devices and their COM port assignments |

### How the Installer Works (install flow)

```
1. Check for Administrator privileges (required)
2. If /testsign:
   a. Install test certificate to TrustedPublisher store
   b. Install test certificate to Root store
   c. Enable TESTSIGNING via bcdedit (user must confirm)
3. Copy cyport.sys to %SystemRoot%\System32\Drivers\
4. Copy INF files to the driver store (SetupCopyOEMInf)
5. Trigger PnP re-enumeration (CM_Reenumerate_DevNode on root)
6. Wait for PnP to detect and start the device
7. Report COM port assignments
```

### How the Uninstaller Works (uninstall flow)

```
1. Check for Administrator privileges (required)
2. Stop the driver service (ControlService SERVICE_CONTROL_STOP)
3. Disable the device (SetupDiSetClassInstallParams + DI_REMOVE)
4. Release COM port numbers (ComDBReleasePort for each port)
5. Delete the service (DeleteService)
6. Remove the driver from the driver store (SetupUninstallOEMInf)
7. Delete cyport.sys from %SystemRoot%\System32\Drivers\
8. Clean registry entries
9. If /removecert:
   a. Remove test certificate from TrustedPublisher
   b. Remove test certificate from Root
   c. Disable TESTSIGNING via bcdedit (user must confirm)
10. Report success — reboot recommended
```
