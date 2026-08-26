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


#ifdef __OS2__
  #define INCL_VIO
  #include <os2.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
/*  #pragma inline */
  #endif
#endif

#include "screen.h"

#pragma warn -rvl
/********************************************************************
 *
 *  Function:  wherey()
 *
 *  report cursor row position
 */

char LIBENTRY wherey(void) {
#ifdef __OS2__
  USHORT Row, Column;
  VioGetCurPos(&Row,&Column,0);
  Scrn_Y = (char) Column;
  return((char) Row);
#else
  asm  Mov  Ah,3
  asm  Xor  Bh,Bh
  asm  Int  10h
  asm  Mov  Al,Dh
/*  return(_AL);*/ /* return Dh in Al */
#endif
}
