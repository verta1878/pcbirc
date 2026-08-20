# PCBoard 15.41 — Development Phases

**Rule: Written once, checked three times. Each phase has a gate test.
No skipping. Every phase is self-contained and testable.**

---

## Completed Phases

### Phase 0 ✅ — Missing Clark Utilities
**Gate:** All 12 Clark utilities compile and link with zero unresolved symbols.

| Binary | Size | Status |
|--------|------|--------|
| PCBSTATS_W.EXE | 31K | ✅ |
| PCBPACK_W.EXE | 84K | ✅ |
| MSETUP_W.EXE | 108K | ✅ |
| PCBMODEM_W.EXE | 528K | ✅ |
| PCBEDIT_W.EXE | 133K | ✅ |
| PCBMONI_W.EXE | 54K | ✅ |
| PCBDIAG_W.EXE | 552K | ✅ |
| PCBFILER_W.EXE | 215K | ✅ |
| PCBNLC_W.EXE | 77K | ✅ |
| OFFLINE_W.EXE | 26K | ✅ |
| WAITBU_W.EXE | 25K | ✅ |
| PCBTITLE_W.EXE | 15K | ✅ |

Libraries written: VMAVL (324 lines), d4all.h (121), conio_compat (102)
Bugs fixed: 23 across pcbis/pcbfoss/qfront
Phase 0 patch: 154_watcom_phase0_complete.patch (12MB, 1,581 files)

---

## 15.41 Phases

### Phase 1 — Watcom Port Completion
**Owner:** hexadecimal
**Gate:** All Clark binaries with source compile under openwatcom2irc.
Zero warnings.

**Work:**
- Complete WATCOMPAT.H for remaining 157 source files
- Fix constrea.h stub (C++ iostream compatibility)
- Fix header ordering (per hexadecimal-phase-update.md)
- Per-binary include path fixes
- Compile all .CPP files with bwpp386

**Explicit target list.** Everything in `phase27/BINARY-CATALOG.md`
section A — these have Clark source and are build targets here, not
binary-analysis targets:

  Main:    PCBOARD, PCBOARD2, PCBOARDM, PPLC
  Setup:   PCBSETUP, PCBSM, PCBMODEM, MSETUP
  Tools:   PCBFILER, PCBEDIT, PCBNLC, PCBPACK, PCBDIAG, PCBMONI,
           PCBSTATS, MKPCBTXT, MAKEIDX, MAKEHELP, USERNET, WAITFILE,
           OFFLINE, PCBTITLE, WAITBU
  Net:     FIDOUTIL, UUIN, UUOUT, UUUTIL, UUXFER
  Xfer:    ZMRECV, ZMSEND
  OS/2:    PCBCP (separate target)

**PCBMODEM specifically** was briefly listed under Phase 27 before a
source check found `Pcb-util/PCBMODEM/` complete — PCBMODEM.CPP,
MSETUP.CPP, MDMCMDS.CPP, COMMON.CPP, QUESTION.CPP, ERRORS.CPP,
TOKEN.C, INPUTNUM.C, and MODEMS.H documenting the MODEMS.DAT record
layout exactly. It builds here like any other utility. Ships with
MODEMS.DAT (67K, ~150 modem definitions) which is data, not code, and
carries over unchanged.

**Test:**
1. wcc386/wpp386 compiles every .C/.CPP in PCBSRC/ — zero errors
2. All 28 binaries link — zero unresolved
3. DOS4GW smoke test: each binary runs and shows usage/help screen
4. No regressions: existing Phase 0 binaries still link

**Check 1:** hexadecimal reviews all WATCOMPAT.H additions
**Check 2:** sysop/0 verifies binary sizes are reasonable vs Borland originals
**Check 3:** Run PCBoard in DOSBox-X, local login, post message, download file

---

### Phase 1b — 15.3 to 15.4 upgrade
**Owner:** hexadecimal
**Gate:** Our build matches Clark's 15.4 beta binaries feature for feature.
**Runs before Phase 1a** — private messages are a 15.4 feature Clark left
unfinished, so the 15.4 base has to exist first.

**Why this phase exists.** The source we hold is **15.3**. The binaries we
hold are **15.4 beta** (July 1997). Clark never released 15.4 source. Without
this step we would be adding 15.41 features to a 15.3 base and calling the
result 15.41.

**No diff exists — this is reimplementation.** Verified: the source is
15.3 (`#define PCBVERSION "15.3"`), and zero of the twelve PPL 3.40
additions are present. `MoveMsg` and `ShortDesc` do appear, but as a
PCBOARD.DAT config field and the file short-description display
respectively, not the PPL additions.

**The spec is Clark's own.** `WHATSNEW` (604 lines) documents every
change, `HISTORY` lists the April-July 1997 bug fixes. Both ship with
the 15.4 beta.

Full delta specification, extracted per-feature with signatures, field
tables and the discrepancies found in Clark's own documentation:
**`PCB-15.3-TO-15.4-DELTA.md`**.

**Do this first: dump the PPLC opcode table from Clark's 15.4
PPLC.EXE.** PPL bytecode is a numbered function table and each addition
took a specific opcode. Guess the numbers and PPEs compiled by our PPLC
will not run under Clark's — which collapses the two-way verification
and is not discoverable until someone tries it.

**Work — PCBoard:**
- CHAT: @X colour codes and sysop-definable action commands
- File flagging by number; view full descriptions from the short-view listing
- User record: birthday, gender, email address, personal web page (via PSA)
- UUIN: reject email or newsgroup messages by name (REJECTS file, bounce or
  discard, max 16 entries)
- PPL compiler: smaller, faster output

**Work — PPL 3.40, twelve additions:**

| Addition | Kind |
|---|---|
| `GetBankBal()` | function — time bank, 12 fields |
| `SetBankBal` | statement |
| `GetMsgHdr()` | function — read message header |
| `SetMsgHdr()` | function — write message header |
| `MoveMsg` | statement — move message to end of base |
| `ShortDesc()` | function |
| `ShortDesc` | statement |
| `U_BIRTHDATE` | variable |
| `U_EMAIL` | variable |
| `U_GENDER` | variable |
| `U_SHORTDESC` | variable |
| `U_WEB` | variable |

`GetMsgHdr()`/`SetMsgHdr()` operate on the header documented in
`DOCDEV/MSGS.TXT` — note the message and reference numbers are `bsreal`
(Microsoft Binary Format floats), not integers; Clark's converters are in
`Pcb-libs/SOURCE/MISC/BS_LONG.C` and `LONG_BS.C`.

**Work — PCB/IC:**
- FTP MGET (multiple gets)

**Work — bug fixes from HISTORY:**
- L command wildcard search
- W command showing the EMAIL prompt for the WEB field
- NEWUSER field spacing for EMAIL and WEB
- PPL crash: `USELMRS FALSE` + `GETALTUSER` + `ADDUSER` together
- Chat actions with common prefixes matching the wrong action
- Gender defaulting to 'M' instead of empty
- UUIN multipart/alternative MIME support

**Compatibility, per Clark's README.1ST:**
- USERS.SYS is dynamic and versioned
- DOORS.LST can specify a version number (3 = v15.2 style) so old doors
  keep working

**We have an oracle.** Clark's 15.4 binaries are here — PCBOARD.EXE,
PCBOARDM.EXE, PPLC.EXE, PCBSETUP.EXE, PCBSM.EXE and the UUCP suite. Same
approach as Phase 27: compare behaviour against the original rather than
against our reading of the documentation.

**Test:**
1. Every PPL 3.40 addition compiles under our PPLC and runs correctly
2. A PPE using `GetMsgHdr()`/`SetMsgHdr()` reads and writes headers that
   Clark's 15.4 PCBOARD.EXE also accepts
3. CHAT renders @X colours; action commands fire, including two actions
   sharing a prefix
4. User record round-trips birthday, gender, email and web through PCBSETUP
   and PCBSM
5. UUIN rejects a named sender, both bounce and discard
6. Every bug in HISTORY is fixed and has a regression test

**Check 1:** Our PPLC output runs under Clark's 15.4 PCBOARD.EXE
**Check 2:** Clark's PPLC output runs under our PCBOARD.EXE
**Check 3:** USERS.SYS written by ours is readable by Clark's, and the reverse
**Check 4:** A v15.2-era door still runs via the DOORS.LST version field

---

### Phase 1a — Private Messages (15.4 completion)
**Owner:** hexadecimal
**Gate:** Caller sends/reads private mail via @ and @W commands.

Clark started private messages in 15.4 but didn't finish.

**Work:**
- Separate message base for private mail (not in conference bases)
- @ reads caller's inbox, @W writes to user/alias
- Y-scan includes E-Mail line for personal mail
- Comments to sysop delivered to inbox
- One inbox across all conferences
- PCBTEXT strings for inbox prompts

**Test:**
1. @W sends private message, @ shows it in recipient's inbox
2. Y-scan finds private mail across conferences
3. Old PPEs reading conference bases don't see inbox messages

**Check 1:** Private mail not visible in conference message bases
**Check 2:** Y-scan E-Mail line works in quick and long scan
**Check 3:** PCBTEXT prompts display correctly

---

### Phase 2 — FidoNet Suite Integration
**Owner:** evga (QFront), wrench (transport), hexadecimal (integration)
**Gate:** Send and receive a FidoNet netmail message between two nodes.

**Work:**
- QFront: complete remaining handlers (evga, from 9,038 lines)
- Wire QFront to PCBTOSS (Clark's tosser in PCBOARDM)
- Wire pcbbinkp as BinkP transport
- Configure nodelist with PCBNLC
- Integration test with mock FidoNet node

**Test:**
1. PCBNLC compiles a test nodelist — NODELIST.DBF/.NDX created correctly
2. QFront outbound: create netmail, QFront packages .PKT, sends via pcbbinkp
3. QFront inbound: receive .PKT via pcbbinkp, QFront delivers to inbound/
4. PCBTOSS tosses inbound .PKT into PCBoard message base
5. User reads the netmail in PCBoard

**Check 1:** hexadecimal verifies .PKT format against FTS-0001
**Check 2:** wrench verifies BinkP session against FTS-5001
**Check 3:** End-to-end: two DOSBox instances exchange mail

---

### Phase 3 — EchoMail and Areafix
**Owner:** evga (qscan), hexadecimal (PCBoard config)
**Gate:** Subscribe to an echo area, receive echomail, reply propagates back.

**Work:**
- qscan /TOSS — toss echomail .PKTs to conference message bases
- qscan /SCAN — scan conferences for new messages, create outbound .PKTs
- Areafix: process subscribe/unsubscribe/list/query commands
- SEEN-BY/PATH handling (in PCBTOSS, already exists)
- Dupe detection (CRC ring, in PCBTOSS, already exists)

**Test:**
1. Create echomail area in PCBSETUP, link to hub address
2. Receive echomail .PKT — messages appear in conference
3. User posts reply — qscan creates outbound .PKT with SEEN-BY
4. Areafix: send +AREANAME to hub — subscription processed
5. Dupe check: send same message twice — second is rejected

**Check 1:** SEEN-BY/PATH lines correct per FTS-0004
**Check 2:** Dupe ring CRC matches PCBTOSS implementation
**Check 3:** 100-message stress test — no lost/duplicated messages

---

### Phase 4 — TIC File Echo Processing
**Owner:** evga (tic.c in QFront)
**Gate:** Receive a TIC file, validate it, store in file directory, forward to downlink.

**Work:**
- tic.c: complete TIC parser (FTS-5006)
- Validate: CRC-32, password, authorized sender address
- Store received file in PCBoard file directory
- Generate DIR entry with FILE_ID.DIZ extraction
- Forward: create outbound TIC + file for downlinks
- Path sanitization (already fixed — no .. or / in filenames)

**Test:**
1. Receive test .TIC + associated file from mock hub
2. Validate CRC — correct file passes, corrupted file rejected
3. Validate password — wrong password rejected
4. File stored in correct PCBoard file directory
5. DIR listing updated with file description
6. Forward: .TIC + file created in outbound for downlink

**Check 1:** CRC-32 matches value in .TIC file
**Check 2:** DIR entry format matches PCBoard DEVELOP9/DIR.DOC
**Check 3:** Multi-node: two nodes process TICs simultaneously without conflict

---

### Phase 5 ✅ — PCBISO File Area Indexer
**Owner:** hexadecimal
**Gate:** Sysop mounts ISO, runs PCBISO, users download files from file area.

**Work:**
- PCBISO.EXE: ISO mounter + directory scanner + DIR file generator
- /MOUNT <iso> — mount ISO image, assign drive letter
- /UNMOUNT <drive> — release drive letter
- /INDEX <drive|path> — scan source, generate PCBoard DIR listing
- /INDEX ALL — index all configured sources
- /REBUILD <drive|path> — delete and regenerate DIR listing from scratch
- /REBUILD ALL — rebuild all file area indexes
- /LIST — quick list of mounted ISOs and configured paths
- /STATUS — detailed view (file counts, last indexed date, status)
- FILE_ID.DIZ extraction from ZIP/ARJ archives
- PCBISO.DAT — persists mount table (which ISOs on which drive letters)
- Reads CNAMES/DIR config to know configured file areas
- Error messages when drive/path not found or not configured
- Sysop menu integration: view mounted ISOs from inside PCBoard
- Support: drive letters, local paths
- No PCBOARD.EXE modifications needed
- No PCBOARD.DAT changes needed
- Uses existing SlowDrives mechanism for file transfer

**Test:**
1. PCBISO /MOUNT C:\ISOS\SHAREWARE.ISO — mounts, assigns drive letter
2. PCBISO /INDEX E: — DIR listing generated with sizes, dates, descriptions
3. PCBISO /LIST — shows mounted ISO with drive letter
4. PCBISO /STATUS — shows file count, last indexed date, status
5. PCBISO /REBUILD E: — DIR listing deleted and regenerated
6. PCBISO /REBUILD X: — error: drive not found
7. PCBISO /UNMOUNT E: — drive letter released
8. Create file area in PCBSETUP, add drive to SlowDrives
9. User downloads file — SlowDrives copies to TmpLoc, transfer succeeds
10. PCBISO /INDEX C:\PCB\FILES\LOCAL — indexes local path

**Check 1:** DIR listing matches actual directory contents
**Check 2:** Downloaded file matches original byte-for-byte
**Check 3:** FILE_ID.DIZ descriptions extracted correctly from ZIPs

### Phase 5a ✅ — ISO Conference Configuration
**Owner:** hexadecimal
**Requires:** Phase 5 complete
**Gate:** Sysop marks conference as ISO-backed, PCBISO /INDEX ALL picks it up.

**Uses Reserved[64] in CNAMES.ADD — no PCBOARD.DAT changes, no upd1541.**

Reserved[64] layout per conference:
- Byte 0: ISO flag (0=normal, 1=ISO-backed)
- Bytes 1-63: source path (62 chars + null)

**Work:**
- PCBISO /SETISO <confnum> <path> — set ISO flag + path in CNAMES.ADD
- PCBISO /CLEARISO <confnum> — clear ISO flag for conference
- Reads/writes CNAMES.ADD Reserved[64] directly
- No PCBOARD.EXE changes needed
- No PCBSETUP changes needed
- No upd1541.exe needed — Reserved bytes already exist in 15.4

**Test:**
1. PCBISO /SETISO 2 D:\SHAREWARE — sets conference 2 as ISO-backed
2. PCBISO /LIST shows conference 2 as "ISO" type with path
3. PCBISO /INDEX ALL indexes conference 2 from D:\SHAREWARE
4. PCBISO /CLEARISO 2 — clears ISO flag
5. PCBISO /LIST shows conference 2 as "Local" again
6. Existing 15.4 PCBoard ignores Reserved bytes — no breakage

**Check 1:** CNAMES.ADD file size unchanged after /SETISO
**Check 2:** Other conference fields not corrupted by Reserved write
**Check 3:** PCBoard 15.4 runs normally with ISO flags set (ignores them)


---

### Phase 6 — pcbis Internet Services
**Owner:** sysop/0 (Pascal), hexadecimal (integration)
**Gate:** Telnet user logs into PCBoard, posts message, downloads file.

**Work:**
- pcbis daemon: compile and run on Linux (fpc264irc)
- Telnet bridge: connect to FOSSIL, bridge to PCBoard node
- PCBOARD.SYS / CALLERS: node file I/O
- WFC console: ANSI TUI showing node status
- All 11 bug fixes applied (this session)

**Test:**
1. pcbis starts, listens on port 2323
2. Telnet client connects, sees PCBoard login prompt
3. User logs in, reads messages, downloads a file
4. WFC console shows connected user with IP, speed, activity
5. User disconnects cleanly — CALLERS log entry written
6. Ctrl-C: daemon shuts down cleanly (double-fork, SIGPIPE handled)

**Check 1:** PCBOARD.SYS record matches DEVELOP9/PCBSYS.DOC format
**Check 2:** CALLERS log matches DEVELOP9/CALLERS.DOC format
**Check 3:** 10 simultaneous telnet connections — no crashes, no leaks

---

### Phase 7 — BinkP in pcbis
**Owner:** sysop/0
**Gate:** pcbis receives FidoNet mail via BinkP, PCBTOSS processes it.

**Work:**
- pcbis_binkp: listen on port 24554
- Receive .PKT files, store in inbound/
- Trigger PCBTOSS after session
- Outbound: send pending .PKTs from outbound/
- 8KB frame limit enforced (BUG-3 fix)

**Test:**
1. pcbis listens on 24554, QFront connects and sends .PKT
2. .PKT arrives in inbound/, PCBTOSS tosses to message base
3. Outbound: pcbis sends pending mail to QFront
4. Frame > 8KB rejected with log entry
5. Session with invalid password rejected

**Check 1:** BinkP session log matches FTS-5001 protocol
**Check 2:** Received .PKT validates against FTS-0001
**Check 3:** Stress: 50 rapid connections — no resource exhaustion

---

### Phase 8 — FTP/HTTP/SMTP in pcbis
**Owner:** sysop/0
**Gate:** FTP download, HTTP status page, SMTP validation email all work.

**Work:**
- pcbis_ftp: FTP server with PCBoard security levels
- pcbis_http: status page (/status, /callers, /online)
- pcbis_smtp: outbound validation emails
- Path traversal fix applied (BUG-1)
- PASV socket leak fix applied (BUG-2)

**Test:**
1. FTP: anonymous login, list files, download file
2. FTP: authenticated login, upload file, security level enforced
3. FTP: `../../etc/passwd` rejected (path traversal blocked)
4. HTTP: /status shows uptime, node count, version
5. HTTP: /callers shows recent callers
6. SMTP: send validation email to test address

**Check 1:** FTP: download file matches original byte-for-byte
**Check 2:** HTTP: uptime shows actual uptime (not current time — BUG-4 fixed)
**Check 3:** FTP: 5 simultaneous downloads — no socket leaks

---

### Phase 9 — NNTP/QWK/UUCP2 in pcbis
**Owner:** sysop/0
**Gate:** NNTP gateway, QWK networking, UUCP2 transport all functional.

**Work:**
- pcbis_nntp: news<->PCBoard conference gateway
- pcbis_qwk: QWK/QWKE offline mail networking
- pcbis_uucp2: UUCP over TCP
- NNTP socket leak fix applied (BUG-7)

**Test:**
1. NNTP: subscribe to conference via news reader, read messages
2. NNTP: post message via news reader, appears in PCBoard conference
3. QWK: generate QWK packet, download, read in offline reader
4. QWK: upload REP packet, messages posted to conferences
5. UUCP2: exchange mail bundle over TCP

**Check 1:** QWK 128-byte block format correct
**Check 2:** NNTP: auth failure closes socket cleanly (BUG-7 fixed)
**Check 3:** Round-trip: post via NNTP, read via QWK — message intact

---

### Phase 10 — SIO and Cyclades Drivers
**Owner:** evga
**Gate:** PCBoard runs multi-node on SIO multiport card under OS/2.

**Work:**
- SIO driver v2K: 31 bugs fixed (done)
- Cyclades Cyclom-Y: ISA + PCI, Win2K-Win11 + DOS (done)
- cyfossil.asm: fix AH range check (BUG-5)
- cyfossil.asm: fix CD1400 CAR register race (BUG-6)

**Test:**
1. SIO: PCBoard opens COM port via SIO, receives call, user logs in
2. SIO: multi-node — 4 nodes on SIO card, simultaneous sessions
3. Cyclades: DOS test utility runs, detects card and channels
4. Cyclades: Windows driver installs via WoW64, COM ports appear

**Check 1:** SIO: no data corruption at 115200 baud over 1-hour session
**Check 2:** Cyclades: all 8 channels accessible simultaneously
**Check 3:** CD1400 CAR race: interrupt storm test — no channel corruption

---

### Phase 11 — FOSSIL Driver Suite
**Owner:** sysop/0 + kiddo, wrench (netmodem2irc integration)
**Gate:** PCBoard runs via FOSSIL on all 4 platforms.

**Work:**
- pcbfoss.pas: all bug fixes applied (BUG-1 through BUG-4, IMP-1, IMP-2)
- DOS: INT 14h TSR — PCBoard talks to FOSSIL
- Linux: socket ASYNC — pcbis telnet bridge
- OS/2: DosDevIOCtl — native OS/2 FOSSIL
- Windows: NTVDM/DOSBox — Windows FOSSIL
- wrench's 37-test conformance suite

**Test:**
1. DOS: PCBoard + FOSSIL TSR + modem — caller logs in
2. Linux: pcbis telnet + pcbfoss — remote user session
3. OS/2: PCBoard + OS/2 FOSSIL — native session
4. Windows: PCBoard in DOSBox + FOSSIL — session works
5. Fn $02 blocks until data available (BUG-1 fixed)
6. Fn $13 writes string, not single char (BUG-2 fixed)
7. wrench's 37-test conformance suite: 37/37 pass

**Check 1:** Run conformance suite on each platform — all pass
**Check 2:** BaudRate in info struct reads 1152 (IMP-2 fixed)
**Check 3:** Ring buffer block I/O: 4KB transfer speed vs per-byte baseline

---

### Phase 12 — pcbwave Audio Support
**Owner:** TBD
**Gate:** PCBoard plays WAV/MOD files to caller during session.

**Work:**
- Audio streaming over modem/telnet connection
- WAV file playback (8-bit PCM, 8/11/22 KHz)
- MOD/S3M tracker playback (optional)
- Integration with PCBoard events (login sound, new mail chime)
- PPL function: PLAYWAVE("filename.wav")

**Test:**
1. PLAYWAVE in PPE plays WAV file — audio heard by caller
2. Login event triggers welcome.wav
3. WAV playback doesn't block BBS operations
4. Caller without audio support — graceful skip

**Check 1:** WAV format matches Microsoft RIFF spec
**Check 2:** Audio stream doesn't corrupt data channel
**Check 3:** 3 nodes playing audio simultaneously — no cross-talk

---

### Phase 13 — 15.41 Release Packaging
**Owner:** hexadecimal + verta1878
**Gate:** 4-disk distribution set passes installation test on clean system.

**Work:**
- Disk 1: PCBOARD.EXE, PCBSETUP.EXE, PPLC.EXE, core files
- Disk 2: Utilities (Phase 0 binaries), PCBNLC, PCBFILER
- Disk 3: pcbis daemon, QFront suite, FOSSIL drivers, docs
- Disk 4: pcbis internet services, sample configs
- upd1541.exe: upgrade utility (15.40 -> 15.41)
- INSTALL.TXT, README.1ST, WHATSNEW.TXT
- FILE_ID.DIZ, FILE_ID.ANS

**Test:**
1. Clean DOSBox install from Disk 1-4 — BBS boots and accepts login
2. Upgrade from 15.40: run upd1541.exe — all files updated, config preserved
3. FidoNet: configure QFront + PCBNLC, exchange test mail
4. Internet: start pcbis, telnet login works
5. ISO: add shareware CD ISO, files appear in directory listing
6. All 5,703 PPEs from collection load without error

**Check 1:** Fresh install: every binary runs and shows version
**Check 2:** Upgrade: PCBOARD.DAT settings preserved, new fields added
**Check 3:** Full integration: telnet + FidoNet + file download + message post

---

## Phase Dependencies

```
Phase 0 ✅ (done)
  └── Phase 1 (Watcom port completion)
        ├── Phase 2 (FidoNet suite)
        │     ├── Phase 3 (EchoMail/Areafix)
        │     └── Phase 4 (TIC file echo)
        ├── Phase 5 (ISO image support)
        ├── Phase 6 (pcbis telnet)
        │     ├── Phase 7 (pcbis BinkP)
        │     ├── Phase 8 (pcbis FTP/HTTP/SMTP)
        │     └── Phase 9 (pcbis NNTP/QWK/UUCP2)
        ├── Phase 10 (SIO/Cyclades drivers)
        ├── Phase 11 (FOSSIL suite)
        └── Phase 12 (pcbwave audio)
              └── Phase 13 (Release packaging)
```

Phases 2-12 can run in parallel after Phase 1. Phase 13 requires all others.

---

## Phases 14-24 — Scoped August 2026

### Phase 14 — DOS/32A Extender Switch
**Owner:** hexadecimal
**Gate:** All OpenWatcom binaries link with DOS/32A, run on real DOS and DOSBox.

**Work:**
- Switch linker target: wcl386 /l=dos32a (replaces /l=dos4g)
- Remove dos4gw.exe from distribution (DOS/32A embeds in .exe)
- Recompile: QFront (5 binaries), PCBISO, all Phase 0 Clark utilities
- Recompile: all Phase 1 Clark binaries when ready
- Update BUILD.md with new linker target
- DOS/32A source in openwatcom2irc contrib/extender/dos32a/
- DOS/4GW was proprietary binary (Tenberry) — no source, can't fix
- DOS/32A is open source, smaller, faster, embeddable

**Test:**
1. All binaries run on real DOS / FreeDOS
2. All binaries run in DOSBox-X
3. All binaries run in NTVDM (Windows 7) — test, may still fail
4. No separate dos4gw.exe needed — extender embedded
5. Binary sizes comparable or smaller than DOS/4GW versions

**Check 1:** QFront qfconfig.exe opens TUI on DOSBox
**Check 2:** PCBISO /LIST runs without error
**Check 3:** Phase 0 utilities show usage/help screen

---

### Phase 15 — Code Style Audit
**Owner:** hexadecimal
**Gate:** All pcbirc crew code follows Clark's PCBoard 15.3 conventions.

**Work:**
- Variable naming: descriptive, capitalized (Xpos, MaxRow, DoEscCodes)
- Module prefixes: Scrn_Addr, Mdm_Speed, Bso_Path
- Struct fields: right-aligned /* comments */ on every field
- Function headers: block comment on every function explaining purpose
- File headers: Clark's banner style /\*!!!...!!!\*/
- Aggressive inline comments: every variable, every branch, every magic number
- No Hungarian notation, no underscore_lowercase
- static for module-internal functions

Files to audit:
  QFront:  21 source files (10,123 lines)
  PCBISO:  pcbiso.c, pcbiso.h (969 lines)
  pcbfoss: cyfossil.asm + C wrappers
  pcbis:   installer/setup TUI

**Test:**
1. All audited files compile clean (zero new warnings)
2. No behavior changes — audit is cosmetic only
3. grep for non-conforming patterns returns zero hits

**Check 1:** Peer review of naming conventions
**Check 2:** Every function has a block comment header
**Check 3:** Every struct field has a right-aligned comment

---

### Phase 16 — PCBDraw ANSI Art Editor
**Owner:** hexadecimal
**Gate:** pcbdraw.exe opens canvas, user draws ANSI art, saves file.

See 1541/pcbdraw/PCBDRAW-PHASES.md for the full 8-phase breakdown:
  PCBDraw Phase 1: Canvas + Screen + Mouse
  PCBDraw Phase 2: Drawing Tools (8 line draw modes)
  PCBDraw Phase 3: Block Operations
  PCBDraw Phase 4: Undo System
  PCBDraw Phase 5: File I/O (10 formats + SAUCE)
  PCBDraw Phase 6: Font + Palette editor
  PCBDraw Phase 7: TCP Teleconference (host/join/serial/password)
  PCBDraw Phase 8: Door Mode (drop file, FOSSIL, Door32)

Compile flag: -dPCBOARD (Clark's original utility build flag)
Reference design: ansiedit from mysticbbsirc (study algorithms, rewrite in C)
DRAW command added to COMMAND.C command table
Configured in pcbis — no drop file needed when launched by pcbis
Standalone: /LOCAL, /L, serial, TCP
Door: DOOR.SYS, DOOR32.SYS, CHAIN.TXT, DORINFOx.DEF

**Test:**
1. pcbdraw.exe opens, shows 80x25 canvas with status bar
2. Draw with all 8 line draw modes — corners/T/cross auto-detect
3. Save as ANSI, load back — canvas matches
4. /SAUCE myart.ans — prints SAUCE info to stdout, exits
5. Teleconference: two instances, host + join, draw simultaneously

**Check 1:** ANSI output matches ansiedit output for same drawing
**Check 2:** SAUCE record matches ACiD SAUCE spec
**Check 3:** Door mode: caller draws over telnet via pcbis

---

### Phase 17 — Watt-32 TCP/IP Integration
**Owner:** sysop/0
**Gate:** DOS/32A program makes a TCP connection via Watt-32 sockets.

**Work:**
- Compile Watt-32 under openwatcom2irc for DOS/32A flat model
- Fix 8.3 filename issues (cflags_buf.h -> cflagsbf.h, known issue)
- Create pcb_sock.c/.h — socket abstraction layer
  #ifdef __DOS__ uses Watt-32 sockets
  #ifdef __LINUX__ uses POSIX sockets
- Packet driver setup documentation
- Reference: VSOUPSRC rgsocket.cc (BSD socket abstraction, 14K lines)

**Test:**
1. TCP connect to remote host from DOS program
2. Send and receive data
3. DNS lookup works
4. Same C code compiles on Linux with POSIX sockets
5. Packet driver loads, Watt-32 initializes, no crash

**Check 1:** TCP echo test — send 1KB, receive 1KB, data matches
**Check 2:** Linux build links against system sockets — same API
**Check 3:** DNS resolves a hostname to IP correctly

---

### Phase 18 — pcbis Telnet Service (Enhanced)
**Owner:** sysop/0
**Gate:** Telnet user logs into PCBoard from DOS and Linux.

Extends Phase 6 (pcbis telnet on Linux) to include DOS via Watt-32.

**Work:**
- DOS: pcbis telnet service using Watt-32 + pcb_sock
- Linux: native POSIX sockets (already in Phase 6)
- Same source, #ifdef platform split
- Multi-node: one socket per node

**Test:**
1. DOS: pcbis starts, listens, telnet client connects
2. Linux: same test (Phase 6 baseline)
3. Both platforms: user logs in, posts message, downloads file

**Check 1:** DOS telnet session matches Linux behavior
**Check 2:** 4 simultaneous connections on DOS — stable
**Check 3:** Clean disconnect — no orphaned sockets

---

### Phase 19 — pcbis SSL/TLS Layer
**Owner:** sysop/0
**Gate:** Telnet-SSL connection to PCBoard works.

**Work:**
- OpenSSL integration:
  DOS: port to OpenWatcom DOS/32A, or link DJGPP-compiled lib
  Linux: link system OpenSSL
- Randomness: DOS = hardware timer entropy; Linux = /dev/urandom
- Wrap pcb_sock with SSL (pcb_ssl.c/.h)
- Certificate loading (PEM format)
- Reference: Windll.zip (DES, IDEA, MD5, MDC source, freeware 1994)

**Test:**
1. Telnet-SSL connection on port 992
2. Self-signed cert accepted by client
3. CA-signed cert validates correctly
4. Expired cert rejected

**Check 1:** openssl s_client connects successfully
**Check 2:** Wireshark shows encrypted traffic
**Check 3:** Invalid cert — connection refused with log entry

---

### Phase 20 — pcbis SMTP + DKIM
**Owner:** sysop/0
**Gate:** PCBoard sends outbound email that passes DKIM validation.

**Work:**
- SMTP server (RFC 5321) in pcbis
- DKIM signing for outbound mail (RFC 6376)
- Queue management (retry, bounce)
- PCBoard netmail integration
- MX record lookup via Watt-32 DNS (DOS) or system DNS (Linux)
- STARTTLS support via pcb_ssl
- Reference: VSOUPSRC smtp.cc (SMTP client, 12K lines)

**Test:**
1. Send email from PCBoard — arrives at destination
2. DKIM signature validates (check-auth@verifier.port25.com)
3. STARTTLS upgrades connection
4. Queue: failed delivery retries, bounces after max retries
5. Inbound: receive email, store as PCBoard netmail

**Check 1:** DKIM-Signature header present and valid
**Check 2:** SPF + DKIM + DMARC all pass at destination
**Check 3:** Bounce message format correct (RFC 3464)

---

### Phase 21 — pcbis FTP + FTPS + SFTP
**Owner:** sysop/0
**Gate:** FTP download of file from PCBoard file base works.

**Work:**
- FTP server (RFC 959) on port 21
- Active + passive mode
- Authenticated via PCBoard user database
- File base directory access per security level
- FTP over SSL (port 990, FTPS implicit + STARTTLS explicit)
- SFTP (SSH file transfer, port 22) — needs libssh2 or similar
- Upload/download to PCBoard file areas

**Test:**
1. FTP: login, list files, download — file matches original
2. FTP: upload file — appears in file base
3. FTPS: same tests over SSL
4. SFTP: same tests over SSH
5. Security: user without download access — transfer denied

**Check 1:** Downloaded file matches original byte-for-byte
**Check 2:** Path traversal (../../etc/passwd) blocked
**Check 3:** 5 simultaneous transfers — no socket leaks

---

### Phase 22 — pcbis Web + HTTPS + Certificate Management
**Owner:** sysop/0
**Gate:** HTTPS status page loads in browser with valid cert.

**Work:**
- HTTP/HTTPS server on ports 80/443
- Static file serving (file base, bulletins, display files)
- Dynamic pages: /status, /callers, /online, /areas
- Uses pcb_ssl for HTTPS
- Certificate management in pcbis TUI:
  Generate self-signed certs
  Load CA-signed certs (PEM format)
  Certificate validation for outbound connections
  Single config location for certs, ports, passwords, IP whitelist

**Test:**
1. HTTP: /status shows uptime, nodes, version
2. HTTPS: same page with valid cert — browser shows padlock
3. Self-signed cert: browser warns but connects
4. CA-signed cert: browser shows trusted
5. pcbis TUI: configure cert path, restart service, cert loads

**Check 1:** HTTPS cert chain validates with openssl verify
**Check 2:** All services share same cert from one config location
**Check 3:** IP whitelist blocks unauthorized access

---

### Phase 23 — pcbnet Multi-Node Networking
**Owner:** sysop/0 + wrench
**Gate:** Two machines share PCBoard nodes over serial + TCP.

**Work:**
- Server/client model for multi-machine multi-node
- Serial line bonding: 2 PCs x 2 COM ports = 4 nodes
- TCP node linking over LAN/WAN
- Shared data access: PCBOARD.DAT, USERS, CNAMES, message base
- System password + config password
- IP whitelist per workstation
- Configured in pcbis TUI
- PCBoard handshake protocol:
  Client: PCB_HELLO (version, capabilities)
  Server: PCB_CHALLENGE (auth challenge)
  Client: PCB_AUTH (password hash)
  Server: PCB_SESSION (granted services, node assignment)
- Reference: Windll.zip MD5 for password hashing

**Test:**
1. Two machines, serial null modem — nodes shared
2. Two machines, TCP LAN — nodes shared
3. Serial bonding: 2 lines = 2 extra nodes on remote machine
4. Shared user database — user logs in on machine B, record updates on A
5. Wrong password — connection rejected

**Check 1:** Handshake completes in under 2 seconds
**Check 2:** Data consistency: message posted on node 3 visible on node 1
**Check 3:** Serial + TCP simultaneously — mixed transport works

---

### Phase 24 — pcbnav PCBoard Navigator (Client)
**Owner:** TBD
**Gate:** pcbnav connects to PCBoard, user reads messages and transfers files.

Stubs only for initial phase. No code yet.

**Work:**
- GUI client application
- RS232 + Telnet + Telnet-SSL connection types
- Uses PCB_HELLO handshake from Phase 23
- Chat client (multi-channel)
- File transfer client (upload, download, browse file base)
- Address book (saved connections)

**Two builds, two UIs:**

- **pcbnav/DOS — text UI, from PCBMAIL's design.** Clark's PCBMAIL
  was a Windows GUI, but its *layout* is what matters: the address
  dialog (To / Subject / Cc), `@LIST@` group mailing, the
  private/public toggle, the addressed-names list at the foot of the
  editor. Rendered as a text-mode UI this is a natural PCBoard-style
  screen. CP437 throughout, which is native here rather than needing
  a font workaround. Phase 27 item 7 reproduces PCBMAIL itself and
  feeds this directly.

- **pcbnav/Windows — GUI, over OLMS.** Offline mail front-end for
  QWK/QWKE/Blue Wave: pick conferences, pull a packet, read and reply
  offline, upload the .REP. This *is* the reader front end from Phase
  O1 — the caller-side half. Backend is the shared `1541/wip/mail/`
  library, so the sysop-side door and this reader run the same packet
  code and cannot disagree about the format.

Both builds share the connection layer, address book and file
transfer. Only the presentation differs.

**UI spec source.** PCBMAIL.HLP (759K) is compiled WinHelp — decompile
with `helpdeco` (ANSI C, freeware, `pmachapman/helpdeco`) to recover
RTF topic text plus every embedded bitmap. The bitmaps are screenshots
of the actual dialogs and are a more precise spec than any prose
reconstruction. We hold both the original .HLP and a newer conversion;
treat the .HLP as authoritative and the conversion as convenience,
since conversions drop images and flatten the topic graph. See
`sdk/HELP-FORMATS.md`.

**Test:**
1. pcbnav connects via telnet to pcbis — login successful
2. pcbnav connects via SSL — encrypted session
3. Message reader: list conferences, read messages, post reply
4. Message editor: address with Cc and @LIST@, private toggle works
5. DOS build: text UI renders CP437 correctly at 80x25
6. Windows build: offline — pull QWK packet, read, reply, upload .REP
7. File transfer: browse file base, download file
8. Address book: save connection, reconnect (both builds)

**Check 1:** Handshake protocol matches Phase 23 spec
**Check 2:** Message display matches PCBoard ANSI output
**Check 3:** File transfer: downloaded file matches original
**Check 4:** .REP uploaded by pcbnav imports correctly via pcbolms

---

### Phase O1 — pcbolms (OpenOLMS, PCBoard fork)
**Owner:** hexadecimal
**Gate:** packet created on the BBS, read on the caller's machine, reply
posted back.

**A PCBoard-specific fork: licence unchanged (GPLv3), code changed.**
Not a port that tracks upstream — it diverges to be PCBoard-native.

**One shared library, two front ends.** The OL_* units do the packet
work; each front end is a UI over them, matching the UI/GUI split
already decided:

| Front end | Side | UI | Job |
|---|---|---|---|
| door | sysop / BBS | **text** — runs over serial or telnet | creates QWK packets |
| reader | caller's machine | **GUI** | reads packets, composes replies |

The door has to be text: it renders over the caller's connection. The
reader runs locally, so it gets the GUI. Same reasoning that puts
PCBMAIL's design on pcbnav DOS as text and OLMS on pcbnav Windows as
GUI.

*Naming still open* — whether `pcbolms` is the door, the reader, or the
whole package. The structure above holds either way.

**Work:**
- Shared library: `1541/wip/mail/` — QWK, QWKE, Blue Wave, Hudson,
  JAM, packer, transfer, filter (from `mterm/OL_*.pas`, 15 units,
  4,098 lines)
- PCBoard-native: read PCBOARD.SYS and the message base directly
- Door front end: DOOR.SYS and Door32 (DOOR32.SYS), standalone mode
  for testing
- Reader front end: GUI, shares the library — see Phase 24
- Enhances/replaces Clark's built-in QWK command in COMMAND.C
- Compiles with fpc264irc, go32v2 extender (CWSDPMI)

**Test:**
1. Door runs under PCBoard — user sees area selection menu
2. QWK packet created with selected conferences
3. Packet opens in a third-party reader (OLX, SLMR, Blue Wave)
4. Packet opens in our own reader front end
5. Reply written in our reader, uploaded, posted to conferences
6. Blue Wave packet creation and upload works
7. Door32 mode: same tests via DOOR32.SYS

Test 3 is the one that matters most — a packet OLX reads is correct by
definition, since that is what callers actually used.

**Check 1:** QWK 128-byte block format matches spec
**Check 2:** REP messages appear in correct conferences
**Check 3:** Blue Wave packet validates in Blue Wave reader

---

### Phase 25 — Conference Multi-Base (15.41)
**Owner:** hexadecimal
**Gate:** One conference holds multiple message areas and file directories.

Conference as container for N message areas + N file directories.
Same model Mystic BBS pioneered (g00r00). Class override on
conference object — old PPEs call base class, new PPEs call
extended class. No breakage.

**Work:**
- Conference holds N message areas (each: own base, FTN tag, QWK name, security)
- Conference holds N file directories (each: own path, security)
- Default area = first message area (old PPE backward compat)
- CNAMES.DAT extended with area/directory count fields
- PCBSETUP updated for area configuration
- File directory: uploader tracking, download counts, FILE_ID.DIZ auto-import

**Test:**
1. Conference with 3 message areas — user sees all 3
2. Conference with 2 file directories — user sees both
3. Old PPE reads default area only — no crash
4. FTN area tag per message area — echomail routes correctly

**Check 1:** Old PPEs work unchanged (default area)
**Check 2:** CNAMES.DAT backward compatible
**Check 3:** QWK packets include messages from all areas

---

### Phase 26 — PPL 3.51 Updates (15.41)
**Owner:** hexadecimal + sysop/0
**Gate:** PPL 3.51 source compiles with PPLC, old PPEs run unchanged.

Incremental from Clark's PPL 3.40. Versioned — old PPEs not affected.
New features require `;$LANGVERSION 351` directive. Conference
multi-base access via class override.

Credit: Mystic BBS (g00r00) for conference multi-base model.

**Work:**
- Conference/MBase/FBase class objects with member access
- AreaId(conf, area) — access specific message area
- ConfAreas(conf) — number of areas in conference
- ConfDirs(conf) — number of file directories in conference
- Variable initializers: INTEGER n = 1
- Compound assignment: +=, -=, *=, /=
- REPEAT ... UNTIL, LOOP ... ENDLOOP
- Typed constants: CONST
- PPLC updated for ;$LANGVERSION 351 directive
- DECLARE optional (compiler collects signatures)

**Test:**
1. Old PPE (no LANGVERSION) compiles and runs unchanged
2. PPL 3.51 source with AreaId() reads multi-base conference
3. Every PPE in reference/roysac/ still runs

**Check 1:** PPE binary format backward compatible
**Check 2:** New features gated by LANGVERSION directive
**Check 3:** Class override doesn't break base class calls

---

### Phase 27 — Missing Binary Reproduction (15.4)
**Owner:** sysop/0 + hexadecimal
**Gate:** All missing Clark binaries reproduced exactly, bug-for-bug.

Not clean-room. EXACT reproduction via binary analysis. Same behaviour,
same output, same bugs. Fix bugs in 15.41 only. Output verification
against Clark's originals.

Full inventory: `phase27/BINARY-CATALOG.md`.

**Rule: check for source before starting any binary analysis.**
PCBMODEM was on this list until a source check found the complete
C++ source in `Pcb-util/PCBMODEM/`, including `MODEMS.H` documenting
the MODEMS.DAT record layout. Source archives use password `pcb153`
(lowercase). Data formats are documented in `PCBXDOT/DOCDEV/`.

Order by difficulty (start simple, work up):

1. PCBVIEW chain — TESTFILE (2.5K), VIEWARCH (2.2K), VIEWZIP (7.8K).
   Fully specified in UTILITY.TXT. PCBVIEW.BAT calls TESTFILE to
   branch on extension, then VIEWARCH/VIEWZIP to write PCBVIEW.TXT,
   which PCBoard displays to the caller. Self-testing end to end:
   hand it a ZIP, diff PCBVIEW.TXT against Clark's.

2. RDPCBTXT (6.7K) — dumps PCBTEXT to PCBTEXT.LST. Exact inverse of
   MKPCBTXT, whose source we hold.

3. ENCRYPT — thin CLI wrapper over `Pcb-libs/SOURCE/MISC/CRYPT.C`,
   which is already ours (encrypt/decrypt, encrypt2/3 variants).
   PKLITE-packed; unpack only to confirm which variant and which
   USERS field offsets it walks.

4. Remaining small utilities — OVLSIZE (9K), PCBDESC (17K),
   MKPCBMNU (32K), UPGRADE (7.7K), FIXTEXT, PACKFIDO (23K),
   PCBNET (40K), VIEWFIX (5.7K). VIEWFIX is post-Clark (2000) and
   recreated for preservation rather than fidelity — it is an
   artifact of the community that kept PCBoard running after Clark
   closed, and 5.7K is cheap for that.

5. INSTALL.EXE (331K) — Script-driven installer.
   Reads INSTALL.DAT, decompresses .RED archives, 250+ @ commands.
   Source path was W:/master/install/main.c. NE/OS2 format, MS C
   runtime. Gated on the .RED decompressor (LZH-family, evidence:
   make_table / raw_in / expand_file strings).

6. PCBIC package — PCBoard Internet Component (April 1997)
   Pcbic.exe (313K), Pcbic2.exe (217K), PCBICCFG.EXE (185K),
   PCBICEVT.EXE (90K), TESTIC.EXE (40K), TESTIC2.EXE (47K).
   Services: FTP, Gopher, Finger, Ping, Telnet, RLOGIN, PPP/SLIP.
   Reference: PCBIC.DOC (112K), PCBIC.PDF (339K), RUNINET.PPS.
   Ancestor of our pcbis.

7. PCBMAIL.EXE (333K) — Win16 GUI message client (Borland C++ 4.50
   + BWCC). Message reader/editor with font selection, @LIST@
   mailing, Cc, private/public toggle. PCBMAIL.HLP (759K) is the
   behavioural spec. Reference design for pcbnav.

Running in parallel, gated on none of the above: **pcbcomm**, the
unified serial layer. Supersedes WCSC COMM-DRV — a port multiplexer
for multiport cards (Boca 16, DigiBoard COM/Xi, Arnet SmartPort
Plus). Intelligent boards carry an onboard CPU and dual-ported RAM
and no UARTs at all. COMM-DRV was an *optional* install group, never
present in a stock install, so replacing it costs nothing in
compatibility.

Pluggable backends: UART / FOSSIL / Win32 / POSIX (have these in
serial.c), plus new dumb-multiport and intelligent-multiport
backends, plus TCP for 15.41 only. Port table maps node -> backend,
matching DRVSETUP's screen so sysop config stays familiar.

**15.4 ships pcbcomm as a TSR** hooking INT 14h — sysops expect one,
their CONFIG.SYS and BOARD.BAT are built around it, and one resident
copy serves several nodes. **15.41 additionally offers a linked-in
build** for those who want the conventional memory back, and adds
TCP. Same source, two link targets.

**SDK: Clark already made the serial backend pluggable.** Toolkit3
ships COMMDRV.OBJ and FOSSIL.OBJ with identical Feb-1994 timestamps —
a door links one or the other into the same slot, API unchanged.
So pcbcomm is two artifacts, not one: PCBCOMM.EXE/.SYS is the driver
(was COMMDRV.EXE/COMMTSR.EXE), PCBCOMM.OBJ is the link-time client
stub (was COMMDRV.OBJ). The SDK doesn't compile pcbcomm — it consumes
it. Existing doors relink with a one-line .PRJ change, no source
edits. Must match the toolkit's Pascal calling convention and the
S/M/C/L x 3-compiler memory-model matrix, and follow the NOxxx.OBJ
link-out idiom so multiport code costs nothing on a single-modem
board.

References: Digi ClassicBoard hardware spec (covers both DigiBoard
and Arnet interrupt modes), FreeBSD `digi` (BSD), Linux `epca` (GPL).
This is the one Phase 27 component where bug-for-bug is explicitly
not the goal; we owe interface compatibility, not instruction
fidelity.

Crew: kiddo + wrench on serial core and UART/FOSSIL backends, evga
on multiport (Digi/Arnet/Boca), sysop/0 on PCBDraw (source from
sysop/0 + Mystic), hexadecimal on SDK packaging and docs.

SDK lives in `sdk/`. See `sdk/README.md` — the serial API is six
functions (initport, commportinkey, modemcommand, slowsendtomodem,
showmodem) plus a handful of globals; COMMDRV.OBJ and FOSSIL.OBJ are
two implementations of exactly those, and PCBCOMM.OBJ is a third.
TCP is not in the toolkit (it is 1994) and teleconference does not
belong there at all — it lives inside PCBOARD.EXE, doors never call
it.

**Third party, not reproduced:**
DOORWAY.EXE is TriMark Engineering shareware, bundled by default and
invoked through REMOTE.SYS (the `9` SysOp remote-DOS command), not
from the menu system — Clark's source never names it. BC450RTL.DLL
and BWCC.DLL are Borland redistributables needed by PCBMAIL only.
Provenance unconfirmed: APPLYCFG.EXE (appears to be SModem).

**Method — Research, Examine, Test, Isolate, Repeat:**
1. Check the source archives first. If source exists, build it.
2. Check `DOCDEV/` for the file formats the binary touches.
3. Examine: strings, event tables, timers, imports, resources.
4. Disassemble (IDA/Ghidra) only what steps 1-3 leave unexplained.
5. Write C source producing identical output.
6. Isolate: verify one behaviour at a time against the original.
7. Ship as 15.4 — bugs and all. Fix bugs in 15.41.

**Test:**
1. PCBVIEW chain: PCBVIEW.TXT byte-identical to Clark's
2. RDPCBTXT: PCBTEXT.LST byte-identical
3. ENCRYPT: USERS file round-trips through Clark's and ours
4. PCBIC: FTP/telnet sessions match Clark's behaviour
5. PCBMAIL: message read/write matches Clark's client

**Check 1:** Output comparison passes against Clark's originals
**Check 2:** INSTALL.DAT script runs identically
**Check 3:** No new features — exact 15.4 reproduction

---

### Phase 28 — SDK
**Owner:** hexadecimal
**Gate:** A door builds from rebuilt sources, links any serial backend,
and runs under PCBoard.

**Two versions, same split as the rest of the project: 15.4 restores
what Clark shipped, 15.41 adds what we build.**

| | Toolkit 15.4 | Toolkit 15.41 |
|---|---|---|
| Purpose | rebuild a 1995 door from source | write new doors |
| Target | 16-bit, 4 models, 3 compilers | **16-bit and flat** |
| Convention | Pascal, as Clark had it | C, Pascal variant available |
| Language | pure C | C API, C++ internals allowed |
| Contents | Clark's 198 functions | Clark's 198 **plus crew code** |
| Status | frozen | grows |

The versions differ in **content**, not architecture. 15.4 is frozen
at exactly what Clark shipped — the moment we add a function it is no
longer that. 15.41 grows. Both build 16-bit.

**Crew code is not a separate library — it is what 15.41 adds.** A
sysop writing a door in 2026 links the 15.41 toolkit and gets Zmodem,
QWK, RIP, SMTP, TCP and everything else we build. Walling that off
would protect 1995 doors at the cost of starving new authors.

**Freezing 15.4 costs nothing.** Existing door *binaries* do not use
the toolkit — they are already compiled. The toolkit only matters to
someone *building* a door. So 15.4 is a preservation artifact for
rebuilding period software with a period compiler; 15.41 is the living
one everyone uses.

**C++ across compilers:** C++ has no stable ABI (Clark knew — 88 C++
files internally, zero in the toolkit), so 15.41 uses a C API with C++
internals allowed, `extern "C"` on the surface.

**Memory models — write 16-bit clean, build both.** Flat 32-bit was
never a decision; it came in with code ported from Mystic. 16-bit-clean
C compiles fine in flat model and the reverse does not, so writing crew
code 16-bit clean costs nothing and gets both targets from one source —
and a 16-bit door author is not shut out of Zmodem, QWK or RIP.

Clark's library shows the real cost: **16 of ~240 files use far/huge
pointers**, about 7%, concentrated in large-array sort/search, block
memory ops and screen buffers. `MISC/VIRTUAL.C` is a disk-backed
virtual memory layer with a block cache for anything larger.

Of the nine crew modules, seven are 16-bit clean cheaply — comm, xfer,
ftn, mail, net, crypto already work in buffers well under 64K, so it is
discipline rather than rework. `pcb/` uses the VIRTUAL.C pattern for
large indexes.

**`term/` and `draw/` port from Pascal to C.** kiddo's RIP engine and
canvas (mtrip.pas 946 lines, mtripgfx.pas 859 lines, all 53 v1.54
commands, zero stubs) and PCBDraw's editor are Free Pascal today. They
port to C and land in the toolkit; `mterm` becomes `pcbterm` inside
pcbnav.

**And they can be 16-bit clean.** The Pascal canvas is byte-per-pixel
(224,000 bytes, plus another 224,000 for the flood-fill visited
buffer). Two changes fix that and improve the code anyway: a
row-pointer array (350 rows x 640 bytes, no allocation over 64K) and a
bitmask visited buffer (224K down to 28K, one segment, faster to
clear). So all nine modules are 16-bit clean — the earlier flat-only
carve-out is withdrawn.

MDL mostly does not port. Its 21 units and 9,312 lines of strings,
datetime, input, output and menu duplicate what the toolkit already
has in SCREEN/ (41 files) and MISC/ (90 files) plus the inputfield
family.

**Four RIP engines, four reconstructed specs — keep them separate.**
The RIPscrip API was lost. v1 (4,186 lines) is the 51 v1.54 commands.
v2 (5,394) was decoded from RIPaint 2.1 scene files — file-format
observation, no code taken. v3 (8,371) was confirmed against RIPtel
Visual Telnet 3.1. v4 (8,646) extends v3. That is Phase 27's binary
analysis applied to a graphics protocol; collapsing them into tiers
would destroy the record of what was recovered from where.

v4 is not "plus printers" — it is Unicode CP437/UTF-8, TTF loading,
MPEG full-motion video, FLI/FLC animation, 32-voice FM MIDI synthesis,
a complete HTML 1.0 parser with DOM/layout/renderer, and a six-driver
print API.

**Real scale: ~89,600 lines**, not the 34,000 an earlier draft
recorded. Engines ~27,500; `img/` 8,625 (32 units); `wav/` 13,597
(44 units); `pasjpeg/` 33,843 (58 units); `prg/` 1,948; `prt/` 882.

**Several subsystems under `v4/` are not RIP.** `wav/` is a general
audio library — MP3 with full huffman/IMDCT/synthesis, 32-voice FM
MIDI, MOD/S3M/XM trackers, FLAC, ADPCM, AIFF, AU, VOC, a Sound Blaster
IRQ-driven DMA driver, a 16-stream mixer. **This largely completes
Phase 12 (pcbwave)**, whose owner is still listed TBD; what remains
there is PCBoard-side wiring, not codecs. `pasjpeg/` is an Independent
JPEG Group port — third-party provenance, a third of the package by
line count. The HTML engine is an HTML engine. Organise by what things
are, not which directory they landed in.

**Attribution:** kiddo built the v1-v4 engines and `mtrip.pas`, plus
mterm, ansiedit and the MDL units. sysop/0 built the rest of
`mystic_rip/` — codecs, HTML, print drivers, audio, utilities.

As shipped 2026-08-19 the two are unconnected — nothing outside
`mystic_rip/` references the v-engines, `mtrip.pas` is a standalone
v1.54 dispatcher, and `mdl/` contains no RIP at all. If kiddo has since
moved to v4 in MDL that is newer than this package; confirm before
planning around it.

**pcbterm cannot use v4 on DOS — and the reason is not buffers.**
`rip4api.pas` has a hard `Uses` clause naming 42 units including TTF,
MPEG, HTML and PasJPEG. That is not a linker-stripping question; the
unit cannot be *compiled* for i8086 at all, whatever subset you call.

**Deeper: v1-v4 are server-side renderers.** Per rip4api's own header,
they render internally and send the resulting image out as ANSI or
bitmap. `mtrip.pas` is a client-side parser — receives RIP, draws
locally. Opposite data flows. pcbterm is a client, so `mtrip.pas` is
architecturally correct, not merely smaller.

**Two engines** — one server, one client. DOS i8086 confirmed live and
real-mode.

  m_rip.pas    from v4, plugin codecs via registration   server-side
  mtrip.pas    exists, 946 lines                         client-side

`m_rip_dos.pas` was killed: with codec registration the DOS engine is
the *same* m_rip.pas with no codecs linked. Link-time decision, not a
source fork. Pattern taken from prnapi.pas, which already does this
with print drivers. v1 and v3 retire to attic (subsets of v2 and v4).

**Two gaps registration does not close.** Codecs plug out because they
are separate units; these are not. (a) The v3/v4 command handlers live
inside m_rip.pas — ~3,250 lines (rip4api 8,646 vs rip2api 5,394) that a
DOS build compiles and never calls. Fix by making the dispatch table a
registration point too, so handlers register like codecs and DOS links
only the v1.54/v2 set — one mechanism instead of ifdefs. (b)
`SavedScreens: Array[0..9] of PRIPPixelBuffer` is 2.24 MB if used and
is unmentioned in the migration plan; options are disk-backed slots via
Clark's MISC/VIRTUAL.C block cache, EMS/XMS, or fewer slots on DOS.

**Provenance: cite the white papers, and audit out the product
claims.** The engines' READMEs claim v2 was "decoded from RIPaint 2.1
scene files" and v3 "confirmed via RIPtel Visual Telnet 3.1". Per
verta1878 those products were not used — the work came from published
white papers, all present in the package (RIPSCRIP_v154.DOC,
RIPScrip-2.0-alpha-4.txt, RIPScrip-3.x-technical-whitepaper.txt, and
two implementation white papers). Those claims are inaccurate and need
auditing out.

242 live-tree files mention the product names, in three groups: 17 are
the source documents themselves (do not touch — TeleGrafix wrote them,
stripping their name would falsify the sources), 13 are third-party
RIP art, and ~27 are our own derivation claims. Five of those have a
product name in the filename, so renaming is part of it. The audit is
re-citation, not deletion — same findings, source attributed to a
document instead of a product.

Provenance markers during the merge then become document citations:
`// v2.0 — RIPScrip 2.0 alpha 4 whitepaper, section 3.2`. Verifiable
against a file in the repo, which is stronger than an inference record.

**Real-mode memory.** m_rip.pas on DOS is server-side, sharing 640K
with DOS, the BBS, the comm driver and TSRs. The 640x350 indexed buffer
is 224,000 bytes on its own. The migration plan's row-pointer layout
handles the per-allocation 64K limit correctly — 640-byte rows, and a
bitmask visited buffer at 80 bytes per row for 28K total, down from
224K.

**pcbterm's GUI is a presenter, not a second engine.** `rip_surface.pas`
already names the seam — *"a presenter (rip_window.pas via sdl_bind
today; LCL/BGI later)"*. The engine produces a pixel buffer; a
presenter displays it. Three of them in `toolkit-15.41/terminal/`:
`present_text.c` (half-blocks, 80x22 — the fallback for plain telnet
with no graphics surface), `present_vga.c` (DOS, mode 12h planar to
A000:0000), `present_sdl.c` (Windows/Linux/macOS — SDL is already a
dependency via mtsound). pcbterm picks one at startup; adding another
later touches nothing else.

Command counts reconciled — not a spec disagreement. kiddo counted `>`
(EraseEOL) as distinct from `K`, and `1W` (WriteIcon) in Level 1
dispatch where ripscr.pas had it as an uncounted no-op. Same spec,
different treatment of aliases and no-ops. Settle it before the port so
both report the same figure.

**OpenOLMS arrived in the 2026-08-19 package** — `mterm/OL_*.pas`, 15
units, 4,098 lines: QWK, Blue Wave, Hudson and JAM packet formats,
packer, transfer, editor, filter, config, users, drop file. That is
Phase O1's pcbolms and Phase 24's offline-mail UI already written. It
lands in `1541/wip/mail/` so the sysop-side door and pcbnav share
one implementation.

**Licence: GPLv3**, settled — permission from Peter Rocca, matches the
project licence. Update the `OL_QWK.pas` header from "Pending licence".

**Untested, but the easiest piece to test — so do it first.** QWK is a
data format with objective pass/fail, unlike RIP where verification is
a person judging 101 BMPs. Oracles: round-trip byte comparison;
generate a packet with PCBoard and read it with OpenOLMS (Clark's QWK
code is the reference for a PCBoard-facing reader); write a .REP and
import it into PCBoard — if PCBoard accepts it, it is correct; and
interop with OLX, SLMR or Blue Wave under DOSBox, since a packet OLX
reads is correct by definition. 4,098 lines against RIP's 86,000, no
RIP dependency, feeds Phase O1 and Phase 24, and it proves the
test-then-port method at a scale where a flaw in the method is cheap to
find.

**Test before porting — v1-v4 are untested.** The package has 441 RIP
files and exactly two BMPs, so no baseline exists; the harnesses
(`mtrip_test`, `test_rip_files.pas`) have never been run as an
acceptance suite. Diffing a C port against untested Pascal output would
enshrine its bugs as the spec and make port bugs indistinguishable from
original ones.

For reconstructed code the corpus is not verification, it is the
specification — v2 was derived by observing RIPaint 2.1 scene files, so
running them is the last step of the reconstruction rather than QA
afterwards. Sequence: run the corpus, review by eye (RIP faults are
visually loud), fix in Pascal, re-run, and only then baseline. Never
port a known bug meaning to fix it in C — that splits the fix across
two codebases. Priority: v1.54 corpus, then v2 against its RIPaint
source material, then v3, then v4.

**pcbirc does not carry the Pascal.** It lives in
`verta1878/mysticbbsirc`, maintained by kiddo. pcbirc holds the C port
and a pinned reference — one maintained home, no second copy to drift.
One port record — `sdk/PORT-RECORD.md` — maps every ported C file to
its Pascal source, with a single commit line covering the batch.
Attribution travels in Clark's existing banner as one extra line
(`RIP engine ported from Pascal by kiddo, verta1878/mysticbbsirc.`)
rather than a separate block that would fight the house style.

Mystic's `attic/` is not carried — retired Mystic engines belong in
mysticbbsirc. pcbirc's `attic/` is for retired PCBoard code.

One exception worth holding locally: **the test corpus**. It is the
acceptance criteria for pcbirc's own C code, so if it lives only
upstream and changes, pcbirc's baseline moves without pcbirc changing.
Keep it, or reference it pinned — but not unpinned.

Port detail and order: `sdk/PORT-PCBDRAW.md`. Placement in the pcbirc
tree: `sdk/PLACEMENT.md`.

Full detail: `sdk/ARCHITECTURE.md` (also explains Pascal calling
convention and memory models plainly), `sdk/README.md`,
`sdk/SOURCE-RECOVERY.md`.

**28a — Rebuild the core toolkit**

Source is present and scattered across seven directories; the gap is
build files, not code.

- Library body from `Pcb-libs/SOURCE/` — MISC (90), DOS (47),
  SCREEN (41), PCB (22), SCRNIO (20), COUNTRY (13), SYSTEM (7)
- Verify exported symbols match PCBTOOLS.H's 198 declarations
- Memory-model matrix per `TOOLKIT/OTHER/MODELS.BAT`: one source,
  four models via `__s__`/`__c__`/`__m__`/`__l__`, per-model output
  dirs. Three compilers x four models = twelve variants, one loop
- Preserve the design intent from `TOOLKIT/OTHER/FEATURES`: 52
  high-level functions, one call to init a door (config + comm port +
  USERS.INF TPA update). That one-call simplicity is what authors used

**28b — Finish the stubs**

All seventeen have source in `Pcb-libs/SOURCE/TOOLKIT/`:
NOANSI, NOCHAT, NODISP, NOHELP, NOINPUT, NOLANG, NOLOG, NOMEMORY,
NOPCBSYS, NOPRINT, NOSCREEN, NOSHELL, NOSTATUS, NOSYS, NOTXT,
NOUPDSYS, NOXLATE, plus SMALLDLY, SMALLERR, SMALLSUB, SMALLTXT,
PCBDAT.

One `.OBJ` per `.C`, per memory model. Note NOPCBSYS and NOPRINT have
source but shipped no `.OBJ` in Toolkit3 — build them and find out
why, or confirm they were simply dropped.

**28c — Serial backends**

ABI recovered from the object files: symbols are `ASYNC_` + uppercase
name (C linkage, Pascal convention's uppercase folding). Both
COMMDRV.OBJ and FOSSIL.OBJ export the same 24-symbol core — INIT,
OPENCOM, CLOSECOM, SETPORT, CSENDBYTE, CSENDSTR, CGETBUF, CGETSTR,
COMMINKEY, CDSTILLUP, ONLINE, CHECKCOMM, TURNON/OFFDTR, TURNON/OFFRTS,
TURNONFIFO, TURNONXMIT, CLEARINBUF, CLEAROUTBUF, COMMGO, COMMPAUSE,
COMMSTOP, DISCONNECTMODEM.

They differ at the edges: COMM-DRV adds error statistics
(OVERRUNERRORS, PARITYERRORS, RINGDETECT, REOPENPORT, OPENMODEM)
because it owns the hardware; FOSSIL adds BAUDDIVISOR because the spec
exposes it. **pcbcomm implements the union** — it owns the hardware so
it can supply the counters, and can expose the divisor too. Anything
that linked either old backend then links pcbcomm unchanged.

- `FOSSIL.OBJ` from `Pcb-main/SOURCE/MODEM/MODEMFOS.C` (829 lines)
- `COMMDRV.OBJ` from `MODEMDRV.C` (521 lines) — the COMM-DRV *client*,
  which is ours to build. The ABI it uses is recovered: INT 14h with
  AX=1000h/1002h returning a port control block, then direct reads of
  `inbuf_count`, `outbuf_count`, `msr_reg`, `cardtype`, `flag` and the
  `auxpcb` error counters
- `PCBCOMM.OBJ` written against the same seam, so pcbcomm drops into
  the slot COMMDRV.OBJ and FOSSIL.OBJ already occupy
- Carry over Clark's DigiBoard COM/Xi workaround: the card reports
  buffer counts lazily, so a zero count is not trusted — detect DIGCXI
  and make an extra fetch that refreshes the counter

**28d — Consolidate crew code into Toolkit 15.41**

The same code currently exists several times over: FOSSIL in both
MODEMFOS.C and QFront's serial.c, Zmodem in QFront and Pcb-misc and
ZMRECV/ZMSEND, RIP soon in both PCBDraw and kiddo's terminal. Every
copy drifts and every fix gets missed somewhere.

  toolkit-15.41/comm/    UART, FOSSIL, multiport, TCP  (= pcbcomm)
  toolkit-15.41/xfer/    Zmodem, Xmodem, Ymodem, HS/Link
  toolkit-15.41/ftn/     BSO, EMSI, YooHoo, nodelist, .PKT, TIC
  1541/wip/mail/    QWK, QWKE, Blue Wave          (from OLMS)
  toolkit-15.41/net/     SMTP, POP3, NNTP              (from VSOUP)
  1541/wip/term/    ANSI, RIP, terminal emulation (kiddo)
  toolkit-15.41/draw/    canvas, drawing tools         (PCBDraw)
  toolkit-15.41/crypto/  MD5, DES, IDEA, MDC
  1541/wip/pcb/     PCBOARD.DAT/SYS, USERS, CNAMES, msgbase
  toolkit-15.41/clark/   Clark's 198 functions, flat-model build

QFront splits about in half — serial/modem/zmodem/xmodem/emsi/wazoo/
nodelist/bso/tic/route move out; qfront/qfconfig/qscan/qnlist/qfutil/
session/events/semaphore/frequest stay as mailer policy. PCBoard keeps
MODEM.C as policy but its backends become 15.41 comm/ backends. ZMRECV
and ZMSEND become thin wrappers over 15.41 xfer/.

**VSOUP is ported, not bundled.** It is GPL so folding it into
the 15.41 toolkit's net/ is permitted, and it is untested — which is the argument for
porting rather than carrying it. Bundled untested code is code nobody
owns. Bring SMTP/POP3/NNTP into project style, build under OpenWatcom,
test against a real server before anything depends on it, and keep GPL
attribution in the headers.

`third-party/` is then reserved for code we genuinely do not touch —
unmodified, own licence file. If we are editing it, it belongs in
the toolkit with its origin recorded.

**term/ and draw/ are deliberately separate.** term/ receives and
renders a remote session; draw/ creates graphics. PCBDraw uses term/
to display; a terminal needs no drawing tools.

**28e — Documentation**

Rebuild the toolkit manual from Clark's 392K `DOCS`, updated for the
new backends and libraries. Keep the chapter structure — door authors
know it.

**Test:**
1. Rebuilt `PCBKIT_S.LIB` exports every symbol PCBTOOLS.H declares
2. Sample door from `TOOLKIT/SAMPLES/` relinks and runs
3. Same door relinks against FOSSIL.OBJ, COMMDRV.OBJ, PCBCOMM.OBJ in
   turn — one `.PRJ` line changed, no source edits, all three work
4. Linking NOCHAT.OBJ + NOSCREEN.OBJ measurably shrinks the binary
5. A door builds a QWK packet via the OLMS library
6. A door draws via the PCBDraw library
7. All twelve memory-model variants build from one invocation

**Check 1:** Pascal calling convention preserved — existing 16-bit
doors relink unmodified
**Check 2:** No Clark-derived and third-party code mixed in any
directory; licences correct per component
**Check 3:** Binary sizes comparable to Clark's originals
**Check 4:** Each program still builds and behaves identically after
its subsystem moves — one subsystem at a time, verified, then the next

**Sequencing note.** Phase 15 (QFront Clark-style conversion) is 17/21
and on hold. Ten of those files are destined for the 15.41 toolkit. Style carries
over unchanged, but do the move before finishing the last four, or
they get styled in a tree they are about to leave.

---

## Updated Phase Dependencies

```
Phase 0 ✅ (done)
Phase 5/5a ✅ (done)
  └── Phase 1 (Watcom port completion)
        ├── Phase 1b (15.3 -> 15.4 upgrade)
        │     └── Phase 1a (Private messages — finishes 15.4)
        │           └── Phase 25 (Conference multi-base)
        │                 └── Phase 26 (PPL 3.51 updates)
        ├── Phase 27 (Missing binary reproduction — 15.4)
        ├── Phase 2 (FidoNet suite)
        │     ├── Phase 3 (EchoMail/Areafix)
        │     └── Phase 4 (TIC file echo)
        ├── Phase 6 (pcbis telnet — Linux)
        │     ├── Phase 7 (pcbis BinkP)
        │     └── Phase 9 (pcbis NNTP/QWK/UUCP2)
        ├── Phase 10 (SIO/Cyclades drivers)
        ├── Phase 11 (FOSSIL suite)
        └── Phase 12 (pcbwave audio)

Phase 14 (DOS/32A switch) — can start immediately
Phase 15 (Code style audit) — can start immediately

Phase 16 (PCBDraw) — after Phase 1
Phase 17 (Watt-32) — after Phase 14
  └── Phase 18 (pcbis telnet DOS+Linux)
        └── Phase 19 (SSL/TLS)
              ├── Phase 20 (SMTP + DKIM)
              ├── Phase 21 (FTP + FTPS + SFTP)
              └── Phase 22 (Web + HTTPS + Certs)
                    └── Phase 23 (Multi-node networking)
                          └── Phase 24 (pcbnav — stubs)

Phase O1 (pcbolms) — independent, can start anytime
  └── feeds Phase 28d (OLMS into Toolkit 15.41)

Phase 28 (SDK) — 28a/28b/28c can start immediately (source in hand)
  28c pairs with pcbcomm
  28d needs Phase O1 (OLMS) and Phase 16 (PCBDraw)

Phase 13 (Release packaging) — requires all others
```

Phase 8 (old FTP/HTTP/SMTP) is superseded by Phases 20/21/22 with
proper SSL, DKIM, and certificate management.

All network services configured in pcbis TUI (Phase 22).
ntvdmx64 DPMI fix is a separate project, not a PCBoard phase.
DOS/32A replaces DOS/4GW for all OpenWatcom builds (Phase 14).
FPC builds stay on CWSDPMI/go32v2 — different extender.

Reference material added to reference/:
- VSOUPSRC.ZIP — SMTP/POP3/NNTP client source (C++, GPL)
- Windll.zip — DES/IDEA/MD5/MDC crypto source (freeware)
- Mystic BBS (g00r00) — conference multi-base model (Phase 25 reference)
- mkrueger/icy_board — PPL toolchain reference (Phase 26 reference)

## PCBOARD.DAT Extension (Deferred)

No PCBOARD.DAT changes needed for Phase 5/5a — ISO config uses the
64 reserved bytes in CNAMES.ADD (per-conference, already exists in 15.4).

Future PCBOARD.DAT extensions (if any feature requires global config
that doesn't fit per-conference) will append after NetCopy[32] using
the same pattern Clark used between versions. upd1541.exe would
migrate existing files. No fields defined yet.

## Rules

1. **Written once, checked three times.** Author writes code. First reviewer
   checks logic. Second reviewer checks edge cases. Third reviewer runs tests.
2. **Each phase has a gate test.** Phase is not complete until gate passes.
3. **No skipping.** Phase 1 must pass before any 15.41 phase starts.
4. **Self-contained.** Each phase can be tested independently. No phase
   depends on unfinished work from another parallel phase.
5. **Backward compatible.** Every 15.4 feature still works in 15.41.
   New features are additive. Nothing is removed.
