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
 *  Function:  xyvalue()
 *
 *  return the XY screen coordinate as an integer offset into the screen buffer
 */

int LIBENTRY xyvalue(int X, int Y) {
  return((Y * 160) + (X << 1));
}


/********************************************************************
 *
 *  Function:  fastputc()
 *
 *  Using an integer XY screen coordinate, place a character onto the screen
 */

void LIBENTRY fastputc(int Loc, char C) {
#ifdef __OS2__
  char *p = (char *) Scrn_Buf;
  int   Line;

  p[Loc] = C;

  Line = (Loc / 160);
  updatelines(UPDATE_MIXED,Line,Line);

#else  /* ifdef __OS2__ */

  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

#ifdef BIOS
  int  X, Y;

  asm  Les Di,dword ptr Scrn_Addr
  asm  Add Di,Loc
  asm  Mov Al,Es:[Di+1]    /* get the current color     */

  ansi_color(_AL);
  Y = Loc / 160;
  X = (Loc - (Y * 160)) >> 1;
  ansi_save();
  gotoxy(X,Y);
  putchar(C);
  ansi_rest();

#else  /* ifdef BIOS */

  asm  Les  Di,dword ptr Scrn_Addr
  asm  Add  Di,Loc
  asm  Mov  Bl,C

  asm  Cmp  Scrn_Rtrc,0
  asm  Je   J2
  asm  Mov  Dx,03DAh       /* Store port addr in Dx */
J1:
  asm  In   Al,Dx          /*   get raster status       */
  asm  Test Al,01          /*   if not 1                */
  asm  Jz   J1             /*     then J1               */
J2:
  asm  Mov Es:[Di],Bl      /* Store Char in scrn buffer */
#endif  /* ifdef BIOS */

  NEEDSEGPOPDS;
#endif /* ifdef __OS2__ */
}


/********************************************************************
 *
 *  Function:  fastputch()
 *
 *  Using an integer XY screen coordinate, place a character AND color onto
 *  the screen
 */

void LIBENTRY fastputch(int Loc, char C, char Color) {
#ifdef __OS2__
  char *p = (char *) Scrn_Buf;
  int   Line;

  p[Loc] = C;
  p[Loc+1] = Color;

  Line = (Loc / 160);
  updatelines(UPDATE_MIXED,Line,Line);

#else  /* ifdef __OS2__ */

  NEEDSEGPUSHDS;
  NEEDSEGGETDS(Scrn_Addr);

#ifdef BIOS
  int  X, Y;

  Y = Loc / 160;
  X = (Loc - (Y * 160)) >> 1;
  ansi_save();
  ansi_color(Color);
  gotoxy(X,Y);
  putchar(C);
  ansi_rest();

#else  /* ifdef BIOS */

  asm  Les  Di,dword ptr Scrn_Addr
  asm  Add  Di,Loc
  asm  Mov  Bl,C
  asm  Mov  Bh,Color

  asm  Cmp  Scrn_Rtrc,0
  asm  Je   J2
  asm  Mov  Dx,03DAh       /* Store port addr in Dx */
J1:
  asm  In   Al,Dx          /*   get raster status       */
  asm  Test Al,01          /*   if not 1                */
  asm  Jz   J1             /*     then J1               */
J2:
  asm  Mov Es:[Di],Bx      /* Store Char and Color in scrn buffer */
#endif  /* ifdef BIOS */

  NEEDSEGPOPDS;
#endif  /* ifdef __OS2__ */
}
