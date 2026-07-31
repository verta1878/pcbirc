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
  #ifdef _MSC_VER
    #include <borland.h>
  #else
//  #pragma inline
  #endif
  #include <asmrules.h>
#endif

#include <string.h>
#include <country.h>

extern "C" int stricmp(const char *str1, const char *str2) {
#ifdef __OS2__
  int dif;
  const unsigned char *p1 = (unsigned char *) str1;
  const unsigned char *p2 = (unsigned char *) str2;

  for (; 1; p1++, p2++) {
    dif = UpperCase[*p1] - UpperCase[*p2];
    if (dif != 0)
      return(dif);
    if (*p1 == 0)
      return(0);
  }

#else  /* ifdef __OS2__ */

        pushDS_

#if !(LDATA)
asm     mov     ax, dx
asm     mov     es, ax
#endif

asm     cld

asm     LDS_    si, str1
asm     LES_    di, str2

#if LDATA
#ifdef CPU386
asm     mov     ax, seg UpperCase
asm     mov     fs, ax
#else
asm     mov     cx, ds
asm     push    bp
asm     mov     ax, seg UpperCase
asm     mov     bp, ax
#endif
#endif
asm     mov     bx, offset UpperCase


/*
  AH and DH will stay zero during this process. This'll be handy when its
  time for a final subtract later.
*/
asm     xor     ax, ax          /* AX <- 0      */
asm     mov     dx, ax          /* DX <- 0      */

cmi_nextCh:
asm     lodsb                   /* AL <- str1[i]        */
asm     mov     dl, ES_ [di]    /* DL <- str2[i]        */
asm     or      al, al          /* null terminator?     */
asm     jz      cmi_end
asm     scasb                   /* str1[i] == str2[i]?, advance DI  */
asm     je      cmi_nextCh

cmi_alUpper:
#if    LDATA
#ifdef CPU386
asm     db      64h
asm     xlat
asm     xchg    al, dl
asm     db      64h
asm     xlat
#else
asm     mov     ds, bp          /* restore DS                    */
asm     xlat                    /* convert str1[i] to uppercase  */
asm     xchg    al, dl
asm     xlat                    /* convert str2[i] to uppercase  */
asm     mov     ds, cx
#endif
#else
asm     xlat                    /* convert str1[i] to uppercase  */
asm     xchg    al, dl
asm     xlat                    /* convert str2[i] to uppercase  */
#endif
asm     xchg    al, dl

cmi_compareAgain:
asm     cmp     al, dl          /* str1[i] == str2[i] */
asm     je      cmi_nextCh

cmi_end:
/*
  We need to do the full word subtract here because ANSI requires unsigned
  comparisons. A simple byte subtract with a CBW would produce the wrong
  result in some cases. Remember that AH, DH are still zero.
*/
asm     sub     ax, dx

#if     LDATA & ! defined(CPU386)
asm     pop     bp
#endif

        popDS_

/*      return _AX; */
#endif  /* ifdef __OS2__ */
}
