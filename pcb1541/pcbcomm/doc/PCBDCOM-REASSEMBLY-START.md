# pcbdcom — Reassembly Starting Point
_wrench's area · captured 2026-09-25 · read this first when picking the work back up_

## What this is
A crash scattered pcbdcom's pieces and lost the method hexadecimal used to
build it. We have the parts, not the assembly. The job is reconstruction
from surviving pieces — inventory first, then rebuild. This doc is the
"where we are / how to start" so it doesn't have to be re-derived.

## The honest state (as of 2026-09-25)
1. Board drivers: hexadecimal finished them — code-complete, UNTESTED.
2. exe/com programs (DRVSETUP, TEST, TSR): NEVER STARTED. New work, not lost.
   (pcbdcom.c has a main()/TSR-install path, but treat exe/com as unstarted
   until the inventory confirms what actually survived — verta: "I don't know
   if the code lived.")
3. The .md that documented the exe/com file info: LOST. Real, gone.
4. DRVSETUP/TEST live SEPARATE from commdrv and from the hexadecimal work —
   the only way to know their state is to look at the commdrv repo.
5. How pcbdcom (earlier "pcbcomm", two m's) was assembled: LOST. Must be
   re-derived from how the pieces fit.
6. pcb1541 = the latest work. It gets ANALYZED into pcb154/delta (not a
   mechanical move), then pcb1541 can be retired. Analysis first.
7. Docs are broken two ways: some STALE (README's "being folded in" merge
   framing is wrong) and some LOST (the exe/com .md). The doc layer can't be
   trusted as a map until repaired.

## The pieces are scattered across
- pcb1541/pcbdcom/        (latest code: 23 backends/src, pcbdcom.c, headers)
- toolkit/pwa154/pcbdcom/ (154 SDK: has the shim + big pcbdcom.h)
- toolkit/pwa153/         (intact Borland reference: FOSSIL.C, COMMDRV.OBJ, TKLIB.MAK)
- toolkit/delta154/       (OpenWatcom toolkit — 0 COMMDRV, never wired)
- pcb154/                 (154 base source — MODEMDRV.C present, NO pcbdcom/ yet)
- commdrv (SEPARATE repo) (holds DRVSETUP/TEST truth — must be pulled)
- pcbdcom-fossil5c zip    (09-23 checkpoint — SUPERSEDED, nothing to recover)

## How to get started (steps 1-4 are all read-only / safe)
1. Pull pcbirc + commdrv onto the bench (clone, read-only).
2. INVENTORY: every surviving piece — where, real/stub/skeleton/missing —
   into one map file. This is the count we don't currently have.
3. FIX THE MAP as it's made: correct stale docs, reconstruct the lost
   exe/com .md, so the map is true before any rebuild.
4. REPORT to verta by the numbers. Verta decides build order AFTER seeing
   the real picture. No build/move/delete before that.

## Then the finish road (after inventory, in rough order)
- Reassemble the scattered pieces into pcb154/delta as one buildable tree.
- Finish the 5 incomplete backends: cyclom, easyio, rocket (TODO markers),
  digi_accel, digi_pcxe (thin skeletons).
- Build the 3 never-started programs: DRVSETUP (card config UI), TEST (port
  tester), TSR.
- Land the SDK shim into the source tree so the linkable .OBJ builds from source.
- Wire the OpenWatcom/delta154 build (comm.h compiler guards, Watcom library).
- TEST: clean build on all compilers; UART/FOSSIL path verifiable in DOSBox-X;
  exotic multiport cards (Cyclades/Boca/Digi/etc.) need REAL hardware — verta's
  bench or sysops with the boards.

## Future seam (note only — do NOT design for it)
pcbdcom's backend vtable and netfossil (netfosdl) could meet someday — a
network/FOSSIL backend, or a multiport-aware bridge. No need identified yet.
The vtable is the join point when one appears. Don't add coupling early.

## Working rules (verta's, honored)
- Do the work in the sandbox, not on verta's PC.
- Never delete without listing it first; restore anything removed unannounced.
- Status as numbered lists (reply by number).
- Confirm before acting — always.
