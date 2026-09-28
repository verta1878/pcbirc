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


#include <string.h>
#include <dosfunc.h>
#include <screen.h>

#define ONESECOND     100
#define QUARTERSECOND 25

unsigned MaxRetryCount = 0xFFFF;
char DosErrorLevel     = 0;

/********************************************************************
*
*  Function:  showerror()
*
*  Desc    :  This function is called whenever a DOS error occurs when trying
*             to open or write to a file.
*
*             It sends the appropriate message to the user and/or the callers
*             log.
*
*             If Timed==FALSE then it gives the message and then quits.
*
*             If Timed==TRUE and it's a network version - then it delays 2
*             seconds between each try.
*/

int pascal showerror(char *path, char *str, bool Timed) {
  if (Timed)
    mydelay(ONESECOND);

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
    ExtendedError  = Int24Error + 41;
    ExtendedAction = 1;
  }

  if (ExtendedAction > 2) {
    showerror(Path,Str,FALSE);             /* show the error and then */
    return(0xFFFF);                        /* get outta here!         */
  }

  mydelay(QUARTERSECOND);                  /* wait 1/4 second for every retry */

  Count++;
  if (Count < 6)
    return(Count);

  if (Count == 6) {
    showerror(Path,Str,TRUE);
  } else {
    if (Count > 15)       /* check to see if we've run out of time */
      return(0xFFFF);    /* return 0xFFFF if time ran out */
  }
  return(Count);
}
