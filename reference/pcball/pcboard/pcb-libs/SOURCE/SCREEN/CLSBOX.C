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
#ifdef BIOS
  #include "ansi.h"
#endif

#ifndef __OS2__
  #include <model.h>
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif


/********************************************************************
 *
 *  Function:  clsbox()
 *
 *  Clear a rectangular section of the screen setting color attributes at
 *  the same time
 */


void LIBENTRY clsbox(char X1, char Y1, char X2, char Y2, char Color) {
#ifdef __OS2__

  int X;
  int Y;
  scrnupdttype ScrnUpdate;
  unsigned short *p;
  celltype Cell;

  Cell.Byte.Value = ' ';
  Cell.Byte.Attrib = Color;

  for (Y = Y1; Y <= Y2; Y++) {
    p = ((unsigned short *) Scrn_Buf) + (Y*80) + X1;
    for (X = X1; X <= X2; X++, p++)
      *p = Cell.Word;
  }

  ScrnUpdate = UPDATE_MIXED;
  if (Y1 == 0) {
    if (Y2 == 24)
      ScrnUpdate = UPDATE_25LINES;
    else if (Y2 == 49)
      ScrnUpdate = UPDATE_50LINES;
  }

  updatelines(ScrnUpdate,Y1,Y2);

#else /* ifdef __OS2 */

#ifdef BIOS
  ansi_clearbox(X1,Y1,X2,Y2,Color);
#else /* ifdef BIOS */
  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

  asm  Cmp  Scrn_Rtrc,1
  asm  Jne  direct

bioscall:
  asm  Push Bp              /* save BP because old PC's forget it */
  asm  Mov  Ax,0600h
  asm  Mov  Bh,Color        /* set attribute to desired color */
  asm  Mov  Ch,Y1
  asm  Mov  Cl,X1
  asm  Mov  Dh,Y2
  asm  Mov  Dl,X2
  asm  Int  10h
  asm  Pop  Bp
  asm  Jmp   short exit

direct:
  asm  Cld
  asm  Les   Di,dword ptr Scrn_Addr
  asm  Xor   Ax,Ax
  asm  Mov   Bx,Ax
  asm  Mov   Dl,Y2            /*                                              */
  asm  Mov   Al,Y1            /*                                              */
  asm  Sub   Dl,Al            /*                                              */
  asm  Inc   Dl               /* Dl = number of lines (Y2-Y1+1)               */
  asm  Mov   Bl,80            /*                                              */
  asm  Mul   Bl               /*                                              */
  asm  Add   Al,X1            /*                                              */
  asm  Adc   Ah,0             /*                                              */
  asm  Shl   Ax,1             /*                                              */
  asm  Add   Di,Ax            /* Di = pointer to Y1*160 + X1*2                */
  asm  Mov   Al,' '           /* we'll be printing blanks                     */
  asm  Mov   Ah,Color         /* on the specified color                       */
  asm  Mov   Bl,X2            /*                                              */
  asm  Sub   Bl,X1            /*                                              */
  asm  Inc   Bl               /* Bl = X2-X1+1 = number of words to fill       */

  asm  Cmp   Bl,80            /* are we doing a full length line?             */
  asm  Jne   partline         /*   no, use the loop method                    */

fullline:
  asm  Xchg  Ax,Dx            /* save ax in dx while putting dx in ax         */
  asm  Mul   Bl               /*                                              */
  asm  Mov   Cx,Ax            /* Cx = 80*Dl = number of words to move         */
  asm  Mov   Ax,Dx            /* restore the ax register                      */
  asm  Rep   Stosw            /* move all of the bytes in one fell swoop      */
  asm  Jmp   short exit

partline:
  asm  Mov  Si,160
  asm  Sub  Si,Bx
  asm  Sub  Si,Bx
pl_loop:
  asm  Mov  Cx,Bx
  asm  Rep  Stosw
  asm  Add  Di,Si
  asm  Dec  Dl
  asm  Jnz  pl_loop

exit:;
  NEEDSEGPOPDS;
#endif  /* ifdef BIOS */
#endif  /* ifdef __OS2__ */
}
