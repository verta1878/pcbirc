# The category libraries under OS/2 — 250 of 275 modules, **nine of nine libraries**

**Verified 2026-09-23**, Open Watcom 2.0 beta, `wpp386 -bt=os2v2 -mf`, in the
sandbox. This is the first time any of the category libraries has been built
for anything but 16-bit DOS.

| Category | modules | built | library | bytes |
|---|---:|---:|---|---:|
| COUNTRY | 13 | **13** | `COUNTRY2.lib` | 22,528 |
| DOS | 46 | 44 | `DOS_2.lib` | 60,416 |
| DOSCLS | 1 | **1** | `DOSCLS2.lib` | 15,360 |
| MISC | 87 | 79 | `MISC_2.lib` | 89,600 |
| PCB | 22 | 19 | `PCB_2.lib` | 128,000 |
| SCREEN | 40 | 33 | `SCREEN2.lib` | 41,472 |
| SCRNIO | 20 | 19 | `SCRNIO2.lib` | 54,784 |
| SYSTEM | 7 | **7** | `SYSTEM2.lib` | 23,552 |
| TOOLKIT | 39 | 35 | `TOOLKIT2.lib` | 62,464 |
| **total** | **275** | **250** | **9 libraries** | |

## The thing nobody had checked: the OS/2 port is already in the source

The triage said this would be a rewrite. 3,286 lines of Borland 16-bit inline
assembler sit in 91 of the 275 modules — `asm Mov Ax,Ds`, which Open Watcom
does not accept in any form.

It compiled anyway, because **Clark already wrote the OS/2 arm**. Every
asm-bearing module is shaped like `COUNTRY\MEMFCMP.C`:

    #ifdef __OS2__
       ... portable C ...
    #else   /* ifdef __OS2__ */
       asm     mov   ax, ds
       ...

`DOSFUNC.H` has the same structure — four separate `#ifdef __OS2__` blocks,
including a 30-handle table and a different `DOSFILE`. The OS/2 support was
written and then never built. All seven asm-bearing COUNTRY modules produce
real objects, 681–3,467 bytes, through the `__OS2__` arm.

## The recipe

    WATCOM=/opt/watcom ; PATH=$WATCOM/binl64:$PATH

    wpp386 -bt=os2v2 -mf -5 -ox -zq -zp1 -ei \
           -d__OS2__ -dLIB -dNDEBUG -dCOMM -dPCBSTATS \
           -dPCB_MAXNODES=25 -dMULTIPORT -dKEY=KEY_25 \
           -i=<toolkit/H> -i=<pcb153/SOURCE/H> -i=$WATCOM/h -i=$WATCOM/h/os2 \
           -fo=obj/<CAT>/<mod>.obj <src>

    wlib -q -b -n lib/<CAT>2.lib +obj/<CAT>/*.obj

Four things about that command line cost real time, and none of them is
obvious from a failure message.

### 1. `wpp386`, not `wcc386`

`PCBTOOLS.H:1278` declares `class CSemaphore`. Compiled as C it dies in a
cascade of *"Missing or misspelled data type near 'class'"* 1,200 lines into a
header, and nothing points at the cause. Clark built these with Borland's
`-P` — compile as C++ — and the same rule holds here, exactly as it does for
`PACKFIDO` and `FIDOUTIL`.

### 2. `-dLIB` is not optional

`PCB.H` has an `#ifdef LIB` split. The library arm declares
`int LIBENTRY getconfrecord(...)`; the other arm declares `void`. `CNAMES.C`
defines `int`. Without `-dLIB` you get *"Inconsistent return type"* on a
function whose definition and declaration are three lines apart in files that
look correct.

### 3. The project include directories must come **first**

Open Watcom ships its own `h/dosfunc.h`. With `-i=$WATCOM/h` ahead of the
toolkit headers it wins, Clark's `DOSFUNC.H` is never read, and you get
*"No prototype found for `dosfseek`"* and *"Symbol `OPEN_DENYNONE` has not
been declared"* — from a header that is sitting right there and was silently
shadowed. Nothing in the diagnostics names the file that got picked.

### 4. `WATCOMPAT.H` must **not** be force-included here

`WATCOMPAT.H` was written for the `pcb154` MAIN tree, where the toolkit
headers are not in play. Force-include it into a category build and it
collides head-on with Clark's own headers — `DOSFILE` already defined,
`OPEN_RDWR` / `OPEN_NORMAL` / `OPEN_RDONLY` redefined, `coreleft`, `getvect`
and `setvect` redefined against `borland.h`.

It is not needed. `DOSFUNC.H` already does the bridging itself:

    #if defined(__BORLANDC__) || defined(__TURBOC__)
      #include <dir.h>
    #else
      #include <dos.h>
      #include <direct.h>
      #include <borland.h>
    #endif

Clark's `borland.h` **is** the compatibility layer, and the `#else` arm was
already waiting for a non-Borland compiler. Drop `-fi=watcompat.h` and the
whole conflict disappears.

## The one source change this needed — `TYPES.HPP`, and it is guarded

Two things in `TYPES.HPP` stop Open Watcom C++ dead, and both are guarded so
the Borland arm is character-for-character Clark's. The DOS builds are
unaffected by construction, not by testing.

**The min/max macros parse as destructor calls.** `maxSType(t)` expands to
`t(~t(t(1) << ...))`, and in C++ `~t(` where `t` is a type name is a
pseudo-destructor call. The standard agrees with Watcom; Borland let it pass.
234 errors from one macro. The Watcom arm writes the same arithmetic with C
casts so the `~` applies to a parenthesised expression.

**`#if __BORLANDC__ < 0x500` is true when `__BORLANDC__` is undefined**, so
under Watcom it defines `bool` and `const bool true/false` — all three are
built-in keywords there. Three sites, now `#if !defined(__WATCOMC__) && …`.

The `delta154`, `irc1541` and `pcb154/LIB` copies of `TYPES.HPP` **already had
these guards** from the earlier OS/2 work. Only the `pwa153` and `pwa154`
toolkit copies were behind; they now match.

## What does not build yet

26 modules. The one that matters most is first:

| Category | Modules |
|---|---|
| MISC | `COPYFP`, `CPUTYPE`, `PSEARCH`, `SWAPENV`, `VIRTUAL`, `WILDCARD`, `ZSWAPSTR`, `ZSWAPVIR` |
| SCREEN | `ANSI`, `GETMODE`, `TIMECHNG`, `TWODIG`, `TWODIG0` and two more |
| PCB | `CNAMES`, `CONFFUNC`, `DATA120` |
| DOS | `DOSFIND`, `SHOWERR` |
| TOOLKIT | `COPY2MSG`, `INIT`, `NOINPUT` |
| SCRNIO | `GETKEY`, `INITSCRN` |
| SYSTEM | `THREADS` |

Several of these are DOS by nature — `CPUTYPE`, `SWAPENV`, `VIRTUAL` and the
`ZSWAP*` pair are the EMS/XMS swapper, `GETMODE` and `ANSI` are BIOS text
mode — and under OS/2 they want stubbing rather than porting. `DOSCLASS.CPP`
is the one that has to be done properly, because nothing above it works
without it.

## Status of this work

These libraries are built **in the sandbox**, not committed. They are the
proof the chain compiles, not a release. A released OS/2 SDK needs
`DOSCLASS.CPP` finished first, then the 26 triaged into ported / stubbed /
dropped, then a link of something real — `FIDOUTIL2.EXE` is the obvious first
target, since it needs eight of these nine.

## DOSCLS: two real bugs, both invisible on DOS

`DOSCLASS.CPP` is one file and it is the whole DOSCLS category, and
`cDOSFILE` is what `FIDOUTIL` does all of its I/O through. It now builds.
Two things were in the way, and neither could ever show up in a DOS build.

**1. `DOSCLASS.CPP` overrode the OS/2 error macros.** Under `__OS2__`,
`PCBTOOLS.H` makes the `dosXXXX` functions carry an extended-error pointer,
because a per-thread error state cannot live in a global:

    #define DOS2ERROR    ,os2errtype *Os2Error
    #define POS2ERROR    ,&Os2Error

`DOSCLASS.CPP` then redefined all three to empty inside a block headed
*"For Toolkit Compatibility"* — correct for DOS, fatal for OS/2. The
result was *"number of arguments for function 'dosread' is incorrect"* on
half a dozen calls. That block is now `#if defined(__OS2__)` / `#else`,
with a file-scope `os2errtype Os2Error` for `POS2ERROR` to point at —
which mirrors exactly what the DOS build does with its global error
variables. The DOS arm is unchanged.

**2. `PCBTOOLS.H` was missing `DOS2ERROR` on `dosappend`.** Every sibling
has it. `DOSFUNC.H:151` has it on the same prototype, `DOS\DOSAPPEN.C:41`
has it on the definition, and `DOS\CHKAPPEN.C:40` passes `POS2ERROR` at
the call. Only `PCBTOOLS.H:1008` dropped it. **On DOS the macro is empty,
so the two headers agreed and nothing ever complained** — the declaration
has been wrong since 1996 and could not be noticed until something
compiled for OS/2. Fixed to match.

## Nested comments — `-C` is load-bearing

Two lines in the 15.3 headers are commented-out code containing a second
comment:

    /*#define  QUEUE_FILE  "FIDOQUE.DAT"   /* Name of the Queue */ */
    /*char     Messages[MAXFLEN];          /* name of file for messages */ */

Standard C ends the comment at the first `*/` and chokes on what is left.
Borland compiles them because `-C` — **nested comments**, and it is in
`ALL.RES` and in `FIDOUTIL.CFG` — is on. Open Watcom has no such switch.
Both lines are now written without the inner `/*`. They are comments
either way, so no generated code changes on any compiler.

Only `pcb153\SOURCE\H\DEFINES.H` and `STRUCTS.H` were affected. The
`upd154` and `pcb154\MAIN` copies of those headers use `//` comments and
needed nothing.

## FIDOUTIL for OS/2 — 8 of 11 modules

    ok    ANALIZE  CONVERT  MAINT  PTSETUP  REPORT
          DATA  PASSTHRU  SHOWERR2
    fail  FIDONET  PCBFU  CI_BUILD

Three left, each identified:

* **`FIDONET.CPP`** — `validatepath` undeclared. The prototype exists, in
  `toolkit\pwa153\H\validate.h`; the file just never includes it and got
  away with it under Borland.
* **`PCBFU.CPP`** — re-declares `srchpath` as `pascal` while `PCB.H`
  declares it `LIBENTRY`. Identical on DOS, a modifier conflict once
  `LIBENTRY` and `pascal` stop being the same thing.
* **`CI_BUILD.C`** — not yet triaged.

Two build-time shims carry the rest and touch no repo source: a Borland
`dir.h` (`MAXDIR` and friends onto `_MAX_DIR`) and `_argc`/`_argv` onto
Watcom's `__argc`/`__argv`.

## DOS did not regress

Rebuilt from scratch under Borland C++ 3.1 with every header change in
place — all 11 objects and the link:

    FIDOUTIL.EXE   154,362 bytes
    sha256 b577da5d4b7422a3a7c995a162bbeec925a07296c0b8cee953ceff74dce2e339

**Identical on two consecutive full rebuilds, byte for byte.** Same size
as before the changes. The earlier `6af917b5…` in this file's history came
from a link over a mixed object set — ten objects from an earlier session
plus one fresh — so `b577da5d…` is the reproducible number and the one to
compare against from here.

---

## Update — FIDOUTIL compiles 11/11 for OS/2

    ok  ANALIZE CONVERT FIDONET MAINT PCBFU PTSETUP REPORT
        CI_BUILD DATA PASSTHRU SHOWERR2      11 of 11, 0 errors

Three things got the last three modules over the line, and the middle one
is the important one.

### `LIBENTRY` is `pascal` on **both** compilers now

This was the bug behind *"function modifier conflicts with previous
declaration"* on `srchpath`, `read120file` and others — errors that point
at a declaration and a definition which both look perfectly correct.

`TYPES.HPP` used to make `LIBENTRY` **empty** under Watcom. But the headers
declare with `LIBENTRY` while a great many `.C` files spell the *same*
convention as the literal word `pascal` on the definition. Under Borland
those are the same word, so they always agreed and the split was invisible.
Watcom has `pascal` as a genuine calling-convention keyword, and **a
`#define` cannot suppress it** — the preprocessor still emits `__pascal`,
which is how this was diagnosed. Emptying `LIBENTRY` therefore put the
declaration and the definition on two different calling conventions.

`LIBENTRY` is now `pascal` unconditionally. Borland is unaffected: it
already resolved to `pascal` down the `#else` arm.

### `validate.h` — the "needs to be finished" note is finished

    #ifndef __OS2__  /* this needs to be finished */
    int LIBENTRY validatepath(FILE *Out, char *Path, char *ResultPath, char Choice);
    #endif

`MISC\VALIDATE.C`, which holds the definition, compiles clean for OS/2 —
the function walks a path string and is target-independent. Hiding the
prototype only stopped its callers (`PCB\DATA120.C`, FIDOUTIL's
`FIDONET.CPP`) from building. Now declared unconditionally.

### `DATA120.C` — the extended-error pointer, house pattern

Three `doscreate` calls now pass `POS2ERROR`, with the guarded local that
`DOS\CHKAPPEN.C` already uses:

    #ifdef __OS2__
    os2errtype Os2Error;
    #endif

`POS2ERROR` is empty on DOS, so the call sites are unchanged there.

## The link gets to symbol resolution

`wlink system os2v2` over the 11 objects and ten libraries runs and
resolves — it fails only on symbols owned by the 25 modules that still do
not compile, plus `VMFUNCS.C`. Named blockers: `getconfrecord`,
`putconfrecord`, `loadcnames` (`PCB\CNAMES.C`), `initscrnio`, `getch`
(`SCRNIO`), the `Scrn_*` set (`SCREEN`), `readcheck`/`writecheck` (`MISC`),
and the `VM*` set.

`VMDATA` is a tenth library now: `VMAVL.C` builds, `VMFUNCS.C` does not.
Its failure is the next class to deal with — **`bool`**. Watcom C++ has a
real `bool` where Borland had `typedef ubyte bool`, so a header saying
`bool` and a source saying `int` are no longer the same function, and you
get *"attempt to overload with a different return type"*.

## DOS, again

Rebuilt after the `LIBENTRY` change:

    FIDOUTIL.EXE  154,362 bytes
    sha256 b577da5d4b7422a3a7c995a162bbeec925a07296c0b8cee953ceff74dce2e339

**Byte-identical to the pre-change build.** Expected — Borland resolved
`LIBENTRY` to `pascal` before and resolves it to `pascal` now.

---

## Build against `OS2TK\H` — IBM's toolkit, already in this repo

**Corrected 2026-09-23.** The recipe above used Open Watcom's own OS/2
headers (`$WATCOM/h/os2`). It should use **IBM's**, which are sitting in
this repo at `OS2TK\H` — 332 headers, *"OS/2 Common Definitions file,
Copyright International Business Machines Corporation 1981, 1988-1992"*,
the OS/2 2.1 toolkit.

    -i=<toolkit H> -i=<pcb153 H> -i=OS2TK\H -i=$WATCOM/h

Open Watcom's OS/2 headers are a partial reimplementation. IBM's are the
real SDK PCBoard's OS/2 arms were written against, and they digest cleanly
under `wpp386 -bt=os2v2` — verified across the whole category chain:
**same 254/275 modules, FIDOUTIL still 11/11, the same nine undefined
references at link.** No module is lost by switching, and one hand-written
workaround is removed.

### The workaround this removes

`SCREEN\GETMODE.C` needed a hand-added `VIO_CONFIG_CURRENT` because Open
Watcom's `bsesub.h` declares `VioGetConfig` and `VIOCONFIGINFO` but omits
the selector. **IBM's `bsesub.h` has it** — line 767. Built against
`OS2TK\H`, `GETMODE.C` compiles clean with no addition at all, and the
patch has been reverted. The file is byte-identical to Clark's again.

The lesson generalises: reach for `OS2TK\H` before hand-defining anything
the OS/2 API should already provide.

---

# FIDOUTIL2.EXE — it links

    OUT\pwa153\os2\FIDOUTIL2.EXE   170,039 bytes
    sha256 224f4a6f6ec581898f7dd6cb3f263d5acc5e302dd3740a87a979f056427e50fc
    LX, OS/2 2.x console, i80386.  Zero undefined references.

Its own strings, out of the binary:

    PCBoard FIDO file conversion utility
    Copyright Clark Development 1996
    Convert 15.21 data to 15.23 format? (Y/N)

## Where the output goes

Following the layout already in `OUT/README.md` and the compiler legs
already under `toolkit/pwa153/`:

    OUT\pwa153\os2\            OS/2 binaries          <- new
    OUT\pwa153\                DOS binaries (Borland)
    OUT\clark-original\OS2\    Clark's shipped PCBOARD2.EXE, reference

    toolkit\pwa153\ow2\lib\    OS/2 libraries         <- new
    toolkit\pwa153\bc31\lib\   Borland DOS libraries
    toolkit\pwa153\msc70\      MSC 7.0 leg
    toolkit\pwa153\tc201\      Turbo C 2.01 leg

`ow2` sits beside `bc31`, `msc70` and `tc201` because that is what it is —
a fourth compiler leg of the same source, built from the same tree.

## The four things that closed the last nine symbols

**1. Stack calling convention — `-5s`, not `-5`.** This was the last one
and the least obvious. `-5` means `-5r`, register-based, and Watcom decorates
those names with a trailing underscore: `PCBFU.CPP` emitted `main_`. The OS/2
C library that has `_getch` is `clib3s.lib`, the **stack**-convention build,
and its startup wants plain `main`. Recompiled `-5s`, `main` appears, and
`clib3s.lib` resolves `_getch`. One flag, no source change.

**2. `MainHead1` / `MainHead2` — a real bug in Clark's FIDOUTIL.**
`SCRNIO\MENU.C:50` defines them as **pointers**, `char *MainHead1;`.
`PCBFU.CPP:55` declared them as **arrays** and line 175 did
`strcpy(MainHead1, "PCBoard Fido Utility")` — a copy through an
uninitialised pointer, every time FIDOUTIL started. `PCBSETUP\SOURCE\INIT.C:296`
does it correctly, by assignment. Borland linked the mismatch because its
linker matches names only; Watcom C++ encodes the type and caught it.
Declared and used the way the definition intends.

**3. `settimer` / `gettimer`** come from `pcb153\SOURCE\DOS\DOSTIME.C`, which
is not in any category library. Compiled for OS/2 and archived as `TIME2.lib`.

**4. `ALTMODEM.C` and `SLOWMODM.C` are left out of the OS/2 `TOOLKIT2.lib`.**
They are alternate DOS modem drivers for door authors, and each defines its
own `PcbData`. The linker pulled `ALTMODEM.obj` in to satisfy `PcbData` and
that dragged the whole DOS comm layer — `openmodem`, `sendbyte`, `comminkey`,
`turnonxmit` — behind it. `PCBINIT.C` supplies `PcbData` properly. Clark has
an OS/2 modem driver of his own, `pcb153\SOURCE\MODEM\MODEMOS2.C`, which is
what an OS/2 PCBoard would use; FIDOUTIL is a configuration utility and needs
no modem at all.

## The DOS binary changed, by sixteen bytes, and here is why

    before  154,362   sha256 b577da5d4b7422a3a7c995a162bbeec925a07296c0b8cee953ceff74dce2e339
    after   154,346   sha256 737c7e0a9aafb7e0d031877a94a505dbc3b5b54047e8f8cc4804c98744fe192a

This is the **only** change to the DOS output in this whole effort, and it is
item 2 above: two `strcpy` calls replaced by two pointer assignments. The
string-copy code goes away, which is where the sixteen bytes went.

It is a behaviour change, and a deliberate one — the old code wrote through an
uninitialised pointer on startup. If byte-for-byte continuity with the previous
DOS build matters more than the fix, the change can be guarded to the OS/2 arm
and the DOS binary returns to `b577da5d…`. That is a call for the project, not
for the build.
