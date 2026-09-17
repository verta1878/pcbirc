# OUT/ — Canonical Binary + Data Output

Binaries by version, data by program. No hunting for releases.

```
OUT/
  pwa153/     15.3 PWA binaries (Borland) — build from pcb153/
  pwa153/upd154/  15.4 PWA — our rebuild target
  clark-original/ Clark's shipped 15.4 beta binaries (12 EXEs, no
                  source) — the byte-match reference for the rebuild
  delta154/   15.4 Delta binaries (OpenWatcom) — 15 EXEs
  irc1541/    15.41 IRC binaries (openwatcom2irc) — future
  data/       ALL data files, by program — no EXEs, no source
  lib/        toolkit libraries
  support/    shared runtime data (PCBOARD.SER, PCBSM.CLR/CNF, ENDPCB)
```

## bins/ vs the version top level

Program EXEs live at the **version top level** (OUT/pwa153/,
OUT/pwa153/upd154/, OUT/delta154/, OUT/irc1541/).

Each `bins/` subdirectory is for **SDK example binaries** only — the small
buildable sample add-ons (a door, a monitor, a front-end/echo demo, sample
.PPEs) that prove the SDK builds end to end. Separate from OUT/lib/ (the
toolkit libraries). Shipping binaries do not go there.

## OUT/data/

All data files the installer ships, grouped by program — never by
duplicating install-tree directory structures, and never mixed with EXEs
or source. One subdirectory per program or data class: ic12/, commdrv/,
pcbmail/, pcbos2/, help/, doc/, ppl/, gen/, fido/, graphics/, files/,
main/, conferences/, root/, dl01/.

Where a program's own layout has name collisions across subdirectories
(PCBIC ships two different PPP files and two different SLIP files), the
subdirectories are mirrored rather than flattened. INSTALL.DAT maps each
file to its install target.

## Per-version layout (Convention A)

Each version owns its source, toolkit, and binaries:

| Version | Source | Toolkit | Binaries |
|---|---|---|---|
| 15.3 PWA | pcb153/SOURCE | toolkit/pwa153 | OUT/pwa153 |
| 15.4 PWA | pcb153/upd154/SOURCE | toolkit/pwa154 | OUT/pwa153/upd154 |
| 15.4 Delta | pcb154/MAIN/SOURCE | toolkit/delta154 | OUT/delta154 |
| 15.41 IRC | pcb1541/ | toolkit/irc1541 | OUT/irc1541 |

## Status

- OUT/pwa153 — empty pending a build from clean 15.3 source
- OUT/pwa153/upd154 — our 15.4 PWA rebuild target. Holds the 6 PCBIC v1.2
  binaries (Pcbic.exe, Pcbic2.exe, PCBICCFG.EXE, PCBICEVT.EXE, TESTIC.EXE,
  TESTIC2.EXE); the other 15.4 EXEs land here when the build-fix pass runs
- OUT/clark-original — **RECOVERED.** Clark's shipped 15.4 beta binaries,
  12 EXEs: 11 DOS + 1 OS/2, newest build stamp 04/22/97. Binaries only, no
  source. Lost in the 2026-08-25 toolkit restructure (deleted from OUT/DOS/
  and OUT/OS2/ in e4181e5), restored from e4181e5^ and cross-verified
  against the original 1997 distribution. The OUT/pwa154/ slot named in
  older notes never existed. SHA256 sums and full provenance in
  clark-original/README.md. Do not overwrite — this is the byte-match
  reference the rebuild must reproduce
- OUT/delta154 — 15 EXEs (OpenWatcom)
- OUT/irc1541 — future (openwatcom2irc)
- OUT/data/ic12 — 32 PCBIC data files (Phase A)

All binaries verified executing under DOSBox-X.

## OUT/pwa153/upd154/

The 15.4 PWA upgrade binaries — compiled from pcb153/upd154/ (Clark's
15.4 reconstructed source). Kept parallel to OUT/pwa153/ (the base 15.3
outputs), matching the source relationship. Still pending the upd154
build-fix pass for everything except PCBIC. See pcb153/upd154/build/README.md.

IC is the 15.4 internet upgrade and belongs to this leg; its source is
pcb153/upd154/SOURCE/IC/ and its data is OUT/data/ic12/. Delta 15.4
inherits the code — see the PCBIC build draft for how.
