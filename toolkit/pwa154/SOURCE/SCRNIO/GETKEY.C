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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <system.h>

#ifdef INCOMPLETE
  #include <country.h>
#endif

#include "scrnio.h"
#include "scrnio.ext"

#ifdef __OS2__
  #include <system.h>
  #include <semafore.hpp>
  #include <process.h>
#endif

#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function: showtime()
*
*  Displays the current time and date on the bottom left corner of the screen
*/

void LIBENTRY showtime(void) {
  char TStr[11];
  char DStr[11];

  if (ShowClock) {
    timestr1(&TStr[1]);
    datestr(&DStr[1]);

    #ifdef INCOMPLETE
      countrydate(&DStr[1]);
    #endif

    DStr[0] = TStr[0] = DStr[9] = TStr[9] = ' ';
    DStr[10] = TStr[10] = 0;

    fastprint( 3,Scrn_BottomRow,TStr,Colors[STATUS]);
    fastprint(16,Scrn_BottomRow,DStr,Colors[STATUS]);
  }
}


/********************************************************************
*
*  Function: showkeystatus()
*
*  Displays the status of CAPSLOCK, NUMLOCK and INSERT on the screen
*/

void LIBENTRY showkeystatus(void) {
  char   Str[32];
  short *p;
  short  CurStatus;

  CurStatus = getkbdstatus();

  if (UpdateKbdStatus) {
    strcpy(Str," caps: OFF  num: OFF  ins: OFF ");
    p = (short *) Str;

    if (CurStatus & CAPLCK)  /* change OFF to ON by replacing 'FF' with 'N ' */
      *(p+4) = 0x204E;

    if (CurStatus & NUM)     /* change OFF to ON by replacing 'FF' with 'N ' */
      *(p+9) = 0x204E;

    if (CurStatus & INSERT)  /* change OFF to ON by replacing 'FF' with 'N ' */
      *(p+14) = 0x204E;

    fastprint(47,Scrn_BottomRow,Str,Colors[STATUS]);
  }

  if (getcursor() != CUR_BLANK) {
    if (CurStatus & INSERT)
      setcursor(CUR_INSERT);
    else
      setcursor(CUR_NORMAL);
  }
}

#ifdef __OS2__
static CUpdateSemaphore ShowStatusSemaphore;


#pragma argsused
void showstatus(void *Ignore) {
  while(1) {
    ShowStatusSemaphore.waitforevent();
    if (ShowStatusSemaphore.isenabled()) {
      showtime();
      showkeystatus();
    }
    ShowStatusSemaphore.reset();
  }
}


void LIBENTRY disableshowstatus(void) {
  ShowStatusSemaphore.disabletimer();
  disablekbdmonitor();
}


void LIBENTRY enableshowstatus(void) {
  ShowStatusSemaphore.enabletimer(1000);  /* update once per second */
  enablekbdmonitor();
}


void LIBENTRY initshowstatus(void) {
  ShowStatusSemaphore.create(NULL);
  disableshowstatus();
  #ifdef __BORLANDC__
    _beginthread(showstatus,4096,NULL);
  #elif defined(__WATCOMC__)
/*      char *Stack = (char *) malloc(16384); */
/*      char *args[2]; */
/*      args[0] = "test"; */
/*      args[1] = "test"; */
      _beginthread(showstatus,malloc(4096),4096,NULL);
  #endif
}
#endif

/********************************************************************
*
*  Function: keyschanged()
*
*  Returns TRUE if the keystatus of CAPSLOCK, NUMLOCK or INSERT has changed
*/

#ifndef __OS2__
static char _OldKeys;

char LIBENTRY keyschanged(void) {
  unsigned short KbdStatus;

  KbdStatus = getkbdstatus();
  if (_OldKeys != KbdStatus) {
    _OldKeys = KbdStatus;
    return(1);
  } else return(0);
}
#endif

/********************************************************************
*
*  Function: inkey()
*
*  - Calls bgetkey() to read the keyboard
*  - if Clock is TRUE then the clock is displayed on the screen
*  - determines if keypressed returned a normal or "extended" code
*  - returns ascii character for keypressed (or extended code)
*  - returns with Ext set to TRUE if it was extended otherwise FALSE
*/

char LIBENTRY inkey(char *Ext, clocktype Clock) {
  bkeytype Key;

  #ifdef __OS2__
    if (Clock || UpdateKbdStatus)
      enableshowstatus();
  #else
    while (! bgetkey(1)) {
      if (Clock && timechanged())
        showtime();
      if (keyschanged())
        showkeystatus();
      giveup();
    }
  #endif

top:
  Key.W = (short) bgetkey(0);
  if (Key.B.LoB == 0) {
    #ifdef __OS2__
      if (Key.W == 0) {
        showkeystatus();
        goto top;
      }
    #endif
    *Ext=TRUE;
    Key.B.LoB = Key.B.HiB;
  } else
    *Ext=FALSE;

  return(Key.B.LoB);
}


/********************************************************************
*
*  Function: keypressed()
*
*  - Calls bgetkey() to test if a key has been press
*  - if Clock is TRUE then the clock is displayed on the screen
*  - if keypressed() returns a TRUE value then inkey() should be called
*/

#pragma argsused
bool LIBENTRY keypressed(clocktype Clock) {
  #ifndef __OS2__
    if (Clock) {
      if (timechanged())
        showtime();
      if (keyschanged())
        showkeystatus();
    }
  #endif
  return((bool) bgetkey(1));
}

