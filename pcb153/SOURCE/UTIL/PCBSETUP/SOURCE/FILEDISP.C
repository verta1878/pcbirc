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


static int jumpcnfmenu(int Before) {
  showeditmsg2(Before,"CNFN");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpgrpchat(int Before) {
  showeditmsg2(Before,"GROUP");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpgrpmenu(int Before) {
  showeditmsg2(Before,"CHATM");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpnoansi(int Before) {
  showeditmsg2(Before,"NOANSI");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpwelcome(int Before) {
  showeditmsg2(Before,"WELCOME");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpnewuser(int Before) {
  showeditmsg2(Before,"NEWUSER");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpclsfile(int Before) {
  showeditmsg2(Before,"CLOSED");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpwrnfile(int Before) {
  showeditmsg2(Before,"WARNING");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

static int jumpexpfile(int Before) {
  showeditmsg2(Before,"EXPIRED");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vUPSTR,DSPFILEHELP+0,ALLFILE,3, 6,30,Questions[FLDDISPFILES+ 0],PcbData.WlcFile      ,NOCLEARFLD,jumpwelcome);
  addquest(P, 1,vUPSTR,DSPFILEHELP+1,ALLFILE,3, 7,30,Questions[FLDDISPFILES+ 1],PcbData.NewFile      ,NOCLEARFLD,jumpnewuser);
  addquest(P, 2,vUPSTR,DSPFILEHELP+2,ALLFILE,3, 8,30,Questions[FLDDISPFILES+ 2],PcbData.ClsFile      ,NOCLEARFLD,jumpclsfile);
  addquest(P, 3,vUPSTR,DSPFILEHELP+3,ALLFILE,3, 9,30,Questions[FLDDISPFILES+ 3],PcbData.WrnFile      ,NOCLEARFLD,jumpwrnfile);
  addquest(P, 4,vUPSTR,DSPFILEHELP+4,ALLFILE,3,10,30,Questions[FLDDISPFILES+ 4],PcbData.ExpFile      ,NOCLEARFLD,jumpexpfile);
  addquest(P, 5,vUPSTR,DSPFILEHELP+5,ALLFILE,3,11,30,Questions[FLDDISPFILES+ 5],PcbData.CnfMenu      ,NOCLEARFLD,jumpcnfmenu);
  addquest(P, 6,vUPSTR,DSPFILEHELP+6,ALLFILE,3,12,30,Questions[FLDDISPFILES+ 6],PcbData.GroupChat    ,NOCLEARFLD,jumpgrpchat);
  addquest(P, 7,vUPSTR,DSPFILEHELP+7,ALLFILE,3,13,30,Questions[FLDDISPFILES+ 7],PcbData.ChatMenu     ,NOCLEARFLD,jumpgrpmenu);
  addquest(P, 8,vUPSTR,DSPFILEHELP+8,ALLFILE,3,14,30,Questions[FLDDISPFILES+ 8],PcbData.NoAnsi       ,NOCLEARFLD,jumpnoansi);
}


void pascal displayfiles(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMDISPFILES * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */
  ExitKeyNum[3] = 61;  ExitKeyFlag[3] = FLAG4;  /*  F3  */

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMDISPFILES-1,0,"File Locations","Display Files",1,NOCLEARFLD);
  freescrn(Fields,NUMDISPFILES-1);

  ExitKeyNum[2] = 0;
  ExitKeyNum[3] = 0;
}
