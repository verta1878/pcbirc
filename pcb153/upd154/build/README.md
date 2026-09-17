# pcb153/upd154/build — build scripts for the 15.4 PWA upgrade

upd154 is Clark's 15.4 reconstructed source (see ../README.md). It
builds with **Borland C++ 3.1** — same compiler/lineage as the 15.3 PWA
base, since 15.4 was a binary upgrade on top of 15.3.

## Scripts

- `BLDUPD154.BAT` — compile the 15.4 PWA source and produce the upgrade
  binaries into `OUT/pwa153/upd154/`.
- (A global "build everything" driver, plus `make clean` / `make
  mrproper` equivalents, will live at the repo top level later and call
  into each version's build — including this one.)

## Status

Not yet a complete build — but **not for the reason this file used to
give.** The "build-fix pass" (STATS control OBJ; DOORS.C UData/header
resolution) was a config error, not a source defect: `CALLWAIT.C` and
`DOORS.C` both compile clean under Clark's real `PCBOARD.CFG` flags.
Retested 2026-09-17. See ../README.md.

**Do not build this with `-DLIB`.** The compile pattern this script
documented — `-DPCB152 -DLIB -DCOMM` — is the *door SDK* flag set, taken
from BLDKBC. Under `-DLIB`, `PCBOARD.H` selects the reduced door-visible
structs, so members the program legitimately uses disappear and you get
exactly the two "gaps" above. PCBoard is built without `-DLIB`.

The working flag set is Clark's, in
`toolkit/pwa153/CFG/BC31/{ALL.RES,PCBOARD.CFG}`, plus what
`MAIN/COMPILE.BAT` adds (`-DSTATS -DMP -D386 -DDBASE -DFIDO`,
`NODES=25`). `OUT/pwa153/BUILD-RECIPE.md` records the 15.3 build that
used it end to end.

What remains genuinely unproven: a full run. Two modules compiling is
not a build.

## Output

Binaries -> `OUT/pwa153/upd154/` (parallel to OUT/pwa153/ for the base
15.3 outputs). The 15.4 upgrade binaries stay separate from the 15.3
base binaries, matching the source relationship (pcb153 = 15.3 base,
pcb153/upd154 = 15.4 upgrade).
