#!/usr/bin/env python3
"""ne-fixup.py — Post-link NE header fixup for INSTALL.EXE

TLINK 5.1 /Toe sets PROTMODEONLY (0x0008) and zeroes STACKSIZE.
Clark's binary has flags=0x0002 (MULTIPLEDATA only) and stack=8000.
This script patches both fields after linking.

Usage: python3 ne-fixup.py INSTALL.EXE
"""
import struct, sys

if len(sys.argv) < 2:
    print("Usage: ne-fixup.py <NE-binary>")
    sys.exit(1)

path = sys.argv[1]
with open(path, 'rb') as f:
    data = bytearray(f.read())

ne_off = struct.unpack("<I", data[0x3C:0x40])[0]
if data[ne_off:ne_off+2] != b'NE':
    print(f"Not an NE binary (sig={data[ne_off:ne_off+2]})")
    sys.exit(1)

# Fix flags: clear PROTMODEONLY (0x0008)
old_flags = struct.unpack('<H', data[ne_off+0x0C:ne_off+0x0E])[0]
new_flags = old_flags & ~0x0008
struct.pack_into('<H', data, ne_off+0x0C, new_flags)

# Fix stack: set to 8000 (matches Clark's reference)
old_stack = struct.unpack('<H', data[ne_off+0x12:ne_off+0x14])[0]
struct.pack_into('<H', data, ne_off+0x12, 8000)

with open(path, 'wb') as f:
    f.write(data)

print(f"ne-fixup: flags 0x{old_flags:04X}->0x{new_flags:04X}, stack {old_stack}->{8000}")
