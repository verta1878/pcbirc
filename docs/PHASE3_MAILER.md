# Phase 3: FidoNet Frontend Mailer — BinkleyTerm Fork + Clean Room

**Project:** pcbrevival  
**Author:** hexadecimal  
**Status:** Planning  
**License:** GPL v3.0 (our code), clean room — no QFront or Portal of Power code used

---

## Why We Need This

PCBoard 15.4 has a **complete built-in FidoNet tosser** (PCBTOSS) that handles echomail import/export, .PKT processing, SEEN-BY, passthrough, netmail, and packet archiving. It also handles **inbound mailer sessions** (EMSI/WaZoo receive). What it does NOT have is **outbound session origination** — the ability to call another FidoNet node (or connect over TCP/IP) and push/pull mail packets.

In the DOS era, sysops ran a "frontend mailer" like QFront, FrontDoor, InterMail, or Portal of Power to handle this. All of these are now dead commercial/abandonware software with no usable source code. QFront 1.20a was released as freeware/donationware in November 2000, but binaries only.

For a working FidoNet node in 2026, we need outbound capability. The options are:

1. **binkd** — open source binkp daemon, Linux native, handles FidoNet-over-TCP/IP. Works NOW. No UI, no phone-line support, but every surviving FidoNet node runs binkp.

2. **Clean room mailer** — a new frontend mailer written from scratch, using QFront's documentation and Portal of Power's architecture (not code) as reference for understanding protocols and session flow. Gives us full control and a proper UI.

We do BOTH: BinkleyTerm (open source C mailer) for immediate FidoNet connectivity, clean room mailer for the long-term replacement.

---

## Reference Materials (for understanding, NOT for copying)

### QFront 1.20a (Freeware/Donationware)
- **Status:** Binaries only. No source code. Freeware since Nov 28, 2000.
- **Author:** RoCo Software, Inc. (Rob Kittredge)
- **Documentation:** QFRONT.DOC (308KB) — complete manual with protocol descriptions, session flow, configuration, event management, mail routing
- **Companion programs:** QFRONT.EXE (mailer), QFCONFIG.EXE (setup), QSCAN.EXE (tosser), QNLIST.EXE (nodelist compiler), QFUTIL.EXE (utility)
- **Use:** Documentation reference only. We read the manual to understand WHAT a mailer does, not HOW QFront does it internally.

### Portal of Power v0.63 9-GPT06 (Copyleft Source)
- **Status:** Full Turbo Pascal source available.
- **Authors:** The Portal Team (Denmark, 1989-97), German Portal Team (1998-2000)
- **License:** Copyleft postcardware — source can be used in non-commercial programs if full source is released under same terms, program is not called "Portal of Power", and original copyright is preserved
- **Architecture:** ~95 Pascal units covering session management, protocol implementations, nodelist handling, modem control, display, area management
- **Use:** Architecture reference only. We study the MODULE STRUCTURE to understand how a mailer is organized (what units exist, what each does, how they interact). We do NOT translate, port, or adapt any Pascal code. Our C implementation is written from protocol specifications (FTS documents).

### DIRECTIO.C (Guy Eddon, 1993)
- **Status:** Published sample code from "Microsoft RPC for Windows NT" book
- **What it does:** Complete Win32 Console API replacement for DOS direct video RAM access
- **Functions:** `mxyputs()` (positioned string), `mxyputc()` (positioned char fill), `moutchar()` (single char), `box()` (single/double border), `clear()`, `clearscreen()`, `get_character_wait()`, `get_character_no_wait()`, `set_vid_mem()`
- **Use:** Direct reference implementation for porting PCBoard's screen I/O from DOS INT 10h/video RAM to Win32 Console API. Maps 1:1 to PCBoard's `fastputc()`, `fastprint()`, `gotoxy()`, `cls()`, `clsbox()`, `box()`, `bgetkey()` functions.

### FTS Protocol Specifications (Public Domain)
The actual implementation reference. All FidoNet protocols are documented in FTS (FidoNet Technical Standards) documents:
- **FTS-0001** — Basic FidoNet Technical Standard (Type 2 packet format, session handshake)
- **FTS-0006** — YooHoo/2U2 (WaZoo session negotiation)
- **FTS-0009** — EMSI/IEMSI (Enhanced Mail System Interface)
- **FTS-0056** — EMSI (revised, the standard everyone uses)
- **FSC-0039** — Janus bidirectional transfer protocol
- **FTS-5001** — Binkp (FidoNet over TCP/IP, the modern standard)
- **FTS-5000** — Nodelist format specification
- **FRL-1002** — Binkp/1.1 extensions

These are the ONLY documents used for protocol implementation.

---

## Clean Room Approach

The clean room discipline:

1. **NEVER** look at QFront's disassembly, binary internals, or reverse-engineered code
2. **NEVER** translate, port, or adapt Portal of Power's Pascal source into C
3. **DO** read QFront's user documentation to understand feature requirements (WHAT it does)
4. **DO** study Portal of Power's module list and unit names to understand architectural patterns (HOW a mailer is organized into components)
5. **DO** implement all protocols from FTS specification documents only
6. **DO** use DIRECTIO.C as the basis for Win32 console screen I/O (published sample code)
7. **DO** use binkd's public documentation for binkp protocol understanding

The result: a new codebase written entirely from specs, with no copyrighted code from any existing mailer.

---

## Architecture: pcbmailer

Name: **pcbmailer** (working title)  
Language: C (OpenWatcom compatible, targeting DOS 32-bit via Watcom and Linux native via GCC)  
License: GPL v3.0

### Module Map

Based on studying what a frontend mailer needs (from QFront docs and Portal of Power's unit structure), our modules:

```
pcbmailer/
├── main.c              — Entry point, command line, main loop
├── config.c            — Configuration file parser
├── event.c             — Event scheduler (timed polls, mail windows)
├── session.c           — Session state machine (inbound/outbound)
├── emsi.c              — EMSI/IEMSI handshake (FTS-0056)
├── wazoo.c             — YooHoo/2U2 negotiation (FTS-0006)
├── binkp.c             — Binkp protocol for TCP/IP (FTS-5001)
├── zmodem.c            — ZModem file transfer
├── protocol.c          — Protocol dispatcher
├── nodelist.c          — Nodelist access (NODELIST.DBF/NDX via CodeBase)
├── PCBNLC              — Nodelist compiler (Clark) (NODELIST.### → .DBF/.NDX)
├── outbound.c          — Outbound queue manager (BSO/ASO)
├── routing.c           — Mail routing logic
├── areafix.c           — Remote area management (AreaFix processor)
├── packet.c            — FTS-0001 Type 2 .PKT creation/parsing
├── screen.c            — Console UI (DIRECTIO.C pattern for Win32, curses for Linux)
├── serial.c            — Serial port I/O (DOS: FOSSIL, Linux: termios)
├── modem.c             — Modem AT command handler
├── tcp.c               — TCP/IP socket layer
├── log.c               — Session logging
└── freq.c              — File request processor
```

### Session Flow (Outbound Call)

```
event scheduler fires "poll 1:234/56"
  → nodelist lookup → get phone/IP address
    → tcp.c: connect to remote (binkp port 24554)
      → binkp.c: binkp handshake
        → authenticate (password from config)
        → exchange file lists
        → send outbound .PKT files from queue
        → receive inbound .PKT files
      → drop received .PKTs in PCBoard's incoming_packets directory
    → PCBTOSS (inside PCBoard) picks up and imports on next event
```

### Session Flow (Inbound — binkp)

```
binkd (or pcbmailer) listens on port 24554
  → remote connects
    → binkp handshake
    → receive .PKT files → incoming_packets directory
    → send any queued outbound .PKT files
  → PCBTOSS imports on next toss event
```

### Integration with PCBoard

```
pcbmailer ↔ .PKT files ↔ PCBTOSS (inside PCBOARDM.EXE)
                ↕
         PCBoard message bases
```

Shared paths from PCBFIDO.CFG / PCBOARD.DAT:
- `directory_info.incoming_packets` — where pcbmailer drops received .PKTs
- `directory_info.outgoing_packets` — where PCBTOSS puts outbound .PKTs
- `directory_info.nodelist_path` — NODELIST.DBF/NDX location

### Console UI (screen.c)

Two backends:

**DOS/Win32** — based on DIRECTIO.C pattern:
```c
// Console API functions (from Guy Eddon's published samples):
// WriteConsoleOutputCharacter — positioned string output
// FillConsoleOutputCharacter  — character fill
// WriteConsoleOutputAttribute — color attributes
// ReadFile / PeekConsoleInput — keyboard input
// GetConsoleScreenBufferInfo  — screen dimensions
```

Maps directly to PCBoard's screen functions:
- `fastputc()` → `FillConsoleOutputCharacter` (1 char)
- `fastprint()` → `WriteConsoleOutputCharacter` + `WriteConsoleOutputAttribute`
- `gotoxy()` → `SetConsoleCursorPosition`
- `cls()` → `FillConsoleOutputCharacter` (80×25)
- `box()` → DIRECTIO.C `box()` (single/double border with box-drawing chars)
- `bgetkey()` → `ReadFile` on stdin with `ENABLE_LINE_INPUT` disabled

**Linux** — ncurses or raw terminal:
```c
// ncurses: mvprintw(), mvaddch(), box(), clear(), getch()
// Or raw: printf("\033[%d;%dH") ANSI positioning
```

### Call-Waiting Screen

The mailer UI during idle (waiting for calls / between events):

```
┌─────────────────────── pcbmailer v0.1 ────────────────────────┐
│ Address: 1:234/56.0          Nodelist: 07/26  Uptime: 02:14:33│
├───────────────────────────────────────────────────────────────-┤
│ Status: Waiting for call                                      │
│ Last in:  1:100/200  EMSI  ZModem  3 files  12.4KB  00:00:42 │
│ Last out: 1:300/400  binkp         8 files  45.1KB  00:01:15 │
├───────────────────────────────────────────────────────────────-┤
│ Queue: 3 systems, 12 packets, next poll in 00:14:22           │
│ Events: E01 02:00 CM poll hub | E02 04:00 toss+scan           │
├────────────────────────── Activity Log ────────────────────────┤
│ 01:45:12 Received 3 .PKT from 1:100/200 (EMSI/ZModem)        │
│ 01:45:14 Tossing inbound packets...                           │
│ 01:45:15 Imported 47 messages to 3 conferences                │
│ 01:30:00 Event E01: polling hub 1:300/400                     │
│ 01:30:02 Connected to hub.example.com:24554 (binkp)           │
│ 01:30:05 Sent 8 packets (45.1KB), received 0                  │
│ 01:30:08 Session complete, 00:00:06 elapsed                   │
└───────────────────────────────────────────────────────────────-┘
 F1=Help F2=Manual Poll F3=Queue F5=Nodelist F7=Config F10=Exit
```

---

## Phase 3 Build Order

### Step 1: Nodelist Compiler (PCBNLC) — ✅ LINKED
Write a standalone tool that reads raw FidoNet nodelist text (NODELIST.###) and compiles it into NODELIST.DBF/NDX using CodeBase 4.x (which we have full LGPL source for). PCBoard needs this to look up node addresses. This is the gap QFront's QNLIST.EXE fills.

Input format (FTS-5000):
```
Zone,1,North_America,Salt_Lake_City_UT,Sysop_Name,1-801-555-1234,9600,CM,XA,V32b,V42b
,2,Region_2,...
,10,Net_10,...
,100,Node_100,...
```

Output: NODELIST.DBF (CodeBase dBASE IV format) + NODELIST.NDX (index)

### Step 2: binkd Integration — IMMEDIATE  
Configure binkd to work with PCBoard's packet directories. No coding needed — binkd is packaged in most Linux distros. Write a setup guide and sample binkd.cfg.

### Step 3: PCBFU.EXE Link — IMMEDIATE
All 7 source files compile. Link into binary #13. Needs the PCBoard library .OBJ files.

### Step 4: pcbmailer Core — MEDIUM TERM
Start with binkp.c (TCP/IP session over sockets), packet.c (FTS-0001 .PKT format), and outbound.c (queue management). This gives us a minimal outbound mailer that can poll a hub over binkp.

### Step 5: Console UI — MEDIUM TERM
screen.c using DIRECTIO.C pattern (Win32) or ncurses (Linux). The call-waiting screen with activity log, queue status, and event countdown.

### Step 6: EMSI/WaZoo + ZModem — LONG TERM
Only needed if anyone still runs a phone-line FidoNet node. EMSI (FTS-0056) handshake + ZModem file transfer. Low priority — binkp handles everything over TCP/IP.

### Step 7: Event Scheduler — LONG TERM
Timed events (poll hub at 2:00 AM, toss at 4:00 AM, compile nodelist on Friday). QFront's event manager was its strongest feature — ours needs to be equally robust.

---

## What We Do NOT Build

- **Phone line dialing** — no POTS lines in 2026. binkp over TCP/IP covers everything.
- **FAX support** — QFront had it, we don't need it.
- **CallerID** — phone-line feature, irrelevant.
- **Overlay manager** — DOS memory constraint, irrelevant on 32-bit.
- **BBS integration hooks** — PCBoard's PCBTOSS already handles the BBS side.

---

## The Crew

- **hexadecimal** — pcbrevival lead, PCBoard 15.4 source maintainer
- **verta1878** — netmodem2irc lead, pcbmailer architecture
- **wrench** — netmodem2irc engine, transport layer (serial.c, tcp.c candidates)
- **sysop/0** — compiler maintainer, build system
- **kiddo/evga** — RIPscrip engine (mailer UI could use RIP graphics someday)
- **evga** — display/monitor

---

## Full Stack (Target)

```
                        ┌─────────────┐
                        │   Internet  │
                        └──────┬──────┘
                               │
                    ┌──────────┴──────────┐
                    │     pcbmailer       │
                    │  (binkp over TCP)   │
                    │  session manager    │
                    │  event scheduler    │
                    │  outbound queue     │
                    └──────────┬──────────┘
                               │
                          .PKT files
                               │
              ┌────────────────┴────────────────┐
              │         PCBOARDM.EXE            │
              │      (via Wine on Linux)        │
              │                                 │
              │  PCBTOSS ──── message bases     │
              │  EMSI/WaZoo ─ inbound sessions  │
              │  PCBFU.EXE ── FidoNet utility    │
              └────────────────┬────────────────┘
                               │
                    ┌──────────┴──────────┐
                    │   netmodem2irc      │
                    │  (FOSSIL transport) │
                    └──────────┬──────────┘
                               │
                    ┌──────────┴──────────┐
                    │   telnet callers    │
                    └─────────────────────┘
```

---

## Addendum: BinkleyTerm XE Source Acquired

BinkleyTerm XE (eXtended Edition) source code is now in `fido/btxe-source.zip`.

- **124 C source files + 67 headers**
- **Platforms:** DOS, OS/2, Win32, Linux
- **License:** Freely available, non-commercial (Bit Bucket Software Co.)
- **Source:** github.com/oldprogs/btxe
- **Original authors:** Vince Perriello + Bob Hartman (1987-96)
- **XE team:** Thomas Waldmann et al. (1996-2013)

BinkleyTerm is the reference FidoNet mailer. It handles:
- EMSI/YooHoo/WaZoo session negotiation
- ZModem/SEAlink/Telink file transfer
- BSO (BinkleyTerm Style Outbound) queue management
- Nodelist compilation and lookup
- Event scheduling
- Call-waiting screen with activity log

The DOS source compiles with Borland C — same compiler as PCBoard.
For our Watcom port, BinkleyTerm's protocol implementations (EMSI, ZModem)
can be studied and adapted. The architecture maps directly to our
pcbmailer module plan.

### Files in fido/

| File | Description |
|------|-------------|
| `btxe-source.zip` | BinkleyTerm XE full source (C, freely available) |
| `QFRONT.zip` | QFront 1.20a binaries + docs (freeware) |
| `portal-of-power-src.zip` | Portal of Power v0.63 Pascal source (copyleft) |
