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
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


int pascal checktime(char *Str, int HelpNum) {
  int  Hour;
  int  Min;
  char *p;

  Hour = atoi(Str);
  if ((p = strchr(Str,':')) != NULL)
    Min = atoi(++p);
  else
    Min = 0;

  if (Hour == 24) {
    if (Min != 0)
      goto error;
  } else {
    if (Hour > 23 || Min > 59)
      goto error;
  }

  sprintf(Str,"%02d:%02d",Hour,Min);
  return(1);

error:
  beep();
  showhelp(HelpNum);
  return(-1);
}

int checkstart(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.AllowLowStrt,ERR300STRT));
}

int checkstop(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.AllowLowStop,ERR300STOP));
}

/********************************************************************
*
*  Function: initmodemfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P,0,vBOOL,MODEMSPEEDS+0,YESNO  , 3, 5, 1,Questions[FLDMODEMACC+0],&PcbData.Allow7E1          ,NOCLEARFLD,NULL);
  addquest(P,1,vLONG,MODEMSPEEDS+1,ALLBAUD, 3, 7, 6,Questions[FLDMODEMACC+1],&PcbData.LowBaudLimit      ,CLEAR     ,checkspeed);
  addquest(P,2,vBOOL,MODEMSPEEDS+2,YESNO  , 3, 8, 1,Questions[FLDMODEMACC+2],&PcbData.AllowLowBaud      ,NOCLEARFLD,NULL);
  addquest(P,3,vSTR ,MODEMSPEEDS+3,ALLTIME, 3, 9, 5,Questions[FLDMODEMACC+3],&PcbData.AllowLowStrt      ,CLEAR     ,checkstart);
  addquest(P,4,vSTR ,MODEMSPEEDS+4,ALLTIME, 3,10, 5,Questions[FLDMODEMACC+4],&PcbData.AllowLowStop      ,CLEAR     ,checkstop);
  addquest(P,5,vBYTE,MODEMSPEEDS+5,ALLNUM , 3,11, 3,Questions[FLDMODEMACC+5],&PcbData.LowBaudSecOverride,NOCLEARFLD,NULL);
}


void pascal modemaccess(void) {
  FldType *ModemFields;

  if ((ModemFields = (FldType *) mallochk(NUMMODEMACC * sizeof(FldType))) == NULL)
    return;

  initfields(ModemFields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(ModemFields,NUMMODEMACC-1,0,"Modem Information","Allowed Access Speeds",1,NOCLEARFLD);
  freescrn(ModemFields,NUMMODEMACC-1);
}
