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


#include <stdio.h>
#include <string.h>
#include <misc.h>
#ifndef __OS2__
  #include <model.h>
  #pragma inline
#endif
#ifdef DEBUG
  #include <memcheck.h>
#endif

int LIBENTRY findfour(char *Base, char *Srch) {
#ifdef __OS2__
  int *p;      /* under OS/2, these are 4-byte integers! */
  int *q;
  int  NumItems;
  int  Save;

  p = (int *) Base;
  q = (int *) Srch;
  Save = NumItems = strlen(Base) >> 2;  /* how many 4-byte words are there? */

  for (; NumItems > 0; NumItems--, p++) {
    if (*p == *q)
      return(Save - NumItems + 1);
  }

  return(0);   /* not found */

#else  /* ifdef __OS2__ */

  strlen(Base);
  asm shr  ax,1
  asm shr  ax,1
  asm mov  cx,ax     /* cx = NumItems */
  asm mov  bx,ax     /* bx = NumItems */

#ifdef SDATA
  asm mov  di,Base
  asm mov  si,Srch
#else
  asm push ds
  asm les  di,Base
  asm lds  si,Srch
#endif
  asm mov  dx,si
                      /* DS:SI points to Srch                       */
                      /* ES:DI points to Base                       */
                      /* CX    is the number of items left to check */
                      /* BX    was the starting number of items     */
                      /* DX    was the start of Srch                */
j0: asm cmpsw
    asm jne  j1
    asm cmpsw
    asm jne  j2
    asm sub  bx,cx
    asm inc  bx
    asm mov  ax,bx
    asm jmp  short j3
j1: asm add  di,2   /* still have to add 2 more to base */
j2: asm mov  si,dx
    asm loop j0

    asm xor  ax,ax
j3:;
#ifdef LDATA
    asm pop  ds
#endif
  return(_AX);
#endif  /* ifdef __OS2__ */
}


#ifdef TEST
void main(void) {
  char Str[80];
  char BatchCommands[] = "REMODAYSSINCEXPDKEEPLOCKPRIN";

  gets(Str);
  strupr(Str);
  printf("%s = %d\n",Str,findfour(BatchCommands,Str));
}
#endif
