.RED Archive Format (Clark Development Company)
================================================

Used by INSTALL.EXE to distribute PCBoard files.
Custom compressed archive format.

Header (25 bytes):
  Offset 0-1:   "RR" signature (0x52 0x52)
  Offset 2:     Version (0x01)
  Offset 3-6:   CRC or total compressed size (LE uint32)
  Offset 7-10:  Total uncompressed size? (LE uint32)
  Offset 11-14: Offset to data? (LE uint32)
  Offset 15-16: 0xFFFF marker or reserved
  Offset 17-20: Unknown (LE uint32)
  Offset 21-22: File count (LE uint16) — always 0x0001?
  Offset 23-24: Filename length (LE uint16)

File entries:
  - Null-terminated filename (e.g. "PCBOARD.EXE\0")
  - Compressed data (LZ-based, needs reverse engineering)

Known .RED archives:
  PCBOARD.RED   — PCBOARD.EXE (1.0MB compressed to 1.0MB)
  PCBOARD2.RED  — OS/2 binaries (PCBCP, PCBOARD2, etc.)
  PPLC.RED      — PPL compiler + sample PPEs with source
  COMMDRV.RED   — Communication driver package
  PCBMAIL.RED   — Windows mail client + DLLs + help
  PCBCFGS.RED   — Config file templates

Compression: appears to be LZ-based (similar to LZSS or LZW).
The strings command shows filenames at predictable offsets,
suggesting the file table is uncompressed but the data is.

TODO: Disassemble INSTALL.EXE decompression routine to determine
exact algorithm. May be a standard algorithm (LZSS, LZ77) or
custom Clark implementation.
