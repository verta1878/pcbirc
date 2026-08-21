/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifdef __WATCOMC__
#include <i86.h>
#include <dos.h>
#endif

char static OldSec;

char pascal timechanged(void) {
#ifdef __WATCOMC__
  union REGS r;
  r.h.ah = 0x2C;
  int386(0x21, &r, &r);
  if (r.h.dh != OldSec) {
    OldSec = r.h.dh;
    return 1;
  }
  return 0;
#else
  asm Mov Ah,2Ch
  asm Int 21h
  asm Xor Ax,Ax
  asm Cmp Dh,OldSec
  asm Je  exit
  asm Mov OldSec,Dh
  asm Mov Al,1
exit:;
  return(_AL);
#endif
}
