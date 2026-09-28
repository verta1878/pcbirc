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


class editlangclass : public editclass {
  public:
    editlangclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec);
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


typedef struct {
  char Lang[26];
  char Ext[5];
  char Country[4];
  char CodePage[4];
  char YesChar[2];
  char NoChar[2];
} langtype;


void pascal editlangclass::loadrecords(DOSFILE *File) {
  langtype Rec;
  char     Buf[256];

  while (dosfgets(Buf,sizeof(Buf),File) != -1) {
    newrecord(&Rec);
    maxstrcpy(Rec.Lang,parse(Buf),sizeof(Rec.Lang));
    maxstrcpy(Rec.Ext,parse(NULL),sizeof(Rec.Ext));
    stripleft(Rec.Ext,'.');
    maxstrcpy(Rec.Country,parse(NULL),sizeof(Rec.Country));
    maxstrcpy(Rec.CodePage,parse(NULL),sizeof(Rec.CodePage));
    maxstrcpy(Rec.YesChar,parse(NULL),sizeof(Rec.YesChar));
    maxstrcpy(Rec.NoChar,parse(NULL),sizeof(Rec.NoChar));
    addrecord(&Rec);
  }
}


void pascal editlangclass::saverecords(DOSFILE *File) {
  long      X;
  langtype *p;
  char      Ext[10];
  char      Temp[100];

  for (X = 1; X <= TotalRecs; X++) {
    p = (langtype *) getrecord(X);

    stripright(p->Lang,' ');
    stripright(p->Ext,' ');
    stripright(p->Country,' ');
    stripright(p->CodePage,' ');
    if (p->YesChar[0] < 'A')
      p->YesChar[0] = 'Y';
    if (p->NoChar[0] < 'A')
      p->NoChar[0] = 'N';
    if (p->Ext[0] == 0)
      Ext[0] = 0;
    else {
      Ext[0] = '.';
      strcpy(&Ext[1],p->Ext);
    }
    sprintf(Temp,"%s,%s,%s,%s,%s,%s\r\n",p->Lang,Ext,p->Country,p->CodePage,p->YesChar,p->NoChar);
    if (dosfputs(Temp,File) == -1)
      break;
  }
}


void pascal editlangclass::newrecord(void *Rec) {
  langtype *p = (langtype *) Rec;

  editclass::newrecord(Rec);
  p->YesChar[0] = 'Y';
  p->NoChar[0] = 'N';
}


void pascal editlangclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  langtype *p = (langtype *) Rec;
  char      Temp[11];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,Temp   ,Colors[ANSWER]);

  fastprint(LEFTSIDE+ 8,LineNum,p->Lang    ,Colors[ANSWER]);
  fastprint(LEFTSIDE+38,LineNum,p->Ext     ,Colors[ANSWER]);
  fastprint(LEFTSIDE+48,LineNum,p->Country ,Colors[ANSWER]);
  fastprint(LEFTSIDE+57,LineNum,p->CodePage,Colors[ANSWER]);
  fastprint(LEFTSIDE+66,LineNum,p->YesChar ,Colors[ANSWER]);
  fastprint(LEFTSIDE+71,LineNum,p->NoChar  ,Colors[ANSWER]);
}


void pascal editlangclass::showheaders(void) {
  fastprint(LEFTSIDE+8,TOPLINE-2,"      Language             Extension  Country  CodePage  Yes  No",Colors[DISPLAY]);
  fastprint(LEFTSIDE+8,TOPLINE-1,"อออออออออออออออออออออออออ  อออออออออ  อออออออ  ออออออออ  อออ  ออ",Colors[DISPLAY]);
//fastprint(LEFTSIDE,Scrn_BottomRow-1,"  ESC=Exit   Alt-R=Repeat a Line   Alt-I=Insert a Line   Alt-D=Delete a Line  ",Colors[DESC]);
}


void pascal editlangclass::editrecord(void *Rec) {
  langtype *p = (langtype *) Rec;

  switch (Column) {
    case 0: inputstr(LEFTSIDE+8,LineNum,25,"",p->Lang,p->Lang,ALLTEXT,INPUT_CLEAR,LANGFILE+0);
            stripright(p->Lang,' ');
            break;
    case 1: inputstr(LEFTSIDE+38,LineNum,3,"",p->Ext,p->Ext,ALLFILE,INPUT_CAPS|INPUT_CLEAR,LANGFILE+1);
            stripright(p->Ext,' ');
            break;
    case 2: inputstr(LEFTSIDE+48,LineNum,3,"",p->Country,p->Country,ALLNUM,INPUT_CAPS|INPUT_CLEAR,LANGFILE+2);
            stripright(p->Country,' ');
            break;
    case 3: inputstr(LEFTSIDE+57,LineNum,3,"",p->CodePage,p->CodePage,ALLNUM,INPUT_CAPS|INPUT_CLEAR,LANGFILE+3);
            stripright(p->CodePage,' ');
            break;
    case 4: inputstr(LEFTSIDE+66,LineNum,1,"",p->YesChar,p->YesChar,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,LANGFILE+4);
            stripright(p->YesChar,' ');
            break;
    case 5: inputstr(LEFTSIDE+71,LineNum,1,"",p->NoChar,p->NoChar,ALLTEXT,INPUT_CAPS|INPUT_CLEAR,LANGFILE+5);
            stripright(p->NoChar,' ');
            break;
  }
}


/********************************************************************
*
*  Function: editlang()
*
*  Desc    : Allows the user to edit the pcbml file.
*
*/

void pascal editlang(void) {
  editlangclass  Lang("Language Table",sizeof(langtype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,5,0);

  if (Lang.load(PcbData.MultiLang) == -1)
    return;

  Lang.edit();
}
