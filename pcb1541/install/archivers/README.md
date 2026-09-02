# pcb1541/install/archivers/ — LHA + redx for .RED (un)packing

> **Location:** lives under `pcb1541/install/` because the installer is
> shared across all PCBoard variants (PWA 15.22, Delta 15.4, 15.41, ...)
> and every variant needs redx + lha to (un)pack the `.RED` disk archives.


This directory contains a vendored copy of **LHa for UNIX 1.14i**
(Koji Arai's autoconfiscated branch of Tsugio Okamoto's original),
imported for use as the reference LH5-family decoder underlying the
crew's `.RED` extractor and installer-recreation work.

Upstream: <https://github.com/jca02266/lha> — imported as of latest
main at clone time. `.git` stripped to keep this as a plain vendored
snapshot, not a submodule. If we need to update, re-clone shallow
and diff.

## Why LHA is in the repo

The PCBoard installer (`INSTALL.EXE`) shipped a proprietary container
format (`.RED`, magic "RR") that wraps LH5-family compressed data with
a Clark-specific framing. To extract or create `.RED` files without
running Clark's Windows/DOS binary, we need our own decoder.

Rather than port the decompiled `expand` routine out of `INSTALL.EXE`
(large, noisy, mixed with data — see `analysis/` scratch work),
we adopt the mature, portable LHA 1.14i source as the base LH5
decoder. The delta between LHA-standard LH5 framing and Clark's
`.RED` framing is small (custom 24-byte per-record header with
compressed size, uncompressed size, CRC16, and method byte) and
can be handled in a thin wrapper.

## What lives here

- **`lha/`** — upstream 1.14i-ac source tree, unmodified. Reference.
- (planned) `redx/` — the crew's `.RED` extractor + creator, built
  on top of `lha/src/slide.c`, `huf.c`, `dhuf.c`, `shuf.c`, `bitio.c`.
  Handles the Clark framing and calls into LHA's LH5 core.
- (planned) `redlib/` — packaged as a static library (for MSC 7.0
  and Watcom targets) so PCB installer recreation and pcbdcom
  tooling can link against one implementation.

## Scope of this phase

**Phase RED-1: `.RED` extractor.** Cross-platform C program that
reads a `.RED` file and unpacks all records. Compiles under host
GCC/clang for development, MSC 7.0 for DOS deployment.
Deliverable: `redx.exe` extracts COMMDRV.RED, PCBOARD.RED,
PCBOARD2.RED, PCBMAIL.RED, PPLC.RED, PCBCFGS.RED byte-exact.

**Phase RED-2: `.RED` creator.** The reverse — pack a directory
of files into a `.RED` archive using Clark's framing + LH5.
Deliverable: `redc.exe` produces `.RED` files that Clark's original
`INSTALL.EXE` accepts without complaint.

**Phase RED-3: Integrate with PCB installer recreation.** Once
`redlib` is stable, the crew-native PCB installer (recreated
separately) uses it for its container operations. This lets us
distribute PCBoard on our own installer end-to-end.

## License

LHA 1.14i ships under the "Original License" — a permissive
Japanese-style license from Yooichi Tagawa, Nobutaka Watazaki,
Tsugio Okamoto, and Masaru Oki. Network redistribution is
explicitly permitted; the only ask is prior notification to the
author when included on offline media (CD-ROM, etc.). Debian,
FreeBSD ports, Arch AUR, and most Linux distros ship it. See
`lha/LICENSE` (or the "License" section of `lha/olddoc/README`)
for the full statement.

The crew's `.RED` extractor + creator (files added in this
directory outside `lha/`) will be **GPLv3** to match the pcbdcom
and other crew driver licensing posture.

## Related work

- `/tmp/commdrv-work/ANALYSIS.md` (host scratch, out of repo) —
  full `.RED` format decoded (record header layout, LH5 method
  identifier byte, per-file CRC16, offsets and sizes verified for
  all 18 records in COMMDRV.RED)
- `pcb1541/install/INSTALL.zip` — Clark's original installer +
  `.RED` files (reference material shipped with pcbirc)
- `pcb1541/pcbdcom/` — downstream consumer for driver-source work
  once commdrv extraction produces analyzable binaries
