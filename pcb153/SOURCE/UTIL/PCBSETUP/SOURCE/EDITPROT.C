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
#define TOPLINE      5
#define BOTTOM       2


class editprotclass : public editclass {
  public:
    editprotclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};

typedef struct {
  char Letter;
  char Shell;
  int  Size;
  bool PortOpen;
  bool LockLines;
  char Desc[50];
  bool MNP;
} prottype;



void pascal editprotclass::loadrecords(DOSFILE *File) {
  char     *p;
  prottype  Rec;
  char      Buf[256];

  while (dosfgets(Buf,sizeof(Buf),File) != -1) {
    newrecord(&Rec);
    p = parse(Buf);
    Rec.Letter = *p;
    p = parse(NULL);
    Rec.Shell = *p;
    Rec.Size = atoi(parse(NULL));
    maxstrcpy(Rec.Desc,parse(NULL),sizeof(Rec.Desc));
    p = parse(NULL);
    if (Rec.Shell == 'I')
      Rec.MNP = (Rec.Letter == 'F' || Rec.Letter == 'G');
    else
      Rec.MNP = (*p == 'Y');
    p = parse(NULL);
    Rec.PortOpen = (*p == 'Y');
    p = parse(NULL);
    Rec.LockLines = (*p == 'Y');
    addrecord(&Rec);
  }
}


void pascal editprotclass::saverecords(DOSFILE *File) {
  long      X;
  prottype *p;
  char      Temp[100];

  for (X = 1; X <= TotalRecs; X++) {
    p = (prottype *) getrecord(X);
    stripright(p->Desc,' ');
    sprintf(Temp,"%c,%c,%d,%s,%c,%c,%c\r\n",
            p->Letter,
            p->Shell,
            p->Size,
            p->Desc,
            (p->MNP ? 'Y' : 'N'),
            (p->PortOpen ? 'Y' : 'N'),
            (p->LockLines ? 'Y' : 'N'));
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


#pragma argsused
void pascal editprotclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  prottype *p = (prottype *) Rec;
  char      Temp[27];

  Temp[1] = 0;
  if (p->Letter != 0) {
    Temp[0] = p->Letter;
    fastprint(3,LineNum,Temp,Colors[ANSWER]);
  }
  if (p->Shell != 0) {
    Temp[0] = p->Shell;
    fastprint(7,LineNum,Temp,Colors[ANSWER]);
  }
  sprintf(Temp,"%4d",p->Size);
  fastprint(11,LineNum,Temp,Colors[ANSWER]);
  Temp[0] = (p->MNP ? 'Y' : 'N');
  Temp[1] = 0;
  fastprint(17,LineNum,Temp,Colors[ANSWER]);
  Temp[0] = (p->PortOpen ? 'Y' : 'N');
  fastprint(21,LineNum,Temp,Colors[ANSWER]);
  Temp[0] = (p->LockLines ? 'Y' : 'N');
  fastprint(27,LineNum,Temp,Colors[ANSWER]);
  fastprint(31,LineNum,p->Desc,Colors[ANSWER]);
}


void pascal editprotclass::showheaders(void) {
  fastprint(20, TOPLINE-3,                  "Port Lock"                                                 ,Colors[DISPLAY]);
  fastprint( 2, TOPLINE-2,"Use Type Size MNP Open Lines             Protocol Description"               ,Colors[DISPLAY]);
  fastprint( 2, TOPLINE-1,"อออ ออออ ออออ อออ ออออ อออออ อออออออออออออออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
}


void pascal editprotclass::editrecord(void *Rec) {
  static char PROTTYPE[5] = { 4, 'D','I','S', 'B' };
  prottype *p = (prottype *) Rec;
  char      Temp[10];

  switch (Column) {
    case 0: Temp[0] = p->Letter; Temp[1] = 0;
            inputstr(3,LineNum,1,"",Temp,Temp,ALLPROTO,INPUT_CAPS,PROTOCOL);
            p->Letter = Temp[0];
            break;
    case 1: Temp[0] = p->Shell; Temp[1] = 0;
            inputstr(7,LineNum,1,"",Temp,Temp,PROTTYPE,INPUT_CAPS,PROTOCOL+1);
            p->Shell = Temp[0];
            break;
    case 2: inputnum(11,LineNum, 4,"",&p->Size,vINT,PROTOCOL+2);
            break;
    case 3: inputnum(17,LineNum, 1,"",&p->MNP,vBOOL,PROTOCOL+3);
            break;
    case 4: inputnum(21,LineNum, 1,"",&p->PortOpen,vBOOL,PROTOCOL+4);
            break;
    case 5: inputnum(27,LineNum, 1,"",&p->LockLines,vBOOL,PROTOCOL+5);
            break;
    case 6: inputstr(31,LineNum,47,"",p->Desc,p->Desc,ALLTEXT,INPUT_CLEAR,PROTOCOL+6);
            break;
  }
}




/********************************************************************
*
*  Function: editprot()
*
*  Desc    : Allows the user to edit the prot table.
*/

void pascal editprot(void) {
  editprotclass  Prot("Transfer Protocols",sizeof(prottype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,6,0);

  if (Prot.load(PcbData.TrnFile) == -1)
    return;

  Prot.edit();
}
