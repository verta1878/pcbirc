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
  #include "model.h"
#endif

#include "misc.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif


/********************************************************************
*
*  Function:  maxstrcpy()
*
*  Desc    :  copies the NULL terminated Srce string into Dest
*
*  Notes   :  Max is used to provide a "maximum number of bytes to copy" such
*             that a NULL terminated string is stored in Dest.
*
*  Example :  maxstrcpy(Dest,"12345",2);  Dest = "1\0"
*             maxstrcpy(Dest,"12345",5);  Dest = "1234\0"
*             maxstrcpy(Dest,"12345",9);  Dest = "12345\0"
*/

void LIBENTRY maxstrcpy(char *Dest, char *Srce, int Max) {
#ifdef __OS2__
  for (Max--; Max > 0 && *Srce > 0; Srce++, Dest++, Max--)
    *Dest = *Srce;
  *Dest = 0;

#else  /* ifdef __OS2__ */

#ifdef SDATA
  asm mov   ax,ds
  asm mov   es,ax
  asm mov   di,Srce
#else
  asm les   di,Srce
#endif

  asm cld
  asm mov   si,di
  asm xor   al,al
  asm mov   cx,Max
  asm mov   dx,cx
  asm repne scasb
  asm sub   dx,cx
  asm mov   cx,dx
  asm dec   cx

copy:
#ifdef SDATA
  asm mov   di,Dest               /* ES:DI = Dest / DS:DI = Srce */
#else
  asm push  ds
  asm mov   ax,es
  asm mov   ds,ax
  asm les   di,Dest               /* ES:DI = Dest / DS:DI = Srce */
#endif

  asm shr   cx,1
  asm rep   movsw
  asm adc   cx,0
  asm rep movsb
  asm xor   ax,ax
  asm stosb

#ifdef LDATA
  asm pop   ds
#endif
#endif  /* ifdef __OS2__ */
}

#ifdef TEST
#include <stdio.h>
#include <string.h>

void main(void) {
  char A[10];
  char B[10];
  int  X;

  strcpy(A,"1234567890");

  puts(A);
  for (X = 1; X < 10; X++) {
    maxstrcpy(B,A,X);
    puts(B);
  }
}
#endif
