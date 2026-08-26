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
/*  #pragma inline */
  #endif
  #include <asmrules.h>
#endif

#include <string.h>
#include <country.h>

/* This function is different from memcmp() in that memfcmp() uses the   */
/* country-dependent collating sequence to return proper comparisons for */
/* sorting of information that potentially includes accented characters  */

int LIBENTRY memfcmp(const void *s1, const void *s2, size_t n) {
#ifdef __OS2__
  int dif;
  const unsigned char *p1 = (unsigned char *) s1;
  const unsigned char *p2 = (unsigned char *) s2;

  for (; n-- >0; p1++, p2++ ) {
    if (*p1 != *p2) {
      dif = Collate[*p1] - Collate[*p2];
      if (dif != 0)
        return(dif);
    }
  }
  return(0);


#else   /* ifdef __OS2__ */

        pushDS_

#if !(LDATA)
asm     mov   ax, ds
asm     mov   es, ax
#endif

asm     mov     ax, n
asm     mov     cx, ax
asm     jcxz    mcm_end

asm     LDS_    si, s1
asm     LES_    di, s2

#if LDATA
asm     mov     dx, ds
asm     push    bp
asm     mov     ax, seg Collate
asm     mov     bp, ax
#endif
asm     mov     bx, offset Collate

asm     cld

startcompare:

asm     rep     cmpsb

asm     jcxz    done
asm     mov     al, [si-1]
asm     mov     ah, ES_ [di-1]
#if LDATA
asm     mov     ds, bp
#endif
asm     xlat
asm     xchg    al, ah
asm     xlat
asm     cmp     al, ah            /* are the character weights equivalent */
#if LDATA
asm     mov     ds, dx
#endif
asm     je      startcompare      /* yes, go back for further comparisons */


done:

/*      The result is the UNsigned difference of the final character pair,
        be they equal or different.
*/
asm     mov     cl, [si-1]
asm     mov     al, ES_ [di-1]
#if LDATA
asm     mov     ds, bp
#endif
asm     xlat
asm     xchg    al, cl
asm     xlat
#if LDATA
asm     pop     bp
#endif

asm     xor     ah, ah
asm     xor     ch, ch

mcm_end:
        popDS_

asm     sub   ax, cx

/*      return( _AX - _CX ); */

#endif  /* ifdef __OS2__ */
}
