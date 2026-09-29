# HISTORY.md — PCBoard 15.4x / pcbirc release history

Version: v0.3.2+  
Build system: DOSBox-X headless, BC 3.1, TC 2.01, TASM  
Repo: https://github.com/verta1878/pcbirc

---

## v0.3.2+ — Build-root, recoveries, toolkit, OS/2 push

Entries 1–41 (2026-09-20 to 2026-09-24) — raw detail in git history.

Summary of work covered:
- BUILDROOT directory structure established
- BC 3.1, TC 2.01, TASM, MSC 7.0 toolchains staged
- DOSBox-X headless build environment created
- 10 toolkit category libs built (BC31 large model): COUNTRYL, DOSCLS_L,
  DOS_L, MISC_L, PCB_L, SCREEN_L, SCRNIO_L, SYSTEM_L, TOOLKITL, VMDATA_L
- 4 small-model toolkit libs built (BC31): COUNTRYS, MISC_S, SCREEN_S, SYSTEM_S
- pcbkit_l.lib (265,216 bytes) — master toolkit library
- PCBOARD.EXE (982,336 bytes) compiled and linked
- PPLC.EXE (196,156 bytes) compiled and linked
- USERNET.EXE (12,412 bytes) compiled and linked
- OS/2 SDK headers staged (332 IBM OS/2 2.1 headers)
- BC++ 2.0 for OS/2 added to BUILDROOT/BCOS2/
- VMDATA.H shim written (hexadecimal) — Clark VMData virtual-memory array API
- VMSeqFinalPass signature reconstructed from PCBSM SORT.C call sites
- 6 pre-built EXEs carried from distribution: FIDOUTIL, MAKEIDX, MKPCBTXT,
  PCBSETUP, PCBSM, COMMDRV

### v0.3.2+ (42) — 2026-09-25: wrench doc update

pcbcomm (renamed from pcbdcom) reassembly project documentation:
5 files delivered (BUILD-STATUS.md, README.md, ATTIC-CLEANUP.BAT,
todo/README.md, todo/SOURCE-RECOVERY.md).

### v0.3.2+ (43) — 2026-09-25: MAKE CLEAN

Stale library purge across the build tree.

### v0.3.2+ (44) — 2026-09-25: PCBIC v1.2 OS/2 source confirmed

Both OS/2 binaries byte-exact from NASM + OpenWatcom wlink.
10 OS/2 DLL imports. No OS/2 VM needed to build.

### v0.3.2+ (45) — 2026-09-25: CodeBase libraries identified as blockers

c4base.lib (DOS/BC31) and b4.lib (OS/2) missing — must compile from
pcbcbase/CODEBASE/SOURCE/. Without them PCBOARD.EXE has no dBASE and
PCBOARD2.EXE cannot link.

### v0.3.2+ (46) — 2026-09-27: UUCP 4/4 targets built (hexadecimal)

All 4 UUCP targets compiled and linked (BC 3.1, large model):

| Target | Size | OBJs | Notes |
|--------|------|------|-------|
| UUUTIL.EXE | 86,572 | 5 + 12 NO* stubs | |
| UUOUT.EXE | 131,486 | 9 + stubs | |
| UUIN.EXE | 206,648 | 16 + stubs + VMDATA.LIB | |
| UUXFER.EXE | 165,320 | 6 + FOSSIL.OBJ + stubs | |

Header fixes: VMDATA.H (VMDebugOn signature + debug flag defines),
pcbtools.h (__PCBMSG_ENUMS_DEFINED__ guard), MESSAGES.H (__PCBTOOLS_H
skip-all guard). STBSTUB.CPP combined shim written as workaround —
sysop0 to rebuild as 4 individual real implementations. FOSSIL.OBJ
placed from repo source. 10 new BC31 CFG/LNK build files created.

**BLDDOS step 8b complete — all 12 DOS targets accounted for.**

### v0.3.2+ (47) — 2026-09-27: STBSTUB replaced — 4 real streambuf implementations

sysop0 delivered 4 individual streambuf source files (GPLv3, clean-room
from BC 3.1 iostream.h).  All compiled and 3 UUCP targets relinked with
4 individual OBJs matching Clark's MAK layout.

| File | Function | OBJ size |
|------|----------|----------|
| stbdsgtn.cpp | do_sgetn (virtual, sbumpc loop) | 564 |
| stbdsptn.cpp | do_sputn (virtual, sputc loop) | 566 |
| stbsgetn.cpp | sgetn (memcpy fast-path) | 610 |
| stbsputn.cpp | sputn (memcpy fast-path) | 611 |

Relinked: UUIN 274,437 · UUOUT 196,499 · UUXFER 247,755.
STBSTUB blocker **RESOLVED**.

### v0.3.2+ (48) — 2026-09-28: PCBKit TC 2.01 — 4 monolithic toolkit libraries (hexadecimal)

119 modules × 4 memory models compiled with TCC 2.01 under DOSBox-X
headless. BLDKIT.BAT + MKLIB.BAT. SUBST workaround for turboc.cfg.

| Library | Size | Model |
|---------|------|-------|
| PCBKITS.LIB | 151,552 | small |
| PCBKITM.LIB | 156,672 | medium |
| PCBKITC.LIB | 163,840 | compact |
| PCBKITL.LIB | 168,960 | large |

dosboxx.zip updated with build output.

### v0.3.2+ (49) — 2026-09-28: case fix — lowercase entire dosboxx zip + build scripts (hexadecimal)

Lowercased all 3,383 files and 186 directories in dosboxx BUILDROOT.
Updated all path references inside build scripts to match across 14
directory roots + all subdirectory components. 16 dosboxx text files
and 38 repo build scripts updated. 4 PCBKIT*.LIB files and !PENTIUM.NFO
lowercased on share repo (binary content unchanged). dosboxx.zip rebuilt.

---

## Open blockers

- **CodeBase**: c4base.lib (DOS/BC31) + b4.lib (OS/2) missing.
  Upstream: https://github.com/MPSystemsServices/CodeBase-for-DBF
- **TKLIB**: TC201 monolithic done, TC201 split categories + MSC70 legs not done
- **SDK paths**: MAK/CFG need ROOT=\OUT\BRANCH\ across 4 branches × 3 compilers
- **turboc.cfg**: TCC 2.01 does not read config file under DOSBox-X — investigating
