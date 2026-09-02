# Function addresses found in install_unpacked.exe

## Breakthrough — 2026-09-03 headless RE

Located actual function entry points by tracing debug-string load
patterns (`mov ax, imm16; mov cx, imm16; push cx; push ax`) followed
by backward scan for `push bp; mov bp, sp` prologues, plus CALL FAR
target enumeration. Addresses cross-verified via both techniques.

## The three functions

| Name        | File offset  | Segment:Offset      | Size    | Callers               |
|-------------|--------------|---------------------|---------|-----------------------|
| kick_char   | **0x1e356**  | 0x1089:0xeb06       | 146 B   | NONE (probably inlined)|
| f_ram       | **0x1e3e8**  | 0x1089:0xeb38       | 56 B    | 1 (from 0x23de2)      |
| (unknown A) | **0x1e420**  | 0x1089:0xeb70       | 132 B   | 1 (from 0x1fc2a)      |
| (unknown B) | **0x1e4a4**  | 0x1089:0xebf4       | 134 B   | 3                     |
| MAIN DECOMPRESSOR | **0x1e52a** | 0x1089:0xec7a  | ~600 B  | 6 (once per record)   |

**flushram** function proper (whose error message we found at file 0x32af2)
is likely one of the "unknown" functions — probably 0x1e420 or 0x1e4a4.
Its debug label string at file 0x32b15 has 0 xrefs → it's not printed
at runtime, only used for symbol linking.

## Headless disassembly of kick_char reveals structure

```
kick_char (seg 0x1089:0xeb06):
  ; params: char c on stack at [bp+6]
  push bp; mov bp,sp; push di; push si
  mov  si,[bp+0x6]
  and  si,0x00FF                    ; si = c & 0xFF
  xor  si,[0xc50]                   ; XOR with running state at [0xc50]
  cmp  word [0xc52],0               ; check init flag
  jnz  skip_init
  mov  word [0xc52],1               ; set init flag
  ; PRINT "kick_char" debug string via debug_print at 0x100:0x4252
  ...
skip_init:
  les  bx,[0x2a1a]                  ; load FAR PTR to ring buffer
  mov  di,[0xc4e]                   ; ring index
  mov  [es:bx+di],al                ; write char to ring
  inc  word [0xc4e]                 ; advance index
  or   si,si                        ; check if XOR was zero
  jnz  done_no_flush
  cmp  [0xc4c],si                   ; check if index hit threshold
  jnz  partial_flush
  push word [0x59e8]                ; push file handle
  call 0x1f91:0x4220                ; full flush (fwrite?)
  jmp  done_common
partial_flush:
  push word [0x2a1c]                ; push flush length arg
  push word [0x2a1a]                ; push ring ptr low
  push word [0x59e8]                ; push handle
  call 0x1f91:0x39a2                ; fwrite(ring, len, handle)
  add  sp,6
  mov  [0xc4e],si                   ; reset index
done_common:
  mov  [0xc4c],si                   ; reset threshold
  mov  al,[bp+6]                    ; reload c
  sub  ah,ah
  mov  [0xc50],ax                   ; save as new state
  mov  word [0xa7e],0
done_no_flush:
  pop si; pop di; pop sp,bp; retf
```

## Interpretation

kick_char is a per-byte output helper. Each call:
1. XORs input byte with running state at [0xc50]
2. Writes byte to ring buffer at [ES:BX+DI] where DI = [0xc4e]
3. If XOR was zero (input == state), triggers a flush
4. Otherwise just advances

The FLUSH mechanism is state-driven, not fixed-interval — flushes when
the input char equals the current [0xc50] state. That's an unusual
encoding pattern.

**Note**: kick_char has NO callers in the current binary — likely
inlined by the C compiler, dead-code, or only invoked under debug
builds. So while its behavior is now clear, it may not actually run.

## f_ram function

```
f_ram (seg 0x1089:0xeb38):
  push si
  cmp  word [0xc54],0               ; init flag
  jnz  done
  mov  word [0xc54],1               ; set flag
  mov  word [0xc50],0x55            ; init state to 0x55 (!)
  xor  si,si
  mov  [0xc4c],si                   ; reset threshold
  mov  [0xc4e],si                   ; reset index
loop:
  mov  al,[si+0xa80]                ; read byte from data area
  push ax
  push cs
  call NEAR 0xeaa6                  ; process byte
  pop  bx
  inc  si
  cmp  si,0x1cb                     ; loop count = 459
  jc   loop
  mov  word [0xc54],0               ; clear flag
done:
  pop si; retf
```

f_ram is an INITIALIZATION routine that runs 459 iterations, calling
`0xeaa6` for each byte at `[0xa80+i]`. This is either:
- Loading a preload dictionary into the ring buffer
- Priming CRC / Huffman state
- Preparing a decoder lookup table

Data at `[0xa80]` needs to be inspected with proper DS resolution
(likely NOT DS=0x2d91 — that address contains executable code).

## What this still needs to solve COMMDRV.EXE (10/10)

The Ghidra session should:

1. **Open `install_unpacked.exe` and jump directly to file offset
   `0x1e356`** (Go → File Offset). Auto-analyze if not done.

2. **Verify function entry points** at:
   - `0x1e356` (kick_char)
   - `0x1e3e8` (f_ram)
   - `0x1e420`, `0x1e4a4`, `0x1e52a` (I/O helpers + main decompressor)

3. **Decompile 0x1e4a4 (3-caller function)** — likely `flushram`.
   Look for what it does at boundary conditions.

4. **Trace call from decompressor `0x1e52a` into f_ram/flushram** —
   understand when + how the I/O layer interacts with the LHA core.

5. **Focus on `[0xc4c]` (threshold) and `[0xc4e]` (index)**:
   - Where are they initialized to non-zero values?
   - What triggers `[0xc4c]` == index comparison?
   - Is `[0xc4c]` ever set to 4096?

## Alternative angle from these addresses

Now that we know f_ram entry (0x1e3e8) and the caller (0x23de2),
extend the emu_v3.py Unicorn Engine to:

1. Load install_unpacked.exe at paragraph 0x100
2. Set CS:IP to 0x23de2's caller function entry
3. Prepare stack with a fake COMMDRV.EXE payload pointer
4. Execute, breakpointing at f_ram entry (0x1089:0xeb38)
5. Log memory at [0xc4c], [0xc4e], [0xc50], [0xc52] before/after
6. Continue to 0xeaa6 in the inner loop and log bit-stream state

That's mechanical work — 100-150 lines of extension to the existing
Unicorn scaffold in `/tmp/ghidra-fresh/emu_v3.py` from prior sessions.
No Ghidra required for that path.

## UPDATE 2026-09-03 (continued): decoder decompilation complete

Successfully disassembled the MAIN LHA decoder — file 0x30a4, function
called from the outer chunk loop.

### Decoder structure (equiv to Yoshi's decode_c_st1)

```
decode(chunk_size, buf_off, buf_seg):
    cx = chunk_size
    si = 0  # output index in chunk
    loop:
        ax = [0x18ac]  # match_remaining
        ax++
        if ax >= cx: goto fast_copy
        
        [0x18ac]--
        if [0x18ac] < 0: goto get_new_code
        
        # copy from ring: read output_buf[ring_pos & 0x1FFF]
        di = buf_off
        bx = di
        es = buf_seg
        ax = [0x18ae]  # ring_source_pos
        ah &= 0x1f     # mask to 13-bit index (0..8191)
        bx += ax
        al = [es:bx]
        [es:bx+si] = al  # write to output
        [0x18ae]++
        si++
        if si != cx: continue
        return
    
    get_new_code:
        # BLOCK BOUNDARY CHECK
        ax = [0x59c0]  # block_remaining
        [0x59c0]--
        if ax != 0: goto decode_code
        
        # NEW BLOCK: read header
        blocksize = getbits(16)  # via call 0x22c6
        [0x59c0] = blocksize
        call read_pt_len(19, 5, 3)  # pt tree for c_len
        call read_c_len()
        call read_pt_len(14, 4, -1)  # pt tree for pos
        [0x59c0]--
    
    decode_code:
        # HUFFMAN LOOKUP (12-bit table optimization)
        bx = [0x536c]      # bit_buf (16-bit)
        bl &= 0xf7         # mask bit 3 (compiler optimization)
        bx >>= 3           # shift = index*2 into word table
        les di, [0x48]     # c_table pointer
        si = [es:bx+di]    # c_table[peekbits(12)]
        if si < NC (=510): goto have_leaf
        
        # tree walk for longer codes
        di = 8  # bit mask
        walk:
            test [0x536c], di
            if 0: bx = si*2 + [0x58]; es = [0x5a]  # left
            else: bx = si*2 + [0x54]; es = [0x56]  # right
            si = [es:bx]
            di >>= 1
            if si >= NC: walk
        
        have_leaf:
            len = c_len[si]
            fillbuf(len)  # via call 0x2246
            
            if si <= 0xff:  # LITERAL
                out[si] = si
                out_pos++
                if out_pos == chunk_size: return
                goto get_new_code
            
            # MATCH
            match_len = si - 253  # = si - 256 + THRESHOLD(3)
            [0x18ac] = match_len
            offset = decode_p()  # via call 0x1c5c
            [0x18ae] = out_pos - offset - 1  # source in ring
            goto fast_copy_loop
```

### Confirmed parameters (all match Yoshi standard -lh5-)

- NT = 19, TBIT = 5, i_special = 3 (pt tree for c_len)
- NP = 14, PBIT = 4, i_special = -1 (pt tree for positions)
- NC = 510 (max literal + match codes)
- Dictionary bits = 13 (mask 0x1FFF)
- table bits = 12 (4096-entry c_table, indexed by peekbits(12))
- Chunk size = 0x2000 = 8192 bytes output per decode() call

### Ring buffer = OUTPUT BUFFER itself

WCSC's decoder uses the caller's OUTPUT BUFFER directly as its ring
buffer, instead of maintaining a separate ring. This means:
- Each 8192-byte chunk is written directly to output_buf
- Historical references (offset back from current position) read from
  output_buf using `(current_pos - offset - 1) & 0x1FFF` as index
- No separate init — output_buf's initial contents = whatever malloc
  returned (typically zeros on first call, previous chunk's data on
  subsequent calls)

### 2026-09-03 further headless attempts (all failed to fix)

Trace shows exact divergence at code #3483:
- code #3483 = MATCH count=3 offset=52 (our decode)
- oracle wanted LITERAL 0x26 followed by...
- code #3483 spans bit positions 32758-32769, straddling byte 4095/4096

Additional tests attempted:
- Byte-alignment at bit 32748, 32750, 32752, 32756, 32758, 32760, 
  32762, 32764, 32766, 32768 with skip amounts 0..19: none extended
  prefix match beyond 7398
- Bit insertion after codes #3480, #3481, #3482, #3483 with 0..15 bits:
  none extended prefix match beyond 7398
- Best incidental match count 15137 with align=32760 skip=7 but 
  first_diff still = 7398 (accidental byte matches after divergence)

### The install.exe evidence points to MS Compression Library

Strings in install_unpacked.exe reveal:
- "MS Run-Time Library - Copyright (c) 1992, Microsoft Corp"
- Function names: `expand`, `expand_file`, `expand_wild`, `@DECOMPRESS`
- These are hallmarks of Microsoft's LZ / LZEXPAND family, not pure Yoshi

WCSC's compressor is likely a Microsoft-modified LHA that uses standard
Yoshi Huffman coding + parameters but has SOME small tweak in the bit
stream framing (possibly related to how chunks are byte-aligned or
how the compressed buffer is written to disk).

### Real next step

Would need actual runtime tracing (dosbox debugger or extended Unicorn
scaffold) to observe WCSC's decoder decoding COMMDRV.EXE.payload at the
bit level and pinpoint what it does at compressed byte 4096.
