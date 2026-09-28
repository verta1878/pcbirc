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


#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <country.h>
#include <help.h>
#include <misc.h>
#include <dosfunc.h>
#include <pcb.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "setup.h"
#include "setup.ext"
#include "edit.hpp"


#ifdef DEBUG
#include <memcheck.h>
#endif

enum { F2 = NEXTKEY };


#define LEFTSIDE              1
#define RIGHTSIDE            78
#define TOPLINE               5
#define BOTTOM                2

typedef struct  {
  char   Verb[20];
  char   Params[45];
} verbtype;

#define NUMVERBS 8

static char Verbs[NUMVERBS][20] = {"Allow-Human-Callers",
                                   "Allow-Route-To",
                                   "Poll",
                                   "Hold",
                                   "Crash",
                                   "Route-To",
                                   "FREQ",
                                   "SEND"};


class editverbclass : public editclass {
  public:
    editverbclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal showfooters(void) { editclass::showfooters(); }
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};



void pascal editverbclass::loadrecords(DOSFILE *File) {
  char     *p;
  verbtype  Rec;
  char      Buf[256];

  while (dosfgets(Buf,256,File) != -1) {
    newrecord(&Rec);
    if ((p = strchr(Buf,',')) != NULL)
      *p = 0;

    maxstrcpy(Rec.Verb,Buf,sizeof(Rec.Verb));
    if (p != NULL) {
      for (p++; *p == ' '; p++); // skip over spaces
      maxstrcpy(Rec.Params,p,sizeof(Rec.Params));
    }

    addrecord(&Rec);
  }
}


void pascal editverbclass::saverecords(DOSFILE *File) {
  int       X;
  verbtype *p;
  char      Temp[80];

  for (X = 1; X <= TotalRecs; X++) {
    p = (verbtype *) getrecord(X);
    if (dosfputs(p->Verb,File) == -1)
      break;

    stripright(p->Params,' ');
    if (p->Params[0] != 0) {
      sprintf(Temp,",%s",p->Params);
      if (dosfputs(Temp,File) == -1)
        break;
    }

    if (dosfputs("\r\n",File) == -1)
      break;
  }
}



void pascal editverbclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  verbtype *p = (verbtype *) Rec;
  char      Temp[10];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE, LineNum, Temp, Colors[ANSWER]);

  fastprint(LEFTSIDE +  5, LineNum, p->Verb  , Colors[ANSWER]);
  fastprint(LEFTSIDE + 27, LineNum, p->Params, Colors[ANSWER]);
}


void pascal editverbclass::showheaders(void) {
  fastprint(LEFTSIDE + 5, TOPLINE - 2, "Fido Verb             Parameters"                                  , Colors[DISPLAY]);
  fastprint(LEFTSIDE + 5, TOPLINE - 1, "อออออออออออออออออออ   ออออออออออออออออออออออออออออออออออออออออออออ", Colors[DISPLAY]);
}



void pascal editverbclass::handlekeys(void *Rec) {
  int       Select;
  verbtype *p = (verbtype *) Rec;

  switch (KeyFlags) {
    case F2  : if ((Select = fileselect(17, (LineNum > 15) ? LineNum - 7 : LineNum, NUMVERBS, Verbs, 20,NUMVERBS)) != -1)
                 strcpy(p->Verb,Verbs[Select]);
               KeyFlags = NOTHING;
               break;
    default  : editclass::handlekeys(Rec); break;
  }
}


void pascal editverbclass::editrecord(void *Rec) {
  verbtype *p = (verbtype *) Rec;

  switch (Column) {
    case  0: inputstr(LEFTSIDE + 5,LineNum,19, "", p->Verb, p->Verb, ALLTEXT, INPUT_CAPS, FIDOATTRIB);
             break;
    case  1: inputstr(LEFTSIDE +27,LineNum,44, "", p->Params, p->Params, ALLTEXT, INPUT_CAPS, FIDOATTRIB);
             break;
  }
}



void pascal editverblist(char *Name) {
  editverbclass Verbs("Fido Attributes Editor",sizeof(verbtype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,1);

  if (Verbs.load(Name) != -1) {
    Verbs.addexitkey(60,F2,"F2=Pick List");
    Verbs.edit();
  }
}
