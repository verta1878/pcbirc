#!/usr/bin/env python3
# ============================================================================
# decompress_v0.4.py — WCSC .RED method 0x000B decompressor
#                     ***WORKING FOR SMALL RECORDS***
#
# STATUS: v0.4 — CRACKED for small records. Method 0x000B is standard
#         LHA -lh5- (Yoshi's 8KB-dictionary dynamic-Huffman) with a
#         2-byte WCSC prefix that we simply skip. 9 of 10 test vectors
#         round-trip BYTE-PERFECT.
#
#         Known limitation: COMMDRV.EXE (large, 45KB compressed, 90KB
#         uncompressed) currently fails. Investigation shows lhasa with
#         skip=3 -lh5- produces exactly correct SIZE (90827B) with the
#         first 7398 bytes byte-perfect, then diverges. This suggests
#         WCSC uses a DIFFERENT BLOCK SIZE (or block boundary marker)
#         for large files. Standard LHA-lh5 has max 65535 tokens per
#         block; WCSC might use smaller blocks. Investigating.
#
#         Small records — including all the COMMDV0N.DRV serial drivers
#         needed for pcbdcom — decompress byte-perfect.
#
# ============================================================================
# The crack story
# ============================================================================
#
# The two string tables recovered from install_unpacked.exe misled us:
# "huf 2..huf 10", "make_table", "kick_char", "f_ram", "flushram", "press",
# "expand" all suggested a HEAVILY MODIFIED LHA variant. Actually most of
# that is INSTALL.EXE's own I/O layer + support for MULTIPLE LHA methods
# on the compression side. On the DECOMPRESSION side, WCSC just uses
# stock Yoshi -lh5- with a small custom prefix.
#
# The prefix format is:
#   Small records (unc_size ~< 32KB):   2 bytes (arbitrary — CRC hint?)
#   Large records (unc_size >~ 32KB):   variable — under investigation
#
# The 2 prefix bytes look random per file (COMMDV00: 86 b8, COMMDV01: 37 30,
# etc.) — suggests they're a checksum/hash of some kind, not size or
# method info. We SKIP them; nothing else needs them for decompression.
#
# ============================================================================
# Container format (fully reversed, from GAP-ANALYSIS.md)
# ============================================================================
#   Offset  Size  Field
#   0-1     2     Magic 'RR'
#   2       1     Version (0x01)
#   3-7     5     Timestamp / archive ID
#   8-11    4     Compressed size (LE u32)
#   12-15   4     Uncompressed size (LE u32)
#   16-17   2     Marker 0xFFFF
#   18-19   2     CRC16 of uncompressed data
#   20-21   2     Padding 0x0000
#   22-23   2     Const 0x0001
#   24-25   2     Method: 0x0001=STORED, 0x000B=WCSC-LHA-lh5
#   26+     var   Null-terminated filename
#   after   var   Compressed payload (2-byte prefix + -lh5- stream)
#   +2      2     CRC16 trailer
#
# ============================================================================
# Test results (9/10 byte-perfect against oracles from CSBACKUP.ARJ)
# ============================================================================
#   COMMDV00.DRV     792B -> 1130B    OK
#   COMMDV01.DRV     836B -> 1115B    OK
#   COMMDV02.DRV    1561B -> 2276B    OK
#   COMMDV03.DRV    1720B -> 2686B    OK
#   COMMDV04.DRV    1717B -> 2797B    OK
#   COMMDV05.DRV    2919B -> 4883B    OK
#   COMMDV06.DRV    1182B -> 1662B    OK
#   COMMDV07.DRV     845B -> 1212B    OK
#   COMMDV08.DRV    1338B -> 2284B    OK
#   COMMDRV.EXE   45807B -> 90827B    FAIL (large-file prefix variant TBD)
#
# ============================================================================
import struct, sys, os
import io as _io
try:
    import lzhlib   # pip install lhafile (which pulls lzhlib as C extension)
    HAVE_LZHLIB = True
except ImportError:
    HAVE_LZHLIB = False


class _FakeLhaInfo:
    """Minimal LhaInfo shim for lzhlib.LZHDecodeSession()."""
    __slots__ = ('compress_type','compress_size','file_size','CRC','filename')
    def __init__(self, method, cmp_size, unc_size):
        self.compress_type = method
        self.compress_size = cmp_size
        self.file_size = unc_size
        self.CRC = 0
        self.filename = 'x.bin'


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
    """Method 0x000B — WCSC's LHA-lh5 variant. WORKS FOR SMALL RECORDS.

    Algorithm: skip 2-byte WCSC prefix, then decompress the remainder as
    standard Yoshi -lh5- (8KB dict, dynamic Huffman).

    Large records (>~10KB compressed, ~>32KB uncompressed) currently fail
    with "internal error" from lzhlib — probably use a longer prefix or
    -lh6-/-lh7- for larger dictionary. Under investigation.
    """
    if not HAVE_LZHLIB:
        raise ImportError("Need lzhlib (from lhafile package). "
                          "Install: pip install lhafile")
    stream = payload[2:]  # skip 2-byte prefix
    fin = _io.BytesIO(stream)
    fout = _io.BytesIO()
    info = _FakeLhaInfo(b'-lh5-', len(stream), unc_size)
    session = lzhlib.LZHDecodeSession(fin, fout, info)
    while not session.do_next():
        pass
    return fout.getvalue()


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
    """Round-trip test: load each pair, decompress payload, compare vs oracle."""
    pairs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pairs')
    if not os.path.isdir(pairs_dir):
        print(f"No pairs/ dir at {pairs_dir}")
        return 0, 0
    files = sorted(f for f in os.listdir(pairs_dir) if f.endswith('.payload'))
    print(f"{'Record':22s} {'Cmp':>7s} {'Unc':>7s}  Result")
    print('-' * 70)
    passed = failed = 0
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
                match_prefix = 0
                for a, b in zip(got, oracle):
                    if a != b: break
                    match_prefix += 1
                result = f'MISMATCH (first {match_prefix}B match, got {len(got)}, want {len(oracle)})'
                failed += 1
        except Exception as e:
            result = f'EXCEPTION: {type(e).__name__}: {str(e)[:40]}'
            failed += 1
        print(f"{base:22s} {len(cmp):>7d} {len(oracle):>7d}  {result}")
    print('-' * 70)
    print(f"passed {passed}, failed {failed}")
    return passed, failed


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("usage:")
        print("  decompress_v0.4.py --test-all           round-trip test all pairs/")
        print("  decompress_v0.4.py <archive.RED>       list contents")
        print("  decompress_v0.4.py <archive.RED> --extract FILE")
        sys.exit(1)
    if sys.argv[1] == '--test-all':
        p, f = test_all_pairs()
        sys.exit(0 if f == 0 else 1)
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
