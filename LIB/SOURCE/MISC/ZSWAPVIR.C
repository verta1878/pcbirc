/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* ZSWAPVIR.C — Swap two 5-byte VirType records                              */
/* Clark Development Company, Inc. (C) 1996. All Rights Reserved.            */
/* Watcom C conversion by pcbrevival (GPL v3.0 for our additions)            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "zsort.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

void zswapvir(char *a, char *b) {
#if defined(__OS2__) || defined(__WATCOMC__)
  char tmp[5];
  tmp[0]=a[0]; tmp[1]=a[1]; tmp[2]=a[2]; tmp[3]=a[3]; tmp[4]=a[4];
  a[0]=b[0]; a[1]=b[1]; a[2]=b[2]; a[3]=b[3]; a[4]=b[4];
  b[0]=tmp[0]; b[1]=tmp[1]; b[2]=tmp[2]; b[3]=tmp[3]; b[4]=tmp[4];
#else
  asm  Mov  Dx,Ds
  asm  Les  Di,b
  asm  Lds  Si,a
  asm  Mov  Ax,Ds:[Si]
  asm  Mov  Bx,Es:[Di]
  asm  Mov  Ds:[Si],Bx
  asm  Mov  Es:[Di],Ax
  asm  Add  Si,2
  asm  Add  Di,2
  asm  Mov  Ax,Ds:[Si]
  asm  Mov  Bx,Es:[Di]
  asm  Mov  Ds:[Si],Bx
  asm  Mov  Es:[Di],Ax
  asm  Add  Si,2
  asm  Add  Di,2
  asm  Mov  Al,Ds:[Si]
  asm  Mov  Bl,Es:[Di]
  asm  Mov  Ds:[Si],Bl
  asm  Mov  Es:[Di],Al
  asm  Mov  Ds,Dx
#endif
}
