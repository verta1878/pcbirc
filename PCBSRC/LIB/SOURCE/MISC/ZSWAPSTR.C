/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


//#pragma inline

#include "zsort.h"
#include "model.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

extern unsigned zsort_width;

void zswapstr(char huge *a, char huge *b) {

asm  Mov  Dx,Ds
asm  Mov  Cx,zsort_width;
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
}
