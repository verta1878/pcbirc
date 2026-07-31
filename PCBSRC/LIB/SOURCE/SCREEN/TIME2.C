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
  #include <stdio.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
  #include "model.h"
#endif

#include <system.h>
#include "screen.h"


/********************************************************************
 *
 *  Function:  timestr()
 *
 *  Returns the Hours:Mins as a string
 */

char * LIBENTRY timestr2(char * TStr) {
  systimetype SysTime;

  getsystime(&SysTime);
  if (SysTime.Hours > 12 && ! Scrn_24Hour)
    SysTime.Hours -= (char) 12;

  #ifdef __OS2__

    sprintf(TStr,"%02d:%02d",
            SysTime.Hours,
            SysTime.Minutes);

  #else /* ifdef __OS2__ */

    #ifdef LDATA
      asm Les  Di,TStr
    #else
      asm Mov  Ax,Ds
      asm Mov  Es,Ax
      asm Mov  Di,TStr
    #endif

    asm Cld
    asm Mov  Al,SysTime.Hours
    asm Aam
    asm Or   Ax,3030h
    asm Xchg Al,Ah
    asm Stosw

    asm Mov  Al,':'
    asm Stosb

    asm Mov  Al,SysTime.Minutes
    asm Aam
    asm Or   Ax,3030h
    asm Xchg Al,Ah
    asm Stosw

    asm Xor  Ax,Ax
    asm Stosb
  #endif /* ifdef __OS2__ */

  return(TStr);
}
