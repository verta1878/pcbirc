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

int jumpregfile(int Before) {
  showeditmsg2(Before,"the NEWASK");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK8|EDIT_GRAPHICS));
}

int jumpansfile(int Before) {
  showeditmsg(Before,"the NEWREG");
  if (Before)
    return(0);
  return(externaledit(EDIT_NONE));
}

int jumplogonscrp(int Before) {
  showeditmsg2(Before,"the LOGON");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK8|EDIT_GRAPHICS));
}

int jumplogonansw(int Before) {
  showeditmsg(Before,"the LOGON.ANS");
  if (Before)
    return(0);
  return(externaledit(EDIT_NONE));
}

int jumplogoffscrp(int Before) {
  showeditmsg2(Before,"the LOGOFF");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK8|EDIT_GRAPHICS));
}

int jumplogoffansw(int Before) {
  showeditmsg(Before,"the LOGOFF.ANS");
  if (Before)
    return(0);
  return(externaledit(EDIT_NONE));
}

/********************************************************************
*
*  Function: initfile2fields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P,0,vUPSTR,QSTFILEHELP+0,ALLFILE,3, 6,30,Questions[FLDQUESFILES+0],PcbData.RegFile  ,NOCLEARFLD,jumpregfile);
  addquest(P,1,vUPSTR,QSTFILEHELP+1,ALLFILE,3, 7,30,Questions[FLDQUESFILES+1],PcbData.AnsFile  ,NOCLEARFLD,jumpansfile);
  addquest(P,2,vUPSTR,QSTFILEHELP+2,ALLFILE,3, 9,30,Questions[FLDQUESFILES+2],PcbData.LogOnScr ,NOCLEARFLD,jumplogonscrp);
  addquest(P,3,vUPSTR,QSTFILEHELP+3,ALLFILE,3,10,30,Questions[FLDQUESFILES+3],PcbData.LogOnAns ,NOCLEARFLD,jumplogonansw);
  addquest(P,4,vUPSTR,QSTFILEHELP+4,ALLFILE,3,12,30,Questions[FLDQUESFILES+4],PcbData.LogOffScr,NOCLEARFLD,jumplogoffscrp);
  addquest(P,5,vUPSTR,QSTFILEHELP+5,ALLFILE,3,13,30,Questions[FLDQUESFILES+5],PcbData.LogOffAns,NOCLEARFLD,jumplogoffansw);
}


void pascal questionnairefiles(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMQUESFILES * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */
  ExitKeyNum[3] = 61;  ExitKeyFlag[3] = FLAG4;  /*  F3  */

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMQUESFILES-1,0,"File Locations","NewUser/Logon/Logoff Questionnaires",1,NOCLEARFLD);
  freescrn(Fields,NUMQUESFILES-1);

  ExitKeyNum[2] = 0;
  ExitKeyNum[3] = 0;
}
