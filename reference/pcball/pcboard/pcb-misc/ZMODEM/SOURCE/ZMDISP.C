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


#include <pcbtools.h>
#include <stdarg.h>
#include <stdio.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

/***************************************************************************
 ***************************************************************************/
void _Cdecl zmDisplay(int X, int Y, int Color, char *Format, ...) {
  char        OutBuf[100];
  va_list     vars;

  va_start(vars, Format);
  vsprintf(OutBuf, Format, vars);  //lint !e534
  va_end(vars);

  if (! Scrn_Mode)
    Color = 0x07;

  fastprint(X, Y, OutBuf, Color);
}
