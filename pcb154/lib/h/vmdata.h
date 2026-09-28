/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* vmdata.h - shim for the Clark VMData virtual-memory array API.            */
/* Written by: hexadecimal, v0.036.                                          */
/*                                                                           */
/* Types and extern declarations only.  Implementation in VMFUNCS.C.         */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_VMDATA
#define H_VMDATA

#include <alloc.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <mem.h>

typedef struct VMDataSet_tag {
    void far * data;
    long       count;
    long       capacity;
    unsigned   recSize;
} VMDataSet;

typedef int VMBool;
typedef unsigned VMSizeFunc(void far *rec);
typedef int VMCompareFunc(const void *, const void *);
typedef int VMSortFunc(void *base, size_t nel, size_t width,
                       int (*compar)(const void *, const void *));
#define VM_SHINT   int
#define VM_FALSE   0
#define VM_TRUE    1
#define VM_INVALID_POS  (-1L)
#define VM_SEQUENTIAL  0
#define VM_RANDOM      1
typedef void (*VMSwapDisplayFunc)(char *msg);

/* --- Function declarations (implemented in VMFUNCS.C) --- */

#ifdef __cplusplus
extern "C" {
#endif

void VMDataStartUp(char *swapFile, int chunk1, int chunk2, int flag);
void VMDataSwapDisplayFuncSet(VMSwapDisplayFunc fn);
void VMDataShutDown(void);
void VMDataShutDownAtExitSet(int flag);

void VMInitRec(VMDataSet *set, void *ignored1, int ignored2, unsigned recSize);
void far * VMRecordCreate(VMDataSet *set, unsigned recSize,
                          void *ignored1, void *ignored2);
void far * VMRecordGetByIndex(VMDataSet *set, long idx, void *ignored);
void VMRecordChanged(VMDataSet *set);
void VMRecordUnChanged(VMDataSet *set);
long VMRecordCount(VMDataSet *set);
void VMSizeLock(VMDataSet *set);

void VMInitRecVarIdx(VMDataSet *set, void *recBuf, unsigned recBufLen,
                     void *sizeFunc, unsigned tables,
                     void *idxBuf, unsigned idxBufLen);

void VMDone(VMDataSet *set);

void VMSort(VMDataSet *set, unsigned recSize, long start, long cnt,
            int direction,
            VMCompareFunc *compar,
            VMSortFunc *sortFn,
            void *scratchBuf, unsigned scratchLen);

void VMEMSStateSave(void);
void VMEMSStateRestore(void);

void VMAccessAttrSet(VMDataSet *set, int mode);
void VMDebugOn(void);

void far * VMRecordGetByPos(VMDataSet *set, long pos);
void VMWrite(VMDataSet *set, void *data, long pos, unsigned size);

#ifdef __cplusplus
}
#endif

#endif /* H_VMDATA */
