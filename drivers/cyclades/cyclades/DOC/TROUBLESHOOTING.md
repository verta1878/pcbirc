# Troubleshooting Guide — Driver Issues, BSOD Recovery, Safe Mode

## IMPORTANT: Read This BEFORE You Install

If you're reading this because the driver already caused a BSOD
and the machine won't boot, skip to **Section 3: Emergency Recovery**.

---

## Section 1: Common Issues After Installation

### 1.1 Driver Loads But No COM Ports Appear

**Symptoms:**
- Device Manager shows the Cyclom-Y card under "Multiport Serial
  Adapters" with a green checkmark (no error)
- But no new COM ports appear under "Ports (COM & LPT)"

**Causes and fixes:**

1. **Child PDOs not enumerated.** The bus enumerator found the card
   but didn't create child port devices.
   ```
   Fix: Open Device Manager → right-click the Cyclom-Y device →
        "Scan for hardware changes"
   ```

2. **Port INF missing.** cyclom-y.inf loaded but cyyport.inf wasn't
   copied to the driver store.
   ```
   Fix: Copy cyyport.inf to the same directory as cyclom-y.inf,
        then: pnputil -a cyclom-y.inf
   ```

3. **COM port numbers exhausted.** The SERIALCOMM database is full
   (256 ports max). Stale entries from previous installs.
   ```
   Fix: reg delete HKLM\HARDWARE\DEVICEMAP\SERIALCOMM /va /f
        Then reboot. The driver will claim fresh COM numbers.
   ```

### 1.2 Device Manager Shows Yellow Exclamation (Code 10, 28, 31, 37, 52)

**Code 10: "This device cannot start"**
- The driver loaded but DriverEntry or START_DEVICE failed.
- Check Event Viewer → System log for CYLOG_* messages.
- Common cause: memory address conflict with another device.
  ```
  Fix: Check if another driver claimed the same memory range.
       Control Panel → System → Device Manager → View → Resources by type
       Look for memory conflicts in the D0000-D8000 range (ISA)
       or in the PCI memory range.
  ```

**Code 28: "Drivers not installed"**
- Windows can't find a driver for the hardware ID.
  ```
  Fix: Right-click → Update Driver → Browse my computer →
       point to the directory with cyclom-y.inf and cyport.sys
  ```

**Code 31: "This device is not working properly"**
- Driver loaded but reported an error during START_DEVICE.
- Usually means no CD1400 chips detected at the mapped address.
  ```
  Fix: Run cytest.exe to verify the card is physically detected.
       If cytest finds nothing, the card may be defective or
       the memory address may be wrong.
  ```

**Code 37: "Windows cannot initialize the device driver"**
- DriverEntry failed (returned an error status).
  ```
  Fix: Check Event Viewer for the specific error.
       Enable checked (debug) build and attach WinDbg.
  ```

**Code 52: "Windows cannot verify the digital signature"**
- Driver signing issue. Windows 10+ with Secure Boot.
  ```
  Fix (development): bcdedit -set TESTSIGNING ON (reboot)
  Fix (production):  Submit driver to Microsoft Dev Portal
                     for attestation signing.
  See SIGNING_WORKFLOW.md for details.
  ```

### 1.3 Port Opens But No Data Transmitted/Received

**Symptoms:**
- COM port appears in Device Manager
- Applications can open the port (no error)
- But WriteFile succeeds without data appearing on the wire
- Or ReadFile never returns data even when remote sends

**Debug steps:**

1. **Check baud rate match.** Both ends must use the same baud.
   ```
   Run: mode COM3         (shows current baud, parity, data bits)
   ```

2. **Check cable.** Null modem cables swap TX/RX. Straight cables
   don't. Use the wrong one and TX goes to TX (nothing received).
   ```
   Loopback test: cyloopback COM3
   If loopback passes, the driver works. Problem is the cable.
   ```

3. **Check flow control.** If CTS is not asserted and CTS flow
   control is enabled, the driver won't transmit.
   ```
   Run: cyloopback COM3 /V    (verbose — shows modem signals)
   Check: CTS should be high if RTS→CTS loopback cable is used.
   ```

4. **Check interrupt delivery.** If the ISR never fires, no data
   flows. This happens when the PCI interrupt is misconfigured.
   ```
   Debug: Enable checked build. Look for "ISR: chip X SVRR=0x..."
          messages in debugger output. If no SVRR messages appear
          when data should be arriving, the interrupt isn't connected.
   ```

### 1.4 BSOD During Normal Operation

**Symptoms:**
- Blue screen while using the serial port
- Bug check codes: IRQL_NOT_LESS_OR_EQUAL (0x0A),
  DRIVER_IRQL_NOT_LESS_OR_EQUAL (0x0D1),
  SYSTEM_THREAD_EXCEPTION_NOT_HANDLED (0x7E),
  KMODE_EXCEPTION_NOT_HANDLED (0x1E)

**Collect crash dump:**
```
1. After reboot, check: %SystemRoot%\MEMORY.DMP (full dump)
   or %SystemRoot%\Minidump\MiniXXXX.dmp (minidump)

2. Open dump in WinDbg:
   File → Open Crash Dump → select the .dmp file
   Type: !analyze -v
   Look for the faulting module — if it says cyport.sys,
   the bug is in our driver.

3. Type: lm vm cyport
   This shows the driver version and timestamp.

4. Type: kb
   This shows the call stack at the time of crash.
```

**Common BSOD causes in serial drivers:**

| Bug Check | Likely Cause | Fix |
|-----------|-------------|-----|
| 0x0A | Accessed paged memory at DISPATCH+ | DPC accessing paged pool |
| 0x0D1 | Accessed paged memory at DIRQL | ISR accessing paged memory |
| 0x7E | Unhandled exception in driver | Null pointer dereference |
| 0x1E | Exception in kernel mode | Invalid register access |
| 0x35 | No more IRP stack locations | Forwarding IRP past bottom |
| 0x44 | Multiple IRP completions | IRP completed twice (race) |
| 0xCE | Driver unloaded with pending ops | Remove lock not used |

---

## Section 2: Safe Mode Driver Removal

### 2.1 Windows 7 Safe Mode

**How to enter Safe Mode:**
```
1. Restart the computer
2. Press F8 REPEATEDLY during boot (before the Windows logo)
3. You'll see the "Advanced Boot Options" menu
4. Select "Safe Mode" (no networking needed)
5. Press Enter
6. Windows boots with minimal drivers — our driver does NOT load
```

**Remove the driver in Safe Mode:**
```bat
REM Open Command Prompt as Administrator
REM Start → All Programs → Accessories → right-click "Command Prompt"
REM → "Run as administrator"

REM Step 1: Stop the service (may already be stopped in Safe Mode)
sc stop cyport
sc delete cyport

REM Step 2: Delete the driver binary
del %SystemRoot%\System32\Drivers\cyport.sys

REM Step 3: Clean up the driver store
REM Find the OEM INF name:
pnputil -e | findstr /i "cyclades"
REM Delete it (replace oem5.inf with the actual name):
pnputil -f -d oem5.inf

REM Step 4: Clean registry
reg delete HKLM\SYSTEM\CurrentControlSet\Services\cyport /f

REM Step 5: Reboot normally
shutdown /r /t 0
```

**If F8 doesn't work (fast boot enabled):**
```
1. If Windows starts to boot and BSODs:
   - Let it BSOD 3 times in a row
   - Windows 7 will automatically offer "Launch Startup Repair"
   - From Startup Repair, click "View advanced options"
   - Choose "Command Prompt"
   - Follow the deletion steps above
   
2. Or: boot from Windows 7 install DVD
   - Select "Repair your computer"
   - Choose "Command Prompt"
   - Delete the driver file (see Section 3)
```

### 2.2 Windows XP Safe Mode

**How to enter Safe Mode:**
```
1. Restart the computer
2. Press F8 during boot
3. Select "Safe Mode"
4. Log in as Administrator
```

**Remove the driver:**
```bat
REM Open Command Prompt
REM Start → Run → cmd

REM Delete the driver
del %SystemRoot%\System32\Drivers\cyport.sys

REM Delete the service
sc delete cyport

REM Reboot
shutdown -r -t 0
```

### 2.3 Windows 10/11 Safe Mode

**How to enter Safe Mode (UEFI systems with fast boot):**
```
Method 1: From login screen
  1. Hold SHIFT and click the Power icon → Restart
  2. Choose: Troubleshoot → Advanced Options → Startup Settings
  3. Click "Restart"
  4. Press 4 for "Enable Safe Mode"

Method 2: From Settings (if you can boot normally)
  1. Settings → Update & Security → Recovery
  2. Under "Advanced startup" click "Restart now"
  3. Troubleshoot → Advanced → Startup Settings → Restart
  4. Press 4 for Safe Mode

Method 3: Force via boot failure
  1. Turn on the PC
  2. When Windows starts loading, hold the power button to force shutdown
  3. Repeat 3 times
  4. Windows enters Automatic Repair mode
  5. Choose: Advanced Options → Startup Settings → Safe Mode
```

**Remove the driver:**
```bat
REM Same steps as Windows 7 Safe Mode (Section 2.1)
REM Plus: disable TESTSIGNING if it was enabled
bcdedit -set TESTSIGNING OFF
```

---

## Section 3: Emergency Recovery (Machine Won't Boot)

### 3.1 The BSOD Loop

**Symptoms:**
- Machine starts, shows the Windows logo, then immediately BSODs
- Reboots automatically and BSODs again
- Infinite loop — machine is unusable

**Root cause:**
The driver loads during boot (PnP detects the Cyclom-Y card and
loads cyport.sys). If the driver crashes during DriverEntry or
START_DEVICE, it BSODs before the desktop appears.

### 3.2 Recovery Console (Windows XP / Server 2003)

```
1. Boot from the Windows XP CD
2. Press R for "Recovery Console"
3. Select the Windows installation (usually "1")
4. Enter the Administrator password

5. Delete the driver:
   cd \windows\system32\drivers
   del cyport.sys
   
6. Disable the service:
   disable cyport

7. Exit and reboot:
   exit
```

### 3.3 Windows Recovery Environment (Windows 7)

```
1. Boot from the Windows 7 install DVD
   (or the pre-installed recovery partition — press F8 during boot,
    select "Repair Your Computer")
    
2. Select your language → click Next
3. Click "Repair your computer"
4. Select the Windows 7 installation from the list
5. Click "Command Prompt"

6. Find the Windows drive:
   dir C:\Windows\System32\Drivers\cyport.sys
   (If not found, try D:\, E:\, etc. The drive letter in WinRE
    may not be C: because WinRE uses its own drive mapping.)

7. Delete the driver:
   del X:\Windows\System32\Drivers\cyport.sys
   (replace X: with the correct drive letter)

8. Disable the service via registry:
   reg load HKLM\TempSystem X:\Windows\System32\config\SYSTEM
   reg delete HKLM\TempSystem\ControlSet001\Services\cyport /f
   reg unload HKLM\TempSystem

9. Close Command Prompt → click "Restart"
```

### 3.4 Windows Recovery Environment (Windows 10/11)

```
1. Boot from Windows 10/11 install USB
2. Click "Repair your computer"
3. Troubleshoot → Advanced Options → Command Prompt

4. Find the Windows drive:
   dir C:\Windows\System32\Drivers\cyport.sys
   dir D:\Windows\System32\Drivers\cyport.sys
   (WinRE often maps the Windows partition to D: or E:)

5. Delete the driver:
   del X:\Windows\System32\Drivers\cyport.sys

6. Disable the service:
   reg load HKLM\TempSys X:\Windows\System32\config\SYSTEM
   reg delete "HKLM\TempSys\ControlSet001\Services\cyport" /f
   reg unload HKLM\TempSys

7. Close → Continue (restart normally)
```

### 3.5 Last Resort: Remove the Cyclom-Y Card

If none of the above works:
```
1. Shut down the PC
2. Open the case
3. Physically remove the Cyclom-Y PCI card
4. Boot normally (PnP won't find the card, driver won't load)
5. Use Safe Mode or normal mode to uninstall the driver:
   cyinstall /uninstall /force
6. Reboot
7. Re-insert the card ONLY after confirming a fixed driver is ready
```

---

## Section 4: Debug Build Troubleshooting

### 4.1 Installing the Checked (Debug) Build

```bat
REM The checked build of cyport.sys has full debug output.
REM Replace the free build with the checked build:

copy /y cyport_checked.sys %SystemRoot%\System32\Drivers\cyport.sys

REM Then either:
REM   a) Reboot (driver loads on next boot)
REM   b) Disable/enable the device in Device Manager (forces reload)
```

### 4.2 Viewing Debug Output

**Method 1: WinDbg (kernel debugger)**
```
1. Connect target machine to host via serial cable or network
2. On target: bcdedit /debug ON (reboot required)
3. On host: Open WinDbg, select kernel debug
4. Set debug filter: ed nt!Kd_DEFAULT_Mask 0xFFFFFFFF
5. All CYPORT debug output appears in WinDbg
```

**Method 2: DbgView (Sysinternals) — easier, no second machine**
```
1. Download DbgView from Microsoft Sysinternals
2. Run DbgView as Administrator
3. Menu: Capture → Capture Kernel (check it)
4. All CYPORT debug messages appear in the window
5. Filter: type "CYPORT" in the Include filter

Debug levels in output:
  CYPORT[1] = ERROR   — something went very wrong
  CYPORT[2] = WARNING — unexpected but recoverable
  CYPORT[3] = INFO    — normal operations (open, close, baud change)
  CYPORT[4] = TRACE   — every IOCTL and IRP
  CYPORT[5] = VERBOSE — ISR-level detail (very noisy)
```

### 4.3 Changing Debug Level at Runtime

```bat
REM Set via registry — takes effect on next driver load
reg add HKLM\SYSTEM\CurrentControlSet\Services\cyport\Parameters ^
    /v DebugLevel /t REG_DWORD /d 4

REM Level 0: Silent
REM Level 1: Errors only (production default)
REM Level 2: Errors + warnings
REM Level 3: Info (checked build default)
REM Level 4: Trace (every IOCTL, every IRP)
REM Level 5: Verbose (ISR register dumps — VERY noisy)
```

### 4.4 Reading Crash Dumps

```
1. Ensure crash dumps are enabled:
   Control Panel → System → Advanced → Startup and Recovery → Settings
   Under "Write debugging information": select "Kernel memory dump"
   Dump file: %SystemRoot%\MEMORY.DMP

2. After a BSOD, the dump is at C:\Windows\MEMORY.DMP

3. Open in WinDbg:
   File → Open Crash Dump
   Commands:
     !analyze -v           — automated crash analysis
     lm vm cyport          — driver version info
     kb                    — call stack
     .bugcheck             — bug check code details
     !irp [address]        — IRP details (if in the stack)
     !devobj [address]     — device object dump
     dt cyport!CY_PDO_EXT [address] — dump our extension

4. Common stack patterns and what they mean:

   cyport!CyInterruptService → ISR bug (register access)
   cyport!CyReadDpcRoutine   → DPC bug (IRP completion)
   cyport!CyDispatchIoCtl    → IOCTL handling bug
   cyport!CyDispatchCreate   → Port open bug
   nt!KeBugCheckEx           → Assertion or invariant violation
```

---

## Section 5: Known Issues and Workarounds

### 5.1 PCI Register Spacing (ISA vs PCI) — FIXED

**Status: FIXED. BusIndex is auto-detected from PCI BAR address
and applied via g_CyBusIndex in CyReadReg/CyWriteReg.**

Cards with physical address >= 1MB are detected as PCI (BusIndex=1,
registers shifted ×2 on top of cd1400.h's ×2 = ×4 total). Cards
below 1MB are ISA (BusIndex=0, no extra shift).

### 5.2 DTR/RTS Pin Inversion — FIXED (Framework)

**Status: FIXED. RtsDtrInv field added to CY_PDO_EXT. Defaults
to FALSE (normal mapping). Can be set via registry per-port.**

The CyMsvrToModemStatus function accepts the inversion flag.
Currently defaults to FALSE for all ports. To enable for a
specific board revision, set via registry:
```
reg add HKLM\SYSTEM\CurrentControlSet\Services\cyport\Parameters ^
    /v RtsDtrInv /t REG_DWORD /d 1
```

### 5.3 COM Port Number Management — FIXED (Basic)

**Status: FIXED. Ports are assigned COM3 + PortIndex (COM3, COM4,
COM5...). Symbolic links are created in START_DEVICE and cleaned
up in REMOVE_DEVICE.**

Pool-allocated symbolic link names persist for the device lifetime.
Full ComDBClaimPort integration for dynamic numbering is a future
enhancement — the current fixed numbering works for all standard
configurations.

### 5.4 Driver Signing on Windows 10/11 (Code 52) — TOOLING READY

**Status: Signing tools included. Process documented. Not yet tested
with Microsoft Dev Portal.**

```
Test signing:    cyinstall /install /testsign
Production:     EV cert + MS Dev Portal attestation signing
Documentation:  See signing/SIGNING_WORKFLOW.md
```

### 5.5 PCI BAR Resource Parsing — FIXED

**Status: FIXED. START_DEVICE now parses AllocatedResourcesTranslated
for CmResourceTypeMemory and CmResourceTypeInterrupt.**

The driver correctly extracts the PCI memory BAR physical address,
length, interrupt vector, level, affinity, and mode from the PnP
resource list. Falls back to ISA default 0xD4000 only when no PCI
resources are present (ISA card or manual configuration).

---

## Section 6: Reporting Bugs

When reporting a bug, include:

1. **Windows version:** (e.g., "Windows 7 SP1 x64")
2. **Driver version:** (from cyinstall /status)
3. **Build type:** Checked or Free
4. **Bug check code:** (e.g., 0x0000000A)
5. **Crash dump:** Attach MEMORY.DMP or minidump
6. **Event Viewer entries:** System log, filter by source "cyport"
7. **DbgView output:** If using checked build
8. **Steps to reproduce:** What were you doing when it crashed?
9. **Hardware:** Cyclom-Y model (4Y, 8Y), PCI or ISA, Rev G or Rev J
10. **Loopback test result:** cyloopback COMn output
11. **Stress test result:** cystress COMn output
