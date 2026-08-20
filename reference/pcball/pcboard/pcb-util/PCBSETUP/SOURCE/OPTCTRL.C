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


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBOOL,SYSCONTROL+ 0,YESNO, 3, 4, 1,Questions[FLDCTRLOPT+ 0],&PcbData.DisableQuick     ,NOCLEARFLD,NULL);
  addquest(P, 1,vBOOL,SYSCONTROL+ 1,YESNO, 3, 5, 1,Questions[FLDCTRLOPT+ 1],&PcbData.DisablePassword  ,NOCLEARFLD,NULL);
  addquest(P, 2,vBOOL,SYSCONTROL+ 2,YESNO, 3, 6, 1,Questions[FLDCTRLOPT+ 2],&PcbData.MultiLingual     ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL,SYSCONTROL+ 3,YESNO, 3, 7, 1,Questions[FLDCTRLOPT+ 3],&PcbData.AllowPwrdOnly    ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL,SYSCONTROL+ 4,YESNO, 3, 8, 1,Questions[FLDCTRLOPT+ 4],&PcbData.AllowAliasChange ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL,SYSCONTROL+ 5,YESNO, 3, 9, 1,Questions[FLDCTRLOPT+ 5],&PcbData.ClosedBoard      ,NOCLEARFLD,NULL);
  addquest(P, 6,vBOOL,SYSCONTROL+ 6,YESNO, 3,10, 1,Questions[FLDCTRLOPT+ 6],&PcbData.EnforceTime      ,NOCLEARFLD,NULL);
  addquest(P, 7,vBOOL,SYSCONTROL+ 7,YESNO, 3,11, 1,Questions[FLDCTRLOPT+ 7],&PcbData.AllowOneName     ,NOCLEARFLD,NULL);
  addquest(P, 8,vBOOL,SYSCONTROL+ 8,YESNO, 3,12, 1,Questions[FLDCTRLOPT+ 8],&PcbData.AllowPwrdComment ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL,SYSCONTROL+ 9,YESNO, 3,13, 1,Questions[FLDCTRLOPT+ 9],&PcbData.GuardLogoff      ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL,SYSCONTROL+10,YESNO, 3,14, 1,Questions[FLDCTRLOPT+10],&PcbData.AllowShell       ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL,SYSCONTROL+11,YESNO, 3,15, 1,Questions[FLDCTRLOPT+11],&PcbData.UseNewAskFile    ,NOCLEARFLD,NULL);
  addquest(P,12,vBOOL,SYSCONTROL+12,YESNO, 3,16, 1,Questions[FLDCTRLOPT+12],&PcbData.SkipProtocol     ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL,SYSCONTROL+13,YESNO, 3,17, 1,Questions[FLDCTRLOPT+13],&PcbData.SkipAlias        ,NOCLEARFLD,NULL);
  addquest(P,14,vBOOL,SYSCONTROL+14,YESNO, 3,18, 1,Questions[FLDCTRLOPT+14],&PcbData.ConfPwrdAdjust   ,NOCLEARFLD,NULL);
  addquest(P,15,vBOOL,SYSCONTROL+15,YESNO, 3,19, 1,Questions[FLDCTRLOPT+15],&PcbData.ConfirmCaller    ,NOCLEARFLD,NULL);
  addquest(P,16,vBOOL,SYSCONTROL+16,YESNO, 3,20, 1,Questions[FLDCTRLOPT+16],&PcbData.AutoRegConf      ,NOCLEARFLD,NULL);
  addquest(P,17,vBOOL,SYSCONTROL+17,YESNO, 3,21, 1,Questions[FLDCTRLOPT+17],&PcbData.Encrypt          ,NOCLEARFLD,NULL);
}


void pascal controloptions(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMCTRLOPT * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint(40,21,"WARNING! Press F1 before setting to Y.",Colors[HEADING]);

  readscrn(Fields,NUMCTRLOPT-1,0,"Configuration Options","System Control",1,NOCLEARFLD);
  freescrn(Fields,NUMCTRLOPT-1);
}
