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
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


int checkevent(int Before) {
  int  Hour;
  int  Min;
  int  RetVal;
  char Temp[6];

  if (Before)
    return(0);

  RetVal = checktime(PcbData.EventTime,ERREVENT);

  strcpy(Temp,PcbData.EventTime);

  Hour = atoi(&Temp[0]);
  Min  = atoi(&Temp[3]);

  if (Hour == 23 && Min == 59 ||
      Hour == 24 && Min == 00 ||
      Hour == 00 && Min <= 01   ) {
    beep();
    showhelp(ERREVENT);
    return(-1);
  } else
    return(RetVal);
}


int jumpevent(int Before) {
  showeditmsg(Before,"EVENT.DAT");
  if (Before)
    return(0);

  if (checkfiles8(FALSE))
    return(-1);
  if (KeyFlags == FLAG3)
    editeventlist(PcbData.EventDatFile);
  return(TRUE);
}


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P,0,vBOOL    ,EVENTINFO+0,YESNO  , 3, 5, 1,Questions[FLDEVENT+ 0],&PcbData.EventActive    ,CLEAR,NULL);
  addquest(P,1,vUPSTR   ,EVENTINFO+1,ALLFILE ,3, 6,32,Questions[FLDEVENT+ 1],PcbData.EventDatFile    ,NOCLEARFLD,jumpevent);
  addquest(P,2,vUPSTR   ,EVENTINFO+2,ALLFILE ,3, 7,32,Questions[FLDEVENT+ 2],PcbData.EventFiles      ,NOCLEARFLD,backslash);

  addquest(P,3,vBYTE    ,EVENTINFO+3,ALLNUM , 3,11, 2,Questions[FLDEVENT+ 3],&PcbData.EventSuspend   ,CLEAR,NULL);
  addquest(P,4,vBOOL    ,EVENTINFO+4,YESNO  , 3,12, 1,Questions[FLDEVENT+ 4],&PcbData.EventStopUplds ,CLEAR,NULL);
  addquest(P,5,vBYTE    ,EVENTINFO+5,ALLNUM , 3,13, 2,Questions[FLDEVENT+ 5],&PcbData.MinPriorToEvent,CLEAR,NULL);
}


void pascal event(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMEVENT * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint(3,9,"For EXPEDITED Events:",Colors[DISPLAY]);
  readscrn(Fields,NUMEVENT-1,0,"Event Information","",1,NOCLEARFLD);
  freescrn(Fields,NUMEVENT-1);

  ExitKeyNum[2] = 0;
}
