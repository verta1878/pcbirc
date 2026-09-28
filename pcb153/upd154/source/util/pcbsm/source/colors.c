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
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <help.h>
#ifdef DEBUG
#include <memcheck.h>
#endif


#define LFT       30
#define TOP        9
#define RHT       45
#define BOT       16
#define NUMATTS   28
#define NUMLINES  22

typedef struct {
  char X1;
  char Y1;
  char X2;
  char Y2;
  char Color;
} AttType;

typedef struct {
  char Line[80];
} arraytype;

static bool Init;

static AttType Attributes[NUMATTS] = {
  {  0, 0,79,24,OUTBOX},
  { 26, 1,51, 1,HEADING},
  {  3,24,12,24,STATUS},
  { 16,24,25,24,STATUS},
  { 47,24,77,24,STATUS},
  { 32,24,42,24,HELPKEY},
  { 2,15,77,15,OUTBOX},
  { 5, 3,34,13,MENUBOX},
  { 7, 7,33,11,MENUSELECT},
  {15, 4,23, 4,MENUTITLE},
  { 9, 7,32, 7,MENUBAR},
  { 7,10,32,11,MENUUNAVAIL},
  { 9,11,32,11,MENUUNBAR},
  { 6,13,33,13,MENUDESC},
  {37, 3,74,13,HELPBOX},
  {51, 5,60, 6,HELPTITLE},
  {49, 8,61, 8,HELPSUB},
  {43,10,68,10,HELPTEXT},
  {38,12,73,12,HELPDESC},
  { 3,17,13,20,QUESTION},
  {17,17,41,20,ANSWER},
  {15,17,15,20,DISPLAY},
  {15,18,15,18,ANSWER},
  {17,18,41,18,EDIT},
  {48,17,73,19,DISPLAY},
  {79, 3,79,21,SCALECOLOR1},
  {79,12,79,12,SCALECOLOR2},
  {19,22,60,22,DESC}
};

static ScrType ScreenLines[NUMLINES] = {
  {26, 1,                        "Headings and Screen Titles"                            ,0},
  { 5, 3,   "ึฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท  ีออออออออออออออออออออออออออออออออออออธ    ",0},
  { 5, 4,   "บ         Main Menu          บ  ณ                                    ณ    ฑ",0},
  { 5, 5,   "บฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤบ  ณ             HELP TITLE             ณ    ฑ",0},
  { 5, 6,   "บ                            บ  ณ             ฤฤฤฤฤฤฤฤฤฤ             ณ    ฑ",0},
  { 5, 7,   "บ A  Selected Menu Item      บ  ณ                           page: 01 ณ    ฑ",0},
  { 5, 8,   "บ B  Other Menu Items ...    บ  ณ           Help Subtitle            ณ    ฑ",0},
  { 5, 9,   "บ C  Other Menu Items ...    บ  ณ                                    ณ    ฑ",0},
  { 5,10,   "บ D  Unavailable Menu Item   บ  ณ     Text for help screen 12345     ณ    ฑ",0},
  { 5,11,   "บ E  Highlighted unavailable บ  ณ                                    ณ    ฑ",0},
  { 5,12,   "บ                            บ  ณ     press  PGDN  to go forward     ณ    ฑ",0},
  { 5,13,   "ำ   Use arrow keys to move   ฝ  ิออออออออออออออออออออออออออออออออออออพ    ฑ",0},
  {79,14,                                                                             "ฑ",0},
  { 2,15,"๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙๙ ฑ",0},
  {79,16,                                                                             "ฑ",0},
  { 3,17, "Question #1 : Unhighlighted Input Field      Descriptive on screen text     ฑ",0},
  { 3,18, "Question #2 ? Current Input Field            --------------------------     ฑ",0},
  { 3,19, "Question #3 : Unhighlighted Input Field      ABCDEFGHIJKLMNOPQRSTUVWXYZ     ฑ",0},
  { 3,20, "Question #4 : Unhighlighted Input Field                                     ฑ",0},
  {79,21,                                                                             "",0},
  {20,22,                  "Special Instructions or Descriptive Text"                    ,0},
  { 3,24, " 12:00:00 อออ 01/01/88 ออออออ F1 อ Help ออออ caps: OFF  num: OFF  ins: OFF "  ,0}
};


static void near pascal setatt2(int X1, int Y1, int X2, int Y2, int ColorNum, arraytype *ColorsArray) {
  int X;
  int Y;

  if (Init) {
    for (Y = Y1; Y <= Y2; Y++)
      for (X = X1; X <= X2; X++)
        ColorsArray[Y].Line[X] = ColorNum;
  } else {
    setatt(X1,Y1,X2,Y2,Colors[ColorNum]);
  }
}

static void near pascal paint(arraytype *ColorsArray) {
  AttType *p;

  for (p = Attributes; p < &Attributes[NUMATTS]; p++)
    setatt2(p->X1,p->Y1,p->X2,p->Y2,p->Color,ColorsArray);
}

static void near pascal movepointer(int Ch, int Top, int Bot, int Lft, int Rht, int *X, int *Y) {
  int X1,Y1;

  X1 = *X;
  Y1 = *Y;
  switch(Ch) {
    case /* F1        */   59: showhelp(0);                            break;
    case /* home      */   71: if (X1 > Lft)    X1--;   else X1 = Rht; //lint !e616  falls thru
    case /* up        */   72: if (Y1 > Top)    Y1--;   else Y1 = Bot; break;
    case /* pgup      */   73: if (Y1 > Top)    Y1--;   else Y1 = Bot; //lint !e616  falls thru
    case /* right     */   77: if (X1 < Rht)    X1++;   else X1 = Lft; break;
    case /* end       */   79: if (Y1 < Bot)    Y1++;   else Y1 = Top; //lint !e616  falls thru
    case /* left      */   75: if (X1 > Lft)    X1--;   else X1 = Rht; break;
    case /* pgdn      */   81: if (X1 < Rht)    X1++;   else X1 = Lft; //lint !e616  falls thru
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
  for (Back = 0, Y = TOP*160; Back < 0x80; Back += 0x10, Y += 160)
    for (ColorNum = Back, Loc = Y+(LFT << 1); ColorNum < Back+16; ColorNum++, Loc+=2)
      fastputch(Loc,'X',ColorNum);
}


static void near pascal changecolors(int CursorX, int CursorY, arraytype *ColorsArray) {
  int  ColorNum;
  int  X;
  int  Y;
  char Ch;
  char Ext;
  savescrntype ScrnBuf;

  ColorNum = ColorsArray[CursorY].Line[CursorX];
  savescreen(&ScrnBuf);

  X = LFT + (Colors[ColorNum] & 0x0F);
  Y = TOP + (Colors[ColorNum] >> 4);

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
        Colors[ColorNum] = ((Y-TOP) << 4) + (X-LFT);
        break;
      }
    } else {
      movepointer(Ch,TOP,BOT,LFT,RHT,&X,&Y);
    }
  }

  restorescreen(&ScrnBuf);
  gotoxy(CursorX,CursorY);
  paint(ColorsArray);
}


static void near pascal instructcolors(void) {
  savescrntype ScrnBuf;
  char Ch;

  savescreen(&ScrnBuf);
  boxcls(14, 5,64,20,Colors[HELPBOX],DOUBLE);
  fastprint(33, 7,"INSTRUCTIONS",Colors[HELPTITLE]);
  fastprint(33, 8,"ออออออออออออ",Colors[HELPTITLE]);
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


void pascal interact(void) {
  arraytype ColorsArray[25];
  char Ch;
  char Ext;
  int  X;
  int  Y;

  clscolor(0);
  box(0,0,79,24,0,DOUBLE);
  showscreen(ScreenLines,NUMLINES);

  Init = TRUE;
  paint(ColorsArray);
  Init = FALSE;
  paint(ColorsArray);

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
        changecolors(X,Y,ColorsArray);
    } else {
      if (Ch == 59)
        showhelp(COLORSELECT);
      else
        movepointer(Ch,0,24,0,79,&X,&Y);
    }
  }
}


void pascal setdefcolor1(void) {
  memcpy(Colors,DefColor1,NUMCOLORS);
}


void pascal setdefcolor2(void) {
  memcpy(Colors,DefColor2,NUMCOLORS);
}


void pascal setdefbandw(void) {
  memcpy(Colors,DefBandW,NUMCOLORS);
}


void pascal customcolors(void) {
  bool Save;

  menusel(0,COLORMENU,SMALL,0,TRUE);

  Save = TRUE;
  boxcls(21,14,57,21,Colors[OUTBOX],DOUBLE);
  fastprint(33,16,"SAVE COLORS",Colors[HEADING]);
  fastprint(33,17,"อออออออออออ",Colors[HEADING]);
  setcursor(CUR_NORMAL);
  inputnum(23,19,1,"Save the color setup to disk",&Save,vBOOL,0);
  if (Save && KeyFlags != ESC) {
    if (Scrn_Mode)
      savecolors(Colors,BandW);
    else
      savecolors(Color,Colors);
  }
}
