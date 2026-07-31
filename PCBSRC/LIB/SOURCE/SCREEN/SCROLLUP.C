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


#ifdef __OS2__
  #include <inline.h>
#else
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

#include <mem.h>
#include <stdio.h>
#include "screen.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

/********************************************************************
 *
 *  Function:  scrollup()
 *
 *  Scroll a specified rectangular area up one line with a default color
 */

void LIBENTRY scrollup(int X1, int Y1, int X2, int Y2, char Color) {
#ifdef __OS2__
  short *p = (short *) Scrn_Buf;
  int    Len;
  int    Y;
  short  Value;
  scrnupdttype ScrnUpdate;

  Len = (X2 - X1 + 1) << 1;
  Value = (short) (((short) Color << 8) + (short) ' ');

  for (Y = Y1, p += (Y1*80 + X1); Y < Y2; Y++, p += 80)
    memcpy(p,p+80,Len);

  for (; Len > 0; Len -= 2, p++)
    *p = Value;

  ScrnUpdate = UPDATE_MIXED;
  if (Y1 == 0) {
    if (Y2 == 24)
      ScrnUpdate = UPDATE_25LINES;
    else if (Y2 == 49)
      ScrnUpdate = UPDATE_50LINES;
  }

  updatelines(ScrnUpdate,Y1,Y2);

#else  /* ifdef __OS2__ */

#ifdef BIOS
  char Str[162];
  int  Len;
  char *p;

  ansi_save();
  Len = X2-X1+1;

  for (; Y1 < Y2; Y1++) {
    readscreen2(X1,Y1+1,Str,Len);
    gotoxy(X1,Y1);
    for (p = Str; *p; p+=2) {
      ansi_color(*(p+1));
      _DL = *p;
      asm Mov Ah,2
      asm Int 21h
    }
  }

  memset(Str,' ',Len);
  Str[Len] = 0;
  gotoxy(X1,Y2);
  ansi_color(Color);
  ansi_print(Str);
  ansi_rest();

#else  /* ifdef BIOS */

  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

  asm  Cmp  Scrn_Rtrc,1
  asm  Jne  direct

bioscall:
  asm  Push Bp             /* save BP because old PC's forget it */
  asm  Mov  Ax,0601h
  asm  Mov  Bh,Color
  asm  Mov  Ch,Y1
  asm  Mov  Cl,X1
  asm  Mov  Dh,Y2
  asm  Mov  Dl,X2
  asm  Int  10h
  asm  Pop  Bp
  asm  Jmp  short exit

direct:
  asm  Push Ds
  asm  Les  Di,dword ptr Scrn_Addr
  asm  Mov  Ax,Es
  asm  Mov  Ds,Ax

  asm  Cld
  asm  Xor  Ax,Ax
  asm  Mov  Bx,Ax
  asm  Mov  Dl,Y2            /*                                              */
  asm  Mov  Al,Y1            /*                                              */
  asm  Sub  Dl,Al            /* Dl = number of lines (Y2-Y1)                 */
  asm  Mov  Bl,80            /*                                              */
  asm  Mul  Bl               /*                                              */
  asm  Add  Al,X1            /*                                              */
  asm  Adc  Ah,0             /*                                              */
  asm  Shl  Ax,1             /*                                              */
  asm  Add  Di,Ax            /* Di = pointer to Y1*160 + X1*2                */
  asm  Mov  Si,Di            /*                                              */
  asm  Add  Si,160           /* Si = pointer to (Y1+1)*160 + X1*2            */
  asm  Mov  Bl,X2            /*                                              */
  asm  Sub  Bl,X1            /*                                              */
  asm  Inc  Bl               /* Bl = X2-X1+1 = number of words to move       */

  asm  Cmp  Bl,80            /* are we doing a full length line?             */
  asm  Jne  partline         /*   no, use the loop method                    */

fullline:
  asm  Mov  Al,Dl            /*                                              */
  asm  Mul  Bl               /*                                              */
  asm  Mov  Cx,Ax            /* Cx = 80*Dl = number of words to move         */
  asm  Rep  Movsw            /* move all of the bytes in one fell swoop      */
  asm  Jmp  short blankline  /*                                              */

partline:
  asm  Mov  Ax,160
  asm  Sub  Ax,Bx
  asm  Sub  Ax,Bx
loop:
  asm  Mov  Cx,Bx
  asm  Rep  Movsw
  asm  Add  Di,Ax
  asm  Add  Si,Ax
  asm  Dec  Dl
  asm  Jnz  loop

blankline:
  asm  Mov  Al,' '
  asm  Mov  Ah,Color
  asm  Mov  Cx,Bx
  asm  Rep  Stosw

  asm  Pop  Ds

exit:;
  NEEDSEGPOPDS;
#endif  /* ifdef BIOS */
#endif  /* ifdef __OS2__ */
}
