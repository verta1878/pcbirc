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
#include "setup.h"
#include "setup.ext"
#include "edit.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif

enum { F2=NEXTKEY };

enum { EDITDIR, SHOWDIRONLY };

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      6
#define BOTTOM       4


static int          Group;


class editdirclass : public editclass {
  public:
    editdirclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal showfooters(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};



void pascal editdirclass::loadrecords(DOSFILE *File) {
  DirListType2  Buffer;
  DirListType   Rec;

  while (dosfread(&Buffer,sizeof(Buffer),File) == sizeof(Buffer)) {
    newrecord(&Rec);
    memcpy(Rec.DirPath,Buffer.DirPath,sizeof(Buffer.DirPath));  removespaces(Rec.DirPath,sizeof(Rec.DirPath));
    memcpy(Rec.DskPath,Buffer.DskPath,sizeof(Buffer.DskPath));  removespaces(Rec.DskPath,sizeof(Rec.DskPath));
    memcpy(Rec.DirDesc,Buffer.DirDesc,sizeof(Buffer.DirDesc));  removespaces(Rec.DirDesc,sizeof(Rec.DirDesc));
    Rec.SortType = Buffer.SortType;
    addrecord(&Rec);
  }
}


void pascal editdirclass::saverecords(DOSFILE *File) {
  long          X;
  DirListType2  Buffer;
  DirListType  *p;

  for (X = 1; X <= TotalRecs; X++) {
    p = (DirListType *) getrecord(X);

    memset(&Buffer,' ',sizeof(Buffer));
    stripright(p->DirPath,' ');
    memcpy(Buffer.DirPath,p->DirPath,strlen(p->DirPath));
    memcpy(Buffer.DskPath,p->DskPath,strlen(p->DskPath));
    memcpy(Buffer.DirDesc,p->DirDesc,strlen(p->DirDesc));
    Buffer.SortType = p->SortType;
    if (dosfwrite(&Buffer,sizeof(Buffer),File) == -1)
      break;
  }
}




void pascal editdirclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  DirListType *p;
  char         Temp[10];

  p = (DirListType *) Rec;

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LeftSide,LineNum,Temp,Colors[ANSWER]);
  fastprint(LeftSide+7,LineNum,p->DirPath,Colors[ANSWER]);

  switch (Group) {
    case 0: fastprint(LeftSide+39,LineNum,p->DskPath,Colors[ANSWER]);
            Temp[0] = p->SortType + '0';
            Temp[1] = 0;
            fastprint(LeftSide+71,LineNum,Temp,Colors[ANSWER]);
            break;
    case 1: fastprint(LeftSide+39,LineNum,p->DirDesc,Colors[ANSWER]);
            break;
  }
}



static void near pascal displaydirhead(int Group) {
  switch (Group) {
    case 0: fastprint(LEFTSIDE+39,TOPLINE-2,"Hard Disk Subdirectory        Sort",Colors[DISPLAY]);
            fastprint(LEFTSIDE+39,TOPLINE-1,"ออออออออออออออออออออออ        ออออ",Colors[DISPLAY]);
            break;
    case 1: fastprint(LEFTSIDE+39,TOPLINE-2,"Description of Directory Contents ",Colors[DISPLAY]);
            fastprint(LEFTSIDE+39,TOPLINE-1,"ออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
            break;
  }
}


void pascal editdirclass::showheaders(void) {
  fastcenter(2,Conf.Name,Colors[HEADING]);
  fastprint(LEFTSIDE+7 ,TOPLINE-2,"DIR Text File Name & Path",Colors[DISPLAY]);
  fastprint(LEFTSIDE+7 ,TOPLINE-1,"อออออออออออออออออออออออออ",Colors[DISPLAY]);
  displaydirhead(Group);
}

void pascal editdirclass::showfooters(void) {
  fastprint(1,Scrn_BottomRow-3,"  Sort orders:   1 = File Name (Ascending)    3 = File Name (Descending)      ",Colors[DESC]);
  fastprint(1,Scrn_BottomRow-2,"    0 = No Sort  2 = File Date (Ascending)    4 = File Date (Descending)      ",Colors[DESC]);
  editclass::showfooters();
//fastprint(1,Scrn_BottomRow-3,"  ESC=Exit   F2=Scrn Toggle   Alt-R=Repeat line   Alt-I=Insert   Alt-D=Delete ",Colors[DESC]);
}



void pascal editdirclass::handlekeys(void *Rec) {
  switch (KeyFlags) {
    case F2    : Group = !Group;
                 displaydirhead(Group);
                 NeedToDisplay = TRUE;
                 NumColumns = (Group == 0 ? 2 : 1);
                 if (Column == 2 && Group != 0)
                   Column = 1;
                 break;
    default    : editclass::handlekeys(Rec);
                 break;
  }
}



void pascal editdirclass::editrecord(void *Rec) {
  DirListType *p = (DirListType *) Rec;

  switch (Group) {
    case 0:  switch (Column) {
               case 0: inputstr(LEFTSIDE+7 ,LineNum,30,"",p->DirPath,p->DirPath,ALLFILE,INPUT_CAPS,0);
                       break;
               case 1: inputstr(LEFTSIDE+39,LineNum,30,"",p->DskPath,p->DskPath,ALLFILE,INPUT_CAPS,0); stripright(p->DskPath,' ');
                       if (strlen(p->DskPath) < 30)
                         addbackslash(p->DskPath,sizeof(p->DskPath));
                       break;
               case 2: inputnum(LEFTSIDE+71,LineNum, 1,"",&p->SortType,vBYTE,0);
                       break;
             }
             break;
    case 1:  switch (Column) {
               case 0: inputstr(LEFTSIDE+7 ,LineNum,30,"",p->DirPath,p->DirPath,ALLFILE,INPUT_CAPS,0);
                       break;
               case 1: inputstr(LEFTSIDE+39,LineNum,35,"",p->DirDesc,p->DirDesc,ALLCHAR,INPUT_CLEAR,0); stripright(p->DskPath,' ');
                       break;
             }
             break;
  }
}


/********************************************************************
*
*  Function: editdirlist()
*
*  Desc    : Reads the table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editdirlist(char *Name) {
  editdirclass DirList("DIR.LST Editor",sizeof(DirListType),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,2,1);

  Group = 0;

  if (DirList.load(Name) == -1)
    return;

  DirList.NumColumns = (Group == 0 ? 2 : 1);
  DirList.addexitkey(60,F2,"F2=Rotate");    // scancode = 60, key = F2
  DirList.edit();
}
