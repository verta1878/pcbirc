# PWA 15.3 — reproducible build from source

Built 2026-09-16. This is the recipe, start to finish. The previous library
chain (pcbsrc v0.1, "arc CLOSED 2026-09-07") produced output that was never
committed and is gone; this file exists so that does not happen twice.

## Result

    PCBOARDM.EXE   961,504 B   PCBoard (R) v15.3/M 25
                               Copyright (C) 1996 Clark Development Company

    9 category libraries, 252 library objects, 113 MAIN objects.
    Linked clean, no errors.

SHA256 sums in `CHECKSUMS.sha256`.

## Read this first

**Use Clark's own build files. Do not reconstruct them.**

`MAIN/COMPILE.BAT` → `153/PCBOARD.MAK` is the real build. Two things that
cost hours when reconstructed by hand instead of read:

1. `PCBOARD.MAK` links the **7 category libraries directly** —
   countryl, dos_l, screen_l, pcb_l, misc_l, system_l, doscls_l (as `.386`)
   plus emu/mathl/overlay/cl and CodeBase. It does **not** use
   `pcbkit_l.lib` at all. `pcbkit_l` is only for PPLC and the standalone
   utilities.
2. It also links some library objects **straight from disk**, not from the
   `.LIB` — e.g. `..\lib\bcdos\bc31\misc\large.386\swap.obj`. The objects
   must exist at Clark's `OBJDIR` layout, not only inside the libraries.

## Source

`reference/pcb153src0014.zip` → `PCBoard 15.3 source code v0.014.zip`.
Contains everything needed:

    B/C31/                       Clark's own Borland C++ 3.1
    PCBSRCV/014/LIB/SOURCE       9 category library source dirs
    PCBSRCV/014/LIB/H            library headers
    PCBSRCV/014/LIB/CFG/BC31     Clark's compiler config (see below)
    PCBSRCV/014/MAIN/SOURCE      PCBoard main source
    LIB/CODEBASE                 CodeBase dBase library

The repo's `pcb153/SOURCE` has **no LIB/ tree**, so the archive is the build
source, not the repo copy.

## Environment

Ubuntu package `dosbox-x` (2024.03.01), headless:

    SDL_VIDEODRIVER=dummy dosbox-x -conf <conf>

DOS output is invisible, so every command redirects to a file on the mounted
drive and is read back from the host:

    bcc ... >> C:\BUILD.LOG

No DOSBOXX.ZIP transfer is needed — the archive carries the compiler.

## Drive layout

Clark's configs use relative include paths that only resolve from his
directory shape. His root was `\PROJ\PCB\` (see `MAIN/G.BAT`). This works:

    C:\B\C31\                    Borland C++ 3.1
    C:\PCB\LIB\{SOURCE,H,CFG}    library tree
    C:\PCB\LIB\BCDOS\BC31\       library output (.LIB, .386)
    C:\PCB\MAIN\{SOURCE,153}     main tree
    C:\LIB\CODEBASE\             CodeBase  (LIBSDIR=..\..\LIB from MAIN)

## Step 1 — category libraries

Per-directory, using **Clark's own config files**, not a transcription:

    cd \PCB\LIB\SOURCE\<DIR>
    bcc +..\..\cfg\bc31\pcboard.cfg @..\..\cfg\bc31\all.res <file>.C
    tasm /mx /d__l__ <file>.ASM, OBJ\bc31\<file>.OBJ
    tlib <LIBDIR>\<lib> + OBJ\bc31\<module>      (module list from MAKEFILE)

`ALL.RES` is:

    -c -P -Od -V -Vmp -Vmd -ff -3 /DCPU386 -ml -DNDEBUG -DCOMM -DPCBSTATS
    -DPCB_MAXNODES=25 -DMULTIPORT -DKEY=KEY_25 -D_FARDATA_=_FAR_

`PCBOARD.CFG` adds `-K -Y -d -f`, ~50 warning switches, and
`-DPCBOARD -DPCBCOMM -DOSDRIVER -DFOSSIL -DBIGNDX -D___USE_VAR___
-DS4ERROR_HOOK -DNDEBUG -DPCB152 -DPCB153 -DCRYPT -DMG -DTOSSCLASS`.

Note `-V -Vmp -Vmd` (vtable control) and `-P`. **`-P` is mandatory** —
without it `TYPES.HPP` fails, because Clark compiles all `.C` as C++.

Take each library's module list from its own `MAKEFILE` `tlib` lines.
`SCRNIO` has no MAKEFILE — all 20 modules go in.

Result: 139/139 required modules. Files that fail to compile
(`TEST.C`, OS/2 variants like `kbd.cpp`/`session.cpp`/`threads.c`,
`init.c`, alternates) are **not referenced by any MAKEFILE** — verify
per-module rather than trusting a pass/fail count.

## Step 2 — stage library objects for the linker

    C:\PCB\LIB\BCDOS\BC31\<subdir>\large.386\*.OBJ

`PCBOARD.MAK` reads objects from here directly. Copy them from each
`LIB/SOURCE/<DIR>/OBJ/BC31/`, or build with `-n$(OBJDIR)` as the MAKEFILEs do.

## Step 3 — PCBOARD

    set BC31PATH=C:\B\C31
    cd \PCB\MAIN
    call COMPILE.BAT

`COMPILE.BAT` sets `-DCOMM -DSTATS -DMP -D386 -DDBASE -DFIDO`, `NODES=25`,
calls `BCDOS.BAT` (which needs `%BC31PATH%`), then runs
`MAKE -f153\pcboard.mak`. Errors land in `MAIN\ERRORS`.

Wall clock: ~48 s for the full MAIN build under dosbox-x.

## Open

- **PPLC** does not link yet. `PPLC.MAK` needs `..\pcbkit_l.lib`, which is
  **not** a merge of the 8 category libs — Clark's own copy has 146 OMF
  modules: category modules *plus* ~40 `MAIN/SOURCE` modules. Toolkit
  modules (ALTMODEM, NODISP, PCBDAT) must be added **before** the MAIN
  modules, since ALTMODEM supplies `_Status` and deliberately replaces
  INKEY, LOG and EXITDOS.
- ASM flags differ by tree: library ASM uses `tasm /mx` (case-preserving),
  `PPLC.MAK` uses `/m3` (uppercasing). Built with `/mx`, `ANSI.ASM` exports
  `_Scrn_Addr` and nothing links against it.
- Remaining PPLC undefined symbols: `_SCRN_ADDR`, `_SCRN_RTRC`,
  `_TURNONRTS`, `_TURNONDTR`, `_CLEAROUTBUF`, `loguseroff`.
- `PCBOARD.EXE` (single-node) not yet built — only `PCBOARDM.EXE`
  (multinode). Same makefile, different target.
