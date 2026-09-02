# TOMORROW

## v1.2 DONE 2026-09-01

All 5 v1.2 features shipped:

1. Arnet SmartPort/Plus backend (src/arnet_backend.c, 231 lines)
2. ser_rs232 shim — 13-function COMMDRV.OBJ replacement (src/ser_rs232_shim.c, 216 lines)
3. INT 14h AH>=0x10 COMM-DRV extensions (src/int14.c extended)
4. _dos_keep() TSR install + .SYS path removed (src/pcbdcom.c)
5. SDK packaging (toolkit/pwa154/pcbdcom/, 32 files)

OpenWatcom verified: PCBDTSR.EXE 37,800 bytes, 15/15 files clean.

Still open (post-v1.2):

# Tomorrow — 2026-08-31

**Build v1 + v1.2 + v2 pcbdcom drivers end-to-end. Build SDK tree
alongside as we go, not as a later step. Document as each piece lands.**

## Order

1. ~~Verify v1.1 builds clean~~ **DONE 2026-08-31 — OpenWatcom 2.0 clean compile, 13 files, PCBDTSR.EXE 35306 bytes. See pcb154/pcbdcom/BUILD-STATUS.md**
2. ~~Extract COMMDRV.RED~~ PARTIAL 2026-09-01:
   * .RED container format FULLY REVERSED (26-byte header + null-name +
     compressed payload + 2-byte CRC16 trailer). All 22 records mapped
     with exact offsets, sizes, method IDs.
   * Compression method 0x000B is a Clark-specific LHA variant. Standard
     LH1 decoder produces correct output size but content fails CRC.
     Standard LH5-7 decoders reject the Huffman tree definition
     ('make_table(): Bad table').
   * STORED records (method 0x0001) extract cleanly (verified: MONITOR.BAT
     24 bytes byte-perfect).
   * Firmware BIOS extraction blocked on decompressor. Two paths:
     (a) Ghidra COMMDRV.EXE reverse to find the exact algorithm
     (b) Run INSTALL.EXE in DOSBox-X on a display-capable workstation
   * Findings captured in pcb154/pcbdcom/GAP-ANALYSIS.md
   * Not blocking v1.2 — Arnet/shim/INT14/_dos_keep all from public sources
3. v1.2 features:
   - arnet_backend.c (8th card — Arnet SmartPort Plus)
   - ser_rs232_shim.c (13-function COMMDRV.OBJ replacement API)
   - int14.c extended (AH=0x10 commgo, AH=0x12 commstop)
   - _dos_keep() TSR fix in pcbdcom.c
   - Drop .SYS device driver path
4. SDK tree `toolkit/pwa154/pcbdcom/` (in parallel with #3):
   - src/ (source copy)
   - lib/ (13 pre-built PCBDCOM_*_*.OBJ per compiler/model)
   - inc/PCBDCOM.H (ser_rs232_* API header)
   - docs/SDK.md + LINKOUT.md
   - examples/simple.c + multiport.c + tsrless.c
   - NOPCBDCOM.OBJ (empty stub)
5. v2 modular loadable drivers (if time):
   - Split PCBDTSR.EXE into skeleton + 9 loadable PCBDV00-08.DRV
   - Skeleton uses DOS EXEC / overlay loader
   - Ship v1.2 + defer v2 to next session if this doesn't complete

## Docs to update as we go

- pcb154/pcbdcom/README.md — bump status line
- pcb154/pcbdcom/SPEC.md — new API additions
- pcb154/pcbdcom/GAP-ANALYSIS.md — fill in 9th card when found
- Each new .c file: header comment cites public sources
- Each commit: what + why + what it unblocks

## Full plan

todo/pcbdcom-clean-room-plan.md — the full picture with file-name
mapping, stub inventory, priority order, etc.

## Ship pattern

One drop zip per major milestone (v1 verified, v1.2 landed,
SDK built). CHECKSUMS regen each time. Push via GitHub Desktop.


## Step 2 status update — 2026-09-01

Attempted steps 1-3 (Ghidra reverse, redx implementation, .RED repack)
but the Ghidra reverse of INSTALL.EXE's FUN_5000_c2f0 decompress
routine needs proper 16-bit segment setup that requires focused GUI
work. Prior /tmp/commdrv-work/ghidra-out/expand.{c,asm} artifacts
have "bad instruction data" throughout — segment mapping was off.

Realistic scope: 4-8 hours in Ghidra GUI with segment configuration
+ decompiler re-run + C translation + redx integration.

Not attempted in this chat turn. State saved for next session.

See pcb154/pcbdcom/GAP-ANALYSIS.md "Step 2b" section for full
investigation notes and reproducible next-session outline.


## Unblock path for BIOS extraction (fastest)

Sysop runs WCSC INSTALL.EXE natively on Windows / DOSBox-X-with-display
against a scratch target dir. That extracts all 22 files from
COMMDRV.RED using WCSC's own algorithm (which is what we can't
replicate yet). Zip results, share with build environment.

Files landed in pcb154/pcbdcom/firmware/:
  XABIOS.BIN, XACOOK.BIN, XACOMX.BIN  Arnet SmartPort firmware
  BOCA1610.BIN                         Boca BB-1016 firmware

Plus reference bytes for everything else:
  COMMDRV.EXE, COMMTSR.EXE, DRVSETUP.EXE, TEST.EXE
  COMMDV00-08.DRV (all 9 card driver modules)
  ARNETSP4/8.DAT, DIGI4E/8E.DAT (card register tables)
  MONITOR.BAT

The reference bytes unblock:
  * pcbdcom firmware/ directory (v1.2)
  * Byte-perfect verification of any future redx decoder we write
  * Ghidra reverse of COMMDRV.EXE itself (rather than INSTALL.EXE,
    which we tried today — its 16-bit segments are messy)

## Long-term unblock (later session)

Ghidra reverse of the decompress routine — needs interactive GUI
with proper segment configuration. 4-8 hours. Unblocks:
  * Repack modified .RED files with pcbdcom.OBJ substituted
  * Ship a modern pcbirc install disk set
  * Extract ANY .RED file (PCBOARD.RED, PCBMAIL.RED, etc.)

Both paths are worth doing. Option A is the immediate unlock.


## Release format request (2026-09-01)

When enough core work is done next time, ship a COMPLETE repo release:
* Two zip files, each under 400 MB
* Includes everything (reference archives, roysac collection, PPE
  archives, etc. — NOT just the essential 6.5 MB slice)
* Suggested split:
  - pcbirc-part1.zip: source + docs + toolkit + smaller refs
  - pcbirc-part2.zip: big reference archives + git bundle


## Next session: .RED crack (steps 1-3)

Everything is staged. Pick up here:

**Step 1 — Reverse (3-4h interactive Ghidra):**
- Open `/tmp/gproj_unpacked/TmpProj` in Ghidra GUI (already analyzed)
- Or reload `install_unpacked.exe` (275KB, at /tmp/ghidra-fresh/)
- Find 'expand' string @ file 0x2e2f8 → outer decompress driver
- Trace: outer → method dispatch → huf N branches (esp. huf 5)
- Find make_table (Bad table string @ file 0x2d76c is its error)
- Find decode_c / decode_p / read_pt_len / read_c_len
- Note WCSC's parameter values vs stock LHA-lh5:
  NC, NP, NT, PBIT, TBIT, CBIT + bit order (LSB vs MSB first)
- Note WCSC I/O layer: kick_char, f_ram 1/2, flushram

**Step 2 — Port to Python (1-2h):**
- Replace `decompress_wcsc_lha()` stub in
  `archivers/redx/refwork/decompress_v0.3.py`
- Run `python3 decompress_v0.3.py --test-all` after each attempt
- 10 oracle pairs in `refwork/pairs/` verify byte-perfect match
- Debug via first-mismatched-byte comparison
- Target: 10/10 OK

**Step 3 — Port to C (~2h):**
- Create `archivers/redx/red_decompress.c` (translate Python)
- Wire into `archivers/redx/redx.c` (currently STORED only)
- Test with same 10 pairs

**Handoff artifacts (all in repo except install_unpacked.exe):**
- install_unpacked.exe — host scratch /tmp/ghidra-fresh/ (275KB)
- refwork/full_disasm.asm.gz — 76,201 lines, grep-able
- refwork/decomp_driver_disasm.txt — 12,077 lines of the driver region
- refwork/decomp_driver.bin — 30KB raw driver code
- refwork/pairs/ — 10 payload/oracle test vectors
- refwork/decompress_v0.3.py — scaffold + harness + v1.0 plan in header

Not blocking v1.2. All 5 v1.2 features shipped, verified building.


## .RED 0x000B — the wall + the new angle (2026-09-01 late)

### Verta1878's hint that changed things
- Load base = paragraph 0x100
- Segment map: 0x100 image / 0x1089 LHA library (~180 funcs) / 0x1f91 CRT
  / 0x2d91 data (strings) / 0x2e71 aux strings
- "huf N" registration at 0x1089:0x25c2, 0x268c, 0x277e (callback table)
- Real work function: 0x100:0xf42c (file offset 0x1044c)
- That work function calls further into 0x1089 for the real Huffman/LZ

### The wall (why past turns kept failing)
Ghidra's auto-xref confused by MSC's segmented addressing. Manual grep
through 76K ndisasm lines re-starts every turn without context. Each
turn I've been retrying the same "read the code" approach and hitting
the same wall.

### The new angle — emulate, don't reverse
`install_unpacked.exe` is a valid MZ EXE. Unicorn already runs the
unpacker end-to-end. Extend that same emulator to:
  1. Load install_unpacked.exe (not INSTALL.EXE)
  2. Hook INT 21h for file I/O:
       AH=0x3D open, 0x3E close, 0x3F read, 0x40 write,
       0x42 seek, 0x41 unlink, 0x4E findfirst, 0x4F findnext
     File table is entirely in-memory (fake FS).
  3. Feed it a synthetic INSTALL.DAT script that calls @Expand on
     one .RED entry.
  4. Capture the write-hook output.
  5. Compare to oracle.

If bytes match → INSTALL.EXE itself is our oracle-in-a-box.
No algorithm knowledge needed. Extend once, use for every method.
This IS a shape headless emulation can hit.

Estimate: 4-6h to build the INT 21h harness + verify with
COMMDV00.DRV. Then instant reuse for all other .RED entries.

### If emulation angle stalls too
Surgical strike location: 0x100:0xf42c (file 0x1044c) is confirmed
work function. Interactive Ghidra there — decompile that + 3-5 nearest
0x1089 callees — should reveal Huffman params (NC/NP/NT/PBIT/TBIT)
and bit-order. Then port directly per v1.0 plan in decompress_v0.3.py.

### Don't retry the same approach
Past 3 turns: hunt xrefs -> disasm patterns -> stuck.
Next turn: emulation path OR interactive Ghidra with the 0xf42c hint.
NOT more xref hunting.


## Step 3 SHIPPED (2026-09-02 late night)

C port of .RED decompressor complete. archivers/redx/red_decompress.c
builds clean with gcc -O2, byte-perfect on all 9 device driver pairs
(same 9/10 result as Python v1.0).

Test:
    cd archivers/redx
    gcc -O2 -Wall -DREDX_STANDALONE_TEST -o red_test red_decompress.c
    for p in refwork/pairs/*.payload; do
        ./red_test "$p" "${p%.payload}.oracle"
    done

Result:
    COMMDRV.EXE     decompress error: -4    (Bad table — WCSC-specific chunked encoding)
    COMMDV00.DRV    OK: 1130 bytes byte-perfect
    COMMDV01.DRV    OK: 1115 bytes byte-perfect
    ...
    COMMDV08.DRV    OK: 2284 bytes byte-perfect
    9 pass / 1 fail (same as Python)

Also CONFIRMED: built Yoshi Watazaki's own LHA v1.14i-ac and ran on
COMMDRV.EXE payload. Yoshi's own tool fails with "make_table(): Bad
table (case b)" — same underlying error. This proves the COMMDRV.EXE
divergence is NOT our bug; it's WCSC using a variant that stock LHA
(either lhasa or Yoshi's reference) cannot decode.

For COMMDRV.EXE specifically: users can extract the pre-built binary
from reference/roysac/CSBACKUP.ARJ (PCBoard 15.22 preservation
snapshot). Full generic support requires reverse-engineering WCSC's
kick_char / f_ram / flushram I/O wrapping — deferred until interactive
Ghidra time is available.
