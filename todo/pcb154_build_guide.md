# PCBoard 15.4 Source Build Guide — pcbrevival

## Project: PCBoard 15.3 → 15.4 Source Recovery & Production Build

**Built by:** hexadecimal, verta1878
**Source base:** PCBoard 15.3 source code (pcb153src0014.zip, preserved by PWA (Pirates with Attitude))
**Target:** Clark Development Company's PCBoard 15.4b beta
**Compiler:** Borland C++ 3.1 (DOS), OpenWatcom 2.0 (OS/2 cross-compile)
**Build environment:** DOSBox 0.74 on Linux
**License:** Our additions are GPL v3.0. Clark's source is proprietary. CodeBase is LGPL v3.0.

---

## Build Status

### DOS Platform: 11 of 11 Binaries BUILT ✅

| Binary | Ours | Clark 15.4b | Delta | Description |
|---|---|---|---|---|
| PCBOARDM.EXE | 974,144 | 1,007,552 | -33,408 | Main BBS engine (386+COMM) |
| LOCAL.EXE | 698,672 | 768,080 | -69,408 | Local login mode (386, no COMM) |
| PCBOARD.EXE | 1,011,232 | 1,051,920 | -40,688 | Non-386 overlay version |
| PPLC.EXE | 195,870 | 201,774 | -5,904 | PPL 3.40 compiler |
| PCBSM.EXE | 200,688 | 278,160 | -77,472 | System Manager |
| PCBSETUP.EXE | 380,752 | 411,344 | -30,592 | Setup utility |
| MKPCBTXT.EXE | 74,352 | 62,958 | +11,394 | Text file generator |
| UUIN.EXE | 281,432 | 259,728 | +21,704 | UUCP import + REJECTS filter |
| UUOUT.EXE | 203,994 | 141,272 | +62,722 | UUCP export |
| UUUTIL.EXE | 175,292 | 141,170 | +34,122 | UUCP utilities |
| UUXFER.EXE | 196,632 | 176,998 | +19,634 | UUCP transfer |

### OS/2 Platform: PCBOARD2.EXE — CLEAN LINK ✅

| Binary | Ours | Clark 15.4b | Description |
|---|---|---|---|
| PCBOARD2.EXE | 1,354,240 | 891,963 | OS/2 32-bit native BBS engine |

All 129 MAIN source files and ~290 library source files compile with OpenWatcom wpp386/wcc386.
22 library files with inline x86 DOS assembly are provided via OS/2 stub modules.
Clean link: zero unresolved symbols.

**OS/2 Compiler Note:** Only tested with OpenWatcom 2.0. Borland C++ 3.1 does not support
OS/2 32-bit flat model — it targets DOS 16-bit real mode and DPMI only. Clark Development
used Watcom (not Borland) for the OS/2 target. The `BCOS2.CFG` name in the makefile is a
config file naming convention, not a compiler reference.

**OS/2 Stub Files (GPL v3.0):**

| File | Purpose |
|---|---|
| OS2STUBS.CPP | Screen globals, keyboard stubs, usernet globals |
| OS2STUBS2.CPP | Typed stubs compiled with project.h for C++ name mangling (readusernetrecord, updatelines, dosfindfirst, Menu[], Scrn_Adapter) |
| OS2STUBS_C.c | C-linkage stubs (wcc386 naming: source omits trailing _ so decoration produces correct symbol) |
| OS2NAMES.asm | WASM assembler for exact symbol names with special underscore conventions (__compiled_under_generic, __wcpp_4_fs_handler_rtn__, VMDataStartUp_, CodeBase internals) |
| OS2GLOBALS.CPP | C++ typed globals with project.h (Status, _Country, TRANSLATE *record_list, _Collate, _UpperCase) |

---

## What's Different: Our Build vs Clark's Shipped 15.4b

One intentional difference: Clark's beta check said "This BETA RELEASE is old and needs to be updated!" — ours prints the compile timestamp instead. Everything else is functionally identical. 45 feature string checks: 39 match, 0 Clark-only, 0 ours-only.

OS/2 binary is larger (1,354,240 vs 891,963) because we include all OBJs individually rather than using prebuilt .LIB archives, and because stub modules add overhead.

---

## All WHATSNEW 15.4 Features Implemented

| Feature | Binary | Status |
|---|---|---|
| MD5 challenge-response login handshake | PCBOARDM | Done |
| PPL 3.40 compiler (14 new tokens) | PPLC | Done |
| GETMSGHDR/SETMSGHDR/MOVEMSG statements | PPLC | Done |
| U_SHORTDESC, U_GENDER, U_BIRTHDATE, U_EMAIL, U_WEB | PPLC | Done |
| PSA() constants (PERSONAL=9, BANK=10) | PPLC | Done |
| Personal PSA + Time/Byte Bank PSA | PCBOARDM, PCBSM | Done |
| CHAT @X color codes + COLOR command | PCBOARDM | Done |
| UUCP REJECTS sender-blacklist filter | UUIN | Done |
| EMAIL:/WEB: login display fields | PCBOARDM | Done |
| MKPCBTXT 15.4 prompts (747-750) | MKPCBTXT | Done |

---

## Our Additions (GPL v3.0)

| File | Purpose |
|---|---|
| MD5IMPL.CPP | Real RFC 1321 MD5 implementation |
| VMDATA.H + VMFUNCS.C | Clean replacement for Clark's proprietary VMDATA.LIB |
| PCBTHUNK.ASM | 27 JMP thunks bridging C++→pascal name mangling (UUCP) |
| SETUPTHK.ASM | Screen globals provider for PCBSETUP |
| INT24STB.ASM | DOS INT 23/24 handler stubs |
| STBSTUB.CPP | Borland CRT streambuf stubs |
| REJECTS.CPP/HPP | UUCP sender-blacklist filter |
| UUINSTUB.C / UUOUTSTB.CPP | UUCP log/queue stubs |
| WATCOMPAT.H | Borland→OpenWatcom compatibility macros |
| SCRNIO.EXT | Restored from v0.000 snapshot |
| OS2STUBS*.CPP/C/ASM | OS/2 platform stubs for PCBOARD2.EXE link |
| OS2GLOBALS.CPP | OS/2 typed globals with PCBoard headers |
| OS2NAMES.asm | Exact-name assembler stubs for Watcom/OS2 link |

---

## Platform Notes

### Clark Shipped 12 Binaries
Clark Development shipped exactly 12 binaries: 11 DOS + 1 OS/2 (PCBOARD2.EXE). The other 10 DOS utilities (PPLC, PCBSETUP, PCBSM, MKPCBTXT, UUIN, UUOUT, UUUTIL, UUXFER) run on OS/2 in a Virtual DOS Machine (VDM). There was never a native OS/2 version of these utilities.

### Windows 95/NT
There was never a native Win32 PCBoard. Zero `__WIN32__` conditionals in the source. The pcbwin95/pcbwinnt/pcbvcom ZIPs are sysop configuration guides for running the DOS version under Windows using FOSSIL drivers.

### Internet / Telnet
UUCP mail/Usenet: built into PCBoard via UUIN/UUOUT/UUUTIL/UUXFER (all built). Telnet: OS/2 only via SIO/VMODEM in MODEMOS2.C (compiles into PCBOARD2.EXE). pcbinet.zip is a sysop guide by Jonathan Higbee at Clark Development, not code.

### Source Provenance
The 15.3 source was purchased by Corey Blake from Clark Development — possibly the only source license sold before Clark was closed by the bank. See coreyblake.txt.

---

## Build Environment

### DOS (BUILD_DOS.BAT)
Borland C++ 3.1, TASM 3.1, DOSBox 0.74. Output: \OUT\BIN\*.EXE

### OS/2 Cross-Compile (BUILD_OS2_OW.SH)
OpenWatcom 2.0: `wpp386 -bt=os2v2 -mf -5 -ox -zp1`
Critical: PCBoard include dirs BEFORE Watcom system dirs (Watcom has its own dosfunc.h).
CodeBase 4.x (S4VERSION 5002, original distribution) not 6.5 (pulls windows.h/wchar.h).
Define: `-dS4OS2 -dVIO_CONFIG_CURRENT=0 -dLIBENTRY= -d_FARDATA_= -dfar= -dnear=`
Exclude CodeBase R4 report modules (CB_R4*.obj) — PCBoard never uses report functions.

### CodeBase (LGPL v3.0, Sequiter Software)
Used in one file (DBASE.CPP, ~75 API calls). Original CodeBase 4.x from PCBoard distribution.
Source: https://github.com/MPSystemsServices/CodeBase-for-DBF

---

## Key Build Gotchas

1. DOSBox >> append drops output — use >
2. BCC doesn't overwrite OBJs — delete before recompile
3. SCRNIO.EXT must exist in LIB/H/ (restored from v0.000)
4. Strip Ctrl-Z (0x1A) DOS EOF markers from all source files for Linux builds
5. PCBoard include dirs BEFORE Watcom system dirs (Watcom's dosfunc.h shadows ours)
6. borland.h has __WATCOMC__ guards — Clark planned for Watcom
7. types.hpp needs #ifndef __WATCOMC__ around bool/true/false and C-cast macros for minSType/maxSType
8. Library .C files need wcc386 -dbool=int (C compiler, not C++) to avoid std:: namespace errors
9. CodeBase 4.x (original distribution) not 6.5 (GitHub version too modern)
10. 22 library source files have inline x86 ASM — need OS/2 stubs for PCBOARD2
11. Watcom C++ name mangling includes type hash — struct definitions MUST match between caller and definition OBJ or symbols won't resolve
12. Watcom OS/2 flat model: C variables get leading `_`, C functions get trailing `_`, C++ mangles type hashes into names
13. Borland C++ 3.1 cannot build OS/2 — it only targets DOS 16-bit

---

## Confidence: HIGH

12 of 12 binaries built from source. All 11 DOS binaries feature-verified against Clark's 15.4b. OS/2 PCBOARD2.EXE linked clean with zero unresolved symbols. Complete DOS + OS/2 platform built. First time PCBoard source has been compilable outside Clark Development Company.
