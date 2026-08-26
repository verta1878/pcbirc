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
  addquest(P, 0,vINT,SYSOPCLEVELS+ 0,ALLNUM, 3, 5, 3,Questions[FLDSYSOPCOM+ 0],&PcbData.SysopSec[SEC_SYSOPLEVEL  ],CLEAR  ,NULL);
  addquest(P, 1,vINT,SYSOPCLEVELS+ 1,ALLNUM, 3, 6, 3,Questions[FLDSYSOPCOM+ 1],&PcbData.SysopSec[SEC_READCOMMENTS],CLEAR  ,NULL);
  addquest(P, 2,vINT,SYSOPCLEVELS+ 2,ALLNUM, 3, 7, 3,Questions[FLDSYSOPCOM+ 2],&PcbData.SysopSec[SEC_READALLMAIL ],CLEAR  ,NULL);
  addquest(P, 3,vINT,SYSOPCLEVELS+ 3,ALLNUM, 3, 8, 3,Questions[FLDSYSOPCOM+ 3],&PcbData.SysopSec[SEC_MOVEMSG     ],CLEAR  ,NULL);
  addquest(P, 4,vINT,SYSOPCLEVELS+ 4,ALLNUM, 3, 9, 3,Questions[FLDSYSOPCOM+ 4],&PcbData.SysopSec[SEC_SUBS        ],CLEAR  ,NULL);
  addquest(P, 5,vINT,SYSOPCLEVELS+ 5,ALLNUM, 3,10, 3,Questions[FLDSYSOPCOM+ 5],&PcbData.SysopSec[SEC_EDITALL     ],CLEAR  ,NULL);
  addquest(P, 6,vINT,SYSOPCLEVELS+ 6,ALLNUM, 3,11, 3,Questions[FLDSYSOPCOM+ 6],&PcbData.SysopSec[SEC_READONLY    ],CLEAR  ,NULL);
  addquest(P, 7,vINT,SYSOPCLEVELS+ 7,ALLNUM, 3,12, 3,Questions[FLDSYSOPCOM+ 7],&PcbData.SysopSec[SEC_BROADCAST   ],CLEAR  ,NULL);
  addquest(P, 8,vINT,SYSOPCLEVELS+ 8,ALLNUM, 3,13, 3,Questions[FLDSYSOPCOM+ 8],&PcbData.SysopSec[SEC_VIEWPRIV    ],CLEAR  ,NULL);
  addquest(P, 9,vINT,SYSOPCLEVELS+ 9,ALLNUM, 3,14, 3,Questions[FLDSYSOPCOM+ 9],&PcbData.SysopSec[SEC_GENERICMSGS ],CLEAR  ,NULL);
  addquest(P,10,vINT,SYSOPCLEVELS+10,ALLNUM, 3,15, 3,Questions[FLDSYSOPCOM+10],&PcbData.SysopSec[SEC_EDITHDR     ],CLEAR  ,NULL);
  addquest(P,11,vINT,SYSOPCLEVELS+11,ALLNUM, 3,16, 3,Questions[FLDSYSOPCOM+11],&PcbData.SysopSec[SEC_PROTECT     ],CLEAR  ,NULL);
  addquest(P,12,vINT,SYSOPCLEVELS+12,ALLNUM, 3,17, 3,Questions[FLDSYSOPCOM+12],&PcbData.SysopSec[SEC_OVERWRITE   ],CLEAR  ,NULL);
  addquest(P,13,vINT,SYSOPCLEVELS+13,ALLNUM, 3,18, 3,Questions[FLDSYSOPCOM+13],&PcbData.SysopSec[SEC_KEEPMSG     ],CLEAR  ,NULL);
  addquest(P,14,vINT,SYSOPCLEVELS+14,ALLNUM, 3,19, 3,Questions[FLDSYSOPCOM+14],&PcbData.SysopSec[SEC_SEERETRCPT  ],CLEAR  ,NULL);
}


void pascal sysopcommands(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMSYSOPCOM * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMSYSOPCOM-1,0,"Security Levels","Sysop Commands",1,NOCLEARFLD);
  freescrn(Fields,NUMSYSOPCOM-1);
}
