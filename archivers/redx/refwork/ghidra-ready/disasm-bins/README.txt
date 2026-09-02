Raw byte extracts from install_unpacked.exe for offline disassembly:

  kick_char.bin          146 bytes, file offset 0x1e356, seg 0x1089:0xeb06
  f_ram.bin              56 bytes,  file offset 0x1e3e8, seg 0x1089:0xeb38
  mid_func.bin           132 bytes, file offset 0x1e420, seg 0x1089:0xeb70
  flushram_candidate.bin 134 bytes, file offset 0x1e4a4, seg 0x1089:0xebf4

Disassemble with:
  ndisasm -b 16 -o 0xeb38 f_ram.bin

Or in Ghidra: load install_unpacked.exe, then Go → File Offset → 0x1e356.
