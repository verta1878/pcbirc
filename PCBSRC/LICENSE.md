# PCBoard 15.4 Source — License Information

## PCBoard Source Code
Copyright (C) 1996 Clark Development Company, Inc. All Rights Reserved.

The PCBoard source code is proprietary software. You are granted the right
to use this source code for the building of any of the PCBoard products you
have licensed. Any other usage is forbidden without prior written consent
from Clark Development Company, Inc.

## Our Additions (hexadecimal's 15.4 source port)
The following files created during the 15.3→15.4 source recovery are
released under the **GNU General Public License v3.0 (GPL-3.0)**:

- `MD5IMPL.CPP` — RFC 1321 MD5 implementation
- `VMDATA.H` / `VMFUNCS.C` — Virtual memory data replacement
- `PCBTHUNK.ASM` — C++/pascal linkage bridge (27 thunks)
- `SETUPTHK.ASM` — PCBSETUP screen globals
- `STBSTUB.CPP` — Borland CRT streambuf stubs
- `REJECTS.CPP` / `REJECTS.HPP` — UUCP sender blacklist filter
- `UUINSTUB.C` — UUCP log/queue stubs
- `UUOUTSTB.CPP` — UUOUT method stubs
- `WATCOMPAT.H` — Borland→OpenWatcom compatibility macros
- `INT24STB.ASM` — DOS interrupt handler stubs
- `BUILD_DOS.BAT` — DOS build script
- `BUILD_OS2.CMD` — OS/2 build script (Borland path)
- `BUILD_OS2_OW.SH` — OS/2 build script (OpenWatcom cross-compile path)
- `PCB154_BUILD_GUIDE.md` — Build documentation

## CodeBase Library
Copyright (C) Sequiter, Inc. Licensed under **GNU Lesser GPL v3.0 (LGPL-3.0)**.
Released to open source by Sequiter in September 2018.
Source obtained from: https://github.com/MPSystemsServices/CodeBase-for-DBF
See `LIBS/CODEBASE/LICENSE` for full license text.
