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


#include <string.h>
#include <help.h>
#include <pcb.h>
#include <misc.h>
#include <stdio.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <country.h>
#include "setup.h"
#include "setup.ext"
#include "edit.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif


enum { F2=NEXTKEY, F3 };

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      6
#define BOTTOM       2
#define RECSIZE     sizeof(ItemListType) - 1  // used because in-memory we hold a NULL terminator


typedef struct {
  char Day[9];
} daytype;

class editdaysclass : public editclass {
  public:
    editdaysclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec) { editclass::handlekeys(Rec); }
};




void pascal editdaysclass::loadrecords(DOSFILE *File) {
  daytype Rec;
  char    Str[20];

  while (dosfgets(Str,sizeof(Str),File) != -1) {
    maxstrcpy(Rec.Day,Str,sizeof(Rec.Day));
    addrecord(&Rec);
  }
}



void pascal editdaysclass::saverecords(DOSFILE *File) {
  long     X;
  daytype *p;

  for (X = 1; X <= TotalRecs; X++) {
    p = (daytype *) getrecord(X);
    if (dosfputs(p->Day,File) == -1 || dosfputs("\r\n",File) == -1)
      break;
  }
}



void pascal editdaysclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  daytype *p = (daytype *) Rec;
  char     Temp[10];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,Temp,Colors[ANSWER]);

  if (p->Day[0] > ' ') {
    strcpy(Temp,p->Day);
    countrydate(Temp);
    fastprint(LEFTSIDE+8,LineNum,Temp,Colors[ANSWER]);
  }
}



void pascal editdaysclass::showheaders(void) {
  fastprint(LEFTSIDE+6,TOPLINE-3,"Disable Peak",Colors[DISPLAY]);
  fastprint(LEFTSIDE+9,TOPLINE-2,   "Dates"    ,Colors[DISPLAY]);
  fastprint(LEFTSIDE+6,TOPLINE-1,"様様様様様様",Colors[DISPLAY]);
}



void pascal editdaysclass::editrecord(void *Rec) {
  static char DateMask[] = {7,0,'0','9','-','/','X','_'};  /* character #7 is handled below */
  int         ValDate;
  daytype    *p = (daytype *) Rec;

  DateMask[7] = Country.DateSep[0];

  do {
    if (p->Day[0] > ' ')
      countrydate(p->Day);

    inputstr(LEFTSIDE+8,LineNum,sizeof(p->Day)-1,"",p->Day,p->Day,DateMask,INPUT_CAPS,HOLIDAYHELP);
    if (p->Day[0] > ' ') {
      formatdate(p->Day);
      uncountrydate(p->Day);
    } else {
      memset(p->Day,' ',sizeof(p->Day)-1);
      p->Day[sizeof(p->Day)-1] = 0;
    }

  } while ((ValDate = datevalid(p->Day)) == FALSE && KeyFlags != ESC);

  if (! ValDate) {
    memset(p->Day,' ',sizeof(daytype)-1);
    p->Day[sizeof(daytype)-1] = 0;
  }
}



/********************************************************************
*
*  Function: editblt()
*
*  Desc    : Reads the table in, displays it on the screen and allows the
*            user to edit it and save the contents when done.
*
*/

void pascal editdays(char *Name) {
  editdaysclass Days("Holiday Editor",sizeof(daytype),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,0,0);

  if (Days.load(Name) == -1)
    return;

  Days.edit();
}
