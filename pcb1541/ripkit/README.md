# RIPKit v1.20 (kiddo)

PCBoard RIPscrip starter kit for experienced PCBoard SysOps who want
to maintain a RIPScrip front-end for their callers.

**Requires PCBoard v15.21+.**

## Provenance

Extracted from `devtools/Ripkt120.zip` (212KB outer archive) which
contained the outer README/docs plus a nested `RIPKIT.ZIP` (268KB)
with the actual kit contents.

- Outer archive: dated Oct 2, 1995
- Inner kit: same date, kit content dates back to 1994-1995
- FILE_ID.DIZ says: "A RIPScrip starter kit for experienced PCBoard
  SysOps who want to maintain a RIPScrip front-end for their callers"

## Layout

- `README.md` (this file)
- `FILE_ID.DIZ` — original one-line description
- `CONCERN.DOC` — usage/adoption notes
- `HISTORY.TXT` — version history through v1.20
- `INSTALL.DOC` — installation guide
- `UPGRADE.DOC` — upgrade notes
- `kit/` — the extracted RIPScrip kit itself:
  - `GRAFMODE.EXE` — mode-detection helper
  - `*.RIP` — 336 RIPscrip screen definitions
  - `MENUS/` — menu definitions (8 .mnu files)
  - `PPE/` — PPL programs (30 .pps sources + 27 .ppe compiled)
  - `DOCS/` — kit documentation

## Owner

Under kiddo's domain in the crew (protocols/RIP/serial).

## Related work

- RIPscrip protocol work lives under kiddo elsewhere in the tree.
- pcb1541 targets modernizing PCBoard 15.41 IRC edition; a RIPScrip
  front-end for it can start from this v1.20 kit.

## No-image note

Per project rules: no dedicated DOSBox-X image for RIPKIT. Use a
folder mount in `DOSBox-X` when a working environment is needed
(mount this directory as e.g. `C:` and run `GRAFMODE.EXE`).
