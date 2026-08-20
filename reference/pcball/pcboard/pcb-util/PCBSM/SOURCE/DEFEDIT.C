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


#include <alloc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#include <help.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMDEFEDITFIELDS 2

static FldType DefEditFields[NUMDEFEDITFIELDS] = {
  {vUPSTR,ALLFILE,EDITORS,3,5,24,NOCLEARFLD,"Text Editor    ","",smConfig.TextEdit,NULL},
  {vUPSTR,ALLFILE,EDITORS,3,6,24,NOCLEARFLD,"Graphics Editor","",smConfig.GrphEdit,NULL}
};

void pascal defineeditors(void) {
  initquest(DefEditFields,NUMDEFEDITFIELDS);
  clsbox(1,1,78,23,Colors[OUTBOX]);
  readscrn(DefEditFields,NUMDEFEDITFIELDS-1,0,MainHead1,"Edit Configuration File",1,NOCLEARFLD);  //lint !e534
  freeanswers(DefEditFields,NUMDEFEDITFIELDS);

  stripright(smConfig.TextEdit,' ');
  stripright(smConfig.GrphEdit,' ');
  writeconfig(&smConfig,ConfigName);
}


