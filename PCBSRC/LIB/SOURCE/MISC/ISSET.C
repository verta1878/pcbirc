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
//  #pragma inline
  #endif
#endif

#include "misc.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

/*============================================================================
 Check to see if a specific bit is set
 ===========================================================================*/
#pragma warn -rvl
int LIBENTRY isset(void _FAR_ *BitStream, int BitNum) {
#if defined(__OS2__) || defined(__WATCOMC__)
  char *p = (char *) BitStream + (BitNum >> 3);  // add BitNum/8 to the pointer
  return(*p & (char) (1 << (BitNum & 0x07)));

#else  /* ifdef __OS2__ */

  asm  Mov Ax,BitNum     /* AX equals the number of the bit to be found   */
  asm  Mov Cx,Ax         /* save the BitNum value in CX for use below     */
  asm  Shr Ax,1          /* calculate BitNum divided by 8                 */
  asm  Shr Ax,1
  asm  Shr Ax,1          /* Ax = quotient of BitNum divided by 8          */

  asm  Les Bx,BitStream  /* Es:Bx points to the array of bits BitStream   */
  asm  Add Bx,Ax         /* add quotient to Bx to point to the right byte */
  asm  Mov Al,Es:[Bx]    /* get the byte from the array                   */

  asm  And Cl,7          /* Cl = remainder of BitNum/8                    */
  asm  Shr Al,Cl         /* move the bits over to the first position      */
  asm  And Ax,1          /* and throw the rest of the bits away           */

/*return;*//* ignore return value error, it's already in AX */
#endif  /* ifdef __OS2__ */
}
