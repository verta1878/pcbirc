# Patches

Three patch layers, applied in order:

## 1. 153_to_154.patch — Clark's 15.3→15.4 Changes

**What**: All changes Clark Development made between PCBoard 15.3
and the unreleased 15.4b beta. Applied to the 15.3 source archive
(pcb153src0014) to produce the 15.4 source tree.

**Size**: ~9MB, ~625 files

**How to apply**:
```
cd PCBSRCV/000
patch -p2 < patches/153_to_154.patch
```

This produces the 15.4 source tree that compiles under Borland
C++ 3.1 in DOSBox. All 11 original DOS binaries build from this.

## 2. 154_borland_to_154_watcom.patch — OpenWatcom Port

**What**: Compiler port changes ONLY. No new features. Gets Clark's
15.4 source compiling and linking under OpenWatcom 2.0 on modern
Linux. Changes include:

- `WATCOMPAT.H` — farmalloc→malloc, bioskey→_bios_keybrd, etc.
- `TYPES.HPP` — `_FAR_` defined empty for 386 flat model
- `constrea.h` — constream class for Watcom C++ iostream compat
- `USERS.H` — header guards for cross-compilation
- `QINT.HPP` — Watcom int64 operator guards
- `pcbfiles.h/ext` — include ordering fixes, PCBSM extern block
- `MESSAGES.H`, `VAR.HPP` — synced H/ and H/H/ copies
- Inline ASM removed from ANSI.C, COUNTRY.C, DELAY.C, HANDLERS.C
- Various `bool`/`char`/`unsigned char` type fixes

**Size**: ~196K lines, ~800 files changed

**How to apply**:
```
cd PCBSRC
patch -p3 < patches/154_borland_to_154_watcom.patch
```

After applying, the source compiles under OpenWatcom 2.0 (`wpp386`)
and links 13 binaries (all Clark originals) including PCBOARD_W.EXE.

## 3. 1541_additions.txt — 15.41 Revival File Manifest

**What**: New files added for the 15.41 revival. These do NOT
modify any Clark source — they are entirely new code added on top
of the 15.4-watcom port. Includes:

- ASM→C replacements (ASYNC.C FOSSIL driver, TIMER.C, etc.)
- Standalone tools (pcbtic, pcbfcfg, nlcomp, upd1541, utrayit)
- Installer (pcbis_ui.c, startup/shutdown scripts)
- Documentation (PCB1541_DRAFT.md 20 sections, WHATSNEW, FIDONET)
- SyncTerm reference source (GPL v2+, for future use)

**Not a patch file** — this is a manifest listing the new files.
The files themselves are in the repo. No existing Clark source
is modified by 15.41 features.

## Separation Principle

The three layers are deliberately separate:

```
15.3 source (PWA archive)
  └→ + 153_to_154.patch = 15.4 (Clark's code, Borland)
      └→ + 154_borland_to_154_watcom.patch = 15.4-watcom (port)
          └→ + 1541_additions.txt files = 15.41 (revival)
```

Anyone can stop at any layer:
- **15.4 Borland** — faithful reproduction of Clark's last build
- **15.4 Watcom** — same code, modern compiler, cross-platform
- **15.41** — new features by the pcbirc crew

## 4. pcbcp_watcom_port.patch — PCBCP OS/2 Control Panel

**What**: Ports the PCBCP OS/2 Presentation Manager Control Panel
from IBM C Set/2 to OpenWatcom 2.0. PCBCP was a separately
distributed Clark utility (not in the licensed source archive).

**Source**: pcball.zip from pcboard.be

**Size**: 181 lines, 12 files changed

**Changes**:
- Added `pcbcp_compat.h` (bool typedef, alloc.h→stdlib.h for Watcom)
- Added `#include "pcbcp_compat.h"` to all 8 .C files
- MAIN.C: removed hardcoded `\toolkt21\` valapi.h include path
- THRD.C: `alloc.h` → handled by compat header
- HELP.C: `_argv` → `__argv` (Watcom C runtime global)
- All files: removed Ctrl-Z (0x1A) DOS EOF markers

**How to apply**:
```
cd addons/PCBCP
patch -p3 < patches/pcbcp_watcom_port.patch
```

**Status**: 8/8 files compile clean under OpenWatcom (wpp386 -bt=os2).
Not yet linked — needs OS/2 PM libraries.
