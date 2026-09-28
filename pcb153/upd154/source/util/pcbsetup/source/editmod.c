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
#include "setup.h"
#include "setup.ext"
#include "edit.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif

enum { F2 = NEXTKEY };


#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       4

typedef struct {
  unsigned ConfNum;
  char     Name[70];
} modtype;


class editmodclass : public editclass {
  public:
    editmodclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal showfooters(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec) { editclass::handlekeys(Rec); }
    void pascal showconfname(unsigned ConfNum);

    unsigned    LastConf;
};


void pascal editmodclass::loadrecords(DOSFILE *File) {
  char *  p;
  modtype Rec;
  char    Buf[256];

  while (dosfgets(Buf,256,File) != -1) {
    newrecord(&Rec);
    Rec.ConfNum = atoi(Buf);
    if ((p = strchr(Buf,',')) == NULL)
      maxstrcpy(Rec.Name,&Buf[6],sizeof(Rec.Name));
    else
      maxstrcpy(Rec.Name,p+1,sizeof(Rec.Name));
    addrecord(&Rec);
  }
}


void pascal editmodclass::saverecords(DOSFILE *File) {
  modtype  *p;
  long      X;
  char      Temp[128];

  for (X = 1; X <= TotalRecs; X++) {
    p = (modtype *) getrecord(X);
    sprintf(Temp,"%u,%s\r\n",p->ConfNum,p->Name);
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


#pragma argsused
void pascal editmodclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  modtype *p = (modtype *) Rec;
  char     Temp[10];

  sprintf(Temp,"%5u",p->ConfNum);
  fastprint(LEFTSIDE+1,LineNum,Temp,Colors[ANSWER]);
  fastprint(LEFTSIDE+8,LineNum,p->Name,Colors[ANSWER]);
}



void pascal editmodclass::showheaders(void) {
  fastprint(LEFTSIDE+1,TOPLINE-2,"Conf#  USENET Moderator Name",Colors[DISPLAY]);
  fastprint(LEFTSIDE+1,TOPLINE-1,"อออออ  อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
}


void pascal editmodclass::showfooters(void) {
  editclass::showfooters();

  LastConf = 0xFFFF;
  fastprint(LEFTSIDE+1,Scrn_BottomRow-3,"Conference Name:",Colors[DISPLAY]);
}


void pascal editmodclass::showconfname(unsigned ConfNum) {
  pcbconftype  Conf;

  if (ConfNum != LastConf) {
    getconfrecord(ConfNum,&Conf);
    clsbox(LEFTSIDE+18,Scrn_BottomRow-3,78,Scrn_BottomRow-3,Colors[QUESTION]);
    fastprint(LEFTSIDE+18,Scrn_BottomRow-3,Conf.Name,Colors[QUESTION]);
    LastConf = ConfNum;
  }
}


void pascal editmodclass::editrecord(void *Rec) {
  static char  ALLNAME[] = {13, 0,'A','z',  0,'0','9',  '.','-','@','!','|','%','+'};
  modtype     *p = (modtype *) Rec;

  showconfname(p->ConfNum);

  switch (Column) {
    case 0: inputnum(LEFTSIDE+1,LineNum,5,"",&p->ConfNum,vUNSIGNED,0);
            showconfname(p->ConfNum);
            break;
    case 1: inputstr(LEFTSIDE+8,LineNum,sizeof(p->Name)-1,"",p->Name,p->Name,ALLNAME,INPUT_DEFAULT,0);
            stripright(p->Name,' ');
            break;
  }
}



void pascal editmodfile(void) {
  editmodclass ModList("USENET Moderator List Editor",sizeof(modtype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,0);

  if (ModList.load(PcbData.uucpModFile) == -1)
    return;

  ModList.edit();
}
