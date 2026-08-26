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


/* this module has not been converted to OS/2 */

#ifndef __OS2__

/*#pragma inline */
#include <model.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


void pascal dadd(dlong *Arg1, dlong *Arg2, dlong *Sum) {
#ifdef LDATA
  asm push ds
  asm lds  si,Arg1
  asm les  di,Sum
#else
  asm mov  si,Arg1
  asm mov  ax,ds
  asm mov  es,ax
  asm mov  di,Sum
#endif

  asm cld
  asm mov  cx,4
  asm rep  movsw

#ifdef LDATA
  asm lds  si,Arg2
  asm les  di,Sum
#else
  asm mov  si,Arg2
  asm mov  di,Sum
#endif

  asm lodsw
  asm add  es:[di],ax
  asm lodsw
  asm adc  es:[di+2],ax
  asm lodsw
  asm adc  es:[di+4],ax
  asm lodsw
  asm adc  es:[di+6],ax

#ifdef LDATA
  asm pop  ds
#endif
}


void pascal dsub(dlong *Arg1, dlong *Arg2, dlong *Sum) {
#ifdef LDATA
  asm push ds
  asm lds  si,Arg1
  asm les  di,Sum
#else
  asm mov  si,Arg1
  asm mov  ax,ds
  asm mov  es,ax
  asm mov  di,Sum
#endif

  asm cld
  asm mov  cx,4
  asm rep  movsw

#ifdef LDATA
  asm lds  si,Arg2
  asm les  di,Sum
#else
  asm mov  si,Arg2
  asm mov  di,Sum
#endif

  asm lodsw
  asm sub  es:[di],ax
  asm lodsw
  asm sbb  es:[di+2],ax
  asm lodsw
  asm sbb  es:[di+4],ax
  asm lodsw
  asm sbb  es:[di+6],ax

#ifdef LDATA
  asm pop  ds
#endif
}


/* Arg1 < Arg2, returns -1 */
/* Arg1 > Arg2, returns  1 */
/* Arg1 = Arg2, returns  0 */
int pascal dcmp(dlong *Arg1, dlong *Arg2) {
#ifdef LDATA
  asm push ds
  asm lds  si,Arg1
  asm les  di,Arg2
#else
  asm mov  si,Arg1
  asm mov  ax,ds
  asm mov  es,ax
  asm mov  di,Arg2
#endif

  asm mov  ax,[si+6]
  asm cmp  ax,es:[di+6]
  asm jg   greater
  asm jl   lesser

  asm mov  ax,[si+4]
  asm cmp  ax,es:[di+4]
  asm jg   greater
  asm jl   lesser

  asm mov  ax,[si+2]
  asm cmp  ax,es:[di+2]
  asm jg   greater
  asm jl   lesser

  asm mov  ax,[si]
  asm cmp  ax,es:[di]
  asm jg   greater
  asm jl   lesser
  asm xor  ax,ax
  asm jmp  short done

greater:;
  asm mov  ax,1
  asm jmp  short done

lesser:;
  asm mov  ax,-1

done:;
#ifdef LDATA
  asm pop  ds
#endif
  return(_AX);
}


void pascal dmul(long Arg1, long Arg2, dlong *Product) {
  int  w0;
  int  w1;
  int  w2;
  int  w3;

  asm mov dx,Arg1+2
  asm mov ax,Arg1        /* DX:AX = A */
  asm mov cx,Arg2+2
  asm mov bx,Arg2        /* CX:BX = B */

  asm mov di,dx
  asm mov si,ax

  asm mul bx
  asm mov w0,ax
  asm mov w1,dx

  asm mov ax,di
  asm mul cx
  asm mov w2,ax
  asm mov w3,dx

  asm mov ax,di
  asm mul bx
  asm add w1,ax
  asm adc w2,dx
  asm adc word ptr w3,0

  asm mov ax,si
  asm mul cx
  asm add w1,ax
  asm adc w2,dx
  asm adc word ptr w3,0

  Product->w0 = w0;
  Product->w1 = w1;
  Product->w2 = w2;
  Product->w3 = w3;
}


long pascal ddiv(dlong *Dividend, long Divisor) {
  int  w0;
  int  w1;
  int  w2;
  int  w3;

  w0 = Dividend->w0;
  w1 = Dividend->w1;
  w2 = Dividend->w2;
  w3 = Dividend->w3;

  asm mov  dx,w3
  asm mov  cx,w2
  asm mov  bx,w1
  asm mov  ax,w0
  asm mov  si,Divisor+2
  asm mov  di,Divisor

  asm push bp
  asm mov  bp,cx
  asm mov  cx,32
  asm clc

ddiv1:
  asm rcl  ax,1
  asm rcl  bx,1
  asm rcl  bp,1
  asm rcl  dx,1
  asm jnc  ddiv3

ddiv2:
  asm sub  bp,di
  asm sbb  dx,si
  asm stc
  asm loop ddiv1
  asm jmp  ddiv5

ddiv3:
  asm cmp  dx,si
  asm jc   ddiv4
  asm jne  ddiv2
  asm cmp  bp,di
  asm jnc  ddiv2

ddiv4:
  asm clc
  asm loop ddiv1
ddiv5:
  asm rcl  ax,1
  asm rcl  bx,1
  asm mov  cx,bp
  asm xchg dx,bx
  asm xchg cx,bx
  asm pop  bp

  /* ignore RETURN error cuz Quotient is in DX:AX already */
}


void pascal ltodlong(long Arg1, dlong *Arg2) {
  Arg2->w0 = (int)  (Arg1 & 0x0000FFFF);
  Arg2->w1 = (int) ((Arg1 & 0xFFFF0000L) >> 16);
  Arg2->w2 = 0;
  Arg2->w3 = 0;
}


long pascal dlongtolong(dlong *Arg1) {
  long Temp;

  Temp = (unsigned) Arg1->w0 + ((long) Arg1->w1 << 16);
  if (Arg1->w3 & 0xF000)  /* is it negative? */
    return(-Temp);
  return(Temp);
}


#ifdef TEST
#include <stdio.h>

void main(void) {
  dlong A;
  dlong B;

  dmul(256,256,&A);
  dadd(&A,&A,&B);
  printf("Quotient = %ld\n",ddiv(&B,1));

  dmul(102400000,102400000,&A);
  printf("Quotient = %ld\n",ddiv(&A,51200000));
}
#endif
#endif  /* ifndef __OS2__ */