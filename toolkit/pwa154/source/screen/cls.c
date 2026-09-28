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
  #include "model.h"
#endif /* ifndef __OS2__ */

#include "screen.h"
#ifdef BIOS
  #include "ansi.h"
#endif

/********************************************************************
 *
 *  Function:  cls()
 *
 *  Clear the entire screen
 */

void LIBENTRY cls(void) {
  #ifdef __OS2__
    int X;
    unsigned long *p;
    p = (unsigned long *) Scrn_Buf;
    for (X = 0; X < Scrn_Size32; X++, p++)
      *p = 0x07200720;

    updatelines(Scrn_BottomRow == 24 ? UPDATE_25LINES : UPDATE_50LINES,0,Scrn_BottomRow);
  #else
    #ifdef BIOS
      ansi_color(0x07);
      ansi_clear();
    #else  /* ifdef BIOS */
      NEEDSEGPUSHDS;
      NEEDSEGGETDS(Scrn_Addr);

      asm  Cmp  Scrn_Rtrc,1
      asm  Jne  direct

      bioscall:
      asm  Push  Bp                /* save BP because old PC's forget it */
      asm  Mov   Ax,0600h
      asm  Mov   Bh,7              /* attribute of lightgray on black */
      asm  Xor   Cx,Cx             /* top left corner (0,0) */
      asm  Mov   Dh,Scrn_BottomRow /* bottom line (0-based) */
      asm  Inc   Dh                /* bottom line (1-based) */
      asm  Mov   Dl,4Fh            /* right corner */
      asm  Int   10h
      asm  Pop   Bp
      asm  Jmp   short exit

      direct:
      asm  Cld
      asm  Mov   Cx,Scrn_Size16  /* CX = number of WORDS in video memory */
      asm  Les   Di,dword ptr Scrn_Addr
      asm  Mov   Ax,0720h        /* spaces with lightgray on black background */
      asm  Rep   Stosw

      exit:;
      NEEDSEGPOPDS;
    #endif  /* ifdef BIOS */
  #endif  /* ifdef __OS2__ */
}
