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

#ifndef __OS2__
  #ifdef BIOS
    #include "ansi.h"
  #endif
  #include <model.h>
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif

/********************************************************************
 *
 *  Function:  fastprint()
 *
 *  Print a string of characters on the screen at a given location with a
 *  specified attribute for the entire string
 */

void LIBENTRY fastprint(int X, int Y, char *StrPtr, char Attr) {

#ifdef __OS2__
  char *p = (char *) Scrn_Buf;

  for (p += (Y*160 + X*2); *StrPtr != 0; StrPtr++) {
    *p = *StrPtr;
    p++;
    *p = Attr;
    p++;
  }

  updatelines(UPDATE_MIXED,Y,Y);

#else  /* ifdef __OS2__ */

  int  Len;

  if ((Len = strlen(StrPtr)) == 0)
    return;

#ifdef BIOS
   ansi_save();
   gotoxy(X,Y);
   ansi_color(Attr);
   ansi_print(StrPtr);
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
      asm  Mov  Al,Scrn_Rtrc

  #ifdef LDATA
      asm  Lds  Si,StrPtr
  #else
      asm  Mov  Si,StrPtr
  #endif

      asm  Or   Al,Al
      asm  Jz   Loop2

      asm  Mov  Dx,03DAh       /* Store port addr in Dx */
  Loop1:
      asm  In   Al,Dx          /*   get raster status                        */
      asm  Test Al,01          /*   if not 1                                 */
      asm  Jz   Loop1          /*     then Loop1                             */

      asm  Lodsb               /* Al = next char of string         (Si=Si+1) */
      asm  Stosw               /* Store Char+Attrib in scrn buffer (Di=Di+2) */
      asm  Loop Loop1          /* Loop if len not zero                       */
      asm  Jmp  short End      /*   else jump to end                         */

  Loop2:
      asm  Lodsb               /* Al = next char of string         (Si=Si+1) */
      asm  Stosw               /* Store Char+Attrib in scrn buffer (Di=Di+2) */
      asm  Loop Loop2          /* Loop if len not zero                       */

  End:;
  #ifdef LDATA
      asm  Pop  Ds
  #endif
#endif  /* ifdef BIOS */
#endif  /* ifdef __OS2__ */
}


#ifndef LIB
void LIBENTRY showscreen(ScrType *p, int NumLines) {
  for (; NumLines; NumLines--, p++)
    fastprint(p->X,p->Y,p->Str,p->Color);
}
#endif
