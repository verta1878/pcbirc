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
**Gate:** All 28 Clark binaries compile under openwatcom2irc. Zero warnings.

**Work:**
- Complete WATCOMPAT.H for remaining 157 source files
- Fix constrea.h stub (C++ iostream compatibility)
- Fix header ordering (per hexadecimal-phase-update.md)
- Per-binary include path fixes
- Compile all .CPP files with bwpp386

**Test:**
1. wcc386/wpp386 compiles every .C/.CPP in PCBSRC/ — zero errors
2. All 28 binaries link — zero unresolved
3. DOS4GW smoke test: each binary runs and shows usage/help screen
4. No regressions: existing Phase 0 binaries still link

**Check 1:** hexadecimal reviews all WATCOMPAT.H additions
**Check 2:** sysop/0 verifies binary sizes are reasonable vs Borland originals
**Check 3:** Run PCBoard in DOSBox-X, local login, post message, download file

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

**Uses FilebaseFlags[64] in CNAMES.ADD — no PCBOARD.DAT changes, no upd1541.**

FilebaseFlags[64] layout per conference:
- Byte 0: ISO flag (0=normal, 1=ISO-backed)
- Bytes 1-63: source path (62 chars + null)

**Work:**
- PCBISO /SETISO <confnum> <path> — set ISO flag + path in CNAMES.ADD
- PCBISO /CLEARISO <confnum> — clear ISO flag for conference
- Reads/writes CNAMES.ADD FilebaseFlags[64] directly
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
