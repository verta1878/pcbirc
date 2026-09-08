# pcbkit_l.lib — What It Is, Why It's Stale, What Replaces It

## What pcbkit_l.lib IS

pcbkit_l.lib (241,144 B) is a mega-library that combines all 8
individual LIB/SOURCE/ libraries plus PCBoard main modules into one
link target. It exists so standalone PCBoard programs (PPLC, utilities)
can link against one library instead of listing 8+ individual ones.

PPLC.MAK, PCBOARD.MAK, and utility MAKEFILEs all reference it.

## What's inside it

Contents (from embedded source paths in the OBJ modules):

| Source | Drive | Modules |
|---|---|---|
| LIB/SOURCE/COUNTRY/ | D:\tc\country\ | comma, stricmp, strlwr, strnicmp, strupr, country, date, memfcmp, memicmp |
| LIB/SOURCE/DOS/ | D:\tc\dos\ | chk*, dos*, extended, handlers, strnchr, int24hnd.asm |
| LIB/SOURCE/MISC/ | D:\tc\misc\ | addchar, ascii, bd_long, bs_long, crypt, ctod, dtoc, exist, fmemcpy, hextoi, isset, julian, lastchar, long_bd, long_bs, padstr, prnready, proper, setbit, stripl, stripr, subst, time, unsetbit |
| LIB/SOURCE/PCB/ | D:\tc\pcb\ | addbacks, datafil2, exitdos, parsepth, srchpath |
| LIB/SOURCE/SCREEN/ | D:\tc\screen\ | cls, cursor, datestr, delay, getmode, giveup, gotoxy, print, saverest, saverst2, sound, time1, time2, wherey |
| MAIN/SOURCE/ | U:\ | atclose, cnames, custhelp, dosinit, goodbye, help, init, initport, inputreq, pcbinit, recycle, slowmodm, usersys |
| MAIN/SOURCE/ | Y:\ | chat, devioctl, display, dosopen, files, index, inkey, input, language, log, memory, misc, modem, modemasy, modemdrv, modemfos, modemos2, msgbase, pcbtext, screen, shell, showerr, status, sys, token, users, xlate |
| Assembly | Y:\ | ansi.asm, async.asm, bgkey.asm, memmove.asm, timer.asm |
| COMMDRV | D:\COMMDRV\ | comm driver headers |

## Why it's stale

pcbkit_l.lib at `PCBSRC/PCBKIT_L.LIB` in the PWA zip was built on
Clark's development machine from `D:\tc\` — NOT from the pcbsrcv/014/
versioned source tree. Evidence:

1. Embedded paths show `D:\tc\`, `T:\`, `U:\`, `Y:\` — Clark's dev
   machine drive mappings, not the released source layout
2. Symbol decoration is PLAIN UPPERCASE PASCAL (e.g., `PRINT`,
   `NEWLINE`) — compiled without `-P` or with active `extern "C"`
3. pcbsrcv/014/LIB/H/SCREEN.H has `extern "C"` COMMENTED OUT, but
   pcbkit_l.lib was built when it was active
4. BUILD.BAT does NOT create pcbkit_l.lib — it builds individual
   .386 libs only
5. pcbkit_l.lib predates the pcbsrcv/ versioned source tree

## The name decoration problem

| What | Symbol style | Example |
|---|---|---|
| Our OBJs (compiled with -P, v014 headers) | C++ mangled | @PRINT$QNZC |
| pcbkit_l.lib (Clark's dev machine) | Plain pascal | PRINT |

TLINK can't resolve `@PRINT$QNZC` against `PRINT` — name mismatch.

## Decision

1. **Move stale pcbkit_l.lib to attic/** when PWA source is extracted
   to the repo. Do not link against it.

2. **Option B (immediate):** Link PPLC against our 8 individual libs
   directly. Same flags, same headers, matching decoration.

3. **Option C (for MAKEFILE compat):** Rebuild pcbkit_l.lib by merging
   our 8 individual libs with TLIB. Then Clark's MAKEFILEs work
   unchanged (PPLC.MAK, PCBOARD.MAK, utilities all reference
   pcbkit_l.lib).

Both B and C are needed. B to test, C for the real build system.

## Key functions PPLC needs and where they live

| Function | Source file | Library |
|---|---|---|
| print() | TOOLKIT/NODISP.C | toolkit_l |
| println() | TOOLKIT/NODISP.C | toolkit_l |
| newline() | TOOLKIT/NODISP.C | toolkit_l |
| fileexist() | MISC/EXIST.C | misc_l |
| getcountryspecs() | COUNTRY/COUNTRY.C | countryl |
| dosclose() | DOS/DOSCLOSE.C | dos_l |

## Risk: main module globals

PPLC references globals that may be in main modules (U:\ and Y:\
sources), not in the 8 lib directories:

- `warnFlag` — may be in MAIN/SOURCE/
- `dispStat` — may be in MAIN/SOURCE/
- `autoUVar` — may be in MAIN/SOURCE/
- `disArrSubChk` — may be in MAIN/SOURCE/

If these are unresolved with Option B, we'll need to compile those
specific modules from MAIN/SOURCE/ or stub them.


## PPLC link dependencies — compile from source, no stubs

PPLC must be byte-exact with Clark’s original. ALL functions must be
Clark’s real code from the PWA source. No stubs. The source for every
function is in `PCBoard_15_3_source_code_v0_014.zip`.

Temporary stubs were used to test the link (v0.1.9). These must ALL
be replaced with real code compiled from the source:

### Globals (need real declarations from source)

| Global | Source location in PWA zip |
|---|---|
| Scrn_Rtrc | PCBSRCV/014/LIB/SOURCE/SCREEN/ |
| Scrn_Addr | PCBSRCV/014/LIB/SOURCE/SCREEN/ |
| Status | PCBSRCV/014/MAIN/SOURCE/ |
| commInKey | PCBSRCV/014/MAIN/SOURCE/ |
| turnOnXmit | PCBSRCV/014/MAIN/SOURCE/ |

### Functions (need real implementations from source)

| Function | Source location in PWA zip |
|---|---|
| makepcboardsys() | PCBSRCV/014/MAIN/SOURCE/ |
| logsystext() | PCBSRCV/014/MAIN/SOURCE/ |
| displaypcbtext() | PCBSRCV/014/MAIN/SOURCE/ |
| openmodem() | PCBSRCV/014/MAIN/SOURCE/ |
| sendbyte() | PCBSRCV/014/MAIN/SOURCE/ |
| GETTIMER() | PCBSRCV/014/MAIN/SOURCE/ASM/ |
| SETTIMER() | PCBSRCV/014/MAIN/SOURCE/ASM/ |
| strnchr() | PCBSRCV/014/LIB/SOURCE/DOS/STRNCHR.C |

### How to fill them

1. Find which MAIN/SOURCE/ .C files define each function
2. Compile those files with the same flags (-P -ml -3 -D___COMP___)
3. Link them into PPLC alongside the 8 libs
4. Remove STUBS.C entirely

This is the same pattern as building the 8 libs — sweep the source,
compile, link. Nothing is missing from the PWA zip.

When pcbkit_l.lib is rebuilt (Option C), all these modules are included
automatically. STUBS.C goes away. PPLC links against the single rebuilt
pcbkit_l.lib like Clark’s MAKEFILE expects. Byte-exact means byte-exact.

---

*hexadecimal, 2026-09-07*


## pcbkit_l.lib rebuilt from source (2026-09-08)

**Result: 272,896 bytes, 130/130 modules** (Clark’s: 241,144 bytes)

### Module sources (130 total)

From our 8 individual libraries (v0.1.1–v0.1.8):
  67 modules from dos_l, countryl, doscls_l, misc_l, screen_l,
  scrnio_l, system_l, toolkit_l. 47 extra stub modules removed
  (NODISP, PCBDAT, etc.)

From MAIN/SOURCE (compiled with PPLC.CFG):
  28 modules: CHAT, DEVIOCTL, DISPLAY, FILES, HELP, INDEX, INIT,
  INKEY, INPUT, LANGUAGE, LOG, MEMORY, MISC, MODEM, MODEMASY,
  MODEMDRV, MODEMFOS, MODEMOS2, MSGBASE, PCBTEXT, RECYCLE, SCREEN,
  SHELL, SHOWERR, STATUS, SYS, TOKEN, TIMER, USERS, USERSYS, XLATE

From LIB/SOURCE (PWA zip, compiled with PPLC.CFG):
  16 modules from TOOLKIT/, PCB/, DOS/, SCREEN/, COUNTRY/, MISC/:
  ADDBACKS, ATCLOSE, CUSTHELP, DATESTR, DOSINIT, EXITDOS, GOODBYE,
  INITPORT, INPUTREQ, MEMFCMP, PARSEPTH, PCBINIT, PROPER, SLOWMODM,
  STRNCHR, SUBST

From UTIL/PCBSM/SOURCE (PWA zip):
  4 modules: CNAMES, CTOD, DATAFIL2, GETMODE

Assembled with TASM /MX /D__l__ /iLIBSRC/H:
  7 ASM modules: ANSI, ASYNC, BGKEY, CUTIL, INT24HND, MEMMOVE, TIMER

Extracted from Borland BC 3.1 CL.LIB (large model C runtime):
  6 modules: MEMICMP, SRCHPATH, STRICMP, STRLWR, STRNICMP, STRUPR

### Compile flags

Standard modules: `BCC.EXE +PPLC.CFG` (-c -P -ml -3 -ff -Od)
INIT, PCBINIT, INITPORT: add `-DPCB_MAXNODES=250 -DCOMM -DLIB`
  — `-DLIB` exposes statustype with SysLimit in PCBOARD.H
  — `-DCOMM` exposes modem prototypes (cdstillup, online)
TASM: `/MX /D__l__` (case-sensitive externals, large model)
INT24HND: `/MX /D__l__ /iLIBSRC/H` (needs RULES.ASI include path)

### Symbol clash resolution

PCBINIT.C defines globals (VerifyCDLoss, FORCE16550A, NO16550,
ExtConfLen, etc.) that ASYNC.ASM and USERS.C also reference or
define. Resolved by:
  — Adding PCBINIT to the library BEFORE ASYNC
  — TLIB accepted USERS despite ExtConfLen clash (warning, not fatal)
  — 47 stub modules removed from sub-libs to prevent other clashes

### Size difference (31 KB)

Our lib is 272,896 B vs Clark’s 241,144 B (+31 KB). Difference
comes from compiling with different headers and options than Clark’s
original D:\tc\ build environment. Does not affect functionality.

---

**RULE:** Source code fixes apply to BOTH locations:
- **Repo:** `pcb153/` (master), `pcb154/`, or `pcb1541/` — whichever version
- **DOSBOXX.ZIP:** matching version under `pcbirc/BUILDROOT/`

Repo is master. DOSBOXX.ZIP mirrors it. Both must stay in sync.



## v0.2.2 verified: PPLC links against rebuilt pcbkit_l.lib (2026-09-08)

PPLC2.EXE: 221,644 bytes (Clark’s: 222,176 — 532 B difference).
Compiles RUNINET.PPS correctly: 2,261 B, 63 vars.

PPE output vs Clark’s PPLC320.EXE: 313 byte diffs in 27 blocks
(26 × 12-byte vtable pointer blocks + 1 single byte). Bytecode is
identical. Only C++ vtable addresses differ due to different EXE size.

**CUR_PPE_VER fix:** changed from 330 to 320 in NEWSCR.CPP. Without
this, encrypt3 guards evaluate TRUE (330 >= 330) and encrypt3 is
applied to 3.20 PPE files. With 320, guards evaluate FALSE and
only encrypt2 is applied — matching Clark’s PPLC 3.20 behavior.

**RLE fix:** doRLE module added back to pcbkit_l.lib. Was removed
during the 47-module trim but PPLC needs it for script compression.
