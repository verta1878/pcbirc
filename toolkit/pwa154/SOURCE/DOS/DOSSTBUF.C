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


#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <malloc.h>
#else
#include <alloc.h>
#endif

#include <string.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

int LIBENTRY dossetbuf(DOSFILE *file, unsigned BufSize) {
  if (file->handle == 0)
    return(-1);

  if (BufSize == file->bufsize)
    return(0);

  dosflush(file);
  free(file->buffer);

  file->buffer  = NULL;
  file->bufsize = BufSize;

  while (1) {
#if defined(DEBUG) && ! defined(__OS2__)
    /* use this line to insure that with MEMCHECK turned on we won't get */
    /* stuck with an "out of memory" warning message - it is SUPPOSED to */
    /* fail the malloc and keep on going but MEMCHECK stops              */
    if (coreleft() > file->bufsize+2048)
#endif
    if ((file->buffer = (char *) malloc(file->bufsize)) != NULL) {
      FileBuffers[file->handle] = file->buffer;
      break;
    }

    if (file->bufsize > 16)
      file->bufsize >>= 1;
    else {
      /* will this ever happen under OS/2??? */
      #ifndef __OS2__
        ExtendedError = 41;
        FileBuffers[file->handle] = NULL;
        dosclose(file->handle);
        memset(file,0,sizeof(DOSFILE));
        return(-1);
      #endif
    }
  }
  return(0);
}
