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


#ifdef _MSC_VER
#include <borland.h>
#else
#pragma  inline
#endif

#include <string.h>
#include <asmrules.h>
#include <country.h>

extern "C" int tolower(int C) {
  return(LowerCase[C]);
}


extern "C" char * strlwr(char *s) {
#ifdef __OS2__
  char *p;

  for (p = s; *p != 0; p++)
    *p = LowerCase[*p];

  return(s);

#else  /* ifdef __OS2__ */

#if LDATA
asm     push    ds
#endif

asm     cld

#ifdef LDATA
#ifdef CPU386
asm     mov     ax, seg LowerCase
asm     mov     fs, ax
#else
asm     mov     cx, seg LowerCase
#endif
#endif

asm     mov     bx, offset LowerCase    /* point CX:BX to LowerCase[]  */
asm     LDS_    si, s                   /* point DS:SI to string       */
#if     LDATA && ! defined(CPU386)
asm     mov     di, ds                  /* save DS register            */
#endif
asm     mov     dx, si                  /* save addr for return        */
        goto    next_char;

convert_loop:
#if    LDATA
#ifdef CPU386
asm    db       64h
asm    xlat
#else
asm    mov      ds, cx
asm    xlat
asm    mov      ds, di
#endif
#else
asm    xlat
#endif

asm     mov     [si-1], al

next_char:
asm     lodsb
asm     and     al, al
asm     jnz     convert_loop
asm     mov     ax, dx          /* return addr of string */
#if LDATA
asm     mov     dx, ds
asm     pop     ds
#endif
/*return; */       /* ignore error - the AX and DX registers are already */
                   /* set to the return values                           */
#endif  /* ifdef __OS2__ */
}
