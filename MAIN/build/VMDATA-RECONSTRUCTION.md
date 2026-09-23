# VMDATA — what we have, what is reconstructed, and the off-by-one

`toolkit/pwa153/SOURCE/VMFUNCS.C` is **not Clark's code**. Its own header
says "Written by: hexadecimal". Clark's virtual-memory library shipped as
`\LIBS\VMDATA\BC31_DOS\VMDATA.LIB`, which this repo has never had.
`VMFUNCS.C` is a crew reimplementation written to get MAKEIDX linking,
and `VMAVL.C` beside it is the AVL tree.

Built by hand into `OUT\PWA153\SDK\BC31\LIB\VMDATA_L.LIB` — two modules,
`VMAVL` and `VMFUNCS`.

## Who uses it

| Program | Calls |
|---|---|
| MAKEIDX | `VMRecordCreate`, `VMRecordGetByIndex`, `VMSort` |
| PCBSM | the above plus `VMDataStartUp`, `VMDataSwapDisplayFuncSet`, `VMDone`, `VMInitRec`, `VMSizeLock`, `VMSeqFinalPass` |
| PCBSETUP | `VMRecordCreate`, `VMRecordGetByIndex`, `VMSort` (CI_BUILD.C) |
| FIDOUTIL | via `CI_BUILD.C` |

## The off-by-one — fixed 2026-09-22

**Clark's VMDATA indices are one-based.** Every caller in the tree says
so, and they are all Clark's own code:

    MAKEIDX.C  266   for (X = 1; X <= PathNum;  X++)  VMRecordGetByIndex(&Paths,X,NULL)
    MAKEIDX.C  313   for (X = 1; X <= NumFiles; X++)  VMRecordGetByIndex(&Files,X,NULL)
    SORT.C     352   for (Counter = 1; Counter <= Recs; )
    CI_BUILD.C 138   Rec = VMRecordGetByIndex(&CnamesIdxSet,i,NULL)

and all four `VMSort()` call sites pass `start` as **1**.

`VMFUNCS.C` treated the index as **zero-based**:

    if (idx < 0 || idx >= set->count) return NULL;
    return base + idx * recSize;

So every record came back shifted by one, and **the last record returned
NULL** — at `idx == count` the `>=` guard rejected it. In PCBSM that is
the users file being sorted; in MAKEIDX the file index. `VMSort` made it
worse by discarding `start` and `cnt` entirely and sorting the whole set
from offset 0.

This had already shipped: `MAKEIDX.EXE` built, ran, and was recorded as
good. It builds and runs — it just indexes wrong. Same lesson as the one
already in `APPLY.txt`: a count is not a verification.

**The fix keeps Clark's callers untouched and corrects the library**,
because the callers are the evidence of the contract:

* `VMRecordGetByIndex` — `if (idx < 1 || idx > set->count) return NULL;`
  then `base + (idx - 1) * recSize`.
* `VMSort` — sorts `cnt` records starting at `start - 1`, clamped to the
  set, instead of the whole array.

The alternative — editing `MAKEIDX.C`, `SORT.C` and `CI_BUILD.C` to be
zero-based — was rejected: it would mean changing Clark's source to match
a reimplementation's accident.

## VMSeqFinalPass — added 2026-09-22

Called three times by `SORT.C` (lines 346, 444, 536), always right after
`VMSort()` and right before the records are written out. It is **declared
nowhere** in this repo except a `#ifdef __WATCOMC__` block in
`pcb153\SOURCE\UTIL\PCBSM\SOURCE\pcbsm_externs.h`:

    extern int (*VMSeqFinalPass)(void *);

That block is the only surviving evidence of the signature — returns
`int`, takes the `VMDataSet`. It is not in `VMDATA.H`'s 28 declarations,
not in the reference PWA source, and not in Clark's own copy of `SORT.C`
at `reference\pcball\pcboard\pcb-util\PCBSM\SOURCE\SORT.C`, which calls
it just the same.

In Clark's VMDATA this closed out a **sequential** pass: sorted order
lived in the swap file and had to be written back before the caller read
the records in sequence. This implementation keeps the whole set resident
(`VMDataSet` is a flat `malloc`'d array — see `VMInitRec`) and `VMSort()`
has already reordered it in place, so there is nothing to flush. It
validates the set and returns `VM_TRUE`.

All three call sites discard the return value, so a caller cannot
currently tell the difference. **If a swap-file implementation is ever
restored, the write-back belongs here.**

## The no-ops, and why each one is a no-op

These are empty on purpose, not unfinished by accident. Every one of them
only means something once there is a backing store:

| Function | Why it is empty here |
|---|---|
| `VMDataStartUp` | takes `swapFile`, `chunk1`, `chunk2` and discards all three — there is no swap file |
| `VMDataShutDown`, `VMDataShutDownAtExitSet` | nothing to tear down |
| `VMDataSwapDisplayFuncSet` | the progress callback exists to show swapping; nothing swaps |
| `VMRecordChanged`, `VMRecordUnChanged` | dirty tracking only matters when pages are written back |
| `VMSizeLock` | size locking guards against the store growing under a pass |
| `VMEMSStateSave`, `VMEMSStateRestore` | no EMS is used |

## The real limit — no virtual memory at all

`VMDataStartUp` ignoring the swap file is not a detail. `VMDataSet` is a
flat `malloc`'d array that doubles when full (`VMRecordCreate`), so the
whole set must fit in the DOS 640K real-mode heap.

Clark's VMDATA existed precisely so that PCBSM could sort a users file
larger than RAM. **Until a paging store is written, PCBSM sorts only
boards small enough to fit in memory**, and it will fail on a large one
rather than swapping.

Finishing that is a scoped piece of work in its own right: a swap-file
and EMS-backed store behind the same API, with a memory-ceiling test as
the acceptance criterion. The seven no-ops above become live the moment
it exists, and not before.

## Verified

After the fix, rebuilt and relinked clean with Borland C++ 3.1:

    VMDATA_L.LIB   6,656 B   VMAVL + VMFUNCS
    MAKEIDX.EXE   29,138 B   0 errors
    PCBSM.EXE    388,496 B   0 errors
    MKPCBTXT.EXE  74,336 B   0 errors

A compile and a link are not proof that the indexing is right — they
never were. The evidence for one-based is the four call sites above.
