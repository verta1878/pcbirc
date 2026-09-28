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
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "edit.hpp"
#include "rules.h"
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

enum {F2=NEXTKEY};

static const int LEFTSIDE  =  1;
static const int RIGHTSIDE = 78;
static const int TOPLINE   =  6;
static const int BOTTOM    =  2;


class editrulesclass : public editclass {
  public:
    editrulesclass(char *TitleStr, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(TitleStr,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editrulesclass(void) { }
    void  newrecord(void *Rec) { editclass::newrecord(Rec); }
    void  loadrecords(DOSFILE *File);
    void  saverecords(DOSFILE *File);
    void  showheaders(void);
    void  displayrecord(long RecNum, int LineNum, void *Rec);
    void  editrecord(void *Rec);
    void  handlekeys(void *Rec);

    int  PrevColumn;
    char Group;
};



int  readonerule(DOSFILE *File, rulestype *Rec) {
  char Temp[2048];

  memset(Temp,0,sizeof(Temp));
  if (dosfgets(Temp,sizeof(Temp),File) == -1)
    return(-1);

  Rec->Cnf  = atoi(Temp);
  Rec->Dir  = atoi(&Temp[6]);
  Rec->Days = atoi(&Temp[12]);

  maxstrcpy(Rec->Name,&Temp[18],sizeof(Rec->Name));
  removespaces(Rec->Name,sizeof(Rec->Name));

  maxstrcpy(Rec->InSpec,&Temp[31],sizeof(Rec->InSpec));
  removespaces(Rec->InSpec,sizeof(Rec->InSpec));

  maxstrcpy(Rec->Criteria,&Temp[44],sizeof(Rec->Criteria));
  removespaces(Rec->Criteria,sizeof(Rec->Criteria));
  return(0);
}


void  editrulesclass::loadrecords(DOSFILE *File) {
  rulestype  Rec;

  newrecord(&Rec);
  while (readonerule(File,&Rec) != -1) {
    addrecord(&Rec);
    newrecord(&Rec);
  }
}


void  editrulesclass::saverecords(DOSFILE *File) {
  long        X;
  rulestype  *p;
  char        Temp[60];

  for (X = 1; X <= TotalRecs; X++) {
    p = (rulestype *) getrecord(X);
    sprintf(Temp,"%5u %5u %5d %-13.13s%-13.13s",p->Cnf,p->Dir,p->Days,p->Name,p->InSpec);
    dosfputs(Temp,File);          //lint !e534
    dosfputs(p->Criteria,File);   //lint !e534
    dosfputs("\r\n",File);        //lint !e534
  }
}


void  editrulesclass::displayrecord(long RecNum, int LineNo, void *Rec) {
  int        X;
  rulestype *p;
  char       Temp[80];

  p = (rulestype *) Rec;

  if (Group == 0) {
    if (p->Cnf == 0xFFFF)
      strcpy(Temp,"   X");
    else
      sprintf(Temp,"%4u",p->Cnf);
    fastprint(LEFTSIDE+1,LineNo,Temp,Colors[ANSWER]);

    sprintf(Temp,"%4u %4d %-12.12s %-12.12s %-35.35s",p->Dir,p->Days,p->Name,p->InSpec,p->Criteria);
    X = LEFTSIDE+6;
  } else {
    sprintf(Temp,"%-76.76s",p->Criteria);
    X = LEFTSIDE+1;
  }

  fastprint(X,LineNo,Temp,Colors[ANSWER]);
}



void  editrulesclass::showheaders(void) {
  clsbox(LEFTSIDE+1,TOPLINE-3,RIGHTSIDE,TOPLINE-1,Colors[DISPLAY]);

  if (Group == 0) {
    fastprint(LEFTSIDE+3,TOPLINE-3,  "Target         File Name   File Name"                                      ,Colors[DISPLAY]);
    fastprint(LEFTSIDE+2,TOPLINE-2, "Cnf  Dir Days w/ Wildcards   Inside     Boolean Search Criteria"            ,Colors[DISPLAY]);
    fastprint(LEFTSIDE+1,TOPLINE-1,"ออออ ออออ ออออ ออออออออออออ ออออออออออออ อออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
  } else {
    fastprint(LEFTSIDE+1,TOPLINE-2,"Boolean Search Criteria"                                                     ,Colors[DISPLAY]);
    fastprint(LEFTSIDE+1,TOPLINE-1,"ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
  }
}




void  editrulesclass::handlekeys(void *Rec) {
  rulestype *p = (rulestype *) Rec;

  switch(KeyFlags) {
    case F2      : NewDirNumber   = p->Dir;
                   NewBoardNumber = p->Cnf;
                   editdirlist(p->Cnf,p->Dir,TRUE,TRUE);
                   setupscreen();
                   NeedToDisplay = (p->Dir != NewDirNumber || p->Cnf != NewBoardNumber);
                   p->Cnf = NewBoardNumber;
                   p->Dir = NewDirNumber;
                   break;
    default:       editclass::handlekeys(Rec);
                   break;
  }
}


void  editrulesclass::editrecord(void *Rec) {
  char static  WILDCARDNAMES[] = { 17, '*','?','-', '.', 'แ',0, '#', ')',0, '0', ':',0, '@', 'z', 0, 128, 255};
  int          Okay;
  rulestype    *p = (rulestype *) Rec;
  char         ConfStr[6];
  char         SearchInput[256];

  if (Column == 5 && PrevColumn != 5) {
    Group = 1;
    showheaders();
    displayscreen(TopRec+1);
  } else if (PrevColumn == 5 && Column != 5) {
    Group = 0;
    showheaders();
    displayscreen(TopRec+1);
  }

  switch (Column) {
    case 0: do {
              if (p->Cnf == 65535U) {
                ConfStr[0] = 'X'; ConfStr[1] = 0;
              } else
                ltoa(p->Cnf,ConfStr,10);
              inputstr(LEFTSIDE+1,LineNum, 4,"",ConfStr,ConfStr,ALLCONF,INPUT_CAPS|INPUT_CLEAR,FEDIT+0);
              if (ConfStr[0] == 'X') {
                p->Cnf = 65535U;
                p->Dir = 0;
                Okay = 0;
              } else {
                p->Cnf = atoi(ConfStr);
                sprintf(ConfStr,"%4u",p->Cnf);
                fastprint(LEFTSIDE+1,LineNum,ConfStr,Colors[ANSWER]);
                Okay = checkconf(p->Cnf);
              }
            } while (Okay == -1);
            break;
    case 1: do {
              inputnum(LEFTSIDE+6,LineNum,4,"",&p->Dir,vUNSIGNED,FEDIT+1);
              Okay = checkdir(p->Cnf,p->Dir);
            } while (Okay == -1);
            break;
    case 2: inputnum(LEFTSIDE+11,LineNum,4,"",&p->Days,vINT,FILEMOVEMENT+0);
            break;
    case 3: inputstr(LEFTSIDE+16,LineNum,12,"",p->Name,p->Name,WILDCARDNAMES,INPUT_CAPS|INPUT_CLEAR,FILEMOVEMENT+1);
            break;
    case 4: inputstr(LEFTSIDE+29,LineNum,12,"",p->InSpec,p->InSpec,WILDCARDNAMES,INPUT_CAPS|INPUT_CLEAR,FILEMOVEMENT+2);
            break;
    case 5: while (1) {
              inputextend(LEFTSIDE+1,LineNum,76,sizeof(p->Criteria)-1,p->Criteria,ALLCHAR,INPUT_CAPS,FILEMOVEMENT+3);

              // validate the criteria by running it through the parsersearch
              // tokenizer, if it returns a value less than zero then there is
              // an error in the token stream
              stripright(p->Criteria,' ');
              if (p->Criteria[0] == 0)
                break;

              strcpy(SrchText,p->Criteria);
              if (tokenscan(SrchText,SearchInput,FALSE) <= 0) {
                memset(&MsgData,0,sizeof(MsgData));
                MsgData.Save    = TRUE;
                MsgData.AutoBox = TRUE;
                MsgData.Msg1    = "A Punctuation Error Has Been Detected";
                MsgData.Line1   = Scrn_BottomRow - 8;
                MsgData.Color1  = Colors[HEADING];
                MsgData.Msg2    = "Check Parentheses '(' ')' and Quotes '\"'.";
                MsgData.Line2   = Scrn_BottomRow - 6;
                MsgData.Color2  = Colors[DISPLAY];
                showmessage();
                continue;
              } else {
                stopsearch();
                break;
              }
            }
            break;
  }

  PrevColumn = Column;
}



void  editrulesfile(char *FileName) {
  editrulesclass Rules("Auto-Selection Rules",sizeof(rulestype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,5,1);

  ShowSwapMessage = TRUE;
  ShowSwapQuick   = TRUE;

  Rules.Group      = 0;
  Rules.PrevColumn = 0;
  Rules.addexitkey(60,F2,"F2=Select DIR");    // scancode = 60, key = F2

  if (Rules.load(FileName) == -1)
    return;

  Rules.edit();
}


void  editrules(void) {
  char  Name[66];

  strcpy(Name,Work.RulesName);

  setcursor(CUR_NORMAL);
  boxcls(3,Scrn_BottomRow-7,76,Scrn_BottomRow-1,Colors[HEADING],SINGLE);
  fastcenter(Scrn_BottomRow-5,"Enter Filename for Auto-Selection Rules",Colors[DISPLAY]);
  inputstr(5,Scrn_BottomRow-3,59,"Filename",Name,Name,ALLFILE,INPUT_CAPS,0);

  if (KeyFlags == ESC || Name[0] == 0)
    return;

  strcpy(Work.RulesName,Name);
  editrulesfile(Name);
}
