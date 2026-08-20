# SIO V1 — Clean-Room Serial I/O Driver Suite for OS/2

**Release:** V1.1.0 — August 7, 2026 23:50 UTC
**License:** GPLv3 (clean-room reimplementation)
**Status:** Feature complete. 16 bugs + 5 missing features + 3 warnings fixed. 9 binaries + FOSTEST. 0 compile errors.

## Components

| Binary | Lines | Size | Format | Description |
|--------|-------|------|--------|-------------|
| SIO.SYS | 2,422 | 6,140 B | LX | OS/2 PDD — replaces COM.SYS |
| VSIO.SYS | 596 | 2,731 B | LX | VDD — virtualizes COM for DOS VDMs |
| VX00.SYS | 953 | 1,072 B | MZ | FOSSIL driver (INT 14h) for DOS VDMs |
| VMODEM.EXE | 1,150 | 32,449 B | LX | Telnet/VMP virtual modem |
| SU.EXE | 326 | 15,923 B | LX | Port status utility |
| PMLM.EXE | 274 | 19,809 B | LX | Line monitor |
| VIEWPMLM.EXE | 189 | 19,073 B | LX | Trace file viewer |
| INSTALL.EXE | 183 | 21,425 B | LX | Driver installer |
| D4TEST.EXE | 592 | 19,235 B | LX | 37-test conformance harness |

## Build

Requires OpenWatcom v2. Set WATCOM environment variable and run `make`.

## Included

- `mystic_bbs/` — Mystic BBS recovered source (compiles, 64K lines)
- `doc/` — Original SIO v1 documentation (SIOREF.TXT etc.)
- `BUGFIXES.md` — Full audit report (31 bugs found across 2 audits, all fixed)
- `CHANGELOG.md` — Complete build history
- `LICENSE.md` — GPLv3 + clean-room declaration
