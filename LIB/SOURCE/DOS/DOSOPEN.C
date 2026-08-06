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

char OpenFileNames[MAXHANDLES+1][MAXNAMELEN];

static void (LIBENTRY *dosopencallback)(char *FileName,int OpenFlags,int Handle,int ExtendedError);


/********************************************************************
*
*  Function:  dosopen()
*
*  Desc    :  calls dos to open an existing file - gets extended error codes
*             when an error occurs.
*
*  Returns :  File Handle when successful
*             Or, -1 when an error occurs in which case ExtendedError and
*             ExtendedAction are set by calling DOS Get Extended Error
*
*/

int LIBENTRY dosopen(char *FileName, int OpenFlags DOS2ERROR) {
  int Handle;

  #ifdef __OS2__
    ULONG   ActionTaken;
    ULONG   NewHandle;
    APIRET  ExtendedError;

    // check to see if any permission flags are set
    if ((OpenFlags & 0x70) == 0) {
      // if no permission flags are set, then OR in the DENY_RDWR flag
      // since that is equivalent to the default.  OS/2 apparently doesn't
      // use DOS's default so it needs an explicity DENY_RDWR status.
      OpenFlags |= OPEN_DENYRDWR;
    }

    ExtendedError = DosOpen(FileName,
                            &NewHandle,
                            &ActionTaken,
                            0,0,
                            OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
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

  #elif defined(__WATCOMC__)
    {
      union REGS r;
      r.h.ah = 0x3D;
      r.h.al = (unsigned char)(OpenFlags | 0x80); /* PRIVATE */
      r.w.dx = (unsigned short)(unsigned long)FileName;
      int386(0x21, &r, &r);
      if (r.w.cflag) { getextendederror(); Handle = -1; goto end; }
      Handle = r.w.ax;
    }
  #else
    #ifdef SDATA
      asm  mov  dx,FileName
    #else
      asm  push ds
      asm  lds  dx,FileName
    #endif

      asm  mov  ah,3Dh        /* dos: open call */
      asm  mov  al,byte ptr OpenFlags
      asm  or   al,10000000b  /* set inheritance flag to 1 (=PRIVATE) */
      int21();

    #ifdef LDATA
      asm  pop  ds
    #endif

      asm  jc   error
      asm  mov  Handle,ax

      if (Handle > MAXHANDLES) {
        /* too many file handles!  close it down now and fake a "too many */
        /* files open" error to avoid corrupting memory on the strcpy()   */
        dosclose(Handle);
        ExtendedError  = 4;    /* too many files open */
        ExtendedClass  = 14;   /* this is our own error class */
        ExtendedAction = 3;
        ExtendedLocus  = 2;
        Handle = -1;
        goto end;
      }

      ExtendedError = 0;
      strcpy(OpenFileNames[Handle],FileName);
      goto end;

    error:
      getextendederror();
      Handle = -1;
  #endif

end:
  if (dosopencallback != NULL)
    dosopencallback(FileName,OpenFlags,Handle,ExtendedError);

  return(Handle);
}


void LIBENTRY setdosopencallback(void (LIBENTRY *cb)(char *FileName,int OpenFlags,int Handle,int ExtendedError)) {
  dosopencallback = cb;
}
