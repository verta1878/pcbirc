/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* fmemset: Far memory set — on flat model, just use memset */

#ifdef __WATCOMC__
#include <string.h>

void pascal fmemset(void *dest, char val, unsigned len) {
  memset(dest, val, len);
}
#else

#ifdef LDATA
  #include "model.h"
#endif

void pascal fmemset(void far *Dest, char Val, unsigned Len) {
  asm Les  Di,Dest
  asm Mov  Al,Val
  asm Mov  Ah,Al
  asm Mov  Cx,Len
  asm Shr  Cx,1
  asm Rep  Stosw
  asm Adc  Cx,Cx
  asm Rep  Stosb
}
#endif
