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


/********************************************************************
*
*  Function:  padstr()
*
*  Desc    :  pads the right hand side of Str with character C up to the
*             maximum number of characters as determined by MaxLen
*/

void LIBENTRY padstr(char *StrPtr, char C, int MaxLen) {
#ifdef __OS2__
  int  Len;
  char *p;

  Len = strlen(StrPtr);
  p   = &StrPtr[Len];
  for (; Len < MaxLen; Len++, p++)
    *p = C;
  *p = 0;

#else  /* ifdef __OS2__ */

#ifdef SDATA
  asm mov   ax,ds
  asm mov   es,ax
  asm mov   di,StrPtr
#else
  asm les   di,StrPtr
#endif

asm   cld                /* scan for the NULL terminator in the string */
asm   xor   al,al
asm   mov   cx,MaxLen
asm   inc   cx
asm   repne scasb
asm   jcxz  skip         /* if CX = 0 then we didn't find the NULL within   */
                         /* MaxLen bytes so don't bother padding it further */

asm   dec   di           /* DI went one too far so back up one              */
asm   mov   al,C         /* put the character into both AL and AH in order  */
asm   mov   ah,al        /* to use WORD movement to fill the string up      */

asm   shr   cx,1         /* divide CX by 2 (for words)                      */
asm   rep   stosw        /* pad the string                                  */
asm   adc   cx,0         /* add the ODD count (if any) to CX                */
asm   rep   stosb        /* pad the rest of the string (if necessary)       */

asm   xor   ax,ax        /* store a NULL terminator at the end              */
asm   stosb

skip:;
#endif  /* ifdef __OS2__ */
}


#ifdef TEST
#include <stdio.h>
#include <string.h>

char Str[80] = "This is a test";

void main(void) {
  char Str[80];

  strcpy(Str,"This is a test");
  padstr(Str,'.',20);
  printf("%s!\n",Str);

  strcpy(Str,"This is a test");
  padstr(Str,'.',5);
  printf("%s!\n",Str);
}
#endif
