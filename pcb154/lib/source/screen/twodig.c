/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* twodig: Convert a byte value (0-99) to two ASCII digits at *Num.
   Leading zero is replaced with space. */

#ifdef __WATCOMC__

void pascal twodig(char *Num) {
  unsigned char val = (unsigned char)*Num;
  char hi = (val / 10) + '0';
  char lo = (val % 10) + '0';
  if (hi == '0') hi = ' ';
  Num[0] = hi;
  Num[1] = lo;
}

#else

#ifdef LDATA
  #include "model.h"
#endif

void pascal twodig(char *Num) {
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
  asm  Cmp  Al,'0'
  asm  Jne  End
  asm  Mov  Al,' '
End:
  asm  Mov  [Si],Ax
#ifndef SDATA
  asm  Pop  Ds
#endif
}

#endif /* __WATCOMC__ */
