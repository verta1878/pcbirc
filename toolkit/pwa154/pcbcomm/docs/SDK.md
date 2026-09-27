# pcbcomm SDK

Free-software (GPLv3) drop-in replacement for WCSC's COMMDRV.OBJ.

## Contents

    src/        Full source (all backends + shim)
    inc/        Public headers (PCBCOMM.H, COMM.H)
    lib/        Pre-built .OBJ and .LIB per compiler + memory model
    docs/       This directory
    examples/   Sample apps

## What ships

Door developers link TWO things:

1. An override .OBJ (COMMDRV.OBJ or FOSSIL.OBJ) — replaces default
   modem handling in the toolkit lib
2. commdrbl.lib + libsbl.lib — the ser_rs232_* API (13 + 6 functions)

## Link matrix — override OBJs

| Compiler   | Large | Flat | Status     |
|------------|-------|------|------------|
| BC 3.1     | ✅    | —    | BUILT      |
| OpenWatcom | —     | ✅   | BUILT      |
| TC 2.01    |       | —    | pending    |
| MSC 7.0    |       | —    | pending    |
| BCOS2      | —     |      | pending    |

## Link matrix — commdrbl.lib + libsbl.lib

| Compiler   | Large | Flat | Status     |
|------------|-------|------|------------|
| BC 3.1     | ✅    | —    | BUILT      |
| OpenWatcom | —     |      | pending    |
| TC 2.01    |       | —    | pending    |
| MSC 7.0    |       | —    | pending    |
| BCOS2      | —     |      | pending    |

## Calling convention

cdecl (C), NOT pascal. Clark's COMMDRV.OBJ imports `_ser_rs232_init`
(lowercase + underscore = cdecl). COMM.H LIBENTRY is empty.

Corrected 2026-09-26 — original COMM.H had `LIBENTRY pascal` which
produced uppercase symbols that would fail to link.

## Substitution recipe

Replace Clark's `COMMDRV.OBJ` with ours in your link line. Add
`commdrbl.lib + libsbl.lib` to your library list. Keep `FOSSIL.OBJ`
as-is. Everything else stays the same.

## File locations

    pcbcbase/COMMDRV/OBJ/   Full 10K override OBJs (PCBOARD.EXE)
    pcbcbase/COMMDRV/LIB/   commdrbl.lib + libsbl.lib
    pcbcbase/COMMDRV/H/     COMM.H (cdecl-corrected)
    pcbcbase/COMMDRV/SRC/   FOSSIL.C, commdrbl.c, libsbl.c

Note: Clark shipped 3K stub OBJs in the toolkit ZIP — placeholders
he never replaced before Clark Development closed. pwa153/154
preserves those stubs. delta154 ships the real OBJs.

## License

GPLv3. Include source or written offer to obtain source per §6.
