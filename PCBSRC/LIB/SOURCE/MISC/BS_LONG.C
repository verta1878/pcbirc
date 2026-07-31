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
//#pragma inline
  #include "model.h"
#endif

#include <stdio.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define EXPBYTE  3
#define SIGNBYTE 2
#define SIGBITS  23

#pragma warn -par
#pragma warn -rvl

long LIBENTRY bassngltolong(bassngl cArray) {
#if defined(_MSC_VER) || defined(__OS2__)
  unsigned long l;

/*
  l = ((long)cArray[2] << 16) + ((unsigned) cArray[1] << 8) + cArray[0];
  l = l | 0x800000L;
  l = (l >> (24-(cArray[3] - 0x80)));
*/

  if (cArray[3] == 0 || cArray[3] == 0x80)
    return(0);

  l = (((*(long*)cArray) & 0x00FFFFFFL) | 0x800000L) >> (24-(cArray[3]-0x80));

  if (cArray[2] & 0x80)
    l = -l;

  return(l);
#else
  char Sign;

  #ifdef LDATA
    asm push ds
    asm lds  si,cArray
  #else
    asm mov  si,cArray
  #endif

  asm mov al,[si+EXPBYTE]

  if (_AL == 0 || _AL == 0x80) {
    asm xor ax,ax
    asm mov dx,ax
    goto exit;
  }

  asm sub al,81h
  asm mov ch,SIGBITS
  asm sub ch,al
  asm mov cl,al

  /* lower 7 bits only - bit 8 is the sign bit */
  asm xor ax,ax
  asm mov al,[si+SIGNBYTE]
  asm mov bl,al
  asm and al,127
  asm mov di,ax

  asm and bl,128
  asm mov Sign,bl

  if (_CL < 16) {
    asm mov bx,1
    asm shl bx,cl
    asm xor dx,dx
  } else {
    asm sub cl,16
    asm xor bx,bx
    asm mov dx,1
    asm shl dx,cl
    asm add cl,16
  }

  /* Register usage is as follows:                                */
  /*                                                              */
  /*   DX:BX  is the long integer return value (later DX:AX)      */
  /*      AX  is a temporary variable used for calculations       */
  /*      CH  holds the SHIFT value from the expression (23-Exp)  */
  /*      CL  is used for bit shifting (starts out = to Exp)      */
  /*      SI  is a pointer to cArray                              */
  /*      DI  is a the value used in cArray[2] without the Sign   */

  if (_CL < 8) {
    if (_DI != 0) {
      asm mov ax,di
      asm mov cl,ch
      asm sub cl,16
      asm shr al,cl
      asm add bx,ax
    }
    goto done;
  }

  if (_CL < 15) {
    asm mov al,[si+SIGNBYTE-1]
    if (_AL != 0) {
      asm xor ah,ah
      asm mov cl,ch
      asm sub cl,8
      asm shr al,cl
      asm add bx,ax
    }
    if (_DI != 0) {
      asm mov ax,di
      asm mov cl,16
      asm sub cl,ch
      asm shl ax,cl
      asm add bx,ax
      asm adc dx,0
    }
    goto done;
  }

  asm mov al,[si+SIGNBYTE-2]
  if (_AL != 0) {
    asm xor ah,ah
    asm mov cl,ch
    asm shr al,cl
    asm add bx,ax
  }
  asm mov al,[si+SIGNBYTE-1]
  if (_AL != 0) {
    asm xor ah,ah
    asm mov cl,8
    asm sub cl,ch
    asm shl ax,cl
    asm add bx,ax
    asm adc dx,0
  }
  if (_DI != 0) {
    asm mov ax,di
    asm mov cl,8           /* take 8 off and we'll adjust our addressing */
    asm sub cl,ch
    asm shl ax,cl
    asm add bh,al
    asm adc dl,ah
    asm adc dh,0
  }

done:
  if (Sign) {
    asm neg dx
    asm neg bx
    asm sbb dx,0
  }

  asm mov ax,bx
  /* ignore "no return value" warning message it's already in DX:AX */
exit:;
  #ifdef LDATA
    asm pop ds
  #endif
#endif
}
#pragma warn +par
#pragma warn +rvl
