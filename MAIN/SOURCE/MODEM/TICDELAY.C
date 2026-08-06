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


#include "project.h"
#pragma hdrstop

void LIBENTRY tickdelay(int Ticks) {
  #ifdef __OS2__
    mydelay(Ticks/10);  // under OS/2 Ticks=1/1000th of a second
  #else
    /* under DOS Ticks=1/18.2 of a second */
    settimer(3,Ticks);
    while (! timerexpired(3)) {
      #ifdef COMM
        turnonxmit();
      #endif
      if (Control.WatchSessionClock && gettimer(5) <= THREEMINUTES)
        checktime();
      giveup();
    }
  #endif
}
