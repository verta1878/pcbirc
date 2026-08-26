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
#include "screen.h"

#ifdef __OS2__
  #include <stdio.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
/*  #pragma  inline */
    #include "model.h"
  #endif
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif
char * LIBENTRY dtoc(int Date, char *Temp) {
  int  Y;
  char M;
  char D;

  if (Date == -1) {
    strcpy(Temp,"OFF-LINE");
    return(Temp);
  }

  if (Date == -2) {
    strcpy(Temp,"DELETED ");
    return(Temp);
  }

  D = (char)  (Date & 0x001F);
  M = (char) ((Date & 0x01E0) >> 5);
  Y =        ((Date & 0xFE00) >> 9) + 80;

  if (Y >= 100)
    Y -= 100;

#ifdef __OS2__

  sprintf(Temp,"%02d%c%02d%c%02d",M,Scrn_DateSeparator,D,Scrn_DateSeparator,Y);

#else

#ifdef LDATA
  asm Les Si,Temp
#else
  asm Mov Ax,Ds
  asm Mov Es,Ax
  asm Mov Si,Temp
#endif
  asm Mov  Al,M             /* month */
  asm Aam
  asm Or   Ax,3030h
  asm Xchg Al,Ah
  asm Mov  Es:[Si],Ax

  asm Mov  Al,D             /* day */
  asm Aam
  asm Or   Ax,3030h
  asm Xchg Al,Ah
  asm Mov  Es:[Si+3],Ax

  asm Mov  Ax,Y             /* year */
  asm Aam
  asm Or   Ax,3030h
  asm Xchg Al,Ah
  asm Mov  Es:[Si+6],Ax

  asm Mov  Bl,Scrn_DateSeparator;
  asm Mov  Es:[Si+2],Bl
  asm Mov  Es:[Si+5],Bl
  asm Mov  byte ptr Es:[Si+8],0
#endif

  return(Temp);
}
