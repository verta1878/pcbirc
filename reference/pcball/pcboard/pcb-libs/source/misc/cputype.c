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


#pragma inline
asm .286P

/********************************************************************
*
*  Function: cputype()
*
*  Desc    : Determines the type of CPU installed in the machine.
*
*            See the November 1987 issue of PC Tech Journal (vol 5 num 11)
*            page 51 for an explanation of the procedures involved.
*
*  Returns :   86 =  8086 /  8088
*             186 = 80186 / 80188
*             286 = 80286 (real mode)
*            -286 = 80286 (protected mode)
*             386 = 80386 (real mode)
*            -386 = 80386 (protected mode)
*             486 = 80486 (real mode)
*            -486 = 80486 (protected mode)
*/

int static CPUvalue = 0;  /* use this to store result in case cputype() gets */
                          /* called more than once - no need to figure out   */
                          /* what type of cpu we have AGAIN!                 */

int pascal cputype(void) {
  if (CPUvalue)
    return(CPUvalue);

  asm  push  bp
  asm  push  sp
  asm  pop   ax
  asm  cmp   ax,sp
  asm  jz    not86
  asm  mov   ax,186
  asm  mov   cl,32
  asm  shl   ax,cl
  asm  jnz   exit
  asm  mov   ax,86
  asm  jmp   exit

not86:
  asm  pushf
  asm  mov   ax,sp
  asm  popf
  asm  inc   ax
  asm  inc   ax
  asm  cmp   ax,sp
  asm  jnz   is32bit

is16bit:
  asm  sub   sp,6
  asm  mov   bp,sp
  asm  sgdt  qword ptr [bp]
  asm  add   sp,4
  asm  pop   ax
  asm  inc   ah
  asm  jnz   is32bit

is286:
  asm  mov   ax,286
  asm  jmp   testprot

is32bit:
  asm  mov   bx,sp
  asm  and   sp,not 3
  asm  db    66h
  asm  pushf
  asm  db    66h
  asm  pop   ax
  asm  db    66h
  asm  mov   cx,ax
  asm  db    66h
  asm  xor   ax,0
  asm  dw    4
  asm  db    66h
  asm  push  ax
  asm  db    66h
  asm  popf
  asm  db    66h
  asm  pushf
  asm  db    66h
  asm  pop   ax

  asm  db    66h
  asm  push  cx
  asm  db    66h
  asm  popf
  asm  mov   sp,bx

  asm  db    66h
  asm  xor   ax,cx
  asm  mov   ax,386
  asm  je    testprot

is486:
  asm  mov   ax,486

testprot:
  asm  smsw  cx
  asm  ror   cx,1
  asm  jnc   exit
  asm  neg   ax

exit:
  asm  pop   bp
  asm  mov   CPUvalue,ax     /* store the value in case we need it again */
  return(_AX);
}

#ifdef TEST
#include <stdio.h>
#include <memcheck.h>

void main(void) {
  int  X;
  char *msg;

  X = cputype();

  switch (X) {
    case    86: msg = "8086 / 8088"; break;
    case   186: msg = "80186 / 80188"; break;
    case   286: msg = "80286 (real mode)"; break;
    case  -286: msg = "80286 (protected mode)"; break;
    case   386: msg = "80386 (real mode)"; break;
    case  -386: msg = "80386 (protected mode)"; break;
    case   486: msg = "80486 (real mode)"; break;
    case  -486: msg = "80486 (protected mode)"; break;
    default   : msg = "unknown"; break;
  }
  puts(msg);
}
#endif
