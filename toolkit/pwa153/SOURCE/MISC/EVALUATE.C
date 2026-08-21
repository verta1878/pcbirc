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


/*  this module has not been converted to OS/2 */

#ifndef __OS2__

#include <stdio.h>
#include <string.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

#pragma inline

long pascal evaluate(void far *p, unsigned Size) {
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
}

/*
char Array[50000U];

void main(void) {

  memset(Array,255,sizeof(Array));
  printf("%lu\n",evaluate(Array,sizeof(Array)));
  Array[49999U] = 254;
  printf("%lu\n",evaluate(Array,sizeof(Array)));
}
*/


#endif  /* ifndef __OS2__ */