/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* EVALUATE.C — Custom hash/checksum of memory block                         */
/* Clark Development Company, Inc. (C) 1996. All Rights Reserved.            */
/* Watcom C conversion by pcbrevival (GPL v3.0 for our additions)            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef __OS2__
#include "model.h"
#ifdef DEBUG
#include <memcheck.h>
#endif
#endif

#include <string.h>
#include <stdio.h>

long pascal evaluate(void *p, unsigned Size) {
#if defined(__OS2__) || defined(__WATCOMC__)
  unsigned char *src = (unsigned char *)p;
  unsigned short bx = 0, dx = 0;
  unsigned i = 0;
  unsigned char val;
  int cl;
  if (Size == 0) return 0;
  while (i < Size) {
    val = src[i++];
    bx = (unsigned short)((bx & 0xFF00) | (((bx & 0xFF) + val) & 0xFF));
    cl = val & 0x0F;
    if (cl) bx = (unsigned short)((bx << cl) | (bx >> (16 - cl)));
    if (i >= Size) break;
    val = src[i++];
    bx = (unsigned short)((bx & 0x00FF) | ((((bx >> 8) + val) & 0xFF) << 8));
    cl = val & 0x0F;
    if (cl) bx = (unsigned short)((bx << cl) | (bx >> (16 - cl)));
    if (i >= Size) break;
    val = src[i++];
    dx = (unsigned short)((dx & 0xFF00) | (((dx & 0xFF) + val) & 0xFF));
    cl = val & 0x0F;
    if (cl) dx = (unsigned short)((dx << cl) | (dx >> (16 - cl)));
    if (i >= Size) break;
    val = src[i++];
    dx = (unsigned short)((dx & 0x00FF) | ((((dx >> 8) + val) & 0xFF) << 8));
    cl = val & 0x0F;
    if (cl) dx = (unsigned short)((dx << cl) | (dx >> (16 - cl)));
  }
  return ((long)dx << 16) | (long)bx;
#else
  if (Size == 0)
    return(0);

  asm push  ds
  asm lds   si,p
  asm cld
  asm mov   di,Size
  asm xor   bx,bx
  asm xor   dx,dx
top:
  asm lodsw
  asm add   bl,al
  asm mov   cl,al
  asm and   cl,1111b
  asm rol   bx,cl
  asm dec   di
  asm jz    end
  asm add   bh,ah
  asm mov   cl,ah
  asm and   cl,1111b
  asm rol   bx,cl
  asm dec   di
  asm jz    end
  asm lodsw
  asm add   dl,al
  asm mov   cl,al
  asm and   cl,1111b
  asm rol   dx,cl
  asm dec   di
  asm jz    end
  asm add   dh,ah
  asm mov   cl,ah
  asm and   cl,1111b
  asm rol   dx,cl
  asm dec   di
  asm jnz   top
end:
  asm mov   ax,bx
  asm pop   ds
#endif
}
