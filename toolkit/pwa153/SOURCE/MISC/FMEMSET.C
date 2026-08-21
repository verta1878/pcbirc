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


/* under OS/2 fmemset() maps directly to memset() */

#ifndef __OS2__

#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef _MSC_VER
#include <borland.h>
#else
#pragma inline
#endif

void pascal fmemset(void far *dest, char c, unsigned len) {
  asm  cld
  asm  les  di,dest
  asm  mov  al,c
  asm  mov  ah,al
  asm  mov  cx,len

  asm  shr  cx,1
  asm  rep  stosw
  asm  adc  cx,0
  asm  rep  stosb
}


#endif  /* ifndef __OS2__ */

