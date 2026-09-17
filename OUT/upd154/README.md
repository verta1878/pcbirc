# OUT/pwa153/upd154 — rebuilt 15.4 PWA upgrade binaries

Where our **recompiled-from-source** 15.4 PWA upgrade binaries will be
released — built from pcb153/upd154/ (Clark's 15.4 reconstructed source)
with Borland C++ 3.1.

Empty until the upd154 build-fix pass lands and BLDUPD154.BAT runs.
See out/upd154/README.md.

## Layout

- `clark-original/` — **Clark's original shipped 15.4 .EXEs** (the 15
  binaries: PCBOARD.EXE, PCBSETUP.EXE, PPLC.EXE, ...). These are the
  reference — what our rebuild must match. Moved here from the old
  OUT/upd154/ slot.
- (top level) — where **our rebuild** of those binaries from the
  reconstructed source will land, so we can compare byte-for-byte
  against clark-original/.
