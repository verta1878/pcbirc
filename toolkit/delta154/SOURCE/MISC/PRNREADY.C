/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifdef __WATCOMC__
#include <i86.h>
#endif

#include "misc.h"

bool LIBENTRY printerready(int LptNum) {
#ifdef __OS2__
  return(TRUE);
#elif defined(__WATCOMC__)
  {
    union REGS r;
    r.h.ah = 2;
    r.w.dx = LptNum - 1;
    int386(0x17, &r, &r);
    return (bool)(r.h.ah & 0x80);  /* bit 7 = not busy */
  }
#else
  asm mov  ah,2
  asm mov  dx,LptNum
  asm dec  dx
  asm int  17h
  asm mov  Al,Ah
  asm xor  Ah,Ah
#endif
}
