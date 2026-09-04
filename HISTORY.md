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
  compiler is in tree at `toolkit/pplc/3.20/PPLC320.EXE`.
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
- **v1.10.6** — disassembly parity check on `INSTALL.EXE`. Byte-exact
  rebuild of INSTALL.EXE itself is deferred to install v1.11+.

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
