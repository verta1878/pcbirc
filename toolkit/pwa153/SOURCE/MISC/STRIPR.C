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


#include <string.h>
#include <misc.h>

#ifndef __OS2__
  #include <model.h>
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif

/********************************************************************
 *
 *  Function:  stripright()
 *
 *  Strips character Ch's from the right side of Str[]
 */

char * LIBENTRY stripright(char *StrPtr, char SrchChar) {
#ifdef __OS2__
  char *p;

  for (p = &StrPtr[strlen(StrPtr)-1]; *p == SrchChar && p >= StrPtr; p--);
  *(++p) = 0;

#else  /* ifdef __OS2__ */

#ifdef LDATA
  asm   les  si,StrPtr
#else
  asm   mov  ax,ds
  asm   mov  es,ax
  asm   mov  si,StrPtr
#endif

asm   mov   di,si
asm   xor   ax,ax
asm   mov   cx,-1
asm   repne scasb
asm   not   cx
asm   dec   cx

asm   jcxz skip
asm   dec  cx

asm   mov  di,si
asm   add  di,cx
asm   mov  al,SrchChar

asm   std
asm   scasb
asm   jne  skip
asm   repe scasb
asm   jz   j1
asm   inc  di
j1:
asm   inc  di
j2:

asm   mov  al,ah
asm   stosb

skip:
  asm   cld
#endif  /* ifdef __OS2__ */

  return(StrPtr);
}
