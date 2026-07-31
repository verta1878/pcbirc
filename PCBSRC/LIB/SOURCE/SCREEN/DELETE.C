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
 *  Function:  deletechar()
 *
 *  Delete a character from a FIELD moving all characters to the right of it
 *  left one character
 */

void LIBENTRY deletechar(int X, int Y, char C, int Len) {
#ifdef __OS2__
  char *p = (char *) Scrn_Buf;

  for (p += (Y*160 + X*2); Len != 0; Len--, p += 2)
    *p = *(p+2);
  *p = C;

  updatelines(UPDATE_MIXED,Y,Y);

#else
  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

  if (Len) {
#ifdef BIOS
    asm  Les  Di,dword ptr Scrn_Addr
    asm  Mov  Ax,Y
    asm  Mov  Bl,80
    asm  Mul  Bl             /* Multiply Row*80          */
    asm  Add  Ax,X           /* Store Column+Row in Ax   */
    asm  Shl  Ax,1           /* Multiply by 2            */
    asm  Add  Di,Ax          /* Point Di to Row;Column   */

    asm  Xor  Ah,Ah
    asm  Mov  Al,Es:[Di+1]

    asm  Push Es
    ansi_color(_AX);
    ansi_save();
    asm  Pop  Es

    asm  Mov  Ah,2
    asm  Mov  Cx,Len
    asm  Dec  Cx
    asm  Jz   End
Top:
    asm  Mov  Dl,Es:[Di+2]
    asm  Int  21h
    asm  Inc  Di
    asm  Inc  Di
    asm  Loop Top

End:
    asm  Mov  Dl, C
    asm  Int  21h

    ansi_rest();

#else  /* ifdef BIOS */

    asm  Mov  Ax,Y
    asm  Mov  Bl,80
    asm  Mul  Bl             /* Multiply Row*80          */
    asm  Add  Ax,X           /* Store Column+Row in Ax   */
    asm  Shl  Ax,1           /* Multiply by 2            */
    asm  Mov  Di,Ax          /* Point Di to Row;Column   */
    asm  Add  Di,word ptr [Scrn_Addr]
    asm  Mov  Ax,Di          /* Store Location in Ax & Si*/
    asm  Mov  Si,Di          /*                          */

    asm  Mov  Cx,Len         /* Cx = Len of field        */
    asm  Dec  Cx             /* Cx = Num Bytes to Move   */

    asm  Inc  Si             /* Point Si to Ax + 2       */
    asm  Inc  Si             /*                          */
    asm  Add  Ax,Cx          /*                          */
    asm  Add  Ax,Cx          /* Ax=Row; Column+(Len * 2) */
    asm  Mov  Bx,Ax          /* Save Row; Column in Bx   */

    asm  Mov  Al,Scrn_Rtrc
    asm  Push Ds
    asm  Mov  Es,word ptr [Scrn_Addr+2]
    asm  Mov  Ds,word ptr [Scrn_Addr+2]

    asm  Or   Cx,Cx          /* Are there bytes to move  */
    asm  Jz   End            /* Skip if NO bytes to move */

    asm  Cld
    asm  Or   Al,Al
    asm  Jz   Loop2

    asm  Mov  Dx,03DAh       /* Store port addr in Dx */
Loop1:
    asm  In   Al,Dx          /* get raster status     */
    asm  Test Al,01          /* if not 1              */
    asm  Jz   Loop1          /*   then Loop1          */
    asm  Movsb               /* Move character        */
    asm  Inc  Di             /* Increment pointers    */
    asm  Inc  Si             /*                       */
    asm  Loop Loop1          /* Loop if len not zero  */
    asm  Jmp  short End      /*   else jump to end    */

Loop2:
    asm  Movsb               /* Move character        */
    asm  Inc  Di             /* Increment pointers    */
    asm  Inc  Si             /*                       */
    asm  Loop Loop2          /* Loop if len not zero  */

End:
    asm  Pop  Ds
    asm  Mov  Si,Bx
    asm  Mov  Al,C
    asm  Mov  Es:[Si],Al
#endif /* ifdef BIOS */
  }

  NEEDSEGPOPDS;
#endif  /* ifdef __OS2__ */
}
