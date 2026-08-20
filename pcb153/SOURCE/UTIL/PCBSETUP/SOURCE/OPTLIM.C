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

int checkpagestart(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.SysopStart,ERR300STRT));
}

int checkpagestop(int Before) {
  if (Before)
    return(0);

  return(checktime(PcbData.SysopStop,ERR300STOP));
}

int checkscroll(int Before) {
  if (Before)
    return(0);

  if (PcbData.MaxScrollBack > 400) {
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
  addquest(P,0,vBYTE   ,LIMITSHELP+0,ALLNUM  , 3, 5, 2,Questions[FLDLIMITS+0],&PcbData.KbdTimeout      ,CLEAR  ,NULL);
  addquest(P,1,vBYTE   ,LIMITSHELP+1,ALLNUM  , 3, 6, 2,Questions[FLDLIMITS+1],&PcbData.NumDescLines    ,CLEAR  ,NULL);
  addquest(P,2,vINT    ,LIMITSHELP+2,ALLNUM  , 3, 7, 3,Questions[FLDLIMITS+2],&PcbData.MaxScrollBack   ,CLEAR  ,checkscroll);
  addquest(P,3,vINT    ,LIMITSHELP+3,ALLNUM  , 3, 8, 4,Questions[FLDLIMITS+3],&PcbData.EnvSize         ,CLEAR  ,NULL);

  addquest(P,4,vINT    ,LIMITSHELP+4,ALLNUM  , 3,10, 3,Questions[FLDLIMITS+4],&PcbData.PwrdUpdate      ,CLEAR  ,NULL);
  addquest(P,5,vINT    ,LIMITSHELP+5,ALLNUM  , 3,11, 3,Questions[FLDLIMITS+5],&PcbData.PwrdWarn        ,CLEAR  ,NULL);
  addquest(P,6,vBYTE   ,LIMITSHELP+6,ALLNUM  , 3,12, 3,Questions[FLDLIMITS+6],&PcbData.MinPwrdLen      ,CLEAR  ,NULL);

  addquest(P,7,vSTR    ,LIMITSHELP+7,ALLTIME , 3,14, 5,Questions[FLDLIMITS+7],PcbData.SysopStart,CLEAR,checkpagestart);
  addquest(P,8,vSTR    ,LIMITSHELP+8,ALLTIME , 3,15, 5,Questions[FLDLIMITS+8],PcbData.SysopStop ,CLEAR,checkpagestop);
}


void pascal limits(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMLIMITS * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();


  fastprint(54,10,"(0=disable)",Colors[QUESTION]);
  readscrn(Fields,NUMLIMITS-1,0,"Configuration Options","Limits",1,NOCLEARFLD);
  freescrn(Fields,NUMLIMITS-1);
}
