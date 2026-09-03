# Compile Status — Final

## DOS Extender

PCBoard 15.3/15.4 = 16-bit real-mode DOS, no extender.
QFront = DOS/32A (Phase 14, complete).

## Compile Flags

Watcom: `wcc -ml -2 -za99 -dPCB152 -dLIB -dCOMM -fi=WATCOMPAT.H`
Borland: `BCC -c -P -ml -DPCB152 -DLIB -DCOMM` (needs TASM in PATH)

## Results

| Target | Compiler | Toolkit | DOS | OS/2 |
|---|---|---|---|---|
| 15.4 Delta | Watcom wcc | irc1541 | **267/267 (100%)** | 6/6 (separate target) |
| 15.3 PWA | Borland BC31 | pwa153 | **270/270 (100%)** | 2/2 (separate target) |

## Fixes Applied

### Both toolkits (irc1541 + pwa153)

| Fix | File | What | Why |
|---|---|---|---|
| bool typedef | TYPES.HPP | removed `#ifndef __cplusplus` guard | BC31 is pre-C++98, no built-in bool |
| _FARDATA_ | TYPES.HPP | added definition for Borland (far) and Watcom (empty) | not defined outside WATCOMPAT.H |
| VIRTUAL1.H include | VIRTUAL1.C | changed `#include "virtual.h"` to `"virtual1.h"` | virtual.h has huge*, virtual1.h has * — must match source |
| explicit casts | VIRTUAL1.C | `(char *)malloc()`, `(VirType *)malloc()` | BC31 C++ mode rejects void* implicit conversion |
| setup.h path | CHKEXIST.C | `\proj\pcbsetup\source\setup.h` → `"setup.h"` | Clark's dev machine path |
| getconfrecord | PCB.H + CNAMES.C | removed duplicate void declarations, aligned return types | two conflicting declarations |
| closecnames | PCB.H (pwa153) | removed duplicate LIBENTRY declaration | atexit() needs C calling convention |

### Watcom only (irc1541)

| Fix | File | What | Why |
|---|---|---|---|
| enum-to-int | PCBTOOLS.H | `__WATCOMC__` added to int path | Watcom rejects enum as function param type |
| waitforkey | PCBOARD.H | guarded overload with `__cplusplus` | C doesn't allow function overloading |
| bgetkey2 | PCBOARD.H | guarded `extern "C"` with `__cplusplus` | extern "C" is C++ syntax |
| inputattrtype | SCRNIO.H | `#define inputattrtype int` for Watcom | enum-to-int, same as PCBTOOLS.H |
| Country files | 5 × COUNTRY/*.C | `extern "C"` guarded | C++ syntax in C files |
| _AX/_DX | DBL_LONG.C, PR_LONG.C | renamed to regAX/regDX | _AX/_DX are Watcom pseudo-registers |
| bsearch | PSEARCH.C | renamed to pcb_bsearch | conflicts with stdlib bsearch |
| huge pointers | ZSWAPSTR.C, ZSWAPVIR.C | added `huge` to match ZSORT.H | header declares huge*, source had * |
| NOINPUT stubs | NOINPUT.C | `displaytype` → `DISPLAYTYPE` | enum mapped to int for Watcom |
| NOMEMORY stub | NOMEMORY.C | `int` → `unsigned` for bmalloc | match PCBTOOLS.H declaration |
| NOSHELL stub | NOSHELL.C | added 4 missing params | 15.4 performshell has 7 params |
| read120file | DATA120.C | `pascal` → `LIBENTRY` | calling convention mismatch |
| CPUTYPE | CPUTYPE.C | rewritten with Watcom `_asm` blocks | Borland asm syntax incompatible |
| ANSI | ANSI.C | ansi_print rewritten using `intdos()` | Borland asm labels unsupported in Watcom |
| FONT_8x8 | WATCOMPAT.H | defined FONT_8x8/FONT_8x14 | Borland graphics constant |
| -2 flag | compile flags | added `-2` (286 instructions) | SMSW is 286+, default is 8086 |
| -dLIB | compile flags | added | SysLimit struct field inside `#ifdef LIB` |
| -dCOMM | compile flags | added | CDokay and waitforempty inside `#ifdef COMM` |

### Borland only (pwa153)

| Fix | File | What | Why |
|---|---|---|---|
| AUTO enum | pcb153 PCBOARD.H | `AUTO=32768` → `AUTO=-32768` (Borland path only) | BC31 16-bit int overflow. Same bit pattern 0x8000. |
| CONFFUNC.C | CONFFUNC.C | moved `#pragma inline` after comment block | asm keywords inside `/* */` comment confused BCC parser |
| -DCOMM | compile flags | added | toolkit references COMM functions (cdstillup, waitforempty) |

## OS/2 Files (separate build target)

Compile with `-d__OS2__ -IOS2TK/H` added to flags.

Watcom: HANDLERS.C, DELAY.C, GIVEUP.C, THREADS.C, COUNTRY.C, TEST.C
Borland: TEST.C, THREADS.C

## Deferred — VIRTUAL.H / VIRTUAL1.H merge

Two files, same API, different pointer sizes. Merge planned but deferred.

**Design (ready to apply):**
- Merge into one VIRTUAL.H with `#ifdef VIRTUAL_HUGE` selecting pointer size
- VIRTUAL_HUGE = huge pointers, long counts (>64KB arrays)
- Default (no define) = standard pointers, unsigned counts (<64KB)
- VIRTUAL.C defines `VIRTUAL_HUGE` at the top (it uses huge pointers)
- VIRTUAL1.C does not define it (standard pointers)
- VIRTUAL1.H becomes a one-line redirect: `#include "virtual.h"`
- Add `#ifndef _VIRTUAL_H_` include guard
- Both `getvirtualptr` macro variants inside the ifdef
- `extern "C"` guard for C++ compatibility
