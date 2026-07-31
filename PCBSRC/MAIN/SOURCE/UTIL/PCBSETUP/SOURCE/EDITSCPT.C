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

enum { F2=NEXTKEY, F3 };

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

typedef struct {
  char QstFile[30];
  char AnsFile[30];
} scriptlisttype;

class editscriptclass : public editclass {
  public:
    editscriptclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};


void pascal editscriptclass::loadrecords(DOSFILE *File) {
  ScriptType Rec;
  scriptlisttype OnDisk;

  memset(&Rec,0,sizeof(Rec));
  while (dosfread(&OnDisk,sizeof(OnDisk),File) == sizeof(OnDisk)) {
    memcpy(Rec.AskPath,OnDisk.QstFile,sizeof(OnDisk.QstFile));
    memcpy(Rec.AnsPath,OnDisk.AnsFile,sizeof(OnDisk.AnsFile));
    removespaces(Rec.AskPath,sizeof(Rec.AskPath));
    removespaces(Rec.AnsPath,sizeof(Rec.AnsPath));
    addrecord(&Rec);
  }
}


void pascal editscriptclass::saverecords(DOSFILE *File) {
  long            X;
  ScriptType     *p;
  scriptlisttype  OnDisk;

  for (X = 1; X <= TotalRecs; X++) {
    p = (ScriptType *) getrecord(X);
    padstr(p->AskPath,' ',sizeof(OnDisk.QstFile));
    padstr(p->AnsPath,' ',sizeof(OnDisk.AnsFile));
    memcpy(OnDisk.QstFile,p->AskPath,sizeof(OnDisk.QstFile));
    memcpy(OnDisk.AnsFile,p->AnsPath,sizeof(OnDisk.QstFile));
    if (dosfwrite(&OnDisk,sizeof(OnDisk),File) == -1)
      break;
  }
}


void pascal editscriptclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  ScriptType *p = (ScriptType *) Rec;
  char        Temp[10];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,Temp       ,Colors[ANSWER]);
  fastprint(LEFTSIDE+8 ,LineNum,p->AskPath,Colors[ANSWER]);
  fastprint(LEFTSIDE+40,LineNum,p->AnsPath,Colors[ANSWER]);
}



void pascal editscriptclass::showheaders(void) {
  fastprint(LEFTSIDE+8,TOPLINE-2,"SCRIPT Questionnaire Files      SCRIPT Answer Files",Colors[DISPLAY]);
  fastprint(LEFTSIDE+8,TOPLINE-1,"ออออออออออออออออออออออออออ      อออออออออออออออออออ",Colors[DISPLAY]);
//fastcenter(21," Press F2 to edit the normal file or F3 to edit the graphics file ",Colors[DESC]);
//fastprint(1,Scrn_BottomRow-1,"  ESC=Exit   Alt-R=Repeat a Line   Alt-I=Insert a Line   Alt-D=Delete a Line  ",Colors[DESC]);
}


void pascal editscriptclass::handlekeys(void *Rec) {
  ScriptType *p = (ScriptType *) Rec;

  switch (KeyFlags) {
    case F2 : editor((Column ? p->AnsPath : p->AskPath),FALSE);  break;
    case F3 : editor((Column ? p->AnsPath : p->AskPath),TRUE);   break;
    default : editclass::handlekeys(Rec); break;
  }
}


void pascal editscriptclass::editrecord(void *Rec) {
  ScriptType *p = (ScriptType *) Rec;

  switch (Column) {
    case 0: inputstr(LEFTSIDE+8 ,LineNum,30,"",p->AskPath,p->AskPath,ALLFILE,INPUT_CAPS,SCRIPTFILE+0);
            stripright(p->AskPath,' ');
            break;
    case 1: inputstr(LEFTSIDE+40,LineNum,30,"",p->AnsPath,p->AnsPath,ALLFILE,INPUT_CAPS,SCRIPTFILE+1);
            stripright(p->AnsPath,' ');
            break;
  }
}


/********************************************************************
*
*  Function: editscript()
*
*  Desc    : Reads the table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editscript(char *Name) {
  editscriptclass Script("SCRIPT.LST Editor",sizeof(ScriptType),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,2);

  if (Script.load(Name) == -1)
    return;

  Script.addexitkey(60,F2,"F2=Text");
  Script.addexitkey(61,F3,"F3=Graphics");
  Script.edit();
}
