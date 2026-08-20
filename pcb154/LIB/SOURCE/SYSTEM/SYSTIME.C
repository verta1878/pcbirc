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
#elif defined(__WATCOMC__)
  #include <i86.h>
  #include <dos.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
//  #pragma inline
  #endif
  #include "model.h"
#endif

#include "system.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

void LIBENTRY getsystime(systimetype *SysTime) {
  #ifdef __OS2__
    DATETIME Time;

    DosGetDateTime(&Time);
    SysTime->Hours      = (ubyte) Time.hours;
    SysTime->Minutes    = (ubyte) Time.minutes;
    SysTime->Seconds    = (ubyte) Time.seconds;
    SysTime->Hundredths = (ubyte) Time.hundredths;

  #elif defined(__WATCOMC__)
    {
      union REGS r;
      r.h.ah = 0x2C;
      int386(0x21, &r, &r);
      SysTime->Hours   = r.h.ch;
      SysTime->Minutes = r.h.cl;
      SysTime->Seconds = r.h.dh;
      SysTime->Hundredths = r.h.dl;
    }
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
