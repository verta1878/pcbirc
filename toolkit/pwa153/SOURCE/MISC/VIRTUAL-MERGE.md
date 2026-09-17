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
