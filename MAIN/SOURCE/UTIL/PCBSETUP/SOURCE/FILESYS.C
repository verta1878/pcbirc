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
#include <misc.h>
#include <pcb.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


int jumpcnames(int Before) {
  static char Old[66];
  if (Before) {
    stripright(PcbData.CnfFile,' ');
    strcpy(Old,PcbData.CnfFile);
    return(0);
  }
  stripright(PcbData.CnfFile,' ');
  if (strcmp(Old,PcbData.CnfFile) != 0) {
    beep();
    showhelp(WARNCNAMES);
  }
  return(checkfiles8(Before));
}

/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vUPSTR,SYSFILEHELP+ 0,ALLFILE,3, 5,30,Questions[FLDSYSTFILES+ 0],PcbData.CnfFile     ,NOCLEARFLD,jumpcnames);
  addquest(P, 1,vUPSTR,SYSFILEHELP+ 1,ALLFILE,3, 6,30,Questions[FLDSYSTFILES+ 1],PcbData.UsrFile     ,NOCLEARFLD,checkfiles8);
  addquest(P, 2,vUPSTR,SYSFILEHELP+ 2,ALLFILE,3, 7,30,Questions[FLDSYSTFILES+ 2],PcbData.InfFile     ,NOCLEARFLD,checkfiles8);
  addquest(P, 3,vUPSTR,SYSFILEHELP+ 3,ALLFILE,3, 8,30,Questions[FLDSYSTFILES+ 3],PcbData.ClrFile     ,NOCLEARFLD,checkfiles6);
  addquest(P, 4,vUPSTR,SYSFILEHELP+ 4,ALLFILE,3, 9,30,Questions[FLDSYSTFILES+ 4],PcbData.ChatFile    ,NOCLEARFLD,checkfiles8);
  addquest(P, 5,vUPSTR,SYSFILEHELP+ 5,ALLFILE,3,10,30,Questions[FLDSYSTFILES+ 5],PcbData.ChtLoc      ,NOCLEARFLD,backslash);
  addquest(P, 6,vUPSTR,SYSFILEHELP+ 6,ALLFILE,3,11,32,Questions[FLDSYSTFILES+ 6],PcbData.StatsFile   ,NOCLEARFLD,checkfiles8);
  addquest(P, 7,vUPSTR,SYSFILEHELP+ 7,ALLFILE,3,12,30,Questions[FLDSYSTFILES+ 7],PcbData.NetFile     ,NOCLEARFLD,checkfiles8);
  addquest(P, 8,vUPSTR,SYSFILEHELP+ 8,ALLFILE,3,13,30,Questions[FLDSYSTFILES+ 8],PcbData.DldFile     ,NOCLEARFLD,checkfiles8);
  addquest(P, 9,vUPSTR,SYSFILEHELP+ 9,ALLFILE,3,14,30,Questions[FLDSYSTFILES+ 9],PcbData.SwapPath    ,NOCLEARFLD,checkfiles8);

  addquest(P,10,vUPSTR,SYSFILEHELP+10,ALLFILE,3,16,30,Questions[FLDSYSTFILES+10],PcbData.TxtLoc      ,NOCLEARFLD,backslash);
  addquest(P,11,vUPSTR,SYSFILEHELP+11,ALLFILE,3,17,30,Questions[FLDSYSTFILES+11],PcbData.NdxLoc      ,NOCLEARFLD,backslash);
  addquest(P,12,vUPSTR,SYSFILEHELP+12,ALLFILE,3,18,30,Questions[FLDSYSTFILES+12],PcbData.TmpLoc      ,NOCLEARFLD,backslash);
  addquest(P,13,vUPSTR,SYSFILEHELP+13,ALLFILE,3,19,30,Questions[FLDSYSTFILES+13],PcbData.HlpLoc      ,NOCLEARFLD,backslash);
  addquest(P,14,vUPSTR,SYSFILEHELP+14,ALLFILE,3,20,30,Questions[FLDSYSTFILES+14],PcbData.SecLoc      ,NOCLEARFLD,backslash);
  addquest(P,15,vUPSTR,SYSFILEHELP+15,ALLFILE,3,21,30,Questions[FLDSYSTFILES+15],PcbData.CmdLoc      ,NOCLEARFLD,backslash);
}


void pascal systemfiles(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMSYSTFILES * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */
  ExitKeyNum[3] = 61;  ExitKeyFlag[3] = FLAG4;  /*  F3  */

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMSYSTFILES-1,0,"File Locations","System Files",1,NOCLEARFLD);
  freescrn(Fields,NUMSYSTFILES-1);

  ExitKeyNum[2] = 0;
  ExitKeyNum[3] = 0;
}
