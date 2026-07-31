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
#define INCL_DOSFILEMGR
#include <os2.h>
#else
#include <dos.h>
#ifdef _MSC_VER
  #define _version _osversion
#endif
//#pragma inline
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


int LIBENTRY doscommit(int Handle) {
  #ifdef __OS2__
    if (DosResetBuffer(Handle) != 0)
      goto error;
  #else
    if ((_version & 0x00FF) >= 4 || (_version & 0xFF00) >= 3) {
      asm  mov ah,68h
      asm  mov bx,Handle
      int21();
      asm  jc  error
    }
  #endif
  return(0);

error:
  return(-1);
}
