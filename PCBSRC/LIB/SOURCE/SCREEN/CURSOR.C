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
  #define INCL_VIO
  #include <os2.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
//  #pragma inline
  #endif
#endif

#include "screen.h"

/********************************************************************
 *
 *  Function:  setcursor()
 *
 *  Sets the cursor size to:  CUR_BLANK, CUR_NORMAL, CUR_BLOCK, CUR_INSERT
 *
 *  NOTE:  Cursors[][] is global and used by both setcursor() & getcursor()
 */

#ifndef __OS2__
short Cursors[3][4] = {{0x2000,0x0B0C,0x000D,0x070D},     /* mono  */
		       {0x2000,0x0607,0x0007,0x0307},     /* color */
		       {0x2000,0x0607,0x0007,0x0307}};    /* ega   */
#endif

void LIBENTRY setcursor(int CursorType) {
  #ifdef __OS2__
  VIOCURSORINFO Cursor;
  VioGetCurType(&Cursor,0);

  if (CursorType == CUR_BLANK)
    Cursor.attr = 0xFFFF;
  else {
    switch (CursorType) {
      case CUR_BLOCK :  Cursor.yStart =  0; break;
      case CUR_INSERT:  Cursor.yStart =  8; break;
      case CUR_NORMAL:  Cursor.yStart = 13; break;
    }
    Cursor.cEnd = 14;
    Cursor.attr = 0;
  }

  VioSetCurType(&Cursor,0);

  #else
  int Num;

  Num = Scrn_ColorCard + Scrn_EGA;
  Num = Cursors[Num][CursorType];

  asm  Mov  Ah,1
  asm  Mov  Cx,Num
  asm  Int  10h
  #endif
}

/********************************************************************
 *
 *  Function:  getcursor()
 *
 *  reports cursor size
 */

int LIBENTRY getcursor(void) {
  #ifdef __OS2__
    VIOCURSORINFO Cursor;
    VioGetCurType(&Cursor,0);

    if (Cursor.attr == 0xFFFF)
      return(CUR_BLANK);
    if (Cursor.cEnd == 14) {
      if (Cursor.yStart == 0)
	return(CUR_BLOCK);
      if (Cursor.yStart == 8)
	return(CUR_INSERT);
    }
    return(CUR_NORMAL);
  #else
    int X,Num,CurType;

    Num = Scrn_ColorCard + Scrn_EGA;

    asm  Mov  Ah,3
    asm  Xor  Bh,Bh
    asm  Int  10h
    asm  Mov  CurType,Cx

    for (X = 3; X >= 0; X--)
      if (CurType == Cursors[Num][X])
	return(X);
    return(1);      /* if not found, return a "normal" cursor value */
  #endif /* ifdef __OS2__ */
}
