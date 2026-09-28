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
/*  #pragma inline */
  #endif
  #ifdef BIOS
    #include "ansi.h"
  #endif
  #include "model.h"
#endif

#include "screen.h"

/********************************************************************
 *
 *  Function:  readscreen2()
 *
 *  Read a field with intermixed characters and attributes
 */

void LIBENTRY readscreen2(int X, int Y, char *Str, int Len) {
#ifdef __OS2__
  short *p = (short *) Scrn_Buf;
  short *q = (short *) Str;

  Str[Len*2] = 0;
  for (p += (Y*80 + X); Len != 0; Len--, p++, q++)
    *q = *p;

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

  asm  Lds  Si,dword ptr Scrn_Addr
  asm  Mov  Ax,Y
  asm  Mov  Bl,80
  asm  Mul  Bl             /* Multiply Row*80        */
  asm  Add  Ax,X           /* Store Column+Row in Ax */
  asm  Shl  Ax,1           /* Multiply by 2          */
  asm  Add  Si,Ax          /* Point Si to Row;Column */

  asm  Cld
  asm  Mov  Cx,Len
  asm  Rep  Movsw          /* Move Len*2 number of bytes from screen to Str */

  asm  Xor  Al,Al          /* Mark the end of the string */
  asm  Stosb
  asm  Pop  Ds
#endif /* ifdef __OS2__ */
}
