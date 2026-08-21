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
  #define INCL_VIO
  #include <os2.h>
#elif defined(__WATCOMC__)
  #include <i86.h>
#elif defined(_MSC_VER)
  #include <borland.h>
#endif

#include <screen.h>

char LIBENTRY wherex(void) {
#ifdef __OS2__
  USHORT Row, Column;
  VioGetCurPos(&Row,&Column,0);
  Scrn_X = (char) Column;
  return((char) Column);
#elif defined(__WATCOMC__)
  union REGS r;
  r.h.ah = 3;
  r.h.bh = 0;
  int386(0x10, &r, &r);
  return (char) r.h.dl;
#else
  asm  Mov  Ah,3
  asm  Xor  Bh,Bh
  asm  Int  10h
  asm  Mov  Al,Dl
#endif
}
