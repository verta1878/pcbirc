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
  addquest(P, 0,vBOOL,SWITCHES+ 0,YESNO, 3, 4, 1,Questions[FLDSYSOPT+ 0],&PcbData.DisableCls      ,NOCLEARFLD,NULL);
  addquest(P, 1,vBOOL,SWITCHES+ 1,YESNO, 3, 5, 1,Questions[FLDSYSOPT+ 1],&PcbData.DisableEdits    ,NOCLEARFLD,NULL);
  addquest(P, 2,vBOOL,SWITCHES+ 2,YESNO, 3, 6, 1,Questions[FLDSYSOPT+ 2],&PcbData.DisableFilter   ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL,SWITCHES+ 3,YESNO, 3, 7, 1,Questions[FLDSYSOPT+ 3],&PcbData.DefaultGraphics ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL,SWITCHES+ 4,YESNO, 3, 8, 1,Questions[FLDSYSOPT+ 4],&PcbData.NonGraphics     ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL,SWITCHES+ 5,YESNO, 3, 9, 1,Questions[FLDSYSOPT+ 5],&PcbData.ExcludeLocals   ,NOCLEARFLD,NULL);
  addquest(P, 6,vBOOL,SWITCHES+ 6,YESNO, 3,10, 1,Questions[FLDSYSOPT+ 6],&PcbData.ExitToDos       ,NOCLEARFLD,NULL);
  addquest(P, 7,vBOOL,SWITCHES+ 7,YESNO, 3,11, 1,Questions[FLDSYSOPT+ 7],&PcbData.EliminateSnow   ,NOCLEARFLD,NULL);
  addquest(P, 8,vCHAR,SWITCHES+ 8,YNA  , 3,12, 1,Questions[FLDSYSOPT+ 8],&PcbData.DisplayNews     ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL,SWITCHES+ 9,YESNO, 3,13, 1,Questions[FLDSYSOPT+ 9],&PcbData.ShowSettings    ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL,SWITCHES+10,YESNO, 3,14, 1,Questions[FLDSYSOPT+10],&PcbData.ForceIntro      ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL,SWITCHES+11,YESNO, 3,15, 1,Questions[FLDSYSOPT+11],&PcbData.FastText        ,NOCLEARFLD,NULL);
  addquest(P,12,vBOOL,SWITCHES+12,YESNO, 3,16, 1,Questions[FLDSYSOPT+12],&PcbData.FastCnames      ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL,SWITCHES+13,YESNO ,3,17, 1,Questions[FLDSYSOPT+13],&PcbData.ScanBlts        ,NOCLEARFLD,NULL);
  addquest(P,14,vBOOL,SWITCHES+14,YESNO ,3,18, 1,Questions[FLDSYSOPT+14],&PcbData.Swap            ,NOCLEARFLD,NULL);
  addquest(P,15,vBOOL,SWITCHES+15,YESNO ,3,19, 1,Questions[FLDSYSOPT+15],&PcbData.SwapDuringBat   ,NOCLEARFLD,NULL);
  addquest(P,16,vBOOL,SWITCHES+16,YESNO ,3,20, 1,Questions[FLDSYSOPT+16],&PcbData.UserSysDuringBat,NOCLEARFLD,NULL);
  addquest(P,17,vBOOL,SWITCHES+17,YESNO ,3,21, 1,Questions[FLDSYSOPT+17],&PcbData.RecordGroupChat ,NOCLEARFLD,NULL);
  addquest(P,18,vBOOL,SWITCHES+18,YESNO ,3,22, 1,Questions[FLDSYSOPT+18],&PcbData.AllowHandles    ,NOCLEARFLD,NULL);
}


void pascal systemoptions(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMSYSOPT * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMSYSOPT-1,0,"Configuration Options","Switches",1,NOCLEARFLD);
  freescrn(Fields,NUMSYSOPT-1);
}
