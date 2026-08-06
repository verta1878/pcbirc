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


#ifdef __OS2__
#define INCL_DOSMISC
#include <os2.h>
#elif defined(__WATCOMC__)
#include <i86.h>
#else
//#pragma inline
#include "model.h"
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifndef __OS2__
int  ExtendedError;
char ExtendedClass;
char ExtendedAction;
char ExtendedLocus;
#endif


/********************************************************************
*
*  Function:  getextendederror()
*
*  Desc    :  calls dos after an error to determine the extended error values
*/

#ifdef __OS2__
void LIBENTRY getextendederror(os2errtype *Os2Error) {
  DosErrClass(Os2Error->ExtendedError,
	      &Os2Error->ExtendedClass,
	      &Os2Error->ExtendedAction,
	      &Os2Error->ExtendedLocus);
}

#ifdef __WATCOMC__
  {
    union REGS r;
    r.h.ah = 0x59;
    r.w.bx = 0;
    int386(0x21, &r, &r);
    ExtendedError = r.w.ax;
    ExtendedClass = r.h.bh;
    ExtendedAction = r.h.bl;
    ExtendedLocus = r.h.ch;
  }
#else

void LIBENTRY getextendederror(void) {
  asm  push si
  asm  push di
  asm  mov  ah,59h        /* dos: get extended error */
  asm  xor  bx,bx
  int21();
  asm  mov  ExtendedError,ax     /* store extended error code  */
  asm  mov  ExtendedClass,bh     /* store extended class code */
  asm  mov  ExtendedAction,bl    /* store extended action code */
  asm  mov  ExtendedLocus,ch     /* store extended locus code */
  asm  pop  di
#endif
  asm  pop  si
}
#endif
