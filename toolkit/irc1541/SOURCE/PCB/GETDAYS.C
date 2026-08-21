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


#include "pcb.h"


void LIBENTRY getdays(char *Days, char *Str) {
  int X;

  for (X = 0, *Days = 0; *Str != 0 && X < 7; X++, Str++) {
    if (*Str == 'Y')
      *Days += (char) (1 << X);
  }
}


void LIBENTRY undogetdays(char *Str, char *Days) {
  int X;

  for (X = 0, *Str = 0; X < 7; X++, Str++)
    *Str = ((*Days & (1 << X)) != 0 ? 'Y' : 'N');
  *Str = 0;
}
