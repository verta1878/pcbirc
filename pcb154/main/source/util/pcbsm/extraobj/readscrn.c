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


#ifndef __OS2__
  #ifdef _MSC_VER
    #include <borland.h>
  #else
//  #pragma inline
  #endif
  #ifdef BIOS
    #include "ansi.h"
  #endif
  #include <model.h>
#endif

#include "screen.h"


/********************************************************************
 *
 *  Function:  readscreen()
 *
 *  Read a field from the screen into a variable changing the attribute bytes
 *  at the same time
 */

void LIBENTRY readscreen(int X, int Y, char Str[80], char Attr, int Len) {
#ifdef __OS2__
  char *p = (char *) Scrn_Buf;

  for (p += (Y*160 + X*2); Len != 0; Len--, Str++) {
    *Str = *p;
    p++;
    *p = Attr;
    p++;
  }
  *Str = 0;

  updatelines(UPDATE_MIXED,Y,Y);

#else  /* ifdef __OS2__ */
  asm  Push Ds

#ifdef SDATA
  asm  Mov  Ax,Ds
  asm  Mov  Es,Ax
  asm  Mov  Di,Str         /* Es:Di points to Str    */
#else
  asm  Les  Di,Str         /* Es:Di points to Str    */
  NEEDSEGGETDS(Scrn_Addr);
#endif

  asm  Cld
  asm  Mov  Bh,Scrn_Rtrc
  asm  Lds  Si,dword ptr Scrn_Addr
  asm  Mov  Ax,Y
  asm  Mov  Bl,80
  asm  Mul  Bl             /* Multiply Row*80        */
  asm  Add  Ax,X           /* Store Column+Row in Ax */
  asm  Shl  Ax,1           /* Multiply by 2          */
  asm  Add  Si,Ax          /* Point Si to Row;Column */

  asm  Mov  Al,Bh
  asm  Mov  Bl,Attr
  asm  Mov  Cx,Len

  asm  And  Al,Al
  asm  Jz   J2
  asm  Mov  Dx,03DAh

J1:
  asm  In   Al,Dx
  asm  Test Al,01
  asm  Jz   J1
  asm  Movsb
#ifndef BIOS
  asm  Mov  [Si],Bl
#endif
  asm  Inc  Si
  asm  Loop J1
  asm  Jmp  short J3

J2:
  asm  Movsb
#ifndef BIOS
  asm  Mov  [Si],Bl
#endif
  asm  Inc  Si
  asm  Loop J2

J3:
  asm  Xor  Al,Al
  asm  Stosb
  asm  Pop  Ds

#ifdef BIOS
  ansi_save();
  gotoxy(X,Y);
  ansi_color(Attr);
  ansi_print(Str);
  ansi_rest();
#endif  /* ifdef BIOS */
#endif  /* ifdef __OS2__ */
}
