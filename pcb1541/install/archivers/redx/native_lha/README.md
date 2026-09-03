# native_lha/ — Native LHA -lh5- encoder (scaffold)

Being folded into `redx pack` to eliminate the shell-out to
`lha 1.14i`. Currently **scaffold only** — actual encoder lands
in install v1.8.1.1 through v1.8.1.5.

Read in order:

1. [`DESIGN.md`](DESIGN.md) — architecture, phase plan, why not
   miniz/zlib.
2. [`ACCEPTANCE.md`](ACCEPTANCE.md) — the four tests that must pass
   before native encoder becomes default. Byte-parity vs lha 1.14i is
   NOT the bar (explained inside); round-trip integrity + interop are.
3. [`lh5_encode.h`](lh5_encode.h) — frozen public API.
4. [`lh5_encode.c`](lh5_encode.c) — stub returning 0 (fall through
   to shell-out).

## Status

| Milestone | Status |
|-----------|--------|
| Scaffold + API + design docs | ✅ install v1.8.1 |
| LZSS slide pass              | ⏳ v1.8.1.1 |
| Huffman pass                 | ⏳ v1.8.1.2 |
| Level-0 LHA container        | ⏳ v1.8.1.3 |
| Wire into red_pack.c behind --native flag | ⏳ v1.8.1.4 |
| All acceptance tests pass, flip default   | ⏳ v1.8.1.5 |

Until v1.8.1.5: `red_pack.c` shells out to `lha 1.14i` as it has
since install v1.6.0. Native encoder is opt-in for testing only.

## Why this is broken into 5+ commits

Byte-perfect round-trip decode of arbitrary LH5-encoded input is a
subtle correctness problem — Yoshi's original code has 10+ years of
bug fixes baked in. Extracting the encode paths cleanly means porting
piece by piece with a working test at each step. Rushing = subtle
bugs that break parity on records that decompressed fine yesterday.
