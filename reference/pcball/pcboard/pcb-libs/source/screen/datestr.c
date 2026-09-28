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


#include <system.h>
#include <screen.h>

#ifdef __OS2__
  #include <stdio.h>
#else
  #include <model.h>
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif


/********************************************************************
 *
 *  Function:  datestr()
 *
 *  Returns the Mon/Day/Year as a string
 */

char * LIBENTRY datestr(char * DStr) {
  sysdatetype SysDate;

  getsysdate(&SysDate);
  SysDate.Year -= (short) 1900;
  if (SysDate.Year >= 100)
    SysDate.Year -= (short) 100;

  #ifdef __OS2__

    sprintf(DStr,"%02d%c%02d%c%02d",
            SysDate.Month,
            Scrn_DateSeparator,
            SysDate.Day,
            Scrn_DateSeparator,
            SysDate.Year);

  #else /* ifdef __OS2__ */

    #ifdef _MSC_VER
      NEEDSEGPUSHDS;
      NEEDSEGGETDS(Scrn_DateSeparator);
      asm Mov  Bl,Scrn_DateSeparator
      NEEDSEGPOPDS;
    #else
      asm Mov  Bl,Scrn_DateSeparator;
    #endif

    #ifdef LDATA
      asm Les  Di,DStr
    #else
      asm Mov  Ax,Ds
      asm Mov  Es,Ax
      asm Mov  Di,DStr
    #endif

    asm Cld
    asm Mov  Al,SysDate.Month
    asm Aam
    asm Or   Ax,3030h
    asm Xchg Al,Ah
    asm Stosw

    asm Mov  Al,Bl
    asm Stosb

    asm Mov  Al,SysDate.Day
    asm Aam
    asm Or   Ax,3030h
    asm Xchg Al,Ah
    asm Stosw

    asm Mov  Al,Bl
    asm Stosb

    asm Mov  Ax,SysDate.Year
    asm Aam
    asm Or   Ax,3030h
    asm Xchg Al,Ah
    asm Stosw

    asm Xor  Ax,Ax
    asm Stosb
  #endif /* ifdef __OS2__ */

  return(DStr);
}
