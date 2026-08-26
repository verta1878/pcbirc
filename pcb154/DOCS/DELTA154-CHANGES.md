# PCBoard 15.4 Delta — Crew Changes

This documents what the crew added, changed, fixed, and moved on top of
Clark Development's original 15.4 beta. The shipped 15.4 docs (WHATSNEW,
HISTORY, README.1ST, REPORT.TXT) describe Clark's features; this file
describes our restoration and build work.

## Compiler

- **15.4 Delta builds with OpenWatcom 1** (stock, /opt/watcom).
  Clark's 15.4 was mid-port to Watcom; we completed it.
- 15.3 PWA builds with Borland C++ 3.1 (Clark's original compiler).
- 15.41 IRC will build with openwatcom2irc (sysop/0's fork).

## Toolkit — Compiles 100% on DOS

267/267 DOS objects compile under Watcom 16-bit.
Flags: `wcc -ml -2 -za99 -dPCB152 -dLIB -dCOMM -fi=WATCOMPAT.H`

### Watcom-specific fixes applied
- PCBTOOLS.H: `__WATCOMC__` added to enum-to-int path (enums as params)
- PCBOARD.H: waitforkey/bgetkey2 guarded with `__cplusplus`
- SCRNIO.H: inputattrtype mapped to int for Watcom
- 5 Country files: `extern "C"` guarded for C compilation
- DBL_LONG.C, PR_LONG.C: `_AX`/`_DX` renamed (Watcom pseudo-registers)
- PSEARCH.C: bsearch renamed to pcb_bsearch (stdlib conflict)
- ZSWAPSTR.C, ZSWAPVIR.C: huge pointers to match ZSORT.H
- CPUTYPE.C: rewritten with Watcom `_asm` blocks (needs -2 for SMSW)
- ANSI.C: ansi_print rewritten using intdos() (asm labels unsupported)
- DATA120.C: read120file pascal to LIBENTRY
- WATCOMPAT.H: FONT_8x8/FONT_8x14 defined
- NOINPUT/NOMEMORY/NOSHELL stub signatures aligned to 15.4 headers

## OS/2 Target

6 files compile on the OS/2 target with `-d__OS2__ -IOS2TK/H`:
HANDLERS, DELAY, GIVEUP, THREADS, COUNTRY, TEST.

## Binaries

15 EXEs in OUT/delta154/ (Watcom, _W suffix).
Verified executing under DOSBox-X.

## Version

15.4 Delta reports v15.4 (DEFINES.H VERSION_MINOR = "4").
PWA reports v15.3.

## Moved / Reorganized

- 15.4 docs moved here from the binary output directory
- Runtime data (PCBOARD.SER, PCBSM.CLR/CNF, ENDPCB) to OUT/support/
- Category library manifests in MAIN/build/manifests/
