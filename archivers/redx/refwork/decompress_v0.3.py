#!/usr/bin/env python3
# ============================================================================
# decompress_v0.3.py — WCSC .RED method 0x000B decompressor + oracle harness
#
# STATUS: v0.3 — ORACLES ACQUIRED for round-trip testing. 10 payload/oracle
#         pairs saved in pairs/ subdirectory, extracted from CSBACKUP.ARJ
#         (PCBoard 15.22 preservation snapshot in reference/roysac/).
#
#         Whoever finishes the port now has INSTANT verification:
#             python3 decompress_v0.3.py --test-all
#         will report which pairs decompress correctly.
#
# ============================================================================
# What we KNOW (v0.3, 2026-09-01 late night)
# ============================================================================
#
# 1. Container format: fully reversed (see GAP-ANALYSIS.md Step 8-9)
#
# 2. Method 0x0001 (STORED): verified byte-perfect
#
# 3. Method 0x000B: WCSC customization of Yoshi's LHA source
#    - Two string tables recovered from unpacked INSTALL.EXE
#    - Decompress driver at file offset 0x1e93e in install_unpacked.exe
#      (Ghidra label FUN_3000_e91e, ~20K code addresses)
#    - MSC large-model C, prologue 55 8B EC 83 EC 14
#    - Standard -lh0/1/2/3/4/5/6/7- all FAIL round-trip on our payloads
#
# 4. TEST VECTORS (this session's key contribution):
#    10 payload/oracle pairs from CSBACKUP.ARJ (extracted PCBoard 15.22
#    install with matching sizes to COMMDRV.RED entries):
#
#      pairs/COMMDV00.DRV.payload    792 bytes  -> .oracle   1130 bytes
#      pairs/COMMDV01.DRV.payload    836 bytes  -> .oracle   1115 bytes
#      pairs/COMMDV02.DRV.payload   1561 bytes  -> .oracle   2276 bytes
#      pairs/COMMDV03.DRV.payload   1720 bytes  -> .oracle   2686 bytes
#      pairs/COMMDV04.DRV.payload   1717 bytes  -> .oracle   2797 bytes
#      pairs/COMMDV05.DRV.payload   2919 bytes  -> .oracle   4883 bytes
#      pairs/COMMDV06.DRV.payload   1182 bytes  -> .oracle   1662 bytes
#      pairs/COMMDV07.DRV.payload    845 bytes  -> .oracle   1212 bytes
#      pairs/COMMDV08.DRV.payload   1338 bytes  -> .oracle   2284 bytes
#      pairs/COMMDRV.EXE.payload  45807 bytes  -> .oracle  90827 bytes
#
# 5. Full disassembly of code region available at:
#      archivers/redx/refwork/full_disasm.asm.gz
#    (76,201 lines, ndisasm -b16 output of install_unpacked.exe code
#    region 0..0x2d000, 184,288 bytes). Grep-friendly for xrefs, code
#    patterns, distinctive LHA algorithm signatures.
#
# 6. Payload byte patterns observed across pairs:
#      COMMDV00: 86 b8 02 c4 6a 82 da c8 ...
#      COMMDV01: 37 30 02 ea 6a 82 ee d5 ...
#      COMMDV02: 24 3a 05 91 73 de fd d5 ...
#      COMMDV03: ef 56 05 f5 73 db da 36 ...
#      COMMDV04: af 49 06 1d 73 f7 7e d6 ...
#      COMMDV05: c9 07 0a 24 7c f7 7e ea ...
#    Every payload ends with 0x00 (bit-alignment padding? EOS marker?)
#    No obvious fixed header prefix — this is raw compressed stream.
#
#    First byte varies widely (0x86, 0x37, 0x24, 0xef, 0xaf, 0xc9) —
#    consistent with LSB-first bit stream where high bits carry meaning.
#
# ============================================================================
# v1.0 completion plan (est. 3-4 hours focused Ghidra)
# ============================================================================
#
# Load install_unpacked.exe in Ghidra:
#
# 1. Follow "expand" string ref @ 0x32ace to outer driver.
#
# 2. Trace driver: it dispatches on the method-id (from RED header byte 24)
#    which is 0x000B for our records. Follow that branch.
#
# 3. Trace read_pt_len (position tree) / read_c_len (char tree) /
#    decode_c / decode_p — LHA-family core functions. The strings
#    "Bad table", "make_table" pinpoint make_table itself, its callers
#    are read_pt_len and read_c_len.
#
# 4. Note DIFFERENCES from stock LHA -lh5- source:
#    - Parameter values: NC (# of char codes), NP (# of position codes),
#      NT (# of tree codes), PBIT, TBIT, CBIT
#    - Bit-order: MSB-first (standard LHA) vs LSB-first (unpacker had LSB;
#      the payload's variable first bytes are consistent with LSB-first)
#    - Blocksize encoding
#
# 5. Trace WCSC I/O layer wrapping the LHA core:
#    - kick_char (per-character write)
#    - f_ram 1 / f_ram 2 (fill-input-buffer stages)
#    - flushram (flush-output-buffer)
#
# 6. Port to Python — replace decompress_wcsc_lha() below with real impl.
#
# 7. Run: python3 decompress_v0.3.py --test-all
#    All 10 pairs must show OK. Any FAIL means the port has a bug.
#
# 8. Once Python round-trips all 10 pairs, port to C:
#    archivers/redx/red_decompress.c
#
# ============================================================================
# Current implementation
# ============================================================================
import struct, sys, os, hashlib




# ============================================================================
# v0.3.1 update — LOAD BASE and callback locations pinned down
# ============================================================================
#
# CRITICAL: install_unpacked.exe was loaded at PARAGRAPH 0x100. So:
#   segment S physical address = (S - 0x100) * 16
#   file offset               = physical + 32 (MZ header)
#
# Method-callback wrappers live in segment 0x1089 at offsets:
#   0x25c2 -> file 0x11e72
#   0x268c -> file 0x11f3c
#   0x277e -> file 0x1202e
#
# All three have identical shape:
#   1. Read header field at [es:bx+0x13/15] to check something
#   2. Push 5-8 args, MOV CX,5 (buffer size)
#   3. Push far ptr to string in seg 0x2e71 (the string segment)
#   4. FAR CALL 0x100:0xf42c (file 0xf44c) — the actual I/O wrapper
#   5. On error, print message via 0x1f91:0x636c (fprintf) or exit via
#      0x1f91:0x5e03
#
# Segment map recovered:
#   seg 0x0100 = program image base (helpers, C runtime interface)
#   seg 0x1089 = LHA library code (~180 functions found)
#   seg 0x1f91 = C runtime (printf, fprintf, exit)
#   seg 0x2d91 = data segment where "huf N" / "Bad table" strings live
#   seg 0x2e71 = additional string segment
#
# The 3 callbacks are dispatch wrappers, NOT the compression core itself.
# The actual decompression bit-loops (make_table, decode_c, decode_p) are
# reached VIA helper_f42c (0x100:0xf42c) which further calls into the
# 0x1089 function bank. Need Ghidra to trace the full chain.


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
    """Method 0x000B — WCSC modified LHA-family variant. NOT YET IMPLEMENTED.

    v0.3: still raises NotImplementedError. Test vectors ready in pairs/.
    See file header for v1.0 completion plan.
    """
    raise NotImplementedError(
        "Method 0x000B: WCSC LHA variant. See v1.0 completion plan in "
        "this file's header. 10 test vectors in pairs/ enable round-trip "
        "verification: python3 decompress_v0.3.py --test-all"
    )


def red_extract(red_data, filename_filter=None):
    """Walk archive, decompress records, yield (filename, bytes|exception)."""
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


def test_all_pairs():
    """Round-trip test: load each pair, decompress payload, compare vs oracle.

    Success criterion: byte-perfect match. Any difference = decompressor bug.
    Zero pairs pass in v0.3 because decompress_wcsc_lha() is a stub.
    """
    pairs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pairs')
    if not os.path.isdir(pairs_dir):
        print(f"No pairs/ dir at {pairs_dir}")
        return
    files = sorted(f for f in os.listdir(pairs_dir) if f.endswith('.payload'))
    print(f"{'Record':22s}  {'CmpSz':>7s}  {'UncSz':>7s}  Result")
    print('-' * 70)
    passed = 0; failed = 0
    for fn in files:
        base = fn[:-len('.payload')]
        with open(os.path.join(pairs_dir, base + '.payload'), 'rb') as f: cmp = f.read()
        with open(os.path.join(pairs_dir, base + '.oracle'),  'rb') as f: oracle = f.read()
        try:
            got = decompress_wcsc_lha(cmp, len(oracle))
            if got == oracle:
                result = 'OK (byte-perfect)'
                passed += 1
            else:
                # Count matching prefix bytes
                match_prefix = 0
                for a, b in zip(got, oracle):
                    if a != b: break
                    match_prefix += 1
                result = f'MISMATCH (first {match_prefix}B ok, got {len(got)}, want {len(oracle)})'
                failed += 1
        except NotImplementedError as e:
            result = 'NOT IMPLEMENTED'
            failed += 1
        except Exception as e:
            result = f'EXCEPTION: {type(e).__name__}: {e}'
            failed += 1
        print(f"{base:22s}  {len(cmp):>7d}  {len(oracle):>7d}  {result}")
    print('-' * 70)
    print(f"passed {passed}, failed {failed}")
    return passed, failed


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("usage:")
        print("  decompress_v0.3.py --test-all           run oracle round-trip on all pairs/")
        print("  decompress_v0.3.py <archive.RED>       list contents of a .RED archive")
        print("  decompress_v0.3.py <archive.RED> --extract FILE")
        sys.exit(1)
    if sys.argv[1] == '--test-all':
        test_all_pairs()
        sys.exit(0)
    with open(sys.argv[1], 'rb') as f: red_data = f.read()
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
