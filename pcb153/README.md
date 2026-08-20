# PCBoard 15.3 Source — PWA Archive

Clark Development Company's PCBoard BBS source code, preserved by
PWA (Pirates with Attitude) from Corey Blake's licensed copy.

## Origin

Corey Blake purchased what may be the only PCBoard source code license
ever sold by Clark Development Company. Cost: over $2,000. The package
arrived on CD-ROMs but was missing two OBJ files (serial number control
and node license count). Clark was closed by the bank two days later.

A programmer later fixed the missing OBJs, and the source compiled.
PWA preserved and distributed the archive. Without them, this project
would not exist.

Original archive: `reference/pcb153src0014.zip` (password: PCB153)

## Versions

| Version | What | Directory |
|---|---|---|
| **15.3 PWA** | Corey Blake's source, patched to compile | `pcb153/` (this directory) |
| **15.4 Delta** | 15.3 + Clark's unreleased 15.4b changes applied | `pcb154/` |
| **15.41 IRC** | 15.4 + crew additions (FidoNet, TCP, RIP, etc) | `1541/` |

## Compiler

Borland C++ 3.1 (1992). Build tools in `PCB153BT.ZIP` at repo root.

Run under DOSBox-X with `DOSBOX.CFG`:

```
cd pcb153
call bcdos.bat
compile.bat
```

Output: `obj\bc31\PCBOARDM.EXE`

## Source Tree

```
pcb153/
  SOURCE/
    MAIN/       PCBoard main — PCBOARD.C, CALLWAIT.C, INKEY.C, etc
    DISPLAY/    screen output, XLATE.C (@-code translator)
    MODEM/      serial I/O — MODEMFOS.C (FOSSIL), MODEMDRV.C (COMM-DRV)
    PPL/        PPL 3.40 compiler and executor
    UTIL/       PCBSM, PCBSETUP, PCBTEXT, MKPCBTXT
    MISC/       USERNET, MD5, ZMODEM
    MKPCBSRC/   source code builder
    H/          headers — PROJECT.H, PCBOARD.H, TYPES.HPP
  153/          build config — PCBOARD.MAK, PCBOARD.CFG, IDEINIT.CFG
  OBJ/          compiled objects
  BCDOS.BAT     sets up Borland C++ 3.1 environment
  COMPILE.BAT   runs MAKE with correct defines
```

## Shared Libraries

Both 15.3 and 15.4 link against the same toolkit library source:

| Directory | What |
|---|---|
| `toolkit/` | Clark's shared toolkit — H/, SOURCE/ (273 files across MISC, DOS, SCREEN, PCB, TOOLKIT, SYSTEM, COUNTRY, SCRNIO) |
| `pcbcbase/` | Third-party: CODEBASE (dBase, LGPL) + prebuilt BC31 .LIB files |

The toolkit compiles into `PCBKIT_L.LIB` (large model). Prebuilt copy
at `pcbcbase/PREBUILT/BC31/PCBKIT_L.LIB` (236 KB, Borland C++ 3.1).

## Build Defines

From `COMPILE.BAT`:

| Define | What |
|---|---|
| `-DCOMM` | serial communication enabled |
| `-DSTATS` | statistics tracking |
| `-DMP` | multiport support |
| `-D386` | 386 optimizations |
| `-DDBASE` | dBase/CODEBASE file access |
| `-DFIDO` | FidoNet support |

Optional: `-DCOMMDRV` (WCSC COMM-DRV instead of FOSSIL), `-DDEBUG`, `-DTD`

## Compile Status

sysop/0 reports 138/295 files compiling (47%) as of 2026-08-05.
Remaining 157 failures categorized in `todo/hexadecimal-phase-update.md`:

- constream.h stubs needed
- YESNO/DOSFILE header clashes
- per-binary include paths
- Linux case-sensitivity (fixed with normalize_case.sh)

## License

Clark Development Company proprietary (licensed copy).
Our additions: GPLv3.
