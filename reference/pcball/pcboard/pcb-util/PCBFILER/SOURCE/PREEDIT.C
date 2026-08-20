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
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <help.h>
#include <dosfunc.h>
#include <vmdata.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#include "rules.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*
*  Returns :
*/

static void near pascal readregulardirs(unsigned BoardNumber, bool FirstTime) {
  static int  Num;
  char        Ch;
  char        Verify;
  int         HighNum;
  int         NumTextDirs;
  char        Ans[2];
  char        S[80];     /* temporary string storage */
  DirListType Rec;
  pcbconftype Conf;

  getconfrecord(BoardNumber,&Conf);

  while (1) {
    NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
    HighNum = NumTextDirs;
    if (Conf.UpldDir[0] != 0)
      HighNum++;
    if (FirstTime)
      Num = HighNum;
    if (HighNum == 0) {
      if (Conf.PrivDir[0] == 0 && Conf.UpldDir[0] == 0) {
        fastprint(12,9,"There are NO directories specified for this selection.",Colors[HELPKEY]);
        fastprintmove(25,13,"Press any key to continue...",Colors[OUTBOX]);
        Ch = inkey(&Ch,CLOCK);
        KeyFlags = ESC;
        return;
      }
      strcpy(S,"The only available DIR number is 0 for PRIVATE");
      HighNum = 0;
    } else {
      sprintf(S,"Available DIR numbers are 0 for PRIVATE and 1 through %d",HighNum);
    }
    clsbox(2,5,78,5,Colors[DISPLAY]);
    fastprint(2,4,Conf.Name,Colors[DISPLAY]);
    fastprint(2,5,S,Colors[DISPLAY]);
    fastcenter(22," Press ESC to Exit  -or-  Press F2 to View DIR Configuration ",Colors[DESC]);

    ExitKeyNum[0] = 60; ExitKeyFlag[0] = FLAG3;
    inputnum(2,7,5,"Enter Number of DIR to Edit",&Num,vINT,PREEDIT);
    if (KeyFlags == FLAG3) {
      NewDirNumber = Num;
      editdirlist(BoardNumber,Num,TRUE,FALSE);
      KeyFlags = NOTHING;
      if (Num == NewDirNumber)
        continue;

      Num = NewDirNumber;
      sprintf(S,"%5u",Num);
      fastprint(32,7,S,Colors[ANSWER]);
      break;
    }

    if (KeyFlags == ESC)
      return;

    if (Num >= 0 && Num <= HighNum)
      break;

    beep();
  }

  ExitKeyNum[0] = 0;
  if (finddirpath(BoardNumber,Num,&Rec) != -1) {
    fastprint( 2, 9,"Selected DIR Text File:",Colors[ANSWER]);
    fastprint(26, 9,Rec.DirPath,Colors[DISPLAY]);
    fastprint( 2,11,"DIR File Description  :",Colors[ANSWER]);
    fastprint(26,11,Rec.DirDesc,Colors[DISPLAY]);

    if (Rec.DskPath[0] != 0) {
      fastprint( 2,10,"Attached Subdirectory :",Colors[ANSWER]);
      fastprint(26,10,Rec.DskPath,Colors[DISPLAY]);
      inputnum(2,14,1,"Include files found on DISK that are not listed in the DIR file",&Work.ReadAll,vBOOL,DEFAULTS+2);
      if (KeyFlags == ESC) {
        KeyFlags = NOTHING;
        return;
      }
      Ans[0] = Work.VerifyExist;
      Ans[1] = 0;
      inputstr(2,16,1,"Verify file existence (Y=attached directory, N=No, A=All download paths)",Ans,Ans,YNAQ,INPUT_CAPS,DEFAULTS+3);
      Verify = Ans[0];
      if (KeyFlags == ESC) {
        KeyFlags = NOTHING;
        return;
      }
    } else {
      fastprint(2,10,"No Attached Subdirectory",Colors[DISPLAY]);
      Ans[0] = Work.VerifyExist;
      Ans[1] = 0;
      inputstr(2,16,1,"Verify file existence (N=No, A=All download paths)",Ans,Ans,YNAQ,INPUT_CAPS,DEFAULTS+2);
      Verify = Ans[0];
      if (Verify == 'Y')
        Verify = 'A';
      if (KeyFlags == ESC) {
        KeyFlags = NOTHING;
        return;
      }
    }

    VMInitRec(&Parents,NULL,0,sizeof(parenttype));
    VMInitRec(&Children,NULL,0,sizeof(childtype));
    VMInitRec(&Combined,NULL,0,sizeof(combinedtype));
    VMInitRec(&ParentsOnly,NULL,0,sizeof(parentsonlytype));

    VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
    VMAccessAttrSet(&Children,VM_SEQUENTIAL);
    VMAccessAttrSet(&Combined,VM_SEQUENTIAL);
    VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

    clsbox(1,20,78,22,Colors[DISPLAY]);
    fastprintmove(24,20,"Reading files, please wait",Colors[QUESTION]);
    startaction('.',TRUE,32);
    if (readdirfile(Rec.DirPath,BoardNumber,Num,SHOWACTION|UPDATECOMBINED|LOADMOVES) != -1) {
      if (Verify == 'Y' || Verify == 'A' || Verify == 'Q') {
        clsbox(1,19,78,19,Colors[DISPLAY]);
        fastprintmove(18,19,"Verifying file existence, please wait",Colors[QUESTION]);
        startaction('.',FALSE,128);
        verifyfiles(Verify,Rec.DskPath,Conf.PthNameLoc);
      }
      if (Work.ReadAll && Rec.DskPath[0] != 0) {
        clsbox(1,19,78,19,Colors[DISPLAY]);
        fastprintmove(19,19,"Checking disk for files, please wait",Colors[QUESTION]);
        startaction('.',FALSE,32);
        if (includedirfiles(Rec.DskPath,BoardNumber,Num,UPDATECOMBINED) == -1)
          goto end;
      }
      if (editdir(Rec.DirDesc,Rec.DskPath,Conf.PthNameLoc,BoardNumber,Num)) {
        if (savedir(&Rec,Conf.PthNameLoc,BoardNumber,Num,SCANMOVES) == -1)
          KeyFlags = ESC;  /* indicate an error to avoid further processing */
      }
    } else {
      KeyFlags = ESC;
    }

end:
    VMDone(&Parents);
    VMDone(&Children);
    VMDone(&Combined);
    VMDone(&ParentsOnly);

    if (KeyFlags != ESC) {
      if (CheckDupes)
        dupeschanged();
      sortchanged();
      LineCounter = 99;
      printscroll("",0);
    }
    KeyFlags = NOTHING;
  }
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*
*  Returns :
*/

void pascal readofflinedir(void) {
  bool Changed;
  DirListType Rec;

  if (finddirpath(0xFFFF,0,&Rec) != -1) {
    VMInitRec(&Parents,NULL,0,sizeof(parenttype));
    VMInitRec(&Children,NULL,0,sizeof(childtype));
    VMInitRec(&Combined,NULL,0,sizeof(combinedtype));
    VMInitRec(&ParentsOnly,NULL,0,sizeof(parentsonlytype));

    VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
    VMAccessAttrSet(&Children,VM_SEQUENTIAL);
    VMAccessAttrSet(&Combined,VM_SEQUENTIAL);
    VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

    TotalLines   = 0;
    TotalParents = 0;
    Changed      = FALSE;

    clsbox(15,19,64,22,Colors[DISPLAY]);
    fastprintmove(19,19,"Checking disk for files, please wait",Colors[QUESTION]);
//  fastprint(30,22,"free memory:     K",Colors[DISPLAY]);
    startaction('.',FALSE,32);
    if (includedirfiles(Rec.DskPath,-1,0,UPDATECOMBINED) != -1) {
      if (editdir(Rec.DirDesc,Rec.DskPath,"",65535U,0)) {
        if (savedir(&Rec,"",-1,0,OFFLINEDIR|SCANMOVES) == -1)
          Changed = FALSE;  /* indicate an error to avoid further processing */
        else
          Changed = TRUE;
      } else
        Changed = FALSE;
    }

    VMDone(&Parents);
    VMDone(&Children);
    VMDone(&Combined);
    VMDone(&ParentsOnly);

    if (Changed) {
      if (CheckDupes)
        dupeschanged();
      sortchanged();
      LineCounter = 99;
      printscroll("",0);
    }
  }
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*
*  Returns :
*/

static void near pascal getdirtoedit(int BoardNumber) {
  bool FirstTime;

  FirstTime = TRUE;
  do {
    TotalLines = 0;
    TotalParents = 0;
    setcursor(CUR_NORMAL);
    clscolor(Colors[OUTBOX]);
    generalscreen(MainHead1,"Directory Selection");
    readregulardirs(BoardNumber,FirstTime);
    FirstTime = FALSE;
  } while (KeyFlags != ESC);
}


void pascal confgetdirtoedit(void) {
  unsigned ConfSel;

  ConfSel = MenuSelection + 1;
  if (ConfSel > PcbData.NumConf) {
    beep();
    KeyFlags = ESC;
    return;
  }

#ifdef DEMO
  if (ConfSel != 1) {
    beep();
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save    = TRUE;
    MsgData.AutoBox = TRUE;
    MsgData.Line1   = 18;
    MsgData.Msg1    = "In the DEMO version only the first conference is selectable";
    MsgData.Color1  = Colors[HEADING];
    showmessage();
    KeyFlags = ESC;
    return;
  }
#endif
  getdirtoedit(ConfSel);
}


void pascal maingetdirtoedit(void) {
  getdirtoedit(0);
}


#ifndef DEMO
void pascal processdirfile(unsigned BoardNumber, unsigned DirNumber, bool ScanDlPath, char *ApplyRulesName) {
  unsigned       HighNum;
  unsigned       NumTextDirs;
  loadflagstype  LoadFlags;
  saveflagstype  SaveFlags;
  DirListType    Rec;
  pcbconftype    Conf;
  savescrntype   ScrnBuf;

  getconfrecord(BoardNumber,&Conf);
  NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
  HighNum = NumTextDirs;
  if (Conf.UpldDir[0] != 0)
    HighNum++;

  if (DirNumber > HighNum)
    return;

  if (finddirpath(BoardNumber,DirNumber,&Rec) != -1) {
    clsbox(2,5,78,5,Colors[DISPLAY]);
    fastprint( 2, 9,"Selected DIR Text File:",Colors[ANSWER]);
    fastprint(26, 9,Rec.DirPath,Colors[DISPLAY]);
    fastprint( 2,11,"DIR File Description  :",Colors[ANSWER]);
    fastprint(26,11,Rec.DirDesc,Colors[DISPLAY]);

    if (Rec.DskPath[0] != 0) {
      fastprint( 2,10,"Attached Subdirectory :",Colors[ANSWER]);
      fastprint(26,10,Rec.DskPath,Colors[DISPLAY]);
    } else {
      fastprint(2,10,"No Attached Subdirectory",Colors[DISPLAY]);
    }

    VMInitRec(&Parents,NULL,0,sizeof(parenttype));
    VMInitRec(&Children,NULL,0,sizeof(childtype));
    VMInitRec(&Combined,NULL,0,sizeof(combinedtype));
    VMInitRec(&ParentsOnly,NULL,0,sizeof(parentsonlytype));

    VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
    VMAccessAttrSet(&Children,VM_SEQUENTIAL);
    VMAccessAttrSet(&Combined,VM_SEQUENTIAL);
    VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

    if (ApplyRulesName != NULL) {
      LoadFlags = SHOWACTION|LOADMOVES;
      SaveFlags = SCANMOVES;
    } else {
      LoadFlags = SHOWACTION;
      SaveFlags = NOSAVEFLAGS;
    }

    clsbox(1,20,78,22,Colors[DISPLAY]);
    fastprintmove(24,20,"Reading files, please wait",Colors[QUESTION]);
    startaction('.',TRUE,32);
    if (readdirfile(Rec.DirPath,BoardNumber,DirNumber,LoadFlags) != -1) {
      clsbox(1,19,78,19,Colors[DISPLAY]);

      if (ApplyRulesName != NULL) {
        savescreen(&ScrnBuf);
        clsbox(1,1,78,1,Colors[HEADING]);
        fastcenter(1,"Applying Rules",Colors[HEADING]);
        startaction('.',FALSE,128);
        applyrulesinit(Rec.DskPath,Conf.PthNameLoc,BoardNumber,DirNumber);
        applyrulesfile(ApplyRulesName,&ScrnBuf);  //lint !e534
      } else {
        fastprintmove(18,19,"Verifying file existence, please wait",Colors[QUESTION]);
        startaction('.',FALSE,128);
        verifyfiles(ScanDlPath ? 'A' : 'Y',Rec.DskPath,Conf.PthNameLoc);
      }

      if (savedir(&Rec,Conf.PthNameLoc,BoardNumber,DirNumber,SaveFlags) != -1) {
        VMDone(&Parents);
        VMDone(&Children);
        VMDone(&Combined);
        VMDone(&ParentsOnly);
        sortchanged();
      } else {
        VMDone(&Parents);
        VMDone(&Children);
        VMDone(&Combined);
        VMDone(&ParentsOnly);
      }
    } else {
      VMDone(&Parents);
      VMDone(&Children);
      VMDone(&Combined);
      VMDone(&ParentsOnly);
    }
  }
}
#endif
