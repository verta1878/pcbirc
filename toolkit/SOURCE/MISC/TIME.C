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
#include "misc.h"

#ifdef __OS2__
  #include <stdio.h>
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif


unsigned LIBENTRY strtominutes(char *Str) {
  unsigned Num;

  Num  = (Str[0]-'0') * 600;
  Num += (Str[1]-'0') * 60;
  Num += (Str[3]-'0') * 10;
  Num += (Str[4]-'0');
  return(Num);
}


unsigned LIBENTRY currentminute(void) {  /* based on (hours*60)+minutes */
  systimetype SysTime;

  getsystime(&SysTime);
  return((SysTime.Hours * 60) + SysTime.Minutes);
}


long LIBENTRY exacttime(void) {  /* based on ((hours*60)+minutes)*60+seconds */
  systimetype SysTime;

  getsystime(&SysTime);
  return((((long) (SysTime.Hours * 60L) + SysTime.Minutes) * 60L) + (long) SysTime.Seconds);
}


#ifndef LIB
unsigned LIBENTRY dostimetofiletime(void) {
  systimetype SysTime;

  getsystime(&SysTime);
  return((SysTime.Hours << 11) + (SysTime.Minutes << 5) + (SysTime.Seconds >> 1));
}


char LIBENTRY currentsecond(void) {      /* only the current second counter */
  systimetype SysTime;

  getsystime(&SysTime);
  return(SysTime.Seconds);
}


/********************************************************************
*
*  Function:  timeinrange()
*
*  Desc    :  Determines if a given minute is within the range specified by
*             StartMinute and StopMinute (compensates for the spanning of
*             midnight in a range)
*
*  Returns :  TRUE if it's in range
*/

bool LIBENTRY timeinrange(char *Start, char *Stop) {
  int Minute;
  int StartMinute;
  int StopMinute;

  Minute      = currentminute();
  StartMinute = strtominutes(Start);
  StopMinute  = strtominutes(Stop);

  if (StartMinute < StopMinute)
    return((bool) (Minute >= StartMinute && Minute <= StopMinute ? TRUE : FALSE));
  else
    return((bool) (Minute >= StopMinute && Minute <= StartMinute ? FALSE : TRUE));
}
#endif
