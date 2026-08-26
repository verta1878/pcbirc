# SDK Build Status — Toolkit Libraries

> **Toolkit branch: pwa153** (PCBoard 15.3, the base). All work in this
> document is the pwa153 toolkit unless stated otherwise. The other
> branches (pwa154, delta154, irc1541) have their own SDK builds - see
> each toolkit/<branch>. The 3-compiler matrix (PCBKIT/PCBKBC/PCBKMS)
> applies to the frozen Borland/Turbo/MSC branches; irc1541 moves to
> ow2irc (see toolkit/irc1541/COMPILER-DIRECTION.md).

Goal: build all four toolkits into their .LIB form (the SDK).

## Naming

| Branch | Compiler | Main lib | Stub lib |
|---|---|---|---|
| pwa153 | Borland | PCBTK_pwa153.LIB | PCBTKL_pwa153.LIB |
| pwa154 | Borland | PCBTK_pwa154.LIB | PCBTKL_pwa154.LIB |
| delta154 | Watcom | PCBTK_delta154.LIB | — |
| irc1541 | ow2irc | PCBTK_irc1541.LIB | — |

Output: `OUT/lib/<branch>/`

## Progress

### pwa153 — PARTIAL (PCBKBC, all 4 models) ⚠️  NOT COMPLETE
CORRECTED to match Clark's actual SDK structure:
- Naming: Clark's PCBKBC (Borland C++ 3.1), NOT invented PCBTK names
- 4 memory models: PCBKBC_S/M/C/L.LIB (113 main modules each)
- Loose override OBJs in obj/ (ALTMODEM, NO*, PCBDAT, SMALLERR) —
  shipped alongside, linked selectively (NOT a second library)
- PCBoard itself uses the MEDIUM model (PCBKBC_M)
- Output: OUT/lib/pwa153/
- INCOMPLETE: 113/118 modules vs Clark's PCBKIT_L.LIB; missing 96
  functions (INITDOOR, OPENMODEM, ASYNC_*, etc). Manifest came from
  only 8 makefiles' lib: targets, not all 286 toolkit sources.
  See OUT/lib/pwa153/STATUS.md.

Compiler families:
- PCBKBC (Borland C++ 3.1) — DONE, all 4 models
- PCBKIT (Turbo C 2.01) — compiler downloaded (devtools/TURBOC201.zip), build pending
- PCBKMS (Microsoft C 7.0) — compiler downloaded (devtools/MSC70.zip), build pending

### pwa154 — pending
### delta154 — pending (needs 22 Watcom fixes ported from irc1541 first)
### irc1541 — pending

## Build recipe (Borland branches)

1. Parse lib: targets from toolkit/<branch>/SOURCE/*/MAKEFILE
2. Compile each object: BCC +config <file>.C (config has -c -P -ml
   -DPCB152 -DLIB -DCOMM + include paths + -n<objdir>)
3. C++ files (.CPP) compile the same way; ASM files use
   TASM /mx /d__l__ <file>.ASM,<objdir>\<name>.OBJ
4. Separate override stubs (ALTMODEM, NODISP, PCBDAT) from main objects
5. TLIB <mainlib> @main.rsp ; TLIB <stublib> @stub.rsp
6. Key gotchas: use BCC +config (long command lines truncate object
   names in DOSBox); output .OBJ is uppercase; response files use
   ' &' continuation.

## Step 2 — PCBKIT (Turbo C 2.01): blocker found

Turbo C 2.01 is installed and working (TCC.EXE compiles). But our
toolkit HEADERS have modern drift that Turbo C 2.0 cannot parse:

- C++ `//` comments (9 in TYPES.HPP, 4 in PCBTOOLS.H, 6 in SCREEN.H)
- `__BORLANDC__ < 0x500` guards referencing Borland C 5.0 (~1996)

These postdate the Turbo C 2.0 era — the source was modernized over
the years (passed through Borland C++ 3.1 and later). Turbo C 2.0 is a
pre-ANSI C compiler (only /* */ comments, no C++).

Confirmed: Clark's PCBKIT_L.LIB is almost all plain C (341 C symbols,
4 C++), so PCBKIT WAS a Turbo C 2.0 C-mode build.

### Decision needed
- PATH A: make headers Turbo-C-compatible (convert //, guard modern
  bits). Small/mechanical but touches SHARED headers — must not break
  the working PCBKBC build. Would need a TC-specific header set or
  careful guards.
- PATH B: treat PCBKBC (Borland C++ 3.1) as the definitive build from
  our source; PCBKIT best-effort or deferred until period-correct
  Turbo-C-era headers are found.

PCBKMS (Microsoft C 7.0) likely hits the same header-modernization
issue.

Recommendation: PATH A with a TC-specific header include dir (leave the
shared headers untouched, provide TC-compatible copies on TCC's include
path) — gets working Turbo C libs without risking PCBKBC.

## Step 2 PATH A — progress (VIRTUAL.C-style header guards)

Approach confirmed working: guard C++-only header constructs with
`#if defined(__cplusplus)` so ONE header set compiles under both
Borland C++ 3.1 (PCBKBC) and Turbo C 2.0 (PCBKIT) — the same
one-file-two-modes pattern as VIRTUAL.C. No separate header copies.

Done:
- TYPES.HPP: guarded the file-scope `const` block (minInt..maxULong,
  all unused in C) and the `bool` typedef under __cplusplus. Converted
  the // comments in that block. Backup: TYPES.HPP.orig.
- VERIFIED PCBKBC (C++) still compiles clean with the guarded header —
  the working build is NOT broken (tested INIT.C, COMMA.C, ANSI.C).
- TCC now parses past the old line-129 break.

Remaining for TCC (Turbo C 2.0):
- TYPES.HPP line-1 "conditional started on line 0": TCC 2.0 preprocessor
  quirk with nested #if inside #if defined(__cplusplus), likely because
  __BORLANDC__ is undefined under TCC. Needs a small adjustment (guard
  the __BORLANDC__ checks so they only apply when __BORLANDC__ is
  defined).
- Once TYPES.HPP is clean, LIBENTRY resolves (its #define is in
  TYPES.HPP:90-92 — the cascade of misc.h LIBENTRY errors was just
  fallout from types.hpp failing early).
- Then sweep remaining // comments in PCBTOOLS.H (4), SCREEN.H (6).

Method proven; remaining work is mechanical per-header guarding. PCBKBC
stays intact throughout (guards are C++-side no-ops).

## Step 2 PATH A — checkpoint (2026-08-25 continued)

TYPES.HPP guarding progress:
- Added __TURBOC__ branches for the sizeof()-in-#if blocks (lines 48-69)
  that TCC 2.0's preprocessor can't evaluate. VIRTUAL.C-by-compiler
  pattern.
- Guarded file-scope const block + bool typedef under __cplusplus.
- PCBKBC (Borland C++) RE-VERIFIED intact after every change — the
  working build is never broken.

Still open: TCC reports "types.hpp 1: Unexpected end of file in
conditional started on line 0" even though all #if/#endif pairs balance
(verified: include guard 33/35/265, all inner blocks matched). This is
a TCC-2.0 preprocessor quirk not yet pinned down — likely an #if
expression form TCC parses differently. Needs focused isolation
(bisect the header by #if-ing out halves) rather than more guard edits.

Approach remains correct (one header, compiler-guarded, PCBKBC safe).
The remaining blocker is a single TCC preprocessor parse issue in
TYPES.HPP, isolated to that one file. TYPES.HPP.orig preserved.

## Step 2 PATH A — TCC blocker CRACKED (2026-08-25)

Root causes found and fixed:

1. **CRLF line endings (the "line 0" phantom error).** My Python edits
   had silently converted TYPES.HPP to LF-only. Turbo C 2.0 requires
   DOS CRLF; with LF it reported "Unexpected end of file in conditional
   started on line 0". Restoring CRLF fixed it. TYPES.HPP now parses
   under TCC. (Lesson: always write headers/source as CRLF for the DOS
   compilers.)

2. **bool undefined in C mode.** The bool typedef was guarded under
   __cplusplus, but MISC.H uses `bool` in C prototypes. Fixed: bool is
   now typedef'd for C mode too (Turbo C has no built-in bool).

3. **__TURBOC__ branches** for the sizeof()-in-#if blocks TCC can't
   evaluate (16-bit DOS: int=2/long=4, exact).

Verified:
- TYPES.HPP compiles under TCC ✓
- COMMA.C compiles under Turbo C 2.01 -> COMMA.OBJ ✓
- PCBKBC (Borland C++) RE-VERIFIED intact after every change (INIT,
  COMMA, PADSTR all still build) ✓

## Remaining for the full PCBKIT build (mechanical)

1. **`//` comments in ~95 .C source files** break TCC (it doesn't know
   `//`). Tested: indented `#` directives are FINE — only `//` is the
   problem. Fix: convert `//` -> `/* */`.
   - Approach: do NOT hand-edit the shared source (risks PCBKBC). Either
     (a) a build-time preprocessing pass that converts // for a TCC-only
     source copy, or (b) convert in place carefully and re-verify PCBKBC
     after (// -> /* */ is C++-safe, so (b) is acceptable if verified).
2. **Inline-asm files need TASM on PATH.** Some .C files (e.g. PADSTR.C
   line 104) invoke inline asm -> TCC calls tasm.exe. We have BC31's
   TASM; add it to the PATH for TCC builds.

The pipeline is proven end to end (a real .OBJ built with Turbo C). The
rest is the // sweep + TASM-on-path, then run the full 119-module
manifest through TCC for all 4 models.

## Step 2 — PCBKIT COMPLETE ✅ (2026-08-25)

All 4 PCBKIT (Turbo C 2.01) libraries built, verified (119 modules,
key door functions present), and installed to OUT/lib/pwa153/. PCBKBC
rebuilt with the shared updated headers — identical sizes, confirming
the compiler guards are behavior-preserving.

The nullHandle clash (last blocker) was fixed with the CDCCONST macro:
file-scope `const` has internal linkage in C++ but EXTERNAL in C, so
every module including types.hpp exported nullHandle and clashed. CDCCONST
= `const` in C++, `static const` in C. Same class of fix as ansicolors.

Turbo C 2.01 added to DOSBOXX.ZIP alongside BC31.

SDK matrix: 8 of 12 (PCBKBC 4/4, PCBKIT 4/4). Next: PCBKMS (MSC 7.0).

## Build scripts + distribution (2026-08-25)

Added runnable, echo-on build scripts so a user can build the SDK
inside DOSBox and watch each step:

  MAIN/build/scripts/
    BLDMENU.BAT  - CHOICE-based menu (pick KBC/KIT/KMS/ALL)
    BLDKBC.BAT   - PCBKBC (Borland C++ 3.1), 119 mods x 4 models
    BLDKIT.BAT   - PCBKIT (Turbo C 2.01), 119 mods x 4 models
    BLDKMS.BAT   - PCBKMS placeholder (until MSC 7.0 done)
    MKLIB.BAT    - assembles OBJ -> LIB (BC31 TLIB)
    *.RSP        - TLIB response files (per model, per compiler)
    README.md

Generated from the 119-module manifest, so they match the verified
build exactly. BLDKIT.BAT was TEST-RUN end to end: it compiled all
119 modules x 4 models and assembled the 4 PCBKIT libs. Scripts echo
"[ n/119] MODULE" progress per module per model.

Distribution:
  - Scripts live in MAIN/build/scripts/ (repo source of truth) AND are
    mirrored into DOSBOXX.ZIP under BUILD/SCRIPTS.
  - DOSBOX.CFG autoexec updated: uses CHOICE to offer "Launch the
    build menu now? [Y/N]" and always prints the manual commands.
  - Standalone compiler archives (parallel set):
      PCB153BT.ZIP  = Borland C++ 3.1 (existing)
      TC201BT.ZIP   = Turbo C 2.01 (NEW) - BIN/INCLUDE/LIB/README
    (MSC70BT.ZIP to follow when PCBKMS is built)

Also cleaned stray build scratch from the repo root (empty TCC/TCS/
TCM/TCL dirs and OC/OL/OM/OS/TCOBJ obj dirs) - real objects are
preserved in OUT/lib/pwa153/pcbkit-obj + pcbkbc-obj.

## Step 2 — PCBKMS (Microsoft C 7.0): toolchain extracted, DPMI blocker

Progress:
- MSC70.zip = 12 install-disk zips (MSC7D01-12), files KWAJ-compressed
  with trailing-$ names (Microsoft C/C++ 7.0 Beta 3).
- Used the disks' own DECOMP.EXE (in DOSBox) to decompress the toolchain:
  CL.EXE, C13216/C23216/C33216 (C compiler passes), C1XX3216 (C++ front
  end), LINK.EXE, LIB.EXE, plus MS32KRNL.DLL, MSDPMI.EXE, MS32EM87.DLL.
  Decompressed 44 headers + 68 runtime libs (SLIBCR/MLIBCR/CLIBCR/
  LLIBCR = the C runtimes, small/medium/compact/large).
- Assembled a clean install tree at MSC70/ (BIN/INCLUDE/LIB).

BLOCKER: the MSC 7.0 compiler binaries are 32-bit DOS-extended (all
"3216" = 32-bit host). Running CL/C1 gives:
  R6901 - DOSX32 : DPMI host required
The compiler's DOSX32 extender needs a DPMI host + MS32KRNL.DLL. MSC 7.0
ships MSDPMI.EXE for this, but MSDPMI.INI is a Windows-3.x 386-enhanced
config (*vddvga, *vpicd VxDs) - it's a Windows-derived DPMI host, not a
plain DOS one. Plain DOSBox doesn't satisfy DOSX32's DPMI probe.

Paths forward (next session):
1. DOSBox-X DPMI: find the right config knob so DOSBox-X presents a
   DPMI host DOSX32 accepts (DOSBox-X has more DPMI support than stock).
2. Load a standalone DPMI host (e.g. CWSDPMI) before CL - if DOSX32
   accepts a generic DPMI 0.9 host.
3. Run under Windows 3.x in DOSBox (heavy, but MSDPMI's native env).
4. Use a real-mode MSC (6.0/7.0 non-beta had 16-bit compilers) if the
   goal is just "an MSC-family PCBKMS" rather than 7.0 specifically.

The 119-module manifest, obj dirs (obj/msc70/), and response files
(MS??.RSP) are already scaffolded, so once the compiler runs, PCBKMS
builds the same way as KBC/KIT. The shared headers are already guarded
for C mode, so few source issues are expected.

## PCBKMS DPMI requirement — CONFIRMED from Microsoft's own README (2026-08-25)

User supplied retail MSC 7.0 (8-18-1992) + update patches. Retail
README.TXT states definitively:

  "Microsoft C/C++ version 7.0 requires DPMI services. If you wish to
   use Windows as your development environment, Windows provides DPMI
   services for you. To use MS-DOS as your development environment you
   must install 386-Max to provide these services."

So the 32-bit compiler (C13216/C23216/C33216 via CL) MANDATES a DPMI
host - either Windows 3.x or the 386-Max memory manager. This is
inherent to the product, not a beta limitation (retail behaves the
same: R6901 DOSX32 DPMI host required).

Both Beta 3 and retail toolchains are now extracted. Retail is the
release version (LINK 5.31, dated 1992). Update patches (C7pat/C7patb)
fix LINK/LIB/PWB/CV - not the compiler DPMI need.

To build PCBKMS headless, we must give DOSBox a DPMI host:
  1. 386-Max (period-correct, what MS recommends for DOS) - need the
     386-Max product.
  2. A generic DPMI host (CWSDPMI/HDPMI) IF DOSX32 accepts it - DOSX32
     is picky (probes for specific DPMI), so this may not work.
  3. Windows 3.x installed in DOSBox - MSDPMI's native env, heaviest.
  4. Run on real hardware / a Win3.1 VM (user has the working software).

Everything else for PCBKMS is ready: manifest, obj/msc70/ dirs, MS??.RSP
response files, C-mode-guarded headers. Compiler-run is the only gap.

## PCBKMS — real-DOS paths clarified (user research, 2026-08-25)

Key facts (from WinWorld / helparchive / malsmith.net):
- MSC/C++ 7.x tools split memory handling:
  * LINK.EXE, BSCMAKE, CV (CodeView) ship a 16-bit DOS extender that
    works with DPMI, VCPI, OR XMS - so the LINKER/librarian side is
    flexible and runs on plain DOS with HIMEM/EMM386.
  * CL.EXE (the compiler driver -> C13216/C23216/C33216) uses a 32-bit
    DOS extender that ONLY accepts DPMI. This is the hard requirement.
- MS's own README: DOS host needs 386-Max (or run under Windows) to
  provide DPMI. HIMEM/EMM386 give XMS/VCPI but not DPMI, so they satisfy
  LINK but NOT CL.

The malsmith.net C7-OS2 add-on (C7OS2.ZIP, widely on Hobbes):
- A NATIVELY-HOSTED 16-bit OS/2 compiler for C/C++ 7.0. Runs in real
  16-bit mode - NO 32-bit DOS extender, NO DPMI.
- Can be driven with Visual C++ 1.5's 16-bit headers/libs alone (per
  malsmith), independent of the DPMI-bound base DOS product.
- Path: build PCBKMS as a 16-bit OS/2-hosted compile using the C7 OS/2
  add-on. This sidesteps DPMI entirely. (We already have OS/2 in the
  picture via OS2TK + the OS/2 build targets.)

So there are now THREE viable PCBKMS routes:
  A. DOS + 386-Max (period-correct DPMI host) under DOSBox.
  B. DOS + a DPMI host DOSBox-X can present (needs the right config).
  C. OS/2-hosted 16-bit compile via the C7 OS/2 add-on (C7OS2.ZIP) +
     VC++ 1.5 16-bit headers/libs - NO DPMI. Cleanest for real-mode.

Route C aligns with the project already carrying OS/2 targets. Worth
pursuing: grab C7OS2.ZIP, pair with VC++ 1.5 16-bit H/LIB, host under
OS/2 (or OS/2 emulation), build the 119-module manifest -> PCBKMS.

Both the retail MSC 7.0 (8-18-1992) and Beta 3 base toolchains are
extracted and on hand; the base product's CL is the DPMI-bound one.

## PCBKMS — OS/2 Hosted Add-on acquired (2026-08-25) — DPMI blocker CLEARED

The MS C/C++ 7.0 OS/2 Hosted Add-on Kit (devtools/C7OS2.zip, 2 disks)
is now in the repo and decompressed into MSC70/OS2/. This is the key
that removes the DPMI wall:

- OS/2-hosted compiler passes: C11616 / C21616 / C31616 (C) + C1XX1616
  (C++) - the 16-bit-hosted, 16-bit-target equivalents of the DOS
  3216 passes. They run natively under OS/2 in REAL 16-bit mode. NO
  DPMI, no 32-bit DOS extender.
- Also: CL (OS/2 driver), LINK, LIB, NMAKE, PWB, BSCMAKE, CVPACK in
  OS2/BINP; bound dual-mode (DOS+OS/2) utilities in OS2/BINB (IMPLIB,
  MAPSYM, RC, RCPP, WINSTUB); MSHELP.DLL in OS2/DLL.
- Per README.OS2, the add-on ships compiler+utilities ONLY and uses the
  base product's headers/libraries - which we already have in
  MSC70/INCLUDE and MSC70/LIB. So MSC70/OS2/BINP + MSC70/INCLUDE +
  MSC70/LIB is a complete 16-bit OS/2 compile environment. (VC++ 1.5's
  H/LIB, mentioned earlier as an alternative, are NOT required - the
  base product's suffice.)

Remaining to build PCBKMS: an OS/2 host (or OS/2 emulation) to run the
OS2/BINP compiler, then compile the 119-module manifest. The project
already carries OS/2 targets (OS2TK/, BUILD_OS2 scripts), so this fits
the existing OS/2 workflow. Everything else (manifest, obj/msc70/ dirs,
MS??.RSP response files, C-mode-guarded headers) is ready.

Note: the OS/2 add-on's README states it targets DOS/Windows apps (it
was sold to OS/2-hosted devs building DOS/Windows software). For our
purposes we only need it to HOST the compiler in 16-bit mode to produce
the PCBKMS .LIB - which is exactly what it does.
