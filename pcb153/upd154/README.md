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

## Status — source only, not yet compiling 100%

There are no libraries or executables here yet. The reconstructed source
does not compile completely on our setup. Known gaps (build-fixes, not
feature changes):

- `CALLWAIT.C` — 'Stats' undefined (the missing STATS control OBJ, the
  same gap the pure 15.3 PWA source has), plus a tossmisc.h language
  declaration mismatch.
- `DOORS.C` — UData member access (TotUpldBytes): a header/struct
  resolution between the reconstructed source and the toolkit.

A sample of the main program (INIT, CHAT, COMMAND, INKEY, RECYCLE)
compiles clean, including 15.4 toolkit features (SPACERIGHTAT), so the
reconstruction is sound — it just needs the build-fix pass.

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
