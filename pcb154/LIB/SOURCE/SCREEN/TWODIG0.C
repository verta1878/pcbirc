/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* twodig0: Convert a byte value (0-99) to two ASCII digits with leading zero. */

#ifdef __WATCOMC__

void pascal twodig0(char *Num) {
  unsigned char val = (unsigned char)*Num;
  Num[0] = (val / 10) + '0';
  Num[1] = (val % 10) + '0';
}

#else

#ifdef LDATA
  #include "model.h"
#endif

void pascal twodig0(char *Num) {
#ifdef SDATA
  asm  Mov  Si,Num
#else
  asm  Push Ds
  asm  Lds  Si,Num
#endif
  asm  Mov  Ax,[Si]
  asm  Aam
  asm  Or   Ax,'00'
  asm  Xchg Al,Ah
  asm  Mov  [Si],Ax
#ifndef SDATA
  asm  Pop  Ds
#endif
}

#endif /* __WATCOMC__ */
