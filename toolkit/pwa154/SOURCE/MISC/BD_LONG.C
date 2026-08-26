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
/*#pragma inline */
  #define EXPBYTE   7
  #define SIGNBYTE  6
  #define SIGBITS  55
  #pragma warn -par
  #pragma warn -rvl
  #include "model.h"
#endif

#include "misc.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

static ulong _NEAR_ LIBENTRY basdbletolongsub(basdble cArray) {
#if defined(_MSC_VER) || defined(__OS2__)
  unsigned long l;

/*
  l = ((long)cArray[6] << 24) + ((long)cArray[5] << 16) + ((unsigned) cArray[4] << 8) + cArray[3];
  l = l | 0x80000000L;
  l = (l >> (32-(cArray[7] - 0x80)));
*/

  if (cArray[7] == 0 || cArray[7] == 0x80)
    return(0);

  l = ((*(unsigned long*)(cArray+3)) | 0x80000000L) >> (32-(cArray[7]-0x80));

  if (cArray[6] & 0x80)
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
      asm sub cl,48
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
      asm sub cl,40
      asm shr al,cl
      asm add bx,ax
    }
    if (_DI != 0) {
      asm mov ax,di
      asm mov cl,48
      asm sub cl,ch
      asm shl ax,cl
      asm add bx,ax
      asm adc dx,0
    }
    goto done;
  }

  if (_CL < 23) {
    asm mov al,[si+SIGNBYTE-2]
    if (_AL != 0) {
      asm xor ah,ah
      asm mov cl,ch
      asm sub cl,32
      asm shr al,cl
      asm add bx,ax
    }
    asm mov al,[si+SIGNBYTE-1]
    if (_AL != 0) {
      asm xor ah,ah
      asm mov cl,40
      asm sub cl,ch
      asm shl ax,cl
      asm add bx,ax
      asm adc dx,0
    }
    if (_DI != 0) {
      asm mov ax,di
      asm mov cl,40          /* take 8 off and we'll adjust our addressing */
      asm sub cl,ch
      asm shl ax,cl
      asm add bh,al
      asm adc dl,ah
      asm adc dh,0
    }
    goto done;
  }

  asm mov al,[si+SIGNBYTE-3]
  if (_AL != 0) {
    asm xor ah,ah
    asm mov cl,ch
    asm sub cl,24
    asm shr al,cl
    asm add bx,ax
  }
  asm mov al,[si+SIGNBYTE-2]
  if (_AL != 0) {
    asm xor ah,ah
    asm mov cl,32
    asm sub cl,ch
    asm shl ax,cl
    asm add bx,ax
    asm adc dx,0
  }
  asm mov al,[si+SIGNBYTE-1]
  if (_AL != 0) {
    asm xor ah,ah
    asm mov cl,32          /* take 8 off and we'll adjust our addressing */
    asm sub cl,ch
    asm shl ax,cl
    asm add bh,al
    asm adc dl,ah
    asm adc dh,0
  }
  if (_DI != 0) {
    asm mov ax,di
    asm mov cl,32          /* take 16 off and we'll adjust our addressing */
    asm sub cl,ch
    asm shl ax,cl
    asm add dx,ax
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


long LIBENTRY basdbletolong(basdble cArray) {
  long l = basdbletolongsub(cArray);
  return(l);
}


ulong LIBENTRY basdbletoulong(basdble cArray) {
  return(basdbletolongsub(cArray));
}
