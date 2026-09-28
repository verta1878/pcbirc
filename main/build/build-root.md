# The build root: the repo IS the drive

Written 2026-09-22, when the pwa153 makefiles were repointed.

## The rule

**Mount the repo folder as the root of the build drive.**

    mount C C:\path\to\pcbircrevival        (DOSBox-X; see DOSBOX.CFG)

Everything the makefiles name is then inside the repo:

    \OUT            -> <repo>\OUT           build output
    \PCB153         -> <repo>\pcb153        15.3 PWA program source
    \TOOLKIT\PWA153 -> <repo>\toolkit\pwa153  headers and library source
    \PCBCBASE       -> <repo>\pcbcbase      CodeBase source

The repo folder can be called anything — `pcbirc`, `pcbircrevival`, a dated
copy — because nothing refers to it by name. What matters is that its root
is the drive root.

If a parent folder is mounted instead, `\OUT` becomes a *sibling* of the repo
and build output lands outside it. `BLDDOS.BAT` and `BLDOS2.CMD` therefore
check for `\APPLY.txt` before doing anything and stop if it is missing.

## Why absolute and not relative

The makefile defaults are absolute (`SRC = \PCB153`, `TKIT = \TOOLKIT\PWA153`)
because relative ones do not survive DOS. With `SRC = ..\..\..` the compile
line for `SOURCE\MISC\IDX` came to:

    BCC +MAKEIDX.CFG -c -ml -nbc31 -Oebglmptv
        -IC:\B\C31\INCLUDE;..\..\..\..\toolkit\pwa153\H;..\..\..\SOURCE\UUCP\COMMON .\makeidx.c

which is past the 127-character DOS command line: *Fatal: Command arguments
too long*. The absolute form is shorter and reads better in a build log.

Every one of them is guarded, so a different layout can override without
editing a makefile:

    !ifndef ROOT
    ROOT     = \OUT
    !endif

`ROOT`, `BRANCH`, `CVER`, `SRC`, `TKIT` and `LIBSDIR` all work that way.
Borland MAKE 3.6 lets a makefile definition beat a `-D` on the command line —
the opposite of GNU make — which is why the guard exists rather than a plain
assignment.

## Output layout

    \OUT\<BRANCH>\                 finished programs        (BRANCH = PWA153)
    \OUT\<BRANCH>\SDK\<CVER>\LIB\  libraries we build       (CVER = BC31)
    \OUT\<BRANCH>\SDK\<CVER>\OBJ\  loose objects the linker reads directly

Object files and the per-program `.CFG` still land beside the source, in each
program's `BC31\` or `OBJ\BC31\` folder. `make CLEAN` removes them, and
`BLDDOS CLEAN` runs that for every program. `\OUT` is never touched by CLEAN.

## 8.3 names

`BUILD_DOS.BAT` and `BUILD_OS2.CMD` are nine characters before the dot. DOS
cannot call a file with a nine-character name — COMMAND.COM answers "Bad
command or filename" — and FAT cannot store one. The build entry points are
therefore **`BLDDOS.BAT`** and **`BLDOS2.CMD`**; the long-named files remain
as notes pointing at them.

## The compiler

**`\BC31` is the standard location**, which is what `DOSBOX.CFG` sets.
`BLDDOS.BAT` uses `BC31PATH` if it is set and otherwise looks for `\BC31`
first, then `\B\C31` (the layout inside `devtools/BC31.zip`), so both work.

No makefile names the compiler any more. `PPLC.MAK` used to carry a
hard-coded `..\..\b\c31\lib`; it now uses `$(LIBPATH)`, which `BCDOS.BAT`
and `BLDDOS.BAT` both set from `BC31PATH`. Moving the compiler is a one-line
change in one place.

## Logs

DOSBox keeps no scrollback worth reading, so send the run to a file:

    BLDDOS ALL > \OUT\BUILD.LOG

## A note on case

DOS creates object folders in upper case (`BC31`, `OBJ\BC31`). That is
invisible on DOS and Windows but matters to the OpenWatcom/Linux builds;
`MAIN/build/scripts/normalize-case.sh` is the tool for that.

`BLDOS2.CMD` uses `BCROOT`, default `\BCOS2`. Borland C++ for OS/2 is 32-bit
flat — one library set, no memory models.
