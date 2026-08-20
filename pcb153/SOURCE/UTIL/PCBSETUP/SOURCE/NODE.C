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
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

int checktimeout(int Before) {
  if (Before)
    return(0);

  if (PcbData.NetTimeout >= 20 && PcbData.NetTimeout <= 99)
    return(0);

  beep();
  showhelp(ERRNETTIME);
  return(-1);
}

int checkdelay(int Before) {
  if (Before)
    return(0);

  if (PcbData.ChatDelay >= 5 && PcbData.ChatDelay <= 99)
    return(0);

  beep();
  showhelp(ERRCHATDLY);
  return(-1);
}


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vSTR     ,NODEINFO+ 0,ALLCHAR, 3, 5,63,Questions[FLDNODE+ 0],PcbData.BoardName       ,CLEAR,NULL);

  addquest(P, 1,vSTR     ,NODEINFO+ 1,ALLCHAR, 3, 7,60,Questions[FLDNODE+ 1],PcbData.Origin          ,NOCLEARFLD,NULL);

  addquest(P, 2,vBYTE    ,NODEINFO+ 2,ALLLPT , 3, 9, 1,Questions[FLDNODE+ 2],&PcbData.ParallelPortNum,CLEAR     ,NULL);

  addquest(P, 3,vBOOL    ,NODEINFO+ 3,YESNO  , 3,11, 1,Questions[FLDNODE+ 3],&PcbData.Network        ,NOCLEARFLD,NULL);
  addquest(P, 4,vINT     ,NODEINFO+ 4,ALLNUM , 3,12, 5,Questions[FLDNODE+ 4],&PcbData.NodeNum        ,CLEAR     ,NULL);
  addquest(P, 5,vBOOL    ,NODEINFO+ 5,YESNO  , 3,13, 1,Questions[FLDNODE+ 5],&PcbData.Float          ,NOCLEARFLD,NULL);
  addquest(P, 6,vBYTE    ,NODEINFO+ 6,ALLNUM , 3,14, 2,Questions[FLDNODE+ 6],&PcbData.NetTimeout     ,CLEAR     ,checktimeout);
  addquest(P, 7,vBYTE    ,NODEINFO+ 7,ALLNUM , 3,15, 2,Questions[FLDNODE+ 7],&PcbData.ChatDelay      ,CLEAR     ,checkdelay);
  addquest(P, 8,vBOOL    ,NODEINFO+ 8,YESNO  , 3,16, 1,Questions[FLDNODE+ 8],&PcbData.IncludeCity    ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL    ,NODEINFO+ 9,YESNO  , 3,17, 1,Questions[FLDNODE+ 9],&PcbData.ShowAlias      ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL    ,NODEINFO+10,YESNO  , 3,18, 1,Questions[FLDNODE+10],&PcbData.Slaves         ,NOCLEARFLD,NULL);
}


void pascal node(void) {
  FldType  *Fields;
  char     *p;
  unsigned  NodeNum;
  char      Str[20];

  if ((Fields = (FldType *) mallochk(NUMNODE * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  if ((p = getenv("PCB")) != NULL) {
    strupr(p);
    if ((p = strstr(p,"/NODE:")) != NULL) {
      NodeNum = atoi(p+6);
      if (NodeNum != 0) {
        sprintf(Str,"* Node Number=%d",NodeNum);
        fastprint(55,12,Str,Colors[HEADING]);
        fastprint(3,21,"* The Node Number is currently overridden by the PCB= environment.",Colors[HEADING]);
      }
    }
  }

  readscrn(Fields,NUMNODE-1,0,"Node Information","",1,NOCLEARFLD);
  freescrn(Fields,NUMNODE-1);
}
