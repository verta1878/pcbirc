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


#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


char DefaultColor[3];
char MsgHdr1Color[3];
char MsgHdr2Color[3];
char MsgHdr3Color[3];
char MsgHdr4Color[3];
char MsgHdr5Color[3];
char MsgHdr6Color[3];

/********************************************************************
*
*  Function: initfields()
*
*/

void near pascal initfields(FldType *P) {
  addquest(P, 0,vUPSTR  ,ONLINECOLORS+0,ALLHEX  ,3, 5, 2,Questions[FLDCOLORS+ 0],DefaultColor,NOCLEARFLD,NULL);
  addquest(P, 1,vUPSTR  ,ONLINECOLORS+2,ALLHEX  ,3, 6, 2,Questions[FLDCOLORS+ 1],MsgHdr1Color,NOCLEARFLD,NULL);
  addquest(P, 2,vUPSTR  ,ONLINECOLORS+2,ALLHEX  ,3, 7, 2,Questions[FLDCOLORS+ 2],MsgHdr2Color,NOCLEARFLD,NULL);
  addquest(P, 3,vUPSTR  ,ONLINECOLORS+2,ALLHEX  ,3, 8, 2,Questions[FLDCOLORS+ 3],MsgHdr3Color,NOCLEARFLD,NULL);
  addquest(P, 4,vUPSTR  ,ONLINECOLORS+2,ALLHEX  ,3, 9, 2,Questions[FLDCOLORS+ 4],MsgHdr4Color,NOCLEARFLD,NULL);
  addquest(P, 5,vUPSTR  ,ONLINECOLORS+2,ALLHEX  ,3,10, 2,Questions[FLDCOLORS+ 5],MsgHdr5Color,NOCLEARFLD,NULL);
  addquest(P, 6,vUPSTR  ,ONLINECOLORS+2,ALLHEX  ,3,11, 2,Questions[FLDCOLORS+ 6],MsgHdr6Color,NOCLEARFLD,NULL);
}


void pascal colors(void) {
  FldType *NewFields;

  if ((NewFields = (FldType *) mallochk(NUMCOLORDEFS * sizeof(FldType))) == NULL)
    return;

  sprintf(DefaultColor,"%02X",PcbData.NewDefaultColor);
  sprintf(MsgHdr1Color,"%02X",PcbData.MsgHdr1Color);
  sprintf(MsgHdr2Color,"%02X",PcbData.MsgHdr2Color);
  sprintf(MsgHdr3Color,"%02X",PcbData.MsgHdr3Color);
  sprintf(MsgHdr4Color,"%02X",PcbData.MsgHdr4Color);
  sprintf(MsgHdr5Color,"%02X",PcbData.MsgHdr5Color);
  sprintf(MsgHdr6Color,"%02X",PcbData.MsgHdr6Color);

  initfields(NewFields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(NewFields,NUMCOLORDEFS-1,0,"Configuration Options","Colors",1,NOCLEARFLD);

  PcbData.NewDefaultColor = hextoint(DefaultColor);
  PcbData.MsgHdr1Color    = hextoint(MsgHdr1Color);
  PcbData.MsgHdr2Color    = hextoint(MsgHdr2Color);
  PcbData.MsgHdr3Color    = hextoint(MsgHdr3Color);
  PcbData.MsgHdr4Color    = hextoint(MsgHdr4Color);
  PcbData.MsgHdr5Color    = hextoint(MsgHdr5Color);
  PcbData.MsgHdr6Color    = hextoint(MsgHdr6Color);

  freescrn(NewFields,NUMCOLORDEFS-1);
}
