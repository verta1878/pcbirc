#!/usr/bin/env python3
# ============================================================================
# decompress_v0.1.py — WCSC .RED method 0x000B decompressor
#                     (LHA-derived custom variant)
#
# STATUS: v0.1 — algorithm family identified as LHA-family via error strings
#         recovered from unpacked INSTALL.EXE. Standard LHA -lh4/5/6/7-
#         all FAIL round-trip test on real .RED payload, so WCSC's 0x000B
#         is a MODIFIED variant. Full algorithm still needs to be reversed
#         from the decompressor code inside install_unpacked.exe (see
#         "How this file was made" below).
#
# ============================================================================
# What we KNOW
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
# 3. Method 0x000B is LHA-family. Evidence from unpacked INSTALL.EXE's
#    decompressor error strings (recovered via Unicorn Engine emulation
#    on 2026-09-01, see /tmp/commdrv-work/unpacker-analysis/):
#
#       "huf 2" "huf 3" "huf 4" "huf 5" "huf 7" "huf 10"
#       "Unable to write" "Bad table" "internal error make_table"
#       "Can't write output data during decompression phase"
#       "expand" "decompression phase"
#
#    These are the EXACT error messages from Haruyasu Yoshizaki's
#    original LHA source (huf.c / maketbl.c in LHarc 2.13 and later).
#
# 4. Round-trip test target: COMMDV00.DRV (first record in COMMDRV.RED)
#      uncompressed size = 1130 bytes
#      compressed size   = 792 bytes
#      compressed payload first bytes:
#        86 b8 02 c4 6a 82 da c8 da 88 7f ef 7b 66 de 7a
#
# ============================================================================
# What we know it's NOT
# ============================================================================
#
# Tested via lhafile.lzhlib C extension (2026-09-01):
#   -lh0-: not compressed (obviously wrong)
#   -lh1- / -lh4-: unsupported by lzhlib
#   -lh5- / -lh6- / -lh7-: decompresses without error but produces
#     34,488 bytes of mostly 0x22 (Huffman tables interpreted wrong;
#     algorithm structure differs from standard -lh5-)
#   -lzs- / -lz4- / -lz5-: unsupported by lzhlib
#
# So it's a WCSC-modified LHA variant. Likely differences from -lh5-:
#   - Custom Huffman table encoding (pt tree / c tree layout)
#   - Different NC / NP / NT parameters
#   - Possibly custom bit-order or byte-order
#   - Possibly no blocksize field, or different blocksize encoding
#
# ============================================================================
# How this file was made (next session: finish reversing)
# ============================================================================
#
# The unpacker for INSTALL.EXE has been successfully emulated end-to-end.
# The resulting unpacked binary is saved at:
#
#   /tmp/ghidra-fresh/install_unpacked.exe   (275,872 bytes)
#
# It's a proper MZ EXE with:
#   CS:IP = 1d91:5c28
#   All 'huf N' / 'Bad table' / 'expand' strings visible as plain text
#   All INSTALL.DAT scripting keywords visible (@READLN, @MCBSIGNATURE, etc.)
#
# To finish v1.0:
#   1. ghidra -import install_unpacked.exe
#   2. Find references to string "Bad table" at offset ~0x2d74c
#   3. Follow xrefs to the make_table function (a Huffman table builder)
#   4. Its callers are read_pt_len and read_c_len — the tree-length decoders
#   5. Follow up to decode_c / decode_p — main bit-decode loops
#   6. Follow up to the outer decompress driver (probably called "expand")
#   7. Compare against standard LHA -lh5- huf.c line by line
#   8. Note the differences (Huffman param values, bit widths, table sizes)
#   9. Update this file's decompress() function to match
#   10. Round-trip test: this file's output on COMMDV00.DRV should be
#       byte-perfect 1130 bytes matching what INSTALL.EXE would produce.
#
# Estimated: 2-3 hours in Ghidra + 1-2 hours porting.
#
# ============================================================================
# Current implementation: container walker + STORED extraction
# (LHA-variant decompress stubbed out — returns error until reversed)
# ============================================================================
import struct, sys


def red_walk(red_data):
    """Yield (offset, filename, method, unc_size, cmp_size, payload) for each record."""
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
    """Method 0x000B — WCSC modified LHA-family variant. NOT YET IMPLEMENTED.

    v0.1: raises NotImplementedError. See file header for reversing plan.
    v1.0: full implementation ported from install_unpacked.exe analysis.
    """
    raise NotImplementedError(
        "Method 0x000B is a WCSC modified LHA-family variant. "
        "Reverse-engineering pending — see decompress_v0.1.py file header "
        "for the plan (load install_unpacked.exe in Ghidra, port from "
        "the make_table + decode_c functions)."
    )


def red_extract(red_data, filename_filter=None):
    """Walk archive, decompress records, yield (filename, bytes) or (filename, exception)."""
    for pos, fn, method, unc, cmp, crc, payload in red_walk(red_data):
        if filename_filter is not None and fn != filename_filter:
            continue
        try:
            if method == 0x0001:
                yield fn, decompress_stored(payload, unc)
            elif method == 0x000B:
                yield fn, decompress_wcsc_lha(payload, unc)
            else:
                raise ValueError(f"Unknown method {method:#06x}")
        except Exception as e:
            yield fn, e


def crc16_arc(data, init=0):
    """CRC-16/ARC (polynomial 0xA001, init 0, reflected in/out).
    LHA and many DOS archivers use this."""
    crc = init
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc & 0xFFFF


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: decompress_v0.1.py <path-to.RED>")
        print("       Walks archive, extracts STORED records, notes LHA-variant records.")
        sys.exit(1)

    with open(sys.argv[1], 'rb') as f: red = f.read()

    print(f"Archive: {sys.argv[1]}, {len(red)} bytes\n")
    print(f"{'File':16s} {'Method':>8s} {'Unc':>8s} {'Cmp':>8s} Status")
    print("-" * 60)

    for fn, result in red_extract(red):
        if isinstance(result, Exception):
            print(f"{fn:16s} — SKIPPED: {result.__class__.__name__}")
        else:
            print(f"{fn:16s} — OK: {len(result)} bytes")
