# Ghidra-ready package for COMMDRV.EXE decoder fix

## Goal

Fix the 7398-byte divergence in `red_decompress.c` when decoding
COMMDRV.EXE from `COMMDRV.RED`. Both our decoder AND Yoshi's own
reference `lha` binary fail identically at this exact point, so the
issue is a WCSC-specific chunking convention in the I/O layer wrapping
the LHA core (functions named `kick_char` / `f_ram` / `flushram` per
the debug strings we found).

## Session budget

30-60 minutes of Ghidra GUI work.

## Inputs (this directory)

| File                       | Purpose                                     |
|----------------------------|---------------------------------------------|
| `install_unpacked.exe.md5` | Verify you have the right unpacked binary   |
| `expected_divergence.txt`  | Exact bit-position + code where we diverge  |
| `func_addresses_hint.txt`  | Known segment map + string offsets to help  |
| `test_vector/`             | COMMDRV.EXE.payload + .oracle for roundtrip |
| `patch_template.c`         | Skeleton patch — fill in from findings      |

## Steps

1. Get `install_unpacked.exe` from previous session (275,872 bytes,
   MD5 in this dir). If lost, regenerate with
   `/tmp/ghidra-fresh/emu_v3.py` from prior work.

2. Open in Ghidra 11.x.
   - Language: `x86:LE:16:Real Mode`
   - Load base: paragraph `0x100` (image base 0x1000)
   - Auto-analyze

3. Find the three functions via string references:
   - Search → Program Text → `flushram`
     Right-click → Xrefs → finds the `error("Internal error in flushram")` call
     Follow xref backward to find `flushram()` entry
   - Same for `f_ram 1` and `f_ram 2` (finds `f_ram()`)
   - Same for `kick_char`

4. Decompile each (Window → Decompile):
   - `kick_char` (~40 lines expected)
   - `f_ram` (~80 lines expected)
   - `flushram` (~80 lines expected)

5. Look for:
   - Any 4KB (0x1000) constant in comparisons
   - Any bit-buffer state reset
   - Any "discard N bits" or "byte-align" operation
   - Any counter that wraps at 4096

6. Fill in `patch_template.c` with the pattern (probably 5-10 lines
   added to `red_decompress.c` near the `fillbuf()` function).

7. Test:
   ```
   cd archivers/redx
   gcc -O2 -Wall -o redx redx.c red_decompress.c red_pack.c
   ./redx extract refwork/pairs/COMMDRV.EXE.payload
   # Should now show OK 90827 bytes
   ```

## What we already know (spare Ghidra time)

- Divergence at output byte **7398**, compressed input byte **4096**
- Bit position when it fires: **32761** (7 bits into new state)
- Code being decoded at divergence: `256` (min copy, 3-byte length)
- Kraft-check fails on next `read_pt_len()` after this point, confirming
  bit alignment lost
- Segment map: `0x100` image, `0x1089` LHA lib (~180 funcs),
  `0x1f91` C runtime, `0x2d91` data, `0x2e71` aux strings
- Real work function: `0x100:0xf42c` (file offset `0x1044c`)
- Yoshi's LHA source in `../lha/src/` has stock `kick_char`-like
  buffer flushing in `crcio.c` — compare WCSC's to this baseline

## What we've ruled out (don't re-try)

- Byte-align at 4KB boundary
- Insert/remove byte at every position 4093-4101
- All DICBIT values 12-16
- All NP/PBIT combos for -lh4/5/6/7-
- Chunked stream with header sizes 0-4
- Force-restart LHA block at 4KB

## UPDATE 2026-09-03: function addresses located headless

**FOUND_FUNCTIONS.md** in this directory contains the file offsets of
all 4 candidate functions (kick_char, f_ram, and 2 unknowns including
likely flushram), plus disassembled pseudocode for kick_char and f_ram.

This changes the Ghidra session shape significantly:
- Skip step 3 (string reference hunting) — addresses are known
- Jump directly to file offsets 0x1e356, 0x1e3e8, 0x1e420, 0x1e4a4
- Focus decompile time on the 3-caller function at 0x1e4a4 (most
  likely flushram) and the main decompressor at 0x1e52a

Session budget likely reduces to 15-30 minutes.

