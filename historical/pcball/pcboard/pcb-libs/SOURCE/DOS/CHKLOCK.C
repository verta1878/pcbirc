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
#endif

#include <io.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function:  doslockcheck()
*
*  Desc    :
*
*  Returns :
*/

int LIBENTRY doslockcheck(int handle, long fileoffset, long rangelength) {
  unsigned Retry;
  #ifdef __OS2__
    FILELOCK   UnLock = {0,0};
    FILELOCK   Lock   = {fileoffset,rangelength};
    APIRET     rc;
    os2errtype Os2Error;
  #endif

  Retry = 0;

j1:
   #ifdef __OS2__
     if ((rc = DosSetFileLocks(handle,&UnLock,&Lock,250,0)) != 0)
       goto lockFailed;
   #else
     asm  mov     ax, 05C00h
     asm  mov     bx, handle
     asm  mov     cx, word ptr fileoffset+2
     asm  mov     dx, word ptr fileoffset
     asm  mov     si, word ptr rangelength+2
     asm  mov     di, word ptr rangelength

     int21();

     asm  jc      lockFailed
     asm  mov     ExtendedError,0
   #endif
   return(0);

lockFailed:

    #ifdef __OS2__
      Os2Error.ExtendedError = rc;
      getextendederror(&Os2Error);
    #else
      getextendederror();
    #endif

    Retry = retrycount(Retry,OpenFileNames[handle],"Locking" POS2ERROR);
    if (Retry != 0xFFFF)
      goto j1;
    return(-1);
}
