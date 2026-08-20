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

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static void (LIBENTRY *dosreadcallback)(char *FileName,int Handle,unsigned BytesRead,int ExtendedError);

/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Returns :
*/

int LIBENTRY dosread(int handle, void *buf, unsigned len DOS2ERROR) {
  unsigned BytesRead;

  #ifdef __OS2__
    APIRET  ExtendedError;
    ULONG   NewBytesRead;

    ExtendedError = DosRead(handle,buf,len,&NewBytesRead);
    if (ExtendedError != 0) {
      Os2Error->ExtendedError = ExtendedError;
      getextendederror(Os2Error);
      NewBytesRead = (unsigned) -1;
    } else {
      /* if ExtendedError==0, then no error occured, so if len!=NewBytesRead */
      /* then we must have reached the end of the file */
      if (len != NewBytesRead) {
        Os2Error->ExtendedError = 40;
        Os2Error->ExtendedAction = 3;
      }
    }

    BytesRead = (unsigned) NewBytesRead;
  #else

    #ifdef SDATA
      asm  mov  dx,buf
    #else
      asm  push ds
      asm  lds  dx,buf
    #endif

      asm  mov  ah,3Fh
      asm  mov  bx,handle
      asm  mov  cx,len
      int21();

    #ifdef LDATA
      asm  pop  ds
    #endif

      ExtendedError = 0;
      asm  mov  BytesRead,ax
      asm  jnc  skipexterr

      getextendederror();
      asm  jmp  short end

    skipexterr:
      asm  mov  ax,len             /* if BytesRead != len then set the      */
      asm  cmp  BytesRead,ax       /* extended error code to 40 to indicate */
      asm  je   end                /* an end of file condition              */
      ExtendedError = 40;
      ExtendedAction = 3;
  #endif

end:
  if (dosreadcallback != NULL)
    dosreadcallback(OpenFileNames[handle],handle,BytesRead,ExtendedError);

  return(BytesRead);    /* return the number of bytes read */
}


void LIBENTRY setdosreadcallback(void (LIBENTRY *cb)(char *FileName,int Handle,unsigned BytesRead,int ExtendedError)) {
  dosreadcallback = cb;
}
