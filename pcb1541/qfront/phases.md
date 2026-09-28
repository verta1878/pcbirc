# QFront Clean-Room Recovery — Phase Plan v2

## Based on: BINARY_ANALYSIS.md (266 lines), QFRONT.DOC (300KB), HISTORY.DOC (606 lines)

---

## Architecture

Original QFront was a 40,000-line Borland Pascal monolith that did
everything: modem control, EMSI/WaZOO handshake, Zmodem/Xmodem,
BSO queue, event scheduler, nodelist compiler, tosser, TUI config
editor, Wildcat! integration.

Our recovery is a ~2,500-line C orchestrator that delegates:

```
┌─────────────────────────────────────────────┐
│                 QFront (C)                  │
│                                             │
│  Config ← qfront.cfg (INI text file)       │
│  BSO Scanner ← FTS-5005 implementation     │
│  Event Scheduler ← 19 flags from binary    │
│  Session Dispatch ← calls binkd             │
│  Nodelist Lookup ← FTS-5001 parser         │
│  Routing ← QTRANS.DAT equivalent           │
│  Semaphores ← QQUEUE/QPOLLED/QUNDIAL      │
│  Logging ← QFRONT.LOG compatible           │
│  Post-session ← calls tosser + TIC proc    │
│                                             │
│  DOES NOT IMPLEMENT:                        │
│    Modem/UART/FOSSIL (IP only)             │
│    EMSI/WaZOO (binkd does BinkP)          │
│    Zmodem/Xmodem (binkd does transfer)    │
│    TUI config editor (text config)         │
│    Wildcat! API (generic hooks)            │
└─────────────────────────────────────────────┘
         │              │              │
    ┌────▼────┐   ┌─────▼─────┐  ┌────▼────┐
    │  binkd   │   │  tosser   │  │  htick  │
    │ (BinkP)  │   │ (hpt/     │  │  (TIC)  │
    │          │   │  pcbtoss) │  │         │
    └──────────┘   └───────────┘  └─────────┘
```

---

## Phase A: Config + BSO Scanner + Orchestrator [DONE — 1,045 lines]

```
✓ qfront.h     203 lines  FTN_ADDR, PKT_HEADER (FTS-0001), BSO types,
                           config struct, event struct, all prototypes

✓ bso.c        367 lines  FTS-5005 outbound scanner:
                           — Scan outbound dir for .?ut/.?lo/.req files
                           — 5 flavours: immediate/continuous/direct/normal/hold
                           — .bsy lock (exclusive create, PID, FTS-5005 §5.1)
                           — .hld hold check with UNIX expiry (FTS-5005 §5.3)
                           — .try attempt counter (NOK/NBAD, FTS-5005 §5.4)
                           — Poll creation (touch empty .?lo)
                           — Zone directory mapping (outbound.ZZZ)
                           — Point subdirectory support (NNNNNNNN.PNT/)

✓ qfront.c     475 lines  Main orchestrator:
                           — INI config loader (25 keys)
                           — Session dispatch (lock → exec binkd → unlock)
                           — Post-session (run tosser + TIC processor)
                           — Event scheduler (day/time/flags check)
                           — CLI: -c config, -p poll, -s single-pass, -d debug
                           — Logging with timestamps and levels
```

## Phase B: Nodelist Lookup + Routing [~500 lines]

### nodelist.c (~350 lines)

Parse FTS-5001 raw nodelist for address→info lookup.

```
Functions:
  nl_open()           Open and index a nodelist file
  nl_lookup()         Find a node by zone:net/node address
  nl_close()          Free nodelist resources

Returns for each node:
  phone               Phone number (or IP:port for IP nodes)
  sysop               Sysop name
  system_name         BBS/system name
  location            City/state
  speed               Max baud rate
  flags               Capability flags string

Flags we care about (from binary: "CM", "MO", "LO"):
  CM                  Continuous Mail (available 24hr)
  MO                  Modem Only (no IP)
  LO                  Listed Only (don't call unless listed)
  IBN                 BinkP capable (IP node)
  INA                 IP address available

Nodelist entry types (from QNLIST.EXE binary):
  Zone,<n>            Zone header
  Region,<n>          Region header
  Host,<n>            Net header
  Hub,<n>             Hub node
  ,<n>                Normal node
  Pvt,<n>             Private node (unlisted phone)
  Down,<n>            Down/unavailable
  Hold,<n>            Hold (don't call)
```

**FTS references:** FTS-5001 (flags), FSC-0087 (extended flags)

### route.c (~150 lines)

Address routing table — equivalent to QTRANS.DAT.

```
Functions:
  rt_load()           Load routing rules from config
  rt_resolve()        Resolve destination for an address
                      Returns: direct (call node) or via (route through hub)

Rule syntax (in qfront.cfg):
  Route 1:*           via 1:234/0       Route all zone 1 through net host
  Route 2:5020/*      via 2:5020/0      Route net through host
  Route 1:234/99      direct            Call this node directly
  NoPoll 1:234/100                      Never poll this node

Maps to original:
  QTRANS.DAT          Address translation table
  "NetMail routing control" (from QFCONFIG binary)
  " marked PRIVATE, DOWN or HOLD" (skip these)
  " is flagged undialable" (skip these)
```

## Phase C: Event Scheduler [~400 lines]

### events.c (~250 lines)

Full event scheduler with all 19 flags from QFCONFIG binary analysis.

```
Event config syntax (in qfront.cfg):
  [Event.ZMH]
  Days=MTWTFSs                    Day mask (MTWT FSs = Mon-Sun)
  Start=01:00
  End=05:00
  ForcePolI=yes
  SendCMOnly=yes
  ExitNoMail=yes
  Errorlevel=3

All 19 flags implemented:
  SCHEDULING:
    SlideTime           Adjust start if late

  MAIL FILTERING:
    SendEchoOnly        Only send echomail
    SendNetOnly         Only send netmail  
    EchoOnly            Only process echomail
    NetOnly             Only process netmail
    ReceiveOnly         Accept incoming, don't dial out

  NODE FILTERING:
    SendCMOnly          Only call CM (Continuous Mail) nodes
    SendNonCMOnly       Only call non-CM nodes
    NodeCritical        Event is critical for specific nodes
    NoHeldAttach        Don't send held file attachments

  ACTIONS:
    ForcePoll           Force outbound poll
    ScanBeforeEvent     Run tosser before event starts
    CompileNodelist     Compile nodelist during event
    CompileOnReceive    Auto-compile when nodediff arrives
    AutoPoll=<addrs>    List of nodes to auto-poll

  EXIT CONDITIONS:
    EndNoMail           End event when queue empty
    ExitNoMail          Exit program when queue empty
    ExitWhenDone        Exit loop when queue drains
    Errorlevel=<n>      Exit with specific code

Errorlevel exits (from binary):
    0   Normal exit
    3   FidoMail received (run tosser)
    5   Fax call received
    7   Nodelist received (run compiler)
    ?   Semaphore file found (configurable)
    ?   Human caller (configurable per-event)
```

### semaphore.c (~150 lines)

Multi-node semaphore management — prevents conflicts when multiple
QFront instances run on different nodes.

```
Functions:
  sem_check()         Check for semaphore files
  sem_create()        Create semaphore
  sem_remove()        Remove semaphore

Data files (from binary):
  QQUEUE.DAT          Outbound queue serialization
                      Prevents two nodes from dialing same system
  QPOLLED.DAT         Recently polled nodes
                      Prevents re-polling within cooldown period
  QUNDIAL.DAT         Undialable nodes
                      Tracks nodes that exceeded max retries
  QFIXUPS.DAT         Fixup queue
                      Incomplete transfers to retry later

Semaphore exit files (from binary/doc):
  MAKEWILD.DAT        Wildcat! integration trigger
  <configurable>      "Semaphore file exit" — any filename triggers exit
                      "<file> found, exiting with errorlevel <n>"
```

## Phase D: TIC Processor Integration [~200 lines]

### tic.c

```
Functions:
  tic_scan_inbound()  Scan inbound for .TIC files
  tic_process()       Parse TIC, move file, forward to downlinks
  tic_call_external() Call external TIC processor (htick/pcbtic)

TIC file format:
  Origin <addr>
  From <addr>
  To <addr>
  File <filename>
  Area <areaname>
  Desc <description>
  CRC <hex>
  Path <addr> <timestamp>
  Seenby <addr>
  Pw <password>
```

## Phase E: Logging + Status [~350 lines]

### log.c (~200 lines)

Enhanced logging — QFRONT.LOG compatible format.

```
Log format (from binary analysis):
  "[ QFRONT SYSTEM LOG ]"           Log header
  Timestamped entries with:
    Session summaries (bytes sent/received, duration)
    "Successfully sent packet(s)/file(s)"
    "Attempt to send packet(s)/file(s) was unsuccessful"
    "Error sending <file>"
    "Error receiving <file>"
    "Error during session"
    Per-node attempt tracking

Debug log (QFRONT.DBG):
  Enabled with /DEBUG or Debug=1
  Verbose protocol-level detail
```

### status.c (~150 lines)

Console status display (text mode, no curses).

```
Status lines (from binary):
  "Waiting for a call"
  "Dialing <address>"
  "Sending file(s)/mail packet(s)"
  "Running event <name>"
  "Ending event <name>"
  "Building queue"
  Current node address and session info
  Outbound queue count
  Last caller info
```

---

## Estimated Line Counts

```
Phase A (DONE):    1,045 lines
Phase B:             500 lines  (nodelist 350 + routing 150)
Phase C:             400 lines  (events 250 + semaphores 150)
Phase D:             200 lines  (TIC integration)
Phase E:             350 lines  (logging 200 + status 150)
                   ─────────
Total:             2,495 lines
```

## Build

```bash
# Linux (primary)
gcc -Wall -O2 -o qfront src/*.c

# DOS (OpenWatcom)
wcl386 -ox -bt=dos -l=dos4g src/*.c

# Windows (OpenWatcom)
wcl386 -ox -bt=nt -l=nt src/*.c
```

## Verification Matrix

| Feature | Doc Page | Binary String | FTS Spec |
|---------|----------|--------------|----------|
| BSO 5 flavours | p.47 events | i/c/d/f/h extensions | FTS-5005 §3.2 |
| .bsy lock | p.43 semaphores | ".BSY" | FTS-5005 §5.1 |
| .hld hold | "undialable" | QUNDIAL.DAT | FTS-5005 §5.3 |
| .try counter | "retry" | "Retry" | FTS-5005 §5.4 |
| PKT header | "mail packet" | "*.PKT" | FTS-0001 §4 |
| Nodelist parse | p.126 compiler | ZONE/REGION/HOST/HUB | FTS-5001 |
| 19 event flags | p.47 event setup | QFCONFIG strings | N/A (QFront) |
| Errorlevels | p.22 events | "exiting with errorlevel" | N/A (QFront) |
| Semaphore files | p.43 | QQUEUE/QPOLLED/QUNDIAL | N/A (QFront) |
| Routing | p.37 routing | "NetMail routing" | N/A (QFront) |
| Display texts | p.95 display | 8 .TXT filenames | N/A (QFront) |
| Data files | throughout | 10 .DAT filenames | N/A (QFront) |
| CLI options | p.24 loading | /DEBUG /LOCALONLY etc. | N/A (QFront) |
| Multitasker | "DESQview" | 5 detected types | N/A (DOS-era) |
| Areafix | p.86 | AREAFIX.TXT | FSC-0057 |
| File requests | p.91 | ".REQ" | FTS-5005 §4 |


---

## Phase F: Modem + Serial Port Control [~800 lines]

From binary: "Initializing modem", "Opening port", "Enabling 16550 UART",
"Using fossil for communications", "Using UART for communications",
"Using DigiBoard for communications"

### modem.c (~500 lines)

```
Functions:
  mdm_init()           Send AT init string, wait for OK
  mdm_dial()           Send ATD+number, wait for CONNECT/BUSY/etc
  mdm_answer()         Send ATA, wait for CONNECT
  mdm_hangup()         Drop DTR or send +++ATH0
  mdm_reset()          Reset modem to known state
  mdm_send_cmd()       Send AT command, wait for response
  mdm_parse_response() Parse: OK, CONNECT <speed>, RING, NO CARRIER,
                       BUSY, NO DIALTONE, NO ANSWER, VOICE, ERROR,
                       CONNECT FAX, RINGING

Modem responses (from binary):
  "Modem response - BUSY"
  "Modem response - CONNECT"
  "Modem response - NO ANSWER"
  "Modem response - NO CARRIER"
  "Modem response - NO DIALTONE"
  "Modem response - RING"
  "Modem response - RINGING"
  "Modem response - VOICE"
  "Called modem is busy"
  "Connect speed = <n>"
  "Carrier speed = <n>"
  "Connect string = <str>"
  "Ring string = <str>"
  "Call is DATA" / "Call is VOICE" / "CONNECT FAX"

Error handling:
  "Modem initialization error, retry <n>"
  "Modem initialization aborted by sysop"
  "Unable to initialize modem"
  "Modem - no command registered"
  "Unexpected response in init"
  "Failed to train with remote modem"

Config fields:
  ModemInit=ATS0=0M0DT          Primary init string
  ModemInit2=                    Secondary init string
  DialPrefix=ATDT                Dial command prefix
  DialSuffix=                    After phone number
  AnswerCmd=ATA                  Answer command
  HangupCmd=ATH0                 Hangup command
  ResetMinutes=5                 Auto-reset interval
  MaxRedials=10                  Max redial attempts
  RedialWait=60                  Seconds between redials
  ConnectWait=60                 Seconds to wait for CONNECT
```

### serial.c (~300 lines)

```
Functions:
  ser_open()           Open COM port (or FOSSIL)
  ser_close()          Close port
  ser_read()           Read byte(s) with timeout
  ser_write()          Write byte(s)
  ser_set_baud()       Set baud rate
  ser_set_flow()       Set RTS/CTS or XON/XOFF
  ser_get_dcd()        Check Data Carrier Detect
  ser_set_dtr()        Set/clear DTR
  ser_flush()          Flush buffers
  ser_break()          Send break signal

Port types (from binary):
  "Using UART for communications"      Direct 16550 UART
  "Using fossil for communications"    FOSSIL driver (INT 14h)
  "Using DigiBoard for communications" DigiBoard proprietary
  "Serial device"
  "16550 UART not found"
  "Enabling 16550 UART buffer"
  "Disabling 16550 UART"
  "Warning! Port speed >= 38400 baud with no 16550 UART"
  "Non-standard port in use"
  "Base address = <hex>"

#ifdef _WIN32
  Win32: CreateFile("COM<n>"), SetCommState, ReadFile/WriteFile
#else
  Linux/DOS: open("/dev/ttyS<n>") or FOSSIL INT 14h
#endif
```


## Phase G: EMSI + WaZOO + File Requests [~900 lines]

### emsi.c (~400 lines)

EMSI handshake — FSC-0056 specification.

```
State machine (from binary):
  1. Send **EMSI_INQ (with CRC: **EMSI_INQC816)
  2. Wait for **EMSI_REQ (with CRC: **EMSI_REQA77E)
     or timeout → fall back to FTS-0001/YooHoo
  3. Send EMSI_DAT packet (our capabilities, address, password)
  4. Wait for EMSI_DAT from remote
  5. Validate CRC, check password
  6. Send EMSI_ACK / EMSI_NAK
  7. Session established → transfer files

Packet format:
  {EMSI}{<hex_length>}{<data>}{<CRC32>}

Commands:
  EMSI_INQ   Inquiry — "are you EMSI capable?"
  EMSI_REQ   Request — "yes, send your EMSI_DAT"
  EMSI_DAT   Data — capabilities, address list, password
  EMSI_ACK   Acknowledge — session accepted
  EMSI_NAK   Negative ack — retry
  EMSI_HBT   Heartbeat — keep alive
  EMSI_CLI   Client info (optional)

Data files:
  EMSI-IN.DAT    Last received EMSI packet
  EMSI-OUT.DAT   Last sent EMSI packet

Functions:
  emsi_handshake()     Full EMSI negotiation
  emsi_send_inq()      Send EMSI_INQ
  emsi_send_dat()      Build and send EMSI_DAT
  emsi_recv_dat()      Receive and parse EMSI_DAT
  emsi_send_ack()      Send EMSI_ACK
  emsi_calc_crc()      CRC-16 or CRC-32 calculation
  emsi_parse_dat()     Parse capability string
```

### wazoo.c (~300 lines)

WaZOO/YooHoo session — FTS-0006 specification.

```
State machine (from binary):
  1. Send YooHoo hello packet (128 bytes with CRC)
  2. Wait for hello packet from remote
  3. Exchange capabilities
  4. Session established

Hello packet fields (FTS-0006):
  Signal (2 bytes)     0x6F (ASCII 'o')
  Product code         Mailer identification
  Serial number        Registration number
  Node address         Zone:Net/Node.Point
  Node name            System name (60 bytes)
  Sysop name           Sysop name (20 bytes)
  Password             Session password (8 bytes)
  Capabilities         Bit flags
  CRC-16               Packet checksum

Messages (from binary):
  "Sending hello packet" / "Sent hello packet"
  "Receiving hello packet" / "Received hello packet"
  "Bad CRC value in hello packet"
  "YooHoo capabilities = <hex>"
  "Established YooHoo protocol"
  "Lost carrier/Max retries exceeded"

Functions:
  wazoo_handshake()    Full YooHoo negotiation
  wazoo_send_hello()   Build and send hello packet
  wazoo_recv_hello()   Receive and validate hello
  wazoo_check_crc()    CRC-16 validation
```

### frequest.c (~200 lines)

File request processing — .REQ files and magic filenames.

```
Data files (from binary):
  *.REQ              Request file (list of filenames to send)
  QMAGIC.DAT         Magic filename aliases
  QRLIMIT.DAT        Request limits (bytes/files per session/day)

Messages (from binary):
  "Processing request file"
  "Requested file(s) <list>"
  "Found magic file <name>"
  "Only allowing requests from <addr>"
  "Requests not allowed during this event"
  "Requests not allowed from unlisted system"
  "Pickup requests not allowed during this event"
  "Connect speed too low for file requests"
  "Minimum connect speed required for requests is <n>"
  "Maximum bytes in requests reached for file <name>"
  "Maximum number of requests reached"
  "File request" / "File update request"
  "Bytes in previous requests today: <n>"
  "Files in previous requests today: <n>"

Functions:
  freq_process()       Process incoming .REQ file
  freq_check_magic()   Look up magic filename in QMAGIC.DAT
  freq_check_limits()  Check QRLIMIT.DAT (bytes/files/security)
  freq_build_req()     Build outgoing .REQ file
```


## Phase H: Zmodem + Xmodem File Transfer [~1,200 lines]

### zmodem.c (~800 lines)

Zmodem file transfer — public Zmodem specification.

```
Features (from binary):
  8k block size        "Using Zmodem 8k block size"
  CRC variants         CrcE, CrcG, CrcQ, CrcW DataSubpackets
  Resume support       "Attempting resume" / "Resuming partial transfer"
  Skip support         "Zmodem - skip file"
  File position        "Zmodem - bad file position"

Error messages (from binary):
  "Zmodem - specified file does not exist"
  "Zmodem - not allowed to overwrite file"
  "Zmodem - never got proper handshake"
  "Zmodem - got garbage from remote"
  "Zmodem - no files to receive"

Transfer status (from binary):
  "Protocol      : Zmodem"
  "Block check   : <type>"
  "Block errors    : <n>"
  "Block size    : <n>"
  "Blocks remaining: <n>"
  "Bytes remaining : <n>"
  "Progress      : <pct>"
  "Elapsed time  : <hms>"
  "Estimated time: <hms>"
  " CPS" (characters per second)

Functions:
  zm_send_files()      Send file list via Zmodem
  zm_recv_files()      Receive files via Zmodem
  zm_send_zrinit()     Initiate receive
  zm_send_zfile()      Send file header
  zm_send_zdata()      Send file data blocks
  zm_send_zeof()       End of file
  zm_send_zfin()       End of session
  zm_recv_header()     Receive and parse header
  zm_parse_subpacket() Parse CrcE/G/Q/W subpackets
  zm_calc_crc()        CRC-16/CRC-32
  zm_resume()          Resume interrupted transfer
```

### xmodem.c (~400 lines)

Xmodem, Xmodem-1K, XmodemCRC, SEAlink file transfer.

```
Variants (from binary):
  "Xmodem"            Standard 128-byte blocks, checksum
  "Xmodem1K"          1024-byte blocks
  "XmodemCRC"         128-byte blocks with CRC-16
  "SEAlink"           Xmodem with FidoNet extensions (sliding window)
  "SEAlink-O"         Overdrive variant

Messages (from binary):
  "Xmodem init failed"
  "Xmodem init was canceled on request"
  "Duplicate block received"
  "Wrong block number received"
  "Block shorter than requested"

Functions:
  xm_send_file()       Send single file
  xm_recv_file()       Receive single file
  xm_send_block()      Send one block (128 or 1024)
  xm_recv_block()      Receive and validate block
  xm_calc_checksum()   Simple checksum
  xm_calc_crc16()      CRC-16 for XmodemCRC
  sea_send_file()      SEAlink send (sliding window)
  sea_recv_file()      SEAlink receive
```


---

## Updated Line Estimates

```
Phase A (DONE):      1,045 lines  — Config, BSO, orchestrator
Phase B (DONE):        500 lines  — Nodelist, routing
Phase C (DONE):        400 lines  — Events, semaphores
Phase D (DONE):        200 lines  — TIC integration
Phase E (DONE):        350 lines  — Logging, status, inbound scan
Phase F (DONE):        800 lines  — Modem + serial port
Phase G (DONE):        900 lines  — EMSI + WaZOO + file requests
Phase H (DONE):      1,200 lines  — Zmodem + Xmodem
                   ─────────────
Current:             2,796 lines  (Phase A-E complete)
Remaining:           2,900 lines  (Phase F-H)
Total:               5,696 lines  (full QFront replacement)
```


---

## Phase 2: QFront Suite — Remaining 4 Programs

### Estimated Scope

| Program | Original | Est. Lines | Priority |
|---------|----------|------------|----------|
| QFUTIL | 63 KB | ~400 | 1 (smallest, CLI only) |
| QNLIST | 255 KB | ~1,200 | 2 (nodelist compiler) |
| QSCAN | 139+83 KB | ~2,500 | 3 (tosser — core mail processing) |
| QFCONFIG | 249+242 KB | ~4,000 | 4 (TUI config editor — 50+ screens) |
| **Total** | **1,302 KB** | **~8,100** | |

### All compile with OpenWatcom only. No GCC.

---

## QFUTIL — CLI Queue/NetMail Utility (~400 lines)

### Commands (from binary)
```
QFUTIL /ADDR:<zone:net/node> <options>

Queue operations:
  /POLL              Create manual poll (.ilo touch)
  /REQUEST           File request (create .REQ)
  /UREQUEST          Update request (newer files only)
  /FORWARD           File forward
  /FILE:<files>      Comma-separated file list
  /FLAGS:<flags>     IMM,HOLD,ABSHOLD,CRASH,DIR,KFS
  /PWRD:<password>   Request password
  /FREQ:<days>       Repeat request every N days

NetMail operations:
  /NETMAIL:<file>    Convert text file → .MSG netmail
  /FROM:<name>       Sender name (use _ for spaces)
  /TO:<name>         Recipient name
  /SUBJ:<subject>    Subject line

Display:
  /COLOR             Force color output
  /MONO              Force monochrome
  /HELP              Show usage
```

---

## QNLIST — Nodelist Compiler (~1,200 lines)

### Operations (from binary)
```
QNLIST /COMPILE       Compile primary + private nodelists
QNLIST /COMPILENEW    Compile only if new diffs found
QNLIST /COLOR         Color mode
QNLIST /MONO          Mono mode
```

### Process flow
```
1. Check for new nodediff archives in inbound
2. Unarchive nodediff
3. Check CRC of old nodelist
4. Apply nodediff to nodelist
5. Check CRC of new nodelist
6. Process private nodelists/pointlists
7. Sort by node number
8. Sort by sysop name
9. Build compiled index (V7/V7+ format)
10. Clean up temp files
```

### Data files
```
NODELIST.???     Raw FTS-5001 nodelist (day number extension)
NODEDIFF.???     Nodelist difference file
*.PNT            Point lists
V7 index:        NODEX.NDX, SYSOP.NDX, NODEX.DAT
```

---

## QSCAN — EchoMail Tosser/Scanner (~2,500 lines)

### CLI Options (from binary)
```
QSCAN /TOSS          Toss inbound packets → message base
QSCAN /SCAN          Scan message base → outbound packets
QSCAN /BOTH          Toss + Scan
QSCAN /NETMAIL       Process NetMail only
QSCAN /AREA:<name>   Process specific area only
QSCAN /FORCE         Force rescan
QSCAN /RESET         Reset high-water marks
QSCAN /SETHIGH       Set high pointers
QSCAN /DEBUG         Debug output
QSCAN /COLOR         Color mode
QSCAN /MONO          Mono mode
```

### Toss operations
```
1. Open inbound .PKT files
2. Parse FTS-0001 packet header
3. Extract messages from packet
4. Route each message to correct echomail area
5. Check for duplicates (DUPES.DAT)
6. Insert into Wildcat!/PCBoard message base
7. Update SEEN-BY and PATH lines
8. Handle areafix commands
9. Move processed packets to BAD/ on error
10. Archive outbound bundles
```

### Scan operations
```
1. Read high-water marks per area
2. Scan message base for new messages above mark
3. Build outbound .PKT files (FTS-0001 format)
4. Add SEEN-BY and PATH lines
5. Route to downlinks per echomail config
6. Archive into bundles per schedule
7. Update high-water marks
```

---

## QFCONFIG — TUI Configuration Editor (~4,000 lines)

### 50+ Menu Screens (from binary)
```
Main menus:
  [ GENERAL ]              System paths, addresses, passwords
  [ FIDOMAIL SETUP ]       Mail processing options
  [ MODEM/DIALOUT ]        Modem init, dial, speed
  [ DISPLAY SETUP ]        Screen mode, fonts
  [ PROGRAM SETUP ]        Shell, swap, keyboard
  [ NETMAIL ]              NetMail conference setup
  [ NETMAIL ROUTING ]      Route rules
  [ MAIL SCANNER ]         Scan/toss options
  [ NODELISTS ]            Nodelist paths, private lists
  [ FILE REQUEST SETUP ]   Magic names, limits, paths
  [ EVENTS ]               Event scheduler
  [ COLOR SETUP ]          TUI color customization

List editors (ALT-D delete, ALT-I insert):
  [ ADDRESSES ]            AKA list
  [ ALIASES ]              Address aliases
  [ ARCHIVERS ]            ZIP/LZH/ARJ/RAR config
  [ AREA MANAGER ]         EchoMail area list
  [ AREAFIX SETUP ]        Areafix configuration
  [ AREAFIX UPLINKS ]      Uplink list
  [ AUTOMATIC POLLS ]      Auto-poll list
  [ DIALOUT FIXUPS ]       Phone number fixups
  [ EXTERNAL MAIL STRINGS ] BBS integration
  [ FUNCTION KEYS ]        F1-F12 hotkeys
  [ MAGIC FILENAMES ]      File request aliases
  [ NODE MANAGER ]         Per-node settings
  [ ORIGIN LINES ]         Origin line list
  [ PRIVATE NODELISTS ]    Private nodelist paths
  [ QUICK LOOKUP NAMES ]   Address book
  [ SEMAPHORE FILES ]      Exit triggers
  [ TRANSLATION/COSTING ]  Phone translations
  [ TRASHCAN USERS ]       Blocked users
  [ USER NETMAIL FLAG OVERRIDES ] Per-user flags
  [ USERNAMES TO IGNORE ]  Scan filter
```

### TUI Framework needed
```
- Text-mode windowed UI (80x25 or 80x50)
- Menu bar with hotkeys
- Scrollable list views with ALT-D/ALT-I/SPACEBAR
- Field editor with TAB navigation
- Help system (F1 = context help)
- Color scheme support (COLOR/MONO)
- VGA font loading
- Mouse support (optional)
- Save/load NODE1.CFG binary format
```


---

## Phase I: QFUTIL — Utility Commands [~400 lines]

CLI tool replacing QFUTIL.EXE. From binary:

```
Commands:
  /POLL              Create .ilo poll file for a node
  /NETMAIL           Create netmail .MSG from command line
  /FORWARD           Forward netmail to another node
  /FREQ              File request (create .REQ)
  /REQUEST           Same as /FREQ
  /UREQUEST          Update request (only newer files)
  /HELP              Show usage

Options:
  /ADDR:<zone:net/node>   Target address
  /FROM:<name>            Sender name
  /TO:<name>              Recipient name
  /SUBJ:<text>            Subject line
  /FILE:<path>            Attach file
  /FLAGS:<flags>          Netmail flags (PVT, K/S, etc.)
  /PWRD:<password>        Session password
  /COLOR                  Color output
  /MONO                   Monochrome output
```

### qfutil.c (~400 lines)

## Phase J: QNLIST — Nodelist Compiler [~600 lines]

CLI tool replacing QNLIST.EXE. From binary:

```
Commands:
  /COMPILE           Compile primary + private nodelists
  /COMPILENEW        Compile only if new diffs found
  /COLOR             Color output
  /MONO              Monochrome output

Operations:
  1. Unarchive nodediff (ARJ/ZIP/LZH)
  2. Verify old nodelist CRC
  3. Apply nodediff to nodelist
  4. Verify new nodelist CRC
  5. Compile to binary index
  6. Process private nodelists/pointlists
  7. Move/archive processed files
```

### qnlist.c (~600 lines)

## Phase K: QSCAN — EchoMail Tosser/Scanner [~1,500 lines]

CLI tool replacing QSCAN.EXE. From binary:

```
Commands:
  /TOSS              Toss inbound packets to message base
  /SCAN              Scan outbound from message base
  /BOTH              Toss + scan
  /NETMAIL           Process netmail only
  /AREA:<name>       Process specific area only
  /RESET             Reset high message pointers
  /SETHIGH           Set high pointers to current
  /FORCE             Force rescan
  /DEBUG             Verbose logging

Operations:
  Toss: .PKT → message base (with dupe detection)
  Scan: message base → .PKT (with seen-by/path)
  Pack: .PKT → archive bundles (.MO0-.SU9)
  Areafix: subscribe/unsubscribe/list/query/help
  Route: netmail routing per config rules
```

### qscan.c (~1,500 lines)

## Phase L: QFCONFIG — Configuration Editor TUI [~2,000 lines]

TUI tool replacing QFCONFIG.EXE. From binary:

```
46 menu sections including:
  Program Setup      — directories, filenames, paths
  FidoMail Setup     — addresses, nodelist, routing
  Display Setup      — colors, fonts, screen mode
  Modem/Dialout      — init strings, speeds, ports
  Events             — scheduling, actions, errorlevels
  Node Manager       — per-node config, passwords
  Area Manager       — echomail areas, conferences
  Areafix Setup      — uplinks, forwarding
  File Request Setup — magic names, paths, limits
  Archivers          — ZIP/ARJ/LZH/RAR commands
  Origin Lines       — echomail origin strings
  Netmail Routing    — route rules editor
  Translation/Cost   — phone prefix translations
  Function Keys      — F1-F12 hotkey bindings
  Semaphore Files    — trigger files
  Color Setup        — 20+ color selectors
```

### qfconfig.c (~2,000 lines) — ncurses/conio TUI

---

## Updated Line Estimates

```
Phase A-H (DONE):    6,620 lines  — Mailer + protocols
Phase I (DONE):        162 lines  — QFUTIL utility
Phase J (DONE):        616 lines  — QNLIST nodelist compiler
Phase K (DONE):        833 lines  — QSCAN tosser/scanner
Phase L (DONE):        717 lines  — QFCONFIG TUI editor
                   ─────────────
Current:             6,620 lines  (Phase A-H complete)
Phase I-L:           2,328 lines  (DONE)
Total:               8,950 lines  (full QFront suite — COMPLETE)
```
