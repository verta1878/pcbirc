/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifdef __OS2__
  #define INCL_VIO
  #include <os2.h>
#elif defined(__WATCOMC__)
  #include <i86.h>
#elif defined(_MSC_VER)
  #include <borland.h>
#endif

#include <screen.h>

void LIBENTRY gotoxy(int X, int Y) {
#ifdef __OS2__
  Scrn_X = (char) X;
  Scrn_Y = (char) Y;
#elif defined(__WATCOMC__)
  {
    union REGS r;
    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dh = (unsigned char) Y;
    r.h.dl = (unsigned char) X;
    int386(0x10, &r, &r);
  }
#else
  asm  mov   ah, 2
  asm  xor   bh, bh
  asm  mov   dh, byte ptr Y
  asm  mov   dl, byte ptr X
  asm  int   10h
#endif
}
