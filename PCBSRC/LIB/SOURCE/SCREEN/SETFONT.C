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

void LIBENTRY setfont(fonttype Font) {
#ifdef __OS2__
  VIOMODEINFO Mode;
  Mode.cb = sizeof(Mode);
  VioGetMode(&Mode,0);
  Mode.row = (Font == FONT_8x8) ? 50 : 25;
  VioSetMode(&Mode,0);
#elif defined(__WATCOMC__)
  {
    union REGS r;
    r.h.ah = 0x11;
    r.h.al = (Font == FONT_8x8) ? 0x12 : 0x14;
    r.h.bl = 0;
    int386(0x10, &r, &r);
  }
#else
  if (Font == FONT_8x8) {
    asm mov ax,1112h
    asm mov bl,0
    asm int 10h
  }
  else {
    asm mov ax,1114h
    asm mov bl,0
    asm int 10h
  }
#endif
}
