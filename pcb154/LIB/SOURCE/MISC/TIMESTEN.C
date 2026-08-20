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
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

long LIBENTRY timesten(char *Str) {
  char *p;
  long X;
  char Digit;

  X = atol(Str) * 10;
  if ((p = strchr(Str,'.')) != NULL) {
    Digit = *(p+1);
    if (Digit >= '0' && Digit <= '9')
      X += Digit - '0';
  }
  return(X);
}

char * LIBENTRY decimal(char *Str, unsigned long Value) {
  unsigned long Tens;
  unsigned long Ones;

  Tens = Value/10;
  Ones = Value - (Tens * 10);

  if (Ones > 0)
    sprintf(Str,"%ld.%ld",Tens,Ones);
  else
    sprintf(Str,"%ld",Tens);

  return(Str);
}
