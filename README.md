# pcbrevival — PCBoard 15.4 BBS Revival

**PCBoard 15.4 source code recovery, OpenWatcom 2.0 port, and modernization.**

pcbirc crew — August 2026

## What Is This

PCBoard was the dominant BBS software of the dial-up era, written by
Clark Development Company. When Clark was closed by the bank in the
late 1990s, the source code nearly disappeared. There is almost no
information about PCBoard 15.4 on the web — no documentation, no
downloads, no history.

Corey Blake purchased what may be the only source license ever sold.
PWA (Pirates with Attitude) preserved that 15.3 source archive —
without them, this project would not exist.

This repo takes that 15.3 source, applies every feature and change
from Clark's unreleased 15.4b beta, and ports the entire codebase
to OpenWatcom 2.0 for cross-compilation on modern Linux.

## Directory Map

```
--- source trees (one per version; each owns its source) ---
pcb153/        15.3 PWA source (Borland C++ 3.1) - the pure base
pcb153/upd154/     15.4 PWA - Clark's 15.4 reconstructed from binaries (source only)
pcb154/        15.4 Delta - the crew's active work (OpenWatcom)
pcb1541/       15.41 IRC - our new work (ports to openwatcom2irc)
               network front-ends: syncterm, binkd, pcbcomm,
               dropbear (SSH secure remote access), ...
patches/       15.4-pwa.patch = the 15.3->15.4 PWA delta over pcb153

--- toolkit + SDK ---
toolkit/       toolkit source per branch (pwa153, pwa154, delta154, irc1541)
OUT/           build outputs per version (pwa153, delta154, irc1541)
  lib/pwa153/  the SDK library matrix (PCBKBC + PCBKIT built; PCBKMS pending)
  support/     shared PCBoard runtime files (non-version-specific)
MAIN/          project model + build system
  DELTA-MODEL.md   the 4-version model
  build/           scripts, manifests, backups, SDK build status
                   (moved here from the repo root)

--- compilers (build tools) ---
MSC70BT.ZIP    Microsoft C 7.0 (retail) + OS/2 add-on - extract to
               C:\MSC70 to build PCBKMS (not shipped unpacked)
PCB153BT.ZIP   Borland C++ 3.1 standalone build tools (for PCBKBC)
TC201BT.ZIP    Turbo C 2.01 standalone build tools (for PCBKIT)
DOSBOXX.ZIP    all-in-one: compilers + dosbox-x + BUILD scripts
devtools/      raw compiler distributions + COMPILERS.md catalog

--- other ---
OS2TK/         OS/2 toolkit
drivers/       SIO serial + netfosdl FOSSIL (GPLv3)
docs/          shared DOCDEV manuals (MSGS.TXT, HEADERS.TXT, etc)
pcbcbase/      third-party CODEBASE (LGPL)
reference/     archives (excluded from release)
todo/          docs to review and merge
normalize_case.sh   lowercase-copy helper for Linux/OpenWatcom builds
```

## Build Status

### SDK library matrix (Clark's toolkit: 3 compilers x 4 memory models = 12 libs)

| Family | Compiler | Status |
|---|---|---|
| PCBKBC | Borland C++ 3.1 | BUILT - 4/4 models |
| PCBKIT | Turbo C 2.01 | BUILT - 4/4 models |
| PCBKMS | Microsoft C 7.0 | compiler in hand (DOS + OS/2), build pending an OS/2 host |

Built libraries live in OUT/lib/pwa153/. Details in
MAIN/build/SDK-BUILD-STATUS.md and devtools/COMPILERS.md. Build scripts:
MAIN/build/scripts (or use the DOSBOXX.ZIP all-in-one bundle).


| Metric | Count |
|---|---|
| Programs | 36 (28 Clark + 8 new tools) |
| Clark binaries (Watcom DOS4G) | 28 (16 main + 12 Phase 0 utilities) |
| New tools | 8 (pcbbinkp, pcbdraw, pcbpscan, pcbfido, pcbis, QFront, pcbfoss, pcbiso) |
| PPE collection | 5,703 archives (incl. Roy/SAC donation) |

### 15.4 Delta Binaries — Main (OpenWatcom 2.0)

> These `_W`-suffixed EXEs are the crew's **15.4 Delta** rebuild,
> compiled with **OpenWatcom** (pcb154/). They are NOT Clark's binaries.
>
> Clark built 15.4 with **Borland C++ 3.1**. His original 1996 binaries
> (Copyright 1996 Clark Development, report v15.4) are preserved as the
> reference at `OUT/pwa153/upd154/clark-original/`. The 15.4 PWA source
> rebuild (also Borland) targets `OUT/pwa153/upd154/`.

| Binary | Size | Description |
|---|---|---|
| PCBOARD_W.EXE | 1.3MB | Main BBS engine |
| LOCAL_W.EXE | 1.3MB | Local login mode |
| PPLC_W.EXE | 1.3MB | PPL 3.40 compiler |
| PCBSETUP_W.EXE | 424KB | Setup utility |
| PCBSM_W.EXE | 221KB | System Manager |
| UUIN/UUOUT/UUUTIL/UUXFER_W | 1.4MB each | UUCP suite |
| MKPCBTXT_W.EXE | 86KB | Text file builder |
| PCBTEXT_W.EXE | 39KB | Text file editor |
| MAKEIDX_W.EXE | 37KB | Index builder |
| USERNET_W.EXE | 28KB | User network flags |
| MAKEHELP_W.EXE | 25KB | Help file builder |
| PCBCP_W.EXE | 77KB | OS/2 PM control panel |
| PCBIS_W.EXE | 48KB | Config TUI |

### Clark Utilities — Phase 0 (hexadecimal + sysop/0)

All 12 Clark utilities that shipped with PCBoard, ported to Watcom DOS4G:

| Binary | Size | Description |
|---|---|---|
| PCBSTATS_W.EXE | 31K | Statistics generator |
| PCBPACK_W.EXE | 84K | Message base packer |
| MSETUP_W.EXE | 108K | Modem database editor |
| PCBMODEM_W.EXE | 528K | Modem config utility |
| PCBEDIT_W.EXE | 133K | Text/config editor |
| PCBMONI_W.EXE | 54K | Node monitor |
| PCBDIAG_W.EXE | 552K | Diagnostics |
| PCBFILER_W.EXE | 215K | File management |
| PCBNLC_W.EXE | 77K | FidoNet nodelist compiler |
| OFFLINE_W.EXE | 26K | Offline flags |
| WAITBU_W.EXE | 25K | Wait for backup |
| PCBTITLE_W.EXE | 15K | OS/2 console title |

### New Tools (GPLv3)

| Tool | Lines | Description |
|---|---|---|
| pcbbinkp | 2,008 | BinkP/1.1 FidoNet mailer |
| pcbdraw | 1,826 | ANSI art viewer/editor |
| pcbpscan | 770 | Upload file scanner |
| pcbfido | 778 | FidoNet console (15.41) |
| pcbiso | 969 | ISO/CD-ROM file area indexer (15.4+) |
| pcbis.exe | 5,710 | Internet services daemon (18 Pascal units) |

## pcbis — PCBoard Internet Services

Disk 4 of the distribution. Multi-protocol daemon bridging PCBoard
to the internet:

- **Telnet** (2323) — FOSSIL bridge, writes PCBOARD.SYS/CALLERS
- **BinkP** (24554) — FidoNet mailer, QFront integration
- **FTP** (21) — PCBoard file areas with security levels
- **HTTP** (8080) — status page, callers, who's online
- **SMTP** — outbound validation emails
- **NNTP** — news↔PCBoard conference gateway
- **QWK** — offline mail networking
- **Events** — timed batch execution

Built with fpc264irc (FPC 2.6.4 fork). Source: `pcbis/src/`

## Drivers

| Driver | Lead | Description |
|---|---|---|
| SIO v1/2K | evga | OS/2 serial I/O, 26 bugs fixed, clean-room |
| FOSSIL | sysop/0 + kiddo | Cross-platform FOSSIL socket layer |

## Libraries Written for Phase 0

| Library | Author | Lines | Purpose |
|---|---|---|---|
| VMAVL | sysop/0 | 324 | AVL tree for VMDataSet (clean room, studied libavl) |
| VMData | hexadecimal | 204 | Virtual memory dataset (v0.036) |
| d4all.h | sysop/0 | 121 | CodeBase type shim for PCBNLC |
| conio_compat | sysop/0 | 108 | Borland conio via BIOS INT 10h |

## Documentation

| Document | Description |
|---|---|
| docs/PHASE0_MISSING_UTILS.md | Phase 0 status — all 12 utilities |
| docs/FEATURE_MATRIX.md | Full feature comparison 15.3→15.4→15.41 |
| docs/PCB1541_DRAFT.md | 15.41 specification (23 sections) |
| docs/FIDONET.md | FidoNet sysop guide |
| docs/WHATSNEW.md | Changelog |
| docs/REFERENCE_CATALOG.md | Reference materials catalog |
| docs/hexadecimal-phase-update.md | Phase 1 roadmap from sysop/0 |

## The Crew

| Handle | Role |
|---|---|
| hexadecimal | PCBoard 15.4 port, Phase 0 utilities, WATCOMPAT.H |
| verta1878 | Project lead, netmodem2irc, OpenOLMS |
| wrench | FOSSIL driver, netmodem2irc engine, pcbmailer |
| sysop/0 | fpc264irc, openwatcomirc, pcbis, VMAVL, tools |
| kiddo | Protocols, serial IRQ, RIP OOP engine |
| evga | SIO driver, Mystic monitor, RIPView engine |
| byte | Recovery + preservation; decrypted PCBIC 1.2 (Pcbic12.zip) |
| dotmatrix | Crew |

## Acknowledgments

- **Corey Blake** — Source license purchase
- **PWA** — 15.3 source archive preservation
- **Roy/SAC (Carsten)** — Physical PCBoard package donation, 4,548 PPE archives,
  box photos. License transferred via POB (German distributor), upgraded 15.21→15.22.
- **mpoli.fi** — BBS software archive hosting
- **CodeBase-for-DBF** — Sequiter Software, LGPL v3.0
  (github.com/MPSystemsServices/CodeBase-for-DBF)
- **libavl** — Buselli/Dankers, LGPL (algorithmic reference for VMAVL)

## License

Our additions: GPLv3
Clark Development source: proprietary (licensed)
CodeBase library: LGPL v3.0
SIO OS/2 driver: GPLv3 (evga, clean-room)

## Links

- GitHub: https://github.com/verta1878/pcbrevival
- FPC 2.6.4irc: https://github.com/verta1878/fpc264irc
- Mystic BBS fork: https://github.com/verta1878/mystic-bbs-irc
