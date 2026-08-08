/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifdef __OS2__
  #define INCL_VIO
  #include <os2.h>
#elif defined(__WATCOMC__)
  #include <i86.h>
#else
  #include "model.h"
#endif

#include <screen.h>

/* Cursor shapes by card type: [card][cursortype] */
static unsigned short Cursors[3][4] = {
/* off     small    normal   large  */
  {0x2020, 0x0607, 0x0507, 0x0007},   /* MDA/Hercules */
  {0x2020, 0x0607, 0x0507, 0x0007},   /* CGA          */
  {0x2020, 0x0B0D, 0x090D, 0x000D}    /* EGA/VGA      */
};


void LIBENTRY setcursor(int CursorType) {
  int Num;

  Num = Scrn_ColorCard + Scrn_EGA;
  Num = Cursors[Num][CursorType];

#ifdef __OS2__
  VIOCURSORINFO Cursor;
  VioGetCurType(&Cursor,0);
  Cursor.yStart = Num >> 8;
  Cursor.cEnd   = Num & 0xFF;
  if (CursorType == 0)
    Cursor.attr = -1;
  else
    Cursor.attr = 0;
  VioSetCurType(&Cursor,0);
#elif defined(__WATCOMC__)
  {
    union REGS r;
    r.h.ah = 1;
    r.w.cx = Num;
    int386(0x10, &r, &r);
  }
#else
  asm  Mov  Ah,1
  asm  Mov  Cx,Num
  asm  Int  10h
#endif
}


int LIBENTRY getcursor(void) {
#ifdef __OS2__
  VIOCURSORINFO Cursor;
  VioGetCurType(&Cursor,0);
  if (Cursor.attr == (USHORT)-1)
    return(0);
  else
    return(2);
#elif defined(__WATCOMC__)
  {
    int Num = Scrn_ColorCard + Scrn_EGA;
    unsigned short CurType;
    int X;
    union REGS r;
    r.h.ah = 3;
    r.h.bh = 0;
    int386(0x10, &r, &r);
    CurType = r.w.cx;
    for (X = 3; X >= 0; X--)
      if (CurType == Cursors[Num][X])
        return(X);
    return(1);
  }
#else
  {
    int Num;
    unsigned short CurType;
    int X;

    Num = Scrn_ColorCard + Scrn_EGA;

    asm  Mov  Ah,3
    asm  Xor  Bh,Bh
    asm  Int  10h
    asm  Mov  CurType,Cx

    for (X = 3; X >= 0; X--)
      if (CurType == Cursors[Num][X])
        return(X);
    return(1);
  }
#endif
}
