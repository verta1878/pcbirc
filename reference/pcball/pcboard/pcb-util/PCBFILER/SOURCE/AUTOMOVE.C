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


#include <mem.h>
#include <pcb.h>
#include <misc.h>
#include <stdio.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#include "rules.h"

void pascal automove(void) {
  bool        SaveBatchMode;
  bool        Moved;
  int         RetVal;
  int         ScrnNum;
  unsigned    Today;
  unsigned    LastCnf;
  unsigned    LastDir;
  long        Count;
  DOSFILE     File;
  DirListType Dir;
  movestype   Rec;
  pcbconftype Conf;

  ScrnNum = memsavescreen();

  Count = loadmoves(SORTMOVES);
  Today = getjuliandate();
  SaveBatchMode = BatchMode;
  BatchMode = TRUE;
  Moved = FALSE;

  unlink("changed.lst");

  if (dosfopen(Defaults.ProcName,OPEN_READ|OPEN_DENYNONE,&File) == -1) {
    memfreescreen(ScrnNum);
    return;
  }


  clsbox(1,20,78,22,Colors[DISPLAY]);

  // 0xFFFF is used as a special indicator for the OFFLINE directory, so we
  // will use 0xFFFE as a special indicator (instead of 0xFFFF) to tell us
  // when we have read a new area inside the loop.
  LastCnf = 0xFFFE;
  LastDir = 0xFFFE;

  while (readonemove(&File,&Rec) != -1) {
    if (Rec.SrceCnf != LastCnf || Rec.SrceDir != LastDir) {
      if (Rec.FileDate + Rec.Days <= Today) {
        LastCnf = Rec.SrceCnf;
        LastDir = Rec.SrceDir;

        if (LastCnf == 65535U) {
          // offline directory - not a conference
          memset(&Conf,0,sizeof(Conf));
        } else {
          getconfrecord(LastCnf,&Conf);
        }

        if (finddirpath(LastCnf,LastDir,&Dir) == -1)
          continue;

        VMInitRec(&Parents,NULL,0,sizeof(parenttype));
        VMInitRec(&Children,NULL,0,sizeof(childtype));
        VMInitRec(&ParentsOnly,NULL,0,sizeof(parentsonlytype));

        VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
        VMAccessAttrSet(&Children,VM_SEQUENTIAL);
        VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

        fastprintmove(24,20,"Reading files, please wait",Colors[QUESTION]);
        startaction('.',TRUE,32);

        if (LastCnf == 65535U) {
          // offline directory
          RetVal = includedirfiles(Dir.DskPath,-1,0,NOLOADFLAGS);
        } else {
          RetVal = readdirfile(Dir.DirPath,LastCnf,LastDir,SHOWACTION|LOADMOVES);
        }

        if (RetVal != -1) {
          ForcedUpdate = TRUE;
          savedir(&Dir,Conf.PthNameLoc,LastCnf,LastDir,SCANMOVES | (LastCnf == 65535U ? OFFLINEDIR : 0));  //lint !e534
          Moved = TRUE;
        }

        VMDone(&Parents);
        VMDone(&Children);
        VMDone(&ParentsOnly);

        // The savedir() function may have updated the processing file, to
        // avoid problems, we need to flush our own buffers and then re-read
        // everything that is in the file - the LastCnf and LastDir variables
        // will be used to ensure that we don't try to re-process a directory
        // that we may have been unable to process.

        dosflush(&File);
        dosrewind(&File);
      }
    }

    if (--Count == 0)
      break;
  }

  if (Moved) {
    if (fileexist("changed.lst") != 255) {
      sortchanged();
      LineCounter = 99;
      printscroll("",0);
    }
  } else {
    if (! SaveBatchMode) {
      memrestorescreen(ScrnNum,FREESCREEN);
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.AutoBox   = TRUE;
      MsgData.Line1     = Scrn_BottomRow-6;
      MsgData.Color1    = Colors[HEADING];
      MsgData.Msg1      = "No Files Found for Processing Today";
      showmessage();
    }
  }

  memfreescreen(ScrnNum);
  dosfclose(&File);
  BatchMode = SaveBatchMode;
}
