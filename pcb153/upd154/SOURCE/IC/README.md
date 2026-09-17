# PCBIC v1.2 Reconstruction Source

All 6 Clark Development binaries from PCBoard's Internet Collection v1.2,
byte-exact verified against originals.

## DOS (BCC 3.1 large model)
| File | Binary | Functions | Source |
|---|---|---|---|
| pcbic_code.asm | Pcbic.exe (313,310B) | 1,074 | NASM raw db |
| pcbiccfg_code.asm | PCBICCFG.EXE (185,398B) | 743 | NASM raw db |
| pcbicevt_code.asm | PCBICEVT.EXE (89,612B) | 474 | NASM raw db |
| testic_code.asm | TESTIC.EXE (40,104B) | 249 | NASM raw db |

## OS/2 (BC++ 2.0 for OS/2, 32-bit LX)
| File | Binary | Functions | Imports | Source |
|---|---|---|---|---|
| pcbic2_code.asm | Pcbic2.exe (217,111B) | 1,090 | 212/110 | NASM + wlink |
| testic2_code.asm | TESTIC2.EXE (46,627B) | 258 | 76/48 | NASM + wlink |

## Build (OS/2 cross-compile on Linux)
```
nasm -f obj -o pcbic2_code.obj os2/pcbic2_code.asm
nasm -f obj -o pcbic2_data.obj os2/pcbic2_data.asm
wlink @os2/pcbic2.lnk
```
Requires: NASM 2.x, OpenWatcom V2 (wlink)
