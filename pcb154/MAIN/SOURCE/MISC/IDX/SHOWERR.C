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


#include <io.h>
#include <dos.h>
#include <mem.h>
#include <bios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include "dosfunc.h"

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

int pascal showerror(char *path, char *str, bool Timed) {
  char Temp[80];
  int  Seconds;

  sprintf(Temp,"(%d:%d) Error %s File - %s",ExtendedError,ExtendedAction,str,doserror(NULL, ExtendedError));
  puts(Temp);

  Seconds = 10;
  if (Timed) {
    timechanged();
    puts("Automatic retry in 10 seconds.  Press ESC to cancel.");
    while (1) {
      while (! bioskey(1)) {
        giveup();
        if (--Seconds < 0)
          return(0);
      }
      if ((bioskey(0) & 0xff) == 27)
        return(-1);
    }
  }
  return(0);
}


/********************************************************************
*
*  Function:  retrycount()
*
*  Desc    :
*
*  Returns :
*/

unsigned pascal retrycount(unsigned Count, char *Path, char *Str) {
  if (ExtendedError == 83) {
    ExtendedError  = Int24Error + 19;  /* errors 19-31 correspond to INT24 errors */
    ExtendedAction = Int24Flags;
  }

  if (ExtendedAction > 2)                  /* errors below 3 should retry    */
    goto show;                             /* show error, wait for keystroke */
  else {

    Count++;                               /* Increment the counter          */
    if (Count > MaxRetryCount)             /* Have we gone over our limit?   */
      goto exit;                           /* If so, then exit now!          */

    if (Count < 4) {                       /* First 4 retries simply delay   */
      mydelay(25);                         /* 1/4 sec, afterwards it will go */
    } else {                               /* into an infinite retry loop    */
      if (showerror(Path,Str,TRUE) == -1)  /* show error with countdown      */
        return(0xFFFF);                    /* user abort, no show, just exit */
    }                                      /* show error, wait for keystroke */

    return(Count);
  }

show:
  if (MaxRetryCount != 0xFFFF) {            /* if it's a BAD error and the   */
    showerror(Path,Str,TRUE);               /* MaxRetryCount is adjusted then*/
    goto exit;                              /* we better just show the msg   */
  }                                         /* and then get outta here!      */

  showerror(Path,Str,FALSE);                /* show final error message */
  return(0xFFFF);

exit:
  DosErrorLevel = 1;
  return(0xFFFF);
}
