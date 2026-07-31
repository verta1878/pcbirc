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
  int X;

  for (X = 0; X < 10; X++)
    addquest(P,X,vSTR,FUNCTIONKEYS,ALLCHAR,3,X+5,60,Questions[FLDFUNCKEYS+X],PcbData.FuncKeys[X],CLEAR,NULL);
}


void pascal functionkeys(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMFUNCKEYS * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMFUNCKEYS-1,0,"Configuration Options","Function Keys",1,NOCLEARFLD);
  freescrn(Fields,NUMFUNCKEYS-1);
}
