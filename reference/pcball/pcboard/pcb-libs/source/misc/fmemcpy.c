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


/* under OS/2, farmemcpy() maps directly to memcpy() */

#ifndef __OS2__

#ifdef _MSC_VER
#include <borland.h>
#else
#pragma inline
#endif

#ifdef DEBUG
#include <memcheck.h>
#endif

void pascal farmemcpy(void far *Dest, void far *Srce, int Len) {
  asm Mov  Dx,Ds        /* save Ds */
  asm Mov  Cx,Len
  asm Shr  Cx,1         /* Cx = number of WORDS to move */
  asm Les  Di,Dest      /* Es:Di points to destination  */
  asm Lds  Si,Srce      /* Ds:Si points to source       */
  asm Cld               /* set to forward direction     */
  asm Rep  Movsw        /* move Cx number of WORDS      */
  asm Jnb  skip         /* any BYTES left over, no skip */
  asm Movsb             /* yes, move single byte        */
skip:
  asm Mov  Ds,Dx        /* restore Ds */
}


#endif  /* ifndef __OS2__ */

