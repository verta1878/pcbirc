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


#if ! (defined(_MSC_VER) || defined(__OS2__))
/*#pragma inline */
  #include "model.h"
#endif

#include <stdio.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define EXPBYTE   7
#define SIGNBYTE  6
#define SIGBITS  55

#pragma warn -par


static void _NEAR_ LIBENTRY longtobasdblesub(basdble cArray, ulong Num, bool IsNeg) {
#if defined(_MSC_VER) || defined(__OS2__)
  int      index;
  unsigned HighNum;

  if (IsNeg)
    Num = -Num;

  if (Num > 0xFFFF) {
    HighNum = (int) (Num >> 16);
    for (index = 0; index < 16; index++)
      if ((1U << index) > (unsigned int) HighNum)
        break;
    index += 16;
  } else {
    for (index = 0; index < 16; index++)
      if ((1U << index) > (unsigned int) Num)
        break;
  }

  Num = (Num << (32 - index));

  cArray[7] = (char) (index + 0x80);
  cArray[6] = (char) ((Num & 0x7F000000L) >> 24);
  cArray[5] = (char) ((Num & 0xFF0000L) >> 16);
  cArray[4] = (char) (((int) Num & 0xFF00) >> 8);
  cArray[3] = (char) ((int) Num & 0x00FF);
  cArray[2] = 0;
  cArray[1] = 0;
  cArray[0] = 0;

  if (IsNeg)
    cArray[6] |= 0x80;

#else
  #ifdef LDATA
    asm push ds
    asm lds  si,cArray
  #else
    asm mov si,cArray
  #endif

  asm xor ax,ax
  asm mov [si],ax
  asm mov [si+2],ax
  asm mov [si+4],ax
  asm mov [si+6],ax

  asm mov bx,word ptr Num[0]
  asm mov dx,word ptr Num[2]
  asm mov ax,bx
  asm or  ax,dx
  asm jnz notzero
  goto exit;

notzero:

  asm mov  al,IsNeg
  asm or   al,al
  asm jz   notneg
  asm neg  dx
  asm neg  bx
  asm sbb  dx,0
  asm mov  byte ptr [si+SIGNBYTE],80h  /* turn on sign bit */

notneg:

  /* Register usage is as follows:                                */
  /*                                                              */
  /*   DX:BX  is the long integer return value (later DX:AX)      */
  /*      AX  is a temporary variable used for calculations       */
  /*      CH  holds the SHIFT value from the expression (23-Exp)  */
  /*      CL  is used for bit shifting                            */
  /*      SI  is a pointer to cArray                              */
  /*      DI  is the EXP value for testing                        */

  asm mov cx,-1
  if (_DX == 0) {   /* less than 65536? */
    asm mov ax,bx
top1:
    asm inc cx
    asm shr ax,1
    asm jnz top1

    asm mov di,cx
    asm mov ax,1
    asm shl ax,cl
    asm dec ax
    asm and bx,ax
  } else {
    asm mov ax,dx
top2:
    asm inc cx
    asm shr ax,1
    asm jnz top2

    asm mov di,cx
    asm add di,16
    asm mov ax,1
    asm shl ax,cl
    asm dec ax
    asm and dx,ax
  }

  asm mov ax,di
  asm mov ch,SIGBITS
  asm sub ch,al
  asm add al,81h
  asm mov [Si+EXPBYTE],al

  if (_DI < 8) {
    asm mov cl,ch
    asm sub cl,48
    asm mov al,bl
    asm shl al,cl
    asm or  [Si+SIGNBYTE],al
    goto exit;
  }

  if (_DI < 15) {
    asm mov cl,ch
    asm sub cl,40
    asm mov al,bl
    asm shl al,cl
    asm mov [Si+SIGNBYTE-1],al

    asm mov cl,48
    asm sub cl,ch
    asm mov ax,bx
    asm shr ax,cl
    asm or  [Si+SIGNBYTE],al
    goto exit;
  }

  if (_DI < 23) {
    asm mov cl,ch
    asm sub cl,32
    asm mov al,bl
    asm shl al,cl
    asm mov [Si+SIGNBYTE-2],al

    asm mov cl,40
    asm sub cl,ch
    asm mov ax,bx
    asm shr ax,cl
    asm mov [Si+SIGNBYTE-1],al

    asm mov cl,40
    asm sub cl,ch
    asm mov al,bh
    asm mov ah,dl
    asm shr ax,cl
    asm or  [Si+SIGNBYTE],al
    goto exit;
  }

  asm mov cl,ch
  asm sub cl,24
  asm mov al,bl
  asm shl al,cl
  asm mov [Si+SIGNBYTE-3],al

  asm mov cl,32
  asm sub cl,ch
  asm mov ax,bx
  asm shr ax,cl
  asm mov [Si+SIGNBYTE-2],al

  asm mov cl,32
  asm sub cl,ch
  asm mov ah,dl
  asm mov al,bh
  asm shr ax,cl
  asm mov [Si+SIGNBYTE-1],al

  asm mov cl,32
  asm sub cl,ch
  asm mov ax,dx
  asm shr ax,cl
  asm or  [Si+SIGNBYTE],al

exit:;
  #ifdef LDATA
    asm pop ds
  #endif
#endif
}


void LIBENTRY longtobasdble(basdble cArray, long Num) {
  longtobasdblesub(cArray,Num,(bool) (Num < 0));
}

void LIBENTRY ulongtobasdble(basdble cArray, ulong Num) {
  longtobasdblesub(cArray,Num,FALSE);
}
#pragma warn +par
