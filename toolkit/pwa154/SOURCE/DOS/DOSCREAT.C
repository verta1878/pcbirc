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
/*#pragma inline */
#include "model.h"
#endif

#include <string.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static void (LIBENTRY *doscreatecallback)(char *FileName,int OpenFlags,int Attributes,int Handle,int ExtendedError);

/********************************************************************
*
*  Function:  doscreate()
*
*  Desc    :  creates a new file or truncates an existing file to 0-bytes.
*
*  Returns :  File Handle when successful
*             Or, -1 when an error occurs in which case ExtendedErrror and
*             ExtendedAction are set by calling DOS Get Extended Error
*/

#pragma argsused
int LIBENTRY doscreate(char *FileName, int OpenFlags, int Attributes DOS2ERROR) {
  int Handle;

  #ifdef __OS2__
    ULONG   ActionTaken;
    ULONG   NewHandle;
    APIRET  ExtendedError;

/* check to see if any permission flags are set */
    if ((OpenFlags & 0x70) == 0) {
/* if no permission flags are set, then OR in the DENY_RDWR flag */
/* since that is equivalent to the default.  OS/2 apparently doesn't */
/* use DOS's default so it needs an explicity DENY_RDWR status. */
      OpenFlags |= OPEN_DENYRDWR;
    }

    ExtendedError = DosOpen(FileName,
                            &NewHandle,
                            &ActionTaken,
                            0,0,
                            OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_REPLACE_IF_EXISTS,
                            OpenFlags,
                            NULL);


    if (ExtendedError != 0) {
      Os2Error->ExtendedError = ExtendedError;
      getextendederror(Os2Error);
      Handle = -1;
    } else {
      Handle = (int) NewHandle;

      if (Handle > MAXHANDLES) {
        /* too many file handles!  close it down now and fake a "too many */
        /* files open" error to avoid corrupting memory on the strcpy()   */
        dosclose(Handle);
        Os2Error->ExtendedError  = 4;    /* too many files open */
        Os2Error->ExtendedClass  = 14;   /* this is our own error class */
        Os2Error->ExtendedAction = 3;
        Os2Error->ExtendedLocus  = 2;
        Handle = -1;
        goto end;
      }

      strcpy(OpenFileNames[Handle],FileName);
    }

  #else

    #ifdef SDATA
      asm  mov  dx,FileName
    #else
      asm  push ds
      asm  lds  dx,FileName
    #endif

      asm  mov  ah,3Ch        /* dos: open call */
      asm  mov  cx,Attributes
      int21();

    #ifdef LDATA
      asm  pop  ds
    #endif

      asm  jc   error
      asm  mov  Handle,ax
      asm  jmp  short end

    error:
      getextendederror();
      asm  mov  word ptr Handle,-1   /* indicate an error return   */
  #endif

end:
  if (doscreatecallback != NULL)
    doscreatecallback(FileName,OpenFlags,Attributes,Handle,ExtendedError);

  #ifndef __OS2__
    asm  mov  bx,Handle
    asm  mov  ah,3Eh        /* dos: close file */
    int21();

    dosopen(FileName,OpenFlags);  /* reopen it using the regular call */
    asm  mov  Handle,ax
  #endif

  return(Handle);
}


void LIBENTRY setdoscreatecallback(void (LIBENTRY *cb)(char *FileName,int OpenFlags,int Attributes,int Handle,int ExtendedError)) {
  doscreatecallback = cb;
}
