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


#include <mem.h>
#include <stdio.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY longtopasreal(char *F, long Num) {
  char Sign;
  int  X;
  int  Y;
  int  Exp;

  memset(F,0,sizeof(pasreal));

  if (Num) {
    Exp = 0;

    if (Num & 0x80000000L) {
      Sign = 0x80;
      Num = -Num;
    } else
      Sign = 0;

    for (X = 32, Y = 47; X > 0; X--) {
      if (Exp == 0) {
        if (isset(&Num,X)) {
          Exp = X-1;
        }
      } else {
        if (isset(&Num,X))
          setbit(F,Y);
        Y--;
      }
    }

    Exp += 0x81;

    *(F+5) |= Sign;
    *F      = (char) Exp;
  }
}
