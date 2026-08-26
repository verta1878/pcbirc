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


#ifdef __OS2__
  #include <string.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
/*  #pragma inline */
  #endif
  #include "model.h"
#endif

#include "misc.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

#pragma warn -rvl
/********************************************************************
*
*  Function:  lastcharinstr()
*
*  Desc    :  scans the string *Line from the end looking for the first
*             character not equal to NotChar - signifying the last character
*             in the string.
*
*  Returns :  the position of the last valid character in *Line
*
*  NOTE    :  the position is 1-based .. meaning, if the last character in the
*             string is in column 1 then it returns a "1" (rather than a 0)
*/

int LIBENTRY lastcharinstr(char *Line, char NotChar) {
#ifdef __OS2__
  int Len;

  Len = strlen(Line);
  for (Line += (Len-1); Len > 0; Len--, Line--) {
    if (*Line != NotChar)
      return(Len);
  }
  return(0);

#else  /* ifdef __OS2__ */

  #ifdef LDATA
    asm Les Di,Line
  #else
    asm Mov Ax,Ds
    asm Mov Es,Ax
    asm Mov Di,Line
  #endif

  /* this is a standard method for finding the length of an ASCIIZ string */

  asm Cld                    /* clear direction flag cuz we're going forward */
  asm Xor   Ax,Ax
  asm Mov   Cx,0FFFFh
  asm Repnz Scasb
  asm Not   Cx
  asm Dec   Cx
  asm Jcxz  done        /* zero-length string so exit now */

  /* at this point CX = length of *Line and Di points */
  /* to the character AFTER the NULL terminator       */

  asm Std               /* now we're going backward so set the flag */
  asm Dec   Di          /* pointing to NULL terminator              */
  asm Dec   Di          /* pointing to last character in the string */
  asm Mov   Al,NotChar
  asm Repz  Scasb       /* scan for last character NOT equal to NotChar  */
  asm Mov   Al,0        /* change Ax to 0 without affecting the flags    */
  asm Cld               /* restore the flag in case someone else uses it */
  asm Je    done        /* if the last char was NotChar then get out now */
  asm Inc   Cx          /* otherwise add one for 1-based offsets and     */
  asm Mov   Ax,Cx       /* return it in the AX register                  */

done:
/*return;*/ /* ignore return value error, it's already in AX */
;
#endif  /* ifdef __OS2__ */
}
