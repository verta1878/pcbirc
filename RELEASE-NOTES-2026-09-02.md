# pcbirc — full repository release, 2026-09-02

## What this is

The complete `verta1878/pcbirc` working tree at commit b143f21+
(see CHECKSUMS.txt for exact hash), split into two zip parts:

- **Part 1** (this zip): source, docs, toolkit, patches, drivers,
  and smaller reference archives — all 9,600+ tracked files EXCEPT
  the roysac PCBoard preservation snapshot.
- **Part 2** (`pcbirc-full-part2-*.zip`): the `reference/roysac/`
  preservation snapshot (CSBACKUP.ARJ + PCB1522 disks + related
  historical archives — 314 MB of already-compressed ZIPs/ARJs).

Both parts unzip into the same `pcbirc/` root. Just extract both to
the same parent directory and the trees merge naturally.

## Notable in this drop

- **v1.2 pcbdcom SHIPPED** (all 5 features):
  Arnet backend, ser_rs232 shim, INT 14h extensions, TSR fix, SDK.
  3,829 lines GPLv3 across 15 .c + 6 .h files, 8 card family backends.
  See `pcb154/pcbdcom/` for the canonical location.

- **.RED decompressor SHIPPED** (Python v1.0 + C port, 9/10 pairs):
  Reverse-engineered Yoshi LHA -lh5- with 2/3-byte WCSC header prefix.
  See `archivers/redx/`. All 9 COMMDV*.DRV device drivers extract
  byte-perfect. COMMDRV.EXE (10th vector) fails at 7398 bytes — this
  is confirmed WCSC-specific chunking (Yoshi's own reference tool
  fails identically at the same spot). Workaround: extract COMMDRV.EXE
  from CSBACKUP.ARJ (in Part 2).

- **v1.2 SDK** in `toolkit/pwa154/pcbdcom/` — 32 files, drop-in
  replacement for COMMDRV.OBJ for anyone linking against it.

## Contents (Part 1)

    Root:               BUILD_*.BAT/CMD, LICENSE, CHECKSUMS trio,
                        TOMORROW.md, FILE_ID.*, DOSBOXX.ZIP
    MAIN/               PCBoard 15.4 source + build scripts
    pcb154/             Canonical build tree (pcbdcom lives here)
    pcb153/             15.3 baseline
    pcb1541/            15.4b1 preservation
    toolkit/            PCBoard toolkits (pwa153 + pwa154 + pcbdcom SDK)
    OS2TK/              IBM OS/2 Toolkit
    drivers/            Device driver sources
    devtools/           BC 3.1, MASM, TASM, TLINK etc for cross-compile
    patches/            15.3→15.4 patch (canonical)
    archivers/          arj, lha (Yoshi 1.14i), redx (.RED decompressor)
    docs/               Documentation
    reference/          (minus roysac — see Part 2)
    attic/              Historical artifacts
    todo/               Open work items
    pcbcbase/, OUT/     Build outputs / scratch

## Apply

1. Extract both zips to the same parent directory
2. Verify: `md5sum -c CHECKSUMS.md5` (should be all OK)
3. `git init && git add -A && git commit -m "pcbirc snapshot 2026-09-02"`
   OR: use GitHub Desktop -> Add existing local repo

## Known issues

- COMMDRV.EXE decompression (see above) — 9/10 pairs work; last one
  needs interactive Ghidra to reverse the WCSC I/O chunking layer.
  Not blocking any pcbdcom functionality.

## License

- pcbdcom: GPLv3
- PCBoard/PPE source: Clark Development Corp, released via
  Clark's estate
- LHA: Yoshi Watazaki (public domain, referenced only)
- Reference archives (part 2): historical preservation, respective
  original owners

## Contact

Antonio Rico (verta1878@github)
