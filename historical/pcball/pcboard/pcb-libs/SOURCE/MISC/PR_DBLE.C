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

/*  converts TP 6-byte real to IEEE 8-byte real  */

double LIBENTRY pasrealtodouble(pasreal OldNum) {
  IEEEdouble NewNum;
  char       Sign;
  int        Exp;
  int        X;

  for(X = 0; X < 2; X++)
    NewNum.byte[X] = 0x00;

  Sign = (char) (OldNum[5] & 0x80);
  Exp = OldNum[0] - 0x81 + 0x3FF;
  NewNum.byte[6] = (char)  (Exp << 4);
  NewNum.byte[7] = (char) ((Exp >> 4) | Sign);

  for(X = 5; X > 1; X--) {
    OldNum[X] <<= 1;
    OldNum[X] |= (char) (OldNum[X-1] >> 7);
  }
  OldNum[1] <<= 1;

  for(X = 6; X >= 2; X--) {
    NewNum.byte[X]  |= (char) (OldNum[X-1] >> 4);
    NewNum.byte[X-1] = (char) (OldNum[X-1] << 4);
  }

  return(NewNum.value);
}
