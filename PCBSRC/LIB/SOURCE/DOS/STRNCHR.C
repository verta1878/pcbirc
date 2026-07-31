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


#ifndef __OS2__
//#pragma inline
  #include "model.h"
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifndef NULL
#ifdef  SDATA
#define NULL    0
#else
#define NULL    0L
#endif
#endif

#pragma warn -rvl
char * LIBENTRY strnchr(char *StrPtr, char C, int Len) {
  #ifdef __OS2__

    for ( ; Len != 0; Len--, StrPtr++)
      if (*StrPtr == C)
        return(StrPtr);
    return(NULL);

  #else
    #ifdef SDATA
      asm mov   ax,ds
      asm mov   es,ax
      asm mov   di,StrPtr
    #else
      asm les   di,StrPtr
      asm mov   dx,es
    #endif

      asm cld
      asm mov   cx,Len
      asm mov   al,C

      asm repnz scasb
      asm jz    found
      asm xor   ax,ax
      asm mov   dx,ax
      asm jmp   short exit

    found:
      asm dec   di
      asm mov   ax,di

    /* ignore warning about needing a return value - it's already in AX or DX:AX */
    exit:;
  #endif
}
