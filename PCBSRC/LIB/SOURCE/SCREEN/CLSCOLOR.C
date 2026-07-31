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
  #include "model.h"
#endif

#include "screen.h"

/********************************************************************
 *
 *  Function:  clscolor()
 *
 *  Clear the entire screen filling in the specified color attribute
 */

void LIBENTRY clscolor(char Color) {
#ifdef __OS2__
  int X;
  unsigned long *p;
  long Value;

  Value = 0x00200020 + (Color << 24) + (Color << 8);
  p = (unsigned long *) Scrn_Buf;
  for (X = 0; X < Scrn_Size32; X++, p++)
    *p = Value;

  updatelines(Scrn_BottomRow == 24 ? UPDATE_25LINES : UPDATE_50LINES,0,Scrn_BottomRow);

#else  /* ifdef __OS2__ */

#ifdef BIOS
  ansi_clearcolor(Color);
#else
  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

  asm  Cmp  Scrn_Rtrc,1
  asm  Jne  direct

bioscall:
  asm  Push  Bp             /* save BP because old PC's forget it */
  asm  Mov   Ax,0600h
  asm  Mov   Bh,Color          /* color attribute for entire screen */
  asm  Xor   Cx,Cx             /* top left corner (0,0) */
  asm  Mov   Dh,Scrn_BottomRow /* bottom line (0-based) */
  asm  Inc   Dh                /* bottom line (1-based) */
  asm  Mov   Dl,4Fh            /* right corner */
  asm  Int   10h
  asm  Pop   Bp
  asm  Jmp   short exit

direct:
  asm  Mov   Ax,80                /* calculate the number of words in the */
  asm  Mov   Bl,Scrn_BottomRow    /* video memory according to the number */
  asm  Inc   Bl                   /* of lines currently on the screen.    */
  asm  Mul   Bl
  asm  Mov   Cx,Ax                /* CX = number of WORDS in video memory */

  asm  Cld
  asm  Les   Di,dword ptr Scrn_Addr
  asm  Mov   Al,' '
  asm  Mov   Ah,Color
  asm  Rep   Stosw

exit:;
  NEEDSEGPOPDS;
#endif  /* ifdef BIOS */
#endif  /* ifdef __OS2__ */
}

