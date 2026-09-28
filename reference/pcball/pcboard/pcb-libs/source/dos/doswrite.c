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
#pragma inline
#include <model.h>
#endif

#include <io.h>
#include <stdio.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static void (LIBENTRY *doswritecallback)(char *FileName,int Handle,unsigned BytesWritten,int ExtendedError);

/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Returns :
*/

int LIBENTRY doswrite(int handle, void *buf, unsigned len DOS2ERROR) {
  unsigned BytesWritten;

  #ifdef __OS2__
    APIRET  ExtendedError;
    ULONG   NewBytesWritten;

    ExtendedError = DosWrite(handle,buf,len,&NewBytesWritten);
    if (ExtendedError != 0) {
      Os2Error->ExtendedError = ExtendedError;
      getextendederror(Os2Error);
    } else
      BytesWritten = (unsigned) NewBytesWritten;

  #else

    #ifdef SDATA
      asm  mov  dx,buf
    #else
      asm  push ds
      asm  lds  dx,buf
    #endif

      asm  mov  ah,40h
      asm  mov  bx,handle
      asm  mov  cx,len
      int21();

    #ifdef LDATA
      asm  pop  ds
    #endif

      ExtendedError = 0;
      asm  mov  BytesWritten,ax
      asm  jc   exterr

      asm  cmp  ax,len             /* if BytesWritten != len then set the   */
      asm  je   end
      ExtendedError = 39;          /* extended error code to 39 to indicate */
      ExtendedAction = 3;          /* a possible disk full condition        */
      asm  jmp  short end

    exterr:
      getextendederror();
  #endif

end:
  if (doswritecallback != NULL)
    doswritecallback(OpenFileNames[handle],handle,BytesWritten,ExtendedError);

  return(BytesWritten);    /* return the number of bytes written */
}


void LIBENTRY setdoswritecallback(void (LIBENTRY *cb)(char *FileName,int Handle,unsigned BytesWritten,int ExtendedError)) {
  doswritecallback = cb;
}
