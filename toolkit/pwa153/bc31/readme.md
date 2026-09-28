# PCBKBC — pwa153 DOOR Developer's Toolkit SDK (Borland C++ 3.1)

Built 2026-09-17 from Clark's source under headless dosbox-x with Clark's
own BC 3.1. **152 modules × 4 memory models.** This is the door SDK —
the library a third-party door program links against.

    PCBKBCS.LIB   245,760 B   small
    PCBKBCC.LIB   258,560 B   compact
    PCBKBCM.LIB   252,928 B   medium     <- PCBoard itself uses medium
    PCBKBCL.LIB   265,216 B   large

All four hold the same 152 modules. Listings in the `.LST` files.
`CHECKSUMS.*` covers every library in this directory — the SDK and the
category libraries both.

Coverage against Clark's shipped `TOOLKIT/OTHER/QUICKREF`: **150 of 150**
documented door functions. 139 appear under their own names; the other 11
(`cdstillup`, `turnonrts`, `clearoutbuf`, …) are function pointers bound
at runtime and are present as `ASYNC_*` — all 25 ASYNC entry points are
in the library.

## Two different library sets live in this directory

Both are built from this toolkit tree with Borland C++ 3.1, which is why
they sit together. They are **not** interchangeable:

| Files | What they are | Built by |
|---|---|---|
| `PCBKBC{S,C,M,L}.LIB` | **the door SDK** — all 152 modules merged, one library per memory model, for third-party door developers | `MAIN/build/scripts/BLDKBC.BAT` |
| `COUNTRYL`, `DOS_L`, `DOSCLS_L`, `MISC_L`, `PCB_L`, `SCREEN_L`, `SCRNIO_L`, `SYSTEM_L`, `TOOLKITL` | **the category libraries** — split by source area, large model only, the link inputs for `PCBOARD.EXE` | Clark's per-directory MAKEFILEs via `MAIN/COMPILE.BAT` -> `153/PCBOARD.MAK`; recipe in `OUT/pwa153/BUILD-RECIPE.md` |

The category libraries moved here from `OUT/pwa153/lib/` on 2026-09-17.
They belong with the tree that produces them, and Clark kept his in
`LIB/BCDOS/BC31/` — inside the library tree, not an output directory.

Two things about them that cost hours when reconstructed instead of
read, both recorded in `OUT/pwa153/BUILD-RECIPE.md`:

1. `PCBOARD.MAK` links the 7 category libraries **directly** and does
   not use `pcbkit_l.lib` at all.
2. It also links some objects **straight from disk**, not from the
   `.LIB` — e.g. `..\lib\bcdos\bc31\misc\large.386\swap.obj`. The
   objects must exist at Clark's `OBJDIR` layout, not only inside the
   libraries. Those loose objects are **not** kept in the repo; the
   PCBOARD.EXE build regenerates them.

## What this is NOT

`pcbkit_l.lib` is **not** the SDK. It is the link target for PPLC and the
standalone utilities (including PCBSETUP), built **without** `-DLIB`, and
`docs/pcboard-internals/PCBKIT-LIB.md` already records it as stale. Using
its module list as the SDK manifest is what produced the failures below.

The 9 category libraries beside it are also not this — see the table
above.

## Verification — the sample doors in OUT/pwa153/bins/

Clark's own samples from `SOURCE/TOOLKIT/SAMPLES/`, compiled and linked
against this library. **6 of 7 link clean in medium and large**:

| Sample | What it demonstrates |
|---|---|
| HELLO | minimal door: `initdoor` / `println` / `closedoor` |
| CALLBACK | callback-driven door |
| COPYTEXT | text file to the message base |
| COPYBIN1 | binary file copy, simple |
| COPYBIN2 | binary file copy, buffered |
| UPGRADE | user upgrade / verification door |

Link line (response file — see the 127-byte trap below):

    c0<m>.obj + <sample>.OBJ + ALTMODEM + NODISP + PCBDAT + SMALLERR
    PCBKBC<M>.LIB + math<m>.lib + emu.lib + c<m>.lib

The override stubs link **before** the library, ALTMODEM first: it
supplies `_Status` and deliberately replaces INKEY, LOG and EXITDOS.

### Small and compact do not link a full door — by design, not a defect

Every sample fails small and compact with `Segment _TEXT exceeds 64K`.
Those models have a single 64K code segment, and the closure of
`initdoor` is larger than that on its own. The libraries are correct and
complete; they are for doors that use a small slice of the toolkit
(string, date, DOS helpers) rather than the full door runtime. Clark
shipped four models for exactly that reason. Do not "fix" this by
rebuilding — there is nothing to fix.

### SAMPINRQ (SAMPLES/INPUTREQ.C) — not shipped

Fails at link with `Undefined symbol _MAIN in module C0.ASM` in medium
and large. It is a *replacement-module* example — it reimplements the
toolkit's own `inputfieldreqstr` — so it is meant to be compiled into a
door, not linked standalone. Unresolved; the `.EXE` TLINK emits is
invalid and is deliberately **not** in `bins/`.

## Build config — three deliberate changes

`PCBKBC.CFG` is Clark's `CFG/BC31/ALL.RES` + `PCBOARD.CFG` with three
removals, plus `-DLIB`. Each was established by experiment:

| Dropped | Why |
|---|---|
| **`-Y`** | Overlay generation. Borland: *"Overlays only supported in medium, large, and huge memory models."* With `-Y`, small and compact produced **6/130** objects; without it, all of them. This — not far pointers — was the small/compact compile blocker. |
| **`-DPCBCOMM`** | Guards PCBoard-internal branches reading `Status.TerseMode`. Under `-DLIB`, `PCBOARD.H` selects the reduced door-visible `statustype`, which has no `TerseMode`. All four `-DPCBOARD`/`-DPCBCOMM` combinations were tested; dropping this one is necessary and sufficient. |
| **`/DCPU386`** | Only `BUG.H` reads it, to route `timerexpired()` through a `long BC386BUG` global that lives in `MAIN/SOURCE/MAIN/PCBOARD.C` — a file a door cannot link. Dropping it uses the direct form and makes the SDK self-contained. `-3` stays, so the code generated is still 386. |

Everything else is Clark's, unchanged — including `-P` (mandatory;
without it `TYPES.HPP` fails), `-V -Vmp -Vmd`, and `-D_FARDATA_=_FAR_`.
Do not drop `-D_FARDATA_=_FAR_`: it sets the struct layout a door shares
with `PCBOARD.EXE` at runtime.

## Manifest corrections

The inherited 119/130-module manifest came from `pcbkit_l.lib`. Three
kinds of error, all found by building and linking rather than reading:

**Dual-tree collisions** — same basename in both trees, wrong one named:

| Module | Manifest had | Correct for the SDK |
|---|---|---|
| CNAMES | `SOURCE/PCB/CNAMES.C` | `SOURCE/TOOLKIT/CNAMES.C` |
| HELP | `SOURCE/SCRNIO/HELP.C` | `SOURCE/TOOLKIT/HELP.C` |
| ANSI | `SOURCE/SCREEN/ANSI.C` | `pcb153/SOURCE/ASM/ANSI.ASM` |

ANSI is the instructive one. `pcb153/SOURCE/DISPLAY/ANSI.C` *looks*
right — it defines `agotoxy`, `asetcolor`, `awherex`, `awherey`,
`curcolor` — but it does not compile under `-DLIB`, and the linker asks
for those symbols in **uppercase** (`AWHEREX`). They come from
`ANSI.ASM`, assembled `tasm /m3` (uppercasing), not `/mx`. The same
casing split is recorded in `OUT/pwa153/BUILD-RECIPE.md` for PPLC.

**22 absent modules**, each traced from a linker "Undefined symbol":
BUILDSTR, CHANGE, DOSDUP, FASTPUTC, INDEX, TICDELAY, BOX, BOXCLS,
CLSBOX, PRNTCNTR, PRNTMOVE, TIMECHNG, WHEREX, DCOMMA, DOSFIND,
GETDRIVE, GETPATH, SETDRIVE, ISOPEN, SYSDATE, SYSTIME, BGETKEY, PRINTER.

**2 modules removed** — `SCRNIO/GETKEY.C` and `DOS/SHOWERR.C` are
PCBSETUP-side, not door-side. They were in `pcbkit_l` because PCBSETUP
links it. Both need host-supplied globals no door provides (`Colors[]`,
`_KBDSTATUS`, `_SHOWCLOCK`, `_UPDATEKBDSTATUS`), and the toolkit already
ships the door-side replacements: `SMALLERR.C` for the error handler,
`NOINPUT.C` for input. Removing them cleared four of the five externals
that were previously unresolved; dropping `/DCPU386` cleared the fifth.
**Nothing is unresolved now.**

**One duplicate symbol** — `ansicolors` is defined at file scope in both
`SCREEN/ANSI.C` and `TOOLKIT/INIT.C`; TLIB refuses the second. Made
`static` in `INIT.C` (used only locally there). The crew's toolkit tree
already carries this fix; the archive copy does not.

## Two sample-source fixes (build-enabling, documented)

- `SAMPLES/COPYBIN2.C:78` — `malloc` result assigned to `char *` without
  a cast. Legal C, rejected by C++, and the samples compile as C++ under
  `-P` to match the library's name decoration. Added `(char *)`.
- `SAMPLES/UPGRADE.C:17` — `#include "\tc\tools\pcbtools.h"`, a
  hardcoded path from Clark's own machine. Changed to `<pcbtools.h>`.

Both are the "hardcoded dev paths / casts for the C++ compiler" category
the project already recognises. Neither changes behaviour.

## Rebuilding

`scripts/BLDKBC.BAT` — 152 modules × 4 models, repo paths:

    C:\BC31              Borland C++ 3.1
    C:\TOOLKIT\PWA153    toolkit tree (SOURCE, H)
    C:\PCB153            PCBoard 15.3 source (SOURCE\H, SOURCE\ASM, ...)

Copy `PCBKBC.CFG` and `KBC?.RSP` to `C:\BC31\`, then run `BLDKBC.BAT`.
Output lands in `C:\TOOLKIT\PWA153\bc31\{OBJ,lib}\`. The script also
builds ALTMODEM, NODISP, PCBDAT, SMALLERR and NOINPUT as loose `.OBJ` —
they are **not** in the library and are linked selectively.

Three DOS traps this script already avoids, all of which cost time:

- **The 127-byte command line.** TLINK silently truncates past it, and
  the symptom is misleading: the **C runtime** appears undefined
  (`_exit`, `__stklen`) because the library field was never read. Use
  response files for TLINK as well as TLIB.
- **TLINK `/L` takes no space**: `/LC:\BC31\LIB`, not `/L C:\BC31\LIB`,
  or the path is linked as an object file.
- **8.3 truncation.** Writing `CALLBACKS.EXE` and `CALLBACKL.EXE` from
  one loop gives you `CALLBACK.EXE` twice. Put each model in its own
  directory instead of suffixing names.

## Not covered here

PCBKIT (Turbo C 2.01) and PCBKMS (Microsoft C 7.0) are separate compiler
families and are **not** rebuilt by this work. Any earlier claim that
they were "DONE, all 4 models" refers to the old manifest and the old
config, and should be re-run before it is trusted.
