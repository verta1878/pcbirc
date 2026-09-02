# .RED decompressor — code addressing map
# 2026-09-01 partial findings from install_unpacked.exe

## CONFIRMED

### DGROUP segment: 0x2e71
All LHA library string messages live at segment 0x2e71 in the loaded image.

Verified string offsets (matched via disassembly xrefs):
| String        | DGROUP offset | File offset |
|---------------|--------------:|------------:|
| `huf 2`       |       0x3218  |     0x2d736 |
| `huf 5`       |       0x322a  |     0x2d748 |
| `Bad table`   |       0x324e  |     0x2d76c |
| `expand`      |       0x3dda  |     0x2e2f8 |

Verification: found `mov ax, 0x322a ; mov dx, 0x2e71 ; push dx ; push ax`
pattern at disasm address 0x118a3 (right before a CALL to 0xd208).

### Function 0xd208 is a printf-family call
The mass of consecutive calls to 0xd208 at addresses 0x117d0..0x1195d is
a sequence of ERROR MESSAGE PRINTS (fprintf/perror style), NOT a method
registration table. My earlier hypothesis of "callback registration" was
wrong — the second seg:off (segment 0x1089) in those calls turned out to
point to error format strings in a secondary data segment, not to code.

## STILL UNKNOWN

### Which function IS the decompressor?
40 function prologues (55 8B EC) in the file region 0x12000..0x14000
alone. The actual LHA huf-decode function must be one of them, but
which one requires interactive Ghidra to decompile and check.

Candidate functions in that range (seg 0x1089 relative offsets):
  0x177e, 0x188e, 0x1912, 0x1a58, 0x1bbe, 0x1cb6, 0x1d70, 0x1e72,
  0x1e9e, 0x1eca, 0x2142, 0x21a4, 0x21e2, 0x2224, 0x22e6, 0x23c4,
  0x2458, 0x2608, 0x297e, 0x2a14, ...

### DGROUP + code segment interpretation
Segment 0x1089 shows up as a common operand in printf calls. It's
probably another data segment (not code). This means MSC's linker
created multiple DGROUPs, which complicates simple segment=offset
tracing.

## PATH FORWARD (interactive Ghidra required)

1. Open install_unpacked.exe in Ghidra GUI
2. Under Program > Memory Map, manually configure segments:
   - CODE at physical range 0x00020..0x2d000
   - DGROUP at segment 0x2e71 (verified from xref pattern above)
   - Secondary data at segment 0x1089 (verified from printf format ptrs)
3. Once segments are correct, Ghidra's auto-analysis will discover
   xrefs properly; strings 'huf 5', 'Bad table', 'make_table' will
   show real xrefs
4. Follow the xref from 'expand' — that gives the OUTER decompress
   entry
5. Trace inward from there to make_table/decode_c/decode_p
6. Port to Python — replace decompress_wcsc_lha() stub
7. Verify with `python3 decompress_v0.3.py --test-all`

## PARTIAL FINDINGS THAT MIGHT STILL HELP

Sequential printf calls at 0x117d0..0x1195d suggest the CALLER of that
block is the outer error-reporting driver for LHA methods. Following
the CALLERS of that block backward might reach the decompress dispatch.
Found:
  - Sequential calls to 0xd208 (error printf)  
  - One call to 0xcf6c (different function, at 0x1180f and 0x119be)
  - Function 0xcf6c might be the actual method-selector or dispatcher

## HONEST ASSESSMENT

I've made 4 attempts at the algorithmic port in one session. Each
attempt made incremental progress in TRACING but did NOT reach the
port itself. The wall is real: this needs interactive Ghidra work
where a human can iterate on segment configuration and follow decompiled
pseudocode across function boundaries.

The next session (with a human at the Ghidra wheel, not another Claude
turn) should be able to finish in 3-4 hours starting from where we are.
