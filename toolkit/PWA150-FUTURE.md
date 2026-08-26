# pwa150 — Old 15.x Toolkit (future goal)

## The idea

Build the older PCBoard 15.x toolkit (pre-15.3) so that someone running
an earlier version has an upgrade path to the 15.3 toolkit. A sysop or
door author on 15.0/15.1/15.2 could then move forward cleanly instead
of being stranded on a toolkit we don't provide.

This matters especially because the IC (Internet Connectivity) works
with 15.2x (its RUNINET.PPE is PPL 3.20, the 15.22 era). Anyone
maintaining a 15.2x system with IC would benefit from a matching
15.2x-era toolkit and a documented path up to 15.3.

## Status: NOT yet buildable

We do NOT have the 15.0/15.1/15.2 toolkit SOURCE. PWA preserved 15.3
onward; earlier versions' source isn't in our tree. What we have:

- 15.3 toolkit source (toolkit/pwa153) — our earliest full source
- pcb150docs (15.0 documentation) — reference only, no source
- reference/mpoli_bbs/CTUB_150.ZIP — a 15.0-era file (to examine)

## What "pwa150" would require

1. Locate PCBoard 15.0/15.1/15.2 toolkit source (or reconstruct from
   a 15.2x distribution if one can be found — same hunt as PPLC 3.20
   for the IC).
2. Set it up as toolkit/pwa150 with its own SOURCE/H/CFG, following the
   same per-version structure as pwa153.
3. Build its libraries the same way (3 compilers x 4 models) using the
   period-correct compilers.
4. Document the 15.0/15.1/15.2 -> 15.3 toolkit upgrade (what changed,
   what a door author must adjust).

## Relationship to the IC work

Finding a 15.2x distribution would serve BOTH goals at once:
- PPLC 3.20 -> byte-exact IC reconstruction (RUNINET.PPE)
- 15.2x toolkit source -> pwa150 / the upgrade path

So the 15.2x hunt is shared between IC reconstruction and pwa150. Worth
pursuing once for both.

## Priority

Lower than completing the 15.3 SDK (pwa153: PCBKBC done; PCBKIT + PCBKMS
pending). Recorded here so the upgrade-path idea isn't lost. Revisit
after the 15.3 three-compiler matrix is complete and once a 15.2x
source is located.
