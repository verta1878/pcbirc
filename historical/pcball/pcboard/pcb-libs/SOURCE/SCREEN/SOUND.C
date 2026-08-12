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


#include "screen.h"

#ifdef __OS2__
  #define INCL_DOSPROCESS
  #include <os2.h>
#else
  #include <dos.h>
  #ifdef _MSC_VER
    #include <borland.h>
  #endif
#endif

void LIBENTRY mysound(int Freq, int Duration) {
  #ifdef __OS2__
    DosBeep(Freq,Duration*10);
  #else
    sound(Freq);
    mydelay(Duration);
    nosound();
  #endif
}

void LIBENTRY beep(void) {
  if (Scrn_Box)           /* don't sound the beep if box is turned off */
    mysound(300,20);      /* because they are probably using it from remote */
}
