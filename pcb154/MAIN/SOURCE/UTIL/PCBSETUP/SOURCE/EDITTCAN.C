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
#include <dosfunc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "edit.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif


#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2


class edittcanclass : public editclass {
  public:
    edittcanclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


typedef struct {
  char Name[26];
} tcantype;


void pascal edittcanclass::loadrecords(DOSFILE *File) {
  tcantype  Rec;
  char      Buf[256];

  while (dosfgets(Buf,sizeof(Buf),File) != -1) {
    newrecord(&Rec);
    maxstrcpy(Rec.Name,Buf,sizeof(Rec.Name));
    addrecord(&Rec);
  }
}


void pascal edittcanclass::saverecords(DOSFILE *File) {
  long      X;
  tcantype *p;
  char      Temp[100];

  for (X = 1; X <= TotalRecs; X++) {
    p = (tcantype *) getrecord(X);
    stripright(p->Name,' ');
    sprintf(Temp,"%s\r\n",p->Name);
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


void pascal edittcanclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  tcantype *p = (tcantype *) Rec;
  char      Temp[21];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,Temp   ,Colors[ANSWER]);
  fastprint(LEFTSIDE+ 6,LineNum,p->Name,Colors[ANSWER]);
}


void pascal edittcanclass::showheaders(void) {
  fastprint(LEFTSIDE+8,TOPLINE-2,   "User Trashcan Names",Colors[DISPLAY]);
  fastprint(LEFTSIDE+6,TOPLINE-1,"อออออออออออออออออออออออออ",Colors[DISPLAY]);
//fastprint(LEFTSIDE,Scrn_BottomRow-1,"  ESC=Exit   Alt-R=Repeat a Line   Alt-I=Insert a Line   Alt-D=Delete a Line  ",Colors[DESC]);
}


void pascal edittcanclass::editrecord(void *Rec) {
  tcantype *p = (tcantype *) Rec;

  inputstr(LEFTSIDE+6,LineNum,25,"",p->Name,p->Name,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,0);
  stripright(p->Name,' ');
}


/********************************************************************
*
*  Function: edittcan()
*
*  Desc    : Allows the user to edit the tcan file.
*
*/

void pascal edittcan(void) {
  edittcanclass  Tcan("User Name Trashcan",sizeof(tcantype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,0,0);

  if (Tcan.load(PcbData.TcnFile) == -1)
    return;

  Tcan.edit();
}
