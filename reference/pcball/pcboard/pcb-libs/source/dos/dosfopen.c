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


#include <io.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <misc.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

char *FileBuffers[MAXHANDLES+1];

int LIBENTRY dosfopen(char *FileName, int OpenFlags, DOSFILE *file) {
  bool Create;
  bool Append;
  bool Write;
  int  Flags;
  #ifdef __OS2__
  os2errtype Os2Error;
  #endif

  Create = (bool) (OpenFlags & OPEN_CREATE);
  Append = (bool) (OpenFlags & OPEN_APPEND);
  Write  = (bool) (OpenFlags & (OPEN_WRIT|OPEN_RDWR));
  Flags  = OpenFlags & 0x73;  /* get share modes and open modes only -    */
                              /* i.e. get read of create and append flags */

  file->curpos = 0;

  if (OpenFlags & OPEN_ALREADY) {
    if ((file->handle = dosdup(file->handle POS2ERROR)) != -1)
      goto opened;
    return(-1);
  }

  if (Append) {         /* if opening for APPEND access */
    if ((file->handle = dosappendcheck(FileName,Flags)) != -1) {
      file->curpos = doslseek(file->handle,0,SEEK_CUR);
      goto opened;
    }
    return(-1);
  }

  if (Create) {         /* if creating a NEW file */
    if ((file->handle = doscreatecheck(FileName,Flags,OPEN_NORMAL)) != -1)
      goto opened;
    return(-1);
  }

  /* don't use error checking the first time */
  if ((file->handle = dosopen(FileName,Flags POS2ERROR)) != -1)
    goto opened;

  /* if no error checking is desired, then get out now */
  if (OpenFlags & OPEN_NOERRCHK)
    return(-1);

  if (Write && fileexist(FileName) == 255) {
    if ((file->handle = doscreatecheck(FileName,Flags,OPEN_NORMAL)) != -1)
      goto opened;
    return(-1);
  }

  /* last try .. see if we can open it with error checking turned on */
  if ((file->handle = dosopencheck(FileName,Flags)) != -1)
    goto opened;
  return(-1);

opened:
  file->bufsize = BEGBUFSIZE;
  while (1) {
    if ((file->buffer = (char *) malloc(file->bufsize)) != NULL)
      break;

    if (file->bufsize > 64)
      file->bufsize >>= 1;
    else {
      #ifndef __OS2__
        ExtendedError = 41;
      #endif
      dosclose(file->handle);
      return(-1);
    }
  }

  file->last_op = OP_NONE;      /* no reads or writes *yet*     */
  file->offset = 0;             /* set offset into file to zero */
  file->end    = 0;             /* set end of buffer to zero    */
  file->status = (char) (Flags & 0x03);  /* bottom two bits of Flags     */

  FileBuffers[file->handle] = file->buffer;
  return(0);
}
