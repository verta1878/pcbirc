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
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vINT,SYSOPFLEVELS+ 0,ALLNUM,3, 5, 3,Questions[FLDSYSOPFUN+ 0],&PcbData.SysopSec[SEC_1 ],CLEAR  ,NULL);
  addquest(P, 1,vINT,SYSOPFLEVELS+ 1,ALLNUM,3, 6, 3,Questions[FLDSYSOPFUN+ 1],&PcbData.SysopSec[SEC_2 ],CLEAR  ,NULL);
  addquest(P, 2,vINT,SYSOPFLEVELS+ 2,ALLNUM,3, 7, 3,Questions[FLDSYSOPFUN+ 2],&PcbData.SysopSec[SEC_3 ],CLEAR  ,NULL);
  addquest(P, 3,vINT,SYSOPFLEVELS+ 3,ALLNUM,3, 8, 3,Questions[FLDSYSOPFUN+ 3],&PcbData.SysopSec[SEC_4 ],CLEAR  ,NULL);
  addquest(P, 4,vINT,SYSOPFLEVELS+ 4,ALLNUM,3, 9, 3,Questions[FLDSYSOPFUN+ 4],&PcbData.SysopSec[SEC_5 ],CLEAR  ,NULL);
  addquest(P, 5,vINT,SYSOPFLEVELS+ 5,ALLNUM,3,10, 3,Questions[FLDSYSOPFUN+ 5],&PcbData.SysopSec[SEC_6 ],CLEAR  ,NULL);
  addquest(P, 6,vINT,SYSOPFLEVELS+ 6,ALLNUM,3,11, 3,Questions[FLDSYSOPFUN+ 6],&PcbData.SysopSec[SEC_7 ],CLEAR  ,NULL);
  addquest(P, 7,vINT,SYSOPFLEVELS+ 7,ALLNUM,3,12, 3,Questions[FLDSYSOPFUN+ 7],&PcbData.SysopSec[SEC_8 ],CLEAR  ,NULL);
  addquest(P, 8,vINT,SYSOPFLEVELS+ 8,ALLNUM,3,13, 3,Questions[FLDSYSOPFUN+ 8],&PcbData.SysopSec[SEC_9 ],CLEAR  ,NULL);
  addquest(P, 9,vINT,SYSOPFLEVELS+ 9,ALLNUM,3,14, 3,Questions[FLDSYSOPFUN+ 9],&PcbData.SysopSec[SEC_10],CLEAR  ,NULL);
  addquest(P,10,vINT,SYSOPFLEVELS+10,ALLNUM,3,15, 3,Questions[FLDSYSOPFUN+10],&PcbData.SysopSec[SEC_11],CLEAR  ,NULL);
  addquest(P,11,vINT,SYSOPFLEVELS+11,ALLNUM,3,16, 3,Questions[FLDSYSOPFUN+11],&PcbData.SysopSec[SEC_12],CLEAR  ,NULL);
  addquest(P,12,vINT,SYSOPFLEVELS+12,ALLNUM,3,17, 3,Questions[FLDSYSOPFUN+12],&PcbData.SysopSec[SEC_13],CLEAR  ,NULL);
  addquest(P,13,vINT,SYSOPFLEVELS+13,ALLNUM,3,18, 3,Questions[FLDSYSOPFUN+13],&PcbData.SysopSec[SEC_14],CLEAR  ,NULL);
  addquest(P,14,vINT,SYSOPFLEVELS+14,ALLNUM,3,19, 3,Questions[FLDSYSOPFUN+14],&PcbData.SysopSec[SEC_15],CLEAR  ,NULL);
}


void pascal sysopfunctions(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMSYSOPFUN * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMSYSOPFUN-1,0,"Security Levels","Sysop Functions",1,NOCLEARFLD);
  freescrn(Fields,NUMSYSOPFUN-1);
}
