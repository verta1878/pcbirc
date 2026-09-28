# FOSSIL.C — compiled, and what the comparison actually showed

2026-09-22. `toolkit/pwa153/SOURCE/TOOLKIT/FOSSIL.C`, sysop/0's 767-line
reconstruction, compiled under Borland C++ 3.1 for the first time.

    bcc -ml -c -I\BC31\INCLUDE -oFOSSIL.OBJ FOSSIL.C
    0 errors, 2 warnings   ->   FOSSIL.OBJ  9,334 B

## Exports: 79 of 79, exact

Against Clark's recovered `FOSSIL.OBJ` (9,157 B, Oct 11 1993):

    only in ours   0
    only in Clark  0
    in both       79

The earlier 79-symbol match was made by a Python OMF parser reading two
symbol tables. This one was made by a compiler. They now agree, but they
did not start out agreeing — the first build exported **83**.

## Three fixes it took to get there

**1. `REMOTE` was undefined** — errors at lines 640, 662, 679. `Asy.Online`
is stubbed as `int`, but the enum it compares against was never declared.
It lives in `pcb153\SOURCE\H\PCBOARD.H:128`:

    typedef enum {OFFLINE, LOCAL, REMOTE } onlinetype;

Added as three `#define`s with the ordinals preserved. A BC 3.1 enum is
int-wide, so this is ABI-identical to Clark's declaration.

**2. `Asy` and `PcbData` were defined, not declared.** Clark's object
*imports* `_Asy` and `_PcbData` — the program that links FOSSIL.OBJ owns
them. Defining them here produced two extra PUBDEFs and would have
collided with PCBOARD's own definitions at link time, or silently
shadowed them. Both are now `extern`.

**3. `OutBufSize` and `ModemFixupsDone` were global.** Clark's object
neither exports nor imports them, which means they are file-local in the
original. Both are now `static`.

None of these could have been found by reading the source. Each one came
out of the symbol-table diff.

## Imports: still 5 ours-only, 7 Clark-only

The interface is right. The implementation is not yet.

| only in ours | only in Clark |
|---|---|
| `_malloc`, `_free` | `_farmalloc`, `_farfree` |
| `_memcpy` | — |
| `TIMEREXPIRED` | `GETTIMER` |
| `__CDokay` | `_CDokay` |
| — | `ASYNC_INIT`, `ASYNC_OPENCOM`, `ASYNC_TURNONFIFO` |

Three things to work through:

* **Near heap vs far heap.** The file defines `fbmalloc`/`fbfree` as
  `farmalloc`/`farfree` at the top but calls plain `malloc`/`free` in
  `FOSSIL_openmodem` and `FOSSIL_disconnectmodem`. Under `-ml` that is a
  64K buffer allocated from the near heap where Clark took it from far
  memory. This is a behaviour difference, not cosmetic.
* **`__CDokay` vs `_CDokay`** is a one-underscore naming mismatch: ours
  calls a C function named `_CDokay`, Clark's calls one named `CDokay`.
  One of them will not resolve at link.
* **Three ASYNC.ASM calls missing.** Clark's `openmodem` path calls
  `ASYNC_INIT`, `ASYNC_OPENCOM` and `ASYNC_TURNONFIFO`; ours does not.
  The ASYNC fallback path is thinner than the original.

## What "verified" means here, and what it does not

79/79 exports means anything linking against this object sees the same
symbols Clark's object offered. That is the interface contract, and it
holds.

It is not proof the object behaves the same. A far/near allocation
difference does not show up in a symbol table. The next real test is
linking UUXFER — the only program that asks for `fossil.obj` — and
running it.

Clark's `FOSSIL.OBJ` and `COMMDRV.OBJ` stay in `toolkit/pwa153/bc31/obj/`
until the import list matches too. `COMMDRV.OBJ` has no reconstruction at
all yet; the same `MODEM.C` with `-DCOMMDRV` is where it would start.

## Where the two objects live, and why that differs

    toolkit/pwa153/bc31/obj/FOSSIL.OBJ    tracked   Clark's, irreplaceable
    OUT/pwa153/SDK/BC31/OBJ/.../FOSSIL.OBJ  ignored  ours, reproducible

That split is deliberate. `.gitignore` line 3 is `*.OBJ`; the negation
`!toolkit/**/obj/*.OBJ` covers the recovered originals only. A build
product that `bcc` will regenerate does not belong in git.
