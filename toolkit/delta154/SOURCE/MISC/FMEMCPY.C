/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Clark Development Company — PCBoard source code library.                 */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* fmemcpy: Far memory copy — on flat model, just use memcpy */

#ifdef __WATCOMC__
#include <string.h>

void pascal fmemcpy(void *dest, void *src, unsigned len) {
  memcpy(dest, src, len);
}
#else

#ifdef LDATA
  #include "model.h"
#endif

void pascal fmemcpy(void far *Dest, void far *Srce, unsigned Len) {
  asm Mov  Dx,Ds
  asm Mov  Cx,Len
  asm Shr  Cx,1
  asm Les  Di,Dest
  asm Lds  Si,Srce
  asm Rep  Movsw
  asm Adc  Cx,Cx
  asm Rep  Movsb
  asm Mov  Ds,Dx
}
#endif
