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

int checknumconf(int Before) {
  if (Before)
    return(0);

#ifdef DEMO
  if (PcbData.NumConf >= 0 && PcbData.NumConf <= 1)
    return(0);
  beep();
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.Save    = TRUE;
  MsgData.AutoBox = TRUE;
  MsgData.Line1   = 18;
  MsgData.Msg1    = "In the DEMO version the range of valid conferences is 0 to 1";
  MsgData.Color1  = Colors[HEADING];
  showmessage();
  return(-1);
#else
  return(0);
#endif
}


int checkmsglines(int Before) {
  if (Before)
    return(0);

  if (PcbData.MaxMsgLines > 400) {
    beep();
    return(-1);
  }
  return(0);
}




/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vUNSIGNED,MSGOPTIONS+ 0,ALLNUM , 3, 4, 5,Questions[FLDMSGOPT+ 0],&PcbData.NumConf          ,CLEAR  ,checknumconf);

  addquest(P, 1,vINT ,MSGOPTIONS+ 1,ALLNUM , 3, 6, 3,Questions[FLDMSGOPT+ 1],&PcbData.MaxMsgLines      ,CLEAR  ,checkmsglines);
  addquest(P, 2,vINT ,MSGOPTIONS+ 2,ALLNUM , 3, 7, 5,Questions[FLDMSGOPT+ 2],&PcbData.MaxTotalMsgs     ,CLEAR  ,NULL);
  addquest(P, 3,vINT ,MSGOPTIONS+ 3,ALLNUM , 3, 8, 5,Questions[FLDMSGOPT+ 3],&PcbData.MaxConfMsgs      ,CLEAR  ,NULL);
  addquest(P, 4,vUPSTR   ,MSGOPTIONS+ 4,ALLFILE, 3, 9, 8,Questions[FLDMSGOPT+ 4], PcbData.CapFile          ,CLEAR  ,checkfiles8);
  addquest(P, 5,vUPSTR   ,MSGOPTIONS+ 6,ALLFILE, 3,10, 8,Questions[FLDMSGOPT+ 5], PcbData.QwkFile          ,CLEAR  ,checkfiles8n);
  addquest(P, 6,vBOOL    ,MSGOPTIONS+ 7,YESNO  , 3,11, 1,Questions[FLDMSGOPT+ 6],&PcbData.StopClockOnCap   ,CLEAR  ,NULL);

  addquest(P, 7,vBOOL    ,MSGOPTIONS+ 8,YESNO  , 3,13, 1,Questions[FLDMSGOPT+ 7],&PcbData.DisableScan     ,NOCLEARFLD,NULL);
  addquest(P, 8,vBOOL    ,MSGOPTIONS+ 9,YESNO  , 3,14, 1,Questions[FLDMSGOPT+ 8],&PcbData.AllowEscCodes   ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL    ,MSGOPTIONS+10,YESNO  , 3,15, 1,Questions[FLDMSGOPT+ 9],&PcbData.AllowCCs        ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL    ,MSGOPTIONS+11,YESNO  , 3,16, 1,Questions[FLDMSGOPT+10],&PcbData.Validate        ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL    ,MSGOPTIONS+12,YESNO   ,3,17, 1,Questions[FLDMSGOPT+11],&PcbData.ForceMain       ,NOCLEARFLD,NULL);
  addquest(P,12,vBOOL    ,MSGOPTIONS+13,YESNO   ,3,18, 1,Questions[FLDMSGOPT+12],&PcbData.Foreign         ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL    ,MSGOPTIONS+14,YESNO   ,3,19, 1,Questions[FLDMSGOPT+13],&PcbData.AutoMakeMsgs    ,NOCLEARFLD,NULL);
  addquest(P,14,vBOOL    ,MSGOPTIONS+15,YESNO  , 3,20, 1,Questions[FLDMSGOPT+14],&PcbData.QuickScan       ,CLEAR     ,NULL);
  addquest(P,15,vBOOL    ,MSGOPTIONS+17,YESNO  , 3,21, 1,Questions[FLDMSGOPT+15],&PcbData.ScanAll         ,CLEAR     ,NULL);
  addquest(P,16,vBOOL    ,MSGOPTIONS+18,YESNO  , 3,22, 1,Questions[FLDMSGOPT+16],&PcbData.PromptToReadMail,CLEAR     ,NULL);
}


void pascal messageoptions(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMMSGOPT * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMMSGOPT-1,0,"Configuration Options","Messages",1,NOCLEARFLD);
  MenuAvail[Menu[1].First+9] = (PcbData.NumConf > 0);
  PcbData.NumAreas = PcbData.NumConf + 1;
  freescrn(Fields,NUMMSGOPT-1);
}
