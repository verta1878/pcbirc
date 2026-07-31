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


#ifndef __OS2__
  #ifdef _MSC_VER
    #include <borland.h>
  #else
//  #pragma inline
  #endif
#endif

#include "misc.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#pragma warn -rvl
#pragma argsused
bool LIBENTRY printerready(int LptNum) {
  #ifdef __OS2__
    return(TRUE);
  #else
    asm mov  ah,2
    asm mov  dx,LptNum         /* lpt 1, 2 or 3 */
    asm dec  dx                /* change to 0, 1 or 2 */
    asm int  17h
    asm mov  Al,Ah
    asm xor  Ah,Ah
/*  return;*/ /* ignore return value error, it's already in AL */
  #endif
}
