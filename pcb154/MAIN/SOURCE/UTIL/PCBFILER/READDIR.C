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
#include <stdlib.h>
#include <misc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <vmdata.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

long          TotalLines;
long          TotalParents;

VMDataSet     Parents;        // DataSet containing Parent and Text lines
VMDataSet     Children;       // DataSet containing secondary lines
VMDataSet     ParentsOnly;    // Used as an index into Parents
VMDataSet     Combined;       // Used as an index into both Parents & Children
VMDataSet     Dupes;          // DataSet for File Dupes with AVL information

VMAVLControl  DupesControl;   // AVL Control for Dupes DataSet
VMAVLTree     DupesTree;      // AVL Tree for Dupes DataSet


VM_SHINT comparedupes(const void *Rec1, const void *Rec2) {
  dupestype *p = (dupestype *)Rec1;
  dupestype *q = (dupestype *)Rec2;
  int RetVal;

  if ((RetVal = strcmp(p->FName,q->FName)) != 0)
    return(RetVal);

  if (p->ParentPos > q->ParentPos)
    return(1);
  if (q->ParentPos > p->ParentPos)
    return(-1);
  return(0);
}


static void near  readmovefile(unsigned CnfNum, unsigned DirNum) {
  parenttype        *p;
  parentsonlytype   *parentsonly;
  long               X;
  long               RecNum;
  long               Start;
  long               MoveCount;
  char               Name[13];
  DOSFILE            In;
  char               Str[80];
  savescrntype       Screen;

  if (fileexist(Work.ProcName) == 255 || dosfopen(Work.ProcName,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    return;

  fastprint( 2,12,"Moves File in Use     :",Colors[ANSWER]);
  fastprint(26,12,Work.ProcName,Colors[DISPLAY]);

//if (! BatchMode) {
    savescreen(&Screen);
    boxcls(17,Scrn_BottomRow-8,62,Scrn_BottomRow-4,Colors[OUTBOX],SINGLE);
    fastprintmove(19,Scrn_BottomRow-6,"Reading Auto Move File ... Please Wait",Colors[HEADING]);
    startaction('.',TRUE,64);
//}

  dossetbuf(&In,32768U);
  MoveCount = 0;
  Start = 0;

  // scan for all matches for this conf+dir, count up a total of the
  // matches, and record the offset where the first entry for this
  // conf+dir begins
  while (dosfgets(Str,sizeof(Str),&In) >= 0) {
    if (atoi(Str) == CnfNum && atoi(&Str[6]) == DirNum)
      MoveCount++;
    else if (MoveCount == 0)
      Start = dosfseek(&In,0,SEEK_CUR);
  }

  if (MoveCount == 0)
    goto exit;

  for (RecNum = 1; RecNum <= TotalParents; RecNum++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,RecNum,NULL);
    p = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);

    // ignore text lines
    if (p->Line != DIRLINE)
      continue;

//  Why?  Is there a good reason for this?
//
//  // ignore any files that are marked as OFFLINE or DELETED
//  if (p->Fields.FDate >= 0xFFFE)
//    continue;

    dosfseek(&In,Start,SEEK_SET);
    for (X = 0; X < MoveCount && dosfgets(Str,sizeof(Str),&In) >= 0; ) {
      // check first that the source conf & dir matches the DIR we are scanning
      if (atoi(Str) == CnfNum && atoi(&Str[6]) == DirNum) {
        // source location matches, now check for a match on the filename
        memcpy(Name,&Str[12],12);
        Name[12] = 0;
        stripright(Name,' ');
        if (strcmp(Name,p->Fields.FName) == 0) {
          // filename matches, apply the rules
          p->Fields.NCnfNum = atoi(&Str[25]);
          p->Fields.NDirNum = atoi(&Str[31]);
          p->Fields.Days    = atoi(&Str[46]);
          p->Fields.Keep    = (Str[52] == 'M' ? MOVE : COPYONLY);
        }
        X++;
      }
    }
  }

exit:
  dosfclose(&In);
//if (! BatchMode)
    restorescreen(&Screen);
}


int  readdirfile(char *FileName, int ConfNum, int DirNum, loadflagstype Flags) {
  bool             Show           = (Flags & SHOWACTION);
  bool             UpdateCombined = (Flags & UPDATECOMBINED);
  bool             UpdateDupes    = (Flags & UPDATEDUPES);
  bool             LoadMoves      = (Flags & LOADMOVES);
  bool             PrevFile;
  unsigned         TestValue;
  unsigned         BufLen;
  parenttype      *p;
  childtype       *c;
  dupestype       *d;
  combinedtype    *combined;
  parentsonlytype *parentsonly;
  long             ChildPos;
  long             PrevChildPos;
  long             ParentPos;
  long             DupePos;
  char             Temp[20];
  DOSFILE          InFile;
  char             Buffer[128];

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = FALSE;

  if (FileName[0] == 0) {
    sprintf(Buffer,"No filename found in DIR.LST for Directory #%d",DirNum);
    MsgData.Msg1   = Buffer;
    MsgData.Line1  = 18;
    MsgData.Color1 = Colors[HEADING];
    beep();
    showmessage();
    return(-1);
  }

  if (fileexist(FileName) == 255 || dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&InFile) == -1) {
    memset(&MsgData,0,sizeof(MsgData));
    beep();
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox   = TRUE;
    MsgData.Save      = TRUE;
    MsgData.Msg1      = "Unable to open file";
    MsgData.Line1     = 16;
    MsgData.Color1    = Colors[HEADING];
    MsgData.Msg2      = FileName;
    MsgData.Line2     = 18;
    MsgData.Color2    = Colors[DISPLAY];
    MsgData.Quest     = "Do you want to create it";
    MsgData.QuestLine = 20;
    MsgData.Answer[0] = 'N';
    MsgData.Answer[1] = 0;
    MsgData.Mask      = YESNO;
    showmessage();
    if (MsgData.Answer[0] == 'Y' && KeyFlags != ESC) {
      if (dosfopen(FileName,OPEN_WRIT|OPEN_DENYRDWR,&InFile) != -1) {
        dosfputs("Filename       Size      Date    Description of File Contents\r\n",&InFile);  //lint !e534
        dosfputs("============ ========  ========  ============================================\r\n",&InFile);  //lint !e534
        dosfclose(&InFile);
        if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&InFile) == -1)
          return(-1);
      } else return(-1);
    } else return(-1);
  }

  dossetbuf(&InFile,8192);
  ShowSwapMessage = TRUE;

  PrevFile = FALSE; // this variable gets set to TRUE if the previous line
                    // read in was a FILE LISTING and set to FALSE otherwise

  TotalLines    =
  TotalParents  = 0;

  ParentPos     =
  ChildPos      = VM_INVALID_POS;

  while (dosfgets(Buffer,sizeof(Buffer),&InFile) != -1) {
    TotalLines++;
    BufLen = strlen(Buffer);

    if (Show)
      showaction();

    // check for a child line -- all children must
    //   1) Begin with a space in column 1
    //   2) Follow a previously valid parent
    //   3) And have a vertical bar (|) somewhere on the line

    if (Buffer[0] == ' ' && PrevFile && strnchr(Buffer,'|',BufLen) != NULL) {
      PrevChildPos = ChildPos;
      c = (childtype *) VMRecordCreate(&Children,sizeof(childtype),&ChildPos,NULL);
      maxstrcpy(c->Str,Buffer,sizeof(c->Str));
      c->ParentPos    = ParentPos;
      c->PrevChildPos = PrevChildPos;
      c->NextChildPos = VM_INVALID_POS;

      if (PrevChildPos != VM_INVALID_POS) {
        c = (childtype *) VMRecordGetByPos(&Children,PrevChildPos);
        c->NextChildPos = ChildPos;
        VMRecordChanged(&Children);
      }

      if (UpdateCombined) {
        combined = (combinedtype *) VMRecordCreate(&Combined,sizeof(combinedtype),NULL,NULL);
        combined->Line = DUPELINE;
        combined->Pos  = ChildPos;
      }

      p = (parenttype *) VMRecordGetByPos(&Parents,ParentPos);
      if (p->Fields.FirstChildPos == VM_INVALID_POS) {
        p->Fields.FirstChildPos = ChildPos;
        VMRecordChanged(&Parents);
      }

      // stored the new child, updated the previous child if necessary,
      // updated the parent to point to the child, added the child to the
      // combined listing, now loop back for more
      continue;
    }

    // initialize the next record's PrevChildPos
    ChildPos = VM_INVALID_POS;


    p = (parenttype *) VMRecordCreate(&Parents,sizeof(parenttype),&ParentPos,NULL);
    parentsonly = (parentsonlytype *) VMRecordCreate(&ParentsOnly,sizeof(parentsonlytype),NULL,NULL);
    parentsonly->Pos = ParentPos;
    TotalParents++;

    if (UpdateCombined) {
      combined = (combinedtype *) VMRecordCreate(&Combined,sizeof(combinedtype),NULL,NULL);
      combined->Pos    = ParentPos;
    }

    memcpy(Temp,&Buffer[23],8); Temp[8] = 0;
    if (Buffer[0] != ' ' && BufLen >= 31 && (TestValue = ctod2(Temp)) != 0) {
      if (UpdateCombined)
        combined->Line = DIRLINE;  //lint !e644  combined is initialized if UpdateCombined is set

      PrevFile                = TRUE;
      p->Line                 = DIRLINE;
      p->Fields.FDate         =
      p->Fields.NDate         = TestValue;
      p->Fields.NCnfNum       = ConfNum;
      p->Fields.NDirNum       = DirNum;
      p->Fields.Exist         = BOTH;
      p->Fields.Keep          = KEEP;
      p->Fields.Days          = 0;
      p->Fields.FirstChildPos = VM_INVALID_POS;

      memcpy(p->Fields.FName,Buffer,12);  p->Fields.FName[12] = 0;
      stripright(p->Fields.FName,' ');
      strcpy(p->Fields.NName,p->Fields.FName);

      memcpy(Temp,&Buffer[12],9);  Temp[9] = 0;
      p->Fields.FSize = atol(Temp);

      if (BufLen > 32)
        maxstrcpy(p->Fields.FDesc,&Buffer[33],sizeof(p->Fields.FDesc));
      else
        p->Fields.FDesc[0] = 0;

      if (UpdateDupes) {
        d = (dupestype *) VMRecordCreate(&Dupes,sizeof(dupestype),&DupePos,NULL);
        strcpy(d->FName,p->Fields.FName);
        d->ParentPos = ParentPos;
        VMAVLAdd(&DupesTree,&DupesControl,DupePos);  //lint !e534
      }
    } else {
      PrevFile       = FALSE;
      if (UpdateCombined)
        combined->Line = TEXTLINE; //lint !e644  combined is initialized if UpdateCombined is set

      p->Line  = TEXTLINE;
      maxstrcpy(p->Str,Buffer,sizeof(p->Str));
    }
  }

  ShowSwapQuick = TRUE;
  dosfclose(&InFile);

  if (LoadMoves)
    readmovefile(ConfNum,DirNum);

  return(0);
}
