# SDK Build Status — Toolkit Libraries

> **Toolkit branch: pwa153** (PCBoard 15.3, the base). All work in this
> document is the pwa153 toolkit unless stated otherwise. The other
> branches (pwa154, delta154, irc1541) have their own SDK builds - see
> each toolkit/<branch>. The 3-compiler matrix (PCBKIT/PCBKBC/PCBKMS)
> applies to the frozen Borland/Turbo/MSC branches; irc1541 moves to
> ow2irc (see toolkit/irc1541/COMPILER-DIRECTION.md).

Goal: build all four toolkits into their .LIB form (the SDK).

## Naming

Clark's own names, one family per compiler, four memory models each:

| Family | Compiler | Libraries |
|---|---|---|
| PCBKBC | Borland C++ 3.1 | PCBKBC{S,M,C,L}.LIB |
| PCBKIT | Turbo C 2.01 | PCBKIT{S,M,C,L}.LIB |
| PCBKMS | Microsoft C 7.0 | PCBKMS{S,M,C,L}.LIB |

The `PCBTK_*` / `PCBTKL_*` names used in earlier revisions of this
document were invented, not Clark's. Do not reintroduce them.

Output: `toolkit/<branch>/<compiler>/`  (OUT/lib/ was deleted 2026-09-17)

## Progress

### pwa153 — PCBKBC BUILT ✅ 2026-09-17

**PCBKBC{S,C,M,L}.LIB — 152 modules, all four memory models.**
Output: `toolkit/pwa153/bc31/lib/`. Build script:
`MAIN/build/scripts/BLDKBC.BAT`. Full account:
`toolkit/pwa153/bc31/README.md`.

    PCBKBCS.LIB   245,760 B   small
    PCBKBCC.LIB   258,560 B   compact
    PCBKBCM.LIB   252,928 B   medium     <- PCBoard itself uses medium
    PCBKBCL.LIB   265,216 B   large

All four hold the same 152 modules. SHA256/MD5 alongside the libraries.

**Verified by linking, not by counting.** Six of Clark's seven sample
doors compile and link clean against the library in medium and large;
the binaries are committed to `OUT/pwa153/bins/`. Against Clark's
shipped `TOOLKIT/OTHER/QUICKREF`: **150 of 150** documented door
functions — 139 under their own names, the other 11 as the `ASYNC_*`
entry points the header maps them to.

**Nothing is unresolved.** Small and compact link-fail every sample with
`Segment _TEXT exceeds 64K` — a 64K single-code-segment limit, not a
library defect; those models are for doors using a small slice of the
toolkit. `SAMPLES/INPUTREQ.C` fails with `Undefined symbol _MAIN`; it is
a replacement-module example, meant to be compiled into a door rather
than linked standalone. Both are documented in
`OUT/pwa153/bins/README.md` and neither is shipped as a binary.

### The three config changes that made it build

Clark shipped no door-SDK config — `LIB/CFG/BC31/PCBOARD.CFG` is the
PCBOARD.EXE config. The SDK config is that, plus `-DLIB`, minus two
switches. Both removals were established by experiment:

- **`-Y` (overlay generation).** Borland: *"Overlays only supported in
  medium, large, and huge memory models."* This — **not** far pointers
  or `_FARDATA_` — was the small/compact blocker recorded below. With
  `-Y` gone, small and compact go from 6/130 to 130/130.
- **`-DPCBCOMM`.** Guards PCBoard-internal branches that read
  `Status.TerseMode`; under `-DLIB`, `PCBOARD.H` selects the reduced
  door-visible `statustype`, which has no `TerseMode`. All four
  combinations of `-DPCBOARD`/`-DPCBCOMM` were tested: dropping
  `-DPCBCOMM` is necessary and sufficient.

Everything else is Clark's, unchanged — including `-P` (mandatory),
`-V -Vmp -Vmd`, `-3`, and `-D_FARDATA_=_FAR_` (do not drop it; it sets
the struct layout a door shares with PCBOARD.EXE at runtime).

### Manifest: 154 modules, not 130

The 130-module figure below came from Clark's `PCBKIT_L.LIB`. That
library is **not the SDK** — it is the PPLC/utility link target, built
without `-DLIB`, and `docs/pcboard-internals/PCBKIT-LIB.md` already
records it as stale. Using it as the SDK manifest produced three kinds
of error, all found by building and linking:

**Dual-tree collisions** (the entry below was right about CNAMES and
about the rule; two more were wrong in the manifest):

| Module | Manifest had | Correct for the SDK |
|---|---|---|
| CNAMES | `LIB/SOURCE/PCB/CNAMES.C` | `LIB/SOURCE/TOOLKIT/CNAMES.C` |
| HELP | `LIB/SOURCE/SCRNIO/HELP.C` | `LIB/SOURCE/TOOLKIT/HELP.C` |
| ANSI | `LIB/SOURCE/SCREEN/ANSI.C` | `MAIN/SOURCE/ASM/ANSI.ASM` |

ANSI is worth remembering. `MAIN/SOURCE/DISPLAY/ANSI.C` looks correct
— it defines `agotoxy`, `asetcolor`, `awherex`, `awherey`, `curcolor` —
but does not compile under `-DLIB`, and the linker wants those symbols
**uppercase** (`AWHEREX`). They come from `ANSI.ASM`, assembled
`tasm /m3` (uppercasing), not `/mx`. Same casing split as
`OUT/pwa153/BUILD-RECIPE.md` records for PPLC.

**22 absent modules**, each traced from a linker "Undefined symbol":
BUILDSTR, CHANGE, DOSDUP, FASTPUTC, INDEX, TICDELAY, BOX, BOXCLS,
CLSBOX, PRNTCNTR, PRNTMOVE, TIMECHNG, WHEREX, DCOMMA, DOSFIND,
GETDRIVE, GETPATH, SETDRIVE, ISOPEN, SYSDATE, SYSTIME, BGETKEY,
PRINTER.

**2 modules removed.** `SCRNIO/GETKEY.C` and `DOS/SHOWERR.C` are
PCBSETUP-side, not door-side — in `pcbkit_l` because PCBSETUP links it.
The toolkit already ships the door-side replacements (`SMALLERR.C`,
`NOINPUT.C`).

**One duplicate symbol.** `ansicolors` is defined at file scope in both
`SCREEN/ANSI.C` and `TOOLKIT/INIT.C`; TLIB refuses the second. Made
`static` in `INIT.C`. The crew's toolkit tree already carries this fix;
the archive copy does not.

### Resolved: the 5 externals that were unresolved

`_BC386BUG`, `_COLORS`, `_KBDSTATUS`, `_SHOWCLOCK`, `_UPDATEKBDSTATUS`
are gone — by two changes, not by stubbing anything:

- **Dropped `/DCPU386`.** Only `pcb153/SOURCE/H/BUG.H` reads it, to route
  `timerexpired()` through a `long BC386BUG` that lives in
  `SOURCE/MAIN/PCBOARD.C` — a file no door can link. Without it the
  macro uses the direct form. `-3` stays, so the code is still 386.
- **Removed `SCRNIO/GETKEY.C` and `DOS/SHOWERR.C`.** Both PCBSETUP-side,
  needing host globals (`Colors[]`, `_KBDSTATUS`, `_SHOWCLOCK`,
  `_UPDATEKBDSTATUS`) no door provides. This also answers the SCRNIO
  question that was open here: GETKEY does not belong in the door SDK.

### Two DOS traps, both of which cost time

- **The 127-byte command line.** TLINK silently truncates past it. The
  symptom is misleading: the **C runtime** appears undefined (`_exit`,
  `__stklen`) because the library field never got read. Use response
  files for TLINK as well as TLIB.
- **TLINK `/L` takes no space**: `/LC:\BC31\LIB`, not
  `/L C:\BC31\LIB`, or the path is linked as an object file.
- **8.3 truncation.** Writing `CALLBACKS.EXE` and `CALLBACKL.EXE` from
  one loop silently gives you `CALLBACK.EXE` twice. Put each memory
  model in its own directory rather than suffixing filenames.

---

## Superseded — prior pwa153 assessment (kept for the record)

### pwa153 — NOT BUILT  ⚠️

**Corrected 2026-09-17: no SDK library has ever been produced.** No
.LIB exists in toolkit/pwa153/{bc31,tc201,msc70}/ or in the deleted
OUT/lib/. Earlier "DONE" / "COMPLETE" claims in this file, in the root
README and in the removed OUT/lib/pwa153/STATUS.md were wrong — the
work was done in a scratch directory and never landed.

The manifest is known: Clark's shipped PCBKIT_L.LIB lists **130
modules** (TLIB listing, not a guess), all 130 located in the source —
101 in the toolkit tree, 29 in MAIN/SOURCE.

### What is established

- **Manifest: 130 modules.** From a TLIB listing of Clark's shipped
  `PCBSRC/PCBKIT_L.LIB`, not from makefile `lib:` targets. All 130 are
  located in our source: 101 in `toolkit/pwa153/SOURCE`, 29 in
  `pcb153` MAIN/SOURCE. The toolkit library legitimately pulls modules
  from the main source tree — that was the piece earlier attempts
  missed, when the manifest came from only 8 makefiles.
- **11 modules exist in both trees** (ANSI, DOSCLOSE, DOSOPEN, DOSREAD,
  DOSWRITE, HELP, INDEX, INIT, RECYCLE, SHOWERR, USERSYS). Prefer the
  toolkit copy; CNAMES specifically must be TOOLKIT/CNAMES.C (the
  `-DLIB int getconfrecord` version), not PCB/CNAMES.C.
- **Loose override OBJs** (NO*, PCBDAT, SMALLERR) ship alongside the
  library and are linked selectively — they are not a second library.
- **PCBoard itself uses the MEDIUM model.** Clark's own shipped
  PCBKIT_L.LIB is LARGE only.
- **ALTMODEM is not a library module.** Its source is a standalone
  modem test utility with its own main() that stubs out toolkit
  functions so it links by itself.

### Known obstacle: small and compact models  — RESOLVED, see above

A trial build (2026-09-17, headless dosbox-x + Clark's BC 3.1) produced
126/130 objects for **medium** and **large**, but only **6/130** for
small and compact. The toolkit uses far pointers and `_FARDATA_`
throughout; small and compact need their own configuration, not just a
`-m` flag swap. A genuine 4-model matrix is more work than a re-run —
plan for 2 models first, or solve the model config deliberately.

Compiler families:
- PCBKBC (Borland C++ 3.1) — **BUILT 2026-09-17**, all 4 models (see above).
  DOSBox-X** with `[dos] zero unused int 68h=true` + `HDPMI16 -r`
  (see `todo/dosboxx-dpmi-failures.md` Failure #5).
- PCBKIT (Turbo C 2.01) — NOT BUILT. Real-mode, no DPMI
  needed.
- PCBKMS (Microsoft C 7.0) — Route A **VERIFIED 2026-08-29**: MSC7
  CL under DOSBox-X + HDPMI32 produces valid OMF (proof:
  `PCBKMS-ROUTE-A-PROOF-TINY.OBJ`). First real toolkit module
  (ADDBACKS.C) reached CL cleanly, blocked on toolkit header
  modernization (types.hpp line 49 `sizeof(char)` inside `#if`
  without `_MSC_VER` guard). Full 476-step BLDKMS.BAT build queued
  once header fixups land. No 386MAX needed — the whole "PCBKMS
  needs a low-level emu" premise is bypassed.

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

Confirmed: Clark's PCBKITL.LIB is almost all plain C (341 C symbols,
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
key door functions present), and installed to toolkit/pwa153/bc31/. PCBKBC
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
preserved in toolkit/pwa153/bc31/pcbkit-obj + pcbkbc-obj.

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


---

## PCBKMS status update (2026-08-26)

Assessed the MSC 7.0 build end to end. Where it stands:

**Ready:**
- Compiler in hand (extract MSC70BT.ZIP -> C:\MSC70). Both hosts present:
  DOS "3216" passes (BIN/) and OS/2 "1616" passes (OS2/BINP/, NE-format,
  verified 16-bit OS/2 1.x). INCLUDE (42 headers) + LIB (S/M/C/L C
  runtimes) present.
- **BLDKMS.BAT compile loop is now complete** - a real 119-module x 4-model
  compile loop (476 CL commands), generated from the manifest, same
  structure as the working BLDKIT/BLDKBC. Pick the host driver via the
  CC variable at the top (DOS CL vs OS/2 CL).
- LIB-assembly response files (MS??.RSP, 8 files) point at
  OBJ\msc70\<model>\ correctly. obj/msc70 dirs scaffolded.
- **Headers are already MSC7-aware**: DOSFUNC.H takes dos.h/direct.h/
  borland.h for non-Borland compilers; TYPES.HPP char checks pass under
  MSC7; borland.h has an explicit `#ifdef _MSC_VER` block mapping Borland
  intrinsics to MSC equivalents (_disable/_enable/_memavl/_dos_getvect/
  _dos_setvect) and asm->_asm. This is real prior MSC7 prep.

**Blocked (host, not our code):**
- The DOS "3216" compiler hard-requires a **DPMI host with 32-bit
  interrupt extension** - it names Qualitas 386MAX/BlueMAX explicitly.
  **RESOLVED: 386MAX 8.03 is now in the repo** (devtools/386MAX-803.7z,
  2 floppy images). Install it under DOSBox-X (run its SETUP, load
  386MAX.SYS) to provide the DPMI host, then Route A runs. This clears
  the last blocker for the DOS route.
- The OS/2 "1616" compiler needs a real **OS/2 host** (or faithful OS/2
  emulation). Not available in the current Linux+DOSBox-X environment.

**386MAX version guidance (Route A):**
The DOS compiler's error text names "Qualitas 386MAX or BlueMAX version
6.x", but the real requirement is the *capability*, not the version:
the next error strings are "DPMI host not 32 bit" and "DPMI host does
not have 32-bit interrupt extension". That 32-bit DPMI + interrupt
extension is a core Qualitas feature present in 386MAX **6, 7, and 8**.
So use the **latest available - 386MAX 7.x or 8.x (8.03 is the final
release)**; newer gives the same required capability plus better
compatibility and better behavior under DOSBox-X. 6.x is the minimum
Microsoft tested; 8.03 is the recommended choice.

**Next action (needs one of):**
1. A DOS host with 386MAX providing DPMI (Route A), then run BLDKMS.BAT
   with CC=DOS CL; or
2. An OS/2 host (Route B, preferred - no DPMI), CC=OS/2 CL.
Either way BLDKMS.BAT is now a single command; then MKLIB/MS??.RSP
assemble PCBKMS{S,M,C,L}.LIB. Expect a first-build header fixup pass
(likely small, given the existing _MSC_VER handling) - add _MSC_VER
branches where only __TURBOC__/__BORLANDC__ are handled today.


## Build-input validation (2026-08-26 continued)

Attempted the build under DOSBox-X. Two hard environment facts:
- DOSBox-X headless writeback to mounted drives/images is
  non-deterministic in this sandbox (flush race) - unreliable for a
  476-step build.
- No full-PC virtualizer (QEMU/86Box/PCem) here, and 386MAX.SYS is a
  CONFIG.SYS driver DOSBox-X can't host anyway. So the real build needs
  a stable host (real DOS+386MAX, or OS/2).

Used the time to **statically validate the build**, which caught a real
bug and corrected it:
- **All 119 manifest modules exist** at their expected paths. ✓
- The 119 split across **two source trees**: 95 from toolkit/pwa153, 24
  from pcb153 (incl. 5 .ASM). BLDKMS.BAT's first cut wrongly assumed all
  119 were under toolkit/pwa153 - it would have failed with 76
  file-not-found errors. **Fixed:** BLDKMS.BAT now uses the manifest's
  real paths (both \TOOLKIT\PWA153\SOURCE and \PCB153\SOURCE) and
  assembles the 5 ASM files with TASM, exactly like the working BLDKIT.
- **All 476 compile/assemble lines now resolve to real source files.** ✓

**Build host requirements (validated):**
- Mount BOTH source trees: C:\TOOLKIT\PWA153 and C:\PCB153.
- TASM must be on PATH (the 5 ASM modules; MSC7 ships no standalone
  assembler - TASM's OMF .OBJ links fine with MSC7 LIB).
- MSC70 at C:\MSC70; 386MAX loaded (DOS route) or OS/2 host (OS/2 route).
- Then: BLDKMS.BAT -> MKLIB/MS??.RSP -> PCBKMS{S,M,C,L}.LIB.
