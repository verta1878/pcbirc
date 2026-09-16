# IC Reconstruction — bit by bit

Rebuild each IC component byte-for-byte, verifying against the shipped
originals in `bin/`. Fix bugs only AFTER byte-exact restoration.

## Targets (byte-exact) and status

| Component     | Original    | Functions | Status |
|---|---|---|---|
| RUNINET.PPE   |   1,808 B   | — | v0.2.8: 2,261 B (PPLC 3.20, 19/19 tests pass) |
| TESTIC.EXE    |  40,104 B   | 249 | ✓ byte-exact, NASM source |
| TESTIC2.EXE   |  46,627 B   | 258 + 76 imports | ✓ byte-exact, NASM + wlink |
| PCBICEVT.EXE  |  89,612 B   | 474 | ✓ byte-exact, NASM source |
| PCBICCFG.EXE  | 185,398 B   | 743 | ✓ byte-exact, NASM source |
| Pcbic.exe     | 313,310 B   | 1,074 | ✓ byte-exact, NASM source |
| Pcbic2.exe    | 217,111 B   | 1,090 + 212 imports | ✓ byte-exact, NASM + wlink |

**All 6 EXE targets: byte-exact SHA256 verified.**

## Compilers

- **DOS:** Borland C++ 3.1, large memory model (Copyright 1991 Borland Intl.)
- **OS/2:** Borland C++ 2.0 for OS/2, 32-bit flat model (Copyright 1994 Borland Intl.)
- All binaries: Copyright 1995-1996 Clark Development Company, Inc.

## OS/2 Cross-Compilation (no VM required)

OS/2 LX binaries rebuilt from Linux:
- **NASM** assembles 32-bit OMF .obj with `extern` import declarations
- **OpenWatcom V2 wlink** links OS/2 LX with `FORMAT OS2 LX`

Pcbic2.exe imports (10 modules, 110 unique): so32dll, tcp32dll, DOSCALLS,
PMWIN, KBDCALLS, VIOCALLS, NLS, PMSHAPI, SESMGR, QUECALLS

TESTIC2.EXE imports (4 modules, 48 unique): tcp32dll, so32dll, DOSCALLS, VIOCALLS

## PCBICEVT.EXE — Event Manager

PCBoard's Internet Collection Event Manager. Processes holding directories,
ranks entries by popularity (`/RANK`), purges inactive entries (`/PURGE`),
force-approves files (`/FORCE`). Reads TCPIP.DAT. Uses Clark's VMData
virtual memory subsystem (source paths `c:\vmdata\src\`).

## Source layout

```
src/dos/pcbic_code.asm         — Pcbic.exe NASM (1,074 functions)
src/dos/pcbiccfg_code.asm      — PCBICCFG.EXE NASM (743 functions)
src/dos/pcbicevt_code.asm      — PCBICEVT.EXE NASM (474 functions)
src/dos/testic_code.asm        — TESTIC.EXE NASM (249 functions)
src/os2/pcbic2_code.asm        — Pcbic2.exe NASM with import externs
src/os2/pcbic2_data.asm        — Pcbic2.exe data section
src/os2/pcbic2.lnk             — wlink script (110 imports)
src/os2/testic2_code.asm       — TESTIC2.EXE NASM with imports
src/os2/testic2_data.asm       — TESTIC2.EXE data section
src/os2/testic2.lnk            — wlink script (48 imports)
```
