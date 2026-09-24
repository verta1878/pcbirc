# FIDOUTIL — it builds. Here is exactly how.

**Verified 2026-09-23.** 11 objects, **0 errors**, `FIDOUTIL.EXE`
**153,674 bytes**, sha256 prefix `b7f01f3885f0f08d…`. Clark's shipped one
is 214,586.

    PCBoard FIDO file conversion utility
    Copyright Clark Development 1996
    Convert 15.21 data to 15.23 format? (Y/N)

— its own strings, out of our binary.

## `USEDBC50` is not a wall

`FIDOUTIL.CFG` said `-nbc50`, `-I\BC5\INCLUDE`, `-L\BC5\LIB`, and the
`USEDBC50` file beside it says *"Now using bc50."* — it sits in **25
directories** across the archive. We have no Borland C++ 5.0.

**It compiles clean under Borland C++ 3.1 anyway.** Every one of the ten
modules, no errors, no source changes. The BC5 markers record what Clark
moved to, not what the source requires.

Two things cost hours before that was clear, and neither was the compiler:

1. **The defines.** Without `-D_FARDATA_=far`, `-DPCBSETUP`,
   `-DQUIET_BUILD`, `-DFIDO`, `-DPCBFU`, `-DFIDOUTIL` the parse dies deep
   inside `PCBOARD.H` with *"Enum syntax error"* at line 123 — an error
   that points at a healthy enum and says nothing about a missing macro.
2. **BC 3.1 reads `TURBOC.CFG`, not `BCC.CFG`.** A config file under the
   wrong name is silently ignored, and you get *"Unable to open include
   file 'stdio.h'"* — which reads like a broken installation.

`-Y` (overlays), `-Vmd` and `-nbc50` were dropped from the config. They
are BC5-era and nothing needs them.

## The recipe

Mount the repo as the drive root, the same convention `BLDTK.BAT` uses,
with Borland C++ 3.1 at `\BC31`:

    mount c <the pcbircrevival folder>      DOSBox-X; or SUBST on real DOS

Compile — `FIDOUTIL.CFG` carries the switches, so `BCC +FIDOUTIL.CFG`,
or copy it to `TURBOC.CFG` and just run `BCC`:

    BCC -c -nOBJ SOURCE\ANALIZE.CPP
    BCC -c -nOBJ SOURCE\CONVERT.CPP
    BCC -c -nOBJ SOURCE\FIDONET.CPP
    BCC -c -nOBJ SOURCE\MAINT.CPP
    BCC -c -nOBJ SOURCE\PCBFU.CPP
    BCC -c -nOBJ SOURCE\PTSETUP.CPP
    BCC -c -nOBJ SOURCE\REPORT.CPP
    BCC -c -nOBJ \pcb153\SOURCE\UTIL\PCBSETUP\SOURCE\CI_BUILD.C
    BCC -c -nOBJ \pcb153\SOURCE\FIDO\DATA.CPP
    BCC -c -nOBJ \pcb153\SOURCE\FIDO\PASSTHRU.CPP
    BCC -c -nOBJ \toolkit\pwa153\SOURCE\DOS\SHOWERR2.C

Link `c0l.obj` + those 11 objects against ten libraries from
`\OUT\pwa153\SDK\BC31\LIB\` — `DOS_L`, `DOSCLS_L`, `PCB_L`, `MISC_L`,
`SCREEN_L`, `SCRNIO_L`, `SYSTEM_L`, `COUNTRYL`, `VMDATA_L`, `TOOLKITL` —
plus `cl.lib`.

### Two things the makefile does not make obvious

**`SHOWERR2.OBJ` is in no library.** It is built from
`toolkit\pwa153\SOURCE\DOS\SHOWERR2.C`. That is why the makefile lists it
as a loose object next to the `.lib` files rather than letting the linker
find it — searched every library we hold, the symbol is in none of them.

**`packfido.obj` is not in the build and must not be.** See the note at
the foot of `FIDOUTIL.MAK`.

### If you build it under DOSBox on Linux

`TLINK` will report `Unable to open file '\out\pwa153\sdk\bc31\lib\dos_l.lib'`.
That is DOSBox lowercasing the path against a case-sensitive host
filesystem, not a repo fault — the file is there. On Windows it resolves
normally. Copy the ten libraries to a short all-caps directory for the
link if you hit it.

## OS/2 — probed, not done

`wpp386 -bt=os2v2 -mf` on `PCBFU.CPP`, with the includes mirrored to
lowercase and `-DLIBENTRY= -Dpascal= -D_FARDATA_=` — **21 errors**, down
from several hundred. Every one is a known Borland→Watcom bridge item,
and none is a design problem:

| Error | What it is |
|---|---|
| `unable to open 'dir.h'`, `'alloc.h'` | Borland-only headers; Watcom has `direct.h`, `malloc.h` |
| `invalid redefinition of the typedef name 'bool'` | Watcom C++ has `bool` built in; `TYPES.HPP` defines it |
| `destructor can be invoked only with '.' or '->'` ×6 in `TYPES.HPP` | `far`/`pascal` stripped, leaving `~` parsed as an operator |
| `defines.h(110)`, `structs.h(474)` syntax | same — a keyword removed by the `-D` bridge |

**`WATCOMPAT.H` already exists** — `toolkit/delta154/H/`,
`toolkit/irc1541/H/`, `pcb154/LIB/H/` — and `BUILD_OS2_OW.SH` was written
around it for `PCBOARD2.EXE`: *8 `#pragma option` wrapped in
`#ifdef __BORLANDC__`, 8 Borland headers with Watcom equivalents, 413
`LIBENTRY`/`pascal` call sites, `LIBENTRY` empty in OS/2 mode.* The same
bridge, pointed at FIDOUTIL.

**The real blocker is below that.** FIDOUTIL links eight category
libraries — `dos_l`, `doscls_l`, `pcb_l`, `misc_l`, `screen_l`,
`scrnio_l`, `system_l`, `countryl` — and **none of them exists for OS/2**.
`BUILD_OS2_OW.SH` sidesteps it by compiling the library *sources* straight
into the executable. An OS/2 FIDOUTIL means building the category chain
with `wcc386 -bt=os2v2` first.

**And it is a port, not a recompile.** FIDOUTIL is C++ throughout and
reads `pcbconftype` and `AREA_STRUCT` off disk through real structs,
compiled `-ml` with `_FARDATA_=far`. Under `wcc386` every `int` in those
structs doubles, and the file layout silently stops matching. It needs
the same fixed-width discipline PACKFIDO got — `pcbword` for every
two-byte on-disk field — applied across `DATA.CPP`, `CONVERT.CPP` and the
shared headers. Nothing warns you when that is wrong; the program just
keeps the wrong areas.
