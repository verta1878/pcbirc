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
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <misc.h>
#include <pcb.h>
#include "pcbpack.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define BOXCOLOR     /* 0x01 */ Colors[OUTBOX]
#define HEADCOLOR    /* 0x0C */ Colors[HEADING]
#define QUESTCOLOR   /* 0x0B */ Colors[QUESTION]
#define DISPLAYCOLOR /* 0x07 */ Colors[DISPLAY]
#define SHOWCLOCK    /* 1    */ ShowClock


unsigned MaxRetryCount = 0xFFFF;
char DosErrorLevel     = 0;

/********************************************************************
*
*  Function:  showerror()
*
*  Desc    :  This function is called whenever a DOS error occurs when trying
*             to open or write to a file.
*
*             It displays an error message stating the condition of the
*             system and the file on which the error occured.
*
*             The screen and cursor positions are saved on entry and restored
*             on exit.
*
*             If Timed==FALSE then the user is simply prompted to press any key
*             to continue.  Otherwise the display counts down 10 seconds before
*             automatically returning to the caller.  Additionally, the count
*             down can be aborted by pressing ESC.
*
*  Returns :  The return value is always 0 unless Timed==TRUE and the user
*             pressed ESC to abort the countdown.
*/

int LIBENTRY showerror(char *path, char *str, bool Timed DOS2ERROR) {
  int  Counter;
  char X1;
  char X2;
  char Len;
  int  CursorStatus;
  int  RetVal;
  char Seconds[3];
  char Temp[80];
  savescrntype  ScrnBuf;

  #ifdef __OS2__
    int ExtendedError  = Os2Error->ExtendedError;
    int ExtendedAction = Os2Error->ExtendedAction;
  #endif

  sprintf(Temp,"(%d:%d) Error %s File - %s",ExtendedError,ExtendedAction,str,doserror(NULL,ExtendedError));

  Len = (char) (Timed ? 52 : 25);
  if (strlen(Temp) > Len)
    Len = (char) strlen(Temp);
  if (strlen(path) > Len)
    Len = (char) strlen(path);
  Len += (char) 4;

  X1  = (char) (39 - (Len >> 1));
  X2  = (char) (X1 + Len);

  savescreen(&ScrnBuf);
  boxcls(X1,14,X2,22,BOXCOLOR,SINGLE);
  X1 = wherex();
  X2 = wherey();
  fastcenter(16,Temp,HEADCOLOR);
  fastcenter(18,path,QUESTCOLOR);
  CursorStatus = getcursor();
  setcursor(CUR_NORMAL);

  RetVal = 0;
  if (Timed) {
    strcpy(Seconds," 9");
    #ifndef __OS2__
      timechanged();
    #endif
    fastprintmove(13,20,"Automatic retry in 10 seconds.  Press ESC to cancel.",DISPLAYCOLOR);
    while (1) {
      if (Seconds[1] < '0')
        goto goback;
      fastprint(32,20,Seconds,DISPLAYCOLOR);
      Seconds[1]--;
      if (waitonesecond()) {
        RetVal = -1;
        goto goback;
      }
    }
  } else {
    beep();
    fastprintmove(27,20,"press any key to continue",DISPLAYCOLOR);
    for (Counter = 3; Counter; Counter--)
      if (waitonesecond())
        break;
  }

goback:
  setcursor(CursorStatus);
  restorescreen(&ScrnBuf);
  gotoxy(X1,X2);
  return(RetVal);
}


/********************************************************************
*
*  Function:  retrycount()
*
*  Desc    :
*
*  Returns :
*/

unsigned LIBENTRY retrycount(unsigned Count, char *Path, char *Str DOS2ERROR) {
  #ifdef __OS2__
    int ExtendedAction = Os2Error->ExtendedAction;
  #else
    if (ExtendedError == 83) {
      ExtendedError  = Int24Error + 19;  /* errors 19-31 correspond to INT24 errors */
      ExtendedAction = Int24Flags;
    }
  #endif

  if (ExtendedAction > 2)                  /* errors below 3 should retry    */
    goto show;                             /* show error, wait for keystroke */
  else {

    Count++;                               /* Increment the counter          */
    if (Count > MaxRetryCount)             /* Have we gone over our limit?   */
      goto exit;                           /* If so, then exit now!          */

    if (Count < 4) {                       /* First 4 retries simply delay   */
      mydelay(25);                         /* 1/4 sec, afterwards it will go */
    } else {                               /* into an infinite retry loop    */
      if (showerror(Path,Str,TRUE POS2ERROR2) == -1)  /* show error with countdown      */
        return(0xFFFF);                    /* user abort, no show, just exit */
    }                                      /* show error, wait for keystroke */

    return(Count);
  }

show:
  if (MaxRetryCount != 0xFFFF) {            /* if it's a BAD error and the   */
    showerror(Path,Str,TRUE POS2ERROR2);    /* MaxRetryCount is adjusted then*/
    goto exit;                              /* we better just show the msg   */
  }                                         /* and then get outta here!      */

  showerror(Path,Str,FALSE POS2ERROR2);     /* show final error message */
  return(0xFFFF);

exit:
  cls();
  setcursor(CUR_NORMAL);
  fastprintmove(0,0,"An error has occured.  The /RETRY:## parameter has forced an abort!",0x0F);
  mydelay(200);
  DosErrorLevel = 1;
  return(0xFFFF);
}
