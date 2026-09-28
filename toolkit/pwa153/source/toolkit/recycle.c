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


#include <dos.h>
#include <process.h>
#include "pcboard.h"
#include "pcboard.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef _MSC_VER
#include <borland.h>
#endif

void LIBENTRY atclosing(void);
void LIBENTRY makepcboardsys(void);
void LIBENTRY closepcbtext(void);
void LIBENTRY writeusersysfile(void);

static bool ForcedCallerOff;

extern char PcbDir[30];
extern char SysName[66];
extern bool ErrorLevel;
extern bool ForceUpdate;


/********************************************************************
*
*  Function:  recycle()
*
*  Desc    :  If a user was online this function takes care of updating his
*             user record.
*
*  NOTES   :  A non-registered user (user who is not in the users file) will
*             have Status.UserRecNo set to ZERO.  If no user was online or
*             the user hung up without his name being found in the users file
*             then Status.UserRecNo is set to 0xFFFF.
*/

void LIBENTRY recycle(void) {
  if (Status.LoggingUserOff)   /* don't let it go recursive */
    return;

  if (Asy.LostCarrier) {
    if (ForceUpdate)
      writeusersysfile();
    tickdelay(36);
  }

  atclosing();

  if (SysName[0] != 0)
    makepcboardsys();

  Status.LoggingUserOff = TRUE;
  Control.WatchKbdClock = FALSE;
  Control.WatchSessionClock = FALSE;
  closemodem(ForcedCallerOff);        /* close modem                       */
  closecallerlog();                   /* close the callers log             */
  closepcbtext();                     /* close pcbtext file if it was open */
  bfreeall();                         /* deallocate anything that's left   */
  closedosfopenfiles();               /* close any open buffered files     */
  #ifndef __OS2__
    setcbrk(CtrlBreak);
  #endif
  exit(ErrorLevel);
}


void LIBENTRY loguseroff(logtype Type) {
  if (Type == ALOGOFF) {
    if (Asy.Online == REMOTE && Asy.LostCarrier)
      turnoffdtr();
    ForcedCallerOff = TRUE;
  } else {
    ForcedCallerOff = FALSE;
  }

  recycle();
}
