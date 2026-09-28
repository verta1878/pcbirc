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


#include <malloc.h>
#include "project.h"
#pragma hdrstop

// #include <search.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
  #ifndef __WATCOMC__
    #define farmalloc _fmalloc
    #define farfree   _ffree
  #endif
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#if defined(__OS2__)
  #define farmalloc(x) malloc(x)
  #define farfree(x)   free(x)
#endif

#ifdef DEBUGMEM
#ifdef __OS2__
  #define coreleft()    0
  #define farcoreleft() 0
#endif
#endif

#define bMAX   200
#define fbMAX   10

typedef struct {
  void *Ptr;
  #ifdef DEBUGMEM
    int      AllocNum;
    unsigned Size;
  #endif
} baseptr;

typedef struct {
  void _FAR_ *Ptr;
  #ifdef DEBUGMEM
    int      AllocNum;
    unsigned Size;
  #endif
} farbaseptr;

baseptr    static _FARDATA_ bArray[bMAX];
farbaseptr static _FARDATA_ fbArray[fbMAX];


int    static  bNext  = 0;  /* The "next" entry to be used in the array */
int    static  fbNext = 0;  /* The "next" entry to be used in the array */

int    static  bTotal  = 0; /* The "total" number of entries in the array */
int    static  fbTotal = 0; /* The "total" number of entries in the array */

#ifdef DEBUGMEM
int    static  bHighest   = 0;
int    static  fbHighest  = 0;
int    static  bAllocNum  = 0;
int    static  fbAllocNum = 0;
#endif


static int sortbaseptr(const void *Rec1, const void *Rec2) {
  baseptr *a = (baseptr *) Rec1;
  baseptr *b = (baseptr *) Rec2;
  long     Temp;

  checkstack();
  if (a->Ptr == NULL) return(1);    /* sort NULL to the end */
  if (b->Ptr == NULL) return(-1);   /* sort NULL to the end */

  Temp = (long) a->Ptr - (long) b->Ptr;
  if (Temp > 0)
    return(1);
  if (Temp < 0)
    return(-1);
  return(0);
}


static int sortfarbaseptr(const void *Rec1, const void *Rec2) {
  farbaseptr *a = (farbaseptr *) Rec1;
  farbaseptr *b = (farbaseptr *) Rec2;
  long        Temp;

  checkstack();
  if (a->Ptr == NULL) return(1);    /* sort NULL to the end */
  if (b->Ptr == NULL) return(-1);   /* sort NULL to the end */

  Temp = (long) a->Ptr - (long) b->Ptr;
  if (Temp > 0)
    return(1);
  if (Temp < 0)
    return(-1);
  return(0);
}


void LIBENTRY initbmalloc(void) {
  checkstack();
  memset(bArray,0,sizeof(bArray));
  bTotal = bNext = 0;
  #ifdef DEBUGMEM
    bHighest = bAllocNum = 0;
  #endif

  memset(fbArray,0,sizeof(fbArray));
  fbTotal = fbNext = 0;
  #ifdef DEBUGMEM
    fbHighest = fbAllocNum = 0;
  #endif
}


void * LIBENTRY bmalloc(unsigned Size) {
  void *p;
  #ifdef DEBUGMEM
    char Str[80];
  #endif

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK) {
        writedebugrecord("Error in bmalloc() - HEAP CORRUPT");
        return(NULL);
      }
    #endif
  #endif

  checkstack();
  if (bTotal >= bMAX) {
    writedebugrecord("Error in bmalloc() - INCREASE BMALLOC");
    return(NULL);
  }

  if ((p = malloc(Size)) == NULL) {
    #ifdef DEBUGMEM
      if (DebugLevel >= 6) {
        sprintf(Str,"Insuff Memory for %u (left: %ld)",Size,coreleft());
        writedebugrecord(Str);
      }
    #endif
    return(NULL);
  }

  #ifdef DEBUGMEM
    /* initialize allocated memory to 0xB4 - meaning "BEFORE" so that if we */
    /* run across uninitialized memory the B4's will stand out as obvious   */
    memset(p,0xB4,Size);
  #endif

  if (bNext >= bMAX) {
    /* if bNext >= bMAX and bTotal < bMAX, then our array has one or more */
    /* empty slots in it.  Sort the array, moving the empty slots to the  */
    /* end of the array, then position bNext to the first empty slot.     */
    qsort(bArray,bNext,sizeof(baseptr),sortbaseptr);
    bNext = bTotal;
  }

  bArray[bNext].Ptr = p;
  #ifdef DEBUGMEM
    /* record the size of the block of memory so that bfree() can use it */
    bArray[bNext].Size = Size;
    bArray[bNext].AllocNum = ++bAllocNum;
  #endif
  bNext++;
  bTotal++;

  #ifdef DEBUGMEM
    if (bTotal > bHighest)
      bHighest = bTotal;

    if (DebugLevel >= 6) {
      sprintf(Str,"Mem Alloc #%d %u (%Fp) %d %d %d",bAllocNum,Size,p,bNext,bTotal,bHighest);
      writedebugrecord(Str);
    }
  #endif
  return(p);
}


void * LIBENTRY brealloc(void *p, unsigned NewSize) {
  baseptr *q;

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK) {
        writedebugrecord("Error in brealloc() - HEAP CORRUPT");
        return(NULL);
      }
    #endif
  #endif

  if (p == NULL || bNext < 1)
    return(NULL);

  q = &bArray[bNext-1];
  while (TRUE) {
    if (p == q->Ptr)
      break;
    if (q == bArray) {
      #ifdef DEBUGMEM
        writedebugrecord("Error in brealloc() - NOT ALLOCATED");
      #endif
      return(NULL);
    }
    q--;
  }

  /* at this point q->Ptr is a pointer that matches p */
  /* try to allocate the memory */
  if ((p = realloc(p,NewSize)) == NULL)
    return(NULL);

  /* fix the q->Ptr and then return the pointer to the new memory */
  q->Ptr = p;
  return(p);
}


void LIBENTRY bfree(void *p) {
  baseptr *q, *end;

  checkstack();

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK)
        writedebugrecord("Error in bfree() - HEAP CORRUPT");
    #endif
  #endif

  if (p == NULL || bNext < 1)
    return;

  q = end = &bArray[bNext-1];
  while (TRUE) {
    if (p == q->Ptr)
      break;
    if (q == bArray) {
      #ifdef DEBUGMEM
        writedebugrecord("Error in bfree() - NOT ALLOCATED");
      #endif
      return;
    }
    q--;
  }

  #ifdef DEBUGMEM
  {
    char Str[80];

    if (DebugLevel >= 6) {
      sprintf(Str,"Mem free #%d %u (%Fp)",q->AllocNum,q->Size,q->Ptr);
      writedebugrecord(Str);
    }

    /* set the size to 0xAF - meaning "AFTER" - so that if we ever use  */
    /* memory that has already been FREE'd it will stand out as obvious */
    memset(p,0xAF,q->Size);
    q->Size = 0;
  }
  #endif

  free(p);
  q->Ptr = NULL;
  bTotal--;

  /* if this pointer was at the end of the array, then walk backwards from  */
  /* here and see if there were any previously empty slots and move the     */
  /* fbNext pointer back so that it points to the first of the "last" group */
  /* of empty slots that are available in the array.                        */

  if (q == end) {
    for (; bNext > 0; ) {
      bNext--;
      q--;
      if (q->Ptr != NULL)
        break;
    }
  }

  #ifdef DEBUGMEM
    if (bNext < 0)
      writedebugrecord("Error in bfree() - bNext < 0");
    if (bTotal < 0)
      writedebugrecord("Error in bfree() - bTotal < 0");
  #endif
}


void LIBENTRY bfreeall(void) {
  baseptr *q;

  checkstack();

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK)
        writedebugrecord("Error in bfreeall() - HEAP CORRUPT");
    #endif
  #endif

  for (q = bArray; q < bArray+bMAX; q++) {
    if (q->Ptr != NULL) {
      #ifdef DEBUGMEM
      {
        char Str[80];

        if (DebugLevel >= 6) {
          sprintf(Str,"Mem free %d %u (%Fp)",q->AllocNum,q->Size,q->Ptr);
          writedebugrecord(Str);
        }

        /* set the size to 0xAF - meaning "AFTER" - so that if we ever use  */
        /* memory that has already been FREE'd it will stand out as obvious */
        memset(q->Ptr,0xAF,q->Size);
        q->Size = 0;

        /* decrement the total, then compare to zero when done */
        bTotal--;
      }
      #endif
      free(q->Ptr);
      q->Ptr = NULL;
    }
  }

  #ifdef DEBUGMEM
    if (bTotal != 0)
      writedebugrecord("Error in bfreeall() - bTotal != 0");
    if (DebugLevel > 0) {
      char Str[80];
      sprintf(Str,"bHighest = %d, bTotal = %d, bNext = %d",bHighest,bTotal,bNext);
      writedebugrecord(Str);
    }
  #endif

  bTotal = bNext = 0;
}


void _FAR_ * LIBENTRY fbmalloc(long Size) {
  void _FAR_ *p;
  #ifdef DEBUGMEM
    char Str[80];
  #endif

  checkstack();
  if (fbTotal >= fbMAX) {
    writedebugrecord("Error in fbmalloc() - INCREASE FBMALLOC");
    return(NULL);
  }

  if ((p = farmalloc(Size)) == NULL) {
    #ifdef DEBUGMEM
      if (DebugLevel >= 6) {
        sprintf(Str,"Insuff Memory for %ld (left: %ld)",Size,coreleft());
        writedebugrecord(Str);
      }
    #endif
    return(NULL);
  }

  #ifdef DEBUGMEM
    /* initialize allocated memory to 0xB4 - meaning "BEFORE" so that if we */
    /* run across uninitialized memory the B4's will stand out as obvious   */
    fmemset(p,0xB4,(unsigned) Size);
  #endif

  if (fbNext >= fbMAX) {
    /* if fbNext >= fbMAX and fbTotal < fbMAX, then our array has one or more*/
    /* empty slots in it.  Sort the array, moving the empty slots to the     */
    /* end of the array, then position fbNext to the first empty slot.       */
    qsort(fbArray,fbNext,sizeof(farbaseptr),sortfarbaseptr);
    fbNext = fbTotal;
  }

  fbArray[fbNext].Ptr = p;
  #ifdef DEBUGMEM
    /* record the size of the block of memory so that bfree() can use it */
    fbArray[fbNext].Size = (unsigned) Size;
    fbArray[fbNext].AllocNum = ++fbAllocNum;
  #endif
  fbNext++;
  fbTotal++;

  #ifdef DEBUGMEM
    if (fbTotal > fbHighest)
      fbHighest = fbTotal;
  #endif
  return(p);
}


void LIBENTRY fbfree(void _FAR_ *p) {
  farbaseptr *q, *end;

  checkstack();
  if (p == NULL || fbNext < 1)
    return;

  q = end = &fbArray[fbNext-1];
  while (TRUE) {
    if (p == q->Ptr)
      break;
    if (q == fbArray) {
      #ifdef DEBUGMEM
        writedebugrecord("Error in fbfree() - NOT ALLOCATED");
      #endif
      return;
    }
    q--;
  }

  #ifdef DEBUGMEM
    /* set the size to 0xAF - meaning "AFTER" - so that if we ever use  */
    /* memory that has already been FREE'd it will stand out as obvious */
    fmemset(p,0xAF,(unsigned) q->Size);
    q->Size = 0;
  #endif

  farfree(p);
  q->Ptr = NULL;
  fbTotal--;

  /* if this pointer was at the end of the array, then walk backwards from  */
  /* here and see if there were any previously empty slots and move the     */
  /* fbNext pointer back so that it points to the first of the "last" group */
  /* of empty slots that are available in the array.                        */

  if (q == end) {
    for (; fbNext > 0; ) {
      fbNext--;
      q--;
      if (q->Ptr != NULL)
        break;
    }
  }

  #ifdef DEBUGMEM
    if (fbNext < 0)
      writedebugrecord("Error in fbfree() - fbNext < 0");
    if (fbTotal < 0)
      writedebugrecord("Error in fbfree() - fbTotal < 0");
  #endif
}


void LIBENTRY fbfreeall(void) {
  farbaseptr *q;

  checkstack();
  for (q = fbArray; q < fbArray+fbMAX; q++) {
    if (q->Ptr != NULL) {
      #ifdef DEBUGMEM
        /* set the size to 0xAF - meaning "AFTER" - so that if we ever use  */
        /* memory that has already been FREE'd it will stand out as obvious */
        fmemset(q->Ptr,0xAF,(unsigned) q->Size);
        q->Size = 0;

        /* decrement the total, then compare to zero when done */
        fbTotal--;
      #endif
      farfree(q->Ptr);
      q->Ptr = NULL;
    }
  }

  #ifdef DEBUGMEM
    if (fbTotal != 0)
      writedebugrecord("Error in fbfreeall() - fbTotal != 0");
    if (DebugLevel > 0) {
      char Str[80];
      sprintf(Str,"fbHighest = %d, fbTotal = %d, fbNext = %d",fbHighest,fbTotal,fbNext);
      writedebugrecord(Str);
    }
  #endif
  fbTotal = fbNext = 0;
}


#ifdef MEMTEST
#include <stdio.h>

void test1(void) {
  char *a, *b, *c;
  printf("coreleft()  = %u\n",coreleft());
  a = bmalloc(2048);
  printf("a bmalloc() = %u\n",coreleft());
  b = bmalloc(2048);
  printf("b bmalloc() = %u\n",coreleft());
  c = bmalloc(2048);
  printf("c bmalloc() = %u\n",coreleft());
  bfree(b);
  printf("b bfree()   = %u\n",coreleft());
  bfree(c);
  printf("c bfree()   = %u\n",coreleft());
  bfreeall();
  printf("bfreeall()  = %u\n",coreleft());
}

/*
void test2(void) {
  char _FAR_ *a, _FAR_ *b, _FAR_ *c;
  printf("farcoreleft() = %ld\n",farcoreleft());
  a = fbmalloc(2048);
  printf("a fbmalloc()  = %ld\n",farcoreleft());
  b = fbmalloc(2048);
  printf("b fbmalloc()  = %ld\n",farcoreleft());
  c = fbmalloc(2048);
  printf("c fbmalloc()  = %ld\n",farcoreleft());
  fbfree(b);
  printf("b fbfree()    = %ld\n",farcoreleft());
  fbfree(c);
  printf("c fbfree()    = %ld\n",farcoreleft());
  fbfreeall();
  printf("fbfreeall()   = %ld\n",farcoreleft());
}
*/

void main(void) {
  test1();
/*
  printf("\n");
  test2();
*/
}
#endif
