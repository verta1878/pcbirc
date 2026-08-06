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


#ifdef FIDO

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

/*
static int checkfidostart(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.FidoMailStart,0));
}


static int checkfidostop(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.FidoMailEnd,0));
}
*/

/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBOOL ,FIDOCONFIG+ 0,YESNO  ,3, 3, 1,Questions[FLDFIDOCFG+ 0],&PcbData.EnableFido         ,NOCLEARFLD,NULL);
  addquest(P, 1,vUPSTR,FIDOCONFIG+ 1,ALLFILE,3, 4,32,Questions[FLDFIDOCFG+ 1], PcbData.FidoLoc            ,NOCLEARFLD,backslash);
  addquest(P, 2,vBOOL ,FIDOCONFIG+ 3,YESNO  ,3, 6, 1,Questions[FLDFIDOCFG+ 2],&PcbData.FidoImportAfterXfer,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL ,FIDOCONFIG+ 4,YESNO  ,3, 7, 1,Questions[FLDFIDOCFG+ 3],&PcbData.FidoProcessIn      ,NOCLEARFLD,NULL);
  addquest(P, 4,vINT  ,FIDOCONFIG+ 5,ALLNUM ,3, 8, 4,Questions[FLDFIDOCFG+ 4],&PcbData.FidoPktFreq        ,CLEAR     ,NULL);
  addquest(P, 5,vBOOL ,FIDOCONFIG+ 6,YESNO  ,3,10, 1,Questions[FLDFIDOCFG+ 5],&PcbData.FidoProcessOut     ,NOCLEARFLD,NULL);
  addquest(P, 6,vINT  ,FIDOCONFIG+ 7,ALLNUM ,3,11, 4,Questions[FLDFIDOCFG+ 6],&PcbData.FidoExportFreq     ,CLEAR     ,NULL);
  addquest(P, 7,vBOOL ,FIDOCONFIG+ 8,YESNO  ,3,13, 1,Questions[FLDFIDOCFG+ 7],&PcbData.FidoDialOut        ,NOCLEARFLD,NULL);
  addquest(P, 8,vINT  ,FIDOCONFIG+ 9,ALLNUM ,3,14, 4,Questions[FLDFIDOCFG+ 8],&PcbData.FidoMailFreq       ,CLEAR     ,NULL);
  addquest(P, 9,vINT  ,FIDOCONFIG+10,ALLNUM ,3,16, 5,Questions[FLDFIDOCFG+ 9],&PcbData.FidoDefaultZone    ,CLEAR     ,NULL);
  addquest(P,10,vINT  ,FIDOCONFIG+11,ALLNUM ,3,17, 5,Questions[FLDFIDOCFG+10],&PcbData.FidoDefaultNet     ,CLEAR     ,NULL);
  addquest(P,11,vBYTE ,FIDOCONFIG+12,ALLNUM ,3,18, 3,Questions[FLDFIDOCFG+11],&PcbData.FidoLogLevel       ,CLEAR     ,NULL);
}


void pascal fidosetup(void) {
  FldType  *Fields;
  char      oldconfig[80];

  strcpy(oldconfig,PcbData.FidoConfig);

  if ((Fields = (FldType *) mallochk(NUMFIDOCFG * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMFIDOCFG-1,0,"Fido Processing Configuration","",1,NOCLEARFLD);
  freescrn(Fields,NUMFIDOCFG-1);

  if(PcbData.FidoConfig[0]==NULL || PcbData.FidoConfig[0]==0x20)
    strcpy(PcbData.FidoConfig,oldconfig);
}
#endif
