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
#include <stdio.h>

void main(void) {
  int CurType, CurX, CurY;
  fonttype Font;
  char Date[9];
  char Time1[9];
  char Time2[6];
  char Str[40];
  savescrntype Screen;

  printf("hello world\n");
  getmode();
  Font = getfont();
  CurType = getcursor();
  CurX = wherex();
  CurY = wherey();
  datestr(Date);
  timestr1(Time1);
  timestr2(Time2);

  beep();
  gotoxy(1,1);
  setcursor(CUR_BLANK);
  setfont(SMALLFONT);
  cls();
  mydelay(100);
  setcursor(CUR_BLOCK);
  clscolor(0x4F);
  mydelay(150);
  setcursor(CUR_INSERT);
  box(0,0,79,24,0x07,DOUBLE);
  updatelinesnow();

  mydelay(150);
  setcursor(CUR_NORMAL);
  box(5,5,74,19,0x0C,SINGLE);
  mydelay(150);
  clsbox(2,2,10,5,0x2F);
  mydelay(150);
  growbox(0,0,79,24,0x1F,VDOUBLE,1);
  mydelay(150);
  gotoxy(CurX,CurY);
  setcursor(CurType);
  fastprint(1,23,"THIS IS A TEST",0x2F);
  fastprintv(78,5,"THIS IS A TEST",0x2F);
  fastcenter(12,"THIS IS A TEST",0x2F);
  fastprint(1,1,Date,0x2F);
  fastcenter(1,Time2,0x2F);
  fastprint(71,1,Time1,0x2F);
  fastprint(20,5,"REMOVE THE 'R' FROM REMOVE",0x2F);
  fastprintmove(20,6,"INSERT ' ' BEFORE INSERT",0x2F);
  deletechar(20,5,' ',26);
  insertchar(20,6,' ',24);
  fastputc(360,'T');
  fastputch(362,'X',0x2F);
  fastprint(30,20,"READ THIS FIELD",0x2F);
  mydelay(150);
  readscreen(30,20,Str,0x4F,16);
  fastprint(30,21,Str,0x2F);
  readscreen2(0,0,Str,10);
  fastprint(30,22,Str,0x2F);
  savescreen(&Screen);
  mydelay(150);

  setfont(BIGFONT);
  cls();
  mydelay(100);
  restorescreen(&Screen);
  mydelay(100);

  cls();
  fastprint(2, 3,"1234567890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ",0x1F);
  fastprint(2, 4,"234567890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1",0x1F);
  fastprint(2, 5,"34567890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ12",0x1F);
  fastprint(2, 6,"4567890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ123",0x1F);
  fastprint(2, 7,"567890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",0x1F);
  fastprint(2, 8,"67890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ12345",0x1F);
  fastprint(2, 9,"7890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ123456",0x1F);
  fastprint(2,10,"890abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567",0x1F);
  fastprint(2,11,"90abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678",0x1F);
  fastprint(2,12,"0abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789",0x1F);
  fastprint(2,13,"abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890",0x1F);
  updatelinesnow();
  mydelay(50);
  scrolldn(10,6,15,10,0x1F);
  updatelinesnow();
  mydelay(50);
  scrolldn(10,6,15,10,0x1F);
  updatelinesnow();
  mydelay(150);
  scrollup(10,6,15,10,0x1F);
  updatelinesnow();
  mydelay(50);
  scrollup(10,6,15,10,0x1F);
  updatelinesnow();
  mydelay(100);
  setatt(10,6,15,10,0x4F);
  updatelinesnow();
  mydelay(150);
  setfont(Font);
}
