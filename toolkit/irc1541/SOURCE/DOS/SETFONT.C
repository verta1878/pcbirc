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


#include "screen.h"

#if defined(__OS2__) || __TURBOC__ >= 0x300
#if __TURBOC__ >= 0x0300
  #include <conio.h>
#endif


fonttype LIBENTRY getfont(void) {
  #ifdef __OS2__
    if (Scrn_Adapter < VID_VGA)
      return(BIGFONT);

    return(getviolines() == 25 ? BIGFONT : SMALLFONT);

  #else  /* ifdef __OS2__ */
    struct text_info Info;

    if (Scrn_Adapter < VID_VGA)
      return(BIGFONT);

    gettextinfo(&Info);
    return(Info.currmode == C4350 ? SMALLFONT : BIGFONT);
  #endif
}

void LIBENTRY setfont(fonttype Font) {
  Scrn_BottomRow = 24;

  if (Scrn_Adapter < VID_VGA)
    return;

  #ifdef __OS2__
    switch (Font) {
      case BIGFONT  :  setviolines(25); Scrn_BottomRow = 24; break;
      case SMALLFONT:  setviolines(50); Scrn_BottomRow = 49; break;
    }
  #else
    textmode(Font == BIGFONT ? C80 : C4350);
    if (Font == SMALLFONT)
      Scrn_BottomRow = 49;

    if (Font == SMALLFONT && getfont() != SMALLFONT)
      setfont(BIGFONT);
  #endif

  Scrn_SizeBytes = (Scrn_BottomRow+1) * 160;
  Scrn_Size16 = (short) (Scrn_SizeBytes >> 1);
  Scrn_Size32 = Scrn_Size16 >> 1;
}

#else    /* the old method - for Turbo C 2.0 */

#include <conio.h>
#pragma inline

fonttype LIBENTRY getfont(void) {
  int num;

  if (Scrn_Adapter < VID_VGA)  /* || Scrn_Mode != VID_COLOR) */
    return(BIGFONT);

  _asm push bp
  _asm mov ax, 0x1130
  _asm xor bx, bx
  _asm xor cx, cx
  _asm int 0x10
  _asm pop bp
  _asm mov num, cx

  return(num == 8 ? SMALLFONT : BIGFONT);
}


void LIBENTRY setfont(fonttype Font) {
  int  num;
  char mode;

  if (Scrn_Adapter < VID_VGA)   /* || Scrn_Mode != VID_COLOR) */
    return;

  if (Font == BIGFONT) {
    num  = 0x1114;
    Scrn_BottomRow = 24;
  } else {
    num  = 0x1112;
    Scrn_BottomRow = 49;
  }

  mode = (Scrn_Mode == VID_COLOR ? 3 : 7);

  _asm  push bp
  _asm  mov al, mode
  _asm  mov ah, 0
  _asm  int 0x10
  _asm  pop bp

  _asm  push bp
  _asm  mov bl, 0
  _asm  mov ax, num
  _asm  int 0x10
  _asm  pop bp

  if (Font == SMALLFONT && getfont() != SMALLFONT)
    setfont(BIGFONT);

  Scrn_SizeBytes = (Scrn_BottomRow+1) * 160;
  Scrn_Size16 = (short) (Scrn_SizeBytes >> 1);
  Scrn_Size32 = Scrn_Size16 >> 1;
}
#endif  /* #if __TURBOC__ >= 0x0300 */


#ifdef TEST
#include <conio.h>
#include <stdio.h>

void LIBENTRY show(char *Str) {
  savescrntype Screen;
  int X;

  boxcls(0,0,79,Scrn_BottomRow,0x1F,SINGLE);

  for (X = 1; X < Scrn_BottomRow; X++)
    fastprint(40,X,"test",0x1A);

  fastprintmove(1,1,Str,0x1F);

  savescreen(&Screen);
  cls();
  getch();
  restorescreen(&Screen);
  getch();
}


void main(void) {
  char     Str[80];
  fonttype Font;

  getmode();
  Font = getfont();

  printf("detected %s font\n",getfont() == SMALLFONT ? "small" : "big");
  getch();

  setfont(SMALLFONT);
  printf("detected %s font\n",getfont() == SMALLFONT ? "small" : "big");
  getch();

  setfont(BIGFONT);
  printf("detected %s font\n",getfont() == SMALLFONT ? "small" : "big");
  getch();

  setfont(SMALLFONT);
  printf("detected %s font\n",getfont() == SMALLFONT ? "small" : "big");
  getch();

  setfont(BIGFONT);
  printf("detected %s font\n",getfont() == SMALLFONT ? "small" : "big");
  getch();

/*
  sprintf(Str,"Original font was %s",Font == SMALLFONT ? "small" : "big");
  show(Str);

  setfont(BIGFONT);
  show("after change to big font");

  setfont(SMALLFONT);
  show("after change to small font");

  setfont(Font);
  show("back to original font");
*/
}
#endif
