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
#include <string.h>

#ifndef __OS2__
  #include <model.h>
  #ifdef BIOS
    #include "ansi.h"
  #endif
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif


/********************************************************************
 *
 *  Function:  fastprintv()
 *
 *  Print a string of characters on the screen at a given location with a
 *  specified attribute for the entire string printing vertically instead of
 *  horizontally
 */

void LIBENTRY fastprintv(int X, int Y, char *Str, char Attr) {
#ifdef __OS2__
  int   Count;
  char *p = (char *) Scrn_Buf;

  for (p += (Y*160 + X*2), Count = 0; *Str != 0; Str++, Count++) {
    *p = *Str;
    p++;
    *p = Attr;
    p += 159;   // 160 bytes per line (159 plus the p++ up above)
  }

  updatelines(UPDATE_MIXED,Y,Y+Count-1);

#else  /* ifdef __OS2__ */

  int  Len;

  Len = strlen(Str);
  if (Len) {
#ifdef BIOS
  ansi_save();
  ansi_color(Attr);
  while ((_DL = *Str) != 0) {
    asm Push Dx
    gotoxy(X,Y);
    asm Pop  Dx
    asm Mov  Ah,2
    asm Int  21h
    Str++;
    Y++;
  }
  ansi_rest();

#else

  #ifdef LDATA
      asm  Push Ds
      NEEDSEGGETDS(Scrn_Addr);
  #endif
      asm  Les  Di,dword ptr Scrn_Addr
      asm  Mov  Ax,Y
      asm  Mov  Bl,80
      asm  Mul  Bl             /* Multiply Row*80        */
      asm  Add  Ax,X           /* Store Column+Row in Ax */
      asm  Shl  Ax,1           /* Multiply by 2          */
      asm  Add  Di,Ax          /* Point Di to Row;Column */

      asm  Mov  Ah,Attr

      asm  Mov  Cx,Len
      asm  Cmp  Scrn_Rtrc,0
  #ifdef LDATA
      asm  Lds  Si,Str
  #else
      asm  Mov  Si,Str
  #endif
      asm  Je   Loop2

      asm  Mov  Dx,03DAh       /* Store port addr in Dx */
  Loop1:
      asm  In   Al,Dx          /*   get raster status                        */
      asm  Test Al,01          /*   if not 1                                 */
      asm  Jz   Loop1          /*     then Loop1                             */
      asm  Lodsb               /* Al = next char of string         (Si=Si+1) */
      asm  Mov  Es:[Di],Ax     /* Store Char+Attrib in scrn buffer           */
      asm  Add  Di,160         /* next line                                  */
      asm  Loop Loop1          /* Loop if len not zero                       */
      asm  Jmp  short End      /*   else jump to end                         */

  Loop2:
      asm  Lodsb               /* Al = next char of string         (Si=Si+1) */
      asm  Mov  Es:[Di],Ax     /* Store Char+Attrib in scrn buffer           */
      asm  Add  Di,160         /* next line                                  */
      asm  Loop Loop2          /* Loop if len not zero                       */

  End:;
  #ifdef LDATA
      asm  Pop  Ds
  #endif
#endif /* ifdef BIOS */
  }
#endif /* ifdef __OS2__ */
}
