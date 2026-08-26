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

Not yet a complete build: the reconstructed source needs the build-fix
pass first (STATS control OBJ; DOORS.C UData/header resolution — see
../README.md). BLDUPD154.BAT is scaffolded to the point those fixes
allow; it targets Borland C++ 3.1 and writes to OUT/pwa153/upd154/.

## Output

Binaries -> `OUT/pwa153/upd154/` (parallel to OUT/pwa153/ for the base
15.3 outputs). The 15.4 upgrade binaries stay separate from the 15.3
base binaries, matching the source relationship (pcb153 = 15.3 base,
pcb153/upd154 = 15.4 upgrade).
