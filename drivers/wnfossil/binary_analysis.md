# WinFOSSIL — Complete Binary Analysis

**Analyst:** evga
**Original author:** Bryan Woodruff, Woodruff Software Systems, 1996
**Binaries:** WinFOSSIL v1.12 (Win95) + WinFOSSIL NT v1.0 Beta 3

---

## 1. Three Releases Required

| Release | Target | Core Driver | Architecture |
|---------|--------|-------------|-------------|
| v1.12 | Win95/98 | FOSSIL.VXD (23 KB) | Ring-0 VxD, hooks INT 14h via VMM |
| v1.0 NT | NT4/2000 | FOSSIL.DLL (30 KB) | VDD, hooks INT 14h via NTVDM |
| v2.0 Modern | Win7 x64+ | FOSSIL.DLL (new) | Win32 API, no VxD/VDD needed |

Each release is self-contained with all distribution files.

---

## 2. FOSSIL.DLL Exported Functions (33 total)

### Port I/O (13 functions)
```
commOpenPort(port)                    Open COM port, start threads
commClosePort(port)                   Close port, stop threads
commReadCharWait(port)                Read byte, block until available
commWriteCharWait(port, ch)           Write byte, block until sent
commWriteCharNoWait(port, ch)         Write byte, return immediately
commPeekRecv(port)                    Check if data available (no consume)
commReadBlock(port, buf, len)         Read block of bytes
commWriteBlock(port, buf, len)        Write block of bytes
commFlushXmit(port)                   Flush transmit buffer
commGetStatus(port)                   Get modem status lines + buffer state
commReadThread(port)                  Background read thread (internal)
commWriteThread(port)                 Background write thread (internal)
commCleanupProcess()                  Close all ports on process exit
```

### Port Configuration (7 functions)
```
commSetParams(port, params)           Set baud/parity/data/stop (FOSSIL Fn00)
commSetParamsEx(port, baud, par, d, s) Extended params (high baud rates)
commSetupDCB(port, dcb)               Direct DCB configuration
commSetDTRState(port, on)             DTR line control
commSetBreakState(port, on)           BREAK signal control
commSetFlowCtrl(port, flags)          XON/XOFF and RTS/CTS flow control
commEtxHandler(port, flags)           Ctrl-C/Ctrl-K interception
```

### Port Management (3 functions)
```
commIsPortEnabled(port, vm, flags)    Check if port is allocated/available
commReactivatePort(port)              Re-open port after DOS session ends
commCleanupProcess()                  Cleanup on DLL unload
```

### VMODEM Engine (8 functions)
```
commVmodemEngine(port)                Main AT command state machine
commVmodemParseCmdStr(port)           Parse AT command string
commVmodemDial(port, address)         ATD — TCP connect to host:port
commVmodemHangup(port)                ATH — TCP disconnect
commVmodemConnectMsg(port)            Send "CONNECT baud" to app
commVmodemFilter(port, buf, len)      Filter telnet IAC sequences
commVmodemEchoChar(port, ch)          Local echo handling
commVmodemSetState(port, state)       Set VMODEM state machine state
commVmodemStuffReadQ(port, buf, len)  Inject data into read buffer
commVmodemWaitForState(port, st, ms)  Wait for state transition w/ timeout
```

### VDD Hooks (5 functions — NT only)
```
VDDInitialize(hInst, reason, ctx)     DLL entry point
VDDRegisterInit()                     Register with NTVDM
VDDI14Dispatch()                      INT 14h handler (FOSSIL API)
VDDI2FDispatch()                      INT 2Fh handler (FOSSIL detect)
VDDHook(hVdd)                         Install VDD hooks
```

### VDD Register Accessors (14 functions — used by dispatch)
```
getAH()  getAL()  getAX()  getEAX()    Read DOS app registers
getBX()  getCX()  getDX()  getDL()
getDI()  getES()  getSP()  getSS()
setAX()  setBH()  setBL()  setCF()     Write DOS app registers
setCX()  setDX()
```

### Crypto (3 functions — registration)
```
_do_rc4(key, data, len)               RC4 stream cipher
_prepare_key(key, keylen, state)       RC4 key schedule
_MD5Init/Update/String                 MD5 hash (registration check)
```

### Helpers (2 functions)
```
_atoh(str)                            ASCII to hex conversion
_wsprintfA                            String formatting
```

---

## 3. Internal Data Structures

### Port State (per-port, linked list via gpPortList)
```c
typedef struct _FOSSIL_PORT {
    struct _FOSSIL_PORT *pNext;       // Linked list
    int      iPort;                   // 0-based port number
    HANDLE   hCom;                    // COM port handle
    HANDLE   hReadThread;             // Background reader
    HANDLE   hWriteThread;            // Background writer
    HANDLE   hReadEvent;              // Read completion event
    HANDLE   hWriteEvent;             // Write completion event
    CRITICAL_SECTION cs;              // Thread synchronization
    
    // Circular buffers
    BYTE     abRecvBuf[4096];         // Receive ring buffer
    DWORD    dwRecvHead, dwRecvTail;
    BYTE     abXmitBuf[4096];         // Transmit ring buffer
    DWORD    dwXmitHead, dwXmitTail;
    
    // Configuration
    DCB      dcb;                     // COM port DCB
    DWORD    dwLockedBaud;            // Locked baud rate (0=unlocked)
    BOOL     fEnabled;                // Port enabled in registry
    BOOL     fOpen;                   // Port currently open
    BOOL     fPortLocked;             // Baud rate locked
    char     szPortName[16];          // "COM1", "COM2", etc.
    
    // VMODEM state
    SOCKET   sock;                    // TCP socket
    SOCKET   sockListen;              // Listening socket
    int      iVmodemState;            // State machine state
    BOOL     fVmodemOnline;           // Connected flag
    BOOL     fVmodemEcho;             // Local echo
    char     szDialAddress[256];      // Last ATD target
    char     szCmdBuf[256];           // AT command accumulator
    int      iCmdLen;                 // Command buffer position
    
    // Performance counters
    DWORD    cbRead;                  // Bytes read total
    DWORD    cbWritten;               // Bytes written total
    DWORD    cbReadTimeOuts;          // Read timeout count
    DWORD    cbWriteTimeOuts;         // Write timeout count
    DWORD    cbVMWakeUps;             // VM wakeup count
} FOSSIL_PORT;
```

### VMODEM States
```
STATE_COMMAND    0     // Waiting for AT commands
STATE_ONLINE     1     // Data mode (connected)
STATE_DIALING    2     // Resolving/connecting
STATE_RINGING    3     // Incoming connection
STATE_HANGUP     4     // Disconnecting
STATE_ESCAPE     5     // +++ guard time
```

### VMODEM AT Commands Supported
```
ATZ             Reset modem (close connection, clear buffers)
ATD<address>    Dial (TCP connect to host:port)
ATDT<address>   Same as ATD (tone dial = TCP)
ATH             Hangup (close TCP connection)
ATE0/ATE1       Echo off/on
ATS0=N          Auto-answer (0=off, 1+=listen on port 23)
AT&D0/AT&D2     DTR handling (ignore/hangup on drop)
+++             Escape to command mode (1 sec guard time)
ATA             Manual answer (accept pending connection)
ATI             Identification string
ATM0/ATM1       Speaker control (no-op in VMODEM)
```

### VMODEM Result Codes
```
OK              Command accepted
CONNECT         TCP connection established (+ baud rate)
RING            Incoming TCP connection
NO CARRIER      Connection lost / connect failed
ERROR           Invalid command
NO DIALTONE     DNS resolution failed
BUSY            Port already in use
```

---

## 4. INT 14h Dispatch Table (FOSSIL API)

```
AH=00h  commSetParams          Set baud rate/parity/data/stop
AH=01h  commWriteCharWait      Send character (wait)
AH=02h  commReadCharWait       Receive character (wait)
AH=03h  commGetStatus          Status request
AH=04h  FOSSIL init            Returns AX=1954h, BL=max_fn, BH=rev
AH=05h  FOSSIL deinit          Close port
AH=06h  commSetDTRState        Raise/lower DTR
AH=07h  (timer tick)           No-op
AH=08h  commFlushXmit          Flush output buffer
AH=09h  (purge output)         PurgeComm TX
AH=0Ah  (purge input)          PurgeComm RX
AH=0Bh  commWriteCharNoWait    Send char (no wait)
AH=0Ch  commPeekRecv           Peek input (no wait)
AH=0Dh  (keyboard peek)        Not implemented
AH=0Eh  (keyboard read)        Not implemented
AH=0Fh  commSetFlowCtrl        Flow control on/off
AH=10h  commEtxHandler         Ctrl-C/K checking
AH=11h  (set cursor)           VDD screen write
AH=12h  (get cursor)           VDD screen read
AH=13h  (write char screen)    VDD screen write
AH=14h  (watchdog)             No-op
AH=15h  (write BIOS)           VDD screen write
AH=18h  commReadBlock          Block read
AH=19h  commWriteBlock         Block write
AH=1Ah  commSetBreakState      Break on/off
AH=1Bh  (get info)             Returns FOSSIL info block
```

---

## 5. Registry Configuration

Key: `HKLM\System\CurrentControlSet\Services\VxD\FOSSIL`

Per-port values:
```
enabled           DWORD    0/1 — port active
port name         SZ       "COM1"
locked baud       DWORD    0 = unlocked, else locked rate
receive buffer size  DWORD 4096
transmit buffer size DWORD 4096
last port selected   DWORD  0-3
```

---

## 6. Distribution Files

### v1.12 (Win95/98) — 16 files
```
FOSSIL.VXD       23,401  VxD FOSSIL driver
WNFOSCTL.EXE     8,329   Port control CLI (LOCK/UNLOCK)
WNFOSSIL.CPL    15,872   Control Panel applet (Property Sheet)
SETUP.EXE       20,992   Installer
WNFOSSIL.HLP             WinHelp file
WNFOSSIL.CNT             Help contents
FOSSIL.INF                VxD device install info
INSTALL.INF               Setup file list + registry entries
LICENSE.TXT               License agreement
README.TXT                User documentation
WHATSNEW.TXT              Change log
REGFORM.TXT               Registration order form (English)
REGFORM.ITA               Registration order form (Italian)
PGPKEY.TXT                PGP public key
FILE_ID.DIZ               BBS description
WFOSKEY2.ZIP              Registration key utility
```

### v1.0 NT — 18 files
```
FOSSIL.DLL       30,208  VDD FOSSIL driver
WNFOSSIL.EXE     1,901   Real-mode loader (AUTOEXEC.NT)
WNFOSCTL.EXE    59,360   Port control CLI (LOCK/UNLOCK)
WNFOSSIL.CPL    15,872   Control Panel applet
SETUP.EXE       20,992   Installer
FOSSIL.USA        1,109   English language strings
WNFOSSIL.HLP             WinHelp file
WNFOSSIL.CNT             Help contents
FOSSIL.INF                Driver install info
INSTALL.INF               Setup configuration
LICENSE.TXT               License agreement
README.TXT                User documentation
README.1ST                Beta notice
WHATSNEW.TXT              Change log
BUGS.TXT                  Known bugs
VMODEM.TXT                VMODEM documentation
REGFORM.TXT               Registration order form
FILE_ID.DIZ               BBS description
```

---

## 7. Build Phases

### Phase A: Core FOSSIL engine (shared)
- Port state structure + linked list
- Circular buffer (ring buffer) with thread-safe access
- commOpenPort / commClosePort with background threads
- commReadCharWait / commWriteCharWait
- commReadBlock / commWriteBlock
- commGetStatus / commSetParams / commSetParamsEx
- commSetDTRState / commSetBreakState / commSetFlowCtrl
- commFlushXmit / commPeekRecv / commEtxHandler
- commReactivatePort / commIsPortEnabled / commCleanupProcess
- Performance counters

### Phase B: VMODEM engine
- AT command parser state machine
- commVmodemEngine (main loop)
- commVmodemParseCmdStr (tokenizer)
- commVmodemDial (Winsock connect)
- commVmodemHangup (socket close)
- commVmodemFilter (telnet IAC handling)
- commVmodemEchoChar / commVmodemStuffReadQ
- commVmodemConnectMsg / commVmodemSetState / WaitForState
- Result code generation (OK/CONNECT/RING/NO CARRIER/ERROR)

### Phase C: VDD layer (NT)
- VDDInitialize / VDDRegisterInit
- VDDI14Dispatch — INT 14h dispatch table
- VDDI2FDispatch — INT 2Fh FOSSIL detect
- Register accessor functions (get/set AX/BX/CX/DX/DI/ES)
- VDDHook

### Phase D: VxD layer (Win95)
- VxD DDB (Device Descriptor Block)
- V86 INT 14h hook via VMM services
- VCOMM port virtualization
- PM API thunking

### Phase E: Control Panel applet (CPL)
- CPlApplet entry point
- Property Sheet with port config tabs
- Registry read/write
- Registration dialog
- Bitmap/icon resources

### Phase F: Utilities
- WNFOSCTL.EXE — CLI port lock/unlock
- SETUP.EXE — file copy + registry setup
- WNFOSSIL.EXE (NT) — AUTOEXEC.NT loader

### Phase G: Documentation + packaging
- README.TXT / WHATSNEW.TXT / BUGS.TXT
- FOSSIL.INF / INSTALL.INF
- LICENSE.TXT / REGFORM.TXT
- WinHelp file (WNFOSSIL.HLP + .CNT)
- FILE_ID.DIZ for each release
- Registration key handling (GPLv3: always registered)

---

## 8. Bugs Found in Original

| Bug | Binary | Description |
|-----|--------|-------------|
| WF-1 | FOSSIL.DLL | "Jesus is the Christ!" Easter egg string |
| WF-2 | WNFOSSIL.EXE | No error handling if FOSSIL.DLL load fails |
| WF-3 | FOSSIL.DLL | MD5+RC4 registration can be bypassed (nag screen only) |
| WF-4 | WNFOSCTL.EXE | No bounds check on port number argument |
| WF-5 | FOSSIL.DLL | VMODEM: "NO DIALTONE" on DNS failure (misleading) |
| WF-6 | FOSSIL.DLL | commWriteThread uses INFINITE wait (can hang on close) |

---

## 8. Complete Registry Map

### Win95/98 (v1.12)

**Key:** `HKLM\System\CurrentControlSet\Services\VxD\FOSSIL`

| Value | Type | Description |
|-------|------|-------------|
| `StaticVxD` | SZ | `"fossil.vxd"` — driver filename |
| `Start` | DWORD | `0x00` — load at boot |
| `enabled` | DWORD | Per-port enable (0/1) |
| `port name` | SZ | `"COM1"`, `"COM2"`, etc. |
| `locked baud` | DWORD | 0=unlocked, else locked rate |
| `receive buffer size` | DWORD | Default 4096 |
| `transmit buffer size` | DWORD | Default 4096 |
| `last port selected` | DWORD | 0-3, last CPL selection |

**Per-port subkeys:** Values are stored under the main key with
port-specific prefixes in the VxD configuration manager.

**Performance counters (VxD perf registry):**
```
port %d bytes read/sec
port %d bytes written/sec
port %d read-timeouts/sec
port %d write-timeouts/sec
port %d VM wakeup calls/sec
times VM has been found sleeping with port activity/sec
```

**Uninstall:**
```
HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\Fossil
  DisplayName = "WinFOSSIL v1.12 for Windows 95"
  UninstallString = "%10%\rundll.exe setupx.dll,InstallHinfSection DefaultUninstall 132 %17%\fossil.inf"
```

### NT (v1.0)

**Key:** `HKLM\Software\Woodruff\WinFOSSIL`

| Value | Type | Description |
|-------|------|-------------|
| `Installed` | DWORD | `0x01` — driver installed flag |
| `enabled` | DWORD | Per-port enable (0/1) |
| `port name` | SZ | `"COM1"`, `"COM2"`, etc. |
| `locked baud` | DWORD | 0=unlocked, else locked rate |
| `receive buffer size` | DWORD | Default 4096 |
| `transmit buffer size` | DWORD | Default 4096 |

**Uninstall:**
```
HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\WinFOSSIL
  DisplayName = "WinFOSSIL v1.00 (Beta) for Windows NT"
  UninstallString = "%11%\rundll32.exe syssetup,SetupInfObjectInstallAction DefaultUninstall 132 %17%\fossil.inf"
```

**AUTOEXEC.NT modification:**
```
lh %SystemRoot%\system32\wnfossil.exe
```

### v2.0 Modern (our version)

**Key:** `HKLM\SOFTWARE\WinFOSSIL`

| Value | Type | Description |
|-------|------|-------------|
| `Port0Enabled` | DWORD | COM1 enable (0/1) |
| `Port0Baud` | DWORD | COM1 baud rate |
| `Port0Locked` | DWORD | COM1 baud locked (0/1) |
| `Port1Enabled` | DWORD | COM2 enable |
| `Port1Baud` | DWORD | COM2 baud rate |
| `Port1Locked` | DWORD | COM2 baud locked |
| `Port2Enabled` | DWORD | COM3 enable |
| `Port2Baud` | DWORD | COM3 baud rate |
| `Port2Locked` | DWORD | COM3 baud locked |
| `Port3Enabled` | DWORD | COM4 enable |
| `Port3Baud` | DWORD | COM4 baud rate |
| `Port3Locked` | DWORD | COM4 baud locked |
| `AutoOpen` | DWORD | Auto-open on use detect |
| `KeepOpen` | DWORD | Keep open during DOS session |
| `TimeSlice` | DWORD | Release timeslice when idle |

---

## 9. INF File Structure (Installer)

### Win95 FOSSIL.INF
```ini
[FossilInstall]
CopyFiles = Fossil.Files.Sys, Fossil.Files.Inf, Fossil.Files.Win
AddReg = Fossil.AddReg
RESTART

[Fossil.Files.Sys]           → %SystemRoot%\System32
fossil.vxd
wnfossil.cpl
wnfossil.hlp
wnfossil.cnt

[Fossil.Files.Inf]           → %SystemRoot%\Inf
fossil.inf

[Fossil.Files.Win]           → %SystemRoot%
wnfosctl.exe

[DestinationDirs]
Fossil.Files.Win = 10        ; %SystemRoot%
Fossil.Files.Sys = 11        ; %SystemRoot%\System32
Fossil.Files.Inf = 17        ; %SystemRoot%\Inf
```

### NT FOSSIL.INF
```ini
[FossilInstall]
CopyFiles = Fossil.Files.Sys, Fossil.Files.Inf, Fossil.Files.Win
AddReg = Fossil.AddReg
UpdateAutoBat = Fossil.AddAutoBat

[Fossil.Files.Sys]           → %SystemRoot%\System32
wnfossil.exe
fossil.dll
wnfossil.cpl
wnfossil.hlp
wnfossil.cnt

[Fossil.AddAutoBat]
CmdAdd=%SystemRoot%\system32\wnfossil.exe
```

---

## 10. FOSSIL.USA Language File Format

The .USA file contains BBS advertisement text displayed during the
registration nag screen. Not a standard language resource — it's a
raw text file shown in a dialog box. In our GPLv3 rebuild, the
registration system is removed and this file is not needed.

---

## 11. Source Base: netmodem2irc (GPLv3)

The netmodem2irc project (github.com/verta1878/netmodem2irc) contains
a complete VxD driver, FOSSIL engine, CPL applet, config app, and
InnoSetup installer — all GPLv3. This is OUR code. WinFOSSIL rebuild
adapts it to match Woodruff's API surface and distribution layout.

### Source Mapping

```
netmodem2irc Source             Lines  WinFOSSIL Target        Adaptation
──────────────────────────────  ─────  ────────────────────    ──────────────────────
driver/src/NETMODEM.ASM         5,712  FOSSIL.VXD              Rename DDB, change
driver/src/VMM.INC              2,892    device ID, strip
driver/src/VCOMM.INC              516    NetModem branding,
driver/src/NETMODEM.INC            175    adapt port struct
driver/src/REGDEF.INC              117    to WinFOSSIL layout
driver/src/SHELL.INC               139
driver/src/VPICD.INC                85
driver/src/VWIN32.INC               98
driver/src/VCOMMW32.INC            118
driver/src/NETMODEM.DEF             24
driver/src/NETMODEM.RC             106
                               ──────
VxD subtotal                    9,982

engine/NM_Fossil.pas              405  FOSSIL.DLL core         Rename exports to
engine/NM_FossilDriver.pas        131    commOpenPort etc,
engine/NM_ATCommand.pas            323    VMODEM engine         swap NetModem AT
engine/NM_UART16550.pas            365    NT VDD layer          extensions for
engine/NM_Int14ISR.pas             211                          standard FOSSIL,
engine/NM_TSR.pas                  210  WNFOSSIL.EXE (NT)      adapt TSR loader
engine/NM_TSRResident.pas          170
engine/NM_Node.pas                 346  Port management         rename node→port
engine/NM_Config.pas               298  Registry I/O            change key paths
engine/NM_ConfigApply.pas           70
engine/NM_DefaultConfig.pas        150
engine/NM_GlobalConfig.pas         369
engine/NM_Debug.pas                255
engine/NM_DebugView.pas          1,455
engine/NM_DirectRelay.pas          299
engine/NM_NamedPipeLink.pas        276
engine/NM_SynapseLink.pas          455  TCP transport           reuse for VMODEM
engine/NM_ServerBridge.pas         390
engine/NM_ServerLink.pas           275
engine/NM_SeamProtocol.pas         251
engine/NM_SeamSender.pas           161
engine/NM_Listserv.pas             190
engine/NM_AutoNews.pas             123
engine/NM_Com0ComLink.pas          236
engine/NetTransport.pas            315
engine/serial_ext.pas              103
engine/test/test_fossil_getinfo     58
                               ──────
Engine subtotal                  7,890

cpl/NetModemCPL.pas                100  WNFOSSIL.CPL            Change registry
cpl/resources/mainicon.ico           -    key path, rename
cpl/resources/NetModemCPL.rc         1    applet name

config/ConfigMain.pas              344  WNFOSCTL.EXE            Rename, change
config/resources/NMConfig.rc         1    registry paths,
config/resources/*.ico,*.bmp        30+   reuse all icons

server/MainForm.pas                413  (tray app base)         Strip server,
server/resources/*.ico,*.bmp        14    keep tray icon

common/NMVxD.pas                    -   VxD interface           Rename structs
common/NetModemVxD.pas              -   VxD Pascal wrapper

InnoIRC561/netmodem2irc.iss         67  SETUP (InnoSetup)       Change app name,
InnoIRC561/out/*.bmp,*.dll          -     files, registry
InnoIRC561/innosetup FPC port       -     (already ported)

docs/netmodem2irc_registry.md       -   Registry reference      Adapt key paths
docs/netmodem2irc_cpl_config.md     -   CPL design doc
docs/netmodem2irc_fossil_*.md       -   FOSSIL implementation

                               ──────
GRAND TOTAL                    18,798 lines of OUR GPLv3 code
```

### What Needs Adapting

#### A. VxD Driver (NETMODEM.ASM → FOSSIL.VXD)

1. **Device name**: `NETMODEM` → `FOSSIL`
2. **DDB (Device Descriptor Block)**: Change device ID, name string
3. **Registry key**: `Software\Allen Software\NetModem` →
   `System\CurrentControlSet\Services\VxD\FOSSIL`
4. **ComportStruct**: 22 bytes (NetModem) → WinFOSSIL layout
   (same fields, different offsets for enabled/locked/buffer)
5. **VCOMM port provider**: Change provider name registration
6. **Branding strings**: All `NetModem` → `WinFOSSIL`
7. **Performance counter names**: Match WinFOSSIL names:
   `port %d bytes read/sec`, `port %d bytes written/sec`, etc.

#### B. FOSSIL Engine (NM_*.pas → comm*.c or comm*.pas)

1. **Function names**: `NM_FossilInit` → `commOpenPort`
   `NM_FossilRead` → `commReadCharWait`, etc.
2. **Export names**: Match original DLL exports:
   `_commOpenPort@4`, `_commClosePort@4`, etc.
3. **VMODEM**: Strip NetModem-specific SEAM protocol,
   keep standard AT command parser (ATD/ATH/ATE/ATS0/ATZ/+++)
4. **Registry**: Change all key paths to WinFOSSIL paths
5. **Node → Port**: NetModem uses "nodes" (multi-node BBS),
   WinFOSSIL uses "ports" (COM1-COM4). Rename throughout.
6. **TCP transport**: NM_SynapseLink uses Synapse sockets.
   WinFOSSIL uses Winsock directly. Adapt or keep Synapse.
7. **INT 14h dispatch**: NM_Int14ISR already implements this.
   Just verify function numbers match FTS-0017.

#### C. CPL Applet (NetModemCPL.pas → WNFOSSIL.CPL)

1. **Applet name**: `NetModem Configuration` → `WinFOSSIL`
2. **Registry key**: Change to WinFOSSIL path
3. **Icon**: Use WinFOSSIL icon (or keep — both are serial port icons)
4. **Dialog fields**: Match original WinFOSSIL CPL:
   - Port selector (COM1-COM4)
   - Baud rate combo (300-115200)
   - Lock baud checkbox
   - Buffer sizes (RX/TX)
   - Auto-open, Keep open, Timeslice, Perf stats checkboxes
5. **Registration dialog**: Remove (GPLv3, always registered)

#### D. Control Utility (ConfigMain.pas → WNFOSCTL.EXE)

1. **Commands**: Keep LOCK/UNLOCK, add STATUS
2. **Usage**: `wnfosctl <port> [LOCK <baud> | UNLOCK]`
3. **Registry**: Change key paths
4. **Branding**: `WinFOSSIL Control Utility`

#### E. Installer (netmodem2irc.iss → wnfossil.iss)

1. **App name**: `NetModem/32` → `WinFOSSIL`
2. **Files**: Adapt file list per release (v1.12 / v1.0 NT / v2.0)
3. **Registry entries**: Per version (VxD path vs Woodruff path)
4. **AUTOEXEC.NT**: NT version adds `lh wnfossil.exe`
5. **Uninstall**: Match original INF uninstall strings
6. **Three .iss files**: One per release

#### F. Icons and Resources

All icons/bitmaps from netmodem2irc/config/resources/ are reusable:
```
mainicon.ico       Main application icon
comports.ico       COM port icon (for CPL)
connection.ico     Connected state
options.ico        Settings
logging.ico        Log viewer
active.bmp         Active indicator
inactive.bmp       Inactive indicator
established.bmp    Connection established
pending.bmp        Connection pending
error.bmp          Error state
```

### What's NOT Needed (strip from NetModem)

- SEAM protocol (NetModem-specific inter-node messaging)
- AutoNews (NetModem-specific BBS news feature)
- Listserv (NetModem-specific mailing list)
- Com0Com link (virtual null-modem — keep as optional)
- DebugView (1,455 lines — nice to have but not in original)
- ServerBridge/ServerLink (NetModem server architecture)

### License

All netmodem2irc code is GPLv3 (see LICENSE file in repo).
The WinFOSSIL rebuild inherits this license. The original
WinFOSSIL by Bryan Woodruff was shareware ($15/$25 registration).
Our rebuild is free software — no registration, no nag screens.

---

## 12. Registry Path Changes (All Versions)

```
Version    Registry Key                                          Platform
───────    ────────────────────────────────────────────────────   ────────
v1.12      HKLM\System\CurrentControlSet\Services\VxD\FOSSIL    Win95/98
v1.0 NT    HKLM\Software\Woodruff\WinFOSSIL                     NT4/2000
v2.0       HKLM\SOFTWARE\WinFOSSIL                               Win7-11
```

Per-port values (all versions):
```
enabled                 DWORD    0/1
port name               SZ       "COM1"
locked baud             DWORD    0 or baud rate
receive buffer size     DWORD    4096
transmit buffer size    DWORD    4096
last port selected      DWORD    0-3
```

v1.12-specific:
```
StaticVxD               SZ       "fossil.vxd"
Start                   DWORD    0x00
```

v1.0 NT-specific:
```
Installed               DWORD    0x01
```

v2.0-specific:
```
AutoOpen                DWORD    Auto-open on first access
KeepOpen                DWORD    Keep open between sessions
TimeSlice               DWORD    Yield CPU when idle
PerfStats               DWORD    Performance monitoring
```

## 13. Access Security (v2.0 New Feature)

Registry: `HKLM\SOFTWARE\WinFOSSIL\Security`

```
RequireAdmin            DWORD    Config changes need elevation
LogAccess               DWORD    Log open/close/dial events
MaxConnections          DWORD    Max TCP connections per port
TCPWhitelist            MULTI_SZ Allowed IP prefixes
TCPBlacklist            MULTI_SZ Blocked IP prefixes
```

Security flow:
```
TCP connect in → check blacklist → check whitelist → check max_conn → allow/deny
Config change  → check RequireAdmin → UAC prompt if needed → allow/deny
Any access     → if LogAccess → write Windows Event Log
```

Blacklist takes priority over whitelist.
Empty whitelist = allow all (unless blacklisted).

Not present in v1.12 or v1.0 NT — those versions have no security.
v2.0 defaults: RequireAdmin=0, LogAccess=0, MaxConnections=4.

---

## 14. COM Port Differences by Platform

| Feature | Win95/98 | NT4/2000 | XP-Win11 |
|---------|----------|----------|----------|
| Port name | `"COM1"` | `"\\\\.\\COM1"` | `"\\\\.\\COM1"` |
| Overlapped I/O | Unreliable | Works | Works |
| fAbortOnError default | FALSE | TRUE | TRUE |
| SetupComm | Ignored by some drivers | Works | Works |
| Port enumeration | Brute force | QueryDosDevice | QueryDosDevice/SetupDi |
| USB-serial | No | Rare | Common (COM5+) |
| High COM numbers | COM1-4 | COM1-256 | COM1-256+ |
| HANDLE size | 32-bit | 32-bit | 32/64-bit |
| CancelIo | Not available | Available | Available |
| PURGE_RXABORT | Not reliable | Works | Works |

### Critical bugs in original WinFOSSIL (fixed in our code):
1. NT4 `fAbortOnError=TRUE` default — reads fail on parity errors
2. Port name format — `"COM10"` fails without `"\\\\.\\COM10"` prefix
3. No `ClearCommError` after failed reads — error state persists
4. No `CancelIo` on timeout — overlapped requests leak on 9x

### Our implementation (`comport_compat.c`):
- `format_port_name()` — correct format per platform
- `wfp_com_open()` — synchronous on 9x, overlapped on NT+
- `wfp_com_read()` — timeout handling per platform
- `wfp_com_write()` — CancelIo on timeout (NT+ only)
- `wfp_com_status()` — full modem + line + flow status mapping
- `wf_enum_ports()` — brute force (9x) or QueryDosDevice (NT+)

---

## 15. Feature Comparison: v1.12 (Win98) vs v1.0 NT vs v2.0

### v1.12 Win95/98 Features (COMPLETE — fully shipped):

| Feature | Status |
|---------|--------|
| COM port FOSSIL (Fn 00h-1Bh) | ✅ Full |
| Performance statistics counters | ✅ 5 counters per port |
| Performance stats in CPL | ✅ Enabled |
| Reflect COM port state to VCD | ✅ VCD passthrough |
| VM wakeup tracking | ✅ Tracks sleeping VMs |
| Port contention handling | ✅ Via VCOMM |
| VxD ring-0 I/O | ✅ Direct UART/FIFO |
| Timeslice release | ✅ Both idle + carrier |
| Keep open during MS-DOS session | ✅ |
| Auto-open on first access | ✅ |
| Registration/nag screen | ✅ MD5+RC4 check |
| Baud lock/unlock | ✅ Via CPL + WNFOSCTL |
| Buffer size config | ✅ RX+TX separate |

### v1.0 NT (BETA 3 — incomplete):

| Feature | Status | Notes |
|---------|--------|-------|
| COM port FOSSIL (Fn 00h-1Bh) | ✅ Full | Via VDD/NTVDM |
| VMODEM telnet | ✅ Partial | ATZ, ATD work. ATH0 NOT implemented |
| Performance statistics | ❌ **Not enabled** | From BUGS.TXT |
| 32-bit app COM handle passing | ❌ **Not working** | 16-bit apps only |
| Multi-node VMODEM answer | ❌ **Timing bug** | Race between Winsock and VDMs |
| Maximus 3.01 compatibility | ❌ **Hangs VMODEM** | Unresolved |
| Reflect COM to VCD | N/A | No VCD on NT |
| VM wakeup tracking | N/A | Different VM model |
| Timeslice release | ✅ | Via Sleep() |
| Keep open during DOS session | ✅ | |
| Auto-open on first access | ✅ | |
| Registration/nag screen | ✅ | Same MD5+RC4 |
| Baud lock/unlock | ✅ | Via CPL + WNFOSCTL |
| VMODEM multi-node listen | ✅ Partial | WinFOSSIL::Listen::Mutex |
| Background I/O threads | ✅ | commReadThread + commWriteThread |

### v2.0 Modern (our rebuild — all features):

| Feature | Status | Notes |
|---------|--------|-------|
| COM port FOSSIL (Fn 00h-1Bh) | ✅ Full | Via CreateFile |
| VMODEM telnet | ✅ Full | ATZ/ATD/ATH/ATE/ATS0/ATA/+++/ATI |
| Performance statistics | ✅ **Fixed** | CPS + peak + totals + session time |
| 32-bit app support | ✅ **Fixed** | Native DLL, no NTVDM needed |
| 64-bit app support | ✅ **New** | x64 build |
| Multi-node VMODEM | ✅ **Fixed** | Proper mutex + event sync |
| ATH0 hangup | ✅ **Fixed** | Was missing in NT beta |
| Access security | ✅ **New** | Whitelist/blacklist/admin/logging |
| System tray icon | ✅ **New** | Right-click menu, status tooltip |
| Port enumeration | ✅ **New** | Detects USB-serial adapters |
| Registry compat | ✅ **New** | Runtime path selection Win98-Win11 |
| COM port compat | ✅ **New** | Overlapped on NT+, sync on 9x |
| Registration | ✅ **Removed** | GPLv3 — always registered |
| Telnet IAC filtering | ✅ Full | WILL/WONT/DO/DONT stripped |
| DOSBox compatibility | ✅ **New** | Works without NTVDM |

### Summary of what NT was missing (fixed in v2.0):

1. **Performance statistics not enabled** — counters existed in code but
   were disabled. v2.0 implements full CPS tracking with peak values.
2. **ATH0 not implemented** — could only disconnect via DTR drop. v2.0
   implements ATH and ATH0 properly.
3. **32-bit to 16-bit COM handle passing broken** — architectural issue
   with NTVDM port inheritance. v2.0 sidesteps this entirely with native
   Win32 DLL (no NTVDM).
4. **Multi-node VMODEM race condition** — timing bug between Winsock
   and multiple DOS VDMs. v2.0 uses proper mutex + event signaling.
5. **Maximus 3.01 hangs** — VMODEM state machine deadlock. v2.0
   has timeout on all state waits (wf_vm_wait_state with timeout_ms).

### Three build targets:

```
out/win98/    v1.12 recreation (Win95/98/ME)
out/nt/       v1.0 recreation (NT4/2000) — with NT bugs FIXED
out/i386/     v2.0 modern 32-bit (XP through Win11)
out/x64/      v2.0 modern 64-bit (Win7 x64 through Win11)
```
