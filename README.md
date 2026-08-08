# pcbirc — PCBoard 15.4 Source Port / 15.41 Revival

**PCBoard 15.4 BBS source code recovery and OpenWatcom port,
with 15.41 revival features by the pcbirc crew.**

pcbirc crew — August 2026

## What Is This

PCBoard was the dominant BBS software of the dial-up era, written by
Clark Development Company. When Clark was closed by the bank in the
late 1990s, the source code nearly disappeared. There is almost no
information about PCBoard 15.4 on the web — no documentation, no
downloads, no history. The software and its source were effectively
lost to time.

Corey Blake purchased what may be the only source license ever sold.
PWA (Pirates with Attitude) preserved that 15.3 source archive —
without them, this project would not exist.

This repo takes that 15.3 source, applies every feature and change
from Clark's unreleased 15.4b beta, and ports the entire codebase
to OpenWatcom 2.0 for cross-compilation on modern Linux. That's the
**15.4 source port** — getting Clark's code building again.

On top of that, the **15.41 revival** adds new features: FidoNet TIC
file echo support, FTP file transfer protocol for telnet callers,
the pcbpscan file scanner, PCBDRAW ANSI art editor integration,
and system tray support via utrayit.

## Build Status

### OpenWatcom 2.0 Port

| Metric | Count |
|---|---|
| Source files compiling | 556/556 (100%) |
| Libraries built | 9 (268/275 files) |
| Binaries linked | 13 (all Clark originals) |

| Binary | Size | Description |
|---|---|---|
| PCBOARD_W.EXE | 1.3MB | Main BBS engine (136 source files) |
| LOCAL_W.EXE | 1.3MB | Local login mode |
| PPLC_W.EXE | 1.3MB | PPL 3.40 compiler |
| PCBSETUP_W.EXE | 424KB | Setup utility (69 source + 32 EXTRAOBJ) |
| PCBSM_W.EXE | 221KB | System Manager (26 source + 23 EXTRAOBJ) |
| UUIN_W.EXE | 1.4MB | UUCP import |
| UUOUT_W.EXE | 1.4MB | UUCP export |
| UUUTIL_W.EXE | 1.4MB | UUCP utilities |
| UUXFER_W.EXE | 1.4MB | UUCP transfer |
| MAKEIDX_W.EXE | 37KB | Index file builder |
| USERNET_W.EXE | 28KB | User network utility |
| MKPCBTXT_W.EXE | 27KB | Text file generator |
| MAKEHELP_W.EXE | 25KB | Help file builder |

### Standalone Tools

| Tool | Description |
|---|---|
| pcbtic | FidoNet TIC file echo processor |
| pcbfcfg | FidoNet FREQ/magic configuration |
| nlcomp | Nodelist compiler |
| upd1541 | 15.3→15.41 data file upgrade |
| pcbpscan | Upload file scanner (replaces THD ProScan) |
| utrayit | System tray support (Win32/Unix) |
| pcbis_ui | Installer TUI |

### DOS: 11 of 11 ✅ (Borland C++ 3.1)

All 11 original shipping binaries build from source under BC++ 3.1
in DOSBox. See PCB154_BUILD_GUIDE.md.

### OS/2: PCBOARD2.EXE ✅ (OpenWatcom 2.0)

OS/2 32-bit native BBS engine. Clean link, zero unresolved symbols.

### OS/2 Addon: PCBCP Control Panel ✅

PCBCP_W.EXE (77KB) — OS/2 Presentation Manager node monitor and
control panel. Source recovered from pcball.zip (pcboard.be),
ported to OpenWatcom 2.0. 8/8 source files compile clean.
PCBCP.INI must be configured by the sysop on first run.

## Documentation

| Document | Lines | Description |
|---|---|---|
| PCB1541_DRAFT.md | 2,322 | 15.41 upgrade specification (22 sections) |
| WHATSNEW.md | 467 | Changelog |
| FIDONET.md | 682 | Complete FidoNet sysop guide |
| PCBIS_UI.md | 452 | Installer and PCBDRAW integration |
| MISSING_FUNCTIONS.md | 350 | Function stubs needed for linking |
| PPL Reference Manual | 7.1MB | Scanned PPL programming manual (PDF) |
| PPL Dev Kit Manual | 6.7MB | PPL developer kit documentation (PDF) |

## The Crew

| Handle | Role |
|---|---|
| hexadecimal | pcbrevival lead, source port |
| verta1878 | netmodem2irc, OpenOLMS |
| wrench | netmodem2irc engine, OpenWatcom dev |
| sysop/0 | fpc264irc compiler, CIADraw/PabloDraw FPC port, utrayit |
| kiddo/evga | Mystic/RIPscrip/RIPView |
| evga | Mystic/RIPView/display |

## License

Our additions: GPLv3
Clark Development source: proprietary (licensed)
CodeBase library: LGPL v3.0
SIO OS/2 driver: GPLv3 (evga, clean-room)
SyncTerm reference source: GPL v2+ (Rob Swindell / Stephen Hurd)

## Links

- Mystic BBS (reference): https://github.com/KALRONG/mysticbbs
- SyncTerm source: https://gitlab.synchro.net/main/sbbs
- PPLEngine (PPE decompiler): https://github.com/mkrueger/PPLEngine
- sysop/0's FPC repo: fpc264irc (separate, 1.6GB)
