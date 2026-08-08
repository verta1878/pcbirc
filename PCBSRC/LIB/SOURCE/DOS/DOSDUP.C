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
#elif defined(__WATCOMC__)
#include <i86.h>
#else
//#pragma inline
#include "model.h"
#endif

#include <string.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

static void (LIBENTRY *dosdupcallback)(char *FileName,int NewHandle,int OldHandle,int ExtendedError);

/********************************************************************
*
*  Function:  dosdup()
*
*  Desc    :  calls dos to duplicate an existing file handle
*
*  Returns :  File Handle when successful
*             Or, -1 when an error occurs in which case ExtendedError and
*             ExtendedAction are set by calling DOS Get Extended Error
*
*/

int LIBENTRY dosdup(int OldHandle DOS2ERROR) {
  int  NewHandle;

  #ifdef __OS2__
    APIRET ExtendedError;
    ULONG  Handle;

    Handle = -1;
    ExtendedError = DosDupHandle(OldHandle,&Handle);
    if (ExtendedError != 0) {
      Os2Error->ExtendedError = ExtendedError;
      getextendederror(Os2Error);
      NewHandle = -1;
    } else {
      NewHandle = (int) Handle;
      if (NewHandle > MAXHANDLES) {
        /* too many file handles!  close it down now and fake a "too many */
        /* files open" error to avoid corrupting memory on the strcpy()   */
        dosclose(NewHandle);
        Os2Error->ExtendedError  = 4;    /* too many files open */
        Os2Error->ExtendedClass  = 14;   /* this is our own error class */
        Os2Error->ExtendedAction = 3;
        Os2Error->ExtendedLocus  = 2;
        NewHandle = -1;
        goto end;
      }

      strcpy(OpenFileNames[NewHandle],OpenFileNames[OldHandle]);
    }

  #elif defined(__WATCOMC__)
    {
      union REGS r;
      r.h.ah = 0x45;
      r.w.bx = OldHandle;
      int386(0x21, &r, &r);
      if (r.w.cflag) goto error;
      NewHandle = r.w.ax;
    }
  #else

    asm  mov  ah,45h        /* dos: open call */
    asm  mov  bx,OldHandle
    int21();

    asm  jc   error
    asm  mov  NewHandle,ax

    if (NewHandle > MAXHANDLES) {
      /* too many file handles!  close it down now and fake a "too many */
      /* files open" error to avoid corrupting memory on the strcpy()   */
      dosclose(NewHandle);
      ExtendedError  = 4;    /* too many files open */
      ExtendedClass  = 14;   /* this is our own error class */
      ExtendedAction = 3;
      ExtendedLocus  = 2;
      NewHandle = -1;
      goto end;
    }

    ExtendedError = 0;
    strcpy(OpenFileNames[NewHandle],OpenFileNames[OldHandle]);
    goto end;

  #endif
    goto end;
    error:
    getextendederror();
    NewHandle = -1;

end:
  if (dosdupcallback != NULL)
    dosdupcallback(getopenfilename(OldHandle),NewHandle,OldHandle,ExtendedError);

  return(NewHandle);
}


void LIBENTRY setdosdupcallback(void (LIBENTRY *cb)(char *FileName,int NewHandle,int OldHandle,int ExtendedError)) {
  dosdupcallback = cb;
}
