#!/usr/bin/env python3
# ============================================================================
# decompress_v0.2.py — WCSC .RED method 0x000B decompressor
#                     (LHA-derived custom variant)
#
# STATUS: v0.2 — decompressor code LOCATED and extracted from unpacked
#         INSTALL.EXE. String map complete, driver disassembly saved.
#         Full port to Python still pending (multi-hour Ghidra sit-down).
#
# ============================================================================
# What we KNOW (v0.2, 2026-09-01 late)
# ============================================================================
#
# 1. Container format (fully reversed, see GAP-ANALYSIS.md):
#    Offset  Size  Field
#    0-1     2     Magic 'RR'
#    2       1     Version (0x01)
#    3-7     5     Timestamp / archive ID
#    8-11    4     Compressed size (LE u32)
#    12-15   4     Uncompressed size (LE u32)
#    16-17   2     Marker 0xFFFF
#    18-19   2     CRC16 of uncompressed data
#    20-21   2     Padding 0x0000
#    22-23   2     Const 0x0001
#    24-25   2     Method: 0x0001=STORED, 0x000B=LHA-family
#    26+     var   Null-terminated filename
#    after   var   Compressed payload
#    +2      2     CRC16 trailer
#
# 2. STORED (method 0x0001): payload IS the file — verified byte-perfect on
#    MONITOR.BAT.
#
# 3. Method 0x000B is a WCSC customization of Yoshi's LHA source. Confirmed
#    by TWO string tables recovered from unpacked INSTALL.EXE:
#
#    Table A @ 0x2d736 — LHA-derived core:
#       "huf 2", "huf 3", "huf 4", "huf 5", "huf 7", "huf 10"
#         (Huffman method labels; gaps at 6/8/9 = methods not compiled in)
#       "Bad table", "internal error make_table"
#       "Unable to write", "decompression phase"
#       "Function press() called w/o proper initialization"
#       "Out-of-range bias parameter to press()"
#       "press 1", "press 2", "press 3" (compression-side levels)
#
#    Table B @ 0x32ace — WCSC I/O layer wrapping the core:
#       "expand", "expand 2"       (decompression driver entries)
#       "kick_char"                 (write a character to output stream)
#       "f_ram 1", "f_ram 2"        (fill-ram-buffer stages)
#       "flushram"                  ("Internal error in flushram")
#       "Decompressing: %s"         (user-visible progress line)
#
# 4. UNPACKER: fully working via Unicorn Engine emulation. Produces valid
#    MZ EXE at /tmp/ghidra-fresh/install_unpacked.exe (275,872 bytes),
#    CS:IP = 1d91:5c28. All target strings visible as plain text.
#
# 5. DECOMPRESS DRIVER: located at file offset 0x1e93e in install_unpacked.exe
#    (Ghidra label FUN_3000_e91e, ~20K bytes of code). MSC large-model C.
#    Classic prologue (55 8B EC 83 EC 14 = push bp / mov bp,sp / sub sp,20).
#    Uses 7 stack args (bp+6 through bp+14) and far calls to helpers at
#    seg 0x1089.
#
# 6. Extracted artifacts (host scratch, in refwork/):
#      decomp_driver.bin           30KB raw driver code
#      decomp_driver_disasm.txt    12,077 lines of ndisasm output
#
# ============================================================================
# Round-trip target
# ============================================================================
#
#   Source archive: COMMDRV.RED (in pcb1541/install/INSTALL.zip)
#   First non-STORED record: COMMDV00.DRV
#     uncompressed size = 1130 bytes
#     compressed size   = 792 bytes
#     compressed first 32 bytes:
#       86 b8 02 c4 6a 82 da c8 da 88 7f ef 7b 66 de 7a
#       ec 2d c0 91 bc 51 17 8a 69 4e 2e 07 12 da e0 2d
#
# ============================================================================
# What we know it's NOT
# ============================================================================
#
# Tested via lhafile.lzhlib C extension:
#   -lh0-: not compressed (obviously wrong)
#   -lh1- / -lh4-: unsupported by lzhlib
#   -lh5- / -lh6- / -lh7-: decompresses but wrong output (Huffman tables
#     interpreted differently; algorithm structure differs from standard)
#   -lzs- / -lz4- / -lz5-: unsupported
#
# ============================================================================
# v1.0 completion plan (est. 3-4 hours focused Ghidra work)
# ============================================================================
#
# Load install_unpacked.exe in Ghidra (interactive), then:
#
# 1. Follow "expand" string ref @ 0x32ace — that names the outer driver.
#    Look for MOV DX,offset_of_expand instructions to find caller.
#
# 2. Trace outer driver: it dispatches on method-id byte read from input.
#    "huf N" strings are debug tags printed if method matches.
#
# 3. Inside each huf N branch: read_pt_len (position tree), read_c_len
#    (char tree), decode_c, decode_p — the LHA-family core functions.
#    Compare against public LHA -lh5- source (huf.c in LHarc 2.13):
#    https://github.com/jca02266/lha (Yoshi's original + community forks)
#
# 4. Note DIFFERENCES from stock LHA:
#    - Huffman parameter values (NC=?, NP=?, NT=?, PBIT=?, TBIT=?)
#    - Bit-order (MSB-first vs LSB-first — probably MSB per LHA convention)
#    - Blocksize field encoding
#    - Whether pt_len is stored or reconstructed
#
# 5. Also trace WCSC I/O layer: kick_char, f_ram (fill ram), flushram.
#    These wrap Yoshi's fillbuf/getbits primitives — usually just buffer
#    management (fill 4KB from disk when depleted, flush 4KB to disk).
#
# 6. Port to Python — replace decompress_wcsc_lha() stub below with real
#    implementation. Keep bit-exact algorithm; portability is 8/16-bit
#    aligned so no endianness surprises in the bit stream.
#
# 7. Round-trip test: decompress_wcsc_lha(COMMDV00.DRV.payload, 1130)
#    output must be byte-perfect against the extracted file that ships in
#    the .DRV of a real PCBoard install.
#
# 8. Once round-trip passes on COMMDV00.DRV, test remaining 9 non-STORED
#    records in COMMDRV.RED, then all records in PCBOARD.RED / PCBMAIL.RED /
#    PCBCFGS.RED / PCBOARD2.RED / PPLC.RED.
#
# 9. Port to C: archivers/redx/red_decompress.c. Same algorithm, same
#    round-trip test.
#
# ============================================================================
# Current implementation
# ============================================================================
import struct, sys


def red_walk(red_data):
    """Yield (offset, filename, method, unc_size, cmp_size, crc, payload) per record."""
    pos = 0
    while pos < len(red_data):
        if red_data[pos:pos+2] != b'RR':
            break
        cmp_size = struct.unpack('<L', red_data[pos+8:pos+12])[0]
        unc_size = struct.unpack('<L', red_data[pos+12:pos+16])[0]
        crc16    = struct.unpack('<H', red_data[pos+18:pos+20])[0]
        method   = struct.unpack('<H', red_data[pos+24:pos+26])[0]
        fn_start = pos + 26
        fn_end   = red_data.index(b'\x00', fn_start)
        fn = red_data[fn_start:fn_end].decode('ascii', errors='replace').rstrip()
        data_start = fn_end + 1
        payload = red_data[data_start:data_start + cmp_size]
        yield pos, fn, method, unc_size, cmp_size, crc16, payload
        pos = data_start + cmp_size + 2  # skip trailing CRC16


def decompress_stored(payload, unc_size):
    """Method 0x0001 — verified byte-perfect."""
    if len(payload) != unc_size:
        raise ValueError(f"STORED size mismatch: payload={len(payload)}, unc={unc_size}")
    return payload


def decompress_wcsc_lha(payload, unc_size):
    """Method 0x000B — WCSC modified LHA-family variant.

    v0.2: still raises NotImplementedError. Driver LOCATED at file 0x1e93e
    in install_unpacked.exe, disassembly saved to decomp_driver_disasm.txt.
    Full port pending 3-4 hours focused Ghidra work per v1.0 completion
    plan above.
    """
    raise NotImplementedError(
        "Method 0x000B: WCSC modified LHA-family. Decompressor driver "
        "located in install_unpacked.exe at file 0x1e93e (Ghidra "
        "FUN_3000_e91e). See decomp_driver_disasm.txt and this file's "
        "header for the v1.0 completion plan."
    )


def red_extract(red_data, filename_filter=None):
    """Walk archive, decompress records, yield (filename, bytes) or (filename, exception)."""
    for pos, fn, method, unc, cmp, crc, payload in red_walk(red_data):
        if filename_filter and fn != filename_filter:
            continue
        try:
            if method == 0x0001:
                yield fn, decompress_stored(payload, unc)
            elif method == 0x000B:
                yield fn, decompress_wcsc_lha(payload, unc)
            else:
                yield fn, ValueError(f"unknown method 0x{method:04x}")
        except Exception as e:
            yield fn, e


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("usage: decompress_v0.2.py <archive.RED> [--extract FILE]")
        sys.exit(1)
    with open(sys.argv[1], 'rb') as f:
        red_data = f.read()
    only = None
    if '--extract' in sys.argv:
        only = sys.argv[sys.argv.index('--extract') + 1]
    for fn, result in red_extract(red_data, only):
        if isinstance(result, Exception):
            print(f"  FAIL {fn}: {type(result).__name__}: {result}")
        else:
            print(f"  OK   {fn}: {len(result)} bytes")
            if only:
                with open(fn, 'wb') as f: f.write(result)
                print(f"  wrote {fn}")
