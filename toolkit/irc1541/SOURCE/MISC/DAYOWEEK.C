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
#include "misc.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

/* Sunday = 1 ... Saturday = 7 */


int LIBENTRY dayofweek(int Mon, int Day, int Year) {
  int DayOfWeek;

  if (Mon <= 2) {
    Mon  += 12;
    Year -= 1;
  }

  DayOfWeek = (Day + Mon*2 + (Mon+1)*6/10 + Year + Year/4 -
               Year/100 + Year/400 + 2) % 7;

  if (DayOfWeek == 0)
    return(7);
  else
    return(DayOfWeek);
}


int LIBENTRY dayofweektoday(void) {
  sysdatetype SysDate;

  getsysdate(&SysDate);
  return(SysDate.DayOfWeek);
}
