#!/usr/bin/env python3
"""
ppedecrypt.py — Decrypt PPE files to raw plaintext bytecode.

Implements Clark's two-layer PPE encryption from CRYPT.C:
  decrypt2: seed 0xDB24, XOR/ROR chain (PPL 3.01+)
  decrypt3: XOR with "DECOMPILERS SUCK!" key (PPL 3.30 only)

Usage:
  python3 ppedecrypt.py input.ppe [output.bin]
  python3 ppedecrypt.py --compare a.ppe b.ppe

Source: LIB/SOURCE/MISC/CRYPT.C (OS/2 C version, lines 284-356)
        LIB/H/MISC.H (#define decrypt3 = encrypt3, XOR is self-inverse)

hexadecimal, 2026-09-07 — pcbirc project
"""

import sys
import struct


def ror16(val, n):
    """16-bit rotate right."""
    n &= 15
    return ((val >> n) | (val << (16 - n))) & 0xFFFF


def ror8(val, n):
    """8-bit rotate right."""
    n &= 7
    return ((val >> n) | (val << (8 - n))) & 0xFF


def decrypt2(data):
    """
    decrypt2 from CRYPT.C (OS/2 version, line 284).
    Seed 0xDB24, ROR/XOR chain. Operates on 16-bit words.
    """
    buf = bytearray(data)
    seed = 0xDB24
    length = len(buf)
    num_words = length >> 1

    for i in range(num_words):
        x = num_words - i  # countdown like the C code
        word_offset = i * 2
        word = struct.unpack_from('<H', buf, word_offset)[0]

        temp = (x & 0xFF) | ((x & 0xFF) << 8)
        rot_val = ((seed & 0xFF) + (x & 0xFF)) & 0xFF

        save = word
        decrypted = (ror16(word, rot_val) ^ temp) ^ seed
        struct.pack_into('<H', buf, word_offset, decrypted & 0xFFFF)
        seed = save

    if length & 1:
        last_byte = buf[-1]
        buf[-1] = ror8(last_byte ^ (seed & 0xFF), rot_val)

    return bytes(buf)


# "DECOMPILERS SUCK!" obfuscated as hex bytes (CRYPT.C line 361-362)
SUCK = bytes([0x8C, 0x53, 0xB8, 0xA7, 0x9E, 0x0F, 0x0A, 0xCB,
              0x28, 0x62, 0x2D, 0x50, 0x7E, 0x05, 0x3D, 0x4E, 0x35])
SUCKLEN = 17


def decrypt3(data):
    """
    decrypt3 = encrypt3 (XOR is self-inverse).
    From CRYPT.C line 364. XOR with "DECOMPILERS SUCK!" key.
    """
    buf = bytearray(data)
    recycle = SUCKLEN
    p = 0  # index into SUCK

    for i in range(len(buf)):
        remaining = len(buf) - i
        buf[i] ^= (SUCK[p] + (remaining & 0xFF)) & 0xFF
        recycle -= 1
        if recycle:
            p += 1
        else:
            p = 0
            recycle = SUCKLEN

    return bytes(buf)


def decrypt_ppe(data):
    """
    Decrypt a PPE file. Returns (header, var_count, plaintext).
    Header is 48 bytes (unencrypted). Byte 48 = var count.
    Bytes 49+ are encrypted variable table + bytecode.

    Layer order on read (NEWSCR.CPP line 1350-1352):
      if version == 3.30: decrypt3 first
      then: decrypt2
    """
    header = data[:48]
    var_count = data[48]

    # Detect PPE version from header string
    header_str = header.decode('ascii', errors='replace')
    is_330 = '3.30' in header_str

    encrypted = data[49:]

    # Apply decryption layers
    if is_330:
        decrypted = decrypt3(encrypted)
        decrypted = decrypt2(decrypted)
    else:
        decrypted = decrypt2(encrypted)

    return header, var_count, decrypted, is_330


def compare_ppe(file_a, file_b):
    """Compare two PPE files after decryption, ignoring vtable pointers."""
    with open(file_a, 'rb') as f:
        data_a = f.read()
    with open(file_b, 'rb') as f:
        data_b = f.read()

    hdr_a, vars_a, plain_a, v330_a = decrypt_ppe(data_a)
    hdr_b, vars_b, plain_b, v330_b = decrypt_ppe(data_b)

    print(f"File A: {file_a}")
    print(f"  Size: {len(data_a)} B, Vars: {vars_a}, Version: {'3.30' if v330_a else '3.20'}")
    print(f"File B: {file_b}")
    print(f"  Size: {len(data_b)} B, Vars: {vars_b}, Version: {'3.30' if v330_b else '3.20'}")
    print()

    if vars_a != vars_b:
        print(f"Variable count differs: {vars_a} vs {vars_b}")
        return False

    if len(plain_a) != len(plain_b):
        print(f"Plaintext size differs: {len(plain_a)} vs {len(plain_b)}")
        return False

    # Compare decrypted plaintext
    diffs = []
    for i in range(len(plain_a)):
        if plain_a[i] != plain_b[i]:
            diffs.append(i)

    if not diffs:
        print("DECRYPTED PLAINTEXT: BYTE-EXACT MATCH!")
        return True

    print(f"Decrypted plaintext differs at {len(diffs)} bytes:")

    # Analyze diff pattern (vtable detection)
    blocks = []
    start = diffs[0]
    prev = diffs[0]
    for d in diffs[1:]:
        if d == prev + 1:
            prev = d
        else:
            blocks.append((start, prev - start + 1))
            start = d
            prev = d
    blocks.append((start, prev - start + 1))

    block_sizes = [s for _, s in blocks]
    if len(set(block_sizes)) == 1:
        print(f"  {len(blocks)} blocks of exactly {block_sizes[0]} bytes each")
        print(f"  (likely vtable pointer serialization artifact)")
    else:
        for offset, size in blocks[:10]:
            print(f"  offset {offset}: {size} bytes")

    return False


def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print("  ppedecrypt.py input.ppe [output.bin]   — decrypt to file")
        print("  ppedecrypt.py --compare a.ppe b.ppe    — compare decrypted")
        sys.exit(1)

    if sys.argv[1] == '--compare':
        if len(sys.argv) != 4:
            print("Usage: ppedecrypt.py --compare a.ppe b.ppe")
            sys.exit(1)
        match = compare_ppe(sys.argv[2], sys.argv[3])
        sys.exit(0 if match else 1)

    # Decrypt mode
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None

    with open(input_file, 'rb') as f:
        data = f.read()

    header, var_count, plaintext, is_330 = decrypt_ppe(data)

    version = '3.30' if is_330 else '3.20'
    print(f"File:      {input_file}")
    print(f"Size:      {len(data)} bytes")
    print(f"Version:   {version}")
    print(f"Variables: {var_count}")
    print(f"Plaintext: {len(plaintext)} bytes")

    if output_file:
        with open(output_file, 'wb') as f:
            f.write(header)
            f.write(bytes([var_count]))
            f.write(plaintext)
        print(f"Written:   {output_file} ({48 + 1 + len(plaintext)} bytes)")
    else:
        # Dump first 64 bytes of plaintext as hex
        print()
        print("First 64 bytes of decrypted plaintext:")
        for i in range(0, min(64, len(plaintext)), 16):
            hex_str = ' '.join(f'{b:02X}' for b in plaintext[i:i+16])
            print(f"  {i:04X}: {hex_str}")


if __name__ == '__main__':
    main()
