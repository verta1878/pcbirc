/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* ZSWAPSTR.C — Swap two records of zsort_width bytes                        */
/* Clark Development Company, Inc. (C) 1996. All Rights Reserved.            */
/* Watcom C conversion by pcbrevival (GPL v3.0 for our additions)            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "zsort.h"
#include "model.h"
#include <string.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

extern unsigned zsort_width;

void zswapstr(char huge *a, char huge *b) {
#if defined(__OS2__) || defined(__WATCOMC__)
  unsigned i;
  char tmp;
  for (i = 0; i < zsort_width; i++) {
    tmp = a[i]; a[i] = b[i]; b[i] = tmp;
  }
#else
  asm  Mov  Dx,Ds
  asm  Mov  Cx,zsort_width
  asm  Les  Di,b
  asm  Lds  Si,a
  asm  Test Cx,1
  asm  Jz   j0
  asm  Mov  Al,Ds:[Si]
  asm  Mov  Bl,Es:[Di]
  asm  Mov  Ds:[Si],Bl
  asm  Mov  Es:[Di],Al
  asm  Inc  Si
  asm  Inc  Di
j0:
  asm  Shr  Cx,1
  asm  Or   Cx,Cx
  asm  Jz   j2
j1:
  asm  Mov  Ax,Ds:[Si]
  asm  Mov  Bx,Es:[Di]
  asm  Mov  Ds:[Si],Bx
  asm  Mov  Es:[Di],Ax
  asm  Inc  Si
  asm  Inc  Si
  asm  Inc  Di
  asm  Inc  Di
  asm  Loop j1
j2:
  asm  Mov  Ds,Dx
#endif
}
