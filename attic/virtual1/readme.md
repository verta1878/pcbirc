# attic/virtual1 — the near virtual-memory variant, recovered

Five files deleted by commit **37bf4e4** ("clean os2 build root need work.",
a move commit) and never brought back. Recovered here from `37bf4e4^` on
2026-09-22, byte-for-byte:

    toolkit/delta154/H/VIRTUAL1.H            2,220
    toolkit/delta154/SOURCE/MISC/VIRTUAL1.C  6,345
    toolkit/irc1541/H/VIRTUAL1.H             2,220
    toolkit/irc1541/SOURCE/MISC/VIRTUAL1.C   6,575
    toolkit/pwa153/SOURCE/MISC/VIRTUAL1.C    6,207

Each sits under the path it had in the tree.

## What VIRTUAL1 is

Clark shipped two implementations of one virtual-memory API, with the same
seven function names, so a program links one or the other:

| | VIRTUAL1 (here) | VIRTUAL |
|---|---|---|
| pointers | `VirType *` (near) | `VirType huge *` |
| record counts | `unsigned` — about 13,100 records at 5 bytes each, one 64 KB segment | `long` |
| backing | memory only | memory plus a disk cache |

## Why it is in the attic and not in the branches

Nothing builds it, and the branches that lost it do not need it:

- **delta154 / irc1541** carry Clark's split **huge** `virtual.h`, which is
  what their one caller wants. PCBFILER's `SAVEDIR.C` declares
  `VirType huge *p`; on 2026-09-22 its include was pointed at `<virtual.h>`
  for exactly this reason — `virtual1.h` was not there to include.
- **pwa153 / pwa154** carry the crew's merged `VIRTUAL.C` + `virtual.h`,
  which contain both implementations behind `#define VIRTUAL_HUGE`. The
  merged file's near half *is* VIRTUAL1 (one change: `<stat.h>` ->
  `<sys/stat.h>`), so a separate VIRTUAL1.C there is a second copy of code
  the merged file already holds. See `toolkit/pwa153/SOURCE/MISC/VIRTUAL-MERGE.md`.
- **`SOURCE/MISC/MAKEFILE`** lists neither `virtual.obj` nor `virtual1.obj`
  in any target or `tlib` line, in any branch.
- The shipped `MISC_L.386` (in `attic/prebuilt-libs/BC31/`) contains one
  module named `VIRTUAL` and no `VIRTUAL1`.

**`pcb154/LIB/` is untouched** and still holds Clark's complete split pair —
`VIRTUAL.C` + `VIRTUAL1.C`, `VIRTUAL.H` + `VIRTUAL1.H`. That is the
preservation copy; see `pcb154/LIB/SOURCE/MISC/VIRTUAL-PAIR.md`.

## Bringing one back

Copy the file to its original path. If it goes back into a branch whose
`virtual.h` is the merged header, the two will declare the same symbols with
different signatures — build one or the other, never both.
