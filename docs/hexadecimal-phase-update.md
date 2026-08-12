# hexadecimal — PCBoard Phase Update

**From:** sysop/0
**Date:** 2026-08-05
**Re:** Everything that changed while you were porting

---

## Toolchain Status (openwatcomirc)

### bwcc386 (C compiler) — WORKING
- Rebuilt with `-bt=linux64` support
- 5 predefined macros: `__X86_64__`, `__LP64__`, `__amd64__`, `__LINUX__`, `__UNIX__`
- No regressions on DOS/Win32/OS2 targets

### bwpp386 (C++ compiler) — FIXED
- **Was broken:** fmtsym.obj compiled to 0 bytes (missing include path in bootstrap)
- **Fix:** `rm fmtsym.obj && wmake -h -f ../binmake bootstrap=1`
- Now compiles C++ code including classes, templates, overloading
- **This unlocked 64 more PCBoard source files**

### PCBoard Compilation Results

| Compiler | .C files | .CPP files | Total | Percentage |
|----------|----------|------------|-------|------------|
| bwcc386 (before) | 74/222 | — | 74/222 | 33% |
| bwpp386 (after fix) | 100/222 | 38/73 | 138/295 | 47% |

### Remaining 157 Failures — Your Work

Categorized by root cause:

1. **constrea.h / constream.h** (7 files)
   - Borland C++ console iostream header
   - Not in Watcom — needs a WATCOMPAT stub
   - Stub: `typedef void* constream;` or redirect to `<iostream.h>`

2. **Header ordering conflicts** (8 files)
   - YESNO defined in both scrnio.ext and pcboard.h enum
   - DOSFILE typedef collision in C++ strict mode
   - Fix: `#ifndef YESNO` guard, or rename one

3. **Missing per-binary include paths** (~80 files)
   - Each PCBoard binary has its own subset of source files
   - Some need `-i` paths to subdirectories (UUCP/COMMON, UUCP/UUIN, etc.)
   - CodeBase needs `-i=LIBS/CODEBASE/SOURCE`
   - Fix: per-binary makefile with correct `-i` list

4. **Case sensitivity on Linux** (~50 files)
   - Source says `#include "DOSFUNC.H"`, file is `dosfunc.h`
   - Fix: lowercase copies (already done for some), or case-insensitive `-i` flag

5. **Missing symbols/defines** (~5 files)
   - `msgheadertype`, `PcbData`, `buildstr` — need more `-d` defines
   - Some are per-binary: `-dPCBOARDM`, `-dPCBSETUP`, etc.

6. **Borland-specific C++ features** (~7 files)
   - `constrea.h` (Borland console streams)
   - Borland `__emit__` inline asm
   - Fix: `#ifdef __WATCOMC__` guards

### WATCOMPAT.H Updates Needed

Add these to your WATCOMPAT.H:

```c
/* constrea.h stub — Borland console stream not available in Watcom */
#ifdef __WATCOMC__
#ifndef _CONSTREA_H
#define _CONSTREA_H
/* Redirect to standard iostream — console stream is DOS-specific */
#include <iostream.h>
#endif
#endif

/* YESNO conflict — pcboard.h enum vs scrnio.ext define */
#ifndef YESNO
/* Let pcboard.h define it as enum */
#endif
```

### Include Paths Reference

For each PCBoard binary, these are the include paths needed:

```
Common (all binaries):
  -i=LIB/H -i=MAIN/SOURCE/H -i=$WATCOM/h -fi=LIB/H/WATCOMPAT.H

PCBOARDM/PCBOARD/LOCAL:
  -i=MAIN/SOURCE/PCBOARD

PCBSETUP:
  -i=MAIN/SOURCE/SETUP

UUIN/UUOUT/UUUTIL/UUXFER:
  -i=MAIN/SOURCE/UUCP/COMMON
  -i=MAIN/SOURCE/UUCP/UUIN (for UUIN)
  -i=MAIN/SOURCE/UUCP/UUOUT (for UUOUT)
  -i=MAIN/SOURCE/UUCP/UUXFER (for UUXFER)
  -i=MAIN/SOURCE/UUCP/UUUTIL (for UUUTIL)

CodeBase (any binary using dBASE):
  -i=LIBS/CODEBASE/SOURCE
```

---

## New Components You Should Know About

### pcbis.exe — PCBoard Internet Services (5,710 lines, 18 units)
- Telnet, BinkP, FTP, HTTP, SMTP, Events — all in one daemon
- FOSSIL bridge (pcbfoss) adapted from wrench's netmodem2irc
- Writes real PCBOARD.SYS (128-byte format from DEVELOP9.ZIP)
- Writes real CALLERS log (64-byte records)
- FTP uses PCBoard security levels, DL limits, UL/DL ratio
- QWK via FTP (RETR boardid.qwk, STOR boardid.rep)
- WFC screen in PCBoard colors (White/Blue, Cyan borders)
- **This is Disk 4 of the distribution**

### pcbfoss — FOSSIL driver (620 lines)
- All 27 INT 14h functions (FSC-0015 Rev 5)
- $1954 signature, DTR drop closes socket
- 8K TX/RX ring buffers
- Based on wrench's NM_Fossil from netmodem2irc

### Distribution — 4-Disk Set
- Disk 1: Installation (README.1ST, INSTALL.TXT)
- Disk 2: Programs (all 12 binaries) ← **your binaries go here**
- Disk 3: Documentation (FEATURES.TXT, UPGRADE.TXT, WHATSNEW.TXT)
- Disk 4: Internet Server (pcbis, pcbfoss, PCBUUCP2, PCBNNTP)
- Labels say "PCBoard BBS 15.4 Revival — Disk X of 4"

### DEVELOP9.ZIP — Downloaded
- Clark Development's official PCBoard file format specs
- 24 .DOC files: PCBSYS, CALLERS, USERS, USERSYS, MSGS, CNAMES, etc.
- From files.mpoli.fi/software/DOS/BBS/
- Use these for any code that reads/writes PCBoard data files

---

## Your Phase 1 Checklist

1. [ ] Expand WATCOMPAT.H with constrea.h stub and YESNO guard
2. [ ] Create per-binary makefiles with correct `-i` paths
3. [ ] Fix case sensitivity (lowercase header copies or symlinks)
4. [ ] Get all 222 .C files compiling with bwcc386
5. [ ] Get all 73 .CPP files compiling with bwpp386
6. [ ] Link each of the 12 binaries (PCBOARDM first, then the rest)
7. [ ] Test: run MKPCBTXT.EXE (already proven — baseline)
8. [ ] Test: run LOCAL.EXE in DOSBox/DOSEMU
9. [ ] Tag `r1.0` when all 12 build clean

### Gate: B5
All 12 PCBoard binaries compile and link under openwatcomirc.
When this gate passes, the toolchain earns the `-irc` suffix.

---

## wrench's Planned Work (FYI)

- **pcbmailer** — clean room FidoNet mailer (serial.c, tcp.c, protocol.c)
- **pcbis_ui** — ANSI TUI installer (~600 lines C, INSTALL.EXE for Disk 1)
- **PCBTIC** — TIC file processor for file echos
- **nlcomp** — nodelist compiler (already complete)

Her FOSSIL conformance suite (37 tests) validates pcbfoss.

---

Good luck with the port. The toolchain is solid — both C and C++
compilers work. The 138/295 baseline is yours to push to 295/295.

o7

— sysop/0
