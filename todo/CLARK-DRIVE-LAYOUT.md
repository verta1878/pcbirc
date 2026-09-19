# Clark's drive layout — what `\\PROJ`-> `\\OUT`, `\\BC31`, `\\TA`, `\\LIBS` are

Short answer: **they are Clark Development's own development machine's
drive roots, hardcoded into the source.** They are not repo directories —
they are the shape a DOS build host has to present. This applies to all
three compiler families, because the paths live in the makefiles and in
`BCDOS.BAT`, not in any compiler.

## They belong in the build root — which already exists

`DOSBOXX.ZIP` -> `pcbirc/BUILDROOT/` **is** the build root. `PCBBLDBT.CONF`
mounts it as `C:`:

&#x20;   mount c pcbirc/BUILDROOT
    SET PATH=C:\\BC31\\BIN;%PATH%


It already carries most of what is needed:

|In BUILDROOT|Holds|
|-|-|
|`BC31\\`|Borland C++ 3.1 — `BIN\\`, `INCLUDE\\`, `LIB\\`|
|`TC201\\`, `MSC70\\`, `BCOS2\\`|the other compiler families|
|`PCB153\\`|the 15.3 source tree + Clark's `BCDOS.BAT`, `COMPILE.BAT`|
|`TOOLKIT\\PWA153\\`|the toolkit tree|
|`OUT\\`, `BUILD\\SCRIPTS\\`|output and the build scripts|
|`386MAX\_S\\`, `D32A\\`, `HX\\`, `CWSDPMI\\`|DPMI hosts|

So `BLDKBC.BAT`'s `C:\\BC31`, `C:\\TOOLKIT\\PWA153`, `C:\\PCB153` are all
real — that is the crew's own layout, and it works.

## What the build root can actually build (checked 2026-09-17)

`DOSBOXX.ZIP` was opened and enumerated rather than assumed. Two gaps
matter.

**1. `PCB153\\153\\` is not in it at all.** The repo has
`pcb153/153/` with `PCBOARD.MAK`, `PCBOARD2.MAK`, `PPLC.MAK`,
`PCBOARD.CFG`, and the `.RES`/`.386` link-response files. `BUILDROOT/PCB153/`
has the batch drivers — `COMPILE.BAT`, `BCDOS.BAT`, `ASMCOMP.BAT`,
`TKCOMP.BAT`, `PPLC.BAT` — but **none of the makefiles they invoke**, and
only `CFG\\PPLC.CFG` under `CFG\\`. So as packaged, the build root cannot
build PCBOARD.EXE, PCBOARDM.EXE, PCBOARD2.EXE or PPLC.EXE. `COMPILE.BAT`
runs and fails. The binaries in `OUT/pwa153/` were produced from a
separately staged tree, not from this zip.

**2. It carries 11 utility makefiles, no more.** Every `.MAK` under
`BUILDROOT/PCB153/`:

&#x20;   SOURCE\\UTIL\\PCBSETUP\\PCBSETUP.MAK     SOURCE\\MISC\\FIDOUTIL\\FIDOUTIL.MAK
    SOURCE\\UTIL\\PCBSM\\PCBSM.MAK           SOURCE\\MISC\\IDX\\MAKEIDX.MAK
    SOURCE\\UTIL\\PCBTEXT\\MKPCBTXT.MAK      SOURCE\\MISC\\USERNET\\USERNET.MAK
    SOURCE\\UUCP\\UUIN\\UUIN.MAK             SOURCE\\MISC\\USERNET\\USERNET2.MAK
    SOURCE\\UUCP\\UUOUT\\UUOUT.MAK           SOURCE\\UUCP\\UUUTIL\\UUUTIL.MAK
    SOURCE\\UUCP\\UUXFER\\UUXFER.MAK


`SOURCE\\MISC\\HELP\\` also has `MAKEHELP.C` and its own `COMPILE.BAT`
but no `.MAK` — it compiles directly.

This is not a defect in the zip so much as a consequence of how it was
made: `BUILDROOT/PCB153/` is a **copy of the repo's `pcb153/SOURCE`**,
and the repo tree only ever carried `UTIL\\{PCBSETUP,PCBSM,PCBTEXT}` and
`MISC\\{FIDOUTIL,HELP,IDX,USERNET}` plus `UUCP`. Nothing that was never
imported into the repo can appear in the build root.

### The thirteen utilities that exist but were never imported

These have **complete source and Clark's own makefiles** in the archive
at `reference/pcb153src0014.zip` -> `PCBSRCV/000/`, and are in neither
the repo nor the build root:

|||
|-|-|
|`UTIL/PCBCP`|`UTIL/PCBMONI` (+ `PCBMONI2`)|
|`UTIL/PCBDIAG`|`UTIL/PCBNLC`|
|`UTIL/PCBEDIT`|`UTIL/PCBPACK` (+ `PCBPACK2`)|
|`UTIL/PCBFILER`|`UTIL/PCBSTATS`|
|`UTIL/PCBMODEM` (+ `MSETUP`)|`MISC/WAITFILE`, `MISC/ZMODEM`|

Importing them is a copy, not a recovery. Until then the build root's
ceiling is 11 utilities, not 24.

### The thirteen with no source anywhere

`PCBDESC`, `DOORWAY`, `ENCRYPT`, `FIXTEXT`, `INIT`, `MKPCBMNU`,
`OVLSIZE`, `PACKFIDO`, `RDPCBTXT`, `TESTFILE`, `UPGRADE`, `VIEWARCH.COM`,
`VIEWZIP`. No makefile in `PCBSRCV/000/` and no source in the repo.
`DOORWAY` was never Clark's — it is Marshall Dudley's third-party door
driver, shipped under licence. `PACKFIDO` is the documented case: the
`.DSK` desktop files place its source at `E:\\TC\\PACKFIDO\\PACKFIDO.C`,
a developer's Turbo C scratch drive that was never in the archive's
scope. See `pcb153/SOURCE/MISC/FIDOUTIL/PACKFIDO-MISSING.md`.

So the three numbers are: **37 shipped**, **24 have a makefile
somewhere**, **11 are in the build root**, **2 are built**. Earlier
drafts conflated them.

## What is missing from it, and the one conflict

The **utility** makefiles under `PCBSRCV/000/` are a different matter.
They are Clark's, untouched, and they want three roots BUILDROOT does not
have:

|Root|Needed for|Status in BUILDROOT|
|-|-|-|
|`\\PROJ`|every utility `.MAK` (`ROOT = \\PROJ`)|**absent**|
|`\\TA`|`BCDOS.BAT`'s `ASMROOT=\\TA` -> `\\TA\\TASM.EXE`|**absent** (TASM is in `BC31\\BIN\\`)|
|`\\LIBS`|`LIBSDIR=\\LIBS` — CodeBase, COMMDRV, VMData|**absent**|

And there is a **direct conflict** worth knowing before you add them:

* BUILDROOT puts the Borland binaries in `BC31\\BIN\\`, and sets
`PATH=C:\\BC31\\BIN`.
* Clark's `BCDOS.BAT` bc31 branch sets `BCPGM=%BCROOT%` — **no `\\BIN`** —
so it looks for `C:\\BC31\\MAKE.EXE` and fails with
`Bad command or filename`.

Two ways out: add root-level copies of the Borland binaries under
`\\BC31`, or patch `BCDOS.BAT`'s bc31 branch to `BCPGM=%BCROOT%\\BIN` like
its tc30 and bc50 branches already do. The second is one line and matches
the rest of the file, but it edits Clark's script — worth a deliberate
decision rather than a silent fix.

Written down 2026-09-17 after rediscovering it the hard way. The build
root existed; what was missing was any document saying what it contained
or which roots Clark's own utility makefiles additionally expect. The
makefiles fail with misleading errors until those exist.

## Where each one is declared (the source side)

|Root|Declared in|Used for|
|-|-|-|
|`\\PROJ` -> '\\OUT'|every utility `.MAK`: `ROOT = \\OUT`|the project root — `$(ROOT)\\LIB`, `$(ROOT)\\PCB\\SOURCE\\H`, `$(ROOT)\\PCBSM\\SOURCE`, `$(ROOT)\\PCBSETUP\\SOURCE`|
|`\\BC31`|`MISC\\BCDOS.BAT`: `BCROOT=\\BC31`|Borland C++ 3.1|
|`\\TA`|`MISC\\BCDOS.BAT`: `ASMROOT=\\TA`|Turbo Assembler — `\\TA\\TASM.EXE`|
|`\\LIBS`|`MISC\\BCDOS.BAT`: `LIBSDIR=\\LIBS`|CodeBase, and `\\LIBS\\VMDATA\\BC31\_DOS\\VMDATA.LIB`|
|`\\TC30`|`MISC\\BCDOS.BAT`: `BCROOT=\\TC30`|Turbo C 3.0 branch|
|`\\BC5`|`MISC\\BCDOS.BAT`: `BCROOT=\\BC5`|Borland C++ 5.0 branch|

`MAIN\\G.BAT` shows his PCBoard tree sat at `\\PROJ\\PCB\\`.

## No symlinks — copies only

**This project does not use symlinks.** The repo lives on Windows, git's
symlink support there is unreliable, and DOSBox-X does not resolve host
*directory* symlinks consistently anyway — a symlinked `C:\\BC31\\INCLUDE`
gives `Unable to open include file 'stdio.h'` with a path that looks
perfectly correct.

That is not a limitation to work around; it is how BUILDROOT already
works. `pcbirc/BUILDROOT/` holds real copies — `PCB153\\` (643 files) and
`TOOLKIT\\PWA153\\` (414 files) are copies of the repo trees, not links.
The layout below follows the same convention.

## Model note

The category libraries are referenced per memory model: `DOS\_L.LIB`,
`DOS\_S.LIB`, `DOS\_C.LIB`. The repo only has the **large** set, so the
small-model utilities fail with:

&#x20;   Fatal: '\\proj\\lib\\bcdos\\bc31\\dos\_s.lib' does not exist


Model per target (from each `.MAK`'s `MDL =` line):

* **large** — FIDOUTIL, MAKEIDX, UUIN, UUOUT, UUUTIL, UUXFER, ZMODEM,
PCBDIAG, PCBEDIT, PCBFILER, MSETUP, PCBMODEM, PCBNLC, PCBPACK,
PCBSETUP, PCBSM
* **small** — USERNET, WAITFILE, PCBMONI, PCBSTATS, MKPCBTXT
* **compact** — the PCBPACK backup variant

Some makefiles also want objects at `<subdir>\\large\\` where the 15.3
build writes `<subdir>\\large.386\\`.

## VMData — source found, library not yet built

`\\LIBS\\VMDATA\\BC31\_DOS\\VMDATA.LIB` and `vmdata.h` — the virtual memory
library. It blocks PCBSETUP, PCBSM, PCBFILER, PCBNLC, FIDOUTIL, MAKEIDX
and UUIN — everything that touches the Fido/message subsystem. PCBDIAG
and ZMODEM compile all modules clean without it.

An earlier version of this file said VMData was "missing everywhere."
**That was wrong.** The source was in `pcb154/LIB/`, and it is not
Clark's — it is the crew's own clean-room work:

|File|Author|
|-|-|
|`VMAVL.C`, `vmavl.h`|sysop/0, GPLv3 clean-room|
|`VMFUNCS.C`|hexadecimal v0.036|

Copied into `toolkit/pwa153/{SOURCE,H}` 2026-09-17 with zero Watcom
markers, so the Borland build can use it unmodified. What remains is
building `VMDATA.LIB` from it and dropping that at
`\\LIBS\\VMDATA\\BC31\_DOS\\`.

Clark's own VMData — the one `PCBICEVT` referenced as `c:\\vmdata\\src\\` —
is still absent, and may never have left his machine. The clean-room
replacement is what this project builds against.

## Where the utility source actually is

Not in `pcb153/SOURCE` — that has only `UTIL/{PCBSETUP,PCBSM,PCBTEXT}`.
The full set is in the archive, `reference/pcb153src0014.zip`, under
**`PCBSRCV/000/`**:

&#x20;   PCBSRCV/000/UTIL/   PCBCP PCBDIAG PCBEDIT PCBFILER PCBMODEM
                        PCBMONI PCBNLC PCBPACK PCBSETUP PCBSM
                        PCBSTATS PCBTEXT PCBUTILS
    PCBSRCV/000/MISC/   FIDOUTIL HELP IDX MD5 USERNET UUCP
                        WAITFILE ZMODEM


\~25 makefile targets with Clark's own `.MAK` files. The repo's `pcb153`
tree is missing most of this — worth importing regardless of whether
anything gets built.

## What `\\LIBS` actually contains

Three third-party libraries, from the makefile references:

|Path|What|Where it is|
|-|-|-|
|`\\LIBS\\CODEBASE\\BOR31\\C4BASE.LIB`|CodeBase dBase engine|repo root `pcbcbase/`, and `LIB/CODEBASE` in the archive|
|`\\LIBS\\COMMDRV\\LIB\\COMMDRBL.LIB`, `LIBSBL.LIB`, `\\LIBS\\COMMDRV\\H`|MicroSystems COMMDRV serial library|`PCBDCOM\\` in BUILDROOT; the decompile is a separate repo|
|`\\LIBS\\VMDATA\\BC31\_DOS\\VMDATA.LIB` + `vmdata.h`|virtual memory library|source in `toolkit/pwa153/{SOURCE,H}` (the crew's clean-room version); **the .LIB is not built yet**|

## Recreating the layout

`PCBSRCV/000/MISC/BCDOS.BAT` is the environment script. Call it with
`BCROOT` and `ASMROOT` unset — it early-outs if either is already set.

Everything below is a **copy**, staged into `pcbirc/BUILDROOT/` before
packaging. Source on the left, destination on the right:

|Copy from|To|
|-|-|
|`toolkit/pwa153/{SOURCE,H,CFG}`|`\\OUT\\LIB\\{SOURCE,H,CFG}`|
|`toolkit/pwa153/bc31/lib/\*.LIB`|`\\OUT\\LIB\\BCDOS\\BC31\\`|
|`pcb153/SOURCE`|`\\OUT\\PCB\\SOURCE`|
|`PCBSRCV/000/UTIL/PCBSM`|`\\OUT\\PCBSM`|
|`PCBSRCV/000/UTIL/PCBSETUP`|`\\OUT\\PCBSETUP`|
|`PCBSRCV/000/UTIL/<x>`|`\\OUT\\UTIL\\<x>`|
|`PCBSRCV/000/MISC/<x>`|`\\OUT\\MISC\\<x>`|
|`BC31\\BIN\\TASM.EXE`|`\\TA\\TASM.EXE`|
|`pcbcbase/CODEBASE`|`\\LIBS\\CODEBASE`|
|COMMDRV libs + headers|`\\LIBS\\COMMDRV\\{LIB,H}`|
|*(missing)*|`\\LIBS\\VMDATA\\BC31\_DOS\\`|

Two things that bite:

* **Delete any stale per-utility `.CFG` first.** The makefiles regenerate
them; the committed ones carry Clark's own machine paths.
* **`\\PROJ\\LIB` must hold the category libraries per memory model**
(`DOS\_L.LIB`, `DOS\_S.LIB`, `DOS\_C.LIB`). Only the large set exists —
see the model note above.

## Watcom needs none of this

OpenWatcom 2.0 has a **Linux-native cross-compiler** that emits 16-bit
DOS. No DOSBox, no drive letters:

&#x20;   WATCOM=/opt/watcom  PATH=$WATCOM/binl64:$PATH  INCLUDE=$WATCOM/h
    wcc -bt=dos -ml -fo=out.obj src.c
    wlink system dos file out.obj name out.exe    -> MS-DOS MZ executable


Verified 2026-09-17. `MAIN/build/scripts/normalize-case.sh` exists for
exactly this — it makes lowercase header copies so the Linux build
resolves `#include <vmdata.h>` against `VMDATA.H`. It supersedes
`pcb154/normalize\_case.sh`, which covered three directories in `pcb154`
and no toolkit tree at all; the new one covers all four branches plus
the program trees, has a `--check` dry run, and exits 2 on a
case-clash where two files differ only in filename case *and* content.

**The copies it makes are build output — do not commit them.** Run it
against a staged build root, not the repo you commit from.

## Two makefile references with nothing behind them

|Reference|In|Status|
|-|-|-|
|`$(ROOT)\\md5\\os2\\md5.obj`|`153/PCBOARD2.MAK`|**resolved** 2026-09-17. The object was recovered from `devtools/Md5.zip` and now lives at `pcb153/SOURCE/MISC/MD5/OS2/MD5.OBJ` (1,492 B, valid OMF, `THEADR md5.asm`) with `MD5.ASM` beside it. `PCBOARD2.MAK` repointed.|
|`packfido.c`|`SOURCE/MISC/FIDOUTIL/FIDOUTIL.MAK`|**absent.** The string `packfido` appears in exactly one place in the archive and the repo: that makefile. Clark's `$(ROOT)\\packfido\\packfido.c` was a top-level directory beside `\\PROJ\\md5\\`; repointed to `$(ROOT)\\source\\packfido.c` so a recovered file drops in, but the source itself is gone. See `pcb153/SOURCE/MISC/FIDOUTIL/PACKFIDO-MISSING.md`. need to be made from the fido ftsc.|



