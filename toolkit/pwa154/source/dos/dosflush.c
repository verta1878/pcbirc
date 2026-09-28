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

int LIBENTRY dosflush(DOSFILE *file) {
  int Stat;

  #ifdef DEBUG
    if (file->handle == 0 ||            /* handle isn't likely to be 0       */
        file->buffer == NULL ||         /* buffer should *never* be NULL     */
        file->buffer != FileBuffers[file->handle] || /* buffer must be equal */
        (file->bufsize & 1) != 0 ||     /* size is always multiple of 2      */
        file->offset > file->bufsize || /* offset can't be bigger than size  */
        file->end    > file->bufsize || /* end can't be bigger than size     */
        file->curpos < 0 ||             /* curpos can't be negative          */
        file->last_op > OP_WRIT)        /* only OP_NONE to OP_WRIT are valid */
      return(-1);
  #endif

  Stat = 0;

  /* write buffer if set to WRITE mode and there's something there to write */
  if ((file->status & DIRTYBIT) && (file->status & OPEN_WRIT+OPEN_RDWR)) {
/*  Let's see what happens when we don't COMMIT the file on a flush
    if (writecheck(file->handle,file->buffer,file->end) == -1 || doscommit(file->handle) == -1)
*/
    if (writecheck(file->handle,file->buffer,file->end) == (unsigned) -1)
      Stat = -1;
  }

  /* in case the read/write pointer has moved, put it back to where our */
  /* internal "curpos" pointer says it should be                        */
  doslseek(file->handle,file->curpos,SEEK_SET);

  file->last_op = OP_NONE;
  file->offset  = 0;
  file->end     = 0;
  file->status &= (char) (~EOFBIT);           /* reset the EOFBIT flag   */
  file->status &= (char) (~DIRTYBIT);         /* reset the DIRTYBIT flag */
  return(Stat);
}
