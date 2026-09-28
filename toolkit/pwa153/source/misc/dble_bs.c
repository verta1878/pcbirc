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

/*  converts IEEE 8-byte real to basic 4-byte real  */

void LIBENTRY doubletobassngl(bassngl *New, double Old) {
  unsigned char  Sign;
  unsigned char  Exp;
  int            X;
  unsigned char *NewNum, *OldNum;

  NewNum = (unsigned char *) New;
  OldNum = (unsigned char *) &Old;

  Sign = (unsigned char) (OldNum[7] & 0x80);
  Exp  = (unsigned char) (((OldNum[7] & 0x7F) << 4) + (OldNum[6] >> 4));
  if (Exp)
    Exp += (unsigned char) (0x81 - 0x3FF);

  for (X = 2; X >= 0; X--) {
    NewNum[X]  = (unsigned char) (OldNum[X+4] << 4);
    NewNum[X] |= (unsigned char) (OldNum[X+3] >> 4);
  }

  for (X = 0; X < 2; X++) {
    NewNum[X] >>= 1;
    NewNum[X] |= (unsigned char) (NewNum[X+1] << 7);
  }
  NewNum[2] >>= 1;

  NewNum[2] |=  Sign;
  NewNum[3] =   Exp;
}
