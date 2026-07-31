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


//#pragma inline

#include <stdio.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

/*============================================================================
 SetupReg subroutine ... placed INLINE

 Called with:    Ds:Di points to Conference String
                 Bx    equals the conference number

 Returns:        Al    equal to character at offset in Conference String
                 Dl    equal to bit mask for Al

 Notes:          modifies Ax,Bx,Dx .. Cx is preserved
 ===========================================================================

  asm     Push  Cx
  asm     Dec Bx            /* calculate (ConfNum-1)/8 and store the */
  asm     Mov Cl,Bl
  asm     And Cl,7          /* Cl = remainder of (ConfNum-1)/8 */
  asm     Shr Bx,1
  asm     Shr Bx,1
  asm     Shr Bx,1          /* Bx = quotient of (ConfNum-1)/8 */

  asm     Mov   Al,[Di+Bx]  /* Adjust Di by quotient and get char value */

  asm     Mov   Dl,1        /* Make Dx into a bit mask with one bit set */
  asm     Shl   Dl,Cl       /* Dx = (1 shl remainder) = 2^remainder */
  asm     Pop   Cx
*/


/*============================================================================
    Create a 40-character registration string
 ============================================================================*/

void pascal getconf(char far *LongStr, char far *ShortStr) {
  asm     Push  Ds
  asm     Push  Es
  asm     Lds   Di,ShortStr       /* Ds:Di points to ShortStr */

  asm     Les   Si,LongStr        /* Es:Si points to LongStr */
  asm     Add   Si,40             /* Si points to end of LongStr+1 */
  asm     Mov   byte ptr Es:[Si],0/* Store null terminator */
  asm     Dec   Si                /* Si points to end of LongStr */
  asm     Mov   Cx,40             /* Set loop to 40 counts */

gcLoop:
  asm     Mov   Bx,Cx             /* Scan thru string (40 bytes) */

  asm     Push  Cx
  asm     Dec Bx            /* calculate (ConfNum-1)/8 and store the */
  asm     Mov Cl,Bl
  asm     And Cl,7          /* Cl = remainder of (ConfNum-1)/8 */
  asm     Shr Bx,1
  asm     Shr Bx,1
  asm     Shr Bx,1          /* Bx = quotient of (ConfNum-1)/8 */

  asm     Mov   Al,[Di+Bx]  /* Adjust Di by quotient and get char value */

  asm     Mov   Dl,1        /* Make Dx into a bit mask with one bit set */
  asm     Shl   Dl,Cl       /* Dx = (1 shl remainder) = 2^remainder */
  asm     Pop   Cx

  asm     And   Al,Dl             /* Test if bit (Dl bit mask) is in Al */
  asm     Jz    gcJ1
  asm     Mov   byte ptr Es:[Si],'X' /* If so, put an 'X' in the string */
  asm     Jmp   short gcJ2
gcJ1:
  asm     Mov   byte ptr Es:[Si],' ' /* else put a space in the string */
gcJ2:
  asm     Dec   Si
  asm     Loop  gcLoop

  asm     Pop   Es
  asm     Pop   Ds
}

/*============================================================================
    Put a 40-character registration string into 5 bytes
 ============================================================================*/

void pascal putconf(char far *LongStr, char far *ShortStr) {
  asm     Push  Ds
  asm     Push  Es
  asm     Lds   Di,ShortStr       /* Ds:Di points to ShortStr */

  asm     Les   Si,LongStr        /* Es:Si points to LongStr */
  asm     Add   Si,39             /* Si points to end of LongStr */
  asm     Mov   Cx,40             /* Set loop to 40 counts */

pcLoop:
  asm     Mov   Bx,Cx             /* Scan thru string (40 bytes) */

  asm     Push  Cx
  asm     Dec Bx            /* calculate (ConfNum-1)/8 and store the */
  asm     Mov Cl,Bl
  asm     And Cl,7          /* Cl = remainder of (ConfNum-1)/8 */
  asm     Shr Bx,1
  asm     Shr Bx,1
  asm     Shr Bx,1          /* Bx = quotient of (ConfNum-1)/8 */

  asm     Mov   Al,[Di+Bx]  /* Adjust Di by quotient and get char value */

  asm     Mov   Dl,1        /* Make Dx into a bit mask with one bit set */
  asm     Shl   Dl,Cl       /* Dx = (1 shl remainder) = 2^remainder */
  asm     Pop   Cx

  asm     Cmp   byte ptr Es:[Si],'X'/* If 'X' then */
  asm     Je    pcJ1              /* jump to pcJ1 to add bit to string */
  asm     Not   Dl                /* Else remove bit from string */
  asm     And   Al,Dl
  asm     Jmp   short pcJ2
pcJ1:
  asm     Or    Al,Dl
pcJ2:
  asm     Mov   [Di+Bx],Al
  asm     Dec   Si
  asm     Loop  pcLoop

  asm     Pop   Es
  asm     Pop   Ds
}
