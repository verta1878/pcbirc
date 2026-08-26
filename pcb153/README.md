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
