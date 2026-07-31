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


//#pragma inline

#include <mem.h>
#include <stdio.h>
#include <model.h>
#include "screen.h"

char ansicolors[] = "04261537";

void pascal ansi_print(char *str) {
/*  for (; *str; str++)  */
/*    putchar(*str);     */

#ifdef LDATA
  asm Push Ds
  asm Lds  Si,str
#else
  asm Mov  Si,str
#endif

  asm Mov  Ah,2
Top:
  asm Mov  Dl,[Si]
  asm Or   Dl,Dl
  asm Jz   End
  asm Int  21h
  asm Inc  Si
  asm Jmp  Short Top
End:

#ifdef LDATA
  asm Pop  Ds
#endif
;
}

void pascal ansi_save(void) {
  ansi_print("[s");
}

void pascal ansi_rest(void) {
  ansi_print("[u");
}

void pascal ansi_clear(void) {
  ansi_save();
  ansi_print("[2J");
  ansi_rest();
}

void pascal ansi_color(int Color) {
  static char LastBackGrnd;
  static char LastForeGrnd;
  static char Str[15];
  char CurrBackGrnd;
  char CurrForeGrnd;
  char *p;

  CurrBackGrnd = (Color >> 4) & 0x0F;
  CurrForeGrnd = Color & 0x0F;
  p            = Str;

  if (CurrBackGrnd == LastBackGrnd && CurrForeGrnd == LastForeGrnd)
    return;


  *p++ = '';
  *p++ = '[';

  if (CurrBackGrnd != LastBackGrnd || (LastForeGrnd>7 && CurrForeGrnd<=7)) {
    *p++ = '0';
    *p++ = ';';
    LastBackGrnd = 0;  /* "0" resets to black background */
    LastForeGrnd = 7;  /* "0" resets to white foreground */
  }

  if (CurrBackGrnd != LastBackGrnd && CurrBackGrnd != 0) {
    *p++ = '4';
    *p++ = ansicolors[CurrBackGrnd];
    *p++ = ';';
    LastBackGrnd = CurrBackGrnd;
  }

  if (CurrForeGrnd != LastForeGrnd) {
    if (CurrForeGrnd > 7) {
      *p++ = '1';
      *p++ = ';';
    }

    if (CurrForeGrnd == LastForeGrnd-8) {
      p--;
    } else {
      *p++ = '3';
      *p++ = ansicolors[CurrForeGrnd & 0x07];
    }
    LastForeGrnd = CurrForeGrnd;
  } else {
    if (*(p-1) == ';')
      p--;
  }

  *p++ = 'm';
  *p   = 0;
  ansi_print(Str);
}

void pascal ansi_clearbox(char X1, char Y1, char X2, char Y2, char Color) {
  char Str[81];
  int  Len;
  int  Row;

  Len = X2-X1+1;
  memset(Str,' ',Len);
  Str[Len] = 0;

  ansi_save();
  ansi_color(Color);
  for (Row = Y1; Row <= Y2; Row++) {
    if (Row == 24 && X2 == 79) {      /* avoid scrolling effect */
      Len--;
      Str[Len] = 0;
    }
    gotoxy(X1,Row);
    ansi_print(Str);
  }
  ansi_rest();
}

void pascal ansi_clearcolor(char Color) {
  ansi_save();
  ansi_color(Color);
  ansi_print("[2J");
  ansi_rest();
}

