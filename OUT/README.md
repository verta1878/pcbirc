# OUT/ — Canonical Binary Output

Binaries by version. No hunting for releases.

```
OUT/
  pwa153/     15.3 PWA binaries (Borland) — build from pcb153/
  pwa153/upd154/  15.4 PWA — rebuild target; clark-original/ holds
                  Clark's 15 shipped 15.4 EXEs (the reference)
  delta154/   15.4 Delta binaries (OpenWatcom) — 15 EXEs
  irc1541/    15.41 IRC binaries (openwatcom2irc) — future
  lib/        toolkit libraries
  support/    shared runtime data (PCBOARD.SER, PCBSM.CLR/CNF, ENDPCB)
```

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
- OUT/pwa153/upd154/clark-original — Clark's 15 shipped 15.4 EXEs
  (Borland, report v15.4); OUT/pwa153/upd154 = our rebuild target
- OUT/delta154 — 15 EXEs (OpenWatcom)
- OUT/irc1541 — future (openwatcom2irc)

All binaries verified executing under DOSBox-X.

## OUT/pwa153/upd154/

The 15.4 PWA upgrade binaries — compiled from pcb153/upd154/ (Clark's
15.4 reconstructed source). Kept parallel to OUT/pwa153/ (the base 15.3
outputs), matching the source relationship. Empty until the upd154
build-fix pass lands. See pcb153/upd154/build/README.md.
