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
        void far * newData = farmalloc(bytes);
        if (newData == NULL) return NULL;
        if (set->data != NULL) {
            _fmemcpy(newData, set->data, (unsigned)(set->count * (long)recSize));
            farfree(set->data);
        }
        set->data     = newData;
        set->capacity = newCap;
    }
    {
        unsigned char far *base = (unsigned char far *) set->data;
        void far *slot = (void far *)(base + (unsigned)(set->count * (long)set->recSize));
        set->count++;
        return slot;
    }
}

void far * VMRecordGetByIndex(VMDataSet *set, long idx, void *ignored)
{
    (void)ignored;
    if (idx < 0 || idx >= set->count) return NULL;
    {
        unsigned char far *base = (unsigned char far *) set->data;
        return (void far *)(base + (unsigned)(idx * (long)set->recSize));
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
    if (set->data != NULL) farfree(set->data);
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
    (void)start; (void)cnt; (void)direction; (void)sortFn;
    (void)scratchBuf; (void)scratchLen;
    if (set->data != NULL && set->count > 1 && compar != NULL) {
        qsort(set->data, (size_t)set->count, (size_t)recSize, *compar);
    }
}

void VMEMSStateSave(void) {}
void VMEMSStateRestore(void) {}

void far * VMRecordGetByPos(VMDataSet *set, long pos)
{
    if (set->data == NULL) return NULL;
    {
        unsigned char far *base = (unsigned char far *) set->data;
        return (void far *)(base + pos);
    }
}

void VMWrite(VMDataSet *set, void *data, long pos, unsigned size)
{
    if (set->data != NULL) {
        unsigned char far *base = (unsigned char far *) set->data;
        _fmemcpy((void far *)(base + pos), (void far *)data, size);
    }
}
