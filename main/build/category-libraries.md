# The nine PCBoard category libraries — how they are built now

Rebuilt from source 2026-09-22. Before this, the only copies were nine
prebuilt `.386` files of unknown provenance, now in
`attic/prebuilt-libs/BC31/`. They are still there; nothing was deleted.

## What they are

`toolkit/pwa153/SOURCE/<CATEGORY>/` each build one library:

| Folder | Library | Modules | Source |
|---|---|---:|---|
| `COUNTRY` | `COUNTRYL.LIB` | 3 | country-specific formatting |
| `DOS` | `DOS_L.LIB` | 45 | checked DOS file I/O |
| `DOSCLS` | `DOSCLS_L.LIB` | 1 | the `dosclass` C++ wrapper |
| `MISC` | `MISC_L.LIB` | 85 | strings, dates, search, virtual memory |
| `PCB` | `PCB_L.LIB` | 22 | PCBOARD.DAT and config readers |
| `SCREEN` | `SCREEN_L.LIB` | 39 | direct-video screen primitives |
| `SCRNIO` | `SCRNIO_L.LIB` | 20 | the full-screen input/menu layer |
| `SYSTEM` | `SYSTEM_L.LIB` | 3 | date, time, BIOS keyboard |
| `TOOLKIT` | `TOOLKITL.LIB` | 3 | door-toolkit entry points |

Output goes to `OUT\PWA153\SDK\BC31\LIB`, objects to
`OUT\PWA153\SDK\BC31\OBJ\<CATEGORY>\LARGE`.

## Building them

From the repo root, with the repo mounted as the drive root:

    BLDTK                 all nine
    BLDTK MISC            one category
    BLDTK CLEAN           delete the objects and the libraries

`BLDTK.BAT` finds the compiler in `%BC31PATH%`, then `\BC31`, then
`\B\C31`, and refuses to run if `\APPLY.txt` is not in the drive root.
A single `MAKE -fTKLIB.MAK` also works, from inside a category folder.

The new makefiles are called `TKLIB.MAK`, not `MAKEFILE`. Clark's
original `MAKEFILE` in each folder is **left exactly as it was** — it is
not used, and it still points at `..\..\bcdos\bc31`, a folder that does
not exist, so a bare `MAKE` in one of these folders fails loudly rather
than doing something unexpected.

Every macro in the MAKEFILEs is guarded, so they can be steered without
editing: `make -DCVER=BC50`, `make -DROOT=\OUT -DBRANCH=PWA154`,
`make -DMODEL=SMALL`.

## Verifying a rebuild

Each OMF library records the name of every module inside it (the THEADR
record), so a rebuild can be checked against Clark's shipped libraries
without having to trust the build log. The module lists are in
`attic/prebuilt-libs/BC31/README.md`. As of this build:

    COUNTRYL   3 modules   matches Clark
    DOSCLS_L   1           matches Clark
    DOS_L     45           Clark's 44 plus INT24HND
    MISC_L    85           matches Clark
    PCB_L     22           matches Clark
    SCREEN_L  39           matches Clark
    SCRNIO_L  20           matches Clark
    SYSTEM_L   4           Clark's 3 plus KBDSTAT
    TOOLKITL   3           matches Clark

`INT24HND` is in Clark's own `SOURCE\DOS\MAKEFILE` but missing from the
`.386` we were handed, so the copy we had was short of it.

`KBDSTAT` is not Clark's file. It supplies the one global that
`BGETKEY.C` declares `extern` and nothing in the tree defined — see
"The `_KbdStatus` gap" below.

Sizes do not match Clark's byte-for-byte and are not expected to: these
are a fresh compile, and the originals were built on a machine we know
nothing about.

## The compiler settings: CFG\BC31\TK.CFG

Clark drove the toolkit with two files, `CFG\BC31\PCBOARD.CFG` (warnings,
defines, paths) and `CFG\BC31\ALL.RES` (the response file with the code
generation switches). Putting both on one BCC command line together with
an absolute `-n` output path runs past the DOS 127-character limit, so
they are merged into `CFG\BC31\TK.CFG` and the MAKEFILEs pass only
`bcc +$(CFG) -n$(OBJDIR) <file>`.

`PCBOARD.CFG` and `ALL.RES` are unchanged and still in the tree.

Three deliberate differences from Clark's pair:

1. `-I` and `-L` are absolute — `\BC31\INCLUDE`, `\BC31\INCLUDE\SYS`,
   `\TOOLKIT\PWA153\H`, `\PCB153\SOURCE\H`,
   `\PCB153\SOURCE\UTIL\PCBSETUP\SOURCE` — because the repo is mounted as
   the drive root. `\BC31\INCLUDE\SYS` is there because the sources
   include `<stat.h>`, which Borland ships as `SYS\STAT.H`. A compiler
   somewhere other than `\BC31` is the one thing that has to be edited
   here.
2. `-C` (nested comments allowed) is added. `SCRNIO\INITSCRN.C` line 45
   and `MISC\PSEARCH.C` line 145 both contain a `/*` comment wrapped
   around a line that already had a comment in it. Without `-C` the
   trailing `*/` is a syntax error. The alternative was editing two
   source files; one switch is less invasive.
3. `-nOBJ\bc31` is dropped, because the MAKEFILE passes `-n$(OBJDIR)`.

**There is no `-DLIB`.** That switch empties `_FARDATA_` and produces the
door-SDK flavour of these modules. `ALL.RES` sets `-D_FARDATA_=_FAR_`,
which is what PCBOARD, PCBSETUP and FIDOUTIL link against.

## Two per-file exceptions

`SCREEN\SCROLLUP.C` is compiled with `-B`, which routes it through TASM.
Its inline assembly does `jmp short exit` to a local label called `exit`;
BCC's built-in assembler resolves `exit` to the library function and
rejects the instruction. `SCROLLDN.C` has the same label but never jumps
to it, which is why only one of the pair fails.

`MISC\VIRTUAL.C` is compiled with `-DVIRTUAL_HUGE`. Since the VIRTUAL1
merge that file carries both implementations, and the default is the
near-pointer one. Clark's `MISC_L.386` contains the huge-pointer
variant, which is what PCBFILER needs. See
`toolkit/pwa153/SOURCE/MISC/VIRTUAL-MERGE.md`.

## Loose objects — built, but deliberately not in a library

Some modules are compiled and left as plain `.OBJ` because the programs
that use them link them by path rather than pulling them from a library:

| Object | Wanted by |
|---|---|
| `OBJ\MISC\LARGE\SWAP.OBJ` | PCBSETUP |
| `OBJ\DOS\LARGE\SHOWERR2.OBJ` | FIDOUTIL |
| `OBJ\TOOLKIT\LARGE\NOANSI, NOCHAT, NODISP, NOLANG, NOLOG, NOPCBSYS, NOSCREEN, NOSHELL, NOSTATUS, NOSYS, NOTXT, SMALLDLY` | the door stubs |

This mirrors Clark's own MAKEFILEs, which had a separate `obj:` target
for exactly this.

**The missing one has been found — and it was never a toolkit module.**
`pcb153\SOURCE\UUCP\UUXFER\UUXFER.MAK` asks for
`toolkit\large\fossil.obj`, and this file used to record that no
`FOSSIL.C` existed anywhere in `toolkit/pwa153/SOURCE`. That was correct,
and it was the wrong place to look.

Clark's original `FOSSIL.OBJ` was recovered on 2026-09-22 from
`TOOLKIT2.ZIP` → `TOOLKIT/BC/PCBKIT_L.EXE` — 9,157 bytes, dated
Oct 11 1993. Its OMF records name the source: **`y:\modem.c`**. So the
object is `pcb153\SOURCE\MODEM\MODEM.C` compiled
`-DCOMM -DMULTIPORT -DLIB -DFOSSIL`, which `#include`s `MODEMFOS.C` and
`MODEMASY.C` as one unit. The nearest real FOSSIL code we held *was* the
answer; it just was not a separate `.C` file and never had been.

`COMMDRV.OBJ` (10,024 bytes) came out of the same archive: the same
`MODEM.C` with `-DCOMMDRV`, pulling in `MODEMDRV.C` instead.

sysop/0 then wrote a standalone reconstruction, 767 lines, whose 79 OMF
exports match Clark's object symbol for symbol — 30 `FOSSIL_*`, 11
`ASYNC_*`, 30 vtable pointers, 8 helpers. It carries its own type stubs
instead of `project.h`/`model.h`, so it builds alone:

    toolkit/pwa153/SOURCE/TOOLKIT/FOSSIL.C
    bcc -ml -c -oFOSSIL.OBJ FOSSIL.C

Both recovered objects are in `toolkit/pwa153/bc31/obj/` with a README.
They stay there, out of the attic, until a build of `FOSSIL.C` is
compared against them — **the 79-symbol match was made by a Python OMF
parser, not by a compiler**, so the acceptance test is still outstanding:
compile under BC 3.1, dump the exports, diff against the original.

## Source edits

`SOURCE/PCB/CHKEXIST.C` line 27 was
`#include <\proj\pcbsetup\source\setup.h>` — Clark's own drive layout,
which cannot be redirected with `-I` because it is absolute. It is now
`#include <setup.h>`, with the folder on the include path in `TK.CFG`.
It is the only `\proj\` include in the whole toolkit.

## What this unblocked

**PCBSETUP.EXE and FIDOUTIL.EXE now build.** Zero errors, zero warnings.

    OUT\PWA153\PCBSETUP.EXE   360,736 bytes   (overlaid, two MZ headers)
    OUT\PWA153\FIDOUTIL.EXE   152,962 bytes

Linking PCBSETUP against the rebuilt libraries first took it from **672
fixup overflows to 59**, and every one of the 59 was inside PCBSETUP's
own source rather than the libraries. Those 59 are now gone too. The
cause was a far/near mismatch in PCBoard's own code:

| Count | Symbol | Declared in | Defined in |
|---:|---|---|---|
| 27 | `_directory_info` | `PCBSETUP\SOURCE\FIDOINFO.C:47` | `pcb153\SOURCE\FIDO\FCONFIG.C:62` as `_FAR_` |
| 16 | `_archiver_info` | `PCBSETUP\SOURCE\FIDOARC.C:47` | `FCONFIG.C:61` as `_FAR_` |
| 6 | `_freq_info` | `PCBSETUP\SOURCE\FIDOFREQ.C:43` | `FCONFIG.C:60` as `_FAR_` |
| 6 | `_emsi_data` | `PCBSETUP\SOURCE\FIDOEMSI.C:39` | `FCONFIG.C:63` as `_FAR_` |
| 2 | `_MenuList` | `toolkit\pwa153\H\SCRNIO.EXT:47`, near | `PCBSETUP\SOURCE\PCBSETUP.C:63` as `_FAR_` |

The first four are defined `_FAR_` in shared FIDO code and were declared
near in PCBSETUP, so the six `extern` lines gained `_FAR_`.

`MenuList` is the other way round: the shared toolkit header
`SCRNIO.EXT` declares it **near**, and PCBSM and PCBFU both define it
near — only PCBSETUP defined it `_FAR_`. So the fix there was to drop
`_FAR_` from `PCBSETUP.C:63`, which makes PCBSETUP agree with the header
and with the other two programs rather than changing a header three
programs share. At `MenuItemTotal` 89 it is 3,204 bytes in DGROUP, and
PCBSM already carries 71×36 near.

## The `_KbdStatus` gap

FIDOUTIL compiled completely but would not link:

    Error: Undefined symbol _KbdStatus in library file system_l.lib in module BGETKEY

`SYSTEM\BGETKEY.C` declares `extern uint far *KbdStatus;` and
`getkbdstatus()`/`setkbdstatus()` read and write through it. **Nothing in
the tree defines it**, and neither did the `SYSTEM_L.386` we were handed.
PCBOARD is fine because `pcb153\SOURCE\MAIN\INIT.C` declares its own and
sets it at line 1104 to `MK_FP(0x40,0x17)`; the utilities that do not
link `INIT.C` have nothing.

`SCRNIO\INITSCRN.C` line 45 still carries Clark's own note over the
commented-out definition — `/* moved to SYSTEM.LIB */` — so SYSTEM.LIB is
where it went; the module just never reached us.

The fix is a new `toolkit/pwa153/SOURCE/SYSTEM/KBDSTAT.C`, one line of
code, built into `SYSTEM_L.LIB`. Because that module defines nothing
else, TLINK pulls it out of the library only when `KbdStatus` is still
unresolved — so PCBOARD keeps using `INIT.C`'s copy, PCBSM keeps using
the shim it already links from `EXTRAOBJ\KBDSTAT.C`, and PCBSETUP and
FIDOUTIL get this one. It is marked as a reconstruction in its own header
and in the `SYSTEM` makefile.

## Still to do

* `FOSSIL.C` — see above; `UUXFER` needs it.
* The other three models (S, C, M) and the `msc70` / `tc201` legs; the
  makefiles already take `-DMODEL=` and `-DCVER=`.
* `OUT\PWA153\*.EXE` may be covered by `.gitignore` — the un-ignore rule
  is written for `OUT/upd154/**/*.EXE`, not `OUT/pwa153`.
