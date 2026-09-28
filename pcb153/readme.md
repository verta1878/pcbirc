# PCBoard 15.3 Source — PWA

Clark Development Company's PCBoard 15.3 source, preserved by PWA
(Pirates with Attitude) from Corey Blake's licensed copy.

## This is the PWA 15.3 base — no 15.4 features

Restored from `reference/pcb153src0014.zip` (password PCB153).
Contains zero 15.4 features. This is the foundation.

The 15.4 upgrades were released as binary upgrades on top of 15.3.
The 15.4 features reverse-engineered from those binaries live in
`pcb153/upd154/ (reconstructed 15.4 source)`. The active 15.4 Delta work
is in `pcb154/`.

**Delta = the diff between this 15.3 base and the completed 15.4.**
We cannot generate that diff yet because the 15.4 code isn't complete.

## Origin

Corey Blake purchased what may be the only PCBoard source license
Clark ever sold (~$2,000). The package was missing two OBJ files
(serial number control, node license count). Clark closed two days
later. A programmer patched the missing pieces so it compiled; PWA
preserved and distributed the archive.

## Build

Borland C++ 3.1. Flags:
`-c -P -ml -Od -V -Vmp -Vmd -ff -DPCB152 -DCOMM -DSTATS -DMP -D386 -DDBASE -DFIDO`
Include paths: BC31\INCLUDE, BC31\INCLUDE\SYS, toolkit/pwa153/H

Builds: PCBOARD.EXE, PCBOARD2.EXE, PPLC.EXE.

## Toolkit

`toolkit/pwa153/` — the 15.3 toolkit (283 C files).
244/262 compile clean; remaining need minor build-path fixes
(hardcoded dev paths, asm files needing TASM, stubs referencing main
headers). These are "patched to compile" fixes, not features.

## Compiler

Build tools in `PCB153BT.ZIP` at repo root (BC31 + TASM).

## VIRTUAL.C / VIRTUAL1.C Merge

VIRTUAL.C and VIRTUAL1.C were merged into a single VIRTUAL.C, and
VIRTUAL.H / VIRTUAL1.H into a single VIRTUAL.H. A compile-time switch
selects the implementation:

- `#define VIRTUAL_HUGE` — huge pointers + disk caching (>64KB datasets)
- default — near pointers, memory-only (the old VIRTUAL1 behavior)

Both modes verified compiling under Borland BC31. Programs that need
the huge/cached version define VIRTUAL_HUGE at build time.

(delta154 and irc1541 toolkits still have the two-file setup; they'll
get the same merge when their turn comes.)

---

## Source Inventory

Source lives in two places: **pcb153/** (active build tree) and
**reference/pcball/pcboard/** (Clark's original PWA source archives:
pcb-main, pcb-util, pcb-misc, pcb-libs). pcb-main has Watcom build
scripts — deferred to 15.4 branch.

### Ship List — 37 EXEs

The 15.3 installer puts 37 EXEs on disk. Four different numbers were
being conflated (audited 2026-09-17):

| | count | meaning |
|---|---|---|
| shipped | **37** | what the 15.3 installer writes to the target |
| has a makefile somewhere | **26** | buildable in principle from surviving source |
| present in the build root | **11** | what DOSBOXX.ZIP can reach today |
| actually built from source | **8** | PCBOARD, PCBOARDM + 6 utilities |

### Tier 1 — In the repo AND the build root (11 + MAKEHELP)

PCBSETUP, PCBSM, MKPCBTXT, FIDOUTIL, MAKEIDX, USERNET, USERNET2,
UUIN, UUOUT, UUUTIL, UUXFER. Plus MISC\HELP\MAKEHELP.C (COMPILE.BAT,
no .MAK).

All build and link. 6 utility EXEs already in dosboxx.zip.
UUCP suite: entry 47 fixed STBSTUB → 4 real stb*.cpp implementations.

### Tier 2 — In the repo, NOT in the build root

PCBOARD, PCBOARDM, PCBOARD2, PPLC100, PPLC330.

BUILDROOT/PCB153/ has no `153\` directory — carries the batch drivers
but none of the makefiles they invoke. The two binaries in OUT/pwa153/
came from a separately staged tree. Fixing the zip is the cheapest
single unblock.

### Tier 3 — Source EXISTS in the archive, never imported (14)

Complete source and Clark's own makefiles, sitting in PCBSRCV/000/
and in neither the repo nor the build root. Importing is a copy, not
a reconstruction:

| Target | Archive path | What it does |
|---|---|---|
| PACKFIDO | SOURCE/MISC/PACKFIDO/PACKFIDO.MAK | FidoNet packet packer |
| PCBDIAG | UTIL/PCBDIAG/PCBDIAG.MAK | Data file integrity checker |
| PCBEDIT | UTIL/PCBEDIT/MAK/PCBEDIT.MAK | Full-screen message editor |
| PCBFILER | UTIL/PCBFILER/PCBFILER.MAK | File directory manager |
| PCBMODEM | UTIL/PCBMODEM/*.MAK | Modem setup utility |
| PCBMONI | UTIL/PCBMONI/PCBMONI.MAK | Node activity monitor |
| PCBNLC | UTIL/PCBNLC/PCBNLC.MAK | FidoNet nodelist compiler |
| PCBPACK | UTIL/PCBPACK/153/PCBPACK.MAK | Message base packer |
| PCBSTATS | UTIL/PCBSTATS/PCBSTATS.MAK | Usage statistics |
| PCBCP | UTIL/PCBCP/1522/PCBCP.MAK | OS/2 PM control panel |
| WAITFILE | MISC/WAITFILE/WAITFILE.MAK | File-appearance waiter |
| ZMRECV | MISC/ZMODEM/ZMODEM.MAK | Zmodem receive |
| ZMSEND | MISC/ZMODEM/ZMODEM.MAK | Zmodem send |

PACKFIDO source is in `pcb153/SOURCE/MISC/PACKFIDO/` (also in
upd154 and pcb154 trees).

Import blockers: small-model category libraries (DOS_S.LIB etc.) have
never been built, and several programs want objects at `<subdir>\large\`
where the 15.3 build writes `<subdir>\large.386\`.

### Tier 4 — No source (11)

No makefile in PCBSRCV/000/, no source in the repo. All binaries
exist in `pcb1541/install/dist/target/` (available for decompilation).
The 15.3 distribution is also in `reference/pcball/pcboard/PCB153-100a
through PCB153-100d (08-31-96)`.

PCBDESC, ENCRYPT, FIXTEXT, INIT, MKPCBMNU, OVLSIZE,
RDPCBTXT, TESTFILE, VIEWARCH.COM, VIEWZIP, DOORWAY (third-party).

UPGRADE has source in `toolkit/pwa153/SOURCE/TOOLKIT/SAMPLES/UPGRADE.C`
(all toolkit branches). Binaries in `OUT/pwa153/SDK/BC31/{large,medium}/`.

Three could be written clean-room: MKPCBMNU and RDPCBTXT (MKPCBTXT
source documents the format), OVLSIZE (reads publicly documented
Borland overlay header). DOORWAY is Marshall Dudley's — not ours.

### Libraries

| Library | Source | Status |
|---|---|---|
| COMMDRV SDK | pcbcbase/COMMDRV/ | DONE — OBJ+LIB, COMM.H cdecl fix |
| pcbkit_l.lib | reference/pcb-libs/SOURCE/TOOLKIT/ (40+ files) | PARTIAL — TC 2.01 build needed |
| pcbtools categories | reference/pcb-libs/SOURCE/ (9 subdirs, ~260 files) | REFERENCE |
| c4base.lib (CodeBase) | pcbcbase/CODEBASE/SOURCE/ | REFERENCE — open blocker |

### Reference Source Map

All source from reference/pcball/pcboard/:

**pcb-util/** (13 programs):

| Program | Source files | What |
|---|---|---|
| PCBSETUP | 60+ .C/.CPP | System configuration (largest utility) |
| PCBSM | 24 .C | Subscription manager |
| PCBFILER | 30+ .C/.CPP + .ASM | File directory manager |
| PCBEDIT | 12 .CPP + .ASM | Full-screen message editor |
| PCBPACK | 10 .C + .ASM | Message base packer |
| PCBMODEM | 8 .CPP/.C | Modem setup utility |
| PCBMONI | 1 .C | Node activity monitor |
| PCBNLC | 3 .C/.CPP | FidoNet nodelist compiler |
| PCBSTATS | 2 .C | Usage statistics |
| PCBTEXT | 2 .C (MKPCBTXT) | Language file generator |
| PCBDIAG | 7 .C | Data file integrity checker |
| PCBCP | 8 .C (OS/2 PM) | OS/2 control panel |
| PCBUTILS | 3 sub-programs | OFFLINE, PCBTITLE, WAITBU |

**pcb-misc/** (7 programs + 1 reference):

| Program | Source files | What |
|---|---|---|
| UUCP | 4 sub-programs + common | UUIN, UUOUT, UUUTIL, UUXFER (25 shared files) |
| FIDOUTIL | 7 .CPP | FidoNet config tool |
| USERNET | 1 .C | Who's-online display |
| WAITFILE | 2 .C + .ASM | File-appearance waiter |
| ZMODEM | .MAK (source in subdirs) | Zmodem transfer protocol |
| HELP | 1 .C (MAKEHELP) | Help file compiler |
| MD5 | RFCs only | Reference documentation (no source) |

**pcb-libs/** (shared library, ~260 source files):

| Category | Files | What |
|---|---|---|
| COUNTRY | 13 | Internationalization, date formats |
| DOS | 37 | DOS file I/O abstraction |
| DOSCLS | 1 | C++ DOS class wrapper |
| MISC | 82 | String, math, file, date, crypto, sort, virtual memory |
| PCB | 22 | PCBoard data file readers, config parsing |
| SCREEN | 40 | Direct screen I/O, ANSI, windows |
| SCRNIO | 20 | Forms, input, menus, keyboard |
| SYSTEM | 7 | OS abstractions (keyboard, semaphores, threads) |
| TOOLKIT | 40+ | SDK init, stubs, samples |

### pcb153/SOURCE/ (active build tree)

| Category | Files | What |
|---|---|---|
| MAIN | 25 | Core BBS engine (PCBOARD.C entry) |
| H | 66 | All shared headers |
| DISPLAY | 12 | Display/ANSI rendering |
| DOS | 5 | Low-level DOS I/O |
| FIDO | 27 | FidoNet mail tosser |
| MODEM | 7 | Serial — async, FOSSIL, OS/2, COMMDRV backends |
| MSG | 6 | Message base engine |
| NODE | 7 | Multi-node management |
| PPL | 18 | PPL script interpreter |
| SUPPORT | 13 | CRC32, memory, overlay, token, xmodem |
| USERS | 6 | User database management |
| UUCP | 4 | stb*.cpp (entry 47) + STB.CFG |
| COMPILER | 3 | PPL compiler (PPLC) |
| ASM | 8 | Assembly routines (C0, ASYNC, ANSI, etc.) |

### Source File Totals

| Area | Source Files | Headers |
|---|---|---|
| pcb153/SOURCE/ | ~150 | 66 |
| pcb-util/ | ~130 | ~40 |
| pcb-misc/ | ~60 | ~20 |
| pcb-libs/ | ~260 | 34 |
| pcbcbase/ | ~15 | ~5 |
| **Total** | **~615** | **~165** |

### Open Blockers

1. **c4base.lib** — CodeBase DOS/BC31 compile (backlog item 6)
2. **pcbkit_l.lib** — TC 2.01 build never committed (backlog item 7)
3. **TKLIB** — TC201 + MSC70 legs (backlog item 8)
4. **SDK paths** — MAK/CFG ROOT= across 4 branches x 3 compilers (backlog item 9)
5. **Tier 4 source** — 11 programs have binaries (`pcb1541/install/dist/target/`) but no source

## the crew 4free
