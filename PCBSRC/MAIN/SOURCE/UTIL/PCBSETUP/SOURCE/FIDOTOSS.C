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


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vINT     ,FIDOTOSS+ 0,ALLNUM ,3, 3, 3,Questions[FLDFIDOTOSSER+ 0],&PcbData.FidoCrashSec       ,CLEAR     ,NULL);
  addquest(P, 1,vBOOL    ,FIDOTOSS+ 1,YESNO  ,3, 4, 1,Questions[FLDFIDOTOSSER+ 1],&PcbData.FidoCreateMsg      ,NOCLEARFLD,NULL);
  addquest(P, 2,vBOOL    ,FIDOTOSS+ 2,YESNO  ,3, 5, 1,Questions[FLDFIDOTOSSER+ 2],&PcbData.FidoEnableRouting  ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL    ,FIDOTOSS+ 3,YESNO  ,3, 6, 1,Questions[FLDFIDOTOSSER+ 3],&PcbData.FidoSecure         ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL    ,FIDOTOSS+ 4,YESNO  ,3, 7, 1,Questions[FLDFIDOTOSSER+ 4],&PcbData.FidoSysopChange    ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL    ,FIDOTOSS+ 5,YESNO  ,3, 9, 1,Questions[FLDFIDOTOSSER+ 5],&PcbData.FidoCheckDupePath  ,NOCLEARFLD,NULL);
  addquest(P, 6,vBOOL    ,FIDOTOSS+ 6,YESNO  ,3,10, 1,Questions[FLDFIDOTOSSER+ 6],&PcbData.FidoCheckDupeMsgId ,NOCLEARFLD,NULL);
  addquest(P, 7,vLONG    ,FIDOTOSS+ 7,ALLNUM ,3,11, 9,Questions[FLDFIDOTOSSER+ 7],&PcbData.FidoNumMsgsToTrack ,CLEAR     ,NULL);
  addquest(P, 8,vBOOL    ,FIDOTOSS+ 8,YESNO  ,3,13, 1,Questions[FLDFIDOTOSSER+ 8],&PcbData.FidoMakeResponse   ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL    ,FIDOTOSS+ 9,YESNO  ,3,14, 1,Questions[FLDFIDOTOSSER+ 9],&PcbData.FidoEnablePassThru ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL    ,FIDOTOSS+10,YESNO  ,3,15, 1,Questions[FLDFIDOTOSSER+10],&PcbData.FidoEnableAreaFix  ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL    ,FIDOTOSS+11,YESNO  ,3,17, 1,Questions[FLDFIDOTOSSER+11],&PcbData.FidoAutoAdd        ,NOCLEARFLD,NULL);
  addquest(P,12,vBOOL    ,FIDOTOSS+12,YESNO  ,3,18, 1,Questions[FLDFIDOTOSSER+12],&PcbData.FidoReAddress      ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL    ,FIDOTOSS+13,YESNO  ,3,19, 1,Questions[FLDFIDOTOSSER+13],&PcbData.FidoRouteEchoMail  ,NOCLEARFLD,NULL);
}


void pascal fidotosser(void) {
  FldType  *Fields;

  if ((Fields = (FldType *) mallochk(NUMFIDOTOSSER * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMFIDOTOSSER-1,0,"Fido Tosser Configuration","",1,NOCLEARFLD);
  freescrn(Fields,NUMFIDOTOSSER-1);
}
#endif
