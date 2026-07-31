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
*  Function: initsysopfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vSTR  ,SYSOPHELP+ 0,ALLTEXT, 3, 5,15,Questions[FLDSYSOP+ 0], PcbData.Sysop            ,CLEAR     ,NULL);
  addquest(P, 1,vUPSTR,SYSOPHELP+ 1,ALLTEXT, 3, 6,12,Questions[FLDSYSOP+ 1], PcbData.Password         ,CLEAR     ,NULL);
  addquest(P, 2,vBOOL ,SYSOPHELP+ 2,YESNO  , 3, 7, 1,Questions[FLDSYSOP+ 2],&PcbData.RequirePwrdToExit,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL ,SYSOPHELP+ 3,YESNO  , 3, 8, 1,Questions[FLDSYSOP+ 3],&PcbData.UseRealName      ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL ,SYSOPHELP+ 4,YESNO  , 3, 9, 1,Questions[FLDSYSOP+ 4],&PcbData.Graphics         ,NOCLEARFLD,NULL);
}


void pascal sysop(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMSYSOP * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMSYSOP-1,0,"Sysop Information","",1,NOCLEARFLD);
  freescrn(Fields,NUMSYSOP-1);
}
