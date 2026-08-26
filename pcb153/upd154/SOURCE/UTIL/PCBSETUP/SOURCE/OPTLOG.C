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
  addquest(P,0,vBOOL,LOGOPTIONS+0,YESNO  , 3, 5, 1,Questions[FLDLOGOPT+0],&PcbData.LogCallerNumber ,NOCLEARFLD,NULL);
  addquest(P,1,vBOOL,LOGOPTIONS+1,YESNO  , 3, 6, 1,Questions[FLDLOGOPT+1],&PcbData.LogConnectStr   ,NOCLEARFLD,NULL);
  addquest(P,2,vBOOL,LOGOPTIONS+2,YESNO  , 3, 7, 1,Questions[FLDLOGOPT+2],&PcbData.LogSecLevel     ,NOCLEARFLD,NULL);
}


void pascal loggingoptions(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMLOGOPT * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMLOGOPT-1,0,"Configuration Options","Callers Log",1,NOCLEARFLD);
  freescrn(Fields,NUMLOGOPT-1);
}
