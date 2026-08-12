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
#include "screen.h"


/********************************************************************
 *
 *  Function:  getseconds()
 *
 *  Returns Hours:Mins:Secs as a single LONG integer represented in seconds
 */

long LIBENTRY getseconds(void) {
  systimetype SysTime;

  getsystime(&SysTime);
  return((long) SysTime.Hours * 3600L + SysTime.Minutes*60 + SysTime.Seconds);
}


char LIBENTRY cursecond(void) {
  systimetype SysTime;

  getsystime(&SysTime);
  return(SysTime.Seconds);
}
