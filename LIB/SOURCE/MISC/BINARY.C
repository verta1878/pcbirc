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


#include <stdio.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY binary(char *Str, unsigned Num) {
  int X;

  for (X = 128; X != 0; X >>= 1, Str++) {
    *Str = (Num & X ? '1' : '0');
  }
  *Str = 0;
}

void LIBENTRY dumpreal(double Real) {
  char *p;
  char X;
  char Temp[10];

  printf("      seeeeeee eeeennnn nnnnnnnn nnnnnnnn nnnnnnnn nnnnnnnn nnnnnnnn nnnnnnnn\n");
  printf("      ");
  for (X = 8, p = ((char *) &Real) + 7; X > 0; X--, p--) {
    binary(Temp,(unsigned) *p);
    printf("%-9s",Temp);
  }
  printf("\n");

  for (X = 8, p = ((char *) &Real) + 7; X > 0; X--, p--)
    printf("      %02x ",(unsigned) *p);
  printf("\n\n");
}
