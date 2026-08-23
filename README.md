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
pcb153/        15.3 source (Borland C++ 3.1) — Corey Blake / PWA archive
pcb154/        15.4 source (OpenWatcom 2.0) — 15.3 + Clark's 15.4b delta applied
pcblibs/       Clark's shared toolkit library — both versions link it
pcbcbase/      third-party: CODEBASE (LGPL) + BC31 prebuilt binaries
1541/          our 15.41 work — new programs, ports, scaffolds
  wip/         kiddo's Pascal from mysticbbsirc, being ported to C
  pcbcp/       Clark's OS/2 PM control program (1996, found in reference/)
  qfront/      QFront mailer (Phase 15, style-audited)
  pcbmail/     PCBoard Mail scaffold (Phase 27)
  install/     INSTALL.EXE scaffold (Phase 27)
  pcbic/       PCBIC scaffold (Phase 27)
  pcbcomm/     unified serial layer design
  binkd/       BinkD FidoNet mailer
  pcbdraw/     ANSI editor
  PPL/         PPL engine
toolkit/       code to recreate — missing Clark binaries
drivers/       netfosdl (wrench's DOS FOSSIL, GPLv3, verified working)
docs/          DOCDEV (MSGS.TXT, HEADERS.TXT, etc)
reference/     Corey Blake's archive, mpoli, roysac, PPE collections
patches/       15.3 v0.014 patch source
todo/          docs to review and merge
```

## Build Status

| Metric | Count |
|---|---|
| Programs | 36 (28 Clark + 8 new tools) |
| Clark binaries (Watcom DOS4G) | 28 (16 main + 12 Phase 0 utilities) |
| New tools | 8 (pcbbinkp, pcbdraw, pcbpscan, pcbfido, pcbis, QFront, pcbfoss, pcbiso) |
| PPE collection | 5,703 archives (incl. Roy/SAC donation) |

### Clark Binaries — Main (OpenWatcom 2.0)

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

## The Crew/Team
| Handle | Role |
|---|---|
| Handle | Role

| verta1878 | Project lead
| sysop/0 | Compiler engineer, FPC + OW2IRC maintainer, Tang Console, USB
| bob | Compiler engineer, wcc64 backend, Glide builds
| evga | Display, Mystic, SIO, Cyclade
| kiddo | Protocols, RIPscrip
| wrench | Transport, FOSSIL, DVI/HDMI
| hexadecimal | PCBoards
| byte | Program recovery
| dotmatrix | Documentation recovery

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

## Links

- GitHub: https://github.com/verta1878/pcbrevival
- FPC 2.6.4irc: https://github.com/verta1878/fpc264irc
- Mystic BBS fork: https://github.com/verta1878/mystic-bbs-irc
- OpenOLMS — Open Offline Mail System: https://github.com/verta1878/OpenOLMS
