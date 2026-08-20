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

enum {F2 = NEXTKEY};


#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      6
#define BOTTOM       2

const  int ColumnSetup[2] = { 9, 6 };
const  int NextGroup[2]   = { 1, 0 };
static int SaveColumns[2];
static int Group;

class editpwrdclass : public editclass {
  public:
    editpwrdclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec);
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};


typedef struct {
  char Pwrd[13];
  char Sec;
  int  Time;
  long Bytes;
  long BaseBaud;
  int  BatchLimit;
  long FileRatio;
  long ByteRatio;
  long TotalFileLimit;
  long TotalKByteLimit;
  bool EnforceTimeLimit;
  bool AllowAlias;
  bool ForceReadMail;
  bool MultipleLogins;
  bool VerifyCaller;       // not being used!
  int  FileRatioCredits;
  int  KByteRatioCredits;
  char EnableAccounting;
} pwrdtype;


static bool near pascal yes(char *Str, bool Default) {
  if (Str == NULL || *Str == 0)
    return(Default);
  if (*Str == 'Y')
    return(TRUE);
  return(FALSE);
}


static char near pascal yesno(bool Condition) {
  return(Condition ? 'Y' : 'N');
}


static char near pascal yesnotrack(char *Str) {
  if (Str != NULL && (*Str == 'Y' || *Str == 'T'))
    return(*Str);
  return('N');
}


void pascal editpwrdclass::loadrecords(DOSFILE *File) {
  pwrdtype Rec;
  char     Buf[256];

  while (dosfgets(Buf,sizeof(Buf),File) != -1) {
    newrecord(&Rec);
    maxstrcpy(Rec.Pwrd,parse(Buf),sizeof(Rec.Pwrd));
    Rec.Sec               = atoi(parse(NULL));
    Rec.Time              = atoi(parse(NULL));
    Rec.Bytes             = atol(parse(NULL));
    Rec.BaseBaud          = atol(parse(NULL));
    Rec.BatchLimit        = atoi(parse(NULL));
    Rec.FileRatio         = atol(parse(NULL));
    Rec.ByteRatio         = atol(parse(NULL));
    Rec.TotalFileLimit    = atol(parse(NULL));
    Rec.TotalKByteLimit   = atol(parse(NULL));
    Rec.EnforceTimeLimit  = yes(parse(NULL),TRUE);
    Rec.AllowAlias        = yes(parse(NULL),FALSE);
    Rec.ForceReadMail     = yes(parse(NULL),FALSE);
    Rec.MultipleLogins    = yes(parse(NULL),FALSE);
    Rec.VerifyCaller      = yes(parse(NULL),FALSE);
    Rec.FileRatioCredits  = atoi(parse(NULL));
    Rec.KByteRatioCredits = atoi(parse(NULL));
    Rec.EnableAccounting  = yesnotrack(parse(NULL));
    addrecord(&Rec);
  }
}



/********************************************************************
*
*  Function: compsecs()
*
*  Desc    : Compares two elements of PWRD (used to sort the table)
*
*  Calls   : NONE
*
*  Returns :  1 if "Num" of Rec1 is greater than "Num" of Rec2
*             0 if they are equal
*            -1 if "Num" of Rec2 is greater than "Num" of Rec1
*/

/*
static int comppwrd(const void *A, const void *B) {
  const pwrdtype *Rec1 = A;
  const pwrdtype *Rec2 = B;

  if (memcmp(Rec1,Rec2,sizeof(pwrdtype)) == 0)
    return(0);
  if (Rec1->Sec == 0 && Rec1->Time == 0)
    return(-1);
  if (Rec2->Sec == 0 && Rec2->Time == 0)
    return(1);
  if (Rec1->Sec > Rec2->Sec)
    return(1);
  if (Rec1->Sec < Rec2->Sec)
    return(-1);
  return(-strcmp(Rec1->Pwrd,Rec2->Pwrd));
}
*/


void pascal editpwrdclass::saverecords(DOSFILE *File) {
  long      X;
  pwrdtype *p;
  char      Temp[256];

//I'll decide later if we still need to sort these or not
//qsort(p,MAXPWRDS,sizeof(pwrdtype),comppwrd);

  for (X = 1; X <= TotalRecs; X++) {
    p = (pwrdtype *) getrecord(X);
    stripright(p->Pwrd,' ');
    sprintf(Temp,"%s,%d,%d,%ld,%ld,%d,%ld,%ld,%ld,%ld,%c,%c,%c,%c,%c,%d,%d,%c\r\n",
            p->Pwrd,p->Sec,p->Time,p->Bytes,p->BaseBaud,p->BatchLimit,
            p->FileRatio,p->ByteRatio,p->TotalFileLimit,p->TotalKByteLimit,
            yesno(p->EnforceTimeLimit),
            yesno(p->AllowAlias),
            yesno(p->ForceReadMail),
            yesno(p->MultipleLogins),
            yesno(p->VerifyCaller),
            p->FileRatioCredits,
            p->KByteRatioCredits,
            p->EnableAccounting);
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


void pascal editpwrdclass::newrecord(void *Rec) {
  pwrdtype *p = (pwrdtype *) Rec;

  editclass::newrecord(Rec);
  p->EnforceTimeLimit = TRUE;
  p->EnableAccounting = 'N';
}


#pragma argsused
void pascal editpwrdclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  pwrdtype *p = (pwrdtype *) Rec;
  char      FileRatio[10];
  char      ByteRatio[10];
  char      Temp[80];

  fastprint( 3,LineNum,p->Pwrd,Colors[ANSWER]);
  sprintf(Temp,"%3d",p->Sec);
  fastprint(17,LineNum,Temp,Colors[ANSWER]);

  switch (Group) {
    case 0: decimal(FileRatio,p->FileRatio);
            decimal(ByteRatio,p->ByteRatio);
            sprintf(Temp,"%4d   %5ld  %6ld %5d %5.5s %5.5s %7ld %7ld",
                    p->Time,p->Bytes,p->BaseBaud,p->BatchLimit,FileRatio,
                    ByteRatio,p->TotalFileLimit,p->TotalKByteLimit);
            fastprint(22,LineNum,Temp,Colors[ANSWER]);
            break;
    case 1: sprintf(Temp,"%6u %6u     %c        %c       %c       %c      %c",
                    p->FileRatioCredits,
                    p->KByteRatioCredits,
                    yesno(p->EnforceTimeLimit),
                    yesno(p->AllowAlias),
                    yesno(p->ForceReadMail),
                    yesno(p->MultipleLogins),
                    p->EnableAccounting);
            fastprint(22,LineNum,Temp,Colors[ANSWER]);
            break;
  }
}


void pascal editpwrdclass::showheaders(void) {
  clsbox(3,TOPLINE-3,77,TOPLINE-1,Colors[DISPLAY]);

  fastprint( 3,TOPLINE-2,"  Password    Sec ",Colors[DISPLAY]);
  fastprint( 3,TOPLINE-1,"ออออออออออออ อออออ",Colors[DISPLAY]);

  switch (Group) {
    case 0: fastprint(29,TOPLINE-3,       "Daily   Base  Batch File  Byte   File    KByte ",Colors[DISPLAY]);
            fastprint(22,TOPLINE-2,"Time  K Bytes  Baud  Limit Ratio Ratio  Limit   Limit ",Colors[DISPLAY]);
            fastprint(22,TOPLINE-1,"อออออ อออออออ ออออออ อออออ อออออ อออออ อออออออ อออออออ",Colors[DISPLAY]);
            break;
    case 1: fastprint(23,TOPLINE-3, "File  K-Byte   Enforce  Allow  Force    Demo  Enable" ,Colors[DISPLAY]);
            fastprint(22,TOPLINE-2,"Credit Credit Time Limit Alias Read Mail Acct. Account",Colors[DISPLAY]);
            fastprint(22,TOPLINE-1,"ออออออ ออออออ ออออออออออ อออออ อออออออออ อออออ อออออออ",Colors[DISPLAY]);
            break;
  }
}


static void near pascal showratio(int X, int Y, long Value) {
  char Ratio[10];
  char Str[10];

  decimal(Ratio,Value);
  Ratio[5] = 0;
  sprintf(Str,"%5.5s",Ratio);
  fastprint(X,Y,Str,Colors[ANSWER]);
}



void pascal editpwrdclass::editrecord(void *Rec) {
  static char ratio_mask[] = {5,' ','.', 0,'0','9'};
  static char track_mask[] = {3,'Y','N','T'};
  pwrdtype *p = (pwrdtype *) Rec;
  char      Str[2];
  char      FileRatio[5];
  char      ByteRatio[5];

  switch(Group) {
    case 0: switch (Column) {
              case 0: inputstr( 3,LineNum,12,"",p->Pwrd,p->Pwrd,ALLTEXT,INPUT_CLEAR,PWRDFILE);
                      stripright(p->Pwrd,' ');
                      break;
              case 1: inputnum(17,LineNum, 3,"",&p->Sec            ,vBYTE,PWRDFILE+1);
                      break;
              case 2: inputnum(22,LineNum, 4,"",&p->Time           ,vINT ,PWRDFILE+2);
                      break;
              case 3: inputnum(29,LineNum, 5,"",&p->Bytes          ,vLONG,PWRDFILE+3);
                      if (p->Bytes > 32767)
                        p->Bytes = 32767;
                      break;
              case 4: inputnum(36,LineNum, 6,"",&p->BaseBaud       ,vLONG,PWRDFILE+4);
                      break;
              case 5: inputnum(44,LineNum, 4,"",&p->BatchLimit     ,vINT ,PWRDFILE+5);
                      break;
              case 6: decimal(FileRatio,p->FileRatio);
                      FileRatio[5] = 0;
                      inputstr(49,LineNum, 5,"",FileRatio,FileRatio,ratio_mask,INPUT_CLEAR,PWRDFILE+6);
                      p->FileRatio = timesten(FileRatio);
                      showratio(49,LineNum,p->FileRatio);
                      break;
              case 7: decimal(ByteRatio,p->ByteRatio);
                      ByteRatio[5] = 0;
                      inputstr(55,LineNum, 5,"",ByteRatio,ByteRatio,ratio_mask,INPUT_CLEAR,PWRDFILE+7);
                      p->ByteRatio = timesten(ByteRatio);
                      showratio(55,LineNum,p->ByteRatio);
                      break;
              case 8: inputnum(61,LineNum, 7,"",&p->TotalFileLimit ,vLONG,PWRDFILE+8);
                      break;
              case 9: inputnum(69,LineNum, 7,"",&p->TotalKByteLimit,vLONG,PWRDFILE+9);
                      break;
            }
            break;
    case 1: switch (Column) {
              case 0: inputnum(23,LineNum, 5,"",&p->FileRatioCredits,vUNSIGNED,PWRDFILE+10);
                      break;
              case 1: inputnum(30,LineNum, 5,"",&p->KByteRatioCredits,vUNSIGNED,PWRDFILE+11);
                      break;
              case 2: inputnum(40,LineNum, 1,"",&p->EnforceTimeLimit,vBOOL   ,PWRDFILE+12);
                      break;
              case 3: inputnum(49,LineNum, 1,"",&p->AllowAlias      ,vBOOL   ,PWRDFILE+13);
                      break;
              case 4: inputnum(57,LineNum, 1,"",&p->ForceReadMail   ,vBOOL   ,PWRDFILE+14);
                      break;
              case 5: inputnum(65,LineNum, 1,"",&p->MultipleLogins  ,vBOOL   ,PWRDFILE+15);
                      break;
              case 6: Str[0] = p->EnableAccounting;
                      inputstr(72,LineNum, 1,"",Str,Str,track_mask,INPUT_CAPS|INPUT_CLEAR,PWRDFILE+16);

                      switch (Str[0]) {
                        case 'Y':
                        case 'T': p->EnableAccounting = Str[0]; break;
                        default : p->EnableAccounting = 'N';    break;
                      }
                      break;
            }
            break;
  }
}


void pascal editpwrdclass::handlekeys(void *Rec) {
  switch (KeyFlags) {
    case F2    : SaveColumns[Group] = Column;
                 Group = NextGroup[Group];
                 Column = SaveColumns[Group];
                 NumColumns = ColumnSetup[Group];
                 showheaders();
                 NeedToDisplay = TRUE;
                 break;
    default    : editclass::handlekeys(Rec);
  }
}


/********************************************************************
*
*  Function: editpwrd()
*
*  Desc    : Allows the user to edit the pwrd table.
*
*/

void pascal editpwrd(void) {
  editpwrdclass  Pwrd("Security Levels Definitions",sizeof(pwrdtype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,9,1);

  Group = 0;
  SaveColumns[0] = 0;
  SaveColumns[1] = 0;

  if (Pwrd.load(PcbData.PwdFile) == -1)
    return;

  Pwrd.addexitkey(60,F2,"F2=Rotate");
  Pwrd.edit();
}
