# Native LHA encoder — DESIGN

**Goal**: fold LH5-family compression into `redx` itself so `redx pack`
no longer shells out to the vendored `lha 1.14i` binary. Single
self-contained `redx` binary.

## Current state (as of install v1.8.1 scaffolding)

`red_pack.c::lha_compress()` invokes `system("lha aq5 out.lzh in")`,
parses the level-0 LHA header from the output file, extracts the raw
compressed payload, and prepends WCSC's 2-byte method-0x000B prefix.

Works. Ships. But requires `lha` on PATH or `--lha-path`.

## Correct acceptance bar

**NOT** byte-identical output vs lha 1.14i. Two conformant LH5
encoders virtually never produce byte-identical streams for the same
input — algorithm choices (hash chain depth, tie-breaking in longest-
match search, huffman tree construction order, canonical vs
non-canonical codes) all produce different bytes while still being
valid LH5.

**Actual acceptance bar** (all four must pass):

1. **Round-trip integrity**: `redx pack --native` produces a `.RED`
   archive that `redx list` and `redx extract` can round-trip, and
   the extracted bytes are byte-identical to the input.

2. **Interop out**: The LH5 stream inside the `.RED` records is a
   valid level-0 LHA archive when extracted with `-lh5-` header
   framing. Verify by extracting our records with the vendored
   `lha 1.14i x` command and comparing output bytes.

3. **Interop in**: `redx extract` continues to correctly decompress
   archives produced by lha 1.14i (regression test — the 6 install
   .RED archives all decode byte-perfect today; must still do so
   after the pack side changes).

4. **Compression effectiveness**: within 10% of lha 1.14i's ratio on
   the 6 install archives (`.RED` payload size comparison). Exact
   parity not required.

## Encoder architecture

Based on the vendored lha 1.14i sources under `../../lha/src/`:

  - `slide.c`  — LZSS sliding-window: 4 KB dict + 60-byte lookahead
                 (LH5 spec: window=8192 for -lh5-)
  - `huf.c`    — Static Huffman on top of slide output
  - `maketree.c` / `maketbl.c` — Canonical Huffman tree construction
  - `bitio.c`  — Bit-level output stream
  - `crcio.c`  — CRC-16-IBM + bit I/O helpers

Total: ~1500 lines in the vendored version, ~1000-1200 lines for the
encode-only extraction (decoder paths dropped).

## File layout (this directory)

    native_lha/
    ├── DESIGN.md               (this file)
    ├── ACCEPTANCE.md           (acceptance tests, TODO)
    ├── lh5_encode.h            (public API, TODO)
    ├── lh5_encode.c            (encoder implementation, TODO)
    ├── lh5_encode_slide.c      (LZSS pass, TODO)
    ├── lh5_encode_huf.c        (Huffman pass, TODO)
    └── lha_container.c         (level-0 header emission, TODO)

## Public API (planned)

```c
/* Compress src to dst using LH5 (-lh5-) algorithm.
 * dst_cap must be >= src_len + 16 (worst case: STORED + overhead).
 * Returns number of bytes written to dst, or 0 on failure.
 * Sets *crc16 to CRC-16-IBM of src (per LHA spec).
 */
size_t lh5_encode(const uint8_t *src, size_t src_len,
                  uint8_t *dst, size_t dst_cap,
                  uint16_t *crc16);

/* Wrap raw LH5 stream in level-0 LHA container.
 * Returns pointer to malloc'd buffer, caller frees. Sets *out_len.
 */
uint8_t *lha_wrap_level0(const uint8_t *lh5_stream, size_t lh5_len,
                         uint32_t unc_size, uint16_t crc16,
                         const char *filename, size_t *out_len);
```

`red_pack.c::lha_compress()` becomes a thin wrapper: call
`lh5_encode()`, then `lha_wrap_level0()`, then prepend the WCSC
2-byte method-0x000B prefix.

## Phase plan

- **install v1.8.1**  (this commit) — scaffold, DESIGN.md,
  ACCEPTANCE.md, empty stubs. `red_pack.c` untouched — shell-out
  path continues to work.
- **install v1.8.1.1** — LZSS slide pass + interop-in regression
  test.
- **install v1.8.1.2** — Huffman pass + full round-trip test.
- **install v1.8.1.3** — Level-0 LHA container emission + interop-out
  test (external lha decodes our output).
- **install v1.8.1.4** — Wire into `red_pack.c` behind `--native`
  flag (opt-in). Shell-out remains default.
- **install v1.8.1.5** — Once all 6 install .RED archives round-trip
  successfully, flip default to `--native`; shell-out becomes
  `--legacy-lha`. Vendored `lha 1.14i` stays for verification.

## Why not port a permissive-licensed existing encoder?

Considered:
- **zlib** — DEFLATE, not LZSS+Huffman. Different bitstream. Would
  not produce valid LH5 output.
- **miniz** — same, DEFLATE only.
- **Yoshi's original LH5 code** — this IS what's vendored under
  `../../lha/src/`. Under a permissive Japanese-BSD-like license,
  compatible with GPLv3 project.

Extracting the encode paths from Yoshi's code (with clear attribution)
is the correct approach. Not clean-room rewriting from spec — that
would be strictly worse (higher bug risk, no parity benefit).

## Attribution

Yoshi's original LHa/LHarc code:
  Copyright (C) Haruyasu Yoshizaki 1988-1991
  Distribution terms in `../../lha/README.md`.

Any code extracted from `../../lha/src/` retains the original headers.
GPLv3 wrapper for our container / method-dispatch / .RED framing code.
