# OUT/support — shared PCBoard runtime support files

Generic PCBoard/PCBSetup runtime files that are NOT version-specific,
so they live here (parallel to the per-version binary dirs OUT/pwa153,
OUT/pwa153/upd154, etc.) rather than being duplicated in each:

- `ENDPCB`       — abnormal-exit / shutdown message text
- `PCBOARD.SER`  — serial-number data
- `PCBSM.CLR`    — PCBSetup colour definitions
- `PCBSM.CNF`    — PCBSetup configuration

These are support/data files produced or used at runtime, kept together
because they're shared across versions.
