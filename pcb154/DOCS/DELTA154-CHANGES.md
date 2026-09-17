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

## PCB/IC — inherited, not rebuilt

IC was missing for 20 years and is being restored on both 15.4 legs.
Delta inherits it, but only partly as source:

- **The 6 IC programs ship as Clark's own binaries**, unchanged.
  Borland C++ 3.1 (DOS) and BC++ 2.0 for OS/2 — no C source survives.
  The reconstruction in `pcb153/upd154/SOURCE/IC/` is byte-exact NASM
  `db` output (3,888 functions), which rebuilds Clark's EXEs exactly
  but cannot be compiled by Watcom or modified. They are standalone
  programs launched by RUNINET.PPE, not linked into PCBOARD.EXE, so
  they run on a Delta system as-is.
- **RUNINET.PPE is inherited as real source** — `RUNINET.PPS`,
  121 lines of PPL, compiled by Delta's own PPLC. Not yet byte-exact
  against Clark's 1,808 B: he used PPLC 3.20, our 3.30 build is
  2,261 B and 3.40 is 2,286 B. `toolkit/pplc/3.20/PPLC320.EXE` should
  close it.
- **PCBIC.DOC, PCBIC.PDF and the 32 IC data files** ship from
  `OUT/data/ic12/`.

Making IC patchable means writing C for the six from the disassembly —
a project on the scale of the PCBIC reconstruction itself, not a patch
revision. Recorded in SYSOP_154.TXT section 8 alongside PCBSETUP and
LOCAL, which are shipped-binary for the same reason.

IC belongs to PWA 15.4 (`pcb153/upd154/`); Clark's originals are the
reference in `OUT/clark-original/`.

## Version

15.4 Delta reports v15.4 (DEFINES.H VERSION_MINOR = "4").
PWA reports v15.3.

## Moved / Reorganized

- 15.4 docs moved here from the binary output directory
- Runtime data (PCBOARD.SER, PCBSM.CLR/CNF, ENDPCB) to OUT/support/
- Category library manifests in MAIN/build/manifests/
