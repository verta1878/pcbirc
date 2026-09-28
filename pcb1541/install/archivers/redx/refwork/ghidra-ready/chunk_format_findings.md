# COMMDRV.EXE 4KB-chunk format — session findings 2026-09-03

## Breakthrough

Yoshi's own `lha` (built from `archivers/lha/src/lha`) decodes the FIRST
4096 payload bytes as a standalone `-lh5-` stream and produces the
first 7397 bytes of the oracle **byte-perfect**.

Method: wrap `payload[3..4099]` (skip 3-byte WCSC prefix, take next
4096 bytes) in a fake level-0 LZH header with `compsize=4096`,
`origsize=7398`, method `-lh5-`. Run `lha xw=... test.lzh`. Result:
CRC error (expected — fake header CRC), but the 7398 extracted bytes
match `oracle[0..7397]` exactly.

**Chunk 1 IS a standard LHA `-lh5-` stream: 4096 compressed → 7398 output.**

## What chunks 2+ are NOT

Tested independently: `payload[4099..8195]` wrapped as a fresh
standalone LZH archive → Yoshi fails with `make_table (case b)` and
produces zero output. Same for various assumed `origsize` values.

So chunks 2+ are neither:
- Standalone LHA streams (Yoshi rejects them as fresh input)
- Simple bit-continuations of chunk 1 (our decoder keeps tables +
  discards bits at 4KB and still fails at 7398)

## Decoder architecture — fully mapped

From disassembly of `install_unpacked.exe`:

| Function | File offset | Segment:Offset | Role |
|---|---:|---|---|
| `decode()` | 0x30a4 | (LHA seg):0x30a4 | 450-byte LZ decompressor loop |
| `fillbuf()` | 0x3266 | ...:0x3266 | 128-byte bit-stream buffer refill |
| `getbits()` | 0x32e6 | ...:0x32e6 | 42-byte peek-N-bits helper |
| `putcode()` | 0x3310 | ...:0x3310 | 66-byte |
| `make_table()` | 0x3352 | ...:0x3352 | 588-byte huffman table builder |
| `read_pt_len()` | 0x2e28 | ...:0x2e28 | 288-byte pt tree reader |
| `read_c_len()` | 0x2f48 | ...:0x2f48 | 348-byte c tree reader |
| main decoder wrapper | 0x1e52a | 0x1089:0xec7a | file-loop, called 6× per record |
| main LHA driver | 0x35e0 | ...:0x35e0 | calls decode() in 8KB chunks |
| init | 0x38e0 | ...:0x38e0 | reads compsize/uncompsize, sets counter |

## State globals (all in the same data segment)

| Address | Purpose |
|---|---|
| `[0x0048]` | c_table (far ptr for 13-bit lookup) |
| `[0x0050]` | pt_pos tree far ptr |
| `[0x0054]` | c_tree base (near) |
| `[0x0056]` | c_tree segment |
| `[0x0058]` | c_tree base (alt) |
| `[0x005a]` | c_tree segment (alt) |
| `[0x18ac]` | `match_remaining` (r) |
| `[0x18ae]` | ring source position |
| `[0x18bc:0x18be]` | compressed_size (32-bit, set from func arg) |
| `[0x18c8:0x18ca]` | **UNKNOWN CHUNK-RELATED VALUE** (set from func arg, added to counter at 0x3c4f) |
| `[0x18d0:0x18d2]` | uncompressed_size copy |
| `[0x5392:0x5394]` | uncompressed_size (remaining, decremented in main loop) |
| `[0x536c]` | bit_buf (16-bit) |
| `[0x53ae:0x53b0]` | 32-bit remaining-input counter (init: compsize + 0x1001 ≈ 50000) |
| `[0x53b6]` | bitcount |
| `[0x59c0]` | blocksize (per-block symbol count) |
| `[0x59d2]` | subbitbuf |

## The specific unknown

At file offset 0x3c4f there's an alternate init path:

```
mov ax, [0x18c8]         ; unknown chunk value
mov dx, [0x18ca]
mov [0x53ae], ax          ; overwrites the remaining-input counter
mov [0x53b0], dx
cmp word [bp-2], 0
jnz skip_add
    mov ax, [0x78]        ; some data segment constant
    add ax, 0xc8          ; + 200
    xor dx, dx
    add [0x53ae], ax      ; add to counter
    adc [0x53b0], dx
skip_add:
```

`[0x18c8]` gets set at file 0x3ade from a function arg `[bp+0x12]`.
That function is the init helper at 0x3a6c.

**This looks like per-chunk counter reload.** If `[0x18c8]` = 4096 and
`[0x78]` + 0xc8 = some overhead constant, then per-chunk counter
starts at 4096 + overhead, decrements per byte read, hits zero →
fillbuf pads zeros → next chunk gets reloaded.

Haven't verified this is called between chunks vs only at init. That's
the next thing to nail down.

## Failed attempts this session (all still stall at output 7398)

1. Byte-align at 4KB boundary crossings
2. Force new-block at 4KB (rc=0 but wrong data)
3. Skip 3-byte header per 4KB chunk
4. Split-stream decode with fresh per-chunk state
5. Chunk size 4093/4094/4095/4096/4097/4098
6. Ring init to zeros vs spaces
7. Chunk-aware fillbuf (added chunk_boundary to bitio_t)
8. Discard bits + keep tables + preserve blocksize

## Next specific probes

1. Trace what CALLS the code at file 0x3c4f. If it's called between
   `decode()` invocations in the main loop at 0x35e0, that confirms
   per-chunk counter reload.
2. Find `[0x18c8]` value at runtime for COMMDRV.EXE. It's set from
   function arg. Trace the caller of the init function.
3. `[0x78]` is never written explicitly — likely a compile-time
   constant in the data segment. Read its value at file offset
   `(data_seg << 4) - 0x1000 + 32 + 0x78`.

## Session 2 (continuation) findings — 2026-09-03

### [0x18c8] is NOT a per-chunk mechanism

Traced [0x18c8]:[0x18ca] through disassembly. Its full role:

1. Set at file 0x3ade from init helper arg `[bp+0x12]` = compsize
2. Used at file 0x3bc0 as a READ CHUNK SIZE for the file-read function pointer at
   [0x18f2]. This is called ONCE from the init path (0x3a33), NOT between decode()
   invocations.
3. The '+ [0x78] + 0xc8' overhead at 0x3c4f is buffer padding, not chunk arithmetic.

So [0x18c8] is the "single fread size" that WCSC uses to slurp the entire compressed
file into memory. Not a chunk mechanism.

### The decoder init is ONE-SHOT, not per-chunk

Function chain at start of main LHA loop (0x35e0):
- Calls thunk at 0x3098 which: 
  - Calls sub-init at 0x3259 which:
    - Calls bitio_init at 0x333b (zeros [0x536c], [0x59d2], [0x53b6], calls fillbuf(16))
    - Resets [0x59c0] (blocksize) to 0
  - Resets [0x18ac] (match_remaining) to 0

This runs ONCE per file. NO per-chunk reset.

### Confirmed: WCSC decode() IS standard LHA (matches Yoshi's implementation)

- The `bl &= 0xF7` before shr on the c_table lookup is an addressing quirk, not
  functionally significant (table is set up to accommodate it).
- The `and byte [0x18af], 0x1f` at function epilogue redundantly re-applies the
  same mask the read paths apply. No semantic difference.
- Verified: our decoder + Yoshi's own lha both produce IDENTICAL output for the
  first ~63828 bytes of "extended decode past 7398" (77% agreement on garbage
  they both produce past the chunk 1 boundary). This proves our decoder IS
  correct LHA.

### The wall

At output byte 7398, our decoder emits 0x8b (as a MATCH code c=256) but oracle
expects literal 0x26. State at that point:
- bio.pos = 4095, bitcount = 2, bitbuf = 0x0a86
- 2 bits still in bit reservoir + 1 unconsumed byte (byte 4095) + rest of stream
- blocksize = 6350 (block still has plenty of symbols left)

**Yoshi's OWN decoder produces the same wrong data at 7398** when given the full
payload with big origsize. This absolutely rules out any bug in our
implementation of LHA.

The compressed stream past chunk 1 is NOT standard LHA continuation. Something
about the encoder side does not match Yoshi's -lh5- once past 4096 bytes of
compressed input.

### Hypotheses eliminated this session

- Not a chunk-header offset (tried skips 0..15, all give same 7398)
- Not a padding pattern between chunks (tried 0x00, 0xFF, 0x01, 0x80 × 1-8 bytes)
- Not solvable by ring buffer initialization (tested with ORACLE-preloaded ring)
- Not a per-chunk reset triggered by [0x18c8] (verified init path runs once)
- Not solvable by resetting blocksize (rc=-3 means new-block parse fails)

### Where the answer must be

The ONLY remaining possibility: WCSC's compressor is a MODIFIED encoder that
does something non-standard when its OUTPUT buffer fills at 4KB. Either:

1. It writes a special marker sequence to the output stream (which our
   decoder needs to detect and skip)
2. It resets ENCODER state (not decoder state) in a way that changes what bits
   get emitted for subsequent symbols
3. The output stream is post-processed after compression (e.g., XOR with a
   nonce, or bit permutation)

Path to solve: reverse-engineer WCSC's COMPRESSOR side (probably in the same
install binary or in DRVSETUP.EXE). If WCSC's compressor is asymmetric with
Yoshi's, the compressor's post-processing logic would tell us what the decoder
needs to undo.

## Session 3 — SOLVED — 2026-09-03

### The answer: WCSC [0x76]!=0 mode embeds CRC-CCITT bytes every 4094 stream bytes

Cracked by tracing the READ FUNCTION at file 0x3a28 (GET_INPUT_BYTE), which
reads from a 4KB buffer refilled by the REFILL FUNCTION at 0x3bc0. In "complex
mode" (when [0x76] != 0), the refill path:

1. Reads 4096 bytes from disk via [0x18f2] (fread pointer)
2. Verifies CRC-CCITT (poly 0x1021) via function at 0x4592
3. Strips trailing 2 CRC bytes via memcpy (function at 0x4978)
4. Effectively delivers 4094 stream bytes per 4KB physical read

The magic constant is [0x78] = 0x1000 = 4096 (buffer read size), and the CRC
interval math at file 0x3e29 is `cx = [0x78]; dec cx; dec cx` → 4094.

### The fix (in red_decompress.c entry)

Before feeding stream to LHA decoder, strip 2 bytes every 4094 stream bytes:

```c
src = skip;
while (src < cmp_len) {
    size_t take = 4094;
    if (take > cmp_len - src) take = cmp_len - src;
    memcpy(reformed + rlen, cmp + src, take);
    rlen += take;
    src += take;
    src += 2;  /* skip 2 CRC bytes */
}
```

Smaller files (9/10 drivers, all under 4094 bytes compressed) are unaffected
because the loop consumes everything without hitting a full 4094-byte segment.
Only COMMDRV.EXE (45804 compressed bytes) has CRC bytes to strip.

### Result

```
COMMDRV.EXE:  BYTE-PERFECT (90827/90827) ← the wall broken
COMMDV00.DRV: BYTE-PERFECT (1130/1130)
COMMDV01.DRV: BYTE-PERFECT (1115/1115)
COMMDV02.DRV: BYTE-PERFECT (2276/2276)
COMMDV03.DRV: BYTE-PERFECT (2686/2686)
COMMDV04.DRV: BYTE-PERFECT (2797/2797)
COMMDV05.DRV: BYTE-PERFECT (4883/4883)
COMMDV06.DRV: BYTE-PERFECT (1662/1662)
COMMDV07.DRV: BYTE-PERFECT (1212/1212)
COMMDV08.DRV: BYTE-PERFECT (2284/2284)
```

End-to-end verified via `redx extract COMMDRV.RED` against oracle files.

### Path summary (for future reference)

- INSTALL.EXE file offset 0x35e0: main LHA driver loop
- 0x30a4: decode() — standard LHA -lh5- decoder
- 0x3a28: GET_INPUT_BYTE — 4KB-buffered byte reader
- 0x3bc0: REFILL — refills 4KB buffer via [0x18f2] fread pointer
- 0x4592: CRC-CCITT computation (poly 0x1021)
- 0x4978: thunk to memcpy (used to strip CRC bytes from buffer)
- 0x3e1c: `sub word [0x59ba], 2` — strip trailing 2 CRC bytes after verify
- 0x3e29: `cx = [0x78]; dec cx; dec cx` → 4094 (chunk interval)
- ds=0x345b, [0x78] = 0x1000 = 4096 (read buffer size)
- [0x76] = mode flag (from init arg); 0 = no CRC, !=0 = strip 2 every 4094
