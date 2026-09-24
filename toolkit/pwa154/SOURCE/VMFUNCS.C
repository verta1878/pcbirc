/*
 * VMFUNCS.C - Compiled implementation of VMDATA functions.
 * Replaces static inline functions in VMDATA.H to avoid
 * "declared but never used" warnings in translation units
 * that include vmdata.h but don't use all functions.
 *
 * Written by: hexadecimal, v0.036
 */

#include <alloc.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <mem.h>

/* Include the struct and type definitions only */
#define VMDATA_TYPES_ONLY
#include <vmdata.h>

void VMDataStartUp(char *swapFile, int chunk1, int chunk2, int flag)
{
    (void)swapFile; (void)chunk1; (void)chunk2; (void)flag;
}

void VMDataSwapDisplayFuncSet(VMSwapDisplayFunc fn) { (void)fn; }
void VMDataShutDown(void) {}
void VMDataShutDownAtExitSet(int flag) { (void)flag; }

void VMInitRec(VMDataSet *set, void *ignored1, int ignored2, unsigned recSize)
{
    (void)ignored1; (void)ignored2;
    set->data     = NULL;
    set->count    = 0;
    set->capacity = 0;
    set->recSize  = recSize;
}

void far * VMRecordCreate(VMDataSet *set, unsigned recSize,
                          void *ignored1, void *ignored2)
{
    (void)ignored1; (void)ignored2;
    if (set->count >= set->capacity) {
        long newCap = set->capacity ? set->capacity * 2 : 256L;
        unsigned long bytes = (unsigned long)newCap * (unsigned long)recSize;
        void * newData = malloc(bytes);
        if (newData == NULL) return NULL;
        if (set->data != NULL) {
            _fmemcpy(newData, set->data, (unsigned)(set->count * (long)recSize));
            free(set->data);
        }
        set->data     = newData;
        set->capacity = newCap;
    }
    {
        unsigned char *base = (unsigned char *) set->data;
        void *slot = (void *)(base + (unsigned)(set->count * (long)set->recSize));
        set->count++;
        return slot;
    }
}

/* VMDATA indices are ONE-BASED.  Every caller in the tree proves it:
 *   MAKEIDX.C 266   for (X = 1; X <= PathNum;  X++)  VMRecordGetByIndex(&Paths,X,..)
 *   MAKEIDX.C 313   for (X = 1; X <= NumFiles; X++)  VMRecordGetByIndex(&Files,X,..)
 *   SORT.C    352   for (Counter = 1; Counter <= Recs; )
 *   CI_BUILD.C 138  same shape
 * and all four VMSort() calls pass start = 1.
 *
 * This used to treat idx as zero-based, which shifted every record by one
 * and returned NULL for the last one (idx == count hit the >= guard).  In
 * PCBSM that is the users file; in MAKEIDX the file index.  Corrected
 * 2026-09-22 -- see MAIN\build\VMDATA-RECONSTRUCTION.md.
 */
void far * VMRecordGetByIndex(VMDataSet *set, long idx, void *ignored)
{
    (void)ignored;
    if (idx < 1 || idx > set->count) return NULL;
    {
        unsigned char *base = (unsigned char *) set->data;
        return (void *)(base + (unsigned)((idx - 1L) * (long)set->recSize));
    }
}

void VMRecordChanged(VMDataSet *set) { (void)set; }
void VMRecordUnChanged(VMDataSet *set) { (void)set; }
long VMRecordCount(VMDataSet *set) { return set->count; }
void VMSizeLock(VMDataSet *set) { (void)set; }

void VMInitRecVarIdx(VMDataSet *set, void *recBuf, unsigned recBufLen,
                     void *sizeFunc, unsigned tables,
                     void *idxBuf, unsigned idxBufLen)
{
    (void)recBuf; (void)recBufLen; (void)sizeFunc; (void)tables;
    (void)idxBuf; (void)idxBufLen;
    set->data     = NULL;
    set->count    = 0;
    set->capacity = 0;
    set->recSize  = 0;
}

void VMDone(VMDataSet *set) {
    if (set->data != NULL) free(set->data);
    set->data = NULL;
    set->count = 0;
    set->capacity = 0;
}

void VMSort(VMDataSet *set, unsigned recSize, long start, long cnt,
            int direction,
            VMCompareFunc *compar,
            VMSortFunc *sortFn,
            void *scratchBuf, unsigned scratchLen)
{
    /* start is ONE-BASED and cnt is a record count -- see the note on
     * VMRecordGetByIndex.  This used to discard both and sort the whole
     * set from offset 0.  sortFn is the caller's sort routine (always
     * qsort in this tree) and scratchBuf was the swap-file staging area,
     * neither of which a resident implementation needs. */
    long first = (start < 1L) ? 0L : start - 1L;
    long num   = cnt;

    (void)direction; (void)sortFn; (void)scratchBuf; (void)scratchLen;

    if (set->data == NULL || compar == NULL) return;
    if (first >= set->count) return;
    if (num <= 0L || first + num > set->count)
        num = set->count - first;
    if (num > 1L) {
        unsigned char *base = (unsigned char *) set->data;
        qsort((void *)(base + (unsigned)(first * (long)recSize)),
              (size_t)num, (size_t)recSize, *compar);
    }
}

/* VMSeqFinalPass - see the note in VMDATA.H.
 *
 * In Clark's VMDATA this closed out a SEQUENTIAL-access pass: the sorted
 * order lived in the swap file and had to be written back before the
 * caller read the records in sequence.  This implementation keeps the
 * whole set resident (VMDataSet is a flat malloc'd array; see VMInitRec)
 * and VMSort() has already reordered it in place with qsort, so there is
 * nothing left to flush.  It therefore validates the set and reports
 * success, in the same spirit as VMSizeLock() and VMRecordChanged()
 * above, which are no-ops here for the same reason.
 *
 * If a swap-file implementation is ever restored, this is where the
 * write-back belongs.  All three call sites in PCBSM's SORT.C discard
 * the return value, so a caller cannot currently tell the difference.
 */
int VMSeqFinalPass(VMDataSet *set)
{
    if (set == NULL)
        return VM_FALSE;
    if (set->count > 0 && set->data == NULL)
        return VM_FALSE;
    return VM_TRUE;
}

void VMEMSStateSave(void) {}
void VMEMSStateRestore(void) {}

void far * VMRecordGetByPos(VMDataSet *set, long pos)
{
    if (set->data == NULL) return NULL;
    {
        unsigned char *base = (unsigned char *) set->data;
        return (void *)(base + pos);
    }
}

void VMWrite(VMDataSet *set, void *data, long pos, unsigned size)
{
    if (set->data != NULL) {
        unsigned char *base = (unsigned char *) set->data;
        _fmemcpy((void *)(base + pos), (void *)data, size);
    }
}

void VMAccessAttrSet(VMDataSet *set, int mode)
{
    (void)set; (void)mode;
    /* Access pattern hint — no-op in in-memory implementation */
}

void VMDebugOn(void)
{
    /* Debug mode — no-op */
}
