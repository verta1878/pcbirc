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

static char *Header;
static int   Offset;

typedef struct {
  char Path[64];
} pathtype;


class editpathclass : public editclass {
  public:
    editpathclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editpathclass::loadrecords(DOSFILE *File) {
  pathtype Rec;
  char     Buf[256];

  while (dosfgets(Buf,sizeof(Buf),File) != -1) {
    newrecord(&Rec);
    maxstrcpy(Rec.Path,Buf,sizeof(Rec.Path));
    addrecord(&Rec);
  }
}


void pascal editpathclass::saverecords(DOSFILE *File) {
  long      X;
  pathtype *p;
  char      Temp[100];

  for (X = 1; X <= TotalRecs; X++) {
    p = (pathtype *) getrecord(X);
    stripright(p->Path,' ');
    sprintf(Temp,"%s\r\n",p->Path);
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


void pascal editpathclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  pathtype *p = (pathtype *) Rec;
  char      Temp[5];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,Temp,Colors[ANSWER]);
  fastprint(LEFTSIDE+6,LineNum,p->Path,Colors[ANSWER]);
}


void pascal editpathclass::showheaders(void) {
  char Str[80];

  memset(Str,'Í',63);
  Str[63] = 0;

  fastprint(LEFTSIDE+6,TOPLINE-2,Header,Colors[DISPLAY]);
  fastprint(LEFTSIDE+6,TOPLINE-1,Str,Colors[DISPLAY]);
}



void pascal editpathclass::editrecord(void *Rec) {
  pathtype *p = (pathtype *) Rec;

  inputstr(LEFTSIDE+6,LineNum,sizeof(p->Path)-1,"",p->Path,p->Path,ALLFILE,INPUT_CAPS,DLPATHFILE+Offset);
  stripright(p->Path,' ');
  if (p->Path[0] != 0 && p->Path[0] != '%')
    addbackslash(p->Path,sizeof(p->Path)-1);
}


/********************************************************************
*
*  Function: editlist()
*
*  Desc    : Reads the table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editpath(char *Name, int FileNum) {
  static char  *FldTitle[2] = { "Enter Paths and IDX Files to scan for Downloads",
                                "Enter Paths and IDX Files to check against New Uploads" };
  editpathclass  Path("Download Path Editor",sizeof(pathtype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,0,0);

  Header = FldTitle[FileNum];
  Offset = FileNum;

  if (Path.load(Name) == -1)
    return;

  Path.edit();
}
