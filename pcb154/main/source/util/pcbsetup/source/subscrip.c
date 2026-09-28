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

/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P,0,vBOOL,SUBSCPTHELP+0,YESNO ,3,5,1,Questions[FLDSUB+0],&PcbData.SubscriptMode   ,NOCLEARFLD,NULL);
  addquest(P,1,vINT ,SUBSCPTHELP+1,ALLNUM,3,6,4,Questions[FLDSUB+1],&PcbData.SubscriptionDays,CLEAR     ,NULL);
  addquest(P,2,vBYTE,SUBSCPTHELP+2,ALLNUM,3,7,3,Questions[FLDSUB+2],&PcbData.DefExpiredLevel ,CLEAR     ,NULL);
  addquest(P,3,vINT ,SUBSCPTHELP+3,ALLNUM,3,8,4,Questions[FLDSUB+3],&PcbData.SubscriptWarning,CLEAR     ,NULL);
}


void pascal subscription(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMSUB * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMSUB-1,0,"Subscription Information","",1,NOCLEARFLD);
  freescrn(Fields,NUMSUB-1);
}
