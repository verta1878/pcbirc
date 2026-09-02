# Yoshi encoder analysis — key finding for COMMDRV.EXE puzzle

## The `bufsiz` mechanism (huf.c line 267-271)

```c
bufsiz = 16 * 1024 * 2;  /* 65408U; */ /* t.okamoto */
while ((buf = (unsigned char *) malloc(bufsiz)) == NULL) {
    bufsiz = (bufsiz / 10) * 9;
    if (bufsiz < 4 * 1024)
        error("Out of memory");
}
```

On memory-starved DOS systems, `bufsiz` gracefully degrades from
32KB down to a floor of 4KB before erroring. `bufsiz` is the size
of a temporary code buffer inside the encoder — when it fills,
`send_block()` is called to flush the current block and start a new one.

## The trigger condition (huf.c line 239)

```c
if (output_pos >= bufsiz - 3 * CHAR_BIT) {
    send_block();
    ...
    output_pos = 0;
}
```

`output_pos` is a byte index into `buf` counting code descriptors,
NOT compressed output bytes. Each code takes 2-3 bytes in `buf`, so
bufsiz=4KB gives room for ~1600-2000 codes per block.

## Why this MIGHT explain COMMDRV.EXE

- All 9 small drivers (< 5KB uncompressed) fit in one block with any
  bufsiz → single-block LHA stream, works with any decoder.
- COMMDRV.EXE (90KB) requires multiple blocks. Block boundaries are
  DECODER-VISIBLE (blocksize field in stream), so a compliant decoder
  should handle them.
- But: block 1 of our COMMDRV.EXE reports blocksize = 9832 codes,
  which is FAR more than fits in a 4KB `buf` (max ~2000 codes).

## So bufsiz alone doesn't explain it

If WCSC's DOS build had bufsiz=4KB, block 1 should have blocksize
~= 2000, not 9832. So either:
- WCSC's bufsiz was much larger (matching our 9832 fitting one block),
  and the divergence at 7398 is NOT a block boundary; OR
- Block 1's blocksize field lies about the actual code count (unlikely
  in stock LHA); OR
- WCSC modified Yoshi's encoder to split blocks at different criteria
  (e.g., at fixed input-byte boundaries rather than output-buffer fill).

## What actually needs to happen

The Ghidra session on the four functions in `disasm-bins/` needs to:

1. **Confirm which function is flushram** (candidate A at 0x1e420 or
   candidate B at 0x1e4a4).
2. **Read main decompressor at 0x1e52a** — specifically look for:
   - Where it computes when to start a new block
   - Whether it checks for a "chunk end" marker beyond just blocksize=0
   - Any 4KB or 8KB constants used in its logic
3. **Look at the loop from decompressor → f_ram/flushram interactions**.

## Alternative: extended Unicorn emulator

Emulate install_unpacked.exe with COMMDRV.RED as input, breakpointing
at the LHA decompressor entry, log every bit read. Compare with our
Python decoder's bit trace. Find the exact bit-position where they
diverge. That will reveal what WCSC does differently at that point.

**Estimated effort**: 200 lines of Unicorn extension work. Prior scaffold
in `/tmp/ghidra-fresh/emu_v3.py` from earlier sessions handles the
LOAD side already; just need to add decompressor invocation + trace hook.

## Headless status: EXHAUSTED

I've tried:
- 4KB byte alignment (byte discard + re-fill)
- Byte insertion 0x00 / 0xFF at positions 4093-4101
- Byte removal at positions 4096-4100
- Chunked stream reassembly (chunk 4096/8192, headers 0-4)
- DICBIT variants 12, 13, 14, 15, 16
- NP/PBIT combinations across all -lh4/5/6/7-
- Force-restart LHA block at 4KB (call start_new_block regardless)
- Initial ring buffer content variations (0x00, 0x20, 0xFF, 0x55, 0xAA)
- Ring position start variations (0, 459, 512, 1000, 4096, 8000)
- Preloaded ring with 459 bytes from f_ram data area at various offsets
- Bit-stream analysis at divergence position (bytes 4092-4108 examined)

None broke past 7398 correct bytes. The barrier is real. Progress
now requires either Ghidra decompile or Unicorn tracing — genuinely
beyond what single-turn headless work can produce.
