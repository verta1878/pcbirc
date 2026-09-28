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
  #ifdef BIOS
    #include "ansi.h"
  #endif
  #ifdef _MSC_VER
    #include <borland.h>
  #else
/*  #pragma inline */
  #endif
#endif

#include "screen.h"

/********************************************************************
 *
 *  Function:  gotoxy()
 *
 *  position cursor at location  X,Y  (base 0)
 */

void LIBENTRY gotoxy(int X, int Y) {
#ifdef __OS2__
/*VioSetCurPos((short) Y, (short) X, 0); */
  Scrn_X = (char) X;
  Scrn_Y = (char) Y;
#else
  asm  mov   ah, 2
  asm  xor   bh, bh
  asm  mov   dh, byte ptr Y
  asm  mov   dl, byte ptr X
  asm  int   10h
#endif
}
