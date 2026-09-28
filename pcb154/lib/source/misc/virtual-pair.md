# VIRTUAL.C and VIRTUAL1.C — both kept here on purpose

`pcb154/LIB/SOURCE/MISC/` holds **both** virtual-memory implementations,
and `pcb154/LIB/H/` holds both headers:

    VIRTUAL.C  + VIRTUAL.H     huge pointers, long record counts, disk cache
    VIRTUAL1.C + VIRTUAL1.H    near pointers, unsigned counts, memory only

This is the only folder in the repo with the pair. It is deliberate, not a
leftover: the pair is kept as Clark shipped it.

They are not duplicates of each other. Same seven-function API
(`getvirtualrec`, `putvirtualrec`, `updvirtualrec`, `insvirtualrec`,
`delvirtualrec`, `openvirtual`, `freevirtual`), incompatible signatures, and
**identical symbol names** — so a program links one or the other, never both.

## How the other branches carry it

| Branch | What is there |
|---|---|
| pwa153, pwa154 | one merged `VIRTUAL.C` + `virtual.h`, both implementations behind `#define VIRTUAL_HUGE`; `VIRTUAL1.*` absent |
| delta154, irc1541 | Clark's split pair, unmerged |
| pcb154/LIB (here) | Clark's split pair, unmerged |

The merge on the two PWA branches was applied to both the source and the
header, and it was applied to **both** PWA branches — pwa153 and pwa154 —
which is why `VIRTUAL1.*` is gone from them. See
`toolkit/pwa153/SOURCE/MISC/VIRTUAL-MERGE.md` for the detail; the merged
file's huge-pointer half is byte-identical to Clark's `VIRTUAL.C`.

**Why the merge was made was never written down.** Recorded here so the next
person does not read this folder as a mistake and "clean it up", and does not
re-merge it. Shipped library evidence: `MISC_L.386` (in
`attic/prebuilt-libs/BC31/`) contains one module named `VIRTUAL` and no
`VIRTUAL1`.

pcbirc, 2026-09-22.
