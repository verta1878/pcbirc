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

static int jumpmodfile(int Before) {
  showeditmsg(Before,"USENET.MOD");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      editmodfile();
  }
  return(0);
}

static int jumpdecompfile(int Before) {
  showeditmsg(Before,"DECOMP.BAT");
  if (! Before)
    return(externaledit(EDIT_CHECK8|EDIT_GRAPHICS));
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
  addquest(P, 0,vSTR     ,UUCPCONFIG+ 0,ALLCHAR, 3, 3,sizeof(PcbData.Organization)-5  ,Questions[FLDUUCP+ 0], PcbData.Organization  ,NOCLEARFLD,NULL);
  addquest(P, 1,vUPSTR   ,UUCPCONFIG+ 1,ALLFILE, 3, 5,sizeof(PcbData.uucpPath)-1      ,Questions[FLDUUCP+ 1], PcbData.uucpPath      ,NOCLEARFLD,backslash);
  addquest(P, 2,vUPSTR   ,UUCPCONFIG+ 2,ALLFILE, 3, 6,sizeof(PcbData.uucpSpoolPath)-1 ,Questions[FLDUUCP+ 2], PcbData.uucpSpoolPath ,NOCLEARFLD,backslash);
  addquest(P, 3,vUPSTR   ,UUCPCONFIG+ 3,ALLFILE, 3, 7,sizeof(PcbData.uucpLogPath)-1   ,Questions[FLDUUCP+ 3], PcbData.uucpLogPath   ,NOCLEARFLD,backslash);
  addquest(P, 4,vUPSTR   ,UUCPCONFIG+ 4,ALLFILE, 3, 8,sizeof(PcbData.DeCompBatFile)-1 ,Questions[FLDUUCP+ 4], PcbData.DeCompBatFile ,NOCLEARFLD,jumpdecompfile);
  addquest(P, 5,vUPSTR   ,UUCPCONFIG+ 5,ALLFILE, 3, 9,sizeof(PcbData.uucpModFile)-1   ,Questions[FLDUUCP+ 5], PcbData.uucpModFile   ,NOCLEARFLD,jumpmodfile);
  addquest(P, 6,vSTR     ,UUCPCONFIG+ 6,ALLCHAR, 3,11,sizeof(PcbData.uucpName)-1      ,Questions[FLDUUCP+ 6], PcbData.uucpName      ,NOCLEARFLD,NULL);
  addquest(P, 7,vSTR     ,UUCPCONFIG+ 7,ALLCHAR, 3,12,sizeof(PcbData.uucpDomainName)-1,Questions[FLDUUCP+ 7], PcbData.uucpDomainName,NOCLEARFLD,NULL);
  addquest(P, 8,vSTR     ,UUCPCONFIG+ 8,ALLCHAR, 3,13,sizeof(PcbData.uucpEmailHost)-1 ,Questions[FLDUUCP+ 8], PcbData.uucpEmailHost ,NOCLEARFLD,NULL);
  addquest(P, 9,vSTR     ,UUCPCONFIG+ 9,ALLCHAR, 3,14,sizeof(PcbData.uucpNewsHost)-1  ,Questions[FLDUUCP+ 9], PcbData.uucpNewsHost  ,NOCLEARFLD,NULL);
  addquest(P,10,vSTR     ,UUCPCONFIG+10,ALLCHAR, 3,15,sizeof(PcbData.uucpDefDist)-1   ,Questions[FLDUUCP+10], PcbData.uucpDefDist   ,NOCLEARFLD,NULL);
  addquest(P,11,vSTR     ,UUCPCONFIG+11,ALLNUM , 3,16,sizeof(PcbData.uucpTimeZone)-1  ,Questions[FLDUUCP+11], PcbData.uucpTimeZone  ,NOCLEARFLD,NULL);
  addquest(P,12,vSTR     ,UUCPCONFIG+12,ALLCHAR, 3,17,sizeof(PcbData.uucpSeparator)-1 ,Questions[FLDUUCP+12], PcbData.uucpSeparator ,NOCLEARFLD,NULL);
  addquest(P,13,vUNSIGNED,UUCPCONFIG+13,ALLNUM , 3,18,5                               ,Questions[FLDUUCP+13],&PcbData.uucpEmailConf ,CLEAR     ,NULL);
  addquest(P,14,vUNSIGNED,UUCPCONFIG+14,ALLNUM , 3,19,5                               ,Questions[FLDUUCP+14],&PcbData.uucpJunkConf  ,CLEAR     ,NULL);
  addquest(P,15,vBOOL    ,UUCPCONFIG+15,YESNO  , 3,20,1                               ,Questions[FLDUUCP+15],&PcbData.uucpBangDomain,NOCLEARFLD,NULL);
  addquest(P,16,vBOOL    ,UUCPCONFIG+16,YESNO  , 3,21,1                               ,Questions[FLDUUCP+16],&PcbData.uucpSubDomain ,NOCLEARFLD,NULL);
  addquest(P,17,vCHAR    ,UUCPCONFIG+17,HighAsciiMask, 3,22,1                         ,Questions[FLDUUCP+17],&PcbData.uucpHighAscii ,NOCLEARFLD,NULL);
}


void pascal uucp(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMUUCP * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMUUCP-1,0,"UUCP Configuration","",1,NOCLEARFLD);
  freescrn(Fields,NUMUUCP-1);

  ExitKeyNum[2] = 0;
}
