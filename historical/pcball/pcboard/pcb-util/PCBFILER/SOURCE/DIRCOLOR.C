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


#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <help.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LFT       30
#define TOP        5
#define RHT       45
#define BOT       20
#define NUMATTS   12
#define NUMLINES  18

static bool ColorsInit;
/* static char DirColorStr[DUPE+1][15]; */
static char ColorsArray[25][80];


typedef struct {
  char X1;
  char Y1;
  char X2;
  char Y2;
  char Color;
} AttType;

static AttType Attributes[NUMATTS] = {
 {  1, 1,78, 4,HEAD },

 {  1, 5,12,22,NAME },
 { 14, 5,21,22,SIZE },
 { 24, 5,31,20,FDTE },
 { 32, 5,32,22,NEWF },
 { 34, 5,78,22,FDSC },

 { 24,21,31,21,OFLN },
 { 24,22,31,22,DLTD },

 {  1, 6,78, 7,DUPE },
 {  1, 9,78,11,DUPE },
 {  1,13,78,14,DUPE },

 {  1,15,78,19,TEXT }
};

static ScrType ScreenLines[NUMLINES] = {
 { 1, 1,"Sample DIR File illustrating color selections"                                 ,0 },
 { 1, 3,"Filename       Size      Date    Description of File Contents"                 ,0 },
 { 1, 4,"============ ========  ========  =============================================",0 },
 { 1, 5,"145HELP.ZIP     34802  06-19-90* PCBoard v14.5 standard help files - colorized",0 },
 { 1, 6,"                                 using the @X macros."                         ,0 },
 { 1, 7,"                                 Uploaded by: Terry West"                      ,0 },
 { 1, 8,"ANSI145.ZIP      7350  03-20-90* Displays and edits your PCBoard 14.5 text"    ,0 },
 { 1, 9,"                                 files.  Converts @X variables to the correct" ,0 },
 { 1,10,"                                 ANSI statements."                             ,0 },
 { 1,11,"                                 Uploaded by: Mark Herring"                    ,0 },
 { 1,12,"145MENUS.ZIP    12110  08-03-90* Menu enhancements for 14.5, now includes to"  ,0 },
 { 1,13,"                                 expert menus for the new expert prompts"      ,0 },
 { 1,14,"                                 Uploaded by: Dean Gangstee"                   ,0 },
 { 1,17,"A text line in the middle of the DIR file (possibly used for sub-titles)"      ,0 },
 { 1,18,"------------------------------------------------------------------------"      ,0 },
 { 1,20,"FIRSTONE.ZIP     2176  08-31-90* First test file"                              ,0 },
 { 1,21,"TESTFILE.ZIP    14536  OFF-LINE  This file has been moved offline"             ,0 },
 { 1,22,"TROJAN.ZIP       2583  DELETED   This file has been deleted"                   ,0 }
};


static void near pascal setatt3(int X1, int Y1, int X2, int Y2, int ColorNum) {
  int X;
  int Y;

  if (ColorsInit) {
    for (Y = Y1; Y <= Y2; Y++)
      for (X = X1; X <= X2; X++)
        ColorsArray[Y][X] = ColorNum;
  } else {
    setatt(X1,Y1,X2,Y2,Work.DirColors[ColorNum]);
  }
}


static void near pascal paint(void) {
  AttType *p;

  for (p = Attributes; p < &Attributes[NUMATTS]; p++)
    setatt3(p->X1,p->Y1,p->X2,p->Y2,p->Color);
}


static void near pascal movepointer(int Ch, int Top, int Bot, int Lft, int Rht, int *X, int *Y) {
  int X1,Y1;

  X1 = *X;
  Y1 = *Y;
  switch(Ch) {
    case /* F1        */   59: showhelp(0);                            break;
    case /* home      */   71: if (X1 > Lft)    X1--;   else X1 = Rht; //lint !e616 fall thru
    case /* up        */   72: if (Y1 > Top)    Y1--;   else Y1 = Bot; break;
    case /* pgup      */   73: if (Y1 > Top)    Y1--;   else Y1 = Bot; //lint !e616 fall thru
    case /* right     */   77: if (X1 < Rht)    X1++;   else X1 = Lft; break;
    case /* end       */   79: if (Y1 < Bot)    Y1++;   else Y1 = Top; //lint !e616 fall thru
    case /* left      */   75: if (X1 > Lft)    X1--;   else X1 = Rht; break;
    case /* pgdn      */   81: if (X1 < Rht)    X1++;   else X1 = Lft; //lint !e616 fall thru
    case /* down      */   80: if (Y1 < Bot)    Y1++;   else Y1 = Top; break;
    case /* ctl-left  */  115: if (X1 > Lft+20) X1-=20; else X1 = Lft; break;
    case /* ctl-right */  116: if (X1 < Rht-20) X1+=20; else X1 = Rht; break;
    case /* ctl-end   */  117: X1 = Lft; Y1 = Bot;                     break;
    case /* ctl-pgdn  */  118: X1 = Rht; Y1 = Bot;                     break;
    case /* ctl-home  */  119: X1 = Lft; Y1 = Top;                     break;
    case /* ctl-pgup  */  132: X1 = Rht; Y1 = Top;                     break;
  }
  *X = X1;
  *Y = Y1;
}


static void near pascal drawpalette(void) {
  int ColorNum;
  int Back;
  int Loc;
  int Y;

  clsbox(LFT-2,TOP-2,RHT+2,BOT+2,0x08);
  box(LFT-1,TOP-1,RHT+1,BOT+1,0x08,SINGLE);
  for (Back = 0, Y = TOP*160; Back <= 0xF0; Back += 0x10, Y += 160)
    for (ColorNum = Back, Loc = Y+(LFT << 1); ColorNum < Back+16; ColorNum++, Loc+=2)
      fastputch(Loc,'X',ColorNum);
}

static void near pascal changedircolors(int CursorX, int CursorY) {
  char Ch;
  bool Ext;
  int  ColorNum;
  int  X;
  int  Y;
  savescrntype ScrnBuf;

  ColorNum = ColorsArray[CursorY][CursorX];
  savescreen(&ScrnBuf);

  X = LFT + (Work.DirColors[ColorNum] & 0x0F);
  Y = TOP + (Work.DirColors[ColorNum] >> 4);

  while(1) {
    gotoxy(X,Y);

    drawpalette();
    box(X-1,Y-1,X+1,Y+1,(Y-TOP ? 0 : 8) + ((Y-TOP) << 4),SINGLE);
    setcursor(CUR_BLANK);
    Ch = inkey(&Ext,CLOCK);
    if (! Ext) {
      if (Ch == 27)
        break;
      if (Ch == 13) {
        Work.DirColors[ColorNum] = ((Y-TOP) << 4) + (X-LFT);
        break;
      }
    } else {
      movepointer(Ch,TOP,BOT,LFT,RHT,&X,&Y);
    }
  }

  restorescreen(&ScrnBuf);
  gotoxy(CursorX,CursorY);
  paint();
}


static void near pascal instructcolors(void) {
  char Ch;
  savescrntype ScrnBuf;

  savescreen(&ScrnBuf);
  boxcls(14, 5,64,20,Colors[HELPBOX],DOUBLE);
  fastprint(33, 7,"INSTRUCTIONS",Colors[HELPTITLE]);
  fastprint(33, 8,"様様様様様様",Colors[HELPTITLE]);
  fastprint(16,10,"1) Move the cursor to the color on the screen",Colors[HELPTEXT]);
  fastprint(19,11,   "you wish to change and press the ENTER key",Colors[HELPTEXT]);
  fastprint(16,13,"2) Select the color you want from the choices",Colors[HELPTEXT]);
  fastprint(19,14,   "available by pointing and pressing ENTER"  ,Colors[HELPTEXT]);
  fastprint(16,16,"3) When finished, press ESC to exit"          ,Colors[HELPTEXT]);
  fastprint(25,18," press any key to continue ",Colors[HELPDESC]);
  setcursor(CUR_NORMAL);
  gotoxy(51,18);
  Ch = inkey(&Ch,CLOCK);
  restorescreen(&ScrnBuf);
}


void pascal choosedircolors(void) {
  char Ch;
  bool Ext;
  int  X;
  int  Y;

  clsbox(1,1,78,23,0);
  showscreen(ScreenLines,NUMLINES);

  ColorsInit = TRUE;
  paint();
  ColorsInit = FALSE;
  paint();

  instructcolors();

  X = 39;
  Y = 15;

  while(1) {
    gotoxy(X,Y);
    setcursor(CUR_BLOCK);
    Ch = inkey(&Ext,CLOCK);
    if (! Ext) {
      if (Ch == 27)
        break;
      if (Ch == 13)
        changedircolors(X,Y);
    } else {
      if (Ch == 59)
        showhelp(COLORSELECT);
      else
        movepointer(Ch,0,24,0,79,&X,&Y);
    }
  }
  savedefaults();
}

