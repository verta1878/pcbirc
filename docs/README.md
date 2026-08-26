# Documentation — Shared / General

Per-version docs now live inside each version's source tree:

- `pcb153/docs/` — 15.3 PWA docs, including the full 15.3 manual
  (`pcb153/docs/manual/`: INTRO, PCBOARD, SETUP, SYSGUIDE, INSTALL, etc.)
- `pcb153/upd154/docs/` — Clark's 15.4 docs (WHATSNEW, HISTORY, REPORT,
  SYSOP_154, PCBoard_154_Reference)
- `pcb154/docs/` — Delta crew notes (DELTA154-CHANGES.md)

This directory holds only manuals and guides that span versions:

- PCBoard_Manual_Scanned.pdf, pcb_ppl_manual_scanned.pdf
- ppldevelopmentkit.pdf, PCBoard_1541_Addendum.pdf
- Pcbguide.zip, calguide.zip, pcbtutor-3.zip, Pcbmodem.zip
- DOORHELP.ZIP, "Pcb Users Help.zip", SaltAir_Bulletins.ZIP
- pcb150docs.zip, PCBDocs.zip (source doc archives)
- fido/, saltair/, sysop/, sysop15/ — topic collections
- PCBXDOT/ — additional reference

## Version Model

Four versions (see MAIN/DELTA-MODEL.md):
- 15.3 PWA (pcb153/) — base, Borland
- 15.4 PWA (pcb153/upd154/) — Clark's 15.4, Borland. Patch: patches/15.4-pwa.patch
- 15.4 Delta (pcb154/) — crew's ongoing work, OpenWatcom
- 15.41 IRC (pcb1541/) — our additions, openwatcom2irc

Per-version docs were moved into their source trees (a move + dedup,
nothing deleted) — every file lives in `pcb153/docs`, `pcb153/upd154/docs`,
or `pcb154/docs`.
