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


#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
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


enum { F2=NEXTKEY, F3 };

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2
#define RECSIZE     sizeof(ItemListType) - 1  // used because in-memory we hold a NULL terminator


class editbltclass : public editclass {
  public:
    editbltclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};




void pascal editbltclass::loadrecords(DOSFILE *File) {
  ItemListType Rec;

  while (dosfread(&Rec,RECSIZE,File) == RECSIZE) {
    Rec.NamePath[RECSIZE] = 0;
    removespaces(Rec.NamePath,sizeof(Rec.NamePath));
    addrecord(&Rec);
  }
}



void pascal editbltclass::saverecords(DOSFILE *File) {
  long          X;
  ItemListType *p;

  for (X = 1; X <= TotalRecs; X++) {
    p = (ItemListType *) getrecord(X);
    if (dosfwrite(p,RECSIZE,File) == -1)
      break;
  }
}



void pascal editbltclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  ItemListType *p = (ItemListType *) Rec;
  char          Temp[10];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,Temp       ,Colors[ANSWER]);
  fastprint(LEFTSIDE+7,LineNum,p->NamePath,Colors[ANSWER]);
}



void pascal editbltclass::showheaders(void) {
  fastprint(LEFTSIDE+1,TOPLINE-2,Title,Colors[DISPLAY]);
  fastprint(LEFTSIDE+1,TOPLINE-1,"様様様様様様様様様様様様様様様様様様",Colors[DISPLAY]);
//  fastcenter(Scrn_BottomRow-3," Press F2 to edit BLTxx  or  F3 to edit BLTxxG ",Colors[DESC]);
//  fastprint(1,Scrn_BottomRow-1,"  ESC=Exit   Alt-R=Repeat a Line   Alt-I=Insert a Line   Alt-D=Delete a Line  ",Colors[DESC]);
}



void pascal editbltclass::handlekeys(void *Rec) {
  ItemListType *p = (ItemListType *) Rec;

  switch (KeyFlags) {
    case F2 : editor(p->NamePath,FALSE);  break;
    case F3 : editor(p->NamePath,TRUE);   break;
    default : editclass::handlekeys(Rec); break;
  }
}



void pascal editbltclass::editrecord(void *Rec) {
  ItemListType *p = (ItemListType *) Rec;

  inputstr(LEFTSIDE+7,LineNum,RECSIZE,"",p->NamePath,p->NamePath,ALLFILE,INPUT_CAPS,BLTFILE);
  stripright(p->NamePath,' ');
}



/********************************************************************
*
*  Function: editblt()
*
*  Desc    : Reads the table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editblt(char *Name, char *FldTitle) {
  editbltclass Blt(FldTitle,sizeof(ItemListType),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,0,2);

  if (Blt.load(Name) == -1)
    return;

  Blt.addexitkey(60,F2,"F2=Text");
  Blt.addexitkey(61,F3,"F3=Graphics");
  Blt.edit();
}
