# PCBoard 15.4 Source — License Information

## PCBoard Source Code
Copyright (C) 1996 Clark Development Company, Inc. All Rights Reserved.

The PCBoard source code is proprietary software. You are granted the right
to use this source code for the building of any of the PCBoard products you
have licensed. Any other usage is forbidden without prior written consent
from Clark Development Company, Inc.

Original 15.3 source preserved and distributed by **PWA (Pirates with Attitude)**.
Without PWA's preservation of pcb153src0014.zip, this source recovery would not
have been possible. The PCBoard source code was purchased by Corey Blake from
Clark Development Company — possibly the only source license sold before Clark
was closed by the bank. PWA ensured it survived.

## pcbrevival Crew

- **hexadecimal** — pcbrevival source maintainer, 15.3→15.4 port
- **verta1878** — pcbrevival crew

## Our Additions (15.3→15.4 source port)

The following files created during the 15.3→15.4 source recovery are
released under the **GNU General Public License v3.0 (GPL-3.0)**:

### Build Infrastructure
- `BUILD_DOS.BAT` — DOS build script (10 targets → `\OUT\BIN\`)
- `BUILD_OS2.CMD` — OS/2 build script (Borland native path)
- `BUILD_OS2_OW.SH` — OS/2 build script (OpenWatcom cross-compile)
- `PCB154_BUILD_GUIDE.md` — Comprehensive build documentation
- `153_to_154.patch` — Full diff from flattened 15.3 to 15.4

### DOS Source Files
- `MD5IMPL.CPP` — RFC 1321 MD5 implementation
- `VMDATA.H` / `VMFUNCS.C` — Clean replacement for proprietary VMDATA.LIB
- `PCBTHUNK.ASM` — 27 JMP thunks bridging C++→pascal name mangling
- `SETUPTHK.ASM` — PCBSETUP screen globals
- `INT24STB.ASM` — DOS INT 23/24 handler stubs
- `STBSTUB.CPP` — Borland CRT streambuf stubs
- `REJECTS.CPP` / `REJECTS.HPP` — UUCP sender-blacklist filter
- `UUINSTUB.C` — UUCP log/queue stubs
- `UUOUTSTB.CPP` — UUOUT method stubs
- `WATCOMPAT.H` — Borland→OpenWatcom compatibility macros

### OS/2 Source Files
- `OS2STUBS.CPP` — Screen globals, keyboard stubs, usernet globals
- `OS2STUBS2.CPP` — Typed stubs with project.h for C++ name mangling
- `OS2STUBS_C.c` — C-linkage stubs (wcc386 naming convention)
- `OS2NAMES.asm` — Exact-name assembler stubs for Watcom/OS2 link
- `OS2GLOBALS.CPP` — C++ typed globals with PCBoard headers

### Rebuilt Libraries
Prebuilt `.LIB`/`.386` archives in `LIBS/PREBUILT/BC31/`, rebuilt from
Clark's library source with Borland C++ 3.1. These enable compilation
of both 15.3 and 15.4 without Clark's original build environment.

## CodeBase Library
Copyright (C) Sequiter, Inc.
Licensed under **GNU Lesser General Public License v3.0 (LGPL-3.0)**.

Released to open source by Sequiter, Inc. in September 2018.
Source obtained from: https://github.com/MPSystemsServices/CodeBase-for-DBF
Repository maintained by M-P Systems Services, Inc.

Full license text and signed open source agreement in `LIBS/CODEBASE/`.
