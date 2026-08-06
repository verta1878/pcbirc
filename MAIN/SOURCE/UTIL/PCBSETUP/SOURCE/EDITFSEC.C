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


#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      6
#define BOTTOM       2


class editfsecclass : public editclass {
  public:
    editfsecclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);

    int  Which;
};


typedef struct {
  char Path[31];
  char Name[13];
  int  Sec;
  char Pwrd[13];
  long Credits;
} fsectype;



void pascal editfsecclass::loadrecords(DOSFILE *File) {
  char     Str[15];
  fsectype Rec;
  char     Buf[256];

  while (dosfgets(Buf,sizeof(Buf),File) != -1) {
    newrecord(&Rec);
    maxstrcpy(Rec.Path,parse(Buf),sizeof(Rec.Path));
    maxstrcpy(Rec.Name,parse(NULL),sizeof(Rec.Name));
    Rec.Sec = atoi(parse(NULL));
    maxstrcpy(Rec.Pwrd,parse(NULL),sizeof(Rec.Pwrd));
    maxstrcpy(Str,parse(NULL),sizeof(Str));
    Rec.Credits = (Str[0] == 0 ? 1 : atol(Str));
    addrecord(&Rec);
  }
}


void pascal editfsecclass::saverecords(DOSFILE *File) {
  long      X;
  fsectype *p;
  char      Temp[100];

  for (X = 1; X <= TotalRecs; X++) {
    p = (fsectype *) getrecord(X);
    stripall(p->Path,' ');
    if (strlen(p->Path) != 2 || p->Path[1] != ':')
      addbackslash(p->Path,sizeof(p->Path));
    stripall(p->Name,' ');
    stripall(p->Pwrd,' ');
    sprintf(Temp,"%s,%s,%d,%s,%ld\r\n",p->Path,p->Name,p->Sec,p->Pwrd,p->Credits);
    if (dosfputs(Temp,File) == -1)
      break;
  }
}



void pascal editfsecclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  fsectype *p = (fsectype *) Rec;
  char      Temp[21];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,Temp   ,Colors[ANSWER]);

  fastprint(LEFTSIDE+ 6,LineNum,p->Path,Colors[ANSWER]);
  fastprint(LEFTSIDE+37,LineNum,p->Name,Colors[ANSWER]);

  sprintf(Temp,"%3d",p->Sec);
  fastprint(LEFTSIDE+50,LineNum,Temp,Colors[ANSWER]);

  sprintf(Temp,"%8ld",p->Credits);
  fastprint(LEFTSIDE+54,LineNum,Temp,Colors[ANSWER]);
  fastprint(LEFTSIDE+63,LineNum,p->Pwrd,Colors[ANSWER]);
}


void pascal editfsecclass::showheaders(void) {
  static char *DbCr[2] = {"Debit","Credit"};

  fastprint(LEFTSIDE+55,TOPLINE-3,DbCr[Which],Colors[DISPLAY]);
  fastprint(LEFTSIDE+15,TOPLINE-2,        "Drive \\ Path            File Name  Sec Multiply   Password"  ,Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 6,TOPLINE-1,"ออออออออออออออออออออออออออออออ ออออออออออออ อออ ออออออออ ออออออออออออ",Colors[DISPLAY]);
}



void pascal editfsecclass::editrecord(void *Rec) {
  fsectype *p = (fsectype *) Rec;

  switch (Column) {
    case 0: inputstr(LEFTSIDE+6,LineNum,30,"",p->Path,p->Path,ALLFILE,INPUT_CAPS,FSECEDIT);
            stripright(p->Path,' ');
            if (strlen(p->Path) != 2 || p->Path[1] != ':')
              addbackslash(p->Path,sizeof(p->Path));
            break;
    case 1: inputstr(LEFTSIDE+37,LineNum,12,"",p->Name,p->Name,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,FSECEDIT+1);
            stripright(p->Name,' ');
            break;
    case 2: inputnum(LEFTSIDE+50,LineNum, 3,"",&p->Sec,vINT,FSECEDIT+2);
            break;
    case 3: inputnum(LEFTSIDE+54,LineNum, 8,"",&p->Credits,vLONG,FSECEDIT+3);
            break;
    case 4: inputstr(LEFTSIDE+63,LineNum,12,"",p->Pwrd,p->Pwrd,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,FSECEDIT+4);
            stripright(p->Pwrd,' ');
            break;
  }
}


/********************************************************************
*
*  Function: editfsec()
*
*  Desc    : Allows the user to edit the FSEC table and save it when done.
*
*/

void pascal editfsec(int Select) {
  char          *Name;
  editfsecclass  Fsec(NULL,sizeof(fsectype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,4,0);

  switch (Select) {
    case 0: Fsec.Title = "Download Security File (FSEC)";
            Name = PcbData.FscFile;
            break;
    case 1: Fsec.Title = "Upload Security File (UPSEC)";
            Name = PcbData.UscFile;
            break;
  }

  Fsec.Which = Select;

  if (Fsec.load(Name) == -1)
    return;

  Fsec.edit();
}
