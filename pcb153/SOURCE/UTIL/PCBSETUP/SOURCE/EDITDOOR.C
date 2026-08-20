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
#define TOPLINE      6
#define BOTTOM       2

static char YN123[] = {5, 'Y', 'N', '1','2','3'};
static char ALLDOOR[]  = {13, ' ','!','$','&','(',')','-', 0,'0','9', 0,'@','Z' };


class editdoorclass : public editclass {
  public:
    editdoorclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec);
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};


void pascal editdoorclass::newrecord(void *Rec) {
  DoorType *p = (DoorType *) Rec;

  editclass::newrecord(Rec);
  p->Shell = 'N';
}


void pascal editdoorclass::loadrecords(DOSFILE *File) {
  char     Temp[15];
  DoorType Rec;
  char     Buf[256];

  while (dosfgets(Buf,256,File) != -1) {
    newrecord(&Rec);
    maxstrcpy(Rec.Name,parse(Buf),sizeof(Rec.Name));
    maxstrcpy(Rec.Pwrd,parse(NULL),sizeof(Rec.Pwrd));
    Rec.Sec = atoi(parse(NULL));
    switch (atoi(parse(NULL))) {
      case -1: Rec.MakeUserSys = 'Y'; break;
      case -2: Rec.MakeUserSys = '1'; break;
      case -3: Rec.MakeUserSys = '2'; break;
      case -4: Rec.MakeUserSys = '3'; break;
      default: Rec.MakeUserSys = 'N'; break;
    }
    Rec.MakeDoorSys = (atoi(parse(NULL)) == -1);
    maxstrcpy(Rec.Path,parse(NULL),sizeof(Rec.Path));
    Rec.Login = (atoi(parse(NULL)) == -1);
    maxstrcpy(Temp,parse(NULL),sizeof(Temp));
    Rec.Shell = (Temp[0] != 'Y' && Temp[0] != 'S' && Temp[0] != 'F' ? 'N' : Temp[0]);
    maxstrcpy(Temp,parse(NULL),sizeof(Temp));
    Rec.ChargePerUse = atof(Temp);
    maxstrcpy(Temp,parse(NULL),sizeof(Temp));
    Rec.ChargePerMin = atof(Temp);
    Rec.Os2 = (atoi(parse(NULL)) == -1);
    addrecord(&Rec);
  }
}


void pascal editdoorclass::saverecords(DOSFILE *File) {
  int       MakeUserSys;
  long      X;
  DoorType *p;
  char      Temp1[20];
  char      Temp2[20];
  char      Temp[128];

  for (X = 1; X <= TotalRecs; X++) {
    p = (DoorType *) getrecord(X);
    stripall(p->Name,' ');
    stripall(p->Pwrd,' ');
    switch (p->MakeUserSys) {
      case 'Y': MakeUserSys = -1; break;
      case '1': MakeUserSys = -2; break;
      case '2': MakeUserSys = -3; break;
      case '3': MakeUserSys = -4; break;
      default : MakeUserSys =  0; break;
    }
    sprintf(Temp1,"%14.4lf",p->ChargePerUse); stripleft(Temp1,' ');
    sprintf(Temp2,"%14.4lf",p->ChargePerMin); stripleft(Temp2,' ');
    sprintf(Temp,"%s,%s,%d,%d,%d,%s,%d,%c,%s,%s,%d\r\n",
            p->Name,
            p->Pwrd,
            p->Sec,
            MakeUserSys,
            (p->MakeDoorSys ? -1 : 0),
            p->Path,
            (p->Login ? -1 : 0),
            p->Shell,
            Temp1,
            Temp2,
            (p->Os2 ? -1 : 0));
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


void pascal editdoorclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  DoorType *p = (DoorType *) Rec;
  char     Temp1[80];
  char     Temp2[80];
  char     Temp[80];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE,LineNum,Temp,Colors[ANSWER]);

  fastprint(LEFTSIDE+5 ,LineNum,p->Name,Colors[ANSWER]);

  sprintf(Temp,"%-6.6s",p->Pwrd);
  fastprint(LEFTSIDE+14,LineNum,Temp   ,Colors[ANSWER]);

  sprintf(Temp,"%3d",p->Sec);
  fastprint(LEFTSIDE+21,LineNum,Temp   ,Colors[ANSWER]);

  Temp[1] = 0;
  Temp[0] = (p->Login ? 'Y' : 'N');
  fastprint(LEFTSIDE+27,LineNum,Temp   ,Colors[ANSWER]);

  if (p->MakeUserSys == 0)
    p->MakeUserSys = 'N';
  Temp[0] = p->MakeUserSys;
  fastprint(LEFTSIDE+32,LineNum,Temp   ,Colors[ANSWER]);

  Temp[0] = (p->MakeDoorSys ? 'Y' : 'N');
  fastprint(LEFTSIDE+37,LineNum,Temp   ,Colors[ANSWER]);

  Temp[0] = p->Shell;
  fastprint(LEFTSIDE+43,LineNum,Temp   ,Colors[ANSWER]);

  Temp[0] = (p->Os2 ? 'Y' : 'N');
  fastprint(LEFTSIDE+48,LineNum,Temp   ,Colors[ANSWER]);

  dcomma(Temp1,p->ChargePerMin);
  dcomma(Temp2,p->ChargePerUse);

  sprintf(Temp,"%6.6s %6.6s %-12.12s",Temp1,Temp2,p->Path);
  fastprint(LEFTSIDE+51,LineNum,Temp,Colors[ANSWER]);
}



void pascal editdoorclass::showheaders(void) {
//fastprint(LEFTSIDE+33,TOPLINE-3,                              "USER DOOR         Charges Per      Path to"  ,Colors[DISPLAY]);
//fastprint(LEFTSIDE+ 4,TOPLINE-2, "Filename  Password Sec Login SYS  SYS  Shell Minute    Use    DOOR Files" ,Colors[DISPLAY]);
//fastprint(LEFTSIDE+ 3,TOPLINE-1,"ออออออออออ ออออออออ อออ อออออ ออออ ออออ อออออ อออออออออออออออ ออออออออออออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+31,TOPLINE-3,                            "USER DOOR            Charges Per     Path to"  ,Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 4,TOPLINE-2, "Filename   Pwrd  Sec Login SYS  SYS  Shell Os2 Minute    Use  DOOR Files" ,Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 3,TOPLINE-1,"ออออออออออ ออออออ อออ อออออ ออออ ออออ อออออ อออ อออออออออออออ ออออออออออออ",Colors[DISPLAY]);
}



void pascal editdoorclass::handlekeys(void *Rec) {
  DoorType *p = (DoorType *) Rec;
  char     FileName[66];

  switch (KeyFlags) {
    case F2 : strcpy(FileName,p->Path);
              strcat(FileName,p->Name);
              editor(FileName,FALSE);
              break;
    default : editclass::handlekeys(Rec); break;
  }
}



void pascal editdoorclass::editrecord(void *Rec) {
  DoorType *p = (DoorType *) Rec;
  char      Temp[10];

  switch (Column) {
    case  0: inputstr(LEFTSIDE+5,LineNum,8,"",p->Name,p->Name,ALLDOOR,INPUT_CAPS|INPUT_CLEAR,DREDIT+0);
             stripright(p->Name,' ');
             break;
    case  1: inputextend(LEFTSIDE+14,LineNum,6,12,p->Pwrd,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,DREDIT+1);
             stripright(p->Pwrd,' ');
             break;
    case  2: inputnum(LEFTSIDE+21,LineNum, 3,"",&p->Sec,vINT,DREDIT+2);
             break;
    case  3: inputnum(LEFTSIDE+27,LineNum, 1,"",&p->Login,vBOOL,DREDIT+3);
             break;
    case  4: Temp[0] = p->MakeUserSys; Temp[1] = 0;
             inputstr(LEFTSIDE+32,LineNum,1,"",Temp,Temp,YN123,INPUT_CAPS,DREDIT+4);
             p->MakeUserSys = Temp[0];
             break;
    case  5: inputnum(LEFTSIDE+37,LineNum, 1,"",&p->MakeDoorSys,vBOOL,DREDIT+5);
             break;
    case  6: Temp[0] = p->Shell; Temp[1] = 0;
             inputstr(LEFTSIDE+43,LineNum,1,"",Temp,Temp,YNSF,INPUT_CAPS,DREDIT+6);
             p->Shell = Temp[0];
             break;
    case  7: inputnum(LEFTSIDE+48,LineNum,1,"",&p->Os2,vBOOL,DREDIT+7);
             break;
    case  8: inputnum(LEFTSIDE+51,LineNum,6,"",&p->ChargePerMin,vFLOATD,DREDIT+8);
             break;
    case  9: inputnum(LEFTSIDE+58,LineNum,6,"",&p->ChargePerUse,vFLOATD,DREDIT+9);
             break;
    case 10: inputextend(LEFTSIDE+65,LineNum,12,sizeof(p->Path)-1,p->Path,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,DREDIT+10);
             stripright(p->Path,' ');
             addbackslash(p->Path,sizeof(p->Path)-1);
             break;
  }
}



/********************************************************************
*
*  Function: editdoor()
*
*  Desc    : Reads the door table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editdoor(char *Name) {
  editdoorclass Door("DOORS File Editor",sizeof(DoorType),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,10,1);

  if (Door.load(Name) == -1)
    return;

  Door.addexitkey(60,F2,"F2=Edit");
  Door.edit();
}
