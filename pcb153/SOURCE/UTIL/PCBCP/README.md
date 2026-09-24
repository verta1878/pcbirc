# PCBCP — PCBoard Control Panel for OS/2

Clark Development Company, version 15.22 era. An OS/2 Presentation
Manager GUI for managing PCBoard nodes. Distributed as a separate
utility, not part of the licensed source archive.

This folder is **the OpenWatcom port**, flattened and normalised
2026-09-23. Clark's untouched original is at
`reference/pcball/pcboard/pcb-util/PCBCP/` and must stay there.

## The separation

PCBCP existed four times. Three of them were the same program at
different stages, one nested inside another, with a build script that
compiled whichever `SOURCE/` it happened to sit beside:

    reference/pcball/pcboard/pcb-util/PCBCP   Clark's, as received — frozen
    pcb1541/pcbcp                             original + the port nested inside it
    pcb1541/pcbcp/PCBCP                       the port
    drivers/PCBCP                             byte-identical to the port

`BUILD_OW.SH` used `$(dirname "$0")/SOURCE`, so running it from the outer
folder compiled the **un-ported** source — the copy that still carries
`#include <\toolkt21\c\os2h\valapi.h>` — and running it from the nested
folder compiled the ported one. Same script, two locations, opposite
results, nothing telling you which you were in. That is why the two
versions are now kept apart rather than stacked.

See `MAIN/build/PCBCP-COPIES.md` for the file-by-file comparison.

## Case: everything uppercase, includes included

Clark's source has a quirk that DOS and OS/2 hid: the header **files**
were uppercase (`MAIN.H`) while the `#include` lines were lowercase
(`#include "main.h"`). Case-insensitive filesystems never noticed. The
first OpenWatcom port fixed it by renaming the files down to lowercase.

This tree does it the other way: **files and includes are both
uppercase.** 5 headers renamed up, 32 include lines rewritten across 8
files, every one verified to resolve against a real file on a
case-sensitive filesystem.

Uppercase because that is what Clark shipped and what the rest of this
repo uses, and because a lowercase header sorts oddly beside `MAIN.C`
and `MAIN.RC`. Either convention works; mixing them does not.

    DLG.H  HELP.H  MAIN.H  XTRN.H  PCBCP_COMPAT.H

## BUILD_OW.SH — what was wrong with it

It could not have built from either location. Four faults:

1. `PCBSRC/LIB/H` and `PCBSRC/MAIN/SOURCE/H` — `PCBSRC` is a
   pre-ROOT-pass name and is not in this repo in any form. The ROOT pass
   audited makefiles and never looked at shell scripts.
2. `PCBSRC/LIB/SOURCE/SYSTEM/SEMAFORE.CPP` — same, and the file is
   really at `toolkit/pwa153/SOURCE/SYSTEM/SEMAFORE.CPP`.
3. `tools/fpc264irc/sdk/os2tk45/lib` — `tools/` does not exist. This is
   the OS/2 Toolkit 4.5 `os2386.lib` the port notes mention.
4. It linked `pcbcp_stubs.obj`, which **nothing compiled**. The port
   notes say "PCBoard library functions (dosclose, readcheck, etc.)
   stubbed for initial link" — that stub source was written, used, and
   never committed.

Fixed here: the script now walks up to the folder holding `APPLY.txt`
and derives everything from it, the same guard `BLDDOS.BAT` and
`BLDOS2.CMD` use, so it works from whatever depth it is filed at.

    PCBLIBH   = $REPO/toolkit/pwa153/H
    PCBMAINH  = $REPO/pcb153/SOURCE/H
    SEMAFORE  = $REPO/toolkit/pwa153/SOURCE/SYSTEM/SEMAFORE.CPP
    OS2TK     = $OS2TK45LIB, else $REPO/devtools/os2tk45/lib

`PCBCP_STUBS.C` is compiled if present and skipped with a note if not,
so an unresolved-symbol list from `wlink` is what you get instead of a
missing-file error.

## Still missing — this does not build yet

Two things are genuinely absent, and no path edit conjures them:

* **`os2386.lib`** — OS/2 Toolkit 4.5. Not in the repo. Set
  `OS2TK45LIB` or drop it at `devtools/os2tk45/lib`.
* **`PCBCP_STUBS.C`** — never committed. Whoever ported this has it, or
  it gets rewritten from whatever `wlink` reports unresolved.

The port notes read as though the build succeeded. With the stub source
missing and the toolkit path pointing at nothing, it cannot have
succeeded from this tree. The claim and the tree disagree; the tree is
the evidence.

## The six original port fixes, preserved

1. `PCBCP_COMPAT.H` — `bool` typedef, `alloc.h` → `stdlib.h`
2. `\toolkt21\valapi.h` include removed — IBM's OS/2 validation API, not
   needed (see `MAIN/build/PCBCP-COPIES.md` for what TOOLKT21 actually is)
3. `alloc.h` handled by the compat header
4. `_argv` → `__argv` — Watcom C runtime global
5. Ctrl-Z (0x1A) DOS EOF markers stripped
6. Include order — PCBoard headers before Watcom's, because Watcom ships
   its own `dosfunc.h` that shadows PCBoard's

## G.CMD

Carried forward. The port dropped the root-level copy; it is 36 bytes
and byte-identical to `1522/G.CMD`, so nothing was actually at risk —
but it was checked rather than assumed.
