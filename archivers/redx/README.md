# redx — WCSC .RED archive tools

Read + write PCBoard install-disk archives (`COMMDRV.RED`, `PCBOARD.RED`,
`PPLC.RED`, etc.).

## Build

```
gcc -O2 -Wall -o redx redx.c red_decompress.c red_pack.c
```

## Commands

### `redx list <archive.RED>`

Show all records with filename, method, compressed size, uncompressed size,
and CRC16.

### `redx extract <archive.RED> [name]`

Extract all records (or just one named record) into the current directory.

Supports:
- Method 0x0001 (STORED) — byte-perfect always
- Method 0x000B (LHA -lh5- + WCSC prefix) — byte-perfect on 9/10 test vectors.
  The 10th (`COMMDRV.EXE`-shape files with WCSC chunking) fails at ~7398 bytes;
  see `refwork/decompress_v1.0.py` APPENDIX for details.

### `redx pack [--stored | --lha=<path>] <out.RED> <file>...`   *(NEW v1.4)*

Build a new `.RED` archive from the listed files.

- **Default**: `--stored` — no compression, always works, larger output
- **`--lha=<path>`**: use LHA compression via external `lha` binary (Yoshi's
  1.14i from `archivers/lha/src/lha` — build with `autoreconf -i && ./configure && make`)
- Falls back to STORED silently if `lha` fails or compression doesn't help
- Files are stored under their basename; full paths are stripped

## Round-trip guarantee

Any `.RED` written by `redx pack` can be read back by `redx extract`.
Verified 9/9 byte-perfect on the driver test set.

For the full COMMDRV.RED archive (both directions):

```
mkdir extracted && cd extracted
redx extract ../COMMDRV.RED           # 9/10 succeed (COMMDRV.EXE fails)
redx pack --lha=/path/to/lha ../repacked.RED *
diff <(redx list ../COMMDRV.RED) <(redx list ../repacked.RED)  # sizes match
```

## Files

| File                        | Purpose                                    |
|-----------------------------|--------------------------------------------|
| `redx.c`                    | CLI entry (list / extract / pack)          |
| `red_decompress.c`          | LHA-lh5 + WCSC prefix decoder              |
| `red_pack.c`                | STORED + LHA archive writer                |
| `red_test.c` / `redx_test.c`| unit / integration tests                   |
| `refwork/`                  | reverse-engineering work + test vectors    |

## License

GPLv3. Yoshi LHA source (in `../lha/`) is referenced as public domain.

## Known limits

- Method 0x000B `COMMDRV.EXE` decompression fails at 7398 bytes (WCSC chunking
  layer not reverse-engineered — needs interactive Ghidra time on
  `kick_char` / `f_ram` / `flushram` functions).
- Only STORED and LHA methods implemented. `.RED` archives with other methods
  will list-only (unknown method reported).
