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
#include <country.h>
#include <account.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

static void near pascal editratefile(void);

static char PeakDays[8];

static int checkpeakstart(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.PeakStart,0));
}


static int checkpeakstop(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.PeakEnd,0));
}


static int jumpinfo(int Before) {
  showeditmsg2(Before,"ACTINFO");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpwarn(int Before) {
  showeditmsg2(Before,"ACTWARN");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}


static int jumpbye(int Before) {
  showeditmsg2(Before,"ACTBYE");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}


static int editrates(int Before) {
  showeditmsg(Before,"ACCOUNT.CFG");
  if (Before)
    return(0);

  if (checkfiles8(Before) == -1)
    return(-1);

  if (KeyFlags == FLAG3)
    editratefile();

  return(0);
}


static int editholidays(int Before) {
  showeditmsg(Before,"HOLIDAYS.CFG");
  if (Before)
    return(0);

  if (checkfiles8(Before) == -1)
    return(-1);

  if (KeyFlags == FLAG3)
    editdays(PcbData.HolidaysFile);

  return(0);
}


static void near pascal initdeffields(FldType *P) {
  addquest(P, 0,vBOOL ,ACTCFGHELP+ 0,YESNO  , 3, 5, 1,Questions[FLDACCTDEF+ 0],&PcbData.EnableAccounting  ,CLEAR     ,NULL);
  addquest(P, 1,vBOOL ,ACTCFGHELP+ 1,YESNO  , 3, 6, 1,Questions[FLDACCTDEF+ 1],&PcbData.ShowCurrency      ,CLEAR     ,NULL);
  addquest(P, 2,vBOOL ,ACTCFGHELP+ 2,YESNO  , 3, 7, 1,Questions[FLDACCTDEF+ 2],&PcbData.Concurrent        ,CLEAR     ,NULL);
  addquest(P, 3,vBOOL ,ACTCFGHELP+ 4,YESNO  , 3, 8, 1,Questions[FLDACCTDEF+ 3],&PcbData.IgnoreDropSecLevel,CLEAR     ,NULL);
  addquest(P, 4,vSTR  ,ACTCFGHELP+ 5,ALLTIME, 3, 9, 5,Questions[FLDACCTDEF+ 4],&PcbData.PeakStart         ,CLEAR     ,checkpeakstart);
  addquest(P, 5,vSTR  ,ACTCFGHELP+ 5,ALLTIME, 3,10, 5,Questions[FLDACCTDEF+ 5],&PcbData.PeakEnd           ,CLEAR     ,checkpeakstop);
  addquest(P, 6,vUPSTR,ACTCFGHELP+ 6,YESNO  , 3,11, 7,Questions[FLDACCTDEF+ 6], PeakDays                  ,NOCLEARFLD,NULL);

  addquest(P, 7,vUPSTR,ACTCFGHELP+ 7,ALLFILE, 3,13,30,Questions[FLDACCTDEF+ 7],PcbData.HolidaysFile        ,NOCLEARFLD,editholidays);
  addquest(P, 8,vUPSTR,ACTCFGHELP+ 8,ALLFILE, 3,14,30,Questions[FLDACCTDEF+ 8],PcbData.AccountConfig      ,NOCLEARFLD,editrates);
  addquest(P, 9,vUPSTR,ACTCFGHELP+ 9,ALLFILE, 3,15,30,Questions[FLDACCTDEF+ 9],PcbData.AccountTrack       ,NOCLEARFLD,checkfiles8);
  addquest(P,10,vUPSTR,ACTCFGHELP+10,ALLFILE, 3,16,30,Questions[FLDACCTDEF+10],PcbData.AccountInfo        ,NOCLEARFLD,jumpinfo);
  addquest(P,11,vUPSTR,ACTCFGHELP+11,ALLFILE, 3,17,30,Questions[FLDACCTDEF+11],PcbData.AccountWarn        ,NOCLEARFLD,jumpwarn);
  addquest(P,12,vUPSTR,ACTCFGHELP+12,ALLFILE, 3,18,30,Questions[FLDACCTDEF+12],PcbData.AccountLogoff      ,NOCLEARFLD,jumpbye);
}


static void near pascal initratefields(FldType *P) {
  addquest(P, 0,vFLOATD,ACTRATEHELP+ 0,ALLNUM , 3, 3,12,Questions[FLDACCTRATES+ 0],&AccountRates.NewUserBalance          ,CLEAR,NULL);
  addquest(P, 1,vFLOATD,ACTRATEHELP+ 1,ALLNUM , 3, 4,12,Questions[FLDACCTRATES+ 1],&AccountRates.WarnLevel               ,CLEAR,NULL);
  addquest(P, 2,vFLOATD,ACTRATEHELP+ 2,ALLNUM , 3, 7,12,Questions[FLDACCTRATES+ 2],&AccountRates.ChargeForCall           ,CLEAR,NULL);
  addquest(P, 3,vFLOATD,ACTRATEHELP+ 3,ALLNUM , 3, 8,12,Questions[FLDACCTRATES+ 3],&AccountRates.ChargeForTime           ,CLEAR,NULL);
  addquest(P, 4,vFLOATD,ACTRATEHELP+ 4,ALLNUM , 3, 9,12,Questions[FLDACCTRATES+ 4],&AccountRates.ChargeForPeakTime       ,CLEAR,NULL);
  addquest(P, 5,vFLOATD,ACTRATEHELP+ 5,ALLNUM , 3,10,12,Questions[FLDACCTRATES+ 5],&AccountRates.ChargeForGroupChat      ,CLEAR,NULL);
  addquest(P, 6,vFLOATD,ACTRATEHELP+ 6,ALLNUM , 3,11,12,Questions[FLDACCTRATES+ 6],&AccountRates.ChargeForMsgRead        ,CLEAR,NULL);
  addquest(P, 7,vFLOATD,ACTRATEHELP+ 7,ALLNUM , 3,12,12,Questions[FLDACCTRATES+ 7],&AccountRates.ChargeForMsgReadCapture ,CLEAR,NULL);
  addquest(P, 8,vFLOATD,ACTRATEHELP+ 8,ALLNUM , 3,13,12,Questions[FLDACCTRATES+ 8],&AccountRates.ChargeForMsgWrite       ,CLEAR,NULL);
  addquest(P, 9,vFLOATD,ACTRATEHELP+ 9,ALLNUM , 3,14,12,Questions[FLDACCTRATES+ 9],&AccountRates.ChargeForMsgWriteEchoed ,CLEAR,NULL);
  addquest(P,10,vFLOATD,ACTRATEHELP+10,ALLNUM , 3,15,12,Questions[FLDACCTRATES+10],&AccountRates.ChargeForMsgWritePrivate,CLEAR,NULL);
  addquest(P,11,vFLOATD,ACTRATEHELP+11,ALLNUM , 3,16,12,Questions[FLDACCTRATES+11],&AccountRates.ChargeForDownloadFile   ,CLEAR,NULL);
  addquest(P,12,vFLOATD,ACTRATEHELP+12,ALLNUM , 3,17,12,Questions[FLDACCTRATES+12],&AccountRates.ChargeForDownloadBytes  ,CLEAR,NULL);
  addquest(P,13,vFLOATD,ACTRATEHELP+13,ALLNUM , 3,20,12,Questions[FLDACCTRATES+13],&AccountRates.PayBackForUploadFile    ,CLEAR,NULL);
  addquest(P,14,vFLOATD,ACTRATEHELP+14,ALLNUM , 3,21,12,Questions[FLDACCTRATES+14],&AccountRates.PayBackForUploadBytes   ,CLEAR,NULL);
}


static void near pascal editratefile(void) {
  FldType         *Fields;
  accountratetype  Save;
  savescrntype     Screen;

  savescreen(&Screen);
  readaccountrates();

  if ((Fields = (FldType *) mallochk(NUMACCTRATES * sizeof(FldType))) == NULL)
    return;

  initratefields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint(3, 6,"Charges:",Colors[DISPLAY]);
  fastprint(3,19,"Pay Back:",Colors[DISPLAY]);

  Save = AccountRates;

top:
  readscrn(Fields,NUMACCTRATES-1,0,"Accounting Rates Configuration","",1,NOCLEARFLD);

  if (memcmp(&AccountRates,&Save,sizeof(accountratetype)) != 0) {
    switch (editexit("Accounting Rates Configuration")) {
      case 'Y': writeaccountrates(); break;
      case 'A': break;
      case 'N': goto top;
    }
  }

  KeyFlags = NOTHING;
  restorescreen(&Screen);
  freescrn(Fields,NUMACCTRATES-1);
}


void pascal account(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMACCTDEF * sizeof(FldType))) == NULL)
    return;

  initdeffields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint(47,10,"(SMTWRFS)",Colors[DISPLAY]);
  undogetdays(PeakDays,&PcbData.PeakDays);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */
  readscrn(Fields,NUMACCTDEF-1,0,"Accounting Configuration","",1,NOCLEARFLD);
  getdays(&PcbData.PeakDays,PeakDays);

  freescrn(Fields,NUMACCTDEF-1);
  ExitKeyNum[2] = 0;
}
