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


#include <stddef.h>
#include <string.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

void huge *ZcompKey;  /* used in comp() function as left  side of compare */
void huge *ZcompPtr;  /* used in comp() function as right side of compare */

char huge * pascal zsearch(void *Key, void huge *Base, long Num, unsigned Len, int (*comp)(void)) {
  long Low, High, Middle;
  int  Ret;

  ZcompKey = Key;
  ZcompPtr = Base;

  if ((Ret = comp()) == 0)
    return((char huge *) Base);
  if (Ret < 0)
    return(NULL);

  Low  = 0;
  High = Num;

  while (1) {
    if (High <= Low+1)
      return(NULL);

    Middle = Low + ((High - Low) >> 1);
    ZcompPtr = (char huge *)Base + ((long)Middle * Len);
    if ((Ret = comp()) == 0)
      return((char huge *) ZcompPtr);
    if (Ret < 0)
      High = Middle;
    else
      Low = Middle;
  }
}


#ifdef TEST
#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>

#define NUM 8000
#define LEN 10

typedef struct {
  char a[LEN];
} mystruct;

mystruct huge *List;

void makelist(void) {
  mystruct huge *p, huge *q;

  q = &List[NUM];
  for (p = &List[0]; p < q; p++) {
    sprintf(p->a,"%p",p);
  }
}

int compare(void) {
  return(strcmp(ZcompKey,ZcompPtr));
}

void searchlist(void) {
  char huge *p;
  char Temp[LEN];
  char Str[40];
  long Count;

  for (Count = 0, p = List[0].a; Count < (long) NUM * LEN; Count++, p++) {
    sprintf(Temp,"%p",p);
    if (zsearch(Temp,List,NUM,LEN,compare) == NULL) {
      sprintf(Str,"%s not found",Temp);
      fastprint(0,0,Str,2);
    } else {
      sprintf(Str,"%s found (%ld)",Temp,Count/LEN);
      fastprint(0,1,Str,15);
    }
  }
}

void main(void) {
  getmode();
  cls();
  gotoxy(0,5);
  if ((List = farmalloc((long) NUM * sizeof(mystruct))) == NULL) {
    fastprint(0,0,"insufficient memory",15);
    return;
  }
  makelist();
  searchlist();
  farfree(List);
}
#endif
