/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* VMAVL.C — AVL Tree for Virtual Memory DataSets                           */
/*                                                                           */
/* Clean room implementation. Algorithm: depth-based AVL with parent         */
/* pointers, single and double rotations. Studied libavl by Buselli/Dankers  */
/* (LGPL) for algorithmic approach; this code is original.                   */
/*                                                                           */
/* Author: sysop/0                                                           */
/* License: GPLv3 (pcbrevival project)                                       */
/* pcbrevival Phase 0, August 2026                                          */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdlib.h>
#include <string.h>

#define VMDATA_TYPES_ONLY
#include <vmdata.h>
#include <vmavl.h>

static VMSwapFailureHandler gSwapHandler = NULL;

/*-----------------------------------------------------------------------*/
/* Access helpers — VMAVLInfo is at byte offset 0 of each VMDataSet rec  */
/*-----------------------------------------------------------------------*/

static VMAVLInfo *nodeAt(VMAVLControl *ctrl, long pos) {
    return (VMAVLInfo *)VMRecordGetByIndex(ctrl->dataSet, pos, NULL);
}

static void *recAt(VMAVLControl *ctrl, long pos) {
    return (void *)VMRecordGetByIndex(ctrl->dataSet, pos, NULL);
}

/*-----------------------------------------------------------------------*/
/* Depth helpers                                                          */
/*-----------------------------------------------------------------------*/

static unsigned char depthOf(VMAVLControl *ctrl, long pos) {
    if (pos == VM_INVALID_POS) return 0;
    return nodeAt(ctrl, pos)->depth;
}

static void updateDepth(VMAVLControl *ctrl, long pos) {
    VMAVLInfo *n;
    unsigned char ld, rd;
    if (pos == VM_INVALID_POS) return;
    n = nodeAt(ctrl, pos);
    ld = depthOf(ctrl, n->left);
    rd = depthOf(ctrl, n->right);
    n->depth = (unsigned char)((ld > rd ? ld : rd) + 1);
}

/* Balance: positive = right-heavy, negative = left-heavy */
static int balanceOf(VMAVLControl *ctrl, long pos) {
    VMAVLInfo *n;
    if (pos == VM_INVALID_POS) return 0;
    n = nodeAt(ctrl, pos);
    return (int)depthOf(ctrl, n->right) - (int)depthOf(ctrl, n->left);
}

/*-----------------------------------------------------------------------*/
/* Superparent — pointer to the slot that points to 'pos'                */
/* Either parent->left, parent->right, or tree->root                     */
/*-----------------------------------------------------------------------*/

static long *superOf(VMAVLTree *tree, VMAVLControl *ctrl, long pos) {
    VMAVLInfo *n = nodeAt(ctrl, pos);
    long par = n->parent;
    if (par == VM_INVALID_POS)
        return &tree->root;
    else {
        VMAVLInfo *p = nodeAt(ctrl, par);
        return (p->left == pos) ? &p->left : &p->right;
    }
}

/*-----------------------------------------------------------------------*/
/* Rotations                                                              */
/*-----------------------------------------------------------------------*/

static void rotateRight(VMAVLTree *tree, VMAVLControl *ctrl, long pos) {
    VMAVLInfo *x, *y;
    long ypos;
    long *sp;

    x = nodeAt(ctrl, pos);
    ypos = x->left;
    y = nodeAt(ctrl, ypos);
    sp = superOf(tree, ctrl, pos);

    /* x.left = y.right */
    x->left = y->right;
    if (x->left != VM_INVALID_POS)
        nodeAt(ctrl, x->left)->parent = pos;

    /* y.right = x */
    y->right = pos;
    y->parent = x->parent;
    x->parent = ypos;

    /* fix parent's child pointer */
    *sp = ypos;

    updateDepth(ctrl, pos);
    updateDepth(ctrl, ypos);
}

static void rotateLeft(VMAVLTree *tree, VMAVLControl *ctrl, long pos) {
    VMAVLInfo *x, *y;
    long ypos;
    long *sp;

    x = nodeAt(ctrl, pos);
    ypos = x->right;
    y = nodeAt(ctrl, ypos);
    sp = superOf(tree, ctrl, pos);

    /* x.right = y.left */
    x->right = y->left;
    if (x->right != VM_INVALID_POS)
        nodeAt(ctrl, x->right)->parent = pos;

    /* y.left = x */
    y->left = pos;
    y->parent = x->parent;
    x->parent = ypos;

    /* fix parent's child pointer */
    *sp = ypos;

    updateDepth(ctrl, pos);
    updateDepth(ctrl, ypos);
}

/*-----------------------------------------------------------------------*/
/* Rebalance — walk from node up to root, rotating where needed          */
/*-----------------------------------------------------------------------*/

static void rebalance(VMAVLTree *tree, VMAVLControl *ctrl, long pos) {
    long cur;
    long par;

    cur = pos;
    while (cur != VM_INVALID_POS) {
        int bal;
        VMAVLInfo *n;

        par = nodeAt(ctrl, cur)->parent;
        updateDepth(ctrl, cur);
        bal = balanceOf(ctrl, cur);

        if (bal < -1) {
            /* left-heavy */
            n = nodeAt(ctrl, cur);
            if (balanceOf(ctrl, n->left) > 0)
                rotateLeft(tree, ctrl, n->left);  /* left-right case */
            rotateRight(tree, ctrl, cur);
        } else if (bal > 1) {
            /* right-heavy */
            n = nodeAt(ctrl, cur);
            if (balanceOf(ctrl, n->right) < 0)
                rotateRight(tree, ctrl, n->right); /* right-left case */
            rotateLeft(tree, ctrl, cur);
        }

        cur = par;
    }
}

/*-----------------------------------------------------------------------*/
/* Public API                                                             */
/*-----------------------------------------------------------------------*/

void VMAVLControlInit(VMDataSet *set, VMAVLControl *ctrl,
                      VMAVLCompareFunc compare, int flags) {
    ctrl->dataSet = set;
    ctrl->compare = compare;
    ctrl->recSize = 0;
    ctrl->flags = flags;
}

void VMAVLTreeInit(VMAVLTree *tree, int flags) {
    tree->root = VM_INVALID_POS;
    tree->count = 0;
    tree->flags = flags;
}

int VMAVLAdd(VMAVLTree *tree, VMAVLControl *ctrl, long pos) {
    VMAVLInfo *nn;
    long cur;
    int cmp;
    VMAVLInfo *cn;

    nn = nodeAt(ctrl, pos);
    nn->left = VM_INVALID_POS;
    nn->right = VM_INVALID_POS;
    nn->depth = 1;

    if (tree->root == VM_INVALID_POS) {
        tree->root = pos;
        nn->parent = VM_INVALID_POS;
        tree->count = 1;
        return 1;
    }

    /* BST insert */
    cur = tree->root;
    while (1) {
        cmp = ctrl->compare(recAt(ctrl, pos), recAt(ctrl, cur));
        cn = nodeAt(ctrl, cur);
        if (cmp <= 0) {
            if (cn->left == VM_INVALID_POS) {
                cn->left = pos;
                nn->parent = cur;
                break;
            }
            cur = cn->left;
        } else {
            if (cn->right == VM_INVALID_POS) {
                cn->right = pos;
                nn->parent = cur;
                break;
            }
            cur = cn->right;
        }
    }

    tree->count++;
    rebalance(tree, ctrl, cur);
    return 1;
}

long VMAVLSearch(VMAVLTree *tree, VMAVLControl *ctrl,
                 const void *key, int addIfNotFound, int *added) {
    long cur;
    int cmp;
    VMAVLInfo *cn;
    void far *newRec;
    long newPos;
    unsigned recSize;

    cur = tree->root;
    if (added) *added = 0;

    while (cur != VM_INVALID_POS) {
        cmp = ctrl->compare(key, recAt(ctrl, cur));
        if (cmp == 0) return cur;
        cn = nodeAt(ctrl, cur);
        cur = (cmp < 0) ? cn->left : cn->right;
    }

    if (addIfNotFound) {
        recSize = ctrl->dataSet->recSize;
        newRec = VMRecordCreate(ctrl->dataSet, recSize, NULL, NULL);
        if (newRec) {
            newPos = VMRecordCount(ctrl->dataSet) - 1;
            memcpy((void *)newRec, key, recSize);
            VMAVLAdd(tree, ctrl, newPos);
            if (added) *added = 1;
            return newPos;
        }
    }
    return VM_INVALID_POS;
}

long VMAVLFirstGet(VMAVLTree *tree, VMAVLControl *ctrl) {
    long cur;

    cur = tree->root;
    if (cur == VM_INVALID_POS) return VM_INVALID_POS;
    while (nodeAt(ctrl, cur)->left != VM_INVALID_POS)
        cur = nodeAt(ctrl, cur)->left;
    return cur;
}

void VMAVLWalkContextInit(VMAVLTree *tree, VMAVLControl *ctrl,
                          VMAVLWalkContext *ctx, long startPos) {
    (void)tree; (void)ctrl;
    ctx->current = startPos;
    ctx->stackTop = 0;
    ctx->initialized = 1;
}

long VMAVLNextGet(VMAVLTree *tree, VMAVLControl *ctrl,
                  VMAVLWalkContext *ctx) {
    long cur, child, par;
    VMAVLInfo *n;

    (void)tree;
    if (!ctx->initialized) return VM_INVALID_POS;

    cur = ctx->current;
    if (cur == VM_INVALID_POS) return VM_INVALID_POS;

    n = nodeAt(ctrl, cur);

    /* If right subtree exists: go right, then leftmost */
    if (n->right != VM_INVALID_POS) {
        cur = n->right;
        while (nodeAt(ctrl, cur)->left != VM_INVALID_POS)
            cur = nodeAt(ctrl, cur)->left;
        ctx->current = cur;
        return cur;
    }

    /* Else walk up until we're a left child */
    child = cur;
    par = n->parent;
    while (par != VM_INVALID_POS) {
        if (nodeAt(ctrl, par)->left == child) {
            ctx->current = par;
            return par;
        }
        child = par;
        par = nodeAt(ctrl, par)->parent;
    }

    ctx->current = VM_INVALID_POS;
    return VM_INVALID_POS;
}

void VMDataSwapFailureHandlerSet(VMSwapFailureHandler handler) {
    gSwapHandler = handler;
}
