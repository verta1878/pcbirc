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

#ifdef DEBUG
#include <memcheck.h>
#endif

/*
    This routine specifically swaps the 5-byte record that VIRTUAL.C uses
    called VirType.
*/

void zswapvir(char huge *a, char huge *b) {

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
}
