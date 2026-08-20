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
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include <dosfunc.h>
#include <country.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "edit.hpp"
#include "rules.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

enum {F2=NEXTKEY};

static const int LEFTSIDE  =  1;
static const int RIGHTSIDE = 78;
static const int TOPLINE   =  6;
static const int BOTTOM    =  2;


class editmovesclass : public editclass {
  public:
    editmovesclass(char *TitleStr, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(TitleStr,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editmovesclass(void) { }
    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal showfooters(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
    void pascal sortrecords(void);
};


static VMDataSet *pDataSet;

static VM_SHINT comparemoves(const void *Rec1, const void *Rec2) {
  movestype *a;
  movestype *b;

  a = (movestype *) VMRecordGetByPos(pDataSet,(*(long *) Rec1));
  b = (movestype *) VMRecordGetByPos(pDataSet,(*(long *) Rec2));

  if (a->SrceCnf < b->SrceCnf)
    return(-1);
  if (a->SrceCnf > b->SrceCnf)
    return(1);

  if (a->SrceDir < b->SrceDir)
    return(-1);
  if (a->SrceDir > b->SrceDir)
    return(1);

  if (a->TargetCnf < b->TargetCnf)
    return(-1);
  if (a->TargetCnf > b->TargetCnf)
    return(1);

  if (a->TargetDir < b->TargetDir)
    return(-1);
  if (a->TargetDir > b->TargetDir)
    return(1);

  return(strcmp(a->Name,b->Name));
}


void pascal editmovesclass::sortrecords(void) {
  long SortBuf[1024];

  if (TotalRecs > 1) {
    pDataSet = &DataSet;
    VMSort(&Idx,sizeof(long),1,TotalRecs,VM_FALSE,comparemoves,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));
  }
}


int pascal readonemove(DOSFILE *File, movestype *Rec) {
  char Temp[80];

  memset(Temp,0,sizeof(Temp));
  if (dosfgets(Temp,sizeof(Temp),File) == -1)
    return(-1);

  Rec->SrceCnf   = atoi(Temp);
  Rec->SrceDir   = atoi(&Temp[6]);
  Rec->TargetCnf = atoi(&Temp[25]);
  Rec->TargetDir = atoi(&Temp[31]);
  Rec->FileDate  = datetojulian(&Temp[37]);
  Rec->Days      = atoi(&Temp[46]);
  Rec->CopyMove  = Temp[52];

  maxstrcpy(Rec->Name,&Temp[12],sizeof(Rec->Name));
  removespaces(Rec->Name,sizeof(Rec->Name));

  if (Rec->FileDate == 0)
    Rec->FileDate = datetojulian("010180");

  return(0);
}


void pascal editmovesclass::loadrecords(DOSFILE *File) {
  movestype  Rec;

  newrecord(&Rec);
  while (readonemove(File,&Rec) != -1) {
    addrecord(&Rec);
    newrecord(&Rec);
  }
}


int pascal writeonemove(DOSFILE *File, unsigned SrceCnf, unsigned SrceDir, char *Name, unsigned TargetCnf, unsigned TargetDir, char *FileDate, unsigned Days, char CopyMove) {
  char Temp[80];

  sprintf(Temp,"%5u %5u %-12.12s %5u %5u %s %5d %c\r\n",
          SrceCnf,
          SrceDir,
          Name,
          TargetCnf,
          TargetDir,
          FileDate,
          Days,
          CopyMove);
  return(dosfputs(Temp,File));
}


void pascal editmovesclass::saverecords(DOSFILE *File) {
  long        X;
  movestype  *p;

  for (X = 1; X <= TotalRecs; X++) {
    p = (movestype *) getrecord(X);
    if (writeonemove(File,p->SrceCnf,p->SrceDir,p->Name,p->TargetCnf,p->TargetDir,juliantodate(p->FileDate),p->Days,p->CopyMove) == -1)
      break;
  }
}


void pascal editmovesclass::displayrecord(long RecNum, int LineNo, void *Rec) {
  movestype *p;
  char       FileDate[9];
  char       MoveDate[9];
  char       Temp[20];
  char       Str[80];

  p = (movestype *) Rec;

  if (p->TargetCnf == 0xFFFF)
    strcpy(Temp,"    X     0");
  else
    sprintf(Temp,"%5u %5u",p->TargetCnf,p->TargetDir);

  strcpy(FileDate,countrydate(juliantodate(p->FileDate)));
  strcpy(MoveDate,countrydate(juliantodate(p->FileDate+p->Days)));

  sprintf(Str,"%5u %5u %-12.12s %s %s %5d %s   %c",
          p->SrceCnf,
          p->SrceDir,
          p->Name,
          FileDate,
          Temp,
          p->Days,
          MoveDate,
          p->CopyMove);

  fastprint(LEFTSIDE+1,LineNo,Str,Colors[ANSWER]);
}



void pascal editmovesclass::showheaders(void) {
  fastprint(LEFTSIDE+3,TOPLINE-3,  "Source                   File     Target    Move    Move   Copy",Colors[DISPLAY]);
  fastprint(LEFTSIDE+2,TOPLINE-2, "Cnf   Dir   File Name     Date    Cnf   Dir  Days    Date   Move",Colors[DISPLAY]);
  fastprint(LEFTSIDE+1,TOPLINE-1,"อออออ อออออ ออออออออออออ ออออออออ อออออ อออออ อออออ ออออออออ ออออ",Colors[DISPLAY]);
}


void pascal editmovesclass::showfooters(void) {
  clsbox(1,Scrn_BottomRow-1,78,Scrn_BottomRow-1,Colors[DESC]);
  fastcenter(Scrn_BottomRow-1,"ESC=Exit  AltD=Delete  AltR=Repeat  Alt5=25/50  F2=Select DIR",Colors[DESC]);
}


void pascal editmovesclass::handlekeys(void *Rec) {
  movestype    *p = (movestype *) Rec;

  switch(KeyFlags) {
    case F2      : NewDirNumber   = p->TargetDir;
                   NewBoardNumber = p->TargetCnf;
                   editdirlist(p->TargetCnf,p->TargetDir,TRUE,TRUE);
                   setupscreen();
                   NeedToDisplay = (p->TargetDir != NewDirNumber || p->TargetCnf != NewBoardNumber);
                   p->TargetCnf = NewBoardNumber;
                   p->TargetDir = NewDirNumber;
                   break;
    case ALTI   :  break;  // i.e. don't allow ALT-I
    default     :  editclass::handlekeys(Rec);
                   break;
  }
}


void pascal editmovesclass::editrecord(void *Rec) {
  static char  MASKCM[3] = {2, 'C', 'M'};
  int          Okay;
  char         CopyMove[2];
  movestype    *p = (movestype *) Rec;
  char         ConfStr[6];
  char         Days[6];
  char         Date[9];

  switch (Column) {
    case 0: do {
              if (p->TargetCnf == 65535U) {
                ConfStr[0] = 'X'; ConfStr[1] = 0;
              } else
                ltoa(p->TargetCnf,ConfStr,10);
              inputstr(LEFTSIDE+35,LineNum, 5,"",ConfStr,ConfStr,ALLCONF,INPUT_CAPS|INPUT_CLEAR,FEDIT+0);
              if (ConfStr[0] == 'X') {
                p->TargetCnf = 65535U;
                p->TargetDir = 0;
                Okay = 0;
              } else {
                p->TargetCnf = atoi(ConfStr);
                sprintf(ConfStr,"%5u",p->TargetCnf);
                fastprint(LEFTSIDE+35,LineNum,ConfStr,Colors[ANSWER]);
                Okay = checkconf(p->TargetCnf);
              }
            } while (Okay == -1);
            break;
    case 1: do {
              inputnum(LEFTSIDE+41,LineNum,5,"",&p->TargetDir,vUNSIGNED,FEDIT+1);
              Okay = checkdir(p->TargetCnf,p->TargetDir);
            } while (Okay == -1);
            break;
    case 2: if (p->FileDate != 0) {
              inputnum(LEFTSIDE+47,LineNum,5,"",&p->Days,vINT,FILEMOVEMENT+0);
              strcpy(Date,countrydate(juliantodate(p->FileDate+p->Days)));
              fastprint(LEFTSIDE+53,LineNum,Date,Colors[ANSWER]);
            }
            break;
    case 3: if (p->FileDate != 0) {
              strcpy(Date,countrydate(juliantodate(p->FileDate+p->Days)));
              inputstr(LEFTSIDE+53,LineNum,8,"",Date,Date,ALLDATE,INPUT_DEFAULT,FILEMOVEMENT+4);
              uncountrydate(Date);
              p->Days = datetojulian(Date) - p->FileDate;
              sprintf(Days,"%5d",p->Days);
              fastprint(LEFTSIDE+47,LineNum,Days,Colors[ANSWER]);
            }
            break;
    case 4: CopyMove[0] = p->CopyMove;
            CopyMove[1] = 0;
            inputstr(LEFTSIDE+64,LineNum,1,"",CopyMove,CopyMove,MASKCM,INPUT_CAPS,FILEMOVEMENT+5);
            p->CopyMove = (CopyMove[0] == 'C' ? 'C' : 'M');
            break;
  }
}


long pascal loadmoves(editmovestype Action) {
  editmovesclass Moves("Scheduled Moves",sizeof(movestype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,4,1);
  long NumRecs;

  Moves.addexitkey(60,F2,"F2=Select DIR");    // scancode = 60, key = F2

  if (Moves.load(Work.ProcName) == -1)
    return(0);

  NumRecs = Moves.TotalRecs;
  Moves.sortrecords();

  if (Action == EDITMOVES)
    Moves.edit();
  else
    Moves.save();

  return(NumRecs);
}


void pascal editmoves(void) {
  loadmoves(EDITMOVES);
}
