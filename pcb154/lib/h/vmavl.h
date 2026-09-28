/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* vmavl.h — AVL Tree for Virtual Memory DataSets                           */
/*                                                                           */
/* Clean room implementation. Depth-based AVL with parent pointers.          */
/* Studied libavl (Buselli/Dankers, LGPL) for algorithm; code is original.  */
/*                                                                           */
/* Author: sysop/0                                                           */
/* License: GPLv3 (pcbrevival project)                                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_VMAVL
#define H_VMAVL

#include <vmdata.h>

/* Node info stored at the start of each record in VMDataSet */
typedef struct {
    long          left;     /* position of left child (VM_INVALID_POS = none) */
    long          right;    /* position of right child */
    long          parent;   /* position of parent node */
    unsigned char depth;    /* subtree depth (1 = leaf) */
    char          pad[3];   /* pad to 16 bytes */
} VMAVLInfo;

/* Compare function: returns <0, 0, >0 */
typedef VM_SHINT (*VMAVLCompareFunc)(const void *, const void *);

/* Control block — binds a VMDataSet to a compare function */
typedef struct {
    VMDataSet        *dataSet;
    VMAVLCompareFunc  compare;
    unsigned          recSize;
    int               flags;
} VMAVLControl;

/* Tree root */
typedef struct {
    long root;
    long count;
    int  flags;
} VMAVLTree;

/* Walk (iteration) context */
typedef struct {
    long current;
    long stack[64];
    int  stackTop;
    int  initialized;
} VMAVLWalkContext;

/* Swap failure types (used by PCBFILER) */
typedef enum {
    VM_SWAP_FULL = 1,
    VM_SWAP_OPEN_FAILED = 2
} VMDataSwapFailure;

typedef enum {
    VM_ABORT = 0,
    VM_MURPHY = 1,
    VM_SANITY_CHECK = 2,
    VM_DEBUG = 3,
    VM_CREATE_NEW = 4
} VMDataSwapFailureAction;

typedef VMDataSwapFailureAction (*VMSwapFailureHandler)(VMDataSwapFailure, char *);

#ifdef __cplusplus
extern "C" {
#endif

void VMAVLControlInit(VMDataSet *set, VMAVLControl *ctrl,
                      VMAVLCompareFunc compare, int flags);
void VMAVLTreeInit(VMAVLTree *tree, int flags);
int  VMAVLAdd(VMAVLTree *tree, VMAVLControl *ctrl, long pos);
long VMAVLSearch(VMAVLTree *tree, VMAVLControl *ctrl,
                 const void *key, int addIfNotFound, int *added);
long VMAVLFirstGet(VMAVLTree *tree, VMAVLControl *ctrl);
void VMAVLWalkContextInit(VMAVLTree *tree, VMAVLControl *ctrl,
                          VMAVLWalkContext *ctx, long startPos);
long VMAVLNextGet(VMAVLTree *tree, VMAVLControl *ctrl,
                  VMAVLWalkContext *ctx);
void VMDataSwapFailureHandlerSet(VMSwapFailureHandler handler);

#ifdef __cplusplus
}
#endif

#endif /* H_VMAVL */
