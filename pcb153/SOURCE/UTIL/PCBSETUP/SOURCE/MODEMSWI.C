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
#include <misc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function: initmodemfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBOOL   ,MODEMSWITCHES+ 0,YESNO , 3, 4, 1,Questions[FLDMODEMSWI+ 0],&PcbData.DisableCTS    ,NOCLEARFLD,NULL);
  addquest(P, 1,vBOOL   ,MODEMSWITCHES+ 1,YESNO , 3, 5, 1,Questions[FLDMODEMSWI+ 1],&PcbData.DisableCTSdrop,NOCLEARFLD,NULL);
  addquest(P, 2,vBOOL   ,MODEMSWITCHES+ 2,YESNO , 3, 6, 1,Questions[FLDMODEMSWI+ 2],&PcbData.FastComm      ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL   ,MODEMSWITCHES+ 3,YESNO  ,3, 7, 1,Questions[FLDMODEMSWI+ 3],&PcbData.AutoReset     ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL   ,MODEMSWITCHES+ 4,YESNO , 3, 8, 1,Questions[FLDMODEMSWI+ 4],&PcbData.ResetModem    ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL   ,MODEMSWITCHES+ 5,YESNO , 3, 9, 1,Questions[FLDMODEMSWI+ 5],&PcbData.OffHook       ,NOCLEARFLD,NULL);
  addquest(P, 6,vINT    ,MODEMSWITCHES+ 6,ALLNUM, 3,10, 1,Questions[FLDMODEMSWI+ 6],&PcbData.ModemDelay    ,NOCLEARFLD,NULL);
  addquest(P, 7,vBOOL   ,MODEMSWITCHES+ 7,YESNO , 3,11, 1,Questions[FLDMODEMSWI+ 7],&PcbData.Packet        ,NOCLEARFLD,NULL);
  addquest(P, 8,vBOOL   ,MODEMSWITCHES+ 8,YESNO  ,3,12, 1,Questions[FLDMODEMSWI+ 8],&PcbData.VerifyCDLoss  ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL   ,MODEMSWITCHES+10,YESNO , 3,13, 1,Questions[FLDMODEMSWI+ 9],&PcbData.LeaveDtrUp    ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL   ,MODEMSWITCHES+11,YESNO , 3,14, 1,Questions[FLDMODEMSWI+10],&PcbData.NoCarrierExit ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL   ,MODEMSWITCHES+12,YESNO , 3,15, 1,Questions[FLDMODEMSWI+11],&PcbData.AnswerRing    ,NOCLEARFLD,NULL);
  addquest(P,12,vBYTE   ,MODEMSWITCHES+13,ALLNUM, 3,16, 1,Questions[FLDMODEMSWI+12],&PcbData.RingsRequired ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL   ,MODEMSWITCHES+14,YESNO  ,3,17, 1,Questions[FLDMODEMSWI+13],&PcbData.MonitorModem  ,NOCLEARFLD,NULL);
  addquest(P,14,vBOOL   ,MODEMSWITCHES+16,YESNO  ,3,18, 1,Questions[FLDMODEMSWI+14],&PcbData.No16550       ,NOCLEARFLD,NULL);
  addquest(P,15,vBOOL   ,MODEMSWITCHES+17,YESNO  ,3,19, 1,Questions[FLDMODEMSWI+15],&PcbData.Force16550A   ,NOCLEARFLD,NULL);
  addquest(P,16,vBOOL   ,MODEMSWITCHES+18,YESNO , 3,20, 1,Questions[FLDMODEMSWI+16],&PcbData.ShareIRQs     ,NOCLEARFLD,NULL);
}


void pascal modemswitches(void) {
  FldType *ModemFields;
  char    *p;
  int      LineNum;

  if ((ModemFields = (FldType *) mallochk(NUMMODEMSWI * sizeof(FldType))) == NULL)
    return;

  initfields(ModemFields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  LineNum = 22;
  if ((p = getenv("PCB")) != NULL) {
    strupr(p);
    if (strstr(p,"/PAD") != NULL) {
      fastprint(40,11,"* PAD Operation Enabled",Colors[HEADING]);
      fastprint(3,LineNum++,"* The PAD Setting is overridden by the PCB= environment.",Colors[HEADING]);
    }
    if (strstr(p,"/NO16550") != NULL) {
      fastprint(40,18,"* 16550 Operation Disabled",Colors[HEADING]);
      fastprint(3,LineNum++,"* The 16550 Setting is overridden by the PCB= environment.",Colors[HEADING]);
    }
  }

  readscrn(ModemFields,NUMMODEMSWI-1,0,"Modem Information","Configuration Switches",1,NOCLEARFLD);
  freescrn(ModemFields,NUMMODEMSWI-1);
}
