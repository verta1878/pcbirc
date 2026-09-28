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
#include <misc.h>
#include <pcb.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


int jumpfsec(int Before) {
  showeditmsg(Before,"this");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3) {
      if (SaveFldPtr->Fptr == PcbData.FscFile)
        editfsec(0);
      else
        editfsec(1);
    }
  }
  return(0);
}

int jumppwrd(int Before) {
  showeditmsg(Before,"the PWRD");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      editpwrd();
  }
  return(0);
}

int jumptcan(int Before) {
  showeditmsg(Before,"the User Name TCAN");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      edittcan();
  }
  return(0);
}


int jumpfiletcan(int Before) {
  showeditmsg(Before,"the Upload TCAN");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      editfiletcan();
  }
  return(0);
}


int jumpmltlang(int Before) {
  showeditmsg(Before,"the LANGUAGE FILE");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      editlang();
  }
  return(0);
}

int jumpprot(int Before) {
  showeditmsg(Before,"the Protocol Data");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      editprot();
  }
  return(0);
}

int jumpallpath(int Before) {
  showeditmsg(Before,"ALL FILES PATH LIST");
  if (KeyFlags == FLAG3 && ! Before)
    editpath(PcbData.AllFilesList,1);
  return(0);
}


int jumpcmdfile(int Before) {
  showeditmsg(Before,"CMD.LST");
  if (Before)
    return(0);

  if (KeyFlags == FLAG3) {
    stripright(SaveFldPtr->Answer,' ');
    if (SaveFldPtr->Answer[0] != 0)
      editcmd(SaveFldPtr->Answer);
  }
  return(0);
}


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vUPSTR,CNFFILEHELP+ 0,ALLFILE ,3, 5,30,Questions[FLDCONFFILES+ 0],PcbData.PwdFile     ,NOCLEARFLD,jumppwrd);
  addquest(P, 1,vUPSTR,CNFFILEHELP+ 1,ALLFILE ,3, 6,30,Questions[FLDCONFFILES+ 1],PcbData.FscFile     ,NOCLEARFLD,jumpfsec);
  addquest(P, 2,vUPSTR,CNFFILEHELP+ 2,ALLFILE ,3, 7,30,Questions[FLDCONFFILES+ 2],PcbData.UscFile     ,NOCLEARFLD,jumpfsec);
  addquest(P, 3,vUPSTR,CNFFILEHELP+ 3,ALLFILE ,3, 8,30,Questions[FLDCONFFILES+ 3],PcbData.TcnFile     ,NOCLEARFLD,jumptcan);
  addquest(P, 4,vUPSTR,CNFFILEHELP+ 4,ALLFILE ,3, 9,30,Questions[FLDCONFFILES+ 4],PcbData.TrnFile     ,NOCLEARFLD,jumpprot);
  addquest(P, 5,vUPSTR,CNFFILEHELP+ 5,ALLFILE ,3,10,30,Questions[FLDCONFFILES+ 5],PcbData.MultiLang   ,NOCLEARFLD,jumpmltlang);
  addquest(P, 6,vUPSTR,CNFFILEHELP+ 6,ALLFILE ,3,11,30,Questions[FLDCONFFILES+ 6],PcbData.ColorFile   ,NOCLEARFLD,checkfiles8);
  addquest(P, 7,vUPSTR,CNFFILEHELP+ 7,ALLFILE ,3,12,30,Questions[FLDCONFFILES+ 7],PcbData.CmdLst      ,NOCLEARFLD,jumpcmdfile);
  addquest(P, 8,vUPSTR,CNFFILEHELP+ 8,ALLFILE ,3,13,30,Questions[FLDCONFFILES+ 8],PcbData.AllFilesList,NOCLEARFLD,jumpallpath);
  addquest(P, 9,vUPSTR,CNFFILEHELP+ 9,ALLFILE ,3,14,30,Questions[FLDCONFFILES+ 9],PcbData.FileTcan    ,NOCLEARFLD,jumpfiletcan);

  addquest(P,10,vUPSTR,CNFFILEHELP+10,ALLFILE ,3,16,30,Questions[FLDCONFFILES+10],PcbData.ViewBatch   ,CLEAR,checkfiles8);
  addquest(P,11,vUPSTR,CNFFILEHELP+11,ALLPROTO,3,17, 3,Questions[FLDCONFFILES+11],PcbData.ViewExt     ,CLEAR,NULL);

  addquest(P,12,vUPSTR,CNFFILEHELP+12,ALLTEXT ,3,19,30,Questions[FLDCONFFILES+12],PcbData.NetCopy     ,CLEAR,NULL);
}


void pascal configurationfiles(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMCONFFILES * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */
  ExitKeyNum[3] = 61;  ExitKeyFlag[3] = FLAG4;  /*  F3  */

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  readscrn(Fields,NUMCONFFILES-1,0,"File Locations","Configuration Files",1,NOCLEARFLD);
  freescrn(Fields,NUMCONFFILES-1);

  ExitKeyNum[2] = 0;
  ExitKeyNum[3] = 0;
}
