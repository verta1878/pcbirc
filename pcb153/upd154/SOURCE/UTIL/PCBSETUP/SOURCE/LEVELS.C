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

int checkseclevel(int Before) {
  int Num;

  if (Before)
    return(0);

  Num = atoi(SaveFldPtr->Answer);
  if (Num >= 0 && Num <= 255)
    return(0);

  beep();
  showhelp(ERRSECLEVEL);
  return(-1);
}

/********************************************************************
*
*  Function: initlevelsfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBYTE,USERLEVELS+ 0,ALLNUM , 3, 3, 3,Questions[FLDLEVELS+ 0],&PcbData.UserLevels[SEC_A],CLEAR,checkseclevel);
  addquest(P, 1,vBYTE,USERLEVELS+ 1,ALLNUM , 3, 4, 3,Questions[FLDLEVELS+ 1],&PcbData.UserLevels[SEC_B],CLEAR,checkseclevel);
  addquest(P, 2,vBYTE,USERLEVELS+ 2,ALLNUM , 3, 5, 3,Questions[FLDLEVELS+ 2],&PcbData.UserLevels[SEC_C],CLEAR,checkseclevel);
  addquest(P, 3,vBYTE,USERLEVELS+ 3,ALLNUM , 3, 6, 3,Questions[FLDLEVELS+ 3],&PcbData.UserLevels[SEC_D],CLEAR,checkseclevel);
  addquest(P, 4,vBYTE,USERLEVELS+ 4,ALLNUM , 3, 7, 3,Questions[FLDLEVELS+ 4],&PcbData.UserLevels[SEC_E],CLEAR,checkseclevel);
  addquest(P, 5,vBYTE,USERLEVELS+ 5,ALLNUM , 3, 8, 3,Questions[FLDLEVELS+ 5],&PcbData.UserLevels[SEC_F],CLEAR,checkseclevel);
  addquest(P, 6,vBYTE,USERLEVELS+ 6,ALLNUM , 3, 9, 3,Questions[FLDLEVELS+ 6],&PcbData.UserLevels[SEC_H],CLEAR,checkseclevel);
  addquest(P, 7,vBYTE,USERLEVELS+ 7,ALLNUM , 3,10, 3,Questions[FLDLEVELS+ 7],&PcbData.UserLevels[SEC_I],CLEAR,checkseclevel);
  addquest(P, 8,vBYTE,USERLEVELS+ 8,ALLNUM , 3,11, 3,Questions[FLDLEVELS+ 8],&PcbData.UserLevels[SEC_J],CLEAR,checkseclevel);
  addquest(P, 9,vBYTE,USERLEVELS+ 9,ALLNUM , 3,12, 3,Questions[FLDLEVELS+ 9],&PcbData.UserLevels[SEC_K],CLEAR,checkseclevel);
  addquest(P,10,vBYTE,USERLEVELS+10,ALLNUM , 3,13, 3,Questions[FLDLEVELS+10],&PcbData.UserLevels[SEC_L],CLEAR,checkseclevel);
  addquest(P,11,vBYTE,USERLEVELS+11,ALLNUM , 3,14, 3,Questions[FLDLEVELS+11],&PcbData.UserLevels[SEC_M],CLEAR,checkseclevel);
  addquest(P,12,vBYTE,USERLEVELS+12,ALLNUM , 3,15, 3,Questions[FLDLEVELS+12],&PcbData.UserLevels[SEC_N],CLEAR,checkseclevel);
  addquest(P,13,vBYTE,USERLEVELS+13,ALLNUM , 3,16, 3,Questions[FLDLEVELS+13],&PcbData.UserLevels[SEC_O],CLEAR,checkseclevel);
  addquest(P,14,vBYTE,USERLEVELS+14,ALLNUM , 3,17, 3,Questions[FLDLEVELS+14],&PcbData.UserLevels[SEC_P],CLEAR,checkseclevel);

  addquest(P,15,vBYTE,USERLEVELS+15,ALLNUM ,43, 3, 3,Questions[FLDLEVELS+15],&PcbData.UserLevels[SEC_Q],CLEAR,checkseclevel);
  addquest(P,16,vBYTE,USERLEVELS+16,ALLNUM ,43, 4, 3,Questions[FLDLEVELS+16],&PcbData.UserLevels[SEC_R],CLEAR,checkseclevel);
  addquest(P,17,vBYTE,USERLEVELS+17,ALLNUM ,43, 5, 3,Questions[FLDLEVELS+17],&PcbData.UserLevels[SEC_S],CLEAR,checkseclevel);
  addquest(P,18,vBYTE,USERLEVELS+18,ALLNUM ,43, 6, 3,Questions[FLDLEVELS+18],&PcbData.UserLevels[SEC_T],CLEAR,checkseclevel);
  addquest(P,19,vBYTE,USERLEVELS+19,ALLNUM ,43, 7, 3,Questions[FLDLEVELS+19],&PcbData.UserLevels[SEC_U],CLEAR,checkseclevel);
  addquest(P,20,vBYTE,USERLEVELS+20,ALLNUM ,43, 8, 3,Questions[FLDLEVELS+20],&PcbData.UserLevels[SEC_V],CLEAR,checkseclevel);
  addquest(P,21,vBYTE,USERLEVELS+21,ALLNUM ,43, 9, 3,Questions[FLDLEVELS+21],&PcbData.UserLevels[SEC_W],CLEAR,checkseclevel);
  addquest(P,22,vBYTE,USERLEVELS+22,ALLNUM ,43,10, 3,Questions[FLDLEVELS+22],&PcbData.UserLevels[SEC_X],CLEAR,checkseclevel);
  addquest(P,23,vBYTE,USERLEVELS+23,ALLNUM ,43,11, 3,Questions[FLDLEVELS+23],&PcbData.UserLevels[SEC_Y],CLEAR,checkseclevel);
  addquest(P,24,vBYTE,USERLEVELS+24,ALLNUM ,43,12, 3,Questions[FLDLEVELS+24],&PcbData.UserLevels[SEC_Z],CLEAR,checkseclevel);

  addquest(P,25,vBYTE,USERLEVELS+25,ALLNUM ,43,13, 3,Questions[FLDLEVELS+25],&PcbData.UserLevels[SEC_CHAT],CLEAR,checkseclevel);
  addquest(P,26,vBYTE,USERLEVELS+26,ALLNUM ,43,14, 3,Questions[FLDLEVELS+26],&PcbData.UserLevels[SEC_OPEN],CLEAR,checkseclevel);
  addquest(P,27,vBYTE,USERLEVELS+27,ALLNUM ,43,15, 3,Questions[FLDLEVELS+27],&PcbData.UserLevels[SEC_TEST],CLEAR,checkseclevel);
  addquest(P,28,vBYTE,USERLEVELS+28,ALLNUM ,43,16, 3,Questions[FLDLEVELS+28],&PcbData.UserLevels[SEC_USER],CLEAR,checkseclevel);
  addquest(P,29,vBYTE,USERLEVELS+29,ALLNUM ,43,17, 3,Questions[FLDLEVELS+29],&PcbData.UserLevels[SEC_WHO ],CLEAR,checkseclevel);

  addquest(P,30,vBYTE,USERLEVELS+30,ALLNUM , 3,19, 3,Questions[FLDLEVELS+30],&PcbData.UserLevels[SEC_BATCH ],CLEAR,checkseclevel);
  addquest(P,31,vBYTE,USERLEVELS+31,ALLNUM , 3,20, 3,Questions[FLDLEVELS+31],&PcbData.UserLevels[SEC_EDIT  ],CLEAR,checkseclevel);
  addquest(P,32,vBYTE,USERLEVELS+32,ALLNUM , 3,21, 3,Questions[FLDLEVELS+32],&PcbData.UserLevels[SEC_REG   ],CLEAR,checkseclevel);
  addquest(P,33,vBYTE,USERLEVELS+33,ALLNUM , 3,22, 3,Questions[FLDLEVELS+33],&PcbData.UserLevels[SEC_NONREG],CLEAR,checkseclevel);
}


void pascal levels(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMLEVELS * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMLEVELS-1,0,"Security Levels","User Levels",1,NOCLEARFLD);
  freescrn(Fields,NUMLEVELS-1);
}
