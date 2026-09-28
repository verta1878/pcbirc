# OUT/pwa153 — PWA 15.3 binaries

Empty pending a build from the clean 15.3 source (pcb153/SOURCE).

Build 15.3: PCBOARD.EXE, PCBOARD2.EXE, PPLC.EXE, etc., with Borland
C++ 3.1 (see pcb153/README.md for flags).

## Note on the 15.4 binaries that used to sit here

The binaries previously in OUT/pwa153/ actually reported **v15.4** —
they were Clark's shipped 15.4 EXEs (the reference for the
reverse-engineered 15.4 source), not 15.3 builds. They now live under
`OUT/pwa153/upd154/clark-original/` alongside the 15.4 PWA rebuild
target, since 15.4 PWA nests under pcb153 as `upd154`.

So OUT/pwa153/ (top level) is reserved for genuine 15.3 output, and
`OUT/pwa153/upd154/` holds the 15.4 PWA world (rebuild + clark-original
reference).
