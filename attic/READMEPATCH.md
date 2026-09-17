# Patches Attic — Superseded

These patches are from earlier project phases. They were generated
against different baselines and directory layouts (various PCBSRCV
versions, scratch work dirs like /home/claude/cdrive, /tmp/pcb_baseline)
and predate the clear 15.3 PWA / 15.4 PWA / 15.4 Delta naming.

## Why moved here

They caused confusion: the filenames mixed "153", "154", "borland",
"watcom" without a consistent model, and their baselines no longer
match the current directory structure. Keeping them in the active
patches/ directory made it unclear which patch was canonical.

## What replaced them

**`../15.4-pwa.patch`** is now the canonical patch:
- 15.3 PWA (pcb153/) -> 15.4 PWA / Clark's (pcb153/upd154/)
- Clean line endings, applies 100% to a fresh 15.3 tree
- Both endpoints build with Borland C++ 3.1

The version model is documented in MAIN/DELTA-MODEL.md.

## What's here (for history)

| File | Was | Superseded by |
|---|---|---|
| 153_to_154.patch | old 15.3->15.4 diff (PCBSRCV/000 baseline) | 15.4-pwa.patch |
| 154_borland_to_154_watcom.patch | Borland->Watcom port diff | (Delta work, pcb154/) |
| 154_watcom_phase0_complete.patch | early Watcom phase snapshot | (Delta work, pcb154/) |
| 1541_additions.txt | 15.41 additions notes | (15.41 IRC phase, later) |
| pcbcp_watcom_port.patch | PCBCP Watcom port | (gap-binary work) |

Kept for provenance. Not for current use.
