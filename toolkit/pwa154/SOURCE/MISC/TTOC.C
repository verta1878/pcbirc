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


#include <misc.h>
#ifdef __OS2__
  #include <stdio.h>
#else
/*#pragma inline */
  #include "model.h"
#endif
#ifdef DEBUG
  #include <memcheck.h>
#endif


char * LIBENTRY ttoc(int Time, char *Temp) {
#ifdef __OS2__
  sprintf(Temp,"%02d:%02d",((Time & 0xF800) >> 11),((Time >> 5) & 0x3F));
#else
#ifdef LDATA
  asm Les Di,Temp
#else
  asm Mov Ax,Ds
  asm Mov Es,Ax
  asm Mov Di,Temp
#endif

  asm Mov  byte ptr Es:[Di+2],':'
  asm Mov  byte ptr Es:[Di+5],0

  asm Mov  Ax,[Time]              /* Store Time in AX           */
  asm Mov  Dx,Ax                  /* Save a copy for use below  */
  asm And  Ah,0F8h                /* Time = Time & F800h        */
  asm Mov  Cl,11
  asm Shr  Ax,Cl                  /* Time = Time shr 11         */

  asm Aam
  asm Or   Ax,3030h
  asm Xchg Al,Ah
  asm Mov  Es:[Di],Ax

  asm Mov  Ax,Dx                  /* Get the time again    */
  asm Mov  Cl,5
  asm Shr  Ax,Cl                  /* Time = Time shr 5     */
  asm And  Al,3Fh                 /* Min  = Time and 003Fh */

  asm Aam
  asm Or   Ax,3030h
  asm Xchg Al,Ah
  asm Mov  Es:[Di+3],Ax
#endif  /* ifdef __OS2__ */
  return(Temp);
}
