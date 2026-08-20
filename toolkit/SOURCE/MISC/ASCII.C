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
    #pragma inline
  #endif
  #include "model.h"
#endif

#include <misc.h>
#include <stdlib.h>


void LIBENTRY lascii(char *Str, long Num) {
#if defined(__OS2__) || defined(__WATCOMC__)
  ltoa(Num,Str,10);
#else

#ifdef CPU386
  char Buf[32];
  char Negative;

asm      Mov EAx,Num

#ifdef LDATA
asm      Push Ds
asm      Mov  Bx,Ss
asm      Mov  Ds,Bx
asm      Lea  Si,Buf
asm      Les  Di,Str
#else
asm      Lea Si,Buf
asm      Mov Di,Str
#endif

/*
    Converts a 32 bit integer in EAX to DS:SI
    Input  = EAX with DS:SI  pointing to string buffer
    Output = number formatted in buffer passed to it

    EAX   = number to convert (moves into EAX for the division)
    BX    = number 10
    CX    = counts the number of digits
    DS:DI = pointer to string
    DX    = receives the remainder of the division
    SI    = points to the user buffer
    DI    = points to a temp buffer
*/


asm      Mov  EBx,10
asm      Xor  Cx,Cx
asm      Mov  Negative,Cl

asm      Cmp EAx,0
asm      Jge TestSize
asm      Neg EAx
asm      Inc byte ptr Negative

TestSize:
asm      Cmp EAx,0FFFFh
asm      Jbe Cvt16

Cvt32:
asm      Xor  EDx,EDx
asm      Div  EBx
asm      Mov  [Si],Dl
asm      Inc  Si
asm      Inc  Cx
asm      Cmp  EAx,0FFFFh
asm      Ja   Cvt32
asm      Or   EAx,EAx
asm      Jz   Done

Cvt16:
asm      Xor  Dx,Dx
asm      Div  Bx
asm      Mov  [Si],Dl
asm      Inc  Si
asm      Inc  Cx
asm      Or   Ax,Ax
asm      Jnz  Cvt16

Done:
asm      Cld
asm      Cmp  byte ptr Negative,0
asm      Je   Reverse
asm      Mov  Al,'-'
asm      Stosb

Reverse:
asm      Dec  Si
asm      Mov  Al,[Si]
asm      Add  Al,'0'
asm      Stosb
asm      Loop Reverse

asm      Xor  Ax,Ax
asm      Stosb

#ifdef LDATA
asm      Pop  Ds
#endif

#else
  ltoa(Num,Str,10);
#endif
#endif /* ifdef __OS2__ */
}


void LIBENTRY ascii(char *Str, int Num) {
#if defined(__OS2__) || defined(__WATCOMC__)
  itoa(Num,Str,10);
#else

  char Buf[32];
  char Negative;


#ifdef LDATA
asm      Push Ds
asm      Mov  Bx,Ss
asm      Mov  Ds,Bx
asm      Lea  Si,Buf
asm      Les  Di,Str
#else
asm      Lea Si,Buf
asm      Mov Ax,Ds
asm      Mov Es,Ax
asm      Mov Di,Str
#endif

asm      Mov Ax,Num

/*
    Converts a 16 bit integer in AX to DS:SI
    Input  = AX with DS:SI  pointing to string buffer
    Output = number formatted in buffer passed to it

    AX    = number to convert (moves into EAX for the division)
    BX    = number 10
    CX    = counts the number of digits
    DS:DI = pointer to string
    DX    = receives the remainder of the division
    SI    = points to the user buffer
    DI    = points to a temp buffer
*/


asm      Mov  Bx,10
asm      Xor  Cx,Cx
asm      Mov  Negative,Cl

asm      Cmp Ax,0
asm      Jge Cvt
asm      Neg Ax
asm      Inc byte ptr Negative

Cvt:
asm      Xor  Dx,Dx
asm      Div  Bx
asm      Mov  [Si],Dl
asm      Inc  Si
asm      Inc  Cx
asm      Or   Ax,Ax
asm      Jnz  Cvt

asm      Cld
asm      Cmp  byte ptr Negative,0
asm      Je   Reverse
asm      Mov  Al,'-'
asm      Stosb

Reverse:
asm      Dec  Si
asm      Mov  Al,[Si]
asm      Add  Al,'0'
asm      Stosb
asm      Loop Reverse

asm      Xor  Ax,Ax
asm      Stosb

#ifdef LDATA
asm      Pop  Ds
#endif
#endif /* ifdef __OS2__ */
}


void LIBENTRY asciispace(char *Str, int Num) {
  ascii(Str,Num);
  if (Str[1] == 0)
    Str[1] = ' ';
}


#ifdef TEST
#include <stdio.h>
void main(void) {
  char Str[80];
  long X;

  for (X = -234567L; X <= 234567L; X++)
    ascii(Str,X);

  puts(Str);
}
#endif


#ifdef TEST2
#include <stdio.h>
void main(void) {
  char Str[80];
  int X;

  for (X = -32000L; X <= 32000; X++)
    ascii(Str,X);

  puts(Str);
}
#endif
