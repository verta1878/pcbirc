# toolkit/pplc — PCBoard Programming Language Compilers

## Source code

The PPLC compiler source (Clark's C++) lives in the PWA branches:

| Location | Version | Era |
|---|---|---|
| `pcb153/SOURCE/PPL/` | 3.30 | PCBoard 15.3 (pwa153) |
| `pcb153/upd154/SOURCE/PPL/` | 3.40 | PCBoard 15.4 update |
| `pcb154/MAIN/SOURCE/PPL/` | 3.40 | PCBoard 15.4 (pwa154) |

Key file: `NEWSCR.CPP` — contains the version `#define`s:
```
#define HDR_TXT "PCBoard Programming Language Executable  3.30\x0D\x0A\x1A"
#define CUR_PPE_VER  330
```

To build any PPLC version, change these two defines and compile with
BC 3.1 under DOSBox-X via `pcb153/153/PPLC.MAK`.

Written by Scott Dale Robison. Copyright (C) 1993-96 Clark Development.

Note: `pcb1541/PPL/pplengine/pplc/` is a **Rust rewrite** for the
15.41 IRC fork — not Clark's C++ source.

## Shipped binaries (reference)

Three shipped PPLC binaries are in the repo inside
`reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`
(sub-path `CSBACKUP-Clean/PCB/PPLC3??.EXE`, dated 1997-08-01):

| Version | Binary | Size | MD5 |
|---------|--------|-----:|-----|
| 3.00 | PPLC300.EXE | 121,558 | `86325a0f3ff8006c5a19763fb8e1c267` |
| 3.10 | PPLC310.EXE | 118,328 | `18409677b4a40497001c2848e0616ce2` |
| 3.20 | PPLC320.EXE | 222,176 | `2a23e7686f79ea07bbb3c4d04e064a75` |

Also on the build image (installed by PCBoard):

| Version | Location | Size |
|---------|----------|-----:|
| 1.00 | `pcb1541/install/dist/target/PPLC100.EXE` | 67,786 |
| 3.30 | `pcb1541/install/dist/target/PPLC330.EXE` | 191,918 |

**Decision:** build PPLC from source rather than extract the shipped
binaries. We own the compiler end-to-end from Clark's source.

## Output directories

Each version has an `out/` directory for compiled `.PPE` files:
- `3.00/out/` — PPEs compiled with PPLC 3.00
- `3.10/out/` — PPEs compiled with PPLC 3.10
- `3.20/out/` — PPEs compiled with PPLC 3.20

This makes byte-exact comparisons across compiler versions trivial:
compile the same `.PPS` with each version, diff the outputs.

## Building PPLC from source

```
# Under DOSBox-X with BC 3.1:
# 1. Edit pcb153/SOURCE/PPL/NEWSCR.CPP:
#    HDR_TXT → "...  3.20\x0D\x0A\x1A"
#    CUR_PPE_VER → 320
# 2. Run:
MAKE -f PPLC.MAK
```

Same build root as INSTALL.EXE (BC 3.1, TLINK 5.1, DOSBox-X).

## Samples

20 sample PPL scripts (.PPS source + .PPE Clark-compiled binaries) plus
2 helper .BATs shipped by Clark with PCBoard 15.41. See
[`samples/SAMPLES.md`](samples/SAMPLES.md) for the full catalog.

These serve two purposes:
- **Tutorial** — HELLO1-7 walk through the language progressively.
- **Round-trip parity target** — recompiling each `.PPS` should produce
  a `.PPE` byte-perfect against Clark's originals in `samples/`.
