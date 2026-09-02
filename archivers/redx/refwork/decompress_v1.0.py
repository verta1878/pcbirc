#!/usr/bin/env python3
"""
decompress_v1.0.py — WCSC .RED method 0x000B decompressor
                     Yoshi LHA -lh5- family port (working reference impl)

STATUS: v1.0 — 9 out of 10 test vectors pass byte-perfect.
        Ready for use on most .RED files. See KNOWN ISSUES below.

TESTED
    python3 decompress_v1.0.py --test-all
    Result:
      OK       COMMDV00.DRV  (1130B)
      OK       COMMDV01.DRV  (1115B)
      OK       COMMDV02.DRV  (2276B)
      OK       COMMDV03.DRV  (2686B)
      OK       COMMDV04.DRV  (2797B)
      OK       COMMDV05.DRV  (4883B)
      OK       COMMDV06.DRV  (1662B)
      OK       COMMDV07.DRV  (1212B)
      OK       COMMDV08.DRV  (2284B)
      EXCEPT   COMMDRV.EXE   — see KNOWN ISSUES

KNOWN ISSUES
    COMMDRV.EXE (45,807B compressed -> 90,827B uncompressed) decodes
    the first 7398 bytes correctly (MZ header + code) then diverges.
    Same failure mode with lhasa (Yoshi\'s reference LHA impl) — both
    my port and lhasa produce identical wrong output past byte 7398.
    
    This suggests WCSC uses a non-standard chunk boundary or multi-
    stream encoding for large files that stock LHA -lh5- doesn\'t
    handle. Small files (< 8KB output) work perfectly.
    
    TO INVESTIGATE:
      - Interactive Ghidra trace of the expand() driver in
        install_unpacked.exe at file 0x1e93e when handling large input
      - Look for chunk-size logic (per-8KB output? per-4KB compressed?)
      - The kick_char/f_ram/flushram WCSC I/O layer likely has the
        chunking behavior

REVERSE ENGINEERING (verta1878, 2026-09-02)
    Segment map:
      0x100  image base (unpacked code)
      0x1089 LHA library function bank (~180 funcs including huf N variants)
      0x1f91 C runtime
      0x2d91 data seg with string tables
      0x2e71 aux strings
    Real decompress driver: 0x100:0xf42c (file offset 0x1044c)

PARAMETERS (per Yoshi\'s lha_macro.h / huf.h)
    NC   = 510       (256 literals + 254 copy-length codes)
    CBIT = 9         (bits for COUNT field in read_c_len)
    NT   = 19        (# temp codes)
    TBIT = 5         (bits for count of temp table)
    NP   = 14        (# position codes for lh5)
    PBIT = 4         (bits for count of position table)
    DICBIT = 13      (8KB dictionary — lh5 semantics)
    THRESHOLD = 3    (min match length)

WCSC-SPECIFIC WRAPPER
    Prepends 2 bytes (typical) or 3 bytes (for files starting with 0x00)
    before the raw LHA -lh5- bit stream. Meaning unclear — likely
    CRC or archive-format marker. Decoder simply skips this prefix.
"""

DICBIT = 13
DICSIZ = 1 << DICBIT
NC = 510
CBIT = 9
NT = 19
TBIT = 5
NP = DICBIT + 1  # =14 for lh5 encoding
PBIT = 4
THRESHOLD = 3


class BitIO:
    """Exact port of Yoshi's bitio.c."""
    def __init__(self, data):
        self.data = data
        self.pos = 0
        self.bitbuf = 0
        self.subbitbuf = 0
        self.bitcount = 0
        self.fillbuf(16)

    def _getc(self):
        if self.pos < len(self.data):
            c = self.data[self.pos]
            self.pos += 1
            return c
        return 0

    def fillbuf(self, n):
        while n > self.bitcount:
            n -= self.bitcount
            self.bitbuf = ((self.bitbuf << self.bitcount) | (self.subbitbuf >> (8 - self.bitcount))) & 0xFFFF
            self.subbitbuf = self._getc()
            self.bitcount = 8
        self.bitcount -= n
        self.bitbuf = ((self.bitbuf << n) | (self.subbitbuf >> (8 - n))) & 0xFFFF
        self.subbitbuf = (self.subbitbuf << n) & 0xFF

    def getbits(self, n):
        x = self.bitbuf >> (16 - n)
        self.fillbuf(n)
        return x

    def peekbits(self, n):
        return self.bitbuf >> (16 - n)


def make_canonical_tree(bitlen, nchar):
    """Canonical Huffman tree. Returns root as [left, right, value_or_None]."""
    max_len = 0
    for i in range(nchar):
        if bitlen[i] > max_len:
            max_len = bitlen[i]
    if max_len == 0:
        return None
    bl_count = [0] * (max_len + 1)
    for i in range(nchar):
        if bitlen[i] > 0:
            bl_count[bitlen[i]] += 1
    next_code = [0] * (max_len + 2)
    code = 0
    for bits in range(1, max_len + 1):
        code = (code + bl_count[bits - 1]) << 1
        next_code[bits] = code
    root = [None, None, None]
    for i in range(nchar):
        cl = bitlen[i]
        if cl == 0: continue
        c = next_code[cl]
        next_code[cl] += 1
        node = root
        for bit_i in range(cl - 1, -1, -1):
            bit = (c >> bit_i) & 1
            if bit_i == 0:
                if node[bit] is None:
                    node[bit] = [None, None, i]
                else:
                    node[bit][2] = i
            else:
                if node[bit] is None:
                    node[bit] = [None, None, None]
                node = node[bit]
    return root


def decode_from_tree(bio, tree):
    if tree is None: return -1
    node = tree
    while node[2] is None:
        bit = bio.getbits(1)
        if node[bit] is None: return -1
        node = node[bit]
    return node[2]


class Decoder:
    def __init__(self, payload):
        self.bio = BitIO(payload)
        self.blocksize = 0
        self.c_tree = None
        self.pt_tree_char = None
        self.pt_tree_pos = None

    def read_pt_len(self, nn, nbit, i_special):
        n = self.bio.getbits(nbit)
        if n == 0:
            c = self.bio.getbits(nbit)
            return [None, None, c]
        pt_len = [0] * nn
        i = 0
        while i < n:
            c = self.bio.peekbits(3)
            if c != 7:
                self.bio.fillbuf(3)
            else:
                mask = 1 << (16 - 4)
                while mask & self.bio.bitbuf:
                    mask >>= 1
                    c += 1
                self.bio.fillbuf(c - 3)
            pt_len[i] = c
            i += 1
            if i == i_special:
                # Yoshi: while (--c >= 0 && i < NPT) — pre-decrement
                c = self.bio.getbits(2)
                while True:
                    c -= 1
                    if c < 0 or i >= nn: break
                    pt_len[i] = 0
                    i += 1
        # Rest of pt_len stays 0
        return make_canonical_tree(pt_len, nn)

    def read_c_len(self):
        n = self.bio.getbits(CBIT)
        if n == 0:
            c = self.bio.getbits(CBIT)
            self.c_tree = [None, None, c]
            return
        c_len = [0] * NC
        i = 0
        while i < min(n, NC):
            c = decode_from_tree(self.bio, self.pt_tree_char)
            if c < 0: raise ValueError("read_c_len decode fail")
            if c <= 2:
                if c == 0:
                    cnt = 1
                elif c == 1:
                    cnt = self.bio.getbits(4) + 3
                else:  # c == 2
                    cnt = self.bio.getbits(CBIT) + 20
                # Yoshi: while (--c >= 0 && i < NC) — pre-decrement
                while True:
                    cnt -= 1
                    if cnt < 0 or i >= NC: break
                    c_len[i] = 0
                    i += 1
            else:
                c_len[i] = c - 2
                i += 1
        self.c_tree = make_canonical_tree(c_len, NC)

    def decode_c(self):
        if self.blocksize == 0:
            self.blocksize = self.bio.getbits(16)
            self.pt_tree_char = self.read_pt_len(NT, TBIT, 3)
            self.read_c_len()
            self.pt_tree_pos = self.read_pt_len(NP, PBIT, -1)
        self.blocksize -= 1
        return decode_from_tree(self.bio, self.c_tree)

    def decode_p(self):
        j = decode_from_tree(self.bio, self.pt_tree_pos)
        if j < 0: raise ValueError("decode_p fail")
        if j != 0:
            j = (1 << (j - 1)) + self.bio.getbits(j - 1)
        return j


def decompress(payload, unc_size):
    """WCSC .RED 0x000B decompression.

    WCSC prepends a variable-length header before the LHA bit stream:
      * 2 bytes for normal-size records (byte 0 nonzero)
      * 3 bytes for large records (byte 0 == 0, possibly signifying multi-block)
    """
    skip = 3 if payload[0] == 0 else 2
    d = Decoder(payload[skip:])
    out = bytearray()
    ring = bytearray(b' ' * DICSIZ)
    ring_pos = 0
    while len(out) < unc_size:
        c = d.decode_c()
        if c < 0: raise ValueError("decode_c fail")
        if c < 256:
            out.append(c)
            ring[ring_pos] = c
            ring_pos = (ring_pos + 1) & (DICSIZ - 1)
        else:
            count = c - 256 + THRESHOLD
            offset = d.decode_p()
            src = (ring_pos - offset - 1) & (DICSIZ - 1)
            for i in range(count):
                b = ring[(src + i) & (DICSIZ - 1)]
                out.append(b)
                ring[ring_pos] = b
                ring_pos = (ring_pos + 1) & (DICSIZ - 1)
                if len(out) >= unc_size: break
    return bytes(out[:unc_size])


if __name__ == '__main__':
    import sys, os, glob
    if '--test-all' in sys.argv:
        pairs_dir = '/tmp/pcbirc/archivers/redx/refwork/pairs'
        pass_count = fail_count = 0
        for pf in sorted(glob.glob(f'{pairs_dir}/*.payload')):
            of = pf[:-8] + '.oracle'
            with open(pf,'rb') as f: p = f.read()
            with open(of,'rb') as f: o = f.read()
            try:
                got = decompress(p, len(o))
                if got == o:
                    print(f"  OK       {os.path.basename(pf)[:-8]}  ({len(got)}B)")
                    pass_count += 1
                else:
                    n_match = 0
                    for a, b in zip(got, o):
                        if a != b: break
                        n_match += 1
                    print(f"  MISMATCH {os.path.basename(pf)[:-8]}  first {n_match}B ok  "
                          f"got: {got[:12].hex()}  want: {o[:12].hex()}")
                    fail_count += 1
            except Exception as e:
                print(f"  EXCEPT   {os.path.basename(pf)[:-8]}  {type(e).__name__}: {e}")
                fail_count += 1
        print(f"\n  {pass_count} passed, {fail_count} failed")

# APPENDIX: COMMDRV.EXE 7398-byte failure — CONFIRMED WCSC-specific 2026-09-02
# 
# Built Yoshi Watazaki's own LHA v1.14i (from archivers/lha/src/) — the
# EXACT reference implementation this algorithm was ported from — and
# tested it on COMMDRV.EXE payload wrapped as a -lh5- LZH archive.
# 
# Result:
#   COMMDV00.DRV: Yoshi's lha extracts byte-perfect (1130/1130) → matches
#                 our Python port exactly
#   COMMDRV.EXE:  Yoshi's lha fails with "make_table(): Bad table (case b)"
#                 same underlying failure mode as our Python port
# 
# This CONFIRMS the COMMDRV.EXE divergence at 7398 bytes is a WCSC-specific
# variant that stock LHA -lh5- (either lhasa's or Yoshi's reference impl)
# cannot decode. The compression happens INSIDE WCSC's I/O layer wrapping
# (kick_char / f_ram / flushram) rather than the LHA core.
# 
# Since CSBACKUP.ARJ ships extracted COMMDRV.EXE at the correct size and
# content, users needing this specific file can use the oracle directly
# until the WCSC I/O wrapping is reverse-engineered.
