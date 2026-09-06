# PCBoard Support Library Chain Build

Build the 7 PCBoard support libraries from `pcb153/SOURCE/`.
This is the foundation for building PPLC, PCBOARD.EXE, and all
PCBoard utilities from source.

## Why

PPLC (and PCBOARD.EXE) link against 7 Clark-built support libraries.
These are not in the repo as pre-built files — they must be compiled
from `pcb153/SOURCE/`. Without them, no PCBoard program builds from
source.

## Libraries to build

| Library | Source directory | Purpose |
|---|---|---|
| countryl.386 | SOURCE/MISC/ | Country/locale support |
| dos_l.386 | SOURCE/DOS/ | DOS abstraction layer |
| screen_l.386 | SOURCE/DISPLAY/ | Screen output (print/println/newline) |
| pcb_l.386 | SOURCE/MAIN/ | PCBoard core support |
| misc_l.386 | SOURCE/MISC/ | Misc utilities (fileexist, etc) |
| system_l.386 | SOURCE/SUPPORT/ | System-level support |
| doscls_l.386 | SOURCE/DOS/ | DOS class wrappers |

External dependencies (already on build image):
- BC 3.1 standard: cl.lib, mathl.lib, emu.lib, overlay.lib
- CodeBase: c4base.lib (dBASE file support — may need sourcing)

## Source tree scope

591 source files across 18 directories in `pcb153/SOURCE/`:

| Directory | Files | Builds |
|---|---|---|
| H/ | 67 | Headers (include path, no build) |
| DOS/ | 5 | dos_l.386, doscls_l.386 |
| DISPLAY/ | 12 | screen_l.386 |
| MISC/ | 67 | misc_l.386, countryl.386 |
| SUPPORT/ | 13 | system_l.386 |
| MAIN/ | 27 | pcb_l.386 |
| PPL/ | 19 | PPLC compiler (links all above) |
| COMPILER/ | 4 | PPLC main() + helpers |
| ASM/ | 8 | Assembly support routines |
| UTIL/ | 207 | PCBoard utilities (builds after libs) |
| FIDO/ | 27 | FidoNet support |
| MODEM/ | 7 | Modem/comm support |
| MSG/ | 6 | Message system |
| NODE/ | 7 | Node management |
| USERS/ | 6 | User management |
| UUCP/ | 100 | UUCP support |
| MKPCBSRC/ | 9 | Build system helpers |

## Build order

Dependencies flow upward:

    1. H/           (headers — include path only)
    2. ASM/         (assembly support — TASM)
    3. DOS/         → dos_l.386, doscls_l.386
    4. MISC/        → misc_l.386, countryl.386
    5. DISPLAY/     → screen_l.386
    6. SUPPORT/     → system_l.386
    7. MAIN/        → pcb_l.386 (depends on all above)
    8. COMPILER/ + PPL/ → PPLC.EXE (links all above)

## Build environment

Same as INSTALL.EXE and 386MAX:

- Compiler: BC 3.1 (C++ mode for .CPP, C mode for .C)
- Assembler: TASM (for CEH.ASM and any .ASM files)
- Librarian: TLIB (creates .386 lib files from .OBJ)
- Linker: TLINK 5.1
- Host: DOSBox-X with host-directory mount
- Makefiles: `pcb153/153/PCBOARD.MAK`, `PPLC.MAK`

## Approach

Same pattern as the 386MAX build sweep:

1. Sweep each directory — compile all .CPP/.C/.ASM files
2. Classify results: clean / errors / missing deps
3. Fix errors (one include fix can unblock many files, like DTE.INC)
4. TLIB the .OBJ files into the .386 library
5. Gate: library compiles clean, TLIB listing matches Clark's
6. Move to next directory in dependency order

## Patches needed

| Patch | Scope |
|---|---|
| patches/pwa153-libchain.patch | Builds the 7 libs from pcb153/SOURCE/ |
| patches/delta154-libchain.patch | Same for 15.4 delta branch |

## What this unlocks

Once the lib chain builds:
- **PPLC 3.00/3.10/3.20/3.30/3.40** — change #defines, compile
- **PCBOARD.EXE** — the main binary, from pcb153/153/PCBOARD.MAK
- **All 207 utilities** in SOURCE/UTIL/
- **Every PPE** — PPLC compiles any .PPS to byte-exact .PPE
- The entire 19-binary rebuild list becomes tractable

---

*hexadecimal, 2026-09-06*
