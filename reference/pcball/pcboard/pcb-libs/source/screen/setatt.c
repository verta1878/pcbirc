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
 *  Function:  setatt()
 *
 *  Set all of the attributes of a specified rectangular field to one color
 */

void LIBENTRY setatt(int X1,int Y1,int X2,int Y2,char Color) {
#ifdef __OS2__
  int X;
  int Y;
  char *p;

  for (Y = Y1; Y <= Y2; Y++) {
    p = ((char *) Scrn_Buf) + (Y*160) + X1*2 + 1;
    for (X = X1; X <= X2; X++, p += 2)
      *p = Color;
  }

  updatelines(UPDATE_MIXED,Y1,Y2);

#else  /* ifdef __OS2__ */

  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

#ifdef BIOS
  ansi_color(Color);
  gotoxy(X1,Y1);

  asm  Les  Si,dword ptr Scrn_Addr

  asm  Mov  Bx,X1
  asm  Sub  X2,Bx
  asm  Inc  word ptr X2
  asm  Mov  Cx,Y2
  asm  Mov  Ax,Y1
  asm  Sub  Cx,Ax
  asm  Inc  Cx
  asm  Mov  Bl,160
  asm  Mul  Bl
  asm  Inc  Ax
  asm  Add  Si,Ax

NextY:

ForX:
  asm  Push Cx
  asm  Mov  Cx,X2
  asm  Mov  Di,X1
  asm  Shl  Di,1
  asm  Add  Di,Si
  asm  Dec  Di

  if (Y1 == 24 && X2 == 80)
    asm Dec Cx

NextX:
  asm  Cmp  Scrn_Rtrc,0
  asm  Je   J2
  asm  Mov  Dx,03DAh

J1:
  asm  In   Al,Dx
  asm  Test Al,01
  asm  Jz   J1

J2:
  asm  Mov  Ah,2
  asm  Mov  Dl,Es:[Di]
  asm  Int  21h
  asm  Inc  Di
  asm  Inc  Di
  asm  Loop NextX

  asm  Pop  Cx
  asm  Add  Si,160
  Y1++;
  gotoxy(X1,Y1);
  asm  Loop NextY

#else  /* ifdef BIOS */

  /***********  non-BIOS mode  ***************/

  asm  Cld
  asm  Les  Si,dword ptr Scrn_Addr

  asm  Mov  Bx,X1
  asm  Sub  X2,Bx
  asm  Inc  word ptr X2
  asm  Mov  Cx,Y2
  asm  Mov  Ax,Y1
  asm  Sub  Cx,Ax
  asm  Inc  Cx
  asm  Mov  Bl,160
  asm  Mul  Bl
  asm  Inc  Ax
  asm  Add  Si,Ax

NextY:

ForX:
  asm  Push Cx
  asm  Mov  Cx,X2
  asm  Mov  Di,X1
  asm  Shl  Di,1
  asm  Add  Di,Si


NextX:
  asm  Cmp  Scrn_Rtrc,0
  asm  Je   J2
  asm  Mov  Dx,03DAh

J1:
  asm  In   Al,Dx
  asm  Test Al,01
  asm  Jz   J1

J2:
  asm  Mov  Al,Color
  asm  Stosb
  asm  Inc  Di
  asm  Loop NextX

  asm  Pop  Cx
  asm  Add  Si,160
  asm  Loop NextY
#endif  /* ifdef BIOS */

  NEEDSEGPOPDS;

#endif  /* ifdef __OS2__ */
}
