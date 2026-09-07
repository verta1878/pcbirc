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

---

*hexadecimal, 2026-09-07*
