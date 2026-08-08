# pcbirc — PCBoard 15.4 Source Port

**PCBoard 15.4 BBS source code recovery and OpenWatcom 2.0 port.**

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
to OpenWatcom 2.0 for cross-compilation on modern Linux.

## Build Status

| Metric | Count |
|---|---|
| Source files compiling | 556/556 (100%) |
| Libraries built | 9 (268/275 files) |
| Binaries linked | 16 |

### Clark's 15.4 Binaries (OpenWatcom 2.0)

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

### Addons

| Binary | Size | Description |
|---|---|---|
| PCBCP_W.EXE | 77KB | OS/2 Presentation Manager control panel |
| PCBBINKP.EXE | 47KB | BinkP/1.1 FidoNet mailer (OS/2) |
| PCBBINKP_W.EXE | 62KB | BinkP/1.1 FidoNet mailer (Windows NT) |

### Standalone Tools (GPLv3)

| Tool | Description |
|---|---|
| pcbbinkp | BinkP/1.1 mailer — poll, answer, status (FTS-1026, CRAM-MD5) |
| pcbpscan | Upload file scanner (replaces THD ProScan) |
| nlcomp | FidoNet nodelist compiler (raw nodelist to CodeBase .DBF/.NDX) |
| pcbis_ui | Installer TUI (Linux/DOS) |

### DOS: 11 of 11 (Borland C++ 3.1)

All 11 original shipping binaries also build from source under
BC++ 3.1 in DOSBox. See PCB154_BUILD_GUIDE.md.

## FidoNet on 15.4

pcbbinkp.exe is a standalone BinkP/1.1 mailer for FidoNet over
TCP/IP. It handles transport only — connecting to remote nodes and
transferring .PKT files and file attaches via Binkley-Style Outbound.

For a working FidoNet setup you also need an external tosser to
process incoming mail into PCBoard message bases. The Husky suite
(HPT + htick) is recommended. See section 23 of PCB1541_DRAFT.md
for the full external tools guide.

Usage:
```
pcbbinkp poll 1:2320/100        Poll a specific node
pcbbinkp answer [port]          Listen for incoming BinkP
pcbbinkp status                 Show outbound queue
```

## Drivers

### SIO v1 / SIO2K — OS/2 Serial I/O (evga)

Clean-room OS/2 serial driver with 31 bugs fixed across 3 audits.
Full 16550A/16650/16750 UART support with proper FIFO detection.
Source at `addons/SIO/src/`.

### FOSSIL Socket Layer (wrench)

Cross-platform FOSSIL driver for DOS, Linux, OS/2, and Windows.
Provides serial port abstraction for BBS software running over
TCP/IP (telnet/SSH). Source at `platform/fossil/`.

## Documentation

| Document | Description |
|---|---|
| PCB1541_DRAFT.md | 2,543 lines — full specification (23 sections) |
| WHATSNEW.md | Changelog |
| FIDONET.md | FidoNet sysop guide |
| PCB154_BUILD_GUIDE.md | Build instructions |
| SETUP_GUIDE.md | Installation guide |

## Directory Layout

```
PCBSRC/             Clark's 15.4 source (proprietary, licensed)
  MAIN/SOURCE/       C/C++ source, headers, makefiles
  LIB/               Pre-built CodeBase library objects
OUT/                 Compiled binaries
  DOS/               DOS target (Borland C++ 3.1)
  OS2/               OS/2 target (OpenWatcom 2.0)
LIBS/                CodeBase 6.5 library source (LGPL v3.0)
addons/
  PCBCP/             OS/2 Control Panel source
  SIO/               SIO serial driver source (GPLv3)
platform/
  fossil/            FOSSIL socket layer (GPLv3)
tools/
  pcbbinkp/          BinkP mailer source (GPLv3)
  binkd/             binkd reference source (GPL, not our code)
  nlcomp.c           Nodelist compiler source
  pcbfcfg.c          FidoNet configurator source
pcbpscan/            File scanner source
installer/           pcbis_ui installer
docs/                Documentation
  fido/              FidoNet specs (FTS/FSP/FSC)
  reference/         PCBoard reference materials
patches/             Port patches (15.3→15.4, Borland→Watcom)
historical/          Preservation materials
PPL/                 PPE collection (2,757 files)
devtools/            PCBoard developer kits and tools
```

## The Crew

| Handle | Role |
|---|---|
| hexadecimal | Project lead, source port |
| verta1878 | netmodem2irc, OpenOLMS |
| wrench | FOSSIL driver, netmodem2irc engine |
| sysop/0 | fpc264irc, CIADraw/PabloDraw FPC port |
| kiddo/evga | Mystic/RIPscrip reference |
| evga | SIO OS/2 driver, VMODEM |

## License

Our additions: GPLv3
Clark Development source: proprietary (licensed)
CodeBase library: LGPL v3.0
SIO OS/2 driver: GPLv3 (evga, clean-room)

## Links

- Husky FidoNet Project: https://github.com/huskyproject
- PPLEngine (PPE decompiler): https://github.com/mkrueger/PPLEngine
- SyncTerm: https://gitlab.synchro.net/main/sbbs
