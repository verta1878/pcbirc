# PCBoard Source Library Chain — Phased Build Plan (pcbsrc v0.1)

## Why this exists

Building ANY PCBoard program from source (PPLC, PCBOARD.EXE, utilities)
requires 7 support libraries that are NOT pre-built in the repo. They
must be compiled from Clark's source. Without them, NOTHING links.

## Prerequisites

Source: `PCBoard_15_3_source_code_v0_014.zip` (uploaded to chat, 26M,
5,414 files). Contains the complete Clark build tree:
- `PCBSRCV/014/LIB/SOURCE/` — library source (8 directories, ~300 files)
- `PCBSRCV/014/LIB/H/` — library headers (112 files)
- `PCBSRCV/014/LIB/CFG/BC31/` — Clark's compiler config
- `PCBSRCV/014/MAIN/SOURCE/` — PCBoard main source
- `PCBSRCV/014/MAIN/SOURCE/H/` — PCBoard main headers (54 files)
- `B/C31/` — Clark's exact BC 3.1 installation

### The CRITICAL flag: -P

Clark compiled ALL .C files as C++ (`-P` flag in `ALL.RES`). Without
this, `TYPES.HPP` fails because it uses C++ constructor-style casts
`t(...)` instead of C-style `((t)(...))`. This is NOT a bug — Clark's
build system assumes C++ mode for everything.

### Compiler invocation

Using Clark's response file approach (verified working 2026-09-06):

    BCC.EXE +C:\LIBSRC\CFG\COMPILE.CFG -nC:\OUT\<dir> <file>.C

Response file (COMPILE.CFG):

    -c -P -ml -3 -ff -Od
    -IC:\LIBSRC\H -IC:\MAINSRC\H -IC:\BC31\INCLUDE
    -DPCBOARD -DPCBCOMM -DNDEBUG -DPCB152 -DPCB153

Assembly: `TASM /mx /d__l__ <file>.ASM, C:\OUT\<dir>\<file>.OBJ`
Library:  `TLIB <libname>.lib + <obj1> + <obj2> + ...`

## Phase table

| Phase | Version | Target | Files | Acceptance |
|---|---|---|---|---|
| 0 | v0.1.0 | Prerequisites staged | — | headers + source + config on host dir, one file compiles |
| 1 | v0.1.1 | dos_l.lib | 48 .C + 1 .ASM | TLIB listing matches Clark's MAKEFILE (35 OBJs) |
| 2 | v0.1.2 | country_l.lib | 14 files | TLIB listing clean |
| 3 | v0.1.3 | doscls_l.lib | 2 files | TLIB listing clean |
| 4 | v0.1.4 | misc_l.lib | 91 files | TLIB listing clean |
| 5 | v0.1.5 | screen_l.lib | 41 files | TLIB listing clean |
| 6 | v0.1.6 | scrnio_l.lib | 20 files | TLIB listing clean |
| 7 | v0.1.7 | system_l.lib | 8 files | TLIB listing clean |
| 8 | v0.1.8 | toolkit_l.lib | 53 files | TLIB listing clean |
| 9 | v0.1.9 | PPLC links | 9 source files | PPLC.EXE produced, runs on test .PPS |
| 10 | v0.1.10 | All verified | — | all 7 .lib + PPLC.EXE, full build from source |

v0.1.0 is DONE (verified 2026-09-06: 5 DOS files compile clean).

## Build order

    1. LIB/H/ + MAIN/SOURCE/H/   (headers — include path only)
    2. LIB/SOURCE/DOS/            → dos_l.lib
    3. LIB/SOURCE/COUNTRY/        → country_l.lib
    4. LIB/SOURCE/DOSCLS/         → doscls_l.lib
    5. LIB/SOURCE/MISC/           → misc_l.lib
    6. LIB/SOURCE/SCREEN/         → screen_l.lib
    7. LIB/SOURCE/SCRNIO/         → scrnio_l.lib
    8. LIB/SOURCE/SYSTEM/         → system_l.lib
    9. LIB/SOURCE/TOOLKIT/        → toolkit_l.lib

Each directory has a MAKEFILE from Clark showing which OBJs
go into TLIB.

## What goes wrong without this document

Without this information, the next session will:
1. Try to build PPLC
2. Discover it needs support libraries
3. Try to compile the libraries
4. Hit TYPES.HPP errors (C++ casts in C mode)
5. Spend a session debugging before discovering the -P flag
6. Discover headers missing from pcb153/SOURCE/H/
7. Need the PWA source upload again
8. Lose another session

This happened on 2026-09-06. It must not happen again.

## What this unlocks

Once all 7 libs build:
- PPLC 3.00/3.10/3.20/3.30/3.40 from source (change #defines)
- PCBOARD.EXE from source (pcb153/153/PCBOARD.MAK)
- All 207 utilities in MAIN/SOURCE/UTIL/
- Every PPE via source-built PPLC
- The entire 19-binary rebuild list

## Patches needed

| Patch | Scope |
|---|---|
| patches/pwa153-libchain.patch | Builds the 7 libs from LIB/SOURCE/ |
| patches/delta154-libchain.patch | Same for 15.4 delta branch |

---

*hexadecimal, 2026-09-06*
