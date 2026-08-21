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

#include <io.h>
#include <stdio.h>
#include <string.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


int LIBENTRY dosfclose(DOSFILE *file) {
  int ret;

  /* first, check to see if the handle shows it MIGHT be an open file */
  if (file->handle > 0 && file->handle <= MAXHANDLES) {

    /* now double-check the validity by making sure that the global array */
    /* of pointers has the same buffer pointer as that which is stored in */
    /* the DOSFILE structure - if NOT then do *NOT* perform a close       */

    if (FileBuffers[file->handle] != NULL && FileBuffers[file->handle] == file->buffer) {

      /* if the file status dirty bit is set, then write out the changes */

      ret = 0;
      if ((file->status & DIRTYBIT) && (file->status & OPEN_WRIT+OPEN_RDWR))
        ret = writecheck(file->handle,file->buffer,file->end);

      /* close the file, free the buffer, and make sure everything is NULL'd out */

      dosclose(file->handle);
      free(file->buffer);
      FileBuffers[file->handle] = NULL;
      memset(file,0,sizeof(DOSFILE));
      return(ret);
    }
  }

  return(-1);
}


void LIBENTRY closedosfopenfiles(void) {
  int X;

  for (X = MAXHANDLES; X > 0; X--) {
    if (FileBuffers[X] != NULL) {
      free(FileBuffers[X]);
      FileBuffers[X] = NULL;
    }
  }
}
