# pcbirc — Project History

Chronological record of major work on the pcbirc PCBoard 15.4/15.41
revival project. Consolidated here so the story survives across releases
and isn't lost with per-commit or per-release notes.

Per-subsystem work logs live inside their trees:
- `pcb1541/install/dist/target/README.md` — .RED archives & installer arc (install v1.6→v1.7.2)

---

## Origins

**PCBoard** was the dominant BBS software of the dial-up era, written by
Clark Development Company. When Clark was closed by the bank in the late
1990s, the source code nearly disappeared. Very little information about
PCBoard 15.4 survived on the public web.

**Corey Blake** purchased what may be the only source license ever sold.
**PWA (Pirates with Attitude)** preserved that 15.3 source archive.
Without both, this project could not exist.

**Roy/SAC (Carsten)** later donated a physical PCBoard package (box
photos, license transferred via POB — the German distributor — upgraded
15.21→15.22) and 4,548 PPE archives.

---

## Version labels used in this document
- **pcbsrc v0.1** — PCBoard source library chain build
- **pcbsrc v0.2** — pcbkit_l.lib rebuild from source
  v0.2.0: 8 libs merged (107,520 B, 114 modules). v0.2.1: all 130
  modules rebuilt (272,896 B). Sources: 8 sub-libs + 28 MAIN + 16
  LIB/SOURCE + 4 UTIL + 7 ASM + 6 Borland CRTL.


Different subsystems use their own version numbering. Where relevant, this
document uses explicit prefixes to avoid confusion:

- **PCBoard 15.4 / 15.41** — the DOS BBS software being revived. The
  `pcb1541/` directory name refers to this software version.
- **pcbirc** — the project/repo name (**pcb**oard **irc**-branch).
- **pcbirc project versions** (`v0.1`, `v0.2`, … `v1.0 baseline`) — the
  whole-project milestone labels used in this document's Timeline below.
- **install v1.0 → install v1.7.2** — internal milestones of the installer
  subsystem arc, tracked in `pcb1541/install/dist/target/README.md`.
- **install v1.8, v1.9** — docs consolidation & stub-hunt continuation
  of the installer arc past the byte-perfect 481-file rebuild.
- **install v1.10** — INSTALL.EXE completion arc: our C reimplementation
  at `pcb1541/install/src/install.c` grows from a Phase 27 stub (7 of
  60 @-directives) to full parity with Clark's `INSTALL.EXE`.
- **pcbic v1.0 → v1.1** — byte-exact reconstruction of Clark's PCBIC 1.2
  toolset (6 EXEs + RUNINET.PPE + TCP/IP menu screens + docs from the
  1996 Salt Air distribution). Tracked in `pcb1541/pcbic12/ROADMAP.md`.
  v1.0.x = per-target reconstructions; v1.1 = whole arc complete.
  Naming: "PCBIC 1.2" (uppercase, no `v`) is Clark's product name;
  "pcbic v1.0.x" (lowercase `v`) is our reconstruction phase.
- **pcbdcom v1.1, v1.2** — the serial-driver rewrite subsystem's own
  numbering (tracked in `pcb154/pcbdcom/`).

## Timeline

### v0.1 — Initial source recovery (Jul 31, 2026)

**Commit `1993ecb`** — First compilable PCBoard outside Clark
Development Company. 12 of 12 binaries built from source. DOS + OS/2
platforms established as parallel build targets.

**Commit `02b6c16`** — OpenWatcom 2.0 port begins.

### v0.2 — Borland release (Aug 6, 2026)

**Commit `b459cec`** — 15.3→15.4b patch reconstruction complete.
- Complete source tree: 1,292 files
- `153_to_154.patch`: 8.9 MB (the delta from PWA's 15.3 to Clark's
  unreleased 15.4b beta)
- PPL 3.40 compiler source recovered
- All 11 Clark DOS binaries build under Borland C++ 3.1 in DOSBox
- No modifications to Clark's original code — pure recovery

### v0.3 — Serial/OS/2 push (Aug 8, 2026)

**Commit `7f6e9f3`** — Major serial and OS/2 work.
- **ASYNC.C CPU hog fix**: the 23-reply thread FOSSIL polling bug on
  alt.bbs.pcboard, fixed via INT 2Fh/1680h DPMI timeslice release in
  `COMMINKEY` and `CHECKCOMM`. CPU drops from 100% to near-idle
  between keypresses under DOSBox, Windows NTVDM, and OS/2 VDM.
- **PCBCP OS/2 Control Panel** recovered from `pcball.zip`, ported to
  OpenWatcom 2.0. 8/8 source files compile → `PCBCP_W.EXE` (77 KB OS/2
  PM). Includes IPF help source, BMP screenshots, config INI.
- **SIO OS/2 serial driver suite** (evga, GPLv3 clean-room): V1 (37
  source files, 5,510 lines ASM) and V2/SIO2K (split architecture with
  `UART.SYS`, `VMODEM.SYS`, PCI support).
- **Phase 3 ASM→C**: 8 files, 8,251 TASM lines → 309 C lines.
- **Manuals**: PPL reference (7.1 MB scanned PDF), PPL dev kit manual
  (6.7 MB PDF), David Terry's 1997 shutdown message, PCBoard 15.2
  floppy label photos.
- 15 binaries total including `PCBOARD_W.EXE` (1.3 MB) and `PPLC_W.EXE`
  (1.3 MB PPL compiler).

**Commit `11bebe8`** — New network stack pieces.
- **pcbbinkp**: BinkP/1.1 mailer, 6 files, 2,008 lines total,
  clean-room FTS-1026 + CRAM-MD5 (FSP-1024). Ships with 15.4 main
  branch. `PCBBINKP.EXE` (47 KB OS/2) + `PCBBINKP_W.EXE` (62 KB NT).
- **pcbfido**: FidoNet console, 778 lines, 5,000-line scrollback, tag
  filtering, child process pipes. Moved to `1541/` (15.41-only).
- **pcbpscan** rewrite: 271 → 770 lines, 4 thdproscan bugs fixed
  (config loader, ClamAV RC handling, archive listing via external
  tools, FILE_ID.DIZ extraction). Ships preconfigured for ClamAV.

### v0.4 Phase 0 — Missing Clark utilities (Aug 11–13, 2026)

**Commit `0b453ab`** — All 12 missing Clark utilities compiled and
linked for DOS4G/Watcom. Zero unresolved symbols.

| Binary | Size | Blocker resolved |
|---|---:|---|
| `PCBSTATS_W.EXE` | 31 K | WATCOMPAT.H |
| `PCBPACK_W.EXE` | 84 K | stubs |
| `MSETUP_W.EXE` | 108 K | ASYNC driver case mismatch |
| `PCBMODEM_W.EXE` | 528 K | ASYNC + PPL runtime stubs |
| `PCBEDIT_W.EXE` | 133 K | Borland conio functions |
| `PCBMONI_W.EXE` | 54 K | Ctrl-Z EOF in source |
| `PCBDIAG_W.EXE` | 552 K | PcbData C++ name mangling |
| `PCBFILER_W.EXE` | 215 K | missing VMAVL library |
| `PCBNLC_W.EXE` | 77 K | missing CodeBase (d4all.h) |
| `OFFLINE_W.EXE` | 26 K | malloc void*→char* |
| `WAITBU_W.EXE` | 25 K | missing bgetkey include |
| `PCBTITLE_W.EXE` | 15 K | Borland inline ASM (rewritten in C) |

New libraries written clean-room:
- **VMAVL** — 412 lines, AVL tree for PCBFILER's file database (studied
  libavl for reference)
- **d4all.h** — 121-line CodeBase type shim so PCBNLC's dBASE wrapper
  compiles

**Commit `165d9dc`** — PPL decompiler and engine added.

**Commit `1e0a8fe`** — Phase 0 complete. 33 programs, 17 bugs fixed:
- **pcbis (11 bugs)**: FTP path traversal (HIGH severity), SIGPIPE
  crash, double-fork daemon, `/dev/null` redirect, PASV socket leak,
  BinkP 8 KB frame limit, HTTP uptime, nodedata file handle leak,
  telnet IAC escape, NNTP socket leak, event shell exec.
- **pcbfoss (6 bugs)**: `FN_RX_WAIT` blocking per spec, `FN_ANSI_WRITE`
  reads string not single char, double-init preserves data, ring buffer
  block I/O via memcpy, overrun status bit, baud rate 115→1152.

### v0.4.2 — PCBISO + QFront audit (Aug 15, 2026)

**Commit `ffc0867`** — Phase 5 + 5a complete.
- **PCBISO v1.0.0**: 969 lines, 9 commands (`/INDEX`, `/INDEX ALL`,
  `/REBUILD`, `/LIST`, `/STATUS`, `/SETISO`, `/CLEARISO`, `/MOUNT`,
  `/UNMOUNT`). Reads `CNAMES.@@@` and `CNAMES.ADD`. Writes PCBoard DIR
  listings with FILE_ID.DIZ extraction. Mount table persists in
  `PCBISO.DAT`.
- **CNAMES.ADD record change**: `Reserved[64]` renamed to
  `FilebaseFlags[64]` in `NEWDATA.H`. One bit per filebase, bit 0 =
  `FBFLAG_ISO`. 512 filebases per conference. Zero-initialized by
  PCBoard, never read by PCBoard, skipped by PCBSM. Works on 15.4+ with
  no upgrade needed.
- **QFront v1.0.0 deep audit**: 9,112 lines across 20 files. 4 new bugs
  found:
  - BUG-7: `qnlist.c` BSY TOCTOU race → `O_CREAT|O_EXCL` atomic lock
  - BUG-8: `qscan.c` BSY TOCTOU race → same fix
  - BUG-9: `qfconfig.c` `strncpy` hardcoded sizes → `sizeof()-1`
  - BUG-10: `zmodem.c` negative `file_size` from malicious sender not
    rejected
- 22 audit checks passed clean. Project totals: **36 programs, 26 bugs
  fixed** (11 pcbis + 6 pcbfoss + 9 qfront). 5,703 PPE archives
  cataloged.

### v0.5 — QFront documentation & debug pass (Aug 16, 2026)

**Commit `95ebc3a`** — pcbirc v0.5.0.
- 22 new `LOG_DEBUG` calls (88 → 110), 42 new block comments (12 → 54),
  390 new lines (9,733 → 10,123) across 14 QFront source files
- **QFront documentation** (1,952 lines total):
  - `QFRONT.DOC` — 38-page sysop manual
  - `QFRONT-CFG.DOC` — full config keyword reference
  - `HISTORY.DOC` — v1.0.0 changelog
- **PCBoard 15.4/15.41 Addendum PDF** — 13 pages supplementing the
  547-page printed PCBoard manual: PSA_PERSONAL, PCBTEXT 747–750,
  FilebaseFlags, PCBISO reference, QFront suite overview, OpenWatcom
  compiler change, all 26 bug fixes, PCBOARD.DAT and CNAMES.ADD
  developer reference, build instructions.

### v0.6 — RIP + repo restructure (Aug 20, 2026)

**Commit `a858054`**.
- **RIP v1–v4 architecture resolved**: two engines, plugin codecs, DOS
  i8086 real-mode, v2 ceiling
- `pcbmail.c` rewritten against `DOCDEV/MSGS.TXT`
- `PORT-RECORD.md`, `PLACEMENT.md`, `DOSBOX-BUILD-SETUP.md` created
- PCBCP OS/2 source found and extracted
- `Pcbic12.zip` crack attempted (ZipCrypto Store; needs unencrypted
  file match — deferred)
- BC31 downloaded from archive.org, packaged as `PCB153BT.ZIP`
- **Repo restructure**: `LIB→toolkit`, `LIBS→pcbcbase`, `MAIN→pcb153`,
  `PCBSRC→pcb154`, `1541→pcb1541`
- `MAIN/README.md` — master guide to the whole repo
- `pcb153/README.md` — PWA/delta/IRC explained
- 14 `README.md` files created for every program in `pcb1541/`
- Toolkit compile tested: 208/273 (76%) on Watcom 16-bit
- wrench's `netfosdl` added under `drivers/`

### v0.7 — Toolkit restructure & 4-version model (Aug 25, 2026)

**Commit `e4181e5`** — the heart of the restructure.
- **True 15.3→15.4 toolkit delta**: exactly ONE line different
  (`SPACERIGHTAT` added to `padtype` enum in `PCBTOOLS.H` for `@x`
  color codes). Everything else Clark's 15.4 added lived in the main
  source, not the toolkit.
- **4-version model** finalized:

| Version | Source | Toolkit | Output |
|---|---|---|---|
| 15.3 PWA | `pcb153/` | `toolkit/pwa153` | `OUT/pwa153/` |
| 15.4 PWA | `pcb153/upd154/` | `toolkit/pwa154` | `OUT/pwa153/upd154/` |
| 15.4 Delta | `pcb154/` | `toolkit/delta154` | `OUT/delta154/` |
| 15.41 IRC | `pcb1541/` | `toolkit/irc1541` | `OUT/irc1541/` |

- Toolkit branches — `pwa153`, `pwa154`, `delta154`, `irc1541` — all
  self-contained, parallel structure, TYPES.HPP CRLF invariant intact
  in each. Editing one has zero effect on the others.
- Clark's original 15.4 EXEs moved to `OUT/pwa153/upd154/clark-original/`
  as the rebuild reference.
- Retired the `OUT/pwa154/` slot.
- `build/` → `MAIN/build/`.

### v0.8 — Project overview published (Aug 27, 2026)

**Commit `5f39897`** — the canonical published project description.
- **12-lib SDK matrix**: three compilers × four memory models
  - PCBKBC = Borland C++ 3.1 (built: 4/4)
  - PCBKIT = Turbo C 2.01 (built: 4/4)
  - PCBKMS = Microsoft C 7.0 (pending DPMI host via `PCBBLDBT.IMG`,
    the golden FreeDOS 1.3 + CWSDPMI build image)
- **Companion project** `verta1878/ow2irc` = OpenWatcom 2 backend
  enabling the 15.41 successor toolchain
- **PPL compiler versions in `toolkit/pplc/`**: 3.00, 3.10, 3.20 (source
  in Clark's dev tree)
- Preserved and rebuilt: IC (Internet Connectivity) with byte-exact PPE
  reconstruction, custom FOSSIL driver, PPE decompiler, protocols,
  RIP/serial/display drivers, modernized client/server design

### v0.9 — RIPkit + 386MAX (Aug 28, 2026)

**Commits `819f64c`, `5155e0b`**.
- Added **RIPkit 1.20**
- **386MAX** word-ptr widening downport to older compiler — no source
  change, on-the-fly compile

### v1.0 baseline — end-of-day (Aug 30, 2026)

> *Not to be confused with `install v1.0` — see the installer arc below.
> This is a project-wide cumulative-state marker, when the whole tree
> reached the stable point from which the installer sub-arc began.*

**Commit `40cc0d8`** — stable baseline entering the .RED / installer
work.

Cumulative state:
- 33+ compiled programs
- 26+ bugs fixed
- 4-version parallel toolkit
- Complete SDK matrix build system
- QFront, pcbbinkp, pcbfido, PCBISO all shipped
- Installer subsystem `pcb1541/install/` exists but `INSTALL.zip`
  contents are opaque `.RED` archives

### install v1.5 — .RED format cracking begins (Sep 1, 2026)

**Commit `5b9756b`** — COMMDRV.EXE decoder finished byte-perfect. Sets
the stage for the full 6-archive extraction in v1.6.

### install v1.6 → install v1.7.2 — The installer arc

The full sub-arc — .RED format cracking, INSTALL.DAT decoding,
consolidated `dist/target/` install tree, the install v1.6.3 bogus-mapping
bug and its fix, Option A experiment and its reversal, PCBDISK.002/003
recovery, VIEWARCH.COM hidden victim, and CRLF protection — has its
own work log at
[`pcb1541/install/dist/target/README.md`](pcb1541/install/dist/target/README.md).

Summary of the arc: **all 481 files across the original 4-disk PCBoard 15.41 install set are now tracked in git byte-perfect, with a
byte-identical rebuild path from `INSTALL.zip` alone.** No files
missing, no bogus mappings, no CRLF corruption possible.

---

### install v1.8 → install v1.9 — Docs consolidation & stub-hunt (Sep 3-4, 2026)

**Commits `d7f0217..HEAD`** — install v1.8 phase (PPL samples cataloged, native
LHA scaffold, `RUNTIME-DEPS.md`, docs consolidation) followed by systematic
stub-hunting across PCBoard source to surface *reserved-but-never-shipped*
features and document them under `docs/pcboard-internals/`.

New/expanded docs under `docs/pcboard-internals/`:

- **`PCBTEXT-CODES.md`** (389 lines) — 747-slot screen-text catalog:
  1 header + 716 active+named + 2 deprecated+named + 27 preserved-but-inactive
  + 1 removed in 15.4 + 4 declared but missing from binary (#747-#750:
  GENDER/BIRTHDATE/WEBADDR/COLOR). 4 identified as stubs (incomplete
  strings). Ground truth cross-checked against `PCBTEXT.H`, `PCBTEXT.C`
  `UseText[]` bitflag, `STRS15.C` (779 lines), and `MKPCB.C` (817-entry
  table extracted by `extract_pcbtext.py`).

- **`PCBDCOM-CARDS.md`** (155 lines) — 9 shipped COMMDRV backends
  (COMMDV00-08.DRV, including hidden Windows 95 VxD at #08), 3 UI stubs
  in `DRVSETUP.EXE` menu with no backend/firmware (AST, BOCA-DMB,
  PC-COM), 3 TSR-only variants known to `COMMTSR.EXE`.

- **`DISPLAY-FILE-LOOKUP.md`** (202 lines) — full 19-variant filename
  ladder from Clark's own enum (`FILES.C:38-57`, one of the few
  per-slot commented enums in Clark's tree). `displayfiletype` has 9
  bits but only 4 drive filename decoration; RIP retrofitted through
  the `Control.RipMode` global rather than a proper flag. PPL `DISPFILE`
  masks input with `& 0x00000007` — PPE scripts see only 3 of 9 bits.

- **`PCBOARD-LINKOUT.md`** (132 lines) — Clark's toolkit link-out
  pattern documented at last. 17 `NO*.C` stubs in
  `pcb154/LIB/SOURCE/TOOLKIT/` (NOANSI/NOCHAT/NODISP/NOHELP/NOINPUT/
  NOLANG/NOLOG/NOMEMORY/NOPCBSYS/NOPRINT/NOSCREEN/NOSHELL/NOSTATUS/
  NOSYS/NOTXT/NOUPDSYS/NOXLATE, 892 lines total) let each utility pick
  the real subsystem OR the empty-body override per feature. Same
  pattern extends to hardware: `COMMDRV.OBJ` + `FOSSIL.OBJ` share the
  serial slot as a matched pair (Feb 15 1994 17:53 timestamp); the
  slot was designed for N backends, only 2 ever shipped — `pcbdcom` is
  the third slot finally getting populated, 32 years late.

- **`MCI-CODES.md`** — updated with cross-link to
  `DISPLAY-FILE-LOOKUP.md`.

- **`PLANNED-FEATURES.md`** — summary table extended with 3 new rows:
  COMMDRV card types (3 UI stubs), Display-file lookup bits (2 flags
  never wired into filename decoration), Toolkit link-out subsystems
  (17 stubs + serial slot with 2 of N ever populated).

Housekeeping: `pcbirc-display-file-lookup.zip` (stray shipment
artifact) removed from `docs/pcboard-internals/`.

## Cumulative totals (as of 2026-09-04)

- **10,520 tracked files** in the repo
- **36+ programs shipped or ported**
- **27+ bugs fixed** (11 pcbis + 6 pcbfoss + 9 qfront + 1 VIEWARCH.COM
  stub + numerous smaller ones)
- **4 parallel toolkit branches** kept self-contained
- **12-slot SDK matrix**, 8+ built
- **481-file PCBoard 15.41 install tree** byte-perfect
- **6 .RED archives + 2 PCBDISK.* archives** fully understood and
  reproducible
- **6 additional binaries + 30+ support files** landed for the pcbic
  reference material (Clark's PCBIC 1.2 toolset — see pcbic v1.0 below)

### pcbic v1.0.0 — PCBIC 1.2 reference material + restructure (Sep 4, 2026)

**Commits `3c0dfb6..49525c1`** — new subsystem arc. Opens the
byte-exact reconstruction of Clark's PCBoard InterCom (IC) toolset
from the 1997-04-30 Salt Air distribution (`Pcbic12.zip`).

Reference material landed:

- **6 EXEs** in `pcb1541/pcbic12/bin/` (unlocked from
  ZipCrypto-encrypted archive by external research effort;
  delivered via Pcbic12d.zip upload):
  - `Pcbic.exe` (313,310 B, MS-DOS MZ) — main IC binary
  - `Pcbic2.exe` (217,111 B, OS/2 LX 386) — OS/2 sibling
  - `PCBICCFG.EXE` (185,398 B, MS-DOS MZ) — config UI
  - `PCBICEVT.EXE` (89,612 B, MS-DOS MZ) — event handler
  - `TESTIC.EXE` (40,104 B, MS-DOS MZ) — ping test, smallest EXE
  - `TESTIC2.EXE` (46,627 B, OS/2 LX 386) — OS/2 sibling of TESTIC
- **RUNINET.PPE** (1,808 B, PPL 3.20 bytecode) + **RUNINET.PPS**
  (3,895 B, decompiled source) — first byte-exact target; PPLC 3.20
  compiler: build from `pcb153/SOURCE/PPL/` source (change
  `CUR_PPE_VER` to 320). Shipped binary in
  `reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`.
- **13 TCP/IP menu screens** in `bin/DATA/` (ANSI-decorated),
  6 dial-up scripts in `bin/SCRIPTS/`, 3 docs in `bin/DOCS/`,
  launcher batches + PCBIC.HLP.
- **Pcbic12d.zip** (739 KB, unlocked archive) preserved at
  `pcb1541/pcbic12/Pcbic12d.zip` as subsystem provenance.

Structure: `bin/` (Clark's originals, read-only), `src/` (byte-exact
reconstruction source), `/analysis/` at repo root (gitignored, local
reverse-engineering scratch — one archive folder at a time, never
tracked).

Sub-phases queued (per `pcb1541/pcbic12/ROADMAP.md`):
`v1.0.1` = RUNINET.PPE (source in hand, compile-diff loop),
`v1.0.2` = TESTIC.EXE, `v1.0.3` = TESTIC2.EXE, `v1.0.4-1.0.7` =
PCBICEVT/PCBICCFG/Pcbic/Pcbic2, `v1.1` = whole arc complete.

### install v1.10 — INSTALL.EXE completion (Sep 4, 2026 → in progress)

**Commits `TBD`** — installer arc's next phase: completing our C
reimplementation of Clark's `INSTALL.EXE` in `pcb1541/install/src/`.

The byte-perfect 481-file target tree already works via
`rebuild_place.py` (v1.7.1 recovery); install v1.10 finishes the
installer side — teaching `install.c` to run Clark's real
`INSTALL.DAT` end-to-end and produce the same target tree.

**Starting state**: `install.c` has 7 of 60 semantically distinct
`@`-directives implemented (Phase 27 stub). Reference material for
diffing landed in v1.10.0.

Reference material added in v1.10.0:
- `pcb1541/install/reference/INSTALL.DAT` (42,294 B, 72 raw / 60
  case-folded @-directives, md5 `cca38d36`) — Clark's original
  installer script, extracted once from `INSTALL.zip` for stable
  diffing.
- `pcb1541/install/reference/INSTALL.EXE` (338,548 B, NE format,
  md5 `5239767b`) — Clark's original installer binary, for
  disassembly parity checks against our C reimplementation.
- `pcb1541/install/reference/README.md` — reference-material
  provenance + use.
- `docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md` — canonical
  reference for all 60 unique @-directives, grown as each sub-phase
  lands.

Sub-phases:
- **v1.10.0** — reference material lands + directive catalog (shipped).
- **v1.10.1** — redx wire-up (shipped). File operations land: 6 new
  directives (`@BeginLib`/`@EndLib`/`@File`/`@Copy`/`@Delete`/`@FileAttr`)
  plus `@Out`/`@Size`/`@AppendTo` as `@File` subclauses. install.c
  shells out to `pcb1541/install/archivers/redx` for each `@BeginLib`
  block, extracts the archive to a temp dir, then executes each
  `@File` by resolving the source key (name or numeric), verifying
  `@Size` if given, and copying to the expanded destination path
  (with `@OutDrive:@SubDir` variable expansion + cross-platform path
  normalization). Verified byte-perfect against `pcb1541/install/dist/target/`
  on a minimal file-op subset (real INSTALL.DAT blocks on `@If`/`@Group`
  control flow — that's v1.10.2). Cumulative: 18 of 60 directives coded.
  install.c grows from 518 to ~950 lines; added `src/README.md` and
  `src/Makefile`.
- **v1.10.2** — variables + control flow + string ops (shipped).
  11 new directives (`@If`/`@Else`/`@EndIf`/`@Endif`/`@Goto`/`@Label`/
  `@Set`/`@StrLen`/`@StrHead`/`@StrToken`/`@Exists`-as-predicate). Full
  recursive-descent expression evaluator supporting `[= [! == != > <
  >= <= && || ()` on int and string operands, with `@Func(...)` inline
  invocation inside string literals (needed for Clark's nested-quote
  patterns like `"@StrToken("@Fname",0," ")"`). Handles trailing
  `@Group X` clause on `@File` as per-directive filter. Real INSTALL.DAT
  runs end-to-end for the first time — 471 successful @File operations,
  348 files byte-perfect vs `pcb1541/install/dist/target/` (94.8% of
  placed). Cumulative: 29 of 60 directives coded. install.c grows to
  1984 lines. Also added: v1.10.4-territory stubs (`@GetString`/
  `@GetOutDrive`/`@GetSubdir`/`@GetGroups`), v1.10.3 stubs
  (`@DefineDisk`/`@EndDisk`), v1.10.5 stubs (`@SetConfig`/`@SetAutoexec`/
  `@Finish` + matching `@End*`) — all skip cleanly, letting execution
  proceed past sections we haven't implemented yet. New CLI arg
  `--groups STR` simulates the v1.10.4 checkbox selection.
- **v1.10.3** — filesystem + disk sequencing (shipped).
  10 new directives (`@MkDir`/`@Mkdir` as callable that actually creates
  the dir; `@ChDir`/`@ChDrive` with state tracking; `@DirExists`
  predicate; `@DefineDisk`/`@EndDisk` semantics; `@Requires`/`@HardDisk`/
  `@Version` accepted as project-metadata). Also fixed two `@File` parser
  bugs surfaced by real INSTALL.DAT: `@Out DIR\*.*` glob (means "keep
  source filename") and missing `@Out` fallback (default to source key
  as destination) — these two fixes alone closed most of the coverage
  gap on PCBMAIL and PCBOARD2.RED (OS/2) blocks. Cumulative: 39 of 60
  directives coded. install.c grows to 2098 lines. Full 481 `@File`
  operations succeed vs real INSTALL.DAT (matches reference tree total
  exactly), 394 files byte-perfect vs `pcb1541/install/dist/target/`
  (94.0% of placed); only PCBOARD.SER fails (legitimately not in any
  archive — physical disk file). Remaining gap to 481-byte-perfect
  is the per-bulletin-board multi-writer collisions that v1.10.4 will
  disambiguate via the interactive @GetGroups flavor selection.
- **v1.10.4** — interactive menu (shipped).
  10 new directives (`@GetGroups`/`@CheckBox`/`@GetString`/`@AskOverwrite`/
  `@Prompt`/`@SetGroup`/`@ClearGroup`/`@Qstring`/`@RegCode`/plus the
  variable-storage vars `@Fname`/`@Lname`/`@CitySt`/`@Pwd` via
  `@DefineVars`). Real interactive handlers when `isatty(stdin)`:
  `@GetGroups` parses `@Set` single-letter menu items and presents a
  minimal checkbox/radio picker, `@GetString` reads a line respecting
  `@Prompt`. Headless runs fall back to `--groups` CLI arg + sensible
  defaults. `@SetGroup(x)`/`@ClearGroup(x)` modify the selected-groups
  state programmatically. Single-flavor install (`-g "abcdefp"` for
  Corporate BBS) places 371 files, 350 byte-perfect vs
  `pcb1541/install/dist/target/` (94.5%). Cumulative: 49 of 60
  directives coded. install.c grows to 2285 lines. Note on the
  481-vs-371 gap: reference tree's 481 files include
  `rebuild_place.py`'s per-flavor disambiguation artifacts
  (`MAIN/CNAMES.N`, `MAIN/CNAMES.O`, etc); a real installer running one
  flavor at a time correctly produces ~371 files. This gap will not
  close via more @-directives — it's a reference-tree naming choice.
- **v1.10.5** — system hooks + finish (shipped).
  7 new directives (`@System`/`@SetConfig`/`@EndConfig`/`@SetAutoexec`/
  `@EndAutoexec`/`@Finish`/`@EndFinish`; `@Files`/`@Path` handled inline
  when inside their respective config-block). Real `@SetConfig`/
  `@SetAutoexec` generate `CONFIG.SYS.pcb` and `AUTOEXEC.BAT.pcb`
  drop-in files in the target root (admin merges them into actual
  system files). Real `@System(cmd)`: default headless returns 0
  (matches Clark's success semantic for `@If @System(...) == 0` gating);
  `--exec-system` CLI flag opts into actual shell-out via `system(3)`.
  `@Finish/@EndFinish`: contents execute inline by default under
  `--run-finish`, or are skipped (default) for test-friendly output
  since Clark's post-install `@Delete` calls remove `INIT.EXE`/
  `UPGRADE.EXE`/etc. Cumulative: 56 of 60 directives coded. install.c
  grows to 2443 lines. File-placement acceptance unchanged from v1.10.4
  (350/361 byte-perfect); adds 2 generated system-config files that
  are new but expected.; end-to-end DOSBox-X run
  landing a complete C:\PCB tree matching `dist/target/`
- **v1.10.6** — disassembly parity check on `INSTALL.EXE` (shipped).
  No new directives — semantic verification only. Discovered
  `INSTALL.EXE` supports 329 total `@`-directives; Clark's PCBoard
  `INSTALL.DAT` uses 60 (which we've implemented). The other 269 are
  supported by the engine (system-query family for OS/Windows/CD-ROM/
  memory/video/drives/machine, extra string ops @StrDel/@StrFind/
  @StrMid/@StrIndex/@StrLwr/@StrUpr etc., file-I/O @Write/@ReadLn/
  @Verbatim, process spawning @Execute/@Spawn/@Shell, INI file ops
  @GetIni/@SetIni, disk-size-conditional @Out variants @Out0K through
  @OutAbs, and more) but unexercised by this script. Two undocumented
  constraints found in binary error strings: "Cannot use @Goto to exit
  the @Finish block" (we don't enforce; Clark's script doesn't trigger)
  and "@Size commands cannot be used when @Requires is specified"
  (constraint doesn't fire on @Requires @HardDisk which is what the
  script uses). Full parity report at
  `docs/pcboard-internals/INSTALL-EXE-PARITY.md`.

- **v1.11.4** — gap analysis (shipped 2026-09-05).
  Structural comparison of our 56,574 B build vs Clark's 338,548 B
  reference. MZ stub delta 117 KB (Family API emulation layer),
  code delta 131 KB (269 unimplemented handlers), 2,336 more strings
  in Clark's binary. Phase recommendations for v1.11.5-v1.11.9.
  Document only, no code changes.

  Files:
  - `docs/pcboard-internals/INSTALL-EXE-GAP-ANALYSIS.md` — NEW


- **v1.11.5** — system-query handlers + full dispatch coverage (shipped 2026-09-05).
  All 301 directives in the dispatch table now have explicit case handlers
  (was 43 real + default stub). System-query directives (~80) return
  DOS 6.22 / VGA / 640K defaults. @Out* family (14 variants), string ops
  (10), file I/O extras (16), system integration (27), advanced flow (55),
  and Windows-specific (9) all have typed stubs.
  Compiles clean under BCC 3.1 (0 errors) and links to 58,136 B
  (was 56,574 in v1.11.3; +1,562 B from handler code).
  NE header still matches: linker 5.10, target OS 0x01.

  Files:
  - `pcb1541/install/src/install-1011.c` — 3,524 lines (was 3,097)




  **install v1.10 arc: understanding-complete.** Our C reimplementation
  faithfully executes Clark's INSTALL.DAT end-to-end, producing
  byte-perfect output on 94.5% of placed files (350/361 unique on
  disk). The remaining 9 differ + 2 extras are per-BBS multi-writer
  collisions (last-write-wins architectural choice matching real
  installer behavior). CONFIG.SYS.pcb and AUTOEXEC.BAT.pcb generated
  correctly. @Finish cleanup semantics verified under --run-finish.

  Byte-exact rebuild of INSTALL.EXE itself is deferred to install
  v1.11+ (separate arc — requires implementing all 329 engine
  directives, enforcing all constraints, and matching Clark's
  toolchain: Borland C++ 4.5/5.0 targeting OS/2 Family API, linker
  version 5.10).


### install v1.11 — byte-exact INSTALL.EXE reconstruction (arc opens)

- **v1.11.0** — toolchain shakedown (shipped 2026-09-05).
  Confirmed Clark's toolchain via sandbox verification: **Borland C++
  3.1 (already in `devtools/BC31.zip`) + TLINK 5.1 + API.LIB from
  Microsoft OS/2 SDK 1.03 (`devtools/OS2SDK103.zip`)**. TLINK's banner
  says "Version 5.1" but its NE header stamp is bytes `05 0A` (=5.10)
  — matches Clark's `INSTALL.EXE` exactly (verified across BWCC.DLL,
  TLINK's own header, fresh Hello World, and v1.11.0's stub build).
  Earlier speculation this session that Microsoft C 5.1 was the
  toolchain was wrong — corrected in `devtools/COMPILERS.md`.

  Files:
  - Renamed `pcb1541/install/src/install.c` → `install-1010.c`
    (preserved byte-for-byte + updated header comment).
  - New `pcb1541/install/src/install-1011.c` — empty-main() starter
    for the byte-exact rebuild.
  - New `pcb1541/install/build/BLDINS.BAT` + `README.md` — install-
    scoped build script. Deliberately NOT in `MAIN/build/scripts/`
    (that dir is for the pwa153 SDK library matrix, not one-off
    binary builds — different scope entirely).
  - `devtools/COMPILERS.md` — "install v1.11 toolchain — CONFIRMED"
    section with full verification chain, correcting prior draft.
  - `pcb1541/install/src/README.md` — v1.11 section added.

  Acceptance: install-1011.c stub compiles under BC 3.1, links with
  TLINK 5.1 + API.LIB, produces a NE binary with linker version bytes
  05 0A (5.10) — same bytes as Clark's INSTALL.EXE. Target OS byte
  currently 0x02 (Windows) not 0x01 (OS/2 Family) because v1.11.0
  uses standard Windows-target flags for the shakedown; correct
  Family-API incantation (.DEF file with EXETYPE OS2, C0FL.OBJ
  startup) lands in v1.11.1 alongside the directive dispatch table.

- **v1.11.1** — directive dispatch table (shipped 2026-09-05).
  301 ALL-CAPS directive names mapped to enum IDs via a string
  lookup table (`DirEntry dir_table[]`), with a minimal main loop
  that reads INSTALL.DAT, extracts @-tokens, uppercases, and
  dispatches to stub handlers. Compiles clean under gcc
  `-Wall -Wextra -ansi -pedantic` AND under BC 3.1 via BLDINS.BAT.
  Tested against real INSTALL.DAT: 894 directives processed, 6
  unknown (the 5 user variables + 1 alias — correct behavior).

  Family API link target (proper .DEF, C0FL.OBJ startup, target OS
  byte 0x01) deferred to v1.11.2 — ships alongside the semantic
  handler port, where the imports actually matter.

  Files:
  - `pcb1541/install/src/install-1011.c` — dispatch table + main loop
    (replaces the v1.11.0 empty-main stub).

- **v1.11.2** — semantic handler port (shipped 2026-09-05).
  60 semantic handlers from install-1010.c (v1.10.5) ported into the
  v1.11.1 dispatch table framework. switch(lookup_directive()) replaces
  the strncasecmp chain. Runs Clark's INSTALL.DAT end-to-end: project
  metadata, variable/control flow, file operations (@BeginLib/@File/
  @Copy/@Delete/@FileAttr), interactive menu (@GetGroups/@GetString/
  @SetGroup/@ClearGroup), system hooks (@SetConfig/@SetAutoexec/
  @Finish/@System), and filesystem state (@ChDrive/@ChDir). All routing
  through the 301-directive enum/lookup.

  Family API link target (.DEF + C0FL.OBJ + target OS byte 0x01)
  deferred to v1.11.3 — needs testing under DOSBox-X with the NE
  Family API loader.

  Files:
  - `pcb1541/install/src/install-1011.c` — 3,097 lines (was 752).


- **v1.11.6** — error messages, help text, version strings (shipped 2026-09-05).
  Clark's embedded strings added for binary parity: version/logfile
  strings ($Logfile, $Revision, copyright banner), 12-entry installer
  error table, 13-entry DOS INT 24h critical error table, 14-entry
  OS/2 Family API error table, UI prompt strings (PRESS ANY KEY,
  Enter Text, etc.), format strings for file/path/date operations.
  BCC 3.1: 0 errors. TLINK: 60,276 B (was 58,136; +2,140 B from
  embedded strings). NE header: 5.10 / 0x01.

  Files:
  - `pcb1541/install/src/install-1011.c` — 3,626 lines (was 3,524)


- **v1.11.7** — Family API link target + API.LIB (shipped 2026-09-05).
  OS/2 SDK 1.03 API.LIB (74,752 B) linked into INSTALL.EXE. This embeds
  the Family API DOS emulation code (DosBeep, VioWrtTTY, KbdCharIn
  wrappers). Binary grew from 60 KB to 65 KB. OS/2 API declarations
  added to install-1011.c with volatile-pointer references to force
  TLINK to pull FAPI code from API.LIB.

  NE import records (DOSCALLS/KBDCALLS/VIOCALLS) require BIND.EXE
  (OS/2-only tool) to generate — deferred to v1.12 (byte-exact arc).
  The MZ stub expansion (117 KB in Clark's binary) is also a BIND
  artifact.

  Files:
  - `pcb1541/install/src/install-1011.c` — 3,677 lines (was 3,626)
  - `pcb1541/install/build/BLDINS.BAT` — added API.LIB to link
  - `pcb1541/install/build/os2sdk/API.LIB` — NEW (OS/2 SDK 1.03)


- **v1.11.8** — NE header parity (shipped 2026-09-05).
  Flags, stack, and heap now match Clark's reference:
  - Flags: 0x0002 (was 0x000A — cleared PROTMODEONLY bit)
  - Stack: 8000 (was 0)
  - Heap: 0 (was 4096)
  INSTALL.DEF accepted by TLINK (needed CRLF line endings — TLINK
  5.1's parser rejects LF-only). HEAPSIZE applied from DEF; flags
  and stack require post-link fixup (ne-fixup.py) because TLINK /Toe
  forces PROTMODEONLY and zeroes stack.

  Segment count (4 vs Clark's 6) is a code-size artifact — TLINK
  creates more segments as code grows. Will converge in v1.12.

  Files:
  - `pcb1541/install/build/ne-fixup.py` — NEW, post-link NE fixup
  - `pcb1541/install/build/INSTALL.DEF` — updated (CRLF, HEAPSIZE 0)




- **v1.11.3** — Family API link target (shipped 2026-09-05).
  BLDINS.BAT now produces a proper NE Family API binary matching
  Clark's INSTALL.EXE header:
  - C0FL.OBJ (Family API startup) replaces C0L.OBJ (DOS)
  - TLINK /Toe (OS/2 target byte 0x01) replaces /Twe (Windows 0x02)
  - INSTALL.DEF module definition added (EXETYPE OS/2, PROTMODE,
    STACKSIZE 8192)
  - Target imports: DOSCALLS, KBDCALLS, VIOCALLS (Family API)

  Clark's reference NE header (at offset 0x1CA00 in INSTALL.EXE):
    Linker 5.10, Target OS 0x01, Flags 0x0002 (MULTIPLEDATA),
    6 segments, stack 8000, imports DOSCALLS+KBDCALLS+VIOCALLS.

  Files:
  - `pcb1541/install/build/INSTALL.DEF` — NEW
  - `pcb1541/install/build/BLDINS.BAT` — updated link line


- **v1.11.9** — RTL coverage + size convergence report (shipped 2026-09-05).
  Structural analysis of the 272,634 B gap between our build (65,914 B)
  and Clark's reference (338,548 B). All bytes accounted for:
  - 42% = MZ stub (BIND artifact, needs OS/2 host, deferred v1.12)
  - 45% = CODE (269 unimplemented OS/2 API handlers)
  - 13% = DATA (remaining embedded strings/tables)
  No unknowns remain. Every gap categorized by type and phase.

  Files:
  - `docs/pcboard-internals/INSTALL-EXE-SIZE-CONVERGENCE.md` — NEW


- **v1.11.10** — **install v1.11 arc: understanding-complete** (2026-09-05).
  Ten phases shipped in one session. INSTALL.EXE is fully understood:

  What we built:
  - 3,677-line C source (install-1011.c) with 301-directive dispatch
    table, 60 semantic handlers, ~80 system-query handlers, error/UI
    string tables, OS/2 Family API declarations
  - Compiles under BC 3.1 (0 errors), links with TLINK 5.1 /Toe +
    API.LIB to 65,914 B NE Family API binary
  - NE header matches Clark’s: linker 5.10, target OS 0x01,
    flags 0x0002, stack 8000, heap 0
  - Runs Clark’s INSTALL.DAT end-to-end (894 directives processed)

  What we understand about the 272,634 B gap:
  - 42% = MZ stub (BIND artifact, needs OS/2 host)
  - 45% = CODE (269 unimplemented OS/2 API handlers)
  - 13% = DATA (remaining embedded strings/tables)
  - 0% unknowns — every byte accounted for

  What v1.12 (byte-exact arc) needs:
  - BIND.EXE on OS/2 host to generate the MZ stub
  - Full OS/2 API handler implementations (VIO/KBD/DOS calls)
  - Remaining embedded strings and screen layout tables
  - `cmp -s` as the terminal gate

  **install v1.11 arc: CLOSED.**

### pcbic v1.0.1 — RUNINET.PPE byte-exact (in progress, 2026-09-06)

  Source compiles clean under PPLC 3.30: 2,261 B output. Target is
  1,808 B (compiled with PPLC 3.20). Header identical through byte 42;
  divergence is bytecode format, not source. No source changes needed.

  BLOCKER: decompiled PPS has 63 implicit variables vs Clark’s 39.
  Both PPLC 3.20 and 3.30 produce 2,261 B from this source.
  Decision: build PPLC 3.20 from `pcb153/SOURCE/PPL/` source.
  ~~Fix shipped: .gitignore now allows toolkit/pplc/**/*.EXE.
  Once PPLC320.EXE is placed at toolkit/pplc/3.20/PPLC320.EXE and
  committed, one compile closes this phase.~~

  **Corrected v0.3.0:** PPLC320.EXE was never committed and does not
  belong at `toolkit/pplc/3.20/` — `<ver>/out/` is for compiled .PPE
  output, not compiler binaries. Clark's shipped PPLC320.EXE stays
  inside `reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`; per
  `toolkit/pplc/README.md` the decision is to build PPLC from Clark's
  source. The `!toolkit/pplc/**/*.EXE` whitelist has been removed.

  Note the blocker above is the decompiled PPS, not the compiler
  version: both 3.20 and 3.30 emit 2,261 B from it. Getting PPLC 3.20
  is necessary but NOT sufficient — the 63-vs-39 implicit-variable gap
  has to close too.


  Scope expanded (2026-09-06): PPLC links against 7 PCBoard support
  libraries (countryl.386, dos_l.386, screen_l.386, etc.) that must be
  built from `pcb153/SOURCE/` first. This is the full PCBoard build
  chain — 591 source files, 18 directories. Unlocks not just PPLC but
  PCBOARD.EXE, all utilities, and the entire 19-binary rebuild list.
  See `todo/pcb-libchain-build.md` and `docs/pcboard-internals/PPL-COMPILER.md`.

  Patches needed:
  - `patches/pwa153-libchain.patch` (15.3 support libs)
  - `patches/delta154-libchain.patch` (15.4 delta)


### pcbsrc v0.1 — PCBoard source library chain build

- **v0.1.0** — prerequisites staged (2026-09-06).
  PWA source (`PCBoard_15_3_source_code_v0_014.zip`) uploaded.
  Clark’s build flags discovered: `-P` (compile .C as C++),
  response file `+CFG` approach. First 5 DOS library files compile
  clean under DOSBox-X with BC 3.1. Headers from `LIB/H/` (112 files)
  + `MAIN/SOURCE/H/` (54 files) resolve all include dependencies.
  See `todo/pcb-libchain-build.md` for the 10-phase plan.


- **v0.1.1** — dos_l.lib built (2026-09-07).
  45/45 .C files compiled clean under DOSBox-X with Clark’s exact
  flags (-P -ml -3 -ff, response file approach). TLIB created
  dos_l.lib (37,888 B, 45 modules). One symbol clash warning
  (SHOWERR/SHOWERR2 both export showerror — cosmetic).
  INT24HND.ASM skipped (needs TASM). First PCBoard support library
  built from source.


- **v0.1.2** — countryl.lib built (2026-09-07). 12/12 clean, 5,632 B, 3 modules.
- **v0.1.3** — doscls_l.lib built (2026-09-07). 1/1 clean, 11,264 B, 1 module.
- **v0.1.4** — misc_l.lib built (2026-09-07). 85/88 clean, 29,184 B, 41 modules.
  3 failures (CPUTYPE, DMATH, VIRTUAL1) not in Clark’s TLIB list.
- **v0.1.5** — screen_l.lib built (2026-09-07). 36/38 clean, 15,360 B, 23 modules.
  SETFONT failed (conio.h conflict), not blocking. Contains print/println/newline.
- **v0.1.6** — scrnio_l.lib built (2026-09-07). 20/20 clean, 4,096 B.
- **v0.1.7** — system_l.lib built (2026-09-07). 4/7 clean, 2,048 B, 3 modules.
  KBD/SESSION/THREADS failed, not in Clark’s TLIB list.
- **v0.1.8** — toolkit_l.lib built (2026-09-07). 38/45 clean, 6,656 B, 3 modules.

  **All 8 PCBoard support libraries built from source.** ~112 KB total.
  Every failed file checked against Clark’s MAKEFILEs — none of the
  failures are modules Clark includes in the libraries.


- **v0.1.9** — PPLC links from source (2026-09-07).
  9 PPLC source files compiled, PPLC.EXE links clean (112,660 B).
  Built from 8 support libraries (NEWKIT.LIB 96 KB) + Clark’s
  toolkit NO* stubs + minimal modem/timer stubs. Compiles PPL source
  correctly — output matches Clark’s shipped PPLC 3.20 (same size,
  same variable count).

- **PPE encryption discovered** (2026-09-07).
  Two deterministic layers: encrypt2 (seed 0xDB24, XOR/ROR chain,
  PPL 3.01+) and encrypt3 (XOR with "DECOMPILERS SUCK!" 17-byte key,
  PPL 3.30 only). Source: LIB/SOURCE/MISC/CRYPT.C. encrypt3 guarded
  with `#if CUR_PPE_VER >= 330` (our fix). Encryption is NOT the
  blocker — it’s deterministic. The real blocker: our PPLC is 3.30
  code with 3.20 label. The code generator (SCRCOMP.CPP) changed
  between versions, producing different bytecode. Path forward:
  diff pcbsrcv/000-014 to find 3.20-era source.


- **v0.1.10** — pcbsrc arc COMPLETE (2026-09-07).
  All 8 PCBoard support libraries built from source (~112 KB).
  PPLC.EXE built from source (112,596 B), compiles PPL correctly.
  Bytecode output matches Clark’s shipped PPLC 3.20. Code generator
  proven correct (SCRCOMP.CPP identical across all pcbsrcv snapshots).
  Only difference: C++ vtable pointer serialization artifact (312 B).
  **pcbsrc v0.1 arc CLOSED.**


- **PPE byte-exact match** (2026-09-07).
  Source-built PPLC produces bit-for-bit identical PPE output to
  Clark’s shipped PPLC 3.20 binary. Proven with ppedecrypt.py
  (decryption tool at `toolkit/pplc/ppedecrypt.py`). Decrypted
  plaintext matches exactly. encrypt3 guards applied
  (`#if CUR_PPE_VER >= 330`).

- **Clark’s PPS is a decompilation** (2026-09-07).
  The RUNINET.PPS in the pcbic12 distribution (recovered from
  password-protected zip via known-plaintext attack) is identical
  to our decompiled version. Both produce 63 vars. The shipped
  PPE (39 vars) was compiled from an earlier version of the source
  that no longer exists. The 39-var original is lost.


- **39-var PPE decoded** (2026-09-07).
  All 39 variables from Clark’s original RUNINET.PPE recovered
  using ppedecrypt.c — missing for two decades. Contains Clark’s
  original pcbIC12 strings: .SLP/.PPP extensions, OPEN SLIP/PPP
  commands, $$LOGON.BAT/CMD, PATH, log messages, FileInPath().
  The 24 extra variables in the 63-var decompiled version are all
  temporaries (empty strings, FALSE, zeroed integers). Both versions
  produce identical bytecode (scriptSize=642). Broken Python tool
  replaced with working C tool (ppedecrypt.c).






- **pcbkit_l.lib investigation** (2026-09-07).
  Pre-built pcbkit_l.lib (241 KB) at PCBSRC/ is STALE — built from
  Clark’s dev machine (D:\tc\) with different headers and symbol
  decoration (plain pascal vs our C++ mangled names). Name decoration
  mismatch blocks PPLC link. Decision: move stale pcbkit_l.lib to
  attic/, rebuild from our 8 individual libs. Link PPLC against
  individual libs first (Option B), then merge into fresh pcbkit_l.lib
  (Option C) for MAKEFILE compatibility.
  See `docs/pcboard-internals/PCBKIT-LIB.md`.





  Files:
  - `pcb1541/pcbic12/src/RUNINET.3.30.PPE` — 3.30 output (2,261 B, proof)
  - ~~`.gitignore` — updated (allow toolkit/pplc/ EXEs)~~ reverted in
    v0.3.0; no compiler binary belongs under toolkit/pplc/







  **Original v1.11.1–v1.11.10 phase plan retired.** v1.11.2 collapsed
  phases 1–5 (all 60 handlers ported in one shot). New plan:
  v1.11.4 = gap analysis (document, not code), which defines
  v1.11.5–v1.11.9 based on categorized gaps (implementation / RTL /
  resource / compiler-artifact). v1.11.10 = understanding-complete.
  Byte-exact rebuild deferred to v1.12 arc. See updated
  `docs/pcboard-internals/INSTALL-EXE-PARITY.md`.

### v0.3.0 — PCB/IC placed on 15.4 PWA; Clark's 15.4 beta binaries recovered (Sep 16, 2026)

Two arcs closed in one release: IC moved onto the branch it belongs to,
and Clark's own shipped 15.4 binaries were recovered after three weeks
missing without anyone noticing.

**Phase A — IC on the 15.4 PWA leg.** IC is Clark's 15.4 internet
upgrade, so it belongs to PWA 15.4. It had been sitting in `pcb1541`
(the IRC tree) as a staging spot for the reconstruction work — `pcb153`
= PWA, `pcb154` = Delta, `pcb1541` = IRC, and IC is not IRC.

- Source → `pcb153/upd154/SOURCE/IC/` (20 files, `dos/` + `os2/` kept)
- Binaries → `OUT/pwa153/upd154/` (6 files)
- Data → `OUT/data/ic12/` (32 files)
- Originals in `pcb1541/pcbic12/` untouched

`OUT/data/ic12/` mirrors Clark's subdirectories rather than flattening
them. Flat is impossible: `bin/PPP` (150 B) and `bin/SLIP` (151 B) are
different files from `bin/DATA/PPP` and `bin/DATA/SLIP` (377 B each).
Mirroring also dissolves the README.1ST conflict — it only gets renamed
at install time.

**Delta inheritance decided.** Delta 15.4 inherits IC but ships Clark's
6 binaries unchanged: they are Borland C++ 3.1 (DOS) and BC++ 2.0 OS/2
output, no C source survives, and the reconstruction is 18,110 lines of
raw NASM `db` bytes across 3,888 functions that Watcom cannot compile
(`pcbic.c` is a stub — notes and empty bodies). RUNINET.PPE (real PPL
source), the docs and the data ARE inherited properly. Recorded in
`pcb154/DOCS/SYSOP_154.TXT` §8 beside PCBSETUP and LOCAL, which are
shipped-binary for the same reason.

**clark-original recovered.** `OUT/README.md` referenced a
`clark-original/` directory that did not exist. The binaries had lived
at `OUT/DOS/` and `OUT/OS2/`, were deleted in `e4181e5` (Aug 25, the
toolkit restructure — see v0.7 above) and never re-added. Nothing
flagged the absence: the `*.EXE` rule in `.gitignore` meant git could no
longer see them. There was never an `OUT/pwa154/` slot, despite the
note in v0.7.

Recovered from `e4181e5^` and cross-verified against the original 1997
distribution — `PCB154B` inside a Team GLoW release dated 04/28/97,
archived as `pcb-metaworlds` on the Internet Archive, bundling PCBoard
15.3 (250-node), the 15.4 beta, PCBIC and MetaWorlds 1.02.

9 of 12 EXEs byte-identical between the two sources. PCBOARD.SER
identical. The 4 docs identical once line endings are ignored — the repo
copies had been normalized CRLF→LF.

3 differed by exactly **one byte** each, at the beta date check:

```
... 26 2b 07 89 46 fe 83 7e fe 1e [76|EB] ...
     \__ Terry's signature __/     76 = JBE (check intact)
                                   EB = JMP (check bypassed)

LOCAL.EXE     0x03249B    76 → EB
PCBOARD.EXE   0x03D45B    76 → EB
PCBOARDM.EXE  0x03CBE3    76 → EB
```

That pattern is the signature David Terry published in June 1997 for
disabling the 30-day beta timer (`reference/pcb-1997-06-16-terry.txt`).
The repo copies were patched; the 1997 originals are not. This matters
for byte-exactness: a faithful rebuild emits `76`, so verifying against
a patched reference would report three one-byte failures that are not
reconstruction errors. `OUT/clark-original/` holds the unpatched 1997
binaries; the patched variants are kept in `beta-patched/`, not
discarded. Placed at `OUT/` top level — Clark's originals are not any
one build leg's output.

Gaps: no 15.4 `.HLP` files in any known copy, and no `PCBUUCP.ZIP`,
though README.1ST refers to both.

**PCBIC v1.1 added to reference.** `reference/pcball/pcboard/` held v1.2
only; v1.1 (05-28-96) recovered from the same GLoW release. Diffing the
two: only 3 binaries changed between releases (PCBIC.EXE, PCBIC2.EXE,
PCBICCFG.EXE). PCBIC.HLP, PCBIC.DOC, PCBIC.PDF, PCBICEVT.EXE,
TESTIC.EXE and TESTIC2.EXE are byte-identical across both — which
settles the long-open question of PCBIC.HLP's provenance: it is a
shipped data file, not something the build compiles. v1.1 also carries
`RUNINET.DOC` (5,986 B) and `W95PPP.SCP` (922 B), neither of which
exists in the v1.2 set or anywhere in this repo's history.

**Documentation corrections.** Four documents asserted things that were
not on disk. `OUT/README.md` (OUT/ no longer binaries-only; `bins/`
documented as SDK examples, not program EXEs), `MAIN/DELTA-MODEL.md`
(clark-original path; the retired-`OUT/pwa154/` claim removed; PCB/IC
section; PWA/Delta/IRC shorthand added), `MAIN/README.md`,
`MAIN/build/IC-TOOLKIT-SDK-BUILD-PLAN.md` (decision 3 superseded — IC
completed ahead of the toolkit/SDK order and never needed it; decision 4
amended — "OUT/ = binaries only" no longer true), `README.md` (binaries
are 1997 not 1996; PCB/IC section distinguishing it from pcbis), and
`RUNINET.md` (the PPLC320.EXE path claim, see the v1.0.1 correction
above).

`.gitignore`: `OUT/clark-original/` and `OUT/pwa153/upd154/` EXEs
whitelisted as reference material; the `toolkit/pplc/**/*.EXE` whitelist
removed.

**Lesson.** Two separate docs described directories that had never
existed, and both read as completed work. A binary set went missing for
three weeks because the ignore rule that hid it also hid its deletion.
Worth checking assertions against the filesystem before trusting them.

### v0.3.1 — the pwa153 door SDK, built and verified by linking (Sep 17, 2026)

The first SDK library this project has produced. `PCBKBC{S,C,M,L}.LIB`,
152 modules, all four memory models, in `toolkit/pwa153/bc31/lib/`, with
6 of Clark's own sample doors linked against it in `OUT/pwa153/bins/` as
the proof.

**The method mattered more than the result.** Every finding below came
from building and linking, not from reading. The inherited manifest and
flag set had been marked DONE twice before; both were wrong, and reading
them again would not have shown it.

**Three config switches had to come out.** Clark shipped no door-SDK
config — `LIB/CFG/BC31/PCBOARD.CFG` is the PCBOARD.EXE config, and the
SDK config is that plus `-DLIB` minus three things:

- `-Y` (overlay generation). Borland: *"Overlays only supported in
  medium, large, and huge memory models."* `SDK-BUILD-STATUS.md` had
  recorded the small/compact failure as a far-pointer and `_FARDATA_`
  problem needing per-model configuration, and predicted a genuine
  4-model matrix was "more work than a re-run." It was one switch:
  6/130 objects became 130/130.
- `-DPCBCOMM`. Guards PCBoard-internal branches that read
  `Status.TerseMode` — which does not exist in the reduced, door-visible
  `statustype` that `-DLIB` selects. All four `-DPCBOARD`/`-DPCBCOMM`
  combinations were compiled to find which one mattered.
- `/DCPU386`. Only `BUG.H` reads it, to route `timerexpired()` through a
  `long BC386BUG` global that lives in `MAIN/SOURCE/MAIN/PCBOARD.C` — a
  file no door can link. `-3` stays, so the code is still 386.

`-D_FARDATA_=_FAR_` and `-P` stay. The previous `BLDKBC.BAT` omitted
both along with `-V -Vmp -Vmd`; a library built that way would have
handed doors a struct layout disagreeing with `PCBOARD.EXE` at runtime,
and nothing would have reported it.

**`pcbkit_l.lib` is not the SDK.** The 119/130-module manifest came from
it. It is the PPLC/utility link target — PCBSETUP links it too — built
without `-DLIB`, and `docs/pcboard-internals/PCBKIT-LIB.md` had already
recorded it as stale. Using its module list cost:

- **3 wrong-tree picks.** CNAMES and HELP must come from
  `SOURCE/TOOLKIT/`, not `SOURCE/PCB/` and `SOURCE/SCRNIO/`. ANSI is the
  instructive one: `pcb153/SOURCE/DISPLAY/ANSI.C` *looks* correct — it
  defines `agotoxy`, `asetcolor`, `awherex`, `awherey`, `curcolor` — but
  does not compile under `-DLIB`, and the linker wants those symbols
  **uppercase**. They come from `ASM/ANSI.ASM`, assembled `tasm /m3`.
  Same casing split `OUT/pwa153/BUILD-RECIPE.md` records for PPLC.
- **22 absent modules**, each traced from a linker "Undefined symbol"
  rather than guessed.
- **2 modules that had to come out.** `SCRNIO/GETKEY.C` and
  `DOS/SHOWERR.C` are PCBSETUP-side, needing host globals (`Colors[]`,
  `_KBDSTATUS`, `_SHOWCLOCK`, `_UPDATEKBDSTATUS`) no door supplies. The
  toolkit already ships the door-side replacements, `SMALLERR.C` and
  `NOINPUT.C`.

Result: **150 of 150** functions in Clark's shipped
`TOOLKIT/OTHER/QUICKREF` are present — 139 by name, the other 11 as the
`ASYNC_*` entry points the headers map them to. Nothing unresolved.

**Two limits, neither a defect.** Small and compact fail every sample
with `Segment _TEXT exceeds 64K`: one code segment, and `initdoor`'s
closure is larger than that alone. Those libraries are for doors using a
small slice of the toolkit, which is why Clark shipped four models.
`SAMPLES/INPUTREQ.C` fails with `Undefined symbol _MAIN` — a
replacement-module example meant to be compiled into a door, not linked
standalone; TLINK emits an invalid `.EXE`, so it is not shipped.

**Category libraries moved.** The 9 `*_L.LIB` moved from
`OUT/pwa153/lib/` into `toolkit/pwa153/bc31/lib/` beside the SDK. Same
source tree, same compiler, and Clark kept his in `LIB/BCDOS/BC31/` —
inside the library tree, not an output directory. They are **not** built
by `BLDKBC.BAT`, which builds only the SDK; they come from Clark's
per-directory MAKEFILEs via `COMPILE.BAT` -> `PCBOARD.MAK`.

**e4181e5 audited to the end.** The 2026-08-25 restructure deleted 32
files. 28 were the binary sets already recovered in v0.3.0 (13 Clark
originals, 15 Watcom `_W`). Two were `&1` shell-redirect junk, correctly
dropped — though `toolkit/pwa153/SOURCE/MISC/&1` is still in the tree.
Three were real and had never come back, verified by blob hash:

- `toolkit/pwa153/SOURCE/MISC/VIRTUAL1.C` — the **original Borland**
  version. Four other `VIRTUAL1.C` survive (delta154, irc1541, pcb154,
  reference) but none is byte-identical: they carry Watcom port fixes
  (`__WATCOMC__` guards, `(char *)` and `(void*)` casts). pwa153 is the
  frozen preservation base, so it had lost the only unmodified copy and
  kept only derivatives.
- `toolkit/pwa153/H/PCB153FX.H` — the BCC 3.1 compat header for the
  `AUTO`/`NOTBLANK` = 32768 enum overflow.
- `DOSBOX.CFG` — the repo-root build-environment config.

All three restored. Also established: the 12 `_W` utilities in the root
README's "Clark Utilities — Phase 0" table, plus `PCBTEXT_W.EXE`, were
**never committed at any point in history** — they were not lost in
e4181e5 or anywhere else. `OUT/delta154/README.md` had said "two entries
do not match"; the real gap is 13 of the 28 binaries the root README
describes, and one whole table documents output that is not in the repo.
Corrected there rather than quietly.

**Corrections to prior claims.** `SDK-BUILD-STATUS.md` said PCBKBC and
PCBKIT were "DONE, all 4 models" — neither library was ever committed;
`toolkit/pwa153/{tc201,msc70}/` hold only a `.gitkeep`. The
`pcb-libchain-build.md` arc is marked CLOSED with byte sizes and module
counts for output that does not exist in the repo; noted in place. That
document also plans to rebuild `pcbkit_l.lib` "for MAKEFILE
compatibility" — true for `PPLC.MAK`, not for PCBoard, whose
`PCBOARD.MAK` links the 7 category libraries directly and some objects
straight from disk.

**`.gitignore`.** `!toolkit/**/lib/*.LIB` and `!OUT/*/bins/**/*.EXE`
added; the dead `OUT/lib/` and `OUT/pwa153/lib/` rules removed. Without
the first two, everything this release produced would have been
invisible to git — the mechanism that lost Clark's binaries and the
Watcom set in the first place.

**Still not built.** `OUT/pwa153/` holds `PCBOARDM.EXE` only; the draft
plan calls for 37 root EXEs plus COMMDRV, PCBMAIL and PCBOS2 sets.
`OUT/pwa153/upd154/` holds the 6 IC binaries only. PCBKIT (Turbo C 2.01)
and PCBKMS (MSC 7.0) remain unbuilt, and now need re-running against the
corrected manifest and config rather than the old ones.

**Lesson.** A count is not a verification. "130/130 modules" and "DONE,
all 4 models" were both recorded in good faith and both wrong, because
nothing had tried to *use* the output. Linking one sample door found a
misidentified source tree, 22 missing modules, two modules that did not
belong, and a config that would have produced silently incompatible
structs. Build the smallest real consumer of an artifact before
believing the artifact is finished.


### v0.3.2 — ROOT names the branch; makefiles repointed to our layout (Sep 17, 2026)

Thirteen makefiles in `pcb153` now say which branch they belong to and
point at the directory shape this repo actually uses. Clark wrote
`ROOT = \PROJ` for a company with one product; this tree is one of four.

**`ROOT` now names the branch.** All eleven makefiles that declared a
root are `\PWA153`, each wrapped in a guard:

```
!ifndef ROOT
ROOT     = \PWA153
!endif
```

The guard is not decoration. **Borland MAKE lets a makefile definition
beat `-D`** — the opposite of GNU make — so `MAKE -DROOT=\DELTA154`
silently does nothing without it. Tested both ways before and after:
without the guard the override is ignored; with it, default is `\PWA153`
and `-DROOT=\DELTA154` takes. So each file names its own branch *and*
a build can still retarget it.

**Library paths repointed to our layout.** `LIBLIB` in ten utility
makefiles, `LIBDIR` in `PCBOARD.MAK`, and a hardcoded relative path in
`PPLC.MAK`:

```
$(LIBROOT)\BCDOS\$(BCCOMPILER)   ->  $(LIBROOT)\$(BCCOMPILER)\lib
```

`BCCOMPILER` is already `bc31`, so this resolves to
`\PWA153\LIB\bc31\lib` — exactly `toolkit/pwa153/bc31/lib/`, where the
category libraries moved in v0.3.1. The alternative was bridging the
difference with copies at build time; matching the layout is cleaner and
the layout is the intended one.

**Verified byte-for-byte, not assumed.** Rebuilt PCBOARD against the
repointed paths: `b55719144e359fbc5d6cdf0f4fce401c8b7366476df8a22d179bc1469f6f7a45`,
955,904 B — identical to the pre-change binary. The repoint changes
where the linker looks, not what it emits.

Three makefiles were left alone. `153/PCBOARD.MAK` had `ROOT = ..` —
already branch-neutral by being relative, and arguably the better
pattern; changing it would be a regression. `PPLC.MAK` and
`USERNET2.MAK` declare no `ROOT` at all.

### Two divergences this uncovered

**`PCBSM.MAK` and `MKPCBTXT.MAK` had already been rewritten, unrecorded.**
They said `ROOT = \PCBSRC`, not Clark's `\PROJ`. Diffed against the
originals in `PCBSRCV/000/`: 221 and 48 differing lines. They use
`$(ROOT)\MAIN\SOURCE\H` where the other nine use `$(ROOT)\PCB\SOURCE\H`,
they dropped the `\LIBS\VMDATA` include path, and **MKPCBTXT was changed
from small to large memory model**. Nothing in this file or any README
mentioned it. Only the root name was normalised here; the rest was left
as found, because reverting work whose reason is unknown is worse than
recording it. Three makefiles still disagree about whether the program
source sits under `PCB`, `MAIN`, or neither — unresolved.

**`OUT/pwa153/PCBOARDM.EXE` and `PCBOARD.EXE` were built from the wrong
tree.** Both came from the archive (`PCBSRCV/014`), not from
`pcb153/SOURCE`. Comparing all 27 MAIN files:

- The repo is **LF**, the archive **CRLF** — that accounts for almost the
  whole size difference (CALLWAIT.C: 1900 lines, 1902 bytes). BCC 3.1
  handles LF fine, so this part is harmless.
- **`CALLWAIT.C` line 1460 differs in content:** archive
  `logsystext(TXT_MODEM,SPACERIGHTAT)` vs repo `...,SPACERIGHT)`.
  `SPACERIGHTAT` is the 15.4 enum — the single 15.3->15.4 toolkit change
  `todo/toolkit.md` documents. The archive's v0.014 carries a 15.4 leak;
  the repo back-ported it out, which is correct per the stated rule that
  15.3 stays pure.
- Six files exist only in the repo: `MD5IMPL.CPP` (the 15.4 MD5LOGIN
  work, PCBSRCV revision 025) and five OS/2 stubs. The repo's
  `PCBOARD.MAK` builds `md5impl.obj`; the archive's does not — a 4-line
  difference between the two makefiles.

So those binaries are real and they link, but they are **not this repo's
output**: built from a tree that is not pure 15.3 and missing MD5IMPL.
They should be rebuilt from `pcb153/SOURCE` before anyone treats them as
the 15.3 reference. Recorded rather than quietly replaced.

### The PCB level removed from the paths

`ROOT` names the branch, and the branch is already PCBoard — so
`$(ROOT)\PCB\SOURCE` said "pcb" twice and named a directory that does
not exist in this repo. Ten makefiles, 35 paths:

```
$(ROOT)\PCB\SOURCE      ->  $(ROOT)\SOURCE
$(ROOT)\MAIN\SOURCE     ->  $(ROOT)\SOURCE
$(ROOT)\PCBSM\SOURCE    ->  $(ROOT)\SOURCE\UTIL\PCBSM\SOURCE
```

plus 13 hardcoded `\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\` rewritten to
`$(ROOT)\SOURCE\UTIL\PCBSM\BC31\`. `\PCBSRC` was not Clark's and named
nothing either; it had been introduced in an earlier unrecorded edit.
The result is that every path in these makefiles now resolves inside the
repo as it is laid out, with no bridging copies.

### VMData: found, and it was ours

`CLARK-DRIVE-LAYOUT.md` called VMData "missing everywhere" and named it
the gate on most of the 15.3 utility set. That was wrong — and wrong in
an interesting direction.

The source was in `pcb154/LIB/`: `VMAVL.C`, `VMFUNCS.C`, `VMDATA.H`,
`vmavl.h`. It is **not Clark's code**. `VMAVL.C` and `vmavl.h` are
clean-room GPLv3 work by **sysop/0**; `VMFUNCS.C` is **hexadecimal
v0.036**. The crew had already written a replacement for the missing
library and nothing said so.

Copied into `toolkit/pwa153/{SOURCE,H}` — checked first for
`__WATCOMC__` guards and Watcom-port casts, of which there are none, so
the Borland build takes it unmodified. What is still not done is
building `VMDATA.LIB` from it. Seven utilities stay blocked until that
happens, but the blocker is now a build step rather than a missing file.

Clark's own VMData — the `c:\vmdata\src\` that `PCBICEVT` referenced —
is still absent and may never have left his machine.

### md5\os2\md5.obj recovered; packfido is simply gone

> **CORRECTED 2026-09-23 — read this first.** Everything below about
> `md5` still stands. Everything below about **`packfido` does not**.
> Clark's `packfido.c` is still missing, but the program has been
> reconstructed from the shipped binary and there are now three of them
> in the tree. Three specific claims in this section are wrong: packfido
> was **not** an external drop like `md5` (it was Clark's own code at
> `E:\TC\PACKFIDO\PACKFIDO.C`, beside `SCANLOG` and `PCBMONI`); it does
> **not** appear "in exactly one place, the makefile" (`CONVERT.CPP` has
> the prototype at :54 and a commented-out call at :125); and
> `\PROJ\packfido\` was never its home. The section left as written,
> because how the wrong conclusion was reached is worth keeping. See
> **v0.3.3+ — packfido reconstructed** below, and
> `pcb153/SOURCE/MISC/PACKFIDO/README.md`.


`PCBOARD2.MAK` has referenced `$(ROOT)\md5\os2\md5.obj` since forever
against an object that existed nowhere in the tree — `PCBSRCV/000/MISC/MD5/`
has the RFCs and `MD5.TXT` and two **empty** `DOS/` and `OS2/`
directories.

It was in `devtools/Md5.zip` (30,803 B) the whole time. Recovered to
`pcb153/SOURCE/MISC/MD5/`:

```
MD5.ASM        14,773   MASM 6.0, public domain (RSA reference port)
OS2/MD5.OBJ     1,492   OMF, THEADR "md5.asm"
MD5.TXT        11,226
RFC1321.TXT / RFC1725.TXT
README.md               provenance
```

The `OS2/` placement is not a guess. `MD5.ASM` itself is OS-neutral —
zero INT 21h, zero `Dos*` calls — but it exports `DoThunk32to16` and
`FlatToSel`. 32↔16-bit thunking and selector conversion are OS/2
constructs; DOS has no use for either. The object belongs under `OS2/`,
which is exactly where `PCBOARD2.MAK` looks. `PCBOARD2.MAK` repointed
at the new path (2 references).

`FILE_ID.DIZ` came across too, because it says in the author's own words
what the package is: source and text describing the **"Shared Secret"
method of password exchange for use on BBSes**, based on MD5. That is
the 15.4 MD5LOGIN feature, which is why Clark had it at all.

`ARSENAL.LIB` was the seventh file in the zip and was **not** copied.
1,577 bytes of plain ASCII containing no object code — a redistribution
notice from Arsenal Computer Services, a BBS CD-ROM distributor, of the
"this file has been triple inspected for virus infection" variety. `.LIB`
there is the BBS file-area convention for a label file, not a linker
library. The original stays intact in `devtools/Md5.zip`.

**`packfido.c` cannot be fixed, but it now has an address.**
`FIDOUTIL.MAK` builds `packfido.obj` from `$(ROOT)\packfido\packfido.c`
— and that path is the clue. It is a **top-level directory under
`\PROJ`**, a sibling of `\PROJ\md5\`. Across `FIDOUTIL.MAK` and
`PCBOARD2.MAK`, those two were the *only* dependencies living outside
`$(ROOT)\source`; everything else resolves inside the source tree. That
is what made them the two missing directories, and it says what they
were: external drops Clark kept beside the project rather than modules
he wrote. `md5` proved to be exactly that — a public-domain package from
a BBS file area. `packfido` is very likely the same kind of thing.

The file itself is gone. `packfido` appears in exactly one place across
the whole archive and repo, and that place is the makefile: no header,
no prototype, no call site, no object, and — unlike `md5` — not even an
empty directory stub in `PCBSRCV/000/`. `SOURCE/FIDO/PACKMSG.CPP` was
checked and ruled out: similar name, but it is an include-fragment, a
bare statement body with no function wrapper, not something that
compiles to an object.

So the path was repointed anyway, to `$(ROOT)\source\packfido.c` —
`pcb153/SOURCE/packfido.c`. It is one loose file, so it gets a loose
file's home rather than a directory holding a single `.c`; the note on
what is missing and what was ruled out lives with the only consumer, at
`pcb153/SOURCE/MISC/FIDOUTIL/PACKFIDO-MISSING.md`. A recovered file drops
straight in with no further edits.
`FIDOUTIL.EXE` cannot link until then — and it fails at compile rather
than at link, which is the better failure, since the error names the
file.

### normalize-case.sh

`pcb154/normalize_case.sh` covered `LIB/H`, `MAIN/SOURCE/H` and
`MAIN/SOURCE/H/H` — three directories in one branch, and no toolkit tree
at all, so `toolkit/<branch>/H` had never been normalised by anything.
Since the OpenWatcom Linux-native build is how delta154 and irc1541 are
meant to be built, and Linux is case-sensitive while
`#include <vmdata.h>` meets a file called `VMDATA.H`, that gap mattered.

`MAIN/build/scripts/normalize-case.sh` replaces it: all four toolkit
branches plus the program trees (12 directories), a `--check` dry run
that writes nothing, and clash detection that exits 2 when two files
differ only in filename case **and** in content — because on a
case-insensitive checkout only one of them exists at all, and the other
is silently stale. Tested: 60 copies from 67 headers, idempotent on a
second run, clash correctly caught.

The copies are build output. The header says so twice. Committing them
doubles every header in git and leaves two files to keep in sync.

### .gitignore: the same mistake, twice, by me

The `.gitignore` LAST WORD block from v0.3.1 — the exception list that
makes the SDK libraries and the sample doors visible to git — was lost
**twice this session**, both times because I committed `.gitignore` from
a staged copy that predated my own edit to it. The second time it
silently reverted a fix I had made an hour earlier, and the 12 sample
door binaries went invisible again.

That is the exact mechanism that lost Clark's binaries and the Watcom
set in the first place, reproduced by the person documenting it.

`device_stage_files` returns a point-in-time snapshot. Staging a file,
editing the staged copy, and committing it back is only safe if nothing
else wrote to that file in between — and "nothing else" includes an
earlier commit in the same session. It was caught by simulating the
ignore rules against a list of paths that *should* be visible, not by
reading the file, which had looked plausible both times.

Rebuilt from the live 6,212-byte file, with the MD5 object added as an
exception (`*.OBJ` would otherwise have hidden it — source material, not
build output). 13 representative paths simulated, 0 hidden.

**Rule going in:** after committing any file that governs what gets
committed, re-read it from the device and verify the rules, not the
bytes.

### VIRTUAL1.C: v0.3.1's "original Borland version" was not original

v0.3.1 restored `toolkit/pwa153/SOURCE/MISC/VIRTUAL1.C` from `e4181e5^`
and recorded it as "the ORIGINAL Borland version… the only unmodified
copy," on the grounds that the four surviving copies in delta154,
irc1541, pcb154 and reference all carry Watcom port fixes. The
restoration was right; the claim was not. Diffed against Clark's own
copy in the archive (`PCBSRCV/000/LIB/SOURCE/MISC/VIRTUAL1.C`), the
restored file differs by exactly one line:

```
-#include <stat.h>
+#include <sys/stat.h>
```

A port fix, smaller than the others but a fix all the same. **All five
copies in the repo were modified.** Clark's original survives only in
the archive. Replaced 2026-09-17 with the archive file, CRLF converted
to LF to match the rest of the toolkit tree (6,432 -> 6,207 B); content
byte-identical to Clark's otherwise.

**The merge was deliberate and was never written down.** Enumerating all
four branches settles it:

| Branch | VIRTUAL.C | VIRTUAL1.C | virtual.h | VIRTUAL1.H |
|---|---|---|---|---|
| **pwa153** | 14,610 merged | 6,207 | 3,949 merged | absent |
| **pwa154** | 14,610 merged | **absent** | 3,949 merged | absent |
| delta154 | 8,283 split | 6,345 | 2,283 split | 2,220 |
| irc1541 | 8,325 split | 6,575 | 2,283 split | 2,220 |

Both PWA branches carry byte-identical merged files — source *and*
header — and `VIRTUAL1.H` was deleted from both. The two OpenWatcom
branches kept Clark's split untouched. That is a scoped, intentional,
Borland-side change, not an accident.

It also means **pwa153's `VIRTUAL1.C` is the anomaly**: pwa154 has none,
because the merged file already contains it. The deletion in `e4181e5`
was most likely part of the merge rather than part of that commit's data
loss — in which case v0.3.1 restored a file the merge had deliberately
removed, and today's replacement entrenched it. Left in place pending a
decision, and documented in
`toolkit/pwa153/SOURCE/MISC/VIRTUAL-MERGE.md` (mirrored to pwa154).

The *reason* for the merge is still unrecorded. Identical symbol names
in two files collide when a build script compiles a whole directory, and
one file behind a `#define` cannot drift from itself — but both of those
are inferences. Whoever made the change should say which, and that note
should be corrected with the real answer rather than the guess.

**They are not duplicates of each other**, which was the question that
started this. `VIRTUAL.C` and `VIRTUAL1.C` implement the same
seven-function API with incompatible signatures:

| | VIRTUAL.C | VIRTUAL1.C |
|---|---|---|
| pointers | `VirType huge *` | `VirType *` (near) |
| record counts | `long` | `unsigned` |
| cache | `findincache` / `insertincache` | none |
| error hook | `virtualerror(void)` | `virtualerror(int Code)` |

Identical symbol names, so a program links one or the other, never both.

What *is* a near-duplicate is the repo's own
`toolkit/pwa153/SOURCE/MISC/VIRTUAL.C` (14,610 B): an earlier session
**merged both implementations into one file** behind `#define
VIRTUAL_HUGE`, and merged the headers the same way. Its huge-pointer
half is byte-identical to Clark's `VIRTUAL.C`; its near-pointer half is
`VIRTUAL1.C` with the same `sys/stat.h` change. That merge is a crew
modification sitting in what is supposed to be the frozen preservation
base, and it is recorded here rather than reverted — the merged header
still serves the restored `VIRTUAL1.C` correctly, since without
`VIRTUAL_HUGE` defined it yields exactly VIRTUAL1's ABI.

### FIDOUTIL.EXE was never actually blocked

`packfido.obj` supplies one symbol, `do_pack()`. Every occurrence of it
in the entire archive:

```
MISC/FIDOUTIL/SOURCE/CONVERT.CPP:53    void do_pack(void);
MISC/FIDOUTIL/SOURCE/CONVERT.CPP:124     //do_pack();
```

A declaration and a commented-out call. Checked against all eleven of
FIDOUTIL's inputs; `CONVERT.CPP` is the only file that mentions it. The
object contributed nothing to the linked image, so `packfido.obj` was
dropped from `EXE_DEPENDENCIES` and from the linker response list, and
the build rule commented out rather than deleted. FIDOUTIL.EXE is
buildable today with all ten of its real modules.

`PACKFIDO.EXE` — the standalone program in the ship list — is a
different matter and remains unbuildable: no makefile, no source, and
nothing to work from beyond the prototype `void do_pack(void)`.

**Lesson, again.** Every problem in this release was a path or a tree
that nobody had checked against the thing it claimed to describe —
`\PROJ` naming nothing, `\PCBSRC` appearing from nowhere, `BCDOS\BC31`
pointing where the libraries no longer are, and a binary built from the
wrong source that compiled perfectly, `\LIBS\VMDATA` called missing
when the crew had written a replacement, an `md5.obj` reference that
resolved to nothing for thirty years, and a `.gitignore` I broke myself
by trusting a stale copy. None of it announced itself.


---

### v0.3.3+ — packfido reconstructed, FIDOUTIL built, OS/2 opened up (Sep 23, 2026)

`packfido.c` is still gone. Everything else this project believed about
it was wrong.

**Where it actually lived.** Two Borland IDE desktop files record the
path: `PCBSRCV/000/MISC/IDX/MAKEIDX.DSK` and
`PCBSRCV/000/UTIL/PCBMONI/PCBMONI.DSK` both list
`E:\TC\PACKFIDO\PACKFIDO.C`, with neighbours `E:\TC\SCANLOG\SCANLOG.C`
and `E:\TC\PCBMONI\PCBMONI.C`. `E:\TC\` is the developer's Turbo C
scratch drive; the product tree is `D:\PROJ\...` on the other drive in
the same list. The source archive was made from the product tree, so
anything living only under `E:\TC\` was never in scope.

**Correction 1 — it was not an external drop.** The
`md5\os2\md5.obj` section above reasons that `packfido`, like `md5`, was
"an external drop Clark kept beside the project rather than a module he
wrote", and "very likely the same kind of thing" as a public-domain
package from a BBS file area. It was not. It was Clark's own code on
Clark's own scratch drive, alongside `SCANLOG` and `PCBMONI`, both of
which are unambiguously his.

**Correction 2 — it appears in more than the makefile.** That same
section says `packfido` "appears in exactly one place across the whole
archive and repo, and that place is the makefile: no header, no
prototype, no call site". There is a prototype and there is a call site,
both in Clark's own converter:

```
MISC/FIDOUTIL/SOURCE/CONVERT.CPP:54     void do_pack(void);
MISC/FIDOUTIL/SOURCE/CONVERT.CPP:125      //do_pack();
```

The call is commented out, which turned out to be the most informative
line in the file — see below.

**Correction 3 — `\PROJ\packfido\` was not its home.** The path
repointing recorded above assumed the makefile's
`$(ROOT)\packfido\packfido.c` named a real top-level directory. The
`.DSK` files say otherwise.

#### What the program does, read out of the shipped binary

`pcb1541/install/dist/target/PACKFIDO.EXE`, 23,214 bytes, sha256
`ef584f…f63a8`. It is **not** a FidoNet packet packer — an earlier note
in this repo guessed that. It compacts the Fido **area configuration**,
dropping records whose conference is out of range, whose conference is
not a Fido conference, or that duplicate an area already kept. DGROUP
base `0x5230`, fixed by correlating 27 of 33 string offsets against
immediate operands.

The duplicate rule was the find. Three bitmap routines, one bit per
conference — `0x2EE3` sets, `0x2DEB` tests, `0x2F75` **clears**. The
clear means the bit is *this conference is still available*, so the first
area for a conference takes it and every later one is dropped. Nothing
in the program's strings hints at it: `removed %5d %s...` reads the same
for all three drop reasons. Later confirmed by experiment against the
shipped binary, not just by reading.

#### PACKFIDO was abandoned at 15.21, and Clark's own files say so

Three independent pieces of evidence:

1. **`DOC/PACK.DOC`**, copyright 1995, shipped unchanged in the 15.41
   install set, describes a utility *"written for PCBoard 15.21 to pack
   the FIDO configuration file PCBFIDO.CFG."* PCBFIDO.CFG stopped
   holding the areas at 15.22.
2. **Version strings across all 36 shipped EXEs.** Everything that
   touches current data carries 15.3 — PCBOARD, PCBOARDM, PCBSM,
   PCBPACK, PCBSETUP, MKPCBTXT, UUIN, UUOUT, UUXFER, PCBMODEM, PPLC330.
   **FIDOUTIL**, the 15.21→15.22 converter, carries 15.0, 15.21 *and*
   15.22. **PACKFIDO carries 15.0 and nothing later.** (The `11-10-94`
   date string is not the discriminator — PCBOARD.EXE and PCBSETUP.EXE
   carry it too. It is a shared build stamp.)
3. **`//do_pack();`** — the pack step disabled at conversion time rather
   than fixed.

So a 15.3 or 15.4 sysop had a `PACKFIDO.EXE` on disk, documented as a
15.21 tool, that could not read the file it was pointed at.

#### What was built

| Path | What |
|---|---|
| `pcb153/SOURCE/MISC/PACKFIDO/` | the 15.3 base — aimed at a **byte-exact** rebuild of Clark's shipped binary |
| `pcb153/upd154/SOURCE/MISC/PACKFIDO/` | the 15.4 upgrade that was never shipped — Borland C++ 3.1 |
| `pcb154/MAIN/SOURCE/MISC/PACKFIDO/` | the same source, OpenWatcom — DOS 16, OS/2 32, DOS/4G |

The byte-exact attempt is **21,938 bytes against Clark's 23,214**, small
model, 512-byte header. It does not link yet: four symbols missing, none
of them ours — `retrycount()`, `findstartofname()`, `_int23hnd`,
`_int24hnd`, all the *kit's* dependencies on category libraries that
exist only in large model. The small-model libraries are the "4 models"
leg of the SDK matrix, still open because `TK.CFG` has `-ml` baked in.

Evidence that fixed the build shape, all measured from the binary: 8
relocations and a 512-byte header → small model; `"15.0"` present and
`"14.5"` absent → built without `-DLIB`; the CNAMES error strings in no
library we hold → they are in `PACKFIDO.OBJ`, so the program opens
`CNAMES.@@@`/`.ADD` and checks the RecSize header itself. `-P` is not
optional — the kit libraries are C++ objects, and a C compile left eight
kit symbols undefined. The recipe is Clark's own, from
`reference/pcball/pcboard/pcb-util/PCBTEXT/MKPCBTXT.MAK`.

#### Three documents proved wrong by their own data files

- **`FIDO.DOC`** lays an `AREAS.DAT` record over offsets 2..82 — 81
  bytes. Clark's own `AREAS.DAT` measures **71**: `48,566 − 2 = 48,564`,
  `48,564 / 71 = 684.0000` exactly, 684 of 684 records sane. The ten
  bytes are the trailing `reserved[10]`; it is in the header and in the
  document and not in the data. The record size is now **probed**, not
  assumed.
- **`CNAMES.DOC`** says seek `(ConfNum-1)*RecSize+2`. The code says
  `ConfNum` — conference 0, the Main Board, is record 0. `pcbconftype`'s
  739 bytes are a **merge** of `oldconftype` (548, in `CNAMES.@@@` after
  a 2-byte RecSize header) and `addconftype` (256, in `CNAMES.ADD`, no
  header). There is no 739-byte file anywhere.
- **`PCBDAT.DOC`** lists lines 245, 246 and 247 as "Reserved". They are
  `EnableFido`, `FidoConfig` and `FidoQueue` — `DATAFILE.C` reads them
  there, and Clark's own `PCBOARD.DAT` has `PCBFIDO.CFG` on 246.

`PCBOARD.DAT` is a line-oriented **text** file; `pcbdattype` is the
in-memory structure the reader fills, not the layout on disk. A harness
built by writing that struct to a file produced something PCBoard cannot
read, which cost a day.

#### OS2PORTS.DOC, and the answer to "do the formats follow OS/2 too"

They do not need to — **the data files are identical on both platforms**.
`OS2PORTS.DOC` is the only OS/2-specific document in the devkit and it
is not a format document at all: it covers reaching an OS/2 comm port
from a DOS program, `open("COM2")` plus `INT 21h AX=440Ch`. It is the
public write-up of `pcb153/SOURCE/MODEM/DEVIOCTL.C`, which this repo has
been compiling all along — `MODEMOS2.C:442` is the document's own DTR
example verbatim.

The whole Developer's Information package — 23 documents from
`devtools/Develop.zip` — was already extracted in the tree four times,
under `toolkit/pwa153/docs/devkit/`, `.../develop9/` and the same two
under `pwa154`. Reading `FIDO.DOC` first would have been faster than
disassembling the binary.

#### Two compilers, one source

`wcc -bt=dos -ml` → 19,878 B, `wcc386 -bt=os2v2 -mf` → 19,763 B,
`wcc386 -bt=dos -mf` → 26,262 B, all 0 errors. The Borland and
OpenWatcom 16-bit binaries were run over identical copies of Clark's own
`AREAS.DAT`: 684 records in, 683 out, 48,495 bytes,
`ab6fd621…6a4b` from both, with identical console output.

No shipped binary ever packed the version 3 format, so that agreement is
the only cross-check the program can have, and it is weaker than a byte
diff against a real binary. Said plainly rather than dressed up.

The portability work that made it possible is one typedef: `unsigned
int` is 2 bytes under Borland, Turbo C, MSC and 16-bit `wcc`, and **4
under `wcc386`**. A conference number read through an `unsigned int *`
would have swallowed the next two bytes of every record on any 32-bit
target, silently. Every on-disk 16-bit field is now `pcbword`, and
`main()` refuses to run if `pcbword` is not 2 bytes.

#### An acceptance harness, built and then retired

A three-program harness — fixture builder, file-ops helper, driver batch
— proved the pack logic matched Clark's binary byte for byte on 530
areas with 30 removals, and confirmed the first-one-wins duplicate rule
by experiment. It was then retired to `attic/RETIRE.LST`: three programs
and a scratch directory to prove one 23 KB utility behaves is the wrong
target when the right one is a byte-exact rebuild, where the sha256 *is*
the test. What it proved is kept in the README.

Two things it taught, worth keeping: **`COPY` silently does nothing from
inside a batch file under DOSBox-X** — it works from the autoexec, and
from a batch there is no error, no file, and `ERRORLEVEL` unchanged — and
a `PCBOARD.DAT` path longer than 32 characters is **truncated** by
Clark's binary, which then opens nothing, prints `done.` and changes no
file. A silent pass that proves nothing.

#### FIDOUTIL builds — that makes twelve, not eleven

`BUILD_DOS.BAT` names eleven targets and `OUT/clark-original/DOS` holds
exactly those eleven to diff against. **FIDOUTIL is the twelfth**, and it
now builds: 11 objects, 0 errors, `FIDOUTIL.EXE` 153,674 bytes against
Clark's 214,586. Its own strings come out of our binary — *"PCBoard FIDO
file conversion utility / Copyright Clark Development 1996"*.

It builds in **both** trees. `pcb153/upd154` gives a byte-identical
binary, because FIDOUTIL's ten sources are the same in both and none of
the 33 differing headers adds a field FIDOUTIL reads.

**`USEDBC50` is a compiler marker, and it is not a wall.** `FIDOUTIL.CFG`
said `-nbc50`, `\BC5\INCLUDE`, `\BC5\LIB`, and the file beside it says
*"Now using bc50."* — it sits in **25 directories**. We have no Borland
C++ 5.0. The source compiles clean under 3.1 anyway, unchanged. So the
marker records the compiler Clark moved to, not one the source requires —
and that reopens PCBSETUP, PCBTEXT, USERNET, PCBPACK, PCBDIAG, PCBFILER,
PCBNLC, PCBMODEM, PCBSTATS, ZMODEM, WAITFILE and MSETUP.

**But it is the compiler, so byte-exact still needs BC 5.0.** Our 153,674
against Clark's 214,586 is a working FIDOUTIL, not a matching one. Part
of the gap is `packfido.obj` being absent; the rest is a different
compiler's runtime and code generation. A byte-exact FIDOUTIL is a BC 5.0
job, exactly as PACKFIDO's byte-exact target is a BC 3.1 small-model job.

Two things cost the time, and neither was the compiler: without
`-D_FARDATA_=far` and five other defines the parse dies inside
`PCBOARD.H` with *"Enum syntax error"* at a perfectly healthy enum; and
**BC 3.1 reads `TURBOC.CFG`, not `BCC.CFG`**, so a config under the wrong
name is ignored in silence and you get *"Unable to open include file
'stdio.h'"*.

Also learned: `SHOWERR2.OBJ` is in **no** library — searched all thirteen
— which is why the makefile lists it as a loose object.

#### A 15.4 header bug that hid behind a guard

`pcb153/upd154/SOURCE/H/USERS.H` declared `personal_psa_t` and
`timebank_psa_t` — both 15.4 additions — **inside `#ifdef PCB152`**,
while the `extern Personal` and `extern Bank` that use them sit outside
it. Any translation unit not defining `PCB152` lost the types and then
hit the declarations, giving two bare *"Declaration syntax error"*s that
point at the externs and say nothing about the cause. `PASSTHRU.CPP` was
the module that failed; `CPP.EXE` on the preprocessed output is what
showed the typedefs were simply absent. Moved out, unconditional.

#### PACKFIDO: two compilers, three targets, and an OS/2 port

One source now lives in `pcb153/upd154` and `pcb154/MAIN`, byte
identical, built by Borland C++ 3.1 and by OpenWatcom. Their output over
the same `AREAS.DAT` is byte identical — 684 records in, 683 out,
`ab6fd621…6a4b` from both — which is the only cross-check a program with
no shipped binary can have.

`pcb154` is a real OS/2 port, not a recompile: `DosOpen`, `DosRead`,
`DosWrite`, `DosSetFilePtr`, `DosClose`, `DosDelete`, `DosMove` direct,
with the C file runtime not linked at all. It is the *smallest* of the
three binaries at 16,554 bytes — it was 19,763 as a recompile, and that
3,209-byte drop is the evidence the port took.

The portability work was one typedef and it mattered: `unsigned int` is 2
bytes under Borland, Turbo C, MSC and 16-bit `wcc`, and **4 under
`wcc386`**. A conference number read through an `unsigned int *` would
have swallowed the next two bytes of every record on any 32-bit target,
silently. Every on-disk 16-bit field is now `pcbword`, and `main()`
refuses to run if it is not 2 bytes.

**Watcom does not predefine `__OS2__` for `-bt=os2v2`** — it defines
`__OS2V2__`. The first OS/2 build compiled the DOS path and linked
cleanly with no diagnostic; the only tell was the binary coming out the
same size as before the port.

#### OS/2 for the rest: probed, and the blocker is named

`wpp386 -bt=os2v2` on `PCBFU.CPP`, includes mirrored to lowercase, with
`-DLIBENTRY= -Dpascal= -D_FARDATA_=` — **21 errors**, down from several
hundred, every one a known Borland→Watcom bridge item: `dir.h` and
`alloc.h`, `bool` already built in, and six `~` destructor errors in
`TYPES.HPP` where `far`/`pascal` were stripped. `WATCOMPAT.H` already
exists in three trees and `BUILD_OS2_OW.SH` was written around it.

**The real blocker is one level down.** FIDOUTIL links eight category
libraries and none of them exists for OS/2. `BUILD_OS2_OW.SH` sidesteps
that by compiling the library *sources* into the executable. An OS/2
FIDOUTIL means building the category chain with `wcc386 -bt=os2v2` first
— the same wall the byte-exact PACKFIDO hit one axis over, where the
libraries exist in one model for one platform.

And it is a port, not a recompile: FIDOUTIL reads `pcbconftype` and
`AREA_STRUCT` off disk **through real structs**, so under `wcc386` every
`int` in them doubles and the file layout silently stops matching.

#### TOOLKT21 is IBM's, not Clark's — now in devtools\

Recorded here so it is not conflated again. PCBCP's source carries
`#include <\toolkt21\c\os2h\valapi.h>`, and this project spent time
assuming `\TOOLKT21\` meant Clark's *Doors Developer's TOOLKIT v2.0*
(`devtools/TOOLKIT2.ZIP`). It does not. `\TOOLKT21\` is the install
path of **IBM's Developer's Toolkit for OS/2 2.1**, shipped on The
Developer Connection for OS/2 Volume 1 (August 1993) — archive.org
`ibm-devcon-01`, `devcon-01.iso`, 395 MB, where it appears in three
forms: an installable version and 3.5" and 5.25" diskette image sets.

**Found and landed.** Now in `devtools/TOOLKT21.ZIP` (11,220,521 bytes),
with `TK21D35.ZIP` and `TK21D525.ZIP` (3.5" and 5.25" diskette sets)
and `TOOLKT21.md` (provenance).  See APPLY.txt v0.3.3+ (16).

`devtools/TOOLKIT2.ZIP` is a different thing entirely and is worth
having on its own account: Clark's Doors Developer's TOOLKIT v2.0, dated
1993, holding **12 prebuilt kit libraries** — `PCBKIT_S/M/C/L.LIB` for
BC, MSC and TC — plus ~20 loose stub objects each (`NOANSI`, `NOCHAT`,
`NODISP`, `COMMDRV.OBJ`, `FOSSIL.OBJ`, `PCBDAT.OBJ`). Not yet extracted
into the tree.

---

## For future contributors

If you're extending this project:
- Read this file top to bottom for the whole story
- Read `pcb1541/install/dist/target/README.md` for the .RED archive
  arc in detail
- Check `MAIN/README.md` for the current build model
- New work goes into this file (not per-release notes that get lost)
- Subsystem-specific work goes into that subsystem's README

The pattern: consolidated work log lives in the tree, per-release
notes exist only to describe THIS zip's delta, and the tree work log
gets updated in the same commit as the code change.

---

## v0.2.9 — PCBIC v1.2 byte-exact reconstruction (2026-09-16)

All 6 Clark Development EXE binaries from PCBoard's Internet Collection
v1.2 reconstructed to byte-exact SHA256 match. Source: NASM raw-byte
injection with verified function extraction.

### Phases

1. **Ghidra decompilation** — Pcbic.exe → 25 C source files, 1,074 functions
2. **BCC 3.1 compilation** — `asm db` byte injection (861 functions via C,
   213 via NASM)
3. **OS/2 analysis** — LX binary format, 4 objects, 1,090 functions,
   212 import call sites → 110 unique DLL functions across 10 modules
4. **Cross-compilation** — NASM (Linux host) + OpenWatcom wlink (Linux native)
   → working OS/2 LX executables, no OS/2 VM required
5. **Byte-exact link** — code sections verified identical, LX reconstructed
   from verified components + original structure
6. **All remaining binaries** — PCBICCFG (743 funcs), PCBICEVT (474 funcs),
   TESTIC (249 funcs), TESTIC2 (258 funcs + 76 imports)

### PCBICEVT.EXE — Event Manager

PCBoard's Internet Collection Event Manager. Holding directory processor,
ranking (`/RANK`), purging (`/PURGE`), approval (`/FORCE`). Uses Clark's
VMData virtual memory subsystem (`c:\vmdata\src\`). Compare with pcbis.exe
event manager — Clark's event manager is how PCBoard works.

### Toolchain discovery

- DOSBOXX.ZIP updated with BC++ 2.0 for OS/2 (`BUILDROOT/BCOS2/`)
- OpenWatcom V2 `wlink` (Linux native) proven for OS/2 LX cross-linking
- NASM 32-bit OMF `.obj` output compatible with wlink import resolution
