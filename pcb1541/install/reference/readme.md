# pcb1541/install/reference/ — Clark's original installer artifacts

Byte-exact copies of the two files our C reimplementation of the
PCBoard installer must match against, extracted once from
`../INSTALL.zip` and tracked here for stable diffing.

**Do not modify.** These are the untouched targets. Any change to
either file breaks the whole install v1.10 acceptance chain.

## Files

| File | Size | md5 | Role |
|---|---:|---|---|
| `INSTALL.EXE` | 338,548 | `5239767bfced0689a1da961799a0f79c` | Clark's compiled 16-bit installer binary. NE format (Windows/OS2 Family API). Header string: `$Logfile: W:/master/install/main.c_v $`, `INSTALL Ver ... Copyright (c) 1987-1995`. |
| `INSTALL.DAT` | 42,294  | `cca38d36b8f0a76fd4bb01448f41af69` | Clark's original installer script — the DAT-language program that INSTALL.EXE interprets to lay down the 481 files of a PCBoard install. Uses 72 unique `@`-directives (60 after case-fold). |

## Provenance

Extracted 2026-09-04 from `pcb1541/install/INSTALL.zip` (Disk 1 of
Clark's original PCBoard 15.3 install-disk set; 12 files, md5
`baseline` verified in install v1.7.1 recovery). The zip stays
tracked as the source-of-truth archive; these two extracted files
are just for convenient diffing without re-unpacking every session.

## Use

- **`INSTALL.DAT`** drives `pcb1541/install/src/install.c` — our
  reimplementation reads this file, parses the `@`-directives, and
  executes them to lay down a target tree matching the reference at
  `pcb1541/install/dist/target/`. See
  `docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md` for the
  canonical directive reference.
- **`INSTALL.EXE`** is the parity target — install v1.10.6 does a
  disassembly parity pass to confirm our @-command semantics match
  Clark's binary. Byte-exact rebuild of `INSTALL.EXE` itself is
  deferred to install v1.11+.

## Related

- `../INSTALL.zip` — full 12-file Disk 1 archive (source of these
  extracts + the 8 `.RED` archives that hold the 481 install files)
- `../src/install.c` — our C reimplementation (Phase 27 stub as of
  install v1.10.0 — grows through v1.10.1 → v1.10.6)
- `../dist/target/` — the 481-file target tree the installer produces
  (byte-verified in install v1.7.1)
- `../archivers/redx/` — .RED archiver CLI (used by install.c to
  decompress `@BeginLib` blocks)
- `../RUNTIME-DEPS.md` — external tool dependencies (FOSSIL,
  archivers, DSZ/GSZ)
- `../../../docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md` —
  canonical @-command reference

--install v1.10.0, 2026-09-04
