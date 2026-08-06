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
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function: standardize()
*
*  Desc    : Takes as input a string Phone[] and converts it to a standard
*            format so that any sorts on phone numbers will produce meaningful
*            results.
*/

static void near pascal standardize(char Phone[]) {
  char *p,*q;
  int  Count;

  for (p = &Phone[12], Count = 0; p >= Phone; p--) {
    if (*p >= '0' && *p <= '9')
      Count++;
  }

  /* don't reformat the phone number unless it is 10 digits or less */

  if (Count <= 10) {
    /* process from right to left */
    for (p = &Phone[12], q = p; p >= Phone && q >= Phone; p--) {
      if (*p >= '0' && *p <= '9') {
        *(q--) = *p;
        if (q == &Phone[4])        *(q--) = ' ';
          else if (q == &Phone[8]) *(q--) = '-';
      }
    }

    /* pad the left of the string with spaces */
    for (; q >= Phone; q--)
      *q = ' ';
  }
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*/

static int pascal standardizesub(URead *p) {
  standardize(p->BusDataPhone);
  standardize(p->HomeVoicePhone);
  return(0);
}

/********************************************************************
*
*  Function: standardizephone()
*
*  Desc    : Scans through the users file converting all phone numbers to a
*            standard format by calling the standardize() function.
*
*/

void pascal standardizephone(void) {
  bool  Okay;
  long  Recs;

  if (! BatchMode) {
    Okay = FALSE;
    boxcls(19,18,59,22,Colors[MENUBOX],SINGLE);
    inputnum(33,20,1,"Are you sure",&Okay,vBOOL,0);
    if (KeyFlags == ESC || ! Okay)
      return;
  }

  if ((Recs = lockusersfile("Standardize Phone Number Format",NONBUFFERED)) == -1)
    return;

  setcursor(CUR_NORMAL);
  fastprintmove(3,9,"Standardizing Phone Numbers...",Colors[STATUS]);

  processusersfile(Recs,TRUE,FALSE,FALSE,NONBUFFERED,standardizesub);
}
