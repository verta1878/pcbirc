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

static bool LogOnOffSelf;
static bool LogOnOffWindow;
static bool ProtocolsSelf;
static bool ProtocolsWindow;
static bool DoorsSelf;
static bool DoorsWindow;
static bool FileTestSelf;
static bool FileTestWindow;
static bool ViewerSelf;
static bool ViewerWindow;
static bool CompressSelf;
static bool CompressWindow;
static bool ShellsSelf;
static bool ShellsWindow;


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBOOL,OS2SETTINGS+ 0,YESNO, 3, 6, 1,Questions[FLDOS2OPT+ 0],&LogOnOffSelf   ,NOCLEARFLD,NULL);
  addquest(P, 1,vBOOL,OS2SETTINGS+ 0,YESNO,40, 6, 1,""                     ,&LogOnOffWindow ,NOCLEARFLD,NULL);

  addquest(P, 2,vBOOL,OS2SETTINGS+ 1,YESNO, 3, 7, 1,Questions[FLDOS2OPT+ 1],&ProtocolsSelf  ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL,OS2SETTINGS+ 1,YESNO,40, 7, 1,""                     ,&ProtocolsWindow,NOCLEARFLD,NULL);

  addquest(P, 4,vBOOL,OS2SETTINGS+ 2,YESNO, 3, 8, 1,Questions[FLDOS2OPT+ 2],&DoorsSelf      ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL,OS2SETTINGS+ 2,YESNO,40, 8, 1,""                     ,&DoorsWindow    ,NOCLEARFLD,NULL);

  addquest(P, 6,vBOOL,OS2SETTINGS+ 3,YESNO, 3, 9, 1,Questions[FLDOS2OPT+ 3],&FileTestSelf   ,NOCLEARFLD,NULL);
  addquest(P, 7,vBOOL,OS2SETTINGS+ 3,YESNO,40, 9, 1,""                     ,&FileTestWindow ,NOCLEARFLD,NULL);

  addquest(P, 8,vBOOL,OS2SETTINGS+ 4,YESNO, 3,10, 1,Questions[FLDOS2OPT+ 4],&ViewerSelf     ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL,OS2SETTINGS+ 4,YESNO,40,10, 1,""                     ,&ViewerWindow   ,NOCLEARFLD,NULL);

  addquest(P,10,vBOOL,OS2SETTINGS+ 5,YESNO, 3,11, 1,Questions[FLDOS2OPT+ 5],&CompressSelf   ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL,OS2SETTINGS+ 5,YESNO,40,11, 1,""                     ,&CompressWindow ,NOCLEARFLD,NULL);

  addquest(P,12,vBOOL,OS2SETTINGS+ 6,YESNO, 3,12, 1,Questions[FLDOS2OPT+ 6],&ShellsSelf     ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL,OS2SETTINGS+ 6,YESNO,40,12, 1,""                     ,&ShellsWindow   ,NOCLEARFLD,NULL);

  addquest(P,14,vINT, OS2SETTINGS+ 7,ALLNUM, 3,16,3,Questions[FLDOS2OPT+ 7],&PcbData.PriorityNormal   ,CLEAR,NULL);
  addquest(P,15,vINT, OS2SETTINGS+ 7,ALLNUM, 3,17,3,Questions[FLDOS2OPT+ 8],&PcbData.PriorityProtocols,CLEAR,NULL);
  addquest(P,16,vINT, OS2SETTINGS+ 7,ALLNUM, 3,18,3,Questions[FLDOS2OPT+ 9],&PcbData.PriorityCompress ,CLEAR,NULL);
  addquest(P,17,vINT, OS2SETTINGS+ 7,ALLNUM, 3,19,3,Questions[FLDOS2OPT+10],&PcbData.PriorityFidoIn   ,CLEAR,NULL);
  addquest(P,18,vINT, OS2SETTINGS+ 7,ALLNUM, 3,20,3,Questions[FLDOS2OPT+11],&PcbData.PriorityFidoOut  ,CLEAR,NULL);
  addquest(P,19,vINT, OS2SETTINGS+ 7,ALLNUM, 3,21,3,Questions[FLDOS2OPT+12],&PcbData.PriorityShells   ,CLEAR,NULL);
}


void pascal os2options(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMOS2OPT * sizeof(FldType))) == NULL)
    return;

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  LogOnOffSelf     = PcbData.MinimizeLogOnOff  & 1;
  LogOnOffWindow   = PcbData.MinimizeLogOnOff  & 2;
  ProtocolsSelf    = PcbData.MinimizeProtocols & 1;
  ProtocolsWindow  = PcbData.MinimizeProtocols & 2;
  DoorsSelf        = PcbData.MinimizeDoors     & 1;
  DoorsWindow      = PcbData.MinimizeDoors     & 2;
  FileTestSelf     = PcbData.MinimizeFileTest  & 1;
  FileTestWindow   = PcbData.MinimizeFileTest  & 2;
  ViewerSelf       = PcbData.MinimizeFileView  & 1;
  ViewerWindow     = PcbData.MinimizeFileView  & 2;
  CompressSelf     = PcbData.MinimizeCompress  & 1;
  CompressWindow   = PcbData.MinimizeCompress  & 2;
  ShellsSelf       = PcbData.MinimizeShells    & 1;
  ShellsWindow     = PcbData.MinimizeShells    & 2;

  fastprint(3,4,"Minimize Window Settings      PCBoard   Shell",Colors[DISPLAY]);
  fastprint(3,5,"ออออออออออออออออออออออออ      อออออออ   อออออ",Colors[DISPLAY]);

  fastprint(3,14,"Priority Settings",Colors[DISPLAY]);
  fastprint(3,15,"อออออออออออออออออ",Colors[DISPLAY]);

  readscrn(Fields,NUMOS2OPT-1,0,"Configuration Options","OS/2 Settings",1,NOCLEARFLD);
  freescrn(Fields,NUMOS2OPT-1);

  PcbData.MinimizeLogOnOff  = (LogOnOffSelf ? 1 : 0)  + (LogOnOffWindow ? 2 : 0);
  PcbData.MinimizeProtocols = (ProtocolsSelf ? 1 : 0) + (ProtocolsWindow ? 2 : 0);
  PcbData.MinimizeDoors     = (DoorsSelf ? 1 : 0)     + (DoorsWindow ? 2 : 0);
  PcbData.MinimizeFileTest  = (FileTestSelf ? 1 : 0)  + (FileTestWindow ? 2 : 0);
  PcbData.MinimizeFileView  = (ViewerSelf ? 1 : 0)    + (ViewerWindow ? 2 : 0);
  PcbData.MinimizeCompress  = (CompressSelf ? 1 : 0)  + (CompressWindow ? 2 : 0);
  PcbData.MinimizeShells    = (ShellsSelf ? 1 : 0)    + (ShellsWindow ? 2 : 0);

  if (PcbData.PriorityNormal > 31)
    PcbData.PriorityNormal = 31;
  else if (PcbData.PriorityNormal < -31)
    PcbData.PriorityNormal = -31;

  if (PcbData.PriorityProtocols > 31)
    PcbData.PriorityProtocols = 31;
  else if (PcbData.PriorityProtocols < -31)
    PcbData.PriorityProtocols = -31;

  if (PcbData.PriorityCompress > 31)
    PcbData.PriorityCompress = 31;
  else if (PcbData.PriorityCompress < -31)
    PcbData.PriorityCompress = -31;

  if (PcbData.PriorityFidoIn > 31)
    PcbData.PriorityFidoIn = 31;
  else if (PcbData.PriorityFidoIn < -31)
    PcbData.PriorityFidoIn = -31;

  if (PcbData.PriorityFidoOut > 31)
    PcbData.PriorityFidoOut = 31;
  else if (PcbData.PriorityFidoOut < -31)
    PcbData.PriorityFidoOut = -31;

  if (PcbData.PriorityShells > 31)
    PcbData.PriorityShells = 31;
  else if (PcbData.PriorityShells < -31)
    PcbData.PriorityShells = -31;
}
