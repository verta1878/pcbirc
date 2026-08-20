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

enum { F2 = NEXTKEY };

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      6
#define BOTTOM       2


class editcmdclass : public editclass {
  public:
    editcmdclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};



void pascal editcmdclass::loadrecords(DOSFILE *File) {
  cmdtype Rec;

  while (dosfread(&Rec,sizeof(cmdtype),File) == sizeof(cmdtype)) {
    removespaces(Rec.Name,sizeof(Rec.Name));
    removespaces(Rec.File,sizeof(Rec.File));
    addrecord(&Rec);
  }
}



void pascal editcmdclass::saverecords(DOSFILE *File) {
  long     X;
  cmdtype *p;

  for (X = 1; X <= TotalRecs; X++) {
    p = (cmdtype *) getrecord(X);
    if (dosfwrite(p,sizeof(cmdtype),File) == -1)
      break;
  }
}



void pascal editcmdclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  cmdtype *p = (cmdtype *) Rec;
  char     Temp1[20];
  char     Temp2[20];
  char     Temp[20];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,Temp   ,Colors[ANSWER]);
  fastprint(LEFTSIDE+ 6,LineNum,p->Name,Colors[ANSWER]);

  dcomma(Temp1,p->ChargePerMin);
  dcomma(Temp2,p->ChargePerUse);

  sprintf(Temp,"%3d %8.8s %8.8s %-33.33s",p->SecLevel,Temp1,Temp2,p->File);
  fastprint(LEFTSIDE+21,LineNum,Temp,Colors[ANSWER]);
}



void pascal editcmdclass::showheaders(void) {
  fastprint(LEFTSIDE+27,TOPLINE-3,                     "Charges Per     PPE/MNU File Specification -or-"  ,Colors[DISPLAY]);
  fastprint(LEFTSIDE+10,TOPLINE-2,    "Command    Sec  Minute    Use    Keystroke Substitution           ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+6 ,TOPLINE-1,"ออออออออออออออ อออ อออออออออออออออออ อออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
}



void pascal editcmdclass::handlekeys(void *Rec) {
  cmdtype *p = (cmdtype *) Rec;

  switch (KeyFlags) {
    case F2 : if (p->Name[0] == 0 && p->File[0] == '%')
                editcmd(&p->File[1]);
              KeyFlags = NOTHING;
              break;
    default : editclass::handlekeys(Rec); break;
  }
}



void pascal editcmdclass::editrecord(void *Rec) {
  cmdtype *p = (cmdtype *) Rec;

  switch (Column) {
    case 0: inputstr(LEFTSIDE+6,LineNum,sizeof(p->Name)-1,"",p->Name,p->Name,ALLTEXT,INPUT_CAPS,CMDSFILE+0);
            stripright(p->Name,' ');
            break;
    case 1: inputnum(LEFTSIDE+21,LineNum,3,"",&p->SecLevel,vBYTE,CMDSFILE+1);
            break;
    case 2: inputnum(LEFTSIDE+25,LineNum,8,"",&p->ChargePerMin,vFLOAT,CMDSFILE+2);
            break;
    case 3: inputnum(LEFTSIDE+34,LineNum,8,"",&p->ChargePerUse,vFLOAT,CMDSFILE+3);
            break;
    case 4: inputextend(LEFTSIDE+43,LineNum,33,sizeof(p->File)-1,p->File,ALLTEXT,INPUT_CAPS,CMDSFILE+4);
            stripright(p->File,' ');
            break;
  }
}



/********************************************************************
*
*  Function: editcmd()
*
*  Desc    : Reads the table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editcmd(char *Name) {
  editcmdclass Cmd("CMD.LST Editor",sizeof(cmdtype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,4,1);

  if (Cmd.load(Name) == -1)
    return;

  Cmd.addexitkey(60,F2,"F2=Edit");
  Cmd.edit();
}
