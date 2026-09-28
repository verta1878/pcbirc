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


#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <cnameidx.h>
#include <help.h>
#include <misc.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define CONFMENU       2
#define MENUBOXLINE    8
#define MENUINPUTLINE  (MENUBOXLINE+2)
#define MENUFOUNDLINE  (MENUBOXLINE+5)
#define MENUINBOXLINE  (MENUBOXLINE+6)


static char ALLCONF[]  = {23, ' ','$','#','!','&', '-', '{', '}', '(',')',',','+','.', 0, '0','9', 0,'A','z', 0,'€','þ','/' };

/********************************************************************
*
*  Function: makeconfmenu()
*
*  Desc    :
*
*  Calls   :
*/

static void near  makeconfmenu(int Offset) {
  pcbconftype Conf;
  char *q;
  unsigned t;
  unsigned x;
  unsigned y;

  x = Menu[CONFMENU].First;
  y = x + 42;
  q = MenuList[x];

  for (t = Offset;  x < y; q+=36, x++, t++) {
    if (t <= PcbData.NumConf) {
      getconfrecord(t,&Conf);
      MenuAvail[x] = TRUE;
      sprintf(q,"%u) %-13.13s",t,Conf.Name);
    } else {
      MenuAvail[x] = FALSE;
      sprintf(q,"%u)",t);
    }
  }
}


int  boundschecking(unsigned Selection, unsigned *Offset) {
  if (Selection == 65535U)
    return(-1);

  if (Selection < *Offset || Selection > *Offset + 41) {
    *Offset = ((Selection / 3) * 3);
    makeconfmenu(*Offset + 1);
  }
  return(0);
}


static void _NEAR_ LIBENTRY showrec(int LineNum, cnamesidxtype *IdxRec, bool Highlight) {
  char Temp[80];

  sprintf(Temp,"%5u) %s",IdxRec->Num,IdxRec->Name);
  padstr(Temp,' ',66);
  fastprint(7,LineNum,Temp,Colors[(Highlight ? MENUBAR : MENUSELECT)]);
}


static void _NEAR_ LIBENTRY showconfmenu(unsigned Rec, cnamesidxtype *IdxRec, int Select) {
  int  X;
  bool Found;

  for (X = MENUINBOXLINE+1, Found = TRUE; X <= MENUINBOXLINE+6; X++) {
    if (Found) {
      showrec(X,IdxRec,(bool) (Select == (X - (MENUINBOXLINE+1))));
      if (getconfbyrec(++Rec,IdxRec) == -1)
        Found = FALSE;
    } else
      clsbox(7,X,72,X,Colors[MENUSELECT]);
  }
}


void LIBENTRY confnamesearch(char *Str) {
  unsigned      Rec;
  cnamesidxtype IdxRec;

  if ((Rec = findconfbyname(Str,&IdxRec)) != (unsigned) -1)
    showconfmenu(Rec,&IdxRec,0);
}


static unsigned _NEAR_ LIBENTRY confnamemenu(unsigned TopRec, int First) {
  bool           Extended;
  int            Key;
  int            X,Y;
  int            Select;
  cnamesidxtype  IdxRec;

  X = wherex();
  Y = wherey();
  Select = 0;
  if (First != Select) {
    setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUSELECT]);
    Select = First;
    setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUBAR]);
  }
  gotoxy(14,MENUINBOXLINE+Select+1);

  while (1) {
    Key = inkey(&Extended,CLOCK);
    if (Extended) {
      switch (Key) {
        case 72: if (Select == 0) {
                   if (TopRec > 0) {
                     if (getconfbyrec(TopRec-1,&IdxRec) != -1) {
                       TopRec--;
                       showconfmenu(TopRec,&IdxRec,0);
                     }
                   }
                 } else {
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUSELECT]);
                   Select--;
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUBAR]);
                   gotoxy(14,MENUINBOXLINE+Select+1);
                 }
                 break;
        case 80: if (Select < 5) {
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUSELECT]);
                   Select++;
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUBAR]);
                   gotoxy(14,MENUINBOXLINE+Select+1);
                 } else {
                   if (getconfbyrec(TopRec+1,&IdxRec) != -1) {
                     TopRec++;
                     showconfmenu(TopRec,&IdxRec,5);
                   }
                 }
                 break;
      }
    } else {
      switch (Key) {
        case  9: // tab or shift-tab, return -1
        case 15: gotoxy(X,Y);
                 return((unsigned) -1);
        case 27: return(0);
        case 13: getconfbyrec(TopRec+Select,&IdxRec);
                 return(IdxRec.Num);
      }
    }
  }
}


/********************************************************************
*
*  Function:  promptuserforconfname()
*
*/

int LIBENTRY promptuserforconfname(int Ch) {
  char           X,Y;
  int            CursorStatus;
  uint           Num;
  uint           Pos;
  cnamesidxtype  Rec;
  char           Temp[61];
  savescrntype   Scrn;

  if (Ch < ' ')
    return(0);

  savescreen(&Scrn);

  CursorStatus = getcursor();
  X = wherex();
  Y = wherey();

  opencnamesidx(FALSE);

// 16  --
// 17  |
// 18  | Begin
// 19  |                                                                |
// 20  |                                                                |
// 21  | Found:
// 22  |
// 23  ---

  boxcls(5,MENUBOXLINE,74,MENUBOXLINE+15,Colors[OUTBOX],DOUBLE);
  fastprint(7,MENUINPUTLINE,"Enter Conference Name (press TAB or DOWN for menu)",Colors[DISPLAY]);
  fastprint(7,MENUFOUNDLINE,"Conferences Found:",Colors[DISPLAY]);
  boxcls(6,MENUINBOXLINE,73,MENUINBOXLINE+8,Colors[MENUBOX],SINGLE);
  fastprint(7,MENUINBOXLINE+8,"  Use arrow keys to move bar, press ENTER to select, ESC to exit  ",Colors[MENUDESC]);

  Input.OrgX     = 7;
  Input.CurX     = 7;
  Input.OrgY     = MENUINPUTLINE+1;
  Input.AnsLimit = 60;
  Input.ScrLimit = 60;
  Input.Old      = Temp;
  Input.Ans      = Temp;
  Input.Mask     = ALLCONF;
  Input.Keyed    = TRUE;
  Input.HelpNum  = 0;

  if (Ch != ' ') {
    Temp[0] = (char) Ch;
    Temp[1] = 0;
    Input.CurX++;
  } else
    Temp[0] = 0;

  confnamesearch(Temp);
  setcursor(CUR_NORMAL);

top:
  inputcallback = confnamesearch;
  inputall();
  inputcallback = NULL;

  Num = 0;
  if (KeyFlags != ESC) {
    Pos = findconfbyname(Input.Ans,&Rec);
    if (KeyFlags == TAB || KeyFlags == DN) {
      if ((Num = confnamemenu(Pos,(KeyFlags == DN ? 1 : 0))) == (unsigned) -1)
        goto top;
    } else {
      if (Pos != (unsigned) -1)
        Num = Rec.Num;
    }
  }

  closecnamesidx();

  restorescreen(&Scrn);
  setcursor(CursorStatus);
  gotoxy(X,Y);
  return(KeyFlags == ESC ? 0 : Num);
}


/********************************************************************
*
*  Function:  promptuserforconfnum()
*
*/

int LIBENTRY promptuserforconfnum(int Ch) {
  char Temp[6];
  int  CursorStatus;
  int  Num;
  char X,Y;

  Temp[0] = (char) Ch;
  Temp[1] = 0;

  CursorStatus = getcursor();
  X = wherex();
  Y = wherey();

  fastprint(49,5,"Conference Number ?",Colors[QUESTION]);
  Input.OrgX     = 69;
  Input.CurX     = 70;
  Input.OrgY     = 5;
  Input.AnsLimit = 5;
  Input.ScrLimit = 5;
  Input.Old      = Temp;
  Input.Ans      = Temp;
  Input.Mask     = ALLNUM;
  Input.Keyed    = TRUE;
  Input.HelpNum  = 0;
  setcursor(CUR_NORMAL);
  inputall();

  Num = atoi(Temp);
  setcursor(CursorStatus);
  clsbox(49,5,73,5,Colors[MENUBOX]);
  gotoxy(X,Y);
  return(KeyFlags == ESC ? 0 : Num);
}


/********************************************************************
*
*  Function: conferences()
*
*  Desc    :
*
*  Calls   :
*/

void  confmenu(void) {
  promptusernum = promptuserforconfnum;
  promptuserstr = promptuserforconfname;
  makeconfmenu(1);
  bigmenusel(CONFMENU,CNAMESMENU,0,confgetdirtoedit,boundschecking,TRUE);
  promptusernum = NULL;
  promptuserstr = NULL;
}
