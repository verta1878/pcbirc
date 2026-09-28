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
/*  #pragma inline */
  #endif
#endif

#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/*============================================================================
 Make sure a specific bit is NOT set on
 ===========================================================================*/

void LIBENTRY unsetbit(void _FAR_ *BitStream, int BitNum) {
#ifdef __OS2__
  char *p = (char *) BitStream;

   p += (BitNum >> 3);  /* add BitNum/8 to the pointer */
  *p &= (char) ~(1 << (BitNum & 0x07));

#else  /* ifdef __OS2__ */

  asm  Mov Ax,BitNum     /* AX equals the number of the bit to be found   */
  asm  Mov Cx,Ax         /* save the BitNum value in CX for use below     */
  asm  Shr Ax,1          /* calculate BitNum divided by 8                 */
  asm  Shr Ax,1
  asm  Shr Ax,1          /* Ax = quotient of BitNum divided by 8          */

  asm  Les Bx,BitStream  /* Es:Bx points to the array of bits BitStream   */
  asm  Add Bx,Ax         /* add quotient to Bx to point to the right byte */

  asm  And Cl,7          /* Cl = remainder of BitNum/8                    */
  asm  Mov Dl,1          /* Make Dx into a bit mask with one bit set      */
  asm  Shl Dl,Cl         /* Dx = (1 shl remainder) = 2^remainder          */
  asm  Not Dl            /* Now turn all bits on EXCEPT that 1 bit        */
  asm  And Es:[Bx],Dl    /* Adjust Di by quotient, AND value with Dl      */
#endif  /* ifdef __OS2__ */
}
