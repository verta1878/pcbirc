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

/*  converts QB 4-byte real to IEEE 8-byte real  */

double LIBENTRY bassngltodouble(bassngl OldNum) {
  IEEEdouble NewNum;
  bassngl    Temp;
  char       Sign;
  int        Exp;
  int        X;

  if (OldNum[7] == 0 || OldNum[7] == 0x80)
    return(0);

  for(X = 0; X < 4; X++)
    NewNum.byte[X] = 0x00;

  Sign = (char) (OldNum[2] & 0x80);
  Exp  = (int)  (OldNum[3] - 0x81 + 0x3FF);
  NewNum.byte[6] = (char) (Exp << 4);
  NewNum.byte[7] = (char) ((Exp >> 4) | Sign);

  memcpy(Temp,OldNum,sizeof(bassngl));

  for(X = 2; X > 0; X--) {
    Temp[X] <<= 1;
    Temp[X] |= (char) (Temp[X-1] >> 7);
  }
  Temp[0] <<= 1;

  for(X = 6; X >= 4; X--) {
    NewNum.byte[X]   |= (char) (Temp[X-4] >> 4);
    NewNum.byte[X-1]  = (char) (Temp[X-4] << 4);
  }

  return(NewNum.value);
}
