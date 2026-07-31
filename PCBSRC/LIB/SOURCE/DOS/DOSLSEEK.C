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
//#pragma inline
#include <asmrules.h>
#include "model.h"
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


long LIBENTRY doslseek(int handle, long distance, char method) {
  #ifdef __OS2__
    APIRET rc;
    ULONG  NewOffset;

    rc = DosSetFilePtr(handle,distance,method,&NewOffset);
    if (rc != 0)
      goto error;

    return(NewOffset);
  #else
    asm mov ah,42h
    asm mov al,method
    asm mov bx,handle
    asm mov dx,word ptr distance
    asm mov cx,word ptr distance+2
    int21();
    asm jc  error
    #ifdef __cplusplus
      return( MK_LONG );
    #else
      return;  /* ignore the error, the return value is in DX:AX */
    #endif
  #endif

error:
  return(-1L);

}
