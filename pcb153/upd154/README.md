# pcb153/upd154 — source for Clark's 15.4 binary upgrades

**upd154 is the source code for Clark Development's 15.4 binary
upgrades to PCBoard 15.3.**

Clark shipped 15.4 not as a full release but as **binary patches** that
upgraded an installed 15.3. This directory is that 15.4 reconstructed
into source form: the 15.3 PWA base (the parent pcb153/ tree) plus the
changes recovered from Clark's 15.4 binaries. In other words, pcb153 is
15.3 and `pcb153/upd154` is the 15.4 update layered on top of it — which
is exactly the relationship Clark's binary upgrades had.

## Why it lives under pcb153

15.4 PWA is an *update to 15.3*, not an independent product. Keeping it
as `pcb153/upd154` mirrors that: 15.3 is the base, upd154 is the upgrade
delta in source form. (The pure binary-level delta is also preserved as
`patches/15.4-pwa.patch`, which applies to the 15.3 base to regenerate
this tree.)

15.3 itself (pcb153/SOURCE) stays **pure** — untouched by the upgrade.

## Status — source only; the "build-fix pass" was a config error

There are no libraries or executables here yet, but **the two blockers
this file used to list are not source defects.** Both were retested on
2026-09-17 under headless dosbox-x with Clark's own Borland C++ 3.1 and
his real `PCBOARD.CFG` + `ALL.RES` flags:

    bcc +<pcboard cfg> -ml -n<obj> CALLWAIT.C     0 errors
    bcc +<pcboard cfg> -ml -n<obj> DOORS.C        0 errors

Both produce clean `.OBJ`. The previously-recorded gaps were:

- `CALLWAIT.C` — "'Stats' undefined (the missing STATS control OBJ, the
  same gap the pure 15.3 PWA source has)". Wrong on both counts. The
  15.3 source compiles `CALLWAIT.C` fine — `CALLWAIT.OBJ` is in the
  working 15.3 build that produced `OUT/pwa153/PCBOARDM.EXE`. `Stats` is
  gated by `-DPCBSTATS` / `-DSTATS`, which the attempted build did not
  define.
- `DOORS.C` — "UData member access (TotUpldBytes): a header/struct
  resolution between the reconstructed source and the toolkit". Also a
  define, not a struct mismatch.

The cause is in `build/BLDUPD154.BAT`, whose documented compile pattern
was `-DPCB152 -DLIB -DCOMM`. **`-DLIB` is the door-SDK switch.** Under
it, `PCBOARD.H` selects the reduced, door-visible versions of `Status`
and the user structs — so members that exist in the real program build
genuinely vanish. PCBoard itself must be built *without* `-DLIB`, with
Clark's `PCBOARD.CFG` flags. Same class of error as the one that made
the PCBKBC SDK look unbuildable in small and compact
(`toolkit/pwa153/bc31/README.md`).

So the reconstruction is sound and does not need a source-fix pass. What
it needs is the correct config and a full run — which has not been done,
so "compiles 100%" is still unproven. Two modules is not a build.

## Toolkit

The 15.4 PWA toolkit lives at `toolkit/pwa154` — under `toolkit/` with
the same structure as every other branch (pwa153, delta154, irc1541).
It is a **self-contained** copy: editing `toolkit/pwa153` has no effect
on it. The only functional difference from the 15.3 toolkit is one enum
value, `SPACERIGHTAT` in PCBTOOLS.H (for @x color codes). See
`toolkit/pwa154/README.md`.

## Build + output

- **Build scripts:** `pcb153/upd154/build/` (this directory's own
  scripts — upd154 builds separately from 15.3, since it's the upgrade
  layer). See build/README.md.
- **Output target:** `OUT/pwa153/upd154/` — the compiled 15.4 PWA
  upgrade binaries will be released there, parallel to OUT/pwa153 (the
  base 15.3 outputs). This keeps the 15.3 base outputs and the 15.4
  upgrade outputs cleanly separated, matching the source relationship.

## Not to be confused with 15.4 Delta

15.4 Delta (`pcb154/`) is the crew's active OpenWatcom work — fixing,
adding, changing. That is a separate effort from this faithful
reconstruction of Clark's shipped 15.4.
