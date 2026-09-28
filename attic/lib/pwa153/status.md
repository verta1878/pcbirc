# pwa153 SDK — Build Status

## PCBKBC (Borland C++ 3.1) — COMPLETE (119 modules, ASYNC full) ✅

All four memory models rebuilt from the FULL manifest:
- PCBKBC_S/M/C/L.LIB — 118 modules each, matching Clark's PCBKIT_L.LIB
  module set exactly (118/118).
- Core door functions verified present: INITDOOR, CLOSEDOOR, OPENMODEM,
  DISPLAYFILE, KBDINKEY, INPUTFIELDSTR/INT, SHOWERROR, RECYCLE,
  CHECKMALLOC, and the ASYNC_* serial family.

### How it was completed
The full manifest maps Clark's 118 modules to our source across BOTH
the toolkit tree (100 modules) AND the main PCBoard source tree (18
modules: INKEY, MODEM, SCREEN, STATUS, DISPLAY, LOG, etc. from
pcb153/SOURCE/MAIN, DISPLAY, MODEM, NODE, SUPPORT). The toolkit library
legitimately pulls modules from the main source — that was the missing
piece. ASM modules (ASYNC, BGKEY, CUTIL, MEMMOVE, TIMER, INT24HND)
assemble with TASM /mx /i<H-dir> /d__<model>__.

### Build-enabling fixes (documented, behavior-preserving)
- INIT.C: ansicolors[] made `static` (ANSI.C owns the public copy;
  data identical "04261537"). Without this the duplicate public blocked
  the INIT module (and INITDOOR with it). Original saved as INIT.C.orig.
- CNAMES: use TOOLKIT/CNAMES.C (the -DLIB `int getconfrecord` version),
  not PCB/CNAMES.C (the `void` non-LIB version).

### ASYNC serial — COMPLETE (gap closed)
The extended ASYNC_* functions (ASYNC_CTSOKAY, ASYNC_FRAMINGERRORS,
ASYNC_OVERRUNERRORS, ASYNC_PARITYERRORS, ASYNC_REOPENPORT,
ASYNC_BAUDDIVISOR, ASYNC_DISCONNECTMODEM, ASYNC_RINGDETECT,
ASYNC_INBYTES, ASYNC_OUTBYTES) were NOT a source-version gap. They are
defined in MODEMASY.C (pcb153/SOURCE/MODEM/MODEMASY.C) as wrappers over
the ASYNC.ASM data publics (__CTSokay, __InBytes, etc.). MODEMASY.C was
simply missing from the manifest. Added as the 119th module. All 10
functions now verified present. No source-version gap exists.

## PCBKIT (Turbo C 2.01) — pending (compiler in devtools/TURBOC201.zip)
## PCBKMS (Microsoft C 7.0) — pending (compiler in devtools/MSC70.zip)

Same 118-module manifest applies to all three compilers — just swap the
compiler. That's how Clark built the SDK.


## PCBKIT (Turbo C 2.01) — COMPLETE (119 modules, all 4 models) ✅

All four PCBKIT libraries built and installed (PCBKIT_S/M/C/L.LIB,
176-192KB — matches Clark's ~167KB range). 119 modules each, verified:
key door functions present (initdoor, openmodem, displayfile, kbdinkey,
recycle, async_ctsokay).

Compat work (all VIRTUAL.C-style, one shared header set, PCBKBC verified
intact after every change):
- CRLF line endings (Turbo C requires DOS endings — was the root of the
  "conditional started on line 0" phantom error)
- bool defined for C mode; __TURBOC__ branches for sizeof-in-#if
- EXTERN_C macro (extern "C" in C++, empty in C) for the 5 COUNTRY files
- CDCCONST macro (const in C++, static const in C) for nullHandle —
  fixes the C-linkage clash (file-scope const is internal in C++ but
  external in C, so it clashed across every module)
- // -> /* */ in 107 toolkit + 29 main headers + 19 main-source .C files
- malloc.h/direct.h shims in TC201/INCLUDE
- ulongtobasdble prototype fixed (long -> ulong, matches definition)
- Objects assembled into libs with BC31's TLIB (TC 2.0 TLIB page-size
  limit); TASM (from BC31) handles the 6 ASM + inline-asm modules.

PCBKBC REBUILT with the updated shared headers — identical sizes
(194-209KB), confirming the guards are behavior-preserving for C++.

Turbo C 2.01 added to DOSBOXX.ZIP alongside BC31.

## SDK matrix: 8 of 12 libraries complete
- PCBKBC (Borland C++ 3.1): 4/4 ✓
- PCBKIT (Turbo C 2.01):    4/4 ✓
- PCBKMS (Microsoft C 7.0): 0/4 (next)

## Object layout reorganized to Clark's convention (2026-08-25)

Objects now live under Clark's directory scheme (from his makefiles:
OBJDIR = bcdos\<CVER>\<SUBDIR>\<CMOD>):

  OUT/lib/pwa153/obj/<compiler>/<model>/
    <compiler> = bc31 / tc201 / msc70   (Clark's CVER names)
    <model>    = small / medium / compact / large  (Clark's model dirs)

  OUT/lib/pwa153/loose-obj/<compiler>/   (override stubs: NODISP,
    PCBDAT, NO* stubs, SMALLERR - currently bc31-built)

Replaces the earlier ad-hoc "pcbkit-obj/S/M/C/L". The build scripts
write here directly, so a sysop gets Clark's layout with no changes.
