# COMMDRV.RED contents — from INSTALL.DAT (public text manifest)

Source: `pcb1541/install/dist/disk2/INSTALL.zip` → `INSTALL.DAT` lines
containing `@BeginLib COMMDRV.RED` block. Plain-text WCSC installer
script. No decompilation required — this is a public manifest of
what the installer copies from the .RED archive to `COMMDRV\` on disk.



## Driver name identification — 2026-09-02 (session 4 morning)

All 9 COMMDV*.DRV files opened and their embedded "family 1.00" version
strings extracted. Complete mapping:

| File          | Embedded name string    | pcbdcom backend             |
|---------------|--------------------------|-----------------------------|
| COMMDV00.DRV  | `GENERIC     1.01`      | uart_backend.c        ✓     |
| COMMDV01.DRV  | `INTEL HUB6  1.00`      | hub6_backend.c        ✓ NEW |
| COMMDV02.DRV  | `DIGI-COMXI  1.00`      | digi_comxi_backend.c  ✓ NEW |
| COMMDV03.DRV  | `ARNET-SPORT 1.00`      | arnet_backend.c       ✓     |
| COMMDV04.DRV  | `BOCA(1610)  1.00`      | boca_backend.c        ✓     |
| COMMDV05.DRV  | `DIGI-PCX*   1.00`      | digi_pcxe_backend.c   ✓     |
| COMMDV06.DRV  | `GTEK(8Fx)   1.00`      | gtek_backend.c        ✓ NEW |
| COMMDV07.DRV  | `INT14H   1.00`         | int14.c               ✓     |
| COMMDV08.DRV  | `COMMDRV VxD 1.00`      | N/A (Windows-only, skip)    |

**Coverage as of v1.3:**
- All 8 DOS drivers supported (100% WCSC-DOS parity)
- 1 not applicable (Windows VxD — COMMDV08)

WCSC-original card lineup for DOS is now feature-complete.

**pcbdcom coverage beyond WCSC:**
- cyclom_backend.c (Cyclades Cyclom-Y)
- digi_accel_backend.c (Digi AccelePort — post-WCSC card)
- rocket_backend.c (Comtrol RocketPort — post-WCSC)
- easyio_backend.c (Comtrol EasyIO — post-WCSC)

## Card driver modules (9 files, per-family design)

| File          | Size (bytes) | Likely card family (from name pattern) |
|---------------|-------------:|----------------------------------------|
| COMMDV00.DRV  |         1130 | (unknown — smallest, possibly null/8250) |
| COMMDV01.DRV  |         1115 | (unknown) |
| COMMDV02.DRV  |         2276 | (unknown) |
| COMMDV03.DRV  |         2686 | (unknown) |
| COMMDV04.DRV  |         2797 | (unknown) |
| COMMDV05.DRV  |         4883 | largest — possibly Digi/AccelePort |
| COMMDV06.DRV  |         1662 | (unknown) |
| COMMDV07.DRV  |         1212 | (unknown) |
| COMMDV08.DRV  |         2284 | (unknown) |

**9 driver modules total.** pcbdcom v1.1 has 7 backends (uart, boca,
cyclom, digi_pcxe, digi_accel, rocket, easyio). We may be missing
up to 2 card families. Confirmed missing: **Arnet SmartPort Plus**
(see BIOS files below).

## Main binaries

| File          | Size (bytes) | Purpose (from filename + PCBoard docs)  |
|---------------|-------------:|-----------------------------------------|
| COMMDRV.EXE   |        90827 | Main driver executable                  |
| COMMTSR.EXE   |        64101 | TSR variant of driver                   |
| DRVSETUP.EXE  |        29752 | Sysop config editor                     |
| TEST.EXE      |        16482 | Hardware test utility                   |
| MONITOR.BAT   |           24 | Startup helper batch                    |

## Card BIOS / firmware blobs (redistributed by WCSC)

| File           | Size (bytes) | Card                                   |
|----------------|-------------:|----------------------------------------|
| XABIOS.BIN     |         2048 | **Arnet SmartPort BIOS**               |
| XACOOK.BIN     |         6144 | **Arnet SmartPort cook firmware**      |
| XACOMX.BIN     |         6144 | **Arnet SmartPort COMX firmware**      |
| BOCA1610.BIN   |         3228 | Boca BB-1016 (16-port) BIOS            |

**These prove the 8th missing card is Arnet SmartPort Plus.** The
BIOS files ship inside COMMDRV.RED, which is redistributed under
WCSC's PCBoard install disk terms — same license context as the
rest of PCBoard.

## Card configuration data files (installed only for advanced group)

| File          | Size (bytes) | Card config table       |
|---------------|-------------:|-------------------------|
| ARNETSP4.DAT  |         2053 | Arnet SmartPort 4-port  |
| ARNETSP8.DAT  |         3397 | Arnet SmartPort 8-port  |
| DIGI4E.DAT    |         2053 | DigiBoard 4-port        |
| DIGI8E.DAT    |         3397 | DigiBoard 8-port        |

`.DAT` files are DATA (register maps, port assignments), not code —
these are legitimate reference material for building compatible
configs, same as reading a card datasheet.

## Immediate takeaways (before any binary work)

1. **8th backend identified**: Arnet SmartPort Plus. XABIOS.BIN +
   XACOOK.BIN + XACOMX.BIN + ARNETSP4/8.DAT confirm it. pcbdcom v1.1
   SPEC.md correctly identified this as the missing card.

2. **9th backend possible**: 9 .DRV files but only 8 named card
   families visible (7 pcbdcom + Arnet). The 9th could be:
     - A null / passthrough driver
     - Different variant of an existing family (e.g., BOCA1610 is
       a 16-port version — might be separate from standard Boca)
     - Something else entirely — needs Phase 1 investigation

3. **BIOS distribution model**: Confirmed by INSTALL.DAT that BIOS
   files (XABIOS, XACOOK, XACOMX, BOCA1610) ship inside COMMDRV.RED.
   Once extracted, we can package them alongside pcbdcom for sysops
   who paid for PCBoard (since they already have redistribution
   rights via their PCBoard license).

4. **Modular per-card driver architecture**: WCSC used exactly the
   same design pattern pcbdcom uses — one file per card family. Our
   backend layout matches their .DRV layout. Suggests our SPEC.md
   design is on the right track.

# Phase 1 — PCBoard integration surface (from PCBoard source)

Sourced from `pcb154/MAIN/SOURCE/MODEM/MODEMDRV.C` (PCBoard's own code,
we have full rights). This is what pcbdcom must expose for a drop-in
replacement of COMM-DRV linked via COMMDRV.OBJ.

## The ser_rs232_* API (called by MODEMDRV.C)

Every COMM-DRV entry point PCBoard actually uses:

| Function                | Purpose                                    |
|-------------------------|--------------------------------------------|
| `ser_rs232_init()`      | One-time driver init                       |
| `ser_rs232_setup(port, pcb)` | Configure port from `port_param`      |
| `ser_rs232_getport(port, pcb)` | Read back port config              |
| `ser_rs232_dtr_on(port)` / `_off` | Modem control                     |
| `ser_rs232_rts_on(port)` / `_off` | Modem control                     |
| `ser_rs232_putbyte(port, &b)`   | Send 1 byte                          |
| `ser_rs232_getbyte(port, &b)`   | Receive 1 byte                       |
| `ser_rs232_putpacket(port, len, buf)` | Send buffer                    |
| `ser_rs232_getpacket(port, len, buf)` | Receive buffer                 |
| `ser_rs232_viewpacket(port, len, buf)` | Peek without consuming        |
| `ser_rs232_flush(port, dir)` | Flush TX (1) or RX (0) or both (2)   |

Return value convention: `RS232ERR_NONE` = 0 for success, non-zero
for various errors.

## INT 14h calls PCBoard makes directly (bypass ser_rs232 API)

Only two, both COMM-DRV extended (AH >= 0x10, not standard FOSSIL):

| AX      | Purpose                                     |
|---------|---------------------------------------------|
| 0x1000  | `COMMDRV_commgo` — enable/start transmit    |
| 0x1002  | `COMMDRV_commstop` — disable/stop transmit  |

DX = port number. AL unused (embedded in AH via AX).

**Implication:** pcbdcom's INT 14h handler needs to recognize AH ≥
0x10 as COMM-DRV extensions, in addition to standard FOSSIL (AH
0x00..0x0F).

## Delta vs pcbdcom v1.1

v1.1 int14.c implements standard FOSSIL 0x00-0x0F. **Missing: 0x1000
and 0x1002** — commgo/commstop. Trivial add (few lines).

pcbdcom currently exposes backend functions but NOT the ser_rs232_*
symbol names. For MODEMDRV.C to link against pcbdcom (COMMDRV.OBJ
replacement), we need a shim layer that maps ser_rs232_* → pcbdcom
internal API. Small file, ~100 lines.

## COMM-DRV ships modular per-card architecture

INSTALL.DAT proves 9 separate .DRV files (COMMDV00-08.DRV, sizes
1115-4883 bytes each). Same architecture pcbdcom already uses (7
backends). WCSC apparently split at exactly the same seams.


# Phase 1 Step 2 — COMMDRV.RED format cracked, extraction partial

## The .RED container format

Fully reverse-engineered from byte-level analysis of COMMDRV.RED
(and confirmed against PCBOARD.RED, PPLC.RED). Each record has a
26-byte fixed header followed by null-terminated filename, then
compressed payload, then 2-byte CRC16 trailer.

```
Offset  Size   Field
------  ----   -----
0-1     2      Magic 'RR'
2       1      Version (0x01)
3-7     5      Timestamp / archive ID
8-11    4      Compressed data size (LE u32)
12-15   4      Uncompressed data size (LE u32)
16-17   2      Marker 0xFFFF
18-19   2      CRC16 of uncompressed data
20-21   2      0x0000 padding
22-23   2      Const 0x0001
24-25   2      Compression method ID (LE u16)
                 0x0001 = STORED (no compression)
                 0x000B = compressed (LHA-family, exact variant TBD)
26+     var    Null-terminated filename
after   var    Compressed payload
+2      2      CRC16 trailer
```

## Records confirmed in COMMDRV.RED (22 total)

INSTALL.DAT documented 18 files, but the actual archive has 22:
the 4 additional are the .DAT card config files (ARNETSP4/8.DAT,
DIGI4E/8E.DAT). INSTALL.DAT lists these separately in the "advanced
install" group, not in the main @BeginLib COMMDRV.RED block, which
is why the initial manifest count differed.

Full list with byte offsets:

| Offset | File | Compressed | Uncompressed | Method |
|-------:|------|-----------:|-------------:|--------|
| 0 | COMMDV00.DRV | 792 | 1130 | 0x000B |
| 833 | COMMDV01.DRV | 836 | 1115 | 0x000B |
| 1710 | COMMDV02.DRV | 1561 | 2276 | 0x000B |
| 3312 | COMMDV03.DRV | 1720 | 2686 | 0x000B |
| 5073 | COMMDV04.DRV | 1717 | 2797 | 0x000B |
| 6831 | COMMDV06.DRV | 1182 | 1662 | 0x000B |
| 8054 | COMMDV07.DRV | 845 | 1212 | 0x000B |
| 8940 | COMMDV08.DRV | 1338 | 2284 | 0x000B |
| 10319 | COMMDV05.DRV | 2919 | 4883 | 0x000B |
| 13279 | COMMDRV.EXE | 45807 | 90827 | 0x000B |
| 59127 | COMMTSR.EXE | 20814 | 64101 | 0x000B |
| 79982 | XABIOS.BIN | 1399 | 2048 | 0x000B |
| 81422 | BOCA1610.BIN | 2116 | 3228 | 0x000B |
| 83579 | XACOOK.BIN | 3722 | 6144 | 0x000B |
| 87342 | MONITOR.BAT | 24 | 24 | 0x0001 (STORED) |
| 87407 | DRVSETUP.EXE | 17881 | 29752 | 0x000B |
| 105329 | TEST.EXE | 10417 | 16482 | 0x000B |
| 115787 | ARNETSP4.DAT | 388 | 2053 | 0x000B |
| 116216 | ARNETSP8.DAT | 446 | 3397 | 0x000B |
| 116703 | DIGI4E.DAT | 388 | 2053 | 0x000B |
| 117132 | DIGI8E.DAT | 447 | 3397 | 0x000B |
| 117620 | XACOMX.BIN | 3520 | 6144 | 0x000B |

Records with offsets, sizes, and method IDs verified directly from
COMMDRV.RED bytes.

## Compression method 0x000B — status: NOT YET CRACKED

INSTALL.EXE strings mention "expand", "huf 2" through "huf 7" —
strongly suggesting LHA-family compression. Tested against every
standard LHA method (LH1, LH4, LH5, LH6, LH7, LZS, LZ4, LZ5) with
various offset skips:

- LH1 decodes to correct output SIZE (1130 bytes for COMMDV00.DRV)
  but content fails CRC check — so LHA's LH1 decoder is producing
  valid-shaped bit output, but the encoding used differs from
  standard LH1.
- LH5-LH7 fail with "make_table(): Bad table (case b)" — Huffman
  tree definition invalid for standard LH5-7 decoders.
- STORED (method 0x0001) works correctly for MONITOR.BAT (24 bytes
  extracted verbatim, matches expected size).

**Conclusion:** 0x000B is a Clark-specific compression variant.
Close enough to standard LHA that LH1 decoder produces same-sized
output, but with different code tables / state initialization.

## Path forward

Options for actually getting the compressed data extracted:

1. **Ghidra COMMDRV.EXE reverse-engineering** — the decompressor
   lives inside COMMDRV.EXE (Turbo C 3.x-compiled). Ghidra can
   identify the decompress routine and give us the exact algorithm.

2. **Run INSTALL.EXE natively** — DOSBox-X in a display environment
   would let INSTALL.EXE self-extract. Not viable in this headless
   build environment, but works on the sysop's Windows workstation.

3. **Compare against known LHA variants** — LArc, older LH1 with
   different initial state, or a proprietary tweak.

Not blocking pcbdcom v1.2 work. The v1.2 features (Arnet backend,
ser_rs232_shim, INT 14h AH>=0x10, _dos_keep TSR fix) are all built
from public sources (Linux GPL, published Arnet datasheets, PCBoard
MODEMDRV.C) and don't need the WCSC binaries as reference.

## What we KNOW from Phase 1

- The 8th card is confirmed Arnet SmartPort Plus (from BIOS blob
  presence: XABIOS/XACOOK/XACOMX + ARNETSP4/8.DAT).
- All 9 COMMDV*.DRV modules exist and each corresponds to one
  card family — same modular architecture pcbdcom uses.
- Firmware blobs (XABIOS.BIN, XACOOK.BIN, XACOMX.BIN, BOCA1610.BIN)
  ship redistributed inside COMMDRV.RED under WCSC's PCBoard install
  disk terms. Extract-then-ship in pcbdcom is legally clear via the
  Digi ditty precedent (kernel driver GPL, firmware from vendor
  disk).
- The .DAT config files (ARNETSP4.DAT etc.) are register-table data
  that pcbdcom's arnet_backend.c can parse at runtime — legal
  reference material.


## Step 2b — Deeper compression investigation 2026-09-01

Tried multiple angles to identify the 0x000B algorithm:

1. **Standard LHA methods** (LH0, LH1, LH4-7, LZS, LZ4, LZ5) tested
   via wrapper trick. LH1 gives correct output size (1130 bytes)
   but wrong content (CRC error). Others fail with make_table or
   produce wrong sizes.

2. **Prefix skip variants** (skip 0, 1, 2, 4, 8 bytes before feeding
   payload to LH5-7 decoders). Only skip=41 produces non-crashing
   output, but with all-zero CRC — decoded random data.

3. **INSTALL.EXE string analysis** confirms LHA-family: "expand",
   "expand_file", "expand:skip", "huf 2" through "huf 10". The
   "huf N" numbers appear to be Huffman table indices, not method
   IDs (method 0x000B doesn't map to any of these).

4. **Prior Ghidra work** (from /tmp/commdrv-work/ghidra-proj/) had
   already located FUN_5000_c2f0 as the decompress entry point,
   called from wrapper FUN_3000_ef40. Ghidra's decompiler produced
   3777-line output but full of "bad instruction data" warnings and
   "unreachable block" removals — the 16-bit segment mapping needs
   proper setup before decompilation succeeds. Existing artifacts
   at /tmp/commdrv-work/ghidra-out/expand.{c,asm} unusable as-is.

5. **MSCOMPRESS check** — INSTALL.EXE built by MSC 6.0, but .RED
   payloads don't have SZDD/KWAJ signature. Not MS Compress format.

## Conclusion

Ghidra reverse of INSTALL.EXE's decompress routine (FUN_5000_c2f0)
is the only known path. Prior work opened the project but got
tripped up by 16-bit segment addressing. A focused 4-8 hour Ghidra
session with proper segment configuration (define code/data
segments correctly, then re-analyze) should extract the algorithm.

**Reasonable next-session outline for cracking this:**

1. Reopen /tmp/commdrv-work/ghidra-proj/commdrv.rep in Ghidra GUI
2. Manually define the code segment containing 0x5000c2f0 with
   correct base+length so decompilation stops seeing "bad
   instruction data"
3. Re-decompile FUN_5000_c2f0 to get real C
4. Translate to portable C in archivers/redx/red_decompress.c
5. Wire into redx_test to verify extraction of COMMDV00.DRV matches
   the shipped bytes (available from a Windows workstation running
   INSTALL.EXE natively in real DOS or DOSBox with display)

## Impact

Not blocking pcbdcom v1.2 (public-source implementation path).
IS blocking:
- Extraction of BIOS blobs from COMMDRV.RED for firmware/ dir
- Extraction of any file from PCBOARD.RED/PCBMAIL.RED/PCBCFGS.RED/PPLC.RED
- Ability to build a modern pcbirc install from original disks
- Ability to repack .RED files with pcbdcom.OBJ substituted for
  COMMDRV.OBJ in the toolkit

All of which the crew still wants — this is on the roadmap after
v1.2 lands and stabilizes.


## Step 2c — Headless native-DOS extraction attempts 2026-09-01

Tried running INSTALL.EXE natively via emulation to bypass the
compression crack requirement:

1. **Wine 9.0** — installed. Requires wine32:i386 for DOS EXE
   support (not enabled). Even then, Wine targets Win16/Win32,
   not real-mode DOS INSTALL.EXE which uses MSC 6.0 DOS calls.

2. **DOSBox (classic)** — installed, ran INSTALL.EXE, but couldn't
   drive interactive prompts headless. INSTALL.EXE just displays
   welcome screen and waits for keypress.

3. **DOSBox-X + response file redirect** (`INSTALL.EXE < RESP.TXT`) —
   hangs indefinitely. Whether INSTALL.EXE ignores stdin
   redirection, or DOSBox-X's stdin isn't being fed properly,
   couldn't verify in this env.

4. **DOSEMU** — not available in Ubuntu 24.04 repos.

## What would work

**Display-capable workstation** (any Windows machine, or Linux with
an actual X server + DOSBox-X installed):

    1. Extract INSTALL.zip contents to a working directory
    2. Boot DOSBox-X GUI, mount that directory as A:
    3. Run INSTALL.EXE
    4. Answer prompts: target drive C:, subdir \PCB\, select
       'Advanced' install group to include COMM-DRV, confirm
    5. Extracted files land in C:\COMMDRV\ and C:\PCB\

Expected output: all 22 files from COMMDRV.RED extracted, plus
the PCBoard install itself in C:\PCB\. Files are byte-perfect
Clark originals — usable as reference for redx verification.

## Path forward remains: Ghidra reverse

Interactive Ghidra GUI session with proper 16-bit segment
configuration on FUN_5000_c2f0 (in prior /tmp/commdrv-work/
ghidra-proj/). Once the algorithm is extracted and implemented
in redx, we can extract ALL .RED files (COMMDRV, PCBOARD,
PCBOARD2, PCBMAIL, PCBCFGS, PPLC) from any Linux host without
DOSBox involvement.

For unblocking pcbdcom v1.2 firmware/ directory contents:
sysop-provided path — sysop extracts INSTALL.EXE on their own
machine, drops XABIOS.BIN + XACOOK.BIN + XACOMX.BIN + BOCA1610.BIN
into `pcb154/pcbdcom/firmware/`. Manual first-load until redx
lands.


## Step 2c — Ghidra RE-analysis correction 2026-09-01

**Prior Ghidra session findings were WRONG.** Rebuilt project with
fresh headless analysis and dumped raw ASM at FUN_5000_c2f0:

```
5000:c2f0  00 02 35 42 fe 56 24 00 05 00 00 02 45 42 08 57
5000:c357  5f 2f 5c 2e 00 40 53 43 52 49 50 54 20 46 49 4c
                          ^^^^^^^^^^^^^^^^ "SCRIPT FIL"
```

Byte 0x53 0x43 0x52 0x49 0x50 0x54 = "SCRIPT" ASCII. This is a
**data table** (probably string references for INSTALL.EXE's
script processor), NOT the decompress function. Prior Ghidra
auto-analysis mistakenly created a function there because some
code referenced 0x5000c2f0 as an address.

## Actual decompressor location: STILL UNKNOWN

Top candidates by function size (biggest 20 functions in INSTALL.EXE):
- FUN_2e91_bdfe (4566 bytes) — segment 2e91, biggest overall
- FUN_5000_7f8a (2417 bytes)
- FUN_4000_200c (2311 bytes)
- FUN_5000_91d2 (1883 bytes)
- FUN_5000_aa32 (1862 bytes)
- FUN_5000_99f2 (1619 bytes)
- FUN_5000_e202 (1440 bytes)
- FUN_2e91_5cec (1330 bytes)

Real decompressor probably in the 1500-2500 byte range (LZ + Huffman
implementations typically that size). Candidates:
- FUN_5000_7f8a, FUN_5000_91d2, FUN_5000_aa32, FUN_5000_99f2

Identification requires: mark data regions as data, re-analyze,
then look for functions matching the signature of an LZ+Huffman
decoder (bit reader + tree walker + sliding window write).

## Real path forward (per prior estimate: 4-8 hours GUI work)

Genuinely needs Ghidra GUI:
1. Open project in Ghidra GUI (`ghidraRun` command)
2. Load INSTALL.EXE with DOS_MZ loader (default settings)
3. Manually inspect memory segments 0x2e91, 0x3000, 0x4000, 0x5000,
   0x6000. Mark string tables and data blocks as data (Ctrl+D).
4. Force re-analysis with just marked-code regions.
5. Search string references properly (Search > For Strings)
6. Find functions calling filesystem/read routines that also do bit
   arithmetic — those are decompressors.
7. Decompile top 4-5 candidates, look for LZSS + Huffman patterns.
8. Manually translate the winning candidate to portable C.

Headless approach cannot do step 3-4 (marking data regions)
programmatically without knowing which addresses are data.

## Status: BLOCKED on GUI Ghidra session

Not attempted further in this chat turn — genuinely requires
interactive Ghidra work with a human at the keyboard. Suggested
duration: 4-8 focused hours.

**Not blocking pcbdcom v1.2** — all v1.2 features use public sources.


## Step 1-3 attempt — 2026-09-01 late session

### Real progress on Ghidra approach

1. **INSTALL.EXE is OS/2 1.x NE, not standard MZ.** Prior sessions'
   confusion (bytes appearing as zeros, "SCRIPT FIL" text where
   we expected the decompressor) is fully explained: Ghidra's
   MZ loader loaded only the DOS stub — the actual code lives in
   NE segments starting at file offset 0x1ca00.

2. **NE segment structure decoded:**
   - Seg 1 (CODE) at file 0x1cc00, 54,574 bytes
   - Seg 2 (CODE) at file 0x2a200, 63,460 bytes
   - Seg 3 (CODE) at file 0x39e00, 51,600 bytes
   - Seg 4 (DATA) at file 0x46c00, 165 bytes
   - Seg 5 (DATA) — MZ stub area
   - Seg 6 (DATA) at file 0x46e00, 48,210 bytes
   File alignment shift = 9 (512-byte sectors).

3. **All 'expand' strings are in seg 6** at offsets 0x591c through
   0x5b50 (11 occurrences).

4. **Located the 'expand' caller function:**
   `seg1:0xa848` — 370 bytes, ENTER 234-byte frame, ends with
   LEAVE/RETF. This function calls the decompressor and passes the
   string "expand" as a debug/log argument (`PUSH DS; PUSH 0x591c`
   at seg1:0xa944 offset).

5. **Function catalogue in seg 1** (largest first):
   ```
   seg1:0x67e2  4482 bytes  — decompressor candidate
   seg1:0x508c  4042 bytes  — decompressor candidate
   seg1:0x2f9e  3668 bytes  — decompressor candidate
   seg1:0x45de  2734 bytes  — likely candidate
   seg1:0x87a6  2574 bytes
   seg1:0xc7c0  2276 bytes
   seg1:0xbf72  2126 bytes
   seg1:0x3df2  2028 bytes
   seg1:0x7964  2004 bytes
   seg1:0x9fe8  1988 bytes
   ```
   The 2000-4500 byte range brackets typical LHA-family
   decompressor size (Huffman table decode + LZSS window). The
   'expand' caller (`0xa848`) makes 14 calls to seg2/seg3
   functions — one of those is the decompress entry.

6. **NE segment relocations parsed:**
   - Seg 1: 23 relocation entries (few external calls)
   - Seg 2: 126 entries
   - Seg 3: 90 entries
   Far CALL fixups can be resolved without runtime loader by
   consulting these reloc tables.

### Extracted artifacts (host scratch)

```
/tmp/ghidra-fresh/seg1_code.bin   54,574 bytes
/tmp/ghidra-fresh/seg2_code.bin   63,460 bytes
/tmp/ghidra-fresh/seg3_code.bin   51,600 bytes
/tmp/ghidra-fresh/seg4_data.bin      165 bytes
/tmp/ghidra-fresh/seg6_data.bin   48,210 bytes
```

Not committed to public repo per Phase 1 rules (binary derivatives).

### What's still needed

1. **Load raw segments into Ghidra** with proper 16-bit segment
   definitions (seg1@0x0000:0000, seg2@0x1000:0000, etc.) so the
   MZ loader confusion is bypassed entirely.
2. **Decompile the 'expand' caller** at seg1:0xa848 to identify
   which of its 14 far calls goes to the decompressor.
3. **Trace call graph** from that decompressor forward — the
   algorithm reveals itself once we can read the C.
4. **Translate to portable C** — implement in
   `archivers/redx/red_decompress.c`.
5. **Round-trip test** — decompress COMMDRV.RED first record,
   verify byte-for-byte match with reference extraction from a
   real DOSBox-X INSTALL.EXE run (needs display environment).

### Not blocking v1.2

pcbdcom v1.2 features (Arnet backend, ser_rs232_shim, INT 14h
extensions, `_dos_keep` TSR fix, SDK packaging) all build from
public sources. Compression crack is only needed to (a) extract
BIOS blobs for firmware/ dir and (b) build a modern install path
that re-packs .RED files with pcbdcom.OBJ substituted.


## Step 1-3 continued 2026-09-01 (major discovery + hard block)

### Progress made
- Loaded seg1 raw binary into fresh Ghidra project at 0x0:0x0000
  successfully (bypasses the OS/2 NE loader issue)
- Auto-analysis found 177 functions in seg1
- Decompiled the 'expand' caller at seg1:0xa848 (1257 bytes actual size,
  not 370 as first measured) — clean C output
- Identified the 26-argument call inside: `func_0x000a8d3a` — this
  is the DECOMPRESS ENTRY POINT (26-arg matches expand_file's
  signature: input/output buffers, sizes, method, callbacks, etc.)
- Decompiled biggest seg2 function (0x9ab0, 2505 bytes) — turned
  out to be FILE COMPARISON/COPY, not decompression

### The real blocker: MZ portion is compressed

Fresh discovery: the entire MZ image portion of INSTALL.EXE (before
the OS/2 NE header at 0x1ca00) is EXEPACK-family compressed.

**Evidence:**
- 'huf 2', 'huf 3', 'huf 5', 'huf 10', 'decompression phase', 'Unable
  to write', 'Bad', 'make', "Can't" ASCII strings visible in raw file
  bytes around offset 0x17fa1 — these are Yoshi LHA-family error
  strings from a `make_table()`-based Huffman decoder
- BUT: Ghidra can't see any of these strings when loading MZ image
  — all memory blocks read as zeros
- Reason: MZ entry point at CS:IP=0:0003 has classic EXEPACK unpacker
  prologue:
  ```
  FC          CLD
  06 1E 0E    PUSH ES / DS / CS
  8C C8       MOV AX,CS
  01 06 38 01 ADD [0138],AX
  BA 74 1C    MOV DX,1c74
  03 C2       ADD DX,AX
  8B D8       MOV BX,AX
  05 8E 1A    ADD AX,1a8e
  8E DB       MOV DS,BX
  8E C0       MOV ES,AX
  33 F6 33 FF XOR SI,SI / DI,DI
  B9 08 00    MOV CX,0008
  F3 A5       REPZ MOVSW    <- data copy loop
  ```
- No standard packer signature found: no EXEPACK "RB\x00", no LZ91
  (LZEXE), no LZ09, no PKLITE, no DIET, no RJSX
- Conclusion: custom EXEPACK-family packer, likely Microsoft's own
  installer packer from OS/2 SDK 1.x era

**Impact:** the actual decompressor code (with 'huf N' Huffman tree
strings) is encoded until runtime. Ghidra can't decompile encoded
code. This is a fundamental blocker for static analysis.

### Two paths forward

1. **Write a Python-based EXEPACK-variant unpacker** that unpacks
   INSTALL.EXE's MZ image based on the entry-point unpacker code
   (reverse the unpacker itself, apply to the packed body). Once
   unpacked, load into Ghidra normally — 'huf N' strings become
   visible, decompressor easy to find.

2. **Run INSTALL.EXE in DOSBox-X on display workstation**, dump
   process memory after unpacking runs, load memory image into
   Ghidra. Same end result — visible decompressor code.

Either path: 4-8 hours of focused work. Path 1 is headless-compatible
(this environment). Path 2 requires display but is more reliable.

### Extracted artifacts on host scratch
```
/tmp/ghidra-fresh/seg1_code.bin       54574 bytes  seg1 CODE
/tmp/ghidra-fresh/seg2_code.bin       63460 bytes  seg2 CODE
/tmp/ghidra-fresh/seg3_code.bin       51600 bytes  seg3 CODE
/tmp/ghidra-fresh/seg6_data.bin       48210 bytes  seg6 DATA (has 'expand' strings)
/tmp/ghidra-fresh/install_mz_only.exe 117248 bytes MZ portion (EXEPACK-compressed)
/tmp/ghidra-fresh/install_mz_body.bin 116736 bytes MZ body without header
/tmp/ghidra-fresh/gproj2.rep          Ghidra project with seg1 loaded (usable)
/tmp/ghidra-fresh/gproj3.rep          Ghidra project with seg2 loaded (usable)
```

Ghidra projects gproj2 and gproj3 have working decompilation of seg1
and seg2 respectively. These are USABLE — the seg1 caller of 'expand'
strings is fully decompiled and the 26-arg decompress-entry call is
identified. What's blocked is following that call INTO the actual
Huffman/LZSS decoder because that lives in the EXEPACK-compressed
MZ portion of INSTALL.EXE.

### Not blocking v1.2 still
pcbdcom v1.2 features (Arnet backend, ser_rs232_shim, INT 14h AH>=0x10,
_dos_keep TSR fix, SDK packaging) all build from public sources.
The compression crack unlocks BIOS extraction and modern install
repack — both scheduled AFTER v1.2 lands.


## Step 1-3 further attempt 2026-09-01 — EXEPACK-variant identified but not standard

### Progress
- Extracted MZ portion of INSTALL.EXE (117,248 bytes) into isolated
  install_mz_only.exe file, patched MZ header to reflect standalone size
- Identified real unpacker location: after hoist relocation, the actual
  decompressor runs from file offset 0x1c83c
- Disassembled decompressor loop — CLASSIC LZSS pattern:
  ```
  20cc: call get_bit    ; read 1 bit
  20cf: jnc  copy_lit   ; bit=0 -> literal
  20d1: movsb           ; copy 1 byte src->dst
  20d4: call get_bit    ; bit=1 -> match
  20d7: lodsb           ; length byte
  ```
  This is unmistakable EXEPACK-family compression.

- Ends with jump to `0x1e91:0x5c28` = real DOS entry point of the
  unpacked program (after decompression finishes)

- Handles segment boundary crossing (SI/DI overflow) at 0x20e7-0x2111
  by adjusting DS/ES accordingly — standard 16-bit segmented packer
  technique.

### Blocker: not standard EXEPACK v4.05/4.06

Wrote unexepack.py using standard EXEPACK header format (18 bytes at
CS:0 with fields: real_ip, real_cs, mem_start, exepack_size, real_sp,
real_ss, dest_len, skip_len, "RB" signature). Parse produced nonsense:
- real CS:IP = fc01:c589 (invalid runtime addresses)
- dest_len = 761,872 bytes (much bigger than the 117KB file)
- signature at header offset 16 = `03 c2` (not "RB")

This is a custom EXEPACK-family variant, likely from Microsoft's
OS/2 SDK 1.x installer builder or hand-rolled. The unpacker code
STRUCTURE matches EXEPACK (bit-stream LZSS with literal/match) but
the packed-file header layout differs.

### What's needed to complete

Full reverse of the specific bit-stream / header layout used by this
variant. Estimated 2-4 hours focused work:
1. Trace `get_bit` function to determine bit-order (MSB-first vs LSB-first)
   and buffer management
2. Trace all match variants (length classes, distance encoding)
3. Trace segment-crossing logic
4. Trace relocation table application
5. Reimplement in Python (or C)
6. Verify output is a proper unpacked MZ EXE that Ghidra can decompile
7. Then proceed to find the .RED decompressor in the unpacked binary

### Artifacts on host scratch (not in public repo per Phase 1 rules)

```
/tmp/commdrv-work/unpacker-analysis/
    NOTES.md                        Session findings summary
    install_mz_only.exe             MZ portion (packed, 117248 bytes)
    install_mz_body.bin             Same without header
    unexepack_v1_incomplete.py      Python first attempt (standard EXEPACK
                                     format — didn't work, variant differs)
    full_disasm.txt                 Complete ndisasm of packed body (55800 lines)

/tmp/ghidra-fresh/gproj2.rep        seg1 loaded, expand caller decompiled
/tmp/ghidra-fresh/gproj3.rep        seg2 loaded, biggest function decompiled
```


## Step 1-7 attempt 2026-09-01 late — Unicorn emulation of unpacker

### Approach
Since the unpacker is a real 16-bit DOS program, run it in Unicorn
Engine (Linux, headless) instead of reverse-engineering the bit format.

### What worked
- Unicorn Engine 2.1.4 installed and working
- Loaded install_mz_only.exe at segment 0x1000
- Applied the single MZ relocation
- Set up CS:IP, SS:SP, DS/ES per DOS EXE convention
- Emulation ran 139,308 instructions successfully
- Hoist loop (7,284 iterations copying 16 bytes each) completed
- Decompressor code got control

### Where it stopped
Invalid instruction at CS:IP=44e5:608c (physical 0x4aedc). The bytes
there are `f0 0f f0 0f 00 00` — LOCK prefix + F0 escape. That's not
valid 16-bit real-mode code; it's data that got mis-interpreted.

Explanation: the decompressor jumped somewhere it hadn't fully
initialized yet, or my emulator setup missed a state variable that
the real DOS loader would have set (PSP fields, DTA, etc.).

### What we recovered
Search of emulator memory dump for target strings:
- 'huf 2': FOUND at 0x17da1 (in the packed data literal region)
- 'huf 3': not found (probably in same region but with different
  compression control bytes around it)
- 'decompression': not found
- 'make': FOUND at 0x17de0
- 'expand': not found in decompressor region

The 'huf N' strings appear as literals in the LZSS-compressed
stream — they're preserved via literal opcode bytes. Meta-bytes
around them are compression control bytes.

Code prologue scan found:
- 2 x `PUSH BP; MOV BP,SP` (typical MSC-compiled function start)
- Multiple ENTER instructions
- 6+ `MZ` signatures (probably random matches in data)

Not enough to be a fully unpacked EXE — emulator crashed before
completion.

### Next-session refinement (2-3 more hours)

1. Set up complete DOS environment for Unicorn:
   - Proper PSP at LOAD_SEG - 0x10
   - Environment block segment
   - DTA at PSP:80h
   - Working DOS INT 21h handler (at least AH=25/35 vector I/O,
     AH=48/49 memory alloc, AH=3D/3F/40 file I/O, AH=4C exit)
2. Run emulator with instruction trace on
3. Find where it crashes, understand why
4. Fix, iterate until unpacker completes cleanly
5. Then dump memory and verify plain-text 'huf N' strings appear
   fully (all of huf 2, huf 3, huf 5, huf 10, plus 'decompression
   phase', 'Unable to write', 'Bad table', 'Can't')
6. Save as unpacked MZ EXE, load into Ghidra
7. Follow the 'huf' string references to the decompress function
8. Reverse the .RED decompress algorithm
9. Implement in archivers/redx/red_decompress.c
10. Round-trip test

### Artifacts (host scratch, not in public repo)

```
/tmp/commdrv-work/unpacker-analysis/
  NOTES.md
  install_mz_only.exe             117248 bytes packed
  install_mz_body.bin             116736 bytes
  unexepack_v1_incomplete.py      first Python attempt (failed)
  emu_unpack.py                   Unicorn-based emulator (139K instr, partial)
  emu_output.bin                  memory dump after emu crash
  full_disasm.txt                 55800 lines complete disasm
```


## Step 8-9 continued 2026-09-01 late-night — Unicorn v2 with DOS environment

### What was completed

Built emu_v2.py — Unicorn Engine emulator with proper DOS environment:
- PSP structure at LOAD_SEG-0x10 (segment 0xF0)
- Environment block at segment 0x80 with minimal PATH=
- Interrupt vector table with INT 21h handlers for:
  AH=0x25 (set vector), 0x35 (get vector), 0x30 (DOS version),
  0x48/0x49/0x4A (memory alloc/free/resize), 0x40 (write),
  0x4C/0x00/0x4B (exit), 0x1A (set DTA), 0x62/0x51 (get PSP)
- INT 20/22/23/24 handled as program exit
- Full 640KB memory map

Confirmed decompressor algorithm location by pattern search in emu dump:
- get_bit signature (D1 ED FE CA 75 05 AD 8B E8 B2 10 C3) found at
  dump offset 0x36EA3 = physical 0x37EA3 = runtime CS:IP 0x35E5:0x2053
- Main LZSS loop signature (E8 84 FF 73 03 A4 EB F8) verified at
  file offset 0x1C83C, matches disassembled bytes

Traced far JMPs — exactly ONE at instruction 131146:
  0100:0035 JMPF -> 35E5:20CC
This is the transition from hoist to decompressor. Everything before
is the hoist copy loop (131,145 instructions for 7,285 iterations *
~18 instructions each).

### Blocker: hoist output has gap where main-loop code should be

The hoist correctly places get_bit code at dump[0x36EA3] but the region
starting at dump[0x36EDE] (right after get_bit ends) is ZEROS through
~0x36F97 where a stray non-zero byte appears.

Expected: continuous file data from 0x1C7C3 (get_bit) through 0x1C8B7
(last non-zero in file) should map to dump[0x36EA3..0x36F97].
Actual: dump[0x36EA3..0x36ED4] has real data, then zeros.

Two possibilities under investigation:
1. Hoist skips segments intermittently (bug in my iteration count math)
2. Some source memory the hoist reads was uninitialized in my emulator

The JMP target 0x35E5:0x20CC = dump[0x36F1C] falls into the zero region,
so CPU jumps to zeros, executes them as `ADD [BX+SI], AL` for many
iterations, then hits `01 F0 00 F0` at 0x36F97+ which is invalid.

### What we do have

Full disassembly of the decompressor from file bytes (not dependent
on emulator success):

  IP 0x2053: get_bit function
    SHR BP, 1    ; shift bit buffer, LSB in CF
    DEC DL       ; decrement bit counter
    JNZ +5       ; if not zero, done
    LODSW        ; refill: DS:SI -> AX, SI += 2
    MOV BP, AX   ; new bit buffer
    MOV DL, 0x10 ; reset counter to 16
    RET

  IP 0x20CC: main LZSS loop
    CALL get_bit
    JNC copy_literal    ; bit=0 -> literal
    MOVSB               ; copy 1 byte
    JMP self
  copy_literal:
    CALL get_bit
    LODSB               ; length byte -> AL, then BL
    MOV BH, 0xFF
    MOV BL, AL
    ; ...complex match decoding...

Bit stream: LSB-first, 16-bit refill via LODSW.
Match format: length byte + distance encoded via bit-controlled cases.

### Path forward (2-3 more hours)

Option A: Fix the hoist emulation bug
  - Add per-iteration write hook to see exactly what bytes get written where
  - Verify DF flag state at hoist entry
  - Verify CX loop count for REP MOVSW
  - Compare against expected [0x1000..0x1D74F] -> [0x1B8E0..0x3801F] shift

Option B: Skip the emulator, implement algorithm directly in Python
  - We have the disassembly of get_bit + main loop
  - Missing: exact bit-encoding of match length classes and distances
  - Would need to trace more of the decompressor disassembly (IP 0x20DC
    onward: BH=0xFF, BL=length byte, then 3-4 conditional match variants)
  - Then run against a known-good STORED-format .RED record to verify
    the packed metadata isn't itself needed (unlikely — .RED records
    with method 0x000B are LHA-family compressed, likely a DIFFERENT
    algorithm than this LZSS)

Option C: Verify assumption that INSTALL.EXE's LZSS = .RED method 0x000B
  - Might not be the same! INSTALL.EXE unpacks itself with LZSS but
  - INSTALL.EXE contains a SEPARATE decompressor (the one with
    'huf 2', 'huf 3', ... 'huf 10' strings) that handles .RED records
  - LHA family with different Huffman levels per method ID
  - So even a completed INSTALL.EXE unpack only gets us to the point
    where we can see 'huf N' strings PLAIN in Ghidra — we'd then need
    to find and reverse the ACTUAL .RED decompressor within the
    unpacked binary

### Artifacts (host scratch, not in public repo)

  /tmp/commdrv-work/unpacker-analysis/
    NOTES.md, install_mz_only.exe, install_mz_body.bin,
    unexepack_v1_incomplete.py, emu_unpack.py, emu_v2.py,
    emu_trace.py, find_bad_jmp.py, emu_output.bin, 
    install_mz_only.exe.emu2.bin (524KB memory dump), full_disasm.txt

Ghidra projects at /tmp/ghidra-fresh/gproj{2,3}.rep remain usable.


## BREAKTHROUGH 2026-09-01 late — INSTALL.EXE fully unpacked via Unicorn

### The bug that blocked everything
MZ header parse: `image_end = (total_pages - 1) * 512 + last_page`.
When last_page == 0, LHA's docs say last page has FULL 512 bytes. My
formula gave (229-1)*512 + 0 = 116,736 bytes. Actual is 229*512 =
117,248 bytes. I was **dropping the last 512 bytes of the image** —
which contained the LZSS decompressor main loop at file[0x1c800..0x1ca00].

Fix: `image_end = total_pages * 512 if last_page == 0 else (total_pages - 1) * 512 + last_page`

The DF flag hint from the user (which was worth investigating) turned
out not to be the bug — Unicorn initializes FLAGS with DF cleared by
default. But it prompted the deeper look that found the MZ parse error.

### After the fix
Emulation ran to completion:
- 3,571,852 instructions executed
- Hoist loop completed (166,432 hoist writes with valid source data now)
- JMP FAR to unpacker main loop 0x35e5:0x20cc succeeded
- Decompressor ran, unpacked ~450KB of program+data
- Unpacked program started running, made DOS calls (INT 21h AH=0x48
  memory alloc, AH=0x35 get vector, AH=0x25 set vector, etc.)
- Eventually crashed at 0x1f91:0xb629 trying to read unmapped memory
  (probably file I/O we didn't stub, but the DECOMPRESSOR is done
  well before this point)

### Unpacked artifacts
`/tmp/ghidra-fresh/install_unpacked.exe` — 275,872 bytes, proper MZ
EXE with CS:IP = 1d91:5c28. Load in Ghidra normally.

**All target strings recovered as plain text:**
- 'huf 2' at 0x2d716, 'huf 3' at 0x2d71c, 'huf 4' (implied),
  'huf 5' at 0x2d728, 'huf 7' (implied), 'huf 10' at 0x2d734
- 'Unable to write' at 0x2d73c
- 'Bad table' at 0x2d74c
- 'internal error make_table' (visible in dump around 0x2d765)
- 'Can't write output data during decompression phase' at 0x2d770
- 'expand' at 0x2e2d8

Plus 1515 total string groups, including all INSTALL.DAT scripting
keywords (@READLN, @MCBSIGNATURE, @OSMAJOR, @DISKSIZE, etc.) —
confirms this is the full unpacked installer.

### Round-trip test with lhafile.lzhlib
Tested COMMDV00.DRV payload (792 bytes cmp -> 1130 unc expected):
- -lh0-, -lh1-, -lh4-: don't fit or unsupported
- -lh5-, -lh6-, -lh7-: decode without error but produce 34,488 bytes
  of mostly 0x22 (wrong Huffman)
- -lzs-, -lz4-, -lz5-: unsupported by lzhlib

**Conclusion**: Method 0x000B is a WCSC MODIFIED LHA-family variant,
not standard -lh5-. Structure matches LHA (huf N error strings,
make_table function name) but parameters or bit-encoding differ.

### v0.1 Python scaffold
Saved: `archivers/redx/refwork/decompress_v0.1.py`
- Full container walker (all 10 records in COMMDRV.RED parsed)
- STORED (0x0001) decompression working
- Method 0x000B: raises NotImplementedError with detailed reversing
  plan in the file header

### Next-session work (finish v1.0, 2-3 hours)
1. `ghidra -import install_unpacked.exe`
2. Find xref to "Bad table" string (~0x2d74c) — leads to make_table
3. make_table's callers = read_pt_len + read_c_len (Huffman tree decoders)
4. Follow up to decode_c / decode_p (bit-decode loops)
5. Follow up to outer 'expand' function (main driver)
6. Compare against standard LHA -lh5- huf.c line-by-line
7. Note the differences (params, bit widths, table sizes)
8. Port to Python, replace decompress_wcsc_lha stub
9. Round-trip test COMMDV00.DRV -> 1130 bytes byte-perfect
10. Ship v1.0


## Step 10 — Decompressor located, port pending (2026-09-01 late)

### Progress
- install_unpacked.exe (275,872 bytes) verified: MZ EXE, CS:IP=1d91:5c28,
  all target strings visible
- Decompress driver LOCATED: file offset 0x1e93e in install_unpacked.exe
  (Ghidra label FUN_3000_e91e, ~20K code addresses). Classic MSC large-
  model C prologue, 7 stack args, far calls to helpers at seg 0x1089
- 30KB driver code extracted → archivers/redx/refwork/decomp_driver.bin
- 12,077 lines of ndisasm → archivers/redx/refwork/decomp_driver_disasm.txt

### Two string tables recovered

**Table A @ 0x2d736 — LHA-derived core:**
- "huf 2", "huf 3", "huf 4", "huf 5", "huf 7", "huf 10" (method labels)
- "Bad table", "internal error make_table"
- "Unable to write", "decompression phase"
- "Function press() called w/o proper initialization"
- "Out-of-range bias parameter to press()"
- "press 1", "press 2", "press 3" (compression-side levels)

**Table B @ 0x32ace — WCSC I/O layer wrapping the core:**
- "expand", "expand 2" (decompression driver entries)
- "kick_char" (write character to output)
- "f_ram 1", "f_ram 2" (fill-ram-buffer stages)
- "flushram" ("Internal error in flushram")
- "Decompressing: %s" (user-visible progress line)

### Conclusion

Method 0x000B is a WCSC customization of Yoshi Yoshizaki's LHA source
with a custom I/O layer (kick_char/f_ram/flushram). The LHA core is
recognizable (huf/make_table/bad table strings) but standard -lh5/6/7-
decoders produce wrong output, confirming WCSC modified either the
Huffman parameters (NC/NP/NT/PBIT/TBIT), the bit-order, or the block
encoding.

### To finish (3-4 hours focused Ghidra work)

Load install_unpacked.exe interactively in Ghidra:
1. Follow "expand" string ref → outer driver entry
2. Trace method dispatch on huf N labels
3. Trace read_pt_len / read_c_len / decode_c / decode_p (LHA core)
4. Note differences vs stock LHA (parameter values, bit-order, block fmt)
5. Trace kick_char / f_ram / flushram (WCSC I/O layer)
6. Port to Python — replace decompress_wcsc_lha() stub in decompress_v0.2.py
7. Round-trip test on COMMDV00.DRV (target: 1130 bytes byte-perfect)
8. Extend to remaining 9 non-STORED records in COMMDRV.RED
9. Port to C in archivers/redx/red_decompress.c


## v1.4 extended backends — PCB1541-only (2026-09-03)

Three post-WCSC intelligent multiport cards added as `#if defined(PCB1541)`-
gated backends. They ship only in 15.41 builds; 15.4 builds stay at
WCSC-parity (10 core backends).

| Card                    | File                          | Status                |
|-------------------------|--------------------------------|-----------------------|
| Stallion Brumby/ONboard | stallion_brumby_backend.c     | Full impl (untested)  |
| Chase Research IOLAN    | chase_iolan_backend.c         | Full impl (untested)  |
| Equinox SST-8/16/32/64  | equinox_sst_backend.c         | Full impl (untested)  |

**Untested** here means: written from public datasheets + Linux driver
references (Stallion `istallion.c`, Chase BSDI history, Equinox docs).
No physical card in the pcbirc lab for validation. Sysops with these
cards are encouraged to test and file issues.

**Why gated instead of always-on:**
- Keeps 15.4 lean and byte-identical to WCSC feature parity
- Isolates post-WCSC / untested-on-hardware code to the extended line
- Follows same pattern as planned TCP_SOCKET backend (also 15.41-only)

**Build for 15.41:**
    wmake -f PCBDCOM.MAK CC=OWC TARGET=15.41

**Build for 15.4 (default):**
    wmake -f PCBDCOM.MAK CC=OWC
    (or TARGET=15.4 explicitly)

When PCB1541 is not defined, the extended backends compile to empty
translation units (verified: 0 errors, 0 warnings under gcc without
`-DPCB1541`). No code, no data, no symbols leak into 15.4 binaries.
