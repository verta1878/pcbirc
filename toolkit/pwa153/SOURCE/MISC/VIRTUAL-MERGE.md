# VIRTUAL.C is a merged file — what was changed and where

`VIRTUAL.C` and `virtual.h` in the **PWA branches** are not Clark's
originals. They are crew-merged files carrying both of his
implementations behind one `#define`. This note records the change,
which was made without documentation and rediscovered 2026-09-17.

## What Clark shipped

Two independent implementations of one virtual-memory API, in four
files:

| File | Pointers | Record counts | Cache |
|---|---|---|---|
| `VIRTUAL.C` + `VIRTUAL.H` | `VirType huge *` | `long` | yes — `findincache` / `insertincache` |
| `VIRTUAL1.C` + `VIRTUAL1.H` | `VirType *` (near) | `unsigned` | no |

Seven functions each — `getvirtualrec`, `putvirtualrec`,
`updvirtualrec`, `insvirtualrec`, `delvirtualrec`, `openvirtual`,
`freevirtual` — plus `virtualerror`, which takes `void` in one and
`int Code` in the other. **The symbol names are identical**, so a
program links one or the other, never both.

Originals: `reference/pcb153src0014.zip` ->
`PCBSRCV/000/LIB/{SOURCE/MISC,H}/`.

## What the merge did

One `VIRTUAL.C` (14,610 B) and one `virtual.h` (3,949 B), selected at
compile time:

```
#ifdef VIRTUAL_HUGE      -> huge pointers + disk caching (>64KB datasets)
#else   (default)        -> near pointers, memory-only  (the old VIRTUAL1)
#endif
```

Verified against the archive: the merged file's huge-pointer half is
**byte-identical** to Clark's `VIRTUAL.C`. The near-pointer half is
Clark's `VIRTUAL1.C` with one change, `#include <stat.h>` ->
`#include <sys/stat.h>`.

## Where it was applied — PWA branches only

| Branch | VIRTUAL.C | VIRTUAL1.C | virtual.h | VIRTUAL1.H |
|---|---|---|---|---|
| **pwa153** | 14,610 **merged** | 6,207 (see below) | 3,949 **merged** | *absent* |
| **pwa154** | 14,610 **merged** | *absent* | 3,949 **merged** | *absent* |
| delta154 | 8,283 split | 6,345 | 2,283 split | 2,220 |
| irc1541 | 8,325 split | 6,575 | 2,283 split | 2,220 |

`pwa153` and `pwa154` carry **byte-identical** merged files — both the
`.C` and the `.h`. The Watcom branches kept Clark's split intact.

Three things follow from that table, and they are observations, not
guesses:

1. The merge was **deliberate and scoped**, not an accident. It was
   applied to both PWA branches, to both the source and the header, and
   `VIRTUAL1.H` was deleted from both as part of it.
2. It is a **Borland-side** change. The PWA branches are the Borland C++
   3.1 legs; delta154 and irc1541 are the OpenWatcom legs and were left
   alone.
3. `pwa154` has **no `VIRTUAL1.C`** — which is what the merge implies,
   since the merged file already contains it.

## Why — not recorded, and only the crew knows

No commit message, README, or todo entry explains the motivation. What
can be said is what the change buys and costs:

- Identical symbol names in two files mean a build script that compiles
  a whole directory produces duplicate definitions at link. One file
  behind a `#define` cannot collide with itself.
- It removes the risk of the two drifting apart, which is the same
  reasoning applied to the lowercase header copies in
  `MAIN/build/scripts/normalize-case.sh`.
- The cost is that `pwa153` is described elsewhere in this repo as the
  frozen preservation base, and a merged file is not a preserved one.
  Clark's `VIRTUAL.C` and `VIRTUAL.H` no longer exist here as separate
  artifacts; they exist only as halves of a file he did not write.

Those are the plausible readings. **Neither is confirmed** — whoever
made the change should say which, and this note should be updated with
the real answer rather than the inference.

## The open question: pwa153's VIRTUAL1.C

`pwa153/SOURCE/MISC/VIRTUAL1.C` exists; `pwa154`'s does not. That
asymmetry is almost certainly an accident of this project's history, not
of Clark's:

- Commit `e4181e5` (2026-08-25 restructure) deleted it.
- Release v0.3.1 (2026-09-17) restored it, treating the deletion as part
  of the data loss that commit caused, and recorded it as "the ORIGINAL
  Borland version… the only unmodified copy."
- That description was wrong — it carried the `sys/stat.h` change — and
  on 2026-09-17 it was replaced with Clark's archive original,
  CRLF -> LF (6,207 B).

If the deletion in `e4181e5` was **part of the merge**, then v0.3.1
re-introduced a file the merge had deliberately removed, and today's
replacement entrenched it. `pwa154` not having one is the strongest
evidence for that reading.

Nothing in the toolkit builds it either way: `SOURCE/MISC/MAKEFILE`
lists neither `virtual.obj` nor `virtual1.obj` in any target or `tlib`
line. And `ZSWAPVIR.C` — which *is* built — names `VIRTUAL.C` in its
comment as the module whose 5-byte record it swaps. `PCBSM.DSK` in the
archive records the development path as `D:\TC\MISC\VIRTUAL.C`; no
`.DSK` mentions VIRTUAL1.

**Left in place pending a decision.** Removing it is a one-line change
if the merge was meant to supersede it; keeping it costs nothing but the
confusion this note is meant to end.

## WHY HUGE — the caller is PCBFILER  (added 2026-09-22)

One program in the repo uses this API:
`pcb154/MAIN/SOURCE/UTIL/PCBFILER/SAVEDIR.C`. Nothing else calls it —
checked across every branch, excluding `reference/`.

SAVEDIR walks the file-directory list with `VirType huge *p`, indexing the
global `Virtual[]`. That is the case the near implementation cannot serve:

    VIRTUAL1 (near)   VirType *        counts are `unsigned`  -> 65,535 records,
                                       one 64 KB segment, memory only
    VIRTUAL  (huge)   VirType huge *   counts are `long`      -> arrays past
                                       64 KB, plus the disk cache

`huge` is NOT extended memory. In Borland C it is a real-mode segment:offset
pointer that the compiler normalises on every arithmetic step, so one array
can cross segment boundaries. No XMS, EMS, DPMI or 386 needed; it runs on an
8086 under stock DOS. The cost is speed, and the 640 KB conventional-memory
limit is unchanged — huge buys one object bigger than 64 KB, not more memory.
So a file directory with more than 65,535 entries, or more than 64 KB of
them, is a plausible reason the huge version was wanted. **Plausible, not
recorded**: nothing in the tree states the motive.

### The mismatch in SAVEDIR.C

`SAVEDIR.C` line 33 includes `<virtual1.h>` — the NEAR header — and then at
line 491 declares `VirType huge *p`. Against Clark's split pair those
disagree: `VIRTUAL1.H` declares `VirType *Virtual` and near-pointer
prototypes; the huge declarations are in `VIRTUAL.H`. So the one caller asks
for the huge implementation through the near header.

That may be the reason for the merge — one header that serves either
implementation, so an include of either name yields a consistent ABI. Still
an inference.

### Caution for anyone building PCBFILER on a PWA branch

The merged `virtual.h` yields VIRTUAL1's NEAR ABI unless `VIRTUAL_HUGE` is
defined. PCBFILER needs the huge ABI. Built without that define, the
prototypes say near while the code and the caller use huge:

- `Virtual` is declared `VirType *` but indexed as `VirType huge *`
- `openvirtual`/`freevirtual` take `unsigned` instead of `long`

Pointer arithmetic then stops normalising and wraps inside a 64 KB segment —
the exact failure huge exists to prevent, and it is silent at run time rather
than an error at compile time. So `PCBFILER.MAK` must define `VIRTUAL_HUGE`,
or `SAVEDIR.C` must include the huge header explicitly.

Suggested fix, not yet applied: change `SAVEDIR.C` line 33 from
`#include <virtual1.h>` to `#include <virtual.h>`, which is correct against
Clark's split pair in `pcb154/LIB/H/` and, with `VIRTUAL_HUGE` defined,
correct against the merged header too.
