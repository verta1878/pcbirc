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


#include <stdlib.h>
#include <screen.h>
#include <system.h>
#include "scrnio.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: timedinkey()
*
*  - Calls bgetkey() to read the keyboard
*  - if Seconds is >0 then timedinkey() waits for either a key stroke or
*    for the number of requested seconds to pass before exiting
*  - determines if keypressed returned a normal or "extended" code
*  - returns ascii character for keypressed (or extended code)
*  - returns with Ext set to TRUE if it was extended otherwise FALSE
*/

char LIBENTRY timedinkey(char *Ext, int Seconds) {
  union {
    int  W;
    struct {
      char LoB,HiB;
    } B;
  } Key;

  long StartTime;
  long CurTime;

  StartTime = getseconds();

  while (1) {
    CurTime = getseconds();
    if (bgetkey(1)) {
      Key.W=bgetkey(0);
      if (! Key.B.LoB) {
        *Ext = TRUE;
        Key.B.LoB = Key.B.HiB;
      } else *Ext = FALSE;
      return(Key.B.LoB);
    }
    if (labs(StartTime - CurTime) >= Seconds) {
      *Ext = FALSE;
      return(0);
    }
    giveup();
    giveup();
  }

  return(0);  /* it'll never get here */
}
