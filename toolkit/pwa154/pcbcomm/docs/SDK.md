# pcbdcom SDK

Free-software (GPLv3) drop-in replacement for WCSC's COMMDRV.OBJ.

## Contents

    src/        Full source (all backends + shim)
    inc/        Public headers (PCBDCOM.H)
    lib/        Pre-built .OBJ variants per compiler + memory model
    docs/       This directory
    examples/   Sample apps

## Link matrix

| Compiler   | Small | Medium | Compact | Large | Huge | Flat |
|------------|-------|--------|---------|-------|------|------|
| BC 3.1     | _BS   | _BM    | _BC     | _BL   | _BH  | —    |
| MSC 7.0    | _7S   | _7M    | _7C     | _7L   | _7H  | —    |
| MSC 8.0    | _8S   | _8M    | _8C     | _8L   | _8H  | —    |
| OpenWatcom | —     | —      | —       | _WL   | —    | _WF  |

Prefix all with `PCBDCOM` (e.g. `PCBDCOM_BL.OBJ`). For PCBoard use
`PCBDCOM_BL.OBJ` (large model, BC-built) to match Clark's original.

## Substitution recipe

Replace `COMMDRV.OBJ` with `PCBDCOM_BL.OBJ` in your link line. Keep
`FOSSIL.OBJ` as-is. Everything else stays the same.

Calling convention: Pascal, callee-cleans, uppercase symbols. Matches
COMM-DRV byte-for-byte; MODEMDRV.C and other legacy consumers link
without any changes.

## License

GPLv3. Include source or written offer to obtain source per §6.
