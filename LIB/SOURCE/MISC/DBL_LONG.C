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

long LIBENTRY doubletolong(void *Num) {
  unsigned char *p;
  int  Exp;
  register int  X;
  unsigned long Sum;
  register int  ShiftVal;

  #if defined(_MSC_VER) || defined(__WATCOMC__)
    short _AX, _DX;
  #endif

  p = (unsigned char *) Num;

  Exp = (*(p+6) >> 4) + ((*(p+7) & 127) << 4) - 0x3ff;

  if (Exp > 0) {
    if (Exp < 16) {
      _AX = (short) (1 << Exp);
      _DX = 0;
    } else {
      _AX = 0;
      _DX = (short) (1 << (Exp-16));
    }
    Sum = (((unsigned long) _DX << 16) | ((unsigned int) _AX));
    for (ShiftVal = Exp-1, X = 52; ShiftVal > -1; ShiftVal--, X--)
      if (isset(Num,X)) {
        if (ShiftVal < 16) {
          _AX = (short) (1 << ShiftVal);
          _DX = 0;
        } else {
          _AX = 0;
          _DX = (short) (1 << (ShiftVal-16));
        }
        Sum += (((unsigned long) _DX << 16) | ((unsigned int) _AX));
      }
  } else return(0);

  if (*(p+7) & 0x80)
    return(-Sum);
  else
    return(Sum);
}
