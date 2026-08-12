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


#include "system.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

#ifdef __OS2__
  #define INCL_DOSDATETIME
  #include <os2.h>
#else
  #include <model.h>
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif


void LIBENTRY getsystime(systimetype *SysTime) {
  #ifdef __OS2__
    DATETIME Time;

    DosGetDateTime(&Time);
    SysTime->Hours      = (ubyte) Time.hours;
    SysTime->Minutes    = (ubyte) Time.minutes;
    SysTime->Seconds    = (ubyte) Time.seconds;
    SysTime->Hundredths = (ubyte) Time.hundredths;

  #else

    #ifdef _MSC_VER
      char  _CL,_CH,_DH,_DL;
    #endif

    asm Mov  Ah,2Ch
    int21();

    #ifdef _MSC_VER
      asm mov _CL,cl
      asm mov _CH,ch
      asm mov _DH,dh
      asm mov _DL,dl
    #endif

    SysTime->Hours      = _CH;
    SysTime->Minutes    = _CL;
    SysTime->Seconds    = _DH;
    SysTime->Hundredths = _DL;
  #endif
}
