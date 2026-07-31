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

#include <model.h>

/********************************************************************
 *
 *  Function:  twodigit()
 *
 *  Takes an integer when received but returns a "two-digit" string equivalent
 *  with a leading space (' ') if necessary
 */

void pascal twodigit(int *Num) {   /*  Num is an INT when received     */
                                        /*  but is returned as 2 characters */
#ifdef SDATA
  asm  Mov  Si,Num
#else
  asm  Push Ds
  asm  Lds  Si,Num
#endif

  asm  Mov  Ax,[Si]
  asm  Aam
  asm  Or   Ax,'00'
  asm  Xchg Al,Ah
  asm  Cmp  Al,'0'
  asm  Jne  End
  asm  Mov  Al,' '
End:
  asm  Mov  [Si],Ax

#ifdef LDATA
  asm  Pop  Ds
#endif
}
