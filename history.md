# HISTORY.md — PCBoard 15.4x / pcbirc release history

Version: v0.3.2+  
Build system: DOSBox-X headless, BC 3.1, TC 2.01, MSC 7.0, TASM  
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

### v0.3.2+ (50) — 2026-10-01: TC201 split categories — 10/10 passing (hexadecimal)

All 10 toolkit category libraries now compile under Turbo C 2.01:

| Category | OBJs | Library |
|----------|------|---------|
| COUNTRY | 3 | countryl.lib |
| DOS | 29 | dos_l.lib |
| DOSCLS | 1 | doscls_l.lib |
| MISC | 27 | misc_l.lib |
| PCB | 22 | pcb_l.lib |
| SCREEN | 39 | screen_l.lib |
| SCRNIO | 3 | scrnio_l.lib |
| SYSTEM | 11 | system_l.lib |
| TOOLKIT | 14 | toolkitl.lib |
| VMDATA | 2 | vmdata_l.lib |

Build: `bldtk tc201` (BLDTK.BAT with -Dtkcc=tcc -Dcver=tc201).
Output: `\out\pwa153\sdk\tc201\lib\`.

Bugs fixed along the way:
- Bug #8: TCC/TASM 50-char path buffer overflow — 3 category folder
  names shortened via conditional in TKLIB.MAK (country→cntry,
  screen→scren, toolkit→tkit; BC31 paths unchanged)
- Bug #9: TCC stricter type matching — 6 stub source files fixed
  (nodisp, noscreen, noshell, nomemory, notxt, scrollup -b conditional)
- Bug #10: 5 Clark source fixes for TCC C89 strictness (dosfind.c,
  crypt.c, dble_bs.c, kbdstat.c, delfiles.c)
- conffunc.c: TCC first-pass `asm` scanner bug — block-commented asm
  lines wrapped in per-line `/* */` (compatible with both TCC and BCC)
- dosclass.c: C port of dosclass.cpp (~600 lines) for TCC C-only
  compilation; dosclass.h CRLF + _MAX_PATH compat defines added

TKLIB.MAK changes (all 10 categories):
- `tkcc` macro replaces hard-coded `bcc` in inference rules
- `cfgflag` auto-derives: `+$(cfg)` for BCC, empty for TCC
- tc201/bin/turboc.cfg = TCC auto-read config (includes + defines)

### v0.3.2+ (51) — 2026-10-02: dosboxx.zip cleanup + cruft removal (hexadecimal)

Cleaned 5 stale root files + ~130 tmp/ files from dosboxx work dir.
Rebuilt dosboxx.zip as 3-part split (20MB z01 + 20MB z02 + 876KB zip).
6,199 files, 41MB total.

### v0.3.2+ (52) — 2026-10-02: fix -d → -D in Borland MAKE macro defines (hexadecimal)

Borland MAKE 3.6 uses uppercase `-D` to define macros on the command
line. Lowercase `-d` is a different flag (debug/date). Comment examples
in 10 TKLIB.MAK files and 4 live make commands in bldtks.bat were using
lowercase, which silently fell through to defaults.

- bldtks.bat: 4 live make commands fixed (lines 8, 13, 18, 23)
- 10 tklib.mak: comment examples fixed (lines 14, 15, 19; vmdata 27, 31)
- bldtks.bat added to repo root (was only inside dosboxx.zip)

### v0.3.2+ (53) — 2026-10-02: PCBKMS MSC 7.0 — small model first pass (hexadecimal)

Microsoft C 7.0 compilation of the 119-module PCBoard toolkit, 12th and
final leg of the SDK compiler matrix (PCBKIT=TC201, PCBKBC=BC31,
PCBKMS=MSC70).  CL.EXE hosted under DOSBox-X via HDPMI32 (Route A,
verified 2026-08-29).

Small model first pass: 100/119 compile clean, 19 PCBoard app module
failures (expected — app modules need MAIN headers not in SDK scope).

Source adaptations (build-time only, Clark's originals unchanged):

- types.hpp: removed illegal sizeof() in #if preprocessor; added
  _MSC_VER branch defining cdcSHORT="" cdcLONG=long (16-bit DOS:
  int=2, long=4); CDCCONST, EXTERN_C, bool/true/false guards
- asmrules.h: added MSC _M_I86LM/_M_I86CM/_M_I86MM to LDATA/LPROG
  checks (Borland __LARGE__/__COMPACT__/__MEDIUM__ absent in MSC7;
  fixed 5 country/misc modules in compact+large)
- exist.c: added #include <asmrules.h> + Watcom find_t DTA struct
  for MSC builds (LDATA undefined without it; fixed compact+large)
- inkey.c: added #include "exec.hpp" for PCB_MAXNODES (fixed all 4
  models)
- 9 source files received _MSC_VER guards for Borland pseudo-vars +
  DTA structs + geninterrupt:

| File | Fix |
|------|-----|
| ansi.c | borland.h include guard |
| conffunc.c | borland.h include guard |
| zswapvir.c | borland.h include guard |
| exist.c | DTA struct guard (MSC uses find_t) |
| julian.c | DTA struct guard |
| datafil2.c | DTA struct guard |
| dmath.c | _AX pseudo-register (Borland-only intrinsic) |
| scrollup.c | _DL pseudo-register |
| giveup.c | 3 fixes: _AX/geninterrupt, _AL/_BH/geninterrupt, _SI |

Build fixes:
- bldkms.bat written (72KB generated script, 119 modules × 4 memory
  models = 452 compile lines + TASM assembly)
- /DLIB added globally to %CC% — without it pcb.h selects wrong
  prototypes (void vs int) for getconfrecord/putconfrecord
- CL.EXE/TASM option prefixes converted / → - (DOSBox-X INT 21h
  converts / to \ at the filesystem level)
- -Ic:\pcb153\source\h added to all 452 compile lines

All changes committed to repo: 9 source files + bldkms.bat +
dosboxx.zip (rebuilt spanned) + sdk-build-status.md.

### v0.3.2+ (54) — 2026-10-30: PCBKMS MSC 7.0 — 476/476 ALL 4 MODELS DONE (hexadecimal)

**SDK compiler matrix: 12/12 legs COMPILATION COMPLETE.**

All 119 toolkit modules × 4 memory models (small/medium/compact/large)
compile clean under Microsoft C 7.0.  476 OBJs, zero errors.

This is the culmination of the SDK build effort.  Clark's original
12-library matrix — the complete toolkit compiled with BC++ 3.1, TC 2.01,
and MSC 7.0, each across small/medium/compact/large memory models —
required adapting Borland-native source to three different compilers' type
systems, preprocessors, and calling conventions.  With PCBKMS finishing
all four models at 119 modules each, the full 12-leg matrix compiles for
the first time since Clark Development closed.

| Model | Flag | OBJs |
|-------|------|------|
| small | -AS | 119 |
| medium | -AM | 119 |
| compact | -AC | 119 |
| large | -AL | 119 |

dosboxx.zip rebuilt as 2-part split (z01 29MB + zip 20MB).  z02 deleted.
.gitignore updated: toolkit OBJ tracking rules + lowercase path
equivalents.  hexadecimal-pcbircrepo-memory.zip updated.

**Remaining**: MKLIB → PCBKMS{S,M,C,L}.LIB (assemble OBJs into
libraries).

### v0.3.2+ (55) — 2026-11-01: pcbfoss placement + repo cleanup (hexadecimal)

pcbfoss Pascal FOSSIL driver source placed at pcb1541/pcbfoss/:

  pcbfoss.pas (13,085B) — main driver, 22/27 FSC-0015 functions
  pcbfoss_rings.pas (3,385B) — ring buffer implementation
  PCBFOSS_AUDIT.md (1,037B) — 6/6 audit items FIXED

Source extracted from cyclom-multiOS-source.zip (MEMORY/Claude outputs/).

Stale file cleanup — 3 files copied to attic/:
- lowercase-repo.log (1.5MB) — one-time case conversion log
- pcbis-wfc-mockup.html (7.4KB) — stale session artifact
- pcbfoss-1.0.zip (5.7KB) — was misplaced at repo root

### v0.3.2+ (56) — 2026-11-04: CodeBase c4base.lib rebuilt with authentic BCC (hexadecimal)

c4base.lib rebuilt using authentic BCC.EXE (BC 3.1) under DOSBox-X
headless, replacing the previous OpenWatcom WCC cross-compile.  99
CodeBase IV source files compiled and assembled with TLIB.

  pcbcbase/codebase/bor31/C4BASE.LIB   330,240 bytes   BCC large model

Build method: BCC.EXE (NE/BOSS format, DPMI16BI.OVL DPMI server)
runs natively under DOSBox-X with VCPI mode switching.  Earlier INT 6
crashes were caused by HDPMI32.EXE conflict, not BCC/BOSS
incompatibility — removing HDPMI32 from autoexec resolved it.

DOSBox-X config: machine=svga_s3, memsize=64, cycles=max,
zero unused int 68h=true.  No HDPMI32 in autoexec.

Build constraints discovered:
- `set PATH=...` in batch files does not work under DOSBox-X — full
  paths required (c:\bc31\bin\bcc)
- Batch files >~15 lines silently fail — split into 10-file batches
- TLIB one-OBJ-per-call (DOS 127-char command line limit)
- printf corrupts backslashes — use heredocs for batch generation

b4.lib (OS/2 target) kept as OpenWatcom build — BCC cannot compile
with -DS4OS2 because d4all.h pulls os2.h headers that BC 3.1 does
not have.

  pcbcbase/codebase/bor31/b4.lib   359,424 bytes   OW large model

**CodeBase blocker RESOLVED** — both libraries built and committed.

---

## Open blockers

- **PCBKMS MKLIB**: All 476 OBJs compile clean (119 × 4 models).
  Remaining: assemble into 4 PCBKMS{S,M,C,L}.LIB using MKLIB,
  then RSP file conversion for 127-char command-line cleanup.
- **SDK paths**: MAK/CFG need ROOT=\OUT\BRANCH\ across 4 branches × 3 compilers
