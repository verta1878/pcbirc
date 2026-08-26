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



/*#pragma inline */

/********************************************************************
 *
 *  Function:  timechanged()
 *
 *  Returns TRUE if the time has changed since the last call
 */

char static OldSec;

char pascal timechanged(void) {
  asm Mov Ah,2Ch
  asm Int 21h
  asm Xor Ax,Ax
  asm Cmp Dh,OldSec
  asm Je  exit
  asm Mov OldSec,Dh
  asm Mov Al,1
exit:;
  return(_AL);
}
