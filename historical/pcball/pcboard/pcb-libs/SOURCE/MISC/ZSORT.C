/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*
#include <alloc.h>
#include <screen.h>
*/

#include "zsort.h"
#ifdef DEBUG
  #include <process.h>
  #include <stdio.h>
  #include <memcheck.h>
#endif

#define SIZE 50
#define SWAP(a,b) (cswap((a),(b)))
#define T  5                         /* subfiles of T or fewer elements will */
                                     /* be sorted by a simple insertion sort */

unsigned zsort_width;

void pascal zsort(char huge *base, long nel, unsigned width, int (*comp)(const void huge *a, const void huge *b), void (*cswap)(char huge *a, char huge *b)) {
#ifdef DEBUG
   int  stackcount;
#endif
   char huge *stack[SIZE], huge **sp;     /* stack and stack pointer        */
   char huge *i, huge *j, huge *limit;    /* scan and limit pointers        */
   unsigned thresh;                       /* size of T elements in bytes    */

   zsort_width = width;                   /* save width in case needed by   */
                                          /* an external swap function      */
#ifdef DEBUG
   stackcount = 0;
#endif

   thresh = T * width;                    /* init threshold (T must be > 2) */
   sp = stack;                            /* init stack pointer             */
   limit = base + ((long) nel * width);   /* pointer past end of array      */
   for ( ;; ) {
      if ( limit - base > thresh ) {      /* if more than T elements        */
         SWAP((((limit-base)/width)/2)*width+base, base);/*swap middle, base*/
         i = base + width;
         j = limit - width;

         if ((*comp)(i,j) > 0)            /* Sedgewick's                    */
            SWAP(i,j);                    /*    three-element sort          */
                                          /*        sets things up          */
         if ((*comp)(base,j) > 0)         /*            so that             */
            SWAP(base,j);                 /*              *i <= *base <= *j */
                                          /* *base is the pivot element     */
         if ((*comp)(i,base) > 0)
            SWAP(i,base);

/****************************************************************************
*    only needed if DUPLICATE keys in sort (???)                            *
*****************************************************************************
*        if ((*comp)(i,j) == 0) {                                           *
*           do {                                                            *
*              i += width;                                                  *
*              j -= width;                                                  *
*           } while ((*comp)(i,j) == 0 && i < j);                           *
*           i -= width;                                                     *
*           j += width;                                                     *
*        }                                                                  *
*****************************************************************************/

         for ( ;; ) {                     /* loop until break               */
            do
               i += width;
            while ((*comp)(i,base) < 0);
            do
               j -= width;
            while ((*comp)(j,base) > 0);
            if (i > j)
               break;
            SWAP(i,j);
         }
         SWAP(base, j);                   /* move pivot into correct place  */
         if ( j - base > limit - i ) {    /* if left subfile is larger...   */
            sp[0] = base;                 /* stack left subfile base & limit*/
            sp[1] = j;
            base = i;                     /* sort the right subfile         */
         } else {                         /* else right subfile is larger   */
            sp[0] = i;                    /* stack right subfile base & lim */
            sp[1] = limit;
            limit = j;                    /* sort the left subfile          */
         }
         sp += 2;                         /* bump stack pointer             */
#ifdef DEBUG
         stackcount += 2;
         if (sp >= &stack[SIZE]) {
           printf("\n%p %p %d",sp,&stack[SIZE],stackcount);
           printf("\nstack overflow ... error!\n");
           exit(99);
         }
#endif
      } else {                /* else subfile is small, use insertion sort  */
         for ( j = base, i = j + width; i < limit; j = i, i += width )
            for (; j >= base && (*comp)(j,j+width) > 0; j-=width)
               SWAP(j,j+width);
         if ( sp != stack ) {             /* if any entries on stack...     */
            sp -= 2;                      /* pop the base and limit         */
#ifdef DEBUG
            stackcount -= 2;
            if (sp < stack) {
              printf("stack underflow ... error!\n");
              exit(99);
            }
#endif
            base = sp[0];
            limit = sp[1];
         } else                           /* else stack empty, all done     */
            break;
      }
   }
}

