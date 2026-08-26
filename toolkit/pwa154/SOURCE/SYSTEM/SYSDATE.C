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
  #define INCL_DOSDATETIME
  #include <os2.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
/*  #pragma inline */
  #endif
  #include "model.h"
#endif

#include "system.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

void LIBENTRY getsysdate(sysdatetype *SysDate) {
  #ifdef __OS2__
    DATETIME Date;

    DosGetDateTime(&Date);
    SysDate->Month     = (ubyte)  Date.month;
    SysDate->Day       = (ubyte)  Date.day;
    SysDate->Year      = (uint)   Date.year;
    SysDate->DayOfWeek = (ubyte) (Date.weekday + 1);

  #else
    #ifdef _MSC_VER
      char  _AL,_DL,_DH;
      short _CX;
    #endif

    asm Mov  Ah,2Ah
    int21();

    #ifdef _MSC_VER
      asm mov _AL,al
      asm mov _DL,dl
      asm mov _DH,dh
      asm mov _CX,cx
    #endif
/*
    asm Mov  SysDate->Month,DH
    asm Mov  SysDate->Day,DL
    asm Mov  SysDate->Year,CX
    asm Mov  SysDate->DayOfWeek,AL
*/
    SysDate->Month     = _DH;
    SysDate->Day       = _DL;
    SysDate->Year      = _CX;
    SysDate->DayOfWeek = _AL + 1;
  #endif
}


#ifdef TEST
#include <stdio.h>
void main(void) {
  sysdatetype Date;

  getsysdate(&Date);
  printf("Month = %d\n"
         "Day   = %d\n"
         "Year  = %d\n"
         "Day of Week = %d\n",
         Date.Month,
         Date.Day,
         Date.Year,
         Date.DayOfWeek);
}
#endif
