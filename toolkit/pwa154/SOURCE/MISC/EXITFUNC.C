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


/*  this module has not been converted to OS/2 */

#ifndef __OS2__

#include <stdio.h>
#include <alloc.h>
#include <process.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

typedef void (**exitfuncs)(void);
exitfuncs static exitfunctions;

/* void static (**exitfunctions)(void);*/

int  static NumFuncAllocated;
int  static FuncPointer;

void pascal initexitfunctions(int NumFunc) {
  if ((exitfunctions = (exitfuncs) malloc(NumFunc * sizeof(exitfunctions))) == NULL) {
    puts("cannot allocate exit functions");
    exit(99);
  }

  NumFuncAllocated = NumFunc;
}

void pascal doexitfunctions(void) {
  while (FuncPointer > 0)
    exitfunctions[--FuncPointer]();

  if (exitfunctions != NULL)
    free(exitfunctions);
}

void pascal pushexitfunction(void (*function)(void)) {
  if (FuncPointer < NumFuncAllocated)
    exitfunctions[FuncPointer++] = function;
  else {
    puts("pushed too many exit functions");
    exit(99);
  }
}

void pascal popexitfunction(void) {
  if (FuncPointer > 0)
    FuncPointer--;
  else {
    puts("popped too many exit functions");
    exit(99);
  }
}

/****************************************************************************
void f1(void) {
  puts("doing 1");
}
void f2(void) {
  puts("doing 2");
}
void f3(void) {
  puts("doing 3");
}
void f4(void) {
  puts("doing 4");
}

void main(void) {
  initexitfunctions(100);
  pushexitfunction(f1);
  pushexitfunction(f2);
  pushexitfunction(f3);
  pushexitfunction(f4);
  popexitfunction();
  doexitfunctions();
}
*****************************************************************************/


#endif  /* ifndef __OS2__ */
