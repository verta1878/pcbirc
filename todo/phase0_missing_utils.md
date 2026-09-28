# Phase 0: Missing Clark Utilities — COMPLETE ✅

## All 12 Clark Utilities Linked

| # | Binary | Size | Description |
|---|--------|------|-------------|
| 1 | PCBSTATS_W.EXE | 31K | Statistics generator |
| 2 | PCBPACK_W.EXE | 84K | Message base packer |
| 3 | MSETUP_W.EXE | 108K | Modem database editor |
| 4 | PCBMODEM_W.EXE | 528K | Modem config utility |
| 5 | PCBEDIT_W.EXE | 133K | Text/config editor |
| 6 | PCBMONI_W.EXE | 54K | Node monitor |
| 7 | PCBDIAG_W.EXE | 552K | Diagnostics utility |
| 8 | PCBFILER_W.EXE | 215K | File management utility |
| 9 | PCBNLC_W.EXE | 77K | Nodelist compiler (FidoNet) |
| 10 | OFFLINE_W.EXE | 26K | Offline flag utility |
| 11 | WAITBU_W.EXE | 25K | Wait for backup |
| 12 | PCBTITLE_W.EXE | 15K | OS/2 console title setter |

Total linked size: ~1,848K (12 binaries)

## New Libraries Written for Phase 0

### VMAVL — AVL Tree for Virtual Memory DataSets
- **Author**: sysop/0
- `PCBSRC/LIB/H/vmavl.h` — header (99 lines)
- `PCBSRC/LIB/SOURCE/VMAVL.C` — implementation (292 lines)
- Replacement for Clark's proprietary VMAVL library
- In-memory AVL tree (no disk paging needed on modern systems)
- 7 public functions: VMAVLControlInit, VMAVLTreeInit, VMAVLAdd,
  VMAVLSearch, VMAVLFirstGet, VMAVLNextGet, VMAVLWalkContextInit
- Required by PCBFILER (32 source files, largest Phase 0 utility)

### VMData — Virtual Memory DataSet
- **Author**: hexadecimal (v0.036)
- `PCBSRC/LIB/H/VMDATA.H` — header (77 lines)
- `PCBSRC/LIB/SOURCE/VMFUNCS.C` — implementation (127 lines)
- In-memory array with VMDataSet struct, record create/get/count

### d4all.h — CodeBase Type Shim
- **Author**: sysop/0
- `PCBSRC/LIB/H/d4all.h` — header (121 lines)
- Opaque type declarations for CODE4, DATA4, FIELD4INFO, TAG4INFO
- Allows dbase.hpp (Clark's CodeBase C++ wrapper) to compile
- Reference: CodeBase-for-DBF on GitHub (LGPL v3.0, Sequiter Software)
- Required by PCBNLC (nodelist compiler)

### conio_compat — Borland conio for Watcom
- **Author**: sysop/0
- `PCBSRC/MAIN/OBJ_WAT/conio_compat.c` — 108 lines
- gotoxy, wherex, wherey, clreol, _setcursortype, puttext, movetext
- BIOS INT 10h + direct video memory at 0xB8000
- Required by PCBEDIT

## PCBNLC — Clark's Nodelist Compiler

PCBNLC is Clark Development's FidoNet nodelist compiler. It is the PCBoard
utility — not to be confused with wrench's standalone nlcomp (retired, see below).

### What PCBNLC does:
- Reads PCBOARD.DAT for paths, reads FidoNet config for nodelist definitions
- Compiles NODELIST.### → NODELIST.DBF (11 fields) + NODELIST.NDX (ZONE+NET+NODE)
- Applies NODEDIFF.### files incrementally (DIFF.C, 337 lines)
- Supports multiple nodelists via FidoNet config
- Creates backups: NODELIST.DBK, NODELIST.IBK
- CLI node lookup: `PCBNLC /find zone net node`
- ANSI progress bar during compilation

### Current status:
- 2/2 source files compiled (PCBNLC.C + DIFF.C)
- Linked with CodeBase type stubs (d4all.h shim + cDBF method stubs)
- dBASE file I/O is stubbed — needs real CodeBase integration for runtime
- CodeBase source available: github.com/MPSystemsServices/CodeBase-for-DBF (LGPL v3.0)

### Retired: wrench's nlcomp
wrench's clean-room nodelist compiler (nlcomp) is retired from the distribution.
It was built standalone with CodeBase 4.x but does not integrate with PCBoard's
config system (PCBOARD.DAT, FidoNet config structs) and may not support nodediffs.
PCBNLC is the correct utility for the PCBoard 15.4 distribution.
