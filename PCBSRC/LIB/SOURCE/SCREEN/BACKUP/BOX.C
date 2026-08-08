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

#if defined(__OS2__) || defined(__WATCOMC__)
  enum {LN_VERT = 0,LN_HOR,LN_TOPLEFT,LN_TOPRIGHT,LN_BOTLEFT,LN_BOTRIGHT};

  static char Boxes[VDOUBLE+1][LN_BOTRIGHT+1] = {
    { 32, 32, 32, 32, 32, 32},  /* Blank   */
    {179,196,218,191,192,217},  /* Single  */
    {186,205,201,187,200,188},  /* Double  */
    {179,205,213,184,212,190},  /* HDouble */
    {186,196,214,183,211,189},  /* VDouble */
  };

#else  /* ifdef __OS2__ */

#include <model.h>

  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif  /* ifdef __OS2__ */


/********************************************************************
 *
 *  Function:  box()
 *
 *  Draw a box on the screen with a specified color and line type
 */

void LIBENTRY box(int X1,int Y1,int X2,int Y2,char Color,boxlinetype LineType) {
#if defined(__OS2__) || defined(__WATCOMC__)
  int      T;
  int      Y1x80;
  int      Y2x80;
  scrnupdttype ScrnUpdate;
  celltype Cell;
  unsigned short *p;
  char      *q;

  if (! Scrn_Box)
    return;

  Cell.Byte.Attrib = Color;

  q = Boxes[LineType];
  p = (unsigned short *) Scrn_Buf;

  Y1x80 = Y1*80;
  Y2x80 = Y2*80;

  Cell.Byte.Value = q[LN_TOPLEFT];  p[Y1x80 + X1] = Cell.Word;
  Cell.Byte.Value = q[LN_TOPRIGHT]; p[Y1x80 + X2] = Cell.Word;
  Cell.Byte.Value = q[LN_BOTLEFT];  p[Y2x80 + X1] = Cell.Word;
  Cell.Byte.Value = q[LN_BOTRIGHT]; p[Y2x80 + X2] = Cell.Word;

  Cell.Byte.Value = q[LN_HOR];
  for (T = X1+1; T <= X2-1; T++) {
    p[Y1x80+T] = Cell.Word;
    p[Y2x80+T] = Cell.Word;
  }

  Cell.Byte.Value = q[LN_VERT];
  for (T = Y1x80+80; T <= Y2x80-80; T += 80) {
    p[T+X1] = Cell.Word;
    p[T+X2] = Cell.Word;
  }

  ScrnUpdate = UPDATE_MIXED;
  if (Y1 == 0) {
    if (Y2 == 24)
      ScrnUpdate = UPDATE_25LINES;
    else if (Y2 == 49)
      ScrnUpdate = UPDATE_50LINES;
  }

  updatelines(ScrnUpdate,Y1,Y2);

#else /* ifdef __OS2__ */


#ifdef BIOS
  int Xpos;
  int Ypos;
#endif

  if (! Scrn_Box)
    return;

  asm  Jmp short Start

  asm  Boxes    db   32, 32, 32, 32, 32, 32   /* Blank   */
  asm           db  179,196,218,191,192,217   /* Single  */
  asm           db  186,205,201,187,200,188   /* Double  */
  asm           db  179,205,213,184,212,190   /* HDouble */
  asm           db  186,196,214,183,211,189   /* VDouble */

/* ********************* */

  asm  Loc:
  asm  Mov  Cx,160
  asm  Mul  Cx
  asm  Shl  Di,1
  asm  Add  Di,Ax
  asm  Add  Di,word ptr [Scrn_Addr]
  asm  RetN

/* ********************* */

  asm  PrintChar:
#ifdef BIOS
  asm  Mov  Ah,2
  asm  Mov  Dl,Bl
  asm  Int  21h
#else
  asm  Cmp  Scrn_Rtrc,0
  asm  Je   J2
  asm  Mov  Dx,03DAh
J1:
  asm  In   Al,Dx
  asm  Test Al,01
  asm  Jz   J1
J2:
  asm  Mov  Es:[Di],Bx
#endif
  asm  RetN

/* ********************* */

Start:
  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

#ifdef BIOS
  ansi_save();
  ansi_color(Color);
#else
  asm  Mov  Es,word ptr [Scrn_Addr+2]
  asm  Mov  Bh,Color
#endif

  asm  Mov  Ax,6
  asm  Mov  Cl,LineType
  asm  Mul  Cl
  asm  Mov  Si,offset Boxes
  asm  Add  Si,Ax

#ifdef BIOS
  gotoxy(X1,Y1);
#else
  asm  Mov  Ax,Y1            /* Top Left */
  asm  Mov  Di,X1
  asm  Call Loc
#endif
  asm  Mov  Bl,Cs:[Si+2]
  asm  Call PrintChar

#ifdef BIOS
  gotoxy(X2,Y1);
#else
  asm  Mov  Ax,Y1            /* Top Right */
  asm  Mov  Di,X2
  asm  Call Loc
#endif
  asm  Mov  Bl,Cs:[Si+3]
  asm  Call PrintChar

#ifdef BIOS
  asm  Cmp  byte ptr Y2,24
  asm  Jne  j1
  asm  Cmp  byte ptr X2,79
  asm  Je   j2
j1:
  gotoxy(X2,Y2);
#else
  asm  Mov  Ax,Y2            /* Bottom Right */
  asm  Mov  Di,X2
  asm  Call Loc
#endif
  asm  Mov  Bl,Cs:[Si+5]
  asm  Call PrintChar

j2:
#ifdef BIOS
  gotoxy(X1,Y2);
#else
  asm  Mov  Ax,Y2            /* Bottom Left */
  asm  Mov  Di,X1
  asm  Call Loc
#endif
  asm  Mov  Bl,Cs:[Si+4]
  asm  Call PrintChar

#ifdef BIOS
  gotoxy(X1+1,Y1);
  asm  Mov  Ax,Y1            /* Top Horizontal Line */
  asm  Mov  Cl,80
  asm  Mul  Cl
  asm  Mov  Cx,X2
  asm  Mov  Di,X1
  asm  Sub  Cx,Di
  asm  Dec  Cx
  asm  Push Cx
#else
  asm  Mov  Ax,Y1            /* Top Horizontal Line */
  asm  Mov  Cl,80
  asm  Mul  Cl
  asm  Mov  Cx,X2
  asm  Mov  Di,X1
  asm  Sub  Cx,Di
  asm  Dec  Cx
  asm  Push Cx
  asm  Add  Di,Ax
  asm  Inc  Di
  asm  Shl  Di,1
  asm  Add  Di,word ptr [Scrn_Addr]
#endif

  asm  Mov  Bl,Cs:[Si+1]
Lin1:
  asm  Call PrintChar
#ifndef BIOS
  asm  Inc  Di
  asm  Inc  Di
#endif
  asm  Loop Lin1

#ifdef BIOS
  gotoxy(X1+1,Y2);
  asm  Pop  Cx
#else
  asm  Mov  Ax,Y2            /* Bottom Horizontal Line */
  asm  Mov  Cl,80
  asm  Mul  Cl
  asm  Mov  Di,X1
  asm  Pop  Cx
  asm  Add  Di,Ax
  asm  Inc  Di
  asm  Shl  Di,1
  asm  Add  Di,word ptr [Scrn_Addr]
#endif

Lin2:
  asm  Call PrintChar
#ifndef BIOS
  asm  Inc  Di
  asm  Inc  Di
#endif
  asm  Loop Lin2

  asm  Mov  Ax,Y1            /* Right Vertical Line */
  asm  Inc  Ax
  asm  Mov  Di,X2
  asm  Mov  Cx,Y2
  asm  Sub  Cx,Ax
  asm  Push Cx
#ifdef BIOS
  Xpos = X2;
  Ypos = Y1+1;
#else
  asm  Push Cx
  asm  Call Loc
  asm  Pop  Cx
#endif
  asm  Mov  Bl,Cs:[Si]

Lin3:
#ifdef BIOS
  gotoxy(Xpos,Ypos);
  asm  Call PrintChar
  Ypos++;
#else
  asm  Call PrintChar
  asm  Add  Di,160
#endif
  asm  Loop Lin3

#ifdef BIOS
  Xpos = X1;
  Ypos = Y1+1;
#else
  asm  Mov  Ax,Y1            /* Left Vertical Line */
  asm  Inc  Ax
  asm  Mov  Di,X1
  asm  Call Loc
#endif
  asm  Pop  Cx

Lin4:
#ifdef BIOS
  gotoxy(Xpos,Ypos);
  asm  Call PrintChar
  Ypos++;
#else
  asm  Call PrintChar
  asm  Add  Di,160
#endif
  asm  Loop Lin4

#ifdef BIOS
  ansi_rest();
#endif

  NEEDSEGPOPDS;
#endif /* ifdef __OS2__ */
}
