# PCBoard Source Code — Master Guide

## Three Versions, One Tree

| Version | Directory | Compiler | What |
|---|---|---|---|
| **15.3 PWA** | `pcb153/` | Borland C++ 3.1 | Corey Blake's licensed source, preserved by PWA |
| **15.4 Delta** | `pcb154/` | OpenWatcom 16-bit | 15.3 + Clark's unreleased 15.4b changes applied |
| **15.41 IRC** | `pcbpcb1541/` | OpenWatcom 16-bit | 15.4 + crew additions (FidoNet, TCP, RIP, etc) |

**PWA** = Pirates with Attitude, who preserved the only source license
ever sold by Clark Development Company.

**Delta** = the differences between 15.3 and 15.4b, Clark's final
unreleased beta. Applied as patches to the PWA base.

**IRC** = our work. New programs, restored missing binaries, and the
port from Borland to OpenWatcom.

## Toolkit — Shared Library

Both 15.3 and 15.4 link against a shared toolkit library. It builds
separately and produces `PCBKIT_L.LIB` (236 KB, large model).

```
toolkit/
  153pwa/       Clark's original toolkit source (from PWA archive)
  154delta/     15.4 patched version (sysop/0's additions: VMAVL, VMDATA, INT24HND)
  1541irc/      our new toolkit code (as it's written)
  devtools/     Pcbic12.zip (encrypted, password unknown)
```

273 source files across 8 areas: MISC (90), DOS (47), SCREEN (41),
TOOLKIT (41), SCRNIO (20), PCB (22), COUNTRY (13), SYSTEM (7).

15.4 delta adds: `VMAVL.C`, `VMDATA.H`, `INT24HND.C`, plus changes to
`DOSFIND.C`, `CNAMES.C`, `INPUTSTR.C`, `NEWDATA.H`, `WATCOMPAT.H`.

Prebuilt BC31 libraries at `pcbcbase/PREBUILT/BC31/`.

## Third-Party Dependencies

```
pcbcbase/
  CODEBASE/     Sequiter Software dBase library (LGPL v3.0)
  PREBUILT/
    BC31/       prebuilt .LIB files (Borland C++ 3.1, large model)
```

CODEBASE provides dBase III/IV file access used by PCBoard's
file directory system and PPL DBASE functions.

## Build Root — DOSBox-X + Borland C++ 3.1

PCBoard 15.3 requires Borland C++ 3.1, a 1992 DOS compiler.
Everything needed to compile is in the repo:

| File | What |
|---|---|
| `PCB153BT.ZIP` | Borland C++ 3.1 — BCC.EXE, TLINK.EXE, TLIB.EXE, MAKE.EXE, TASM.EXE, headers, libraries |
| `DOSBOX.CFG` | DOSBox-X configuration — mounts repo, sets BC31 paths |
| `GETDOSBX.BAT` | Where to download DOSBox-X for each platform |

### Setup

1. Install DOSBox-X (`apt install dosbox-x` / `brew install dosbox-x` / GitHub releases)
2. Extract `PCB153BT.ZIP` to `BC31\` in the repo root
3. Run `dosbox-x -conf DOSBOX.CFG`

### Compile 15.3

```
C:\> cd pcb153
C:\pcb153> call bcdos.bat
C:\pcb153> compile.bat
```

Output: `pcb153\obj\bc31\PCBOARDM.EXE`

### Compile 15.4

OpenWatcom runs natively on Linux — no DOSBox needed for compiling.
DOSBox-X is only needed to test the resulting DOS executables.

```bash
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl64:$PATH
wcc -zq -w1 -ml -za99 \
  -fi=toolkit/H/WATCOMPAT.H \
  -Itoolkit/H -Ipcb154/MAIN/SOURCE/H -I$WATCOM/h \
  -dCOMM -dSTATS -dMP -d386 -dDBASE -dFIDO \
  -fo=obj/filename.obj \
  pcb154/MAIN/SOURCE/MAIN/PCBOARD.C
```

WATCOMPAT.H bridges Borland-to-Watcom differences. 76% of the
toolkit compiles clean; three header fixes reach 90%.

## OS/2 Support

| Directory | What |
|---|---|
| `OS2TK/` | OS/2 toolkit headers (332 files) |
| `pcbpcb1541/pcbcp/` | Clark's OS/2 PM control program source (1996, 8 C files) |
| `BUILD_OS2.CMD` | OS/2 build script |
| `BUILD_OS2_OW.SH` | OpenWatcom cross-compile for OS/2 |

## Drivers

| Directory | What |
|---|---|
| `drivers/netfosdl/` | wrench's DOS FOSSIL driver (GPLv3, verified working in DOSBox-X) |

## Programs in pcbpcb1541/

Our 15.41 work — new programs, restored binaries, and ports:

| Directory | What |
|---|---|
| `qfront/` | QFront mailer (Phase 15, style-audited) |
| `pcbmail/` | PCBoard Mail scaffold (Phase 27) |
| `install/` | INSTALL.EXE scaffold (Phase 27) |
| `pcbic/` | PCBIC scaffold (Phase 27) |
| `pcbcomm/` | unified serial layer design |
| `pcbcp/` | Clark's OS/2 PM control program |
| `binkd/` | BinkD FidoNet mailer |
| `pcbdraw/` | ANSI editor |
| `PPL/` | PPL engine |
| `wip/` | Pascal from mysticbbsirc, being ported to C |

## Build Status

| Target | Compiler | Status |
|---|---|---|
| 15.3 DOS (Borland) | BC31 | 138/295 (47%) — sysop/0 |
| 15.4 Toolkit (Watcom 16-bit) | wcc | 208/273 (76%) — hexadecimal |
| 15.4 Main (Watcom 16-bit) | wcc | blocked on 3 header fixes |

## The Crew

| Handle | Role |
|---|---|
| verta1878 | project lead |
| sysop/0 | compiler, codecs, utilities |
| hexadecimal | PCBoard port, Phase 0, docs |
| wrench | FOSSIL, networking |
| kiddo | protocols, RIP engines, serial |
| evga | SIO driver, display, multiport |

## Documentation

All docs in `todo/` pending review and merge. Master plan:
`todo/PCB1541-PHASES.md` (~1,270 lines, 32+ phases).

## License

Clark Development source: proprietary (licensed copy)
Our additions: GPLv3
CODEBASE: LGPL v3.0
