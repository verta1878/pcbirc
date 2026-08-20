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


#include <dos.h>
#include <stdio.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#ifdef DEBUG
#include <memcheck.h>
#endif

int LIBENTRY allowmore(int Num, int Max) {
  char Str[5];

  if (Num > Max) {
    sprintf(Str,"%4d",Max);
    boxcls(6,19,73,23,Colors[MENUBOX],SINGLE);
    fastprint(9,21,"Too many lines in file. The maximum number of lines is",Colors[ANSWER]);
    fastprint(66,21,Str,Colors[HEADING]);
    beep();
    mydelay(350);
    return(0);
  }

  return(Num+1);
}
