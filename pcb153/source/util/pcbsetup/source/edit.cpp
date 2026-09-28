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
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include "edit.hpp"
#ifdef DEBUG
#include <memcheck.h>
#define mallochk malloc
#endif


const int NumInternalKeys = 4;  // ALT-I, ALT-D, ALT-R, ALT-5

char YNA[4] = {  3, 'Y', 'N', 'A' };



// this function is also used outside of the editclass

char pascal editexit(char *Str) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox   = TRUE;
  MsgData.Save      = TRUE;
  MsgData.Msg1      = Str;
  MsgData.Line1     = Scrn_BottomRow - 6;
  MsgData.Color1    = Colors[QUESTION];
  MsgData.Quest     = "Save the changes made (Yes; No-resume edit; Abort-exit, no save)";
  MsgData.QuestLine = Scrn_BottomRow - 4;
  MsgData.Answer[0] = 'Y';
  MsgData.Answer[1] = 0;
  MsgData.Mask      = YNA;
  showmessage();
  if (KeyFlags == ESC)
    MsgData.Answer[0] = 'N';
  return(MsgData.Answer[0]);
}


void pascal editclass::setupscreen(void) {
  // 50=max lines, minus (number of lines at top plus number of lines at
  // bottom, plus one more because top line is 0-based)
  MaxScrnLines = 50 - (TopLine + KeepBottom + 1);
  CurNumLines = (getfont() == BIGFONT ? 25 : 50) - (TopLine + KeepBottom + 1);
  setupscale(CurNumLines-1,177,TopLine);
}


editclass::editclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) {
  int          X;
  scrnrectype *p;

  Allocated    = FALSE;
  RecSize      = Size;
  TotalRecs    = 0;
  TotalIdx     = 0;
  LeftSide     = Left;
  RightSide    = Right;
  TopLine      = Top;
  KeepBottom   = Bottom;       // number of lines to protect on the bottom
  NumColumns   = Columns;
  NumExitKeys  = NumKeys + NumInternalKeys;
  SaveKeyNum   = NULL;
  SaveKeyFlag  = NULL;
  SaveRec      = NULL;
  KeyDesc      = NULL;

  setupscreen();

  if (Title != NULL)
    editclass::Title = Title;
  else
    editclass::Title = "Untitled";

  if ((SaveRec = mallochk(Size)) == NULL)
    return;

  if ((Screen = (scrnrectype *) mallochk(sizeof(scrnrectype) * MaxScrnLines)) == NULL)
    return;

  for (X = 0, p = Screen; X < MaxScrnLines; X++, p++) {
    if ((p->Rec = mallochk(Size)) == NULL)
      return;
  }

  if ((SaveKeyNum = (char *) mallochk(NumExitKeys * sizeof(char))) == NULL)
    return;

  memcpy(SaveKeyNum,ExitKeyNum,NumExitKeys * sizeof(char));

  if ((SaveKeyFlag = (int *) mallochk(NumExitKeys * sizeof(int))) == NULL)
    return;

  memcpy(SaveKeyFlag,ExitKeyFlag,NumExitKeys * sizeof(int));

  if (NumKeys != 0 && (KeyDesc = (char **) mallochk(NumKeys * sizeof(char *))) == NULL)
    return;

  ExitKeyNum[0] = 23;  ExitKeyFlag[0] = ALTI;   /*  alt-i  */
  ExitKeyNum[1] = 32;  ExitKeyFlag[1] = ALTD;   /*  alt-d  */
  ExitKeyNum[2] = 19;  ExitKeyFlag[2] = ALTR;   /*  alt-r  */
  ExitKeyNum[3] =124;  ExitKeyFlag[3] = ALT5;   /*  alt-5  */
  NextExitKey = NumInternalKeys;

  savescreen(&ScrnBuf);
  VMInitRec(&DataSet,NULL,0,Size);
  VMInitRec(&Idx,NULL,0,sizeof(long));
  Allocated = TRUE;
}


editclass::~editclass(void) {
  int          X;
  scrnrectype *p;

  if (SaveRec != NULL) {
    free(SaveRec);
    SaveRec = NULL;
  }

  if (Screen != NULL) {
    for (X = 0, p = Screen; X < MaxScrnLines; X++, p++) {
      if (p->Rec != NULL) {
        free(p->Rec);
        p->Rec = NULL;
      }
    }
    free(Screen);
    Screen = NULL;
  }

  if (SaveKeyNum != NULL) {
    memcpy(ExitKeyNum,SaveKeyNum,NumExitKeys * sizeof(char));
    free(SaveKeyNum);
    SaveKeyNum = NULL;
  }

  if (SaveKeyFlag != NULL) {
    memcpy(ExitKeyFlag,SaveKeyFlag,NumExitKeys * sizeof(int));
    free(SaveKeyFlag);
    SaveKeyFlag = NULL;
  }

  if (KeyDesc != NULL) {
    free(KeyDesc);
    KeyDesc = NULL;
  }

  if (Allocated) {
    VMDone(&DataSet);
    VMDone(&Idx);
    restorescreen(&ScrnBuf);
  }
}


void pascal editclass::addexitkey(char KeyNum, int Flag, char *Desc) {
  if (NextExitKey < NumExitKeys) {
    ExitKeyNum[NextExitKey]  = KeyNum;
    ExitKeyFlag[NextExitKey] = Flag;
    KeyDesc[NextExitKey-NumInternalKeys] = Desc;
    NextExitKey++;
  }
}


void pascal editclass::showfooters(void) {
  int  X;
  int  Len;
  int  New;
  char Str[80];

  clsbox(1,Scrn_BottomRow-1,78,Scrn_BottomRow-1,Colors[DESC]);

  strcpy(Str,"ESC=Exit AltI=Insert AltD=Delete AltR=Repeat Alt5=25/50");
  for (X = 0, Len = strlen(Str); X < NumExitKeys-NumInternalKeys; X++) {
    New = strlen(KeyDesc[X]);
    if (New > 0) {
      New++;
      if (Len + New < 78) {
        Str[Len] = ' ';
        strcpy(&Str[Len+1],KeyDesc[X]);
        Len += New;
      }
    }
  }

  fastcenter(Scrn_BottomRow-1,Str,Colors[DESC]);
}


void pascal editclass::paint(void) {
  setcursor(CUR_BLANK);
  gotoxy(1,Scrn_BottomRow-1);
  clscolor(Colors[OUTBOX]);
  generalscreen(Title,NULL);
  showheaders();
  showfooters();
  setcursor(CUR_NORMAL);
}


void pascal editclass::newrecord(void *Rec) {
  memset(Rec,0,RecSize);
}


void pascal editclass::addidx(long NewPos) {
  long *p;

  TotalIdx++;

  if (TotalRecs > VMRecordCount(&Idx))
    p = (long *) VMRecordCreate(&Idx,sizeof(long),NULL,NULL);
  else
    p = (long *) VMRecordGetByIndex(&Idx,TotalIdx,NULL);

  *p = NewPos;
  VMRecordChanged(&Idx);
}


void pascal editclass::insertrecord(long CurRec) {
  long             X;
  void            *pNewRec;
  long            *p;
  long             NewPos;
  long             Save;

  pNewRec = (void *) VMRecordCreate(&DataSet,RecSize,&NewPos,NULL);
  newrecord(pNewRec);

  for (X = CurRec + 1; X <= TotalRecs; X++) {
    p = (long *) VMRecordGetByIndex(&Idx,X,NULL);
    Save = *p;
    *p = NewPos;
    NewPos = Save;
    VMRecordChanged(&Idx);
  }

  TotalRecs++;
  addidx(NewPos);
}


void pascal editclass::deleterecord(long CurRec) {
  long   X;
  long  *p;
  long   NewPos;
  long   Save;

  NewPos = VM_INVALID_POS;

  for (X = TotalIdx;  X >= CurRec; X--) {
    p = (long *) VMRecordGetByIndex(&Idx,X,NULL);
    Save = *p;
    *p = NewPos;
    NewPos = Save;
    VMRecordChanged(&Idx);
  }

  TotalRecs--;
  TotalIdx--;

  if (TotalRecs == 0)   // if we just deleted the very last record, then
    insertrecord(0);    // add a new blank record at the top

}


void pascal editclass::addrecord(void *Rec) {
  void *p;
  long  Pos;

  p = (void *) VMRecordCreate(&DataSet,RecSize,&Pos,NULL);
  memcpy(p,Rec,RecSize);

  TotalRecs++;
  addidx(Pos);
}


void * pascal editclass::getrecord(long RecNum) {
  long *pPos;

  pPos = (long *) VMRecordGetByIndex(&Idx,RecNum,NULL);
  return (VMRecordGetByPos(&DataSet,*pPos));
}


void pascal editclass::updaterecord(void *Rec, long Pos) {
  VMWrite(&DataSet,Rec,Pos,RecSize);
}


int pascal editclass::load(char *Name) {
  DOSFILE File;

  stripright(Name,' ');
  if (Name[0] == 0)
    return(-1);

  FileName = Name;
  if (fileexist(FileName) != 255) {
    if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&File) == -1)
      return(-1);

    loadrecords(&File);
    dosfclose(&File);
  }

  if (TotalRecs == 0) {
    newrecord(SaveRec);
    addrecord(SaveRec);
  }

  return(0);
}


void pascal editclass::save(void) {
  DOSFILE File;

  if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&File) != -1) {
    saverecords(&File);
    dosfclose(&File);
  }
}


void pascal editclass::fillscreen(long TopRec) {
  int   X;
  long  Pos;
  long  RecNum;

  for (X = 0, RecNum = TopRec; X < CurNumLines; RecNum++, X++) {
    if (RecNum > TotalRecs)
      return;

    Pos = * (long *) VMRecordGetByIndex(&Idx,RecNum,NULL);
    Screen[X].Pos =  Pos;
    memcpy(Screen[X].Rec,VMRecordGetByPos(&DataSet,Pos),RecSize);
  }
}


void pascal editclass::displayscreen(long TopRec) {
  int          LineNum;
  int          X;
  scrnrectype *s;
  long         RecNum;

  fillscreen(TopRec);
//clsbox(LeftSide,TopLine,RightSide,TopLine+CurNumLines-1,Colors[OUTBOX]);

  LineNum = TopLine;
  RecNum  = TopRec;
  s       = Screen;

  for (X = 0; X < CurNumLines; X++, LineNum++, RecNum++, s++) {
    clsbox(LeftSide,LineNum,RightSide,LineNum,Colors[ANSWER]);
    if (RecNum <= TotalRecs)
      displayrecord(RecNum,LineNum,s->Rec);
  }
}


#pragma argsused
void pascal editclass::handlekeys(void *Rec) {
  void *p;

  switch (KeyFlags) {
    case ALTI  : insertrecord(CurRec);
                 KeyFlags = DN;
                 processcursorkeys(&Index,&TopRec,&Column,TotalRecs,NumColumns,CurNumLines);
                 Changed = NeedToDisplay = TRUE;
                 break;
    case ALTD  : deleterecord(CurRec);
                 if (CurRec > TotalRecs) {
                   KeyFlags = UP;
                   processcursorkeys(&Index,&TopRec,&Column,TotalRecs,NumColumns,CurNumLines);
                 }
                 Changed = NeedToDisplay = TRUE;
                 break;
    case ALTR  : insertrecord(CurRec);
                 memcpy(SaveRec,getrecord(CurRec),RecSize);
                 p = getrecord(CurRec+1);
                 memcpy(p,SaveRec,RecSize);
                 VMRecordChanged(&DataSet);

                 KeyFlags = DN;
                 processcursorkeys(&Index,&TopRec,&Column,TotalRecs,NumColumns,CurNumLines);
                 Changed = NeedToDisplay = TRUE;
                 break;
    case ALT5  : if (getfont() == BIGFONT) {
                   setfont(SMALLFONT);
                   if (getfont() == BIGFONT)  // font did not change, so
                     break;                   // get out now
                 } else {
                   setfont(BIGFONT);
                 }
                 setupscreen();
                 paint();
                 if (Index > CurNumLines - 1)
                   Index = CurNumLines - 1;
                 NeedToDisplay = TRUE;
                 break;
    case ESC   : ExitEditor = TRUE;
                 break;
    default    : NeedToDisplay = processcursorkeys(&Index,&TopRec,&Column,TotalRecs,NumColumns,CurNumLines);
                 break;
  }
}


bool pascal editclass::edit(void) {
  fonttype     SaveFont;
  scrnrectype *s;
  char         Str[80];

  Changed  = FALSE;
  TopRec   = 0;
  Index    = 0;
  Column   = 0;
  KeyFlags = 0;
  SaveFont = getfont();

  paint();

  showkeystatus();
  NeedToDisplay = TRUE;
  ExitEditor = FALSE;

top:
  while (! ExitEditor) {
    CurRec = TopRec + Index + 1;
    LineNum = TopLine + Index;
    scale(CurRec-1,TotalRecs-1);

    if (NeedToDisplay) {
      displayscreen(TopRec+1);
      NeedToDisplay = FALSE;
    }

    s = &Screen[Index];

    memcpy(SaveRec,s->Rec,RecSize);
    editrecord(s->Rec);

    if (memcmp(SaveRec,s->Rec,RecSize) != 0) {
      updaterecord(s->Rec,s->Pos);
      Changed = TRUE;
    }

    handlekeys(s->Rec);

    if (memcmp(SaveRec,s->Rec,RecSize) != 0) {
      updaterecord(s->Rec,s->Pos);
      Changed = TRUE;
    }
  }

  if (Changed) {
    sprintf(Str,"Exit %s",Title);
    switch(editexit(Str)) {
      case 'Y': save();
                break;
      case 'N': KeyFlags = NOTHING;
                ExitEditor = FALSE;
                goto top;
      case 'A': break;
    }
  }

  if (getfont() != SaveFont)
    setfont(SaveFont);

  KeyFlags = ESC;
  return(Changed);
}
