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
  #pragma inline
  #include "model.h"
#endif

#include "misc.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

/*     WARNING!!!   This function expects the string "Srce" to be in all   */
/*                  UPPERCASE letters                                      */



/*
  A,E,H,I,O,U,W,Y  = '0'  (discarded characters)
  B,F,P,V          = '1'
  C,G,J,K,Q,S,X,Z  = '2'
  D,T              = '3'
  L                = '4'
  M,N              = '5'
  R                = '6'
*/


void LIBENTRY soundex(char *Dest, char *Srce) {
#if defined(__OS2__) || defined(__WATCOMC__)
  static char Table[] = {48,49,50,51,48,49,50,48,48,50,50,52,53,53,48,49,50,
                         54,50,51,48,49,48,50,48,50};
  char *LastByte;
  char  Byte;

  *Dest++ = *Srce;            // first byte of Dest equals first byte of Srce

  for (Srce++, LastByte = Dest+3; *Srce != 0; Srce++) {
    if (*Srce >= 'A' && *Srce <= 'Z') {
      Byte = Table[*Srce-'A'];
      if (Byte != '0') {
        *Dest++ = Byte;
        if (Dest == LastByte) {
          *Dest = 0;
          return;
        }
      }
    }
  }

  while (Dest < LastByte)
    *Dest++ = '0';
  *Dest = 0;

#else  /* ifdef __OS2__ */

/*
 *   Si points to Srce
 *   Di points to Dest
 *   Cx = Ending point (reference to Di)
 *   Al = Character / LookupValue
 *   Dl = Previous
 */

     asm   jmp short start

  asm  Table    db  48,49,50,51,48,49,50,48,48,50,50,52,53,53,48,49,50
  asm           db  54,50,51,48,49,48,50,48,50

start:
#ifdef SDATA
     asm   mov     Si,word ptr Srce       /* point si to Srce   */
     asm   mov     Di,word ptr Dest       /* point di to Dest   */
#else
     asm   Push    Ds
     asm   Lds     Si,dword ptr Srce      /* point si to Srce   */
     asm   Les     Di,dword ptr Dest      /* point di to Dest   */
#endif

     asm   Mov     Cx,Di
     asm   Add     Cx,4                   /* point Cx to Dest+4 */

#ifdef SDATA
     asm   Mov     word ptr [Di],3030h    /* Initialize Dest to "0000" */
     asm   Mov     word ptr [Di+2],3030h
     asm   Mov     byte ptr [Di+4],0
#else
     asm   Mov     word ptr Es:[Di],3030h /* Initialize Dest to "0000" */
     asm   Mov     word ptr Es:[Di+2],3030h
     asm   Mov     byte ptr Es:[Di+4],0
#endif

     asm   Cld
     asm   Xor     Ax,Ax
     asm   Xor     Dx,Dx                   /* Initialize Previous to 0      */
     asm   Lodsb                           /* Get Srce[0]                   */
j1:  asm   Stosb                           /* Store in Dest[0]              */

j2:  asm   Lodsb                           /* Get next char from Srce       */
     asm   Cmp     Al,0                    /* Is it a null terminator?      */
     asm   Je      j3                      /*   if yes, jump to j3 to end   */
     asm   Cmp     Di,Cx                   /* Is Dest ptr >= end point?     */
     asm   Jge     j3                      /*   if yes, jump to j3 to end   */

     asm   Cmp     Al,'A'                  /* Is char less than 'A'         */
     asm   Jl      j2                      /*   if yes, go back to top      */
     asm   Cmp     Al,'Z'                  /* Is char greater than 'z'      */
     asm   Jg      j2                      /*   if yes, go back to top      */

                                           /* Store character in Bx for use */
     asm   Mov     Bx,Ax                   /* as Lookup in SoundexTable     */
     asm   Mov     Al,byte ptr cs:Table[bx-65]             /* get Lookup    */

     asm   cmp     Al,'0'                  /* Is it equal to '0'            */
     asm   je      j2                      /*   if yes, go back to top      */
     asm   cmp     Al,Dl                   /* Is it equal to Previous       */
     asm   je      j2                      /*   if yes, go back to top      */

     asm   Stosb                           /* Store Lookup in Dest          */
     asm   mov     Dl,Al                   /* Store Lookup in Previous      */
     asm   Jmp short j2                    /* Go back to top                */
j3:
#ifdef LDATA
     asm   Pop     Ds
#endif
;
#endif  /* ifdef __OS2__ */
}


#ifdef TEST
#include <stdio.h>

void main(int argc, char **argv) {
  char Value[5];

  /* NOTE:  argv[1] needs to be in uppercase letters */
  soundex(Value,argv[1]);
  puts(Value);
}
#endif
