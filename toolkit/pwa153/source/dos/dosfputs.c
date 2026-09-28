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

int LIBENTRY dosfputs(char *Str, DOSFILE *file) {
  unsigned Len;
  unsigned Remain;

  #ifdef DEBUG
    if (file->handle == 0 ||            /* handle isn't likely to be 0       */
        file->buffer == NULL ||         /* buffer should *never* be NULL     */
        file->buffer != FileBuffers[file->handle] || /* buffer must be equal */
        (file->bufsize & 1) != 0 ||     /* size is always multiple of 2      */
        file->offset > file->bufsize || /* offset can't be bigger than size  */
        file->end    > file->bufsize || /* end can't be bigger than size     */
        file->curpos < 0 ||             /* curpos can't be negative          */
        file->last_op > OP_WRIT ||      /* only OP_NONE to OP_WRIT are valid */
        Str == NULL)                    /* Str should *never* be NULL        */
      return(-1);
  #endif

  if ((Len = strlen(Str)) == 0)
    return(0);

  if (file->last_op == OP_READ)
    dosflush(file);

  file->last_op = OP_WRIT;

  Remain = file->bufsize - file->offset;
  if (Remain != 0) {
    process:
    if (Len <= Remain) {
      memcpy(&file->buffer[file->offset],Str,Len);
      file->offset += Len;
      file->curpos += Len;
      if (file->offset > file->end)
        file->end = file->offset;
      file->status |= DIRTYBIT;
      return(0);
    }

    memcpy(&file->buffer[file->offset],Str,Remain);
    file->curpos += Remain;
    Len -= Remain;
    Str += Remain;
    file->status |= DIRTYBIT;
  }

  if (file->status & DIRTYBIT)
    if (writecheck(file->handle,file->buffer,file->bufsize) == (unsigned) -1)
      return(-1);

  file->offset = 0;
  file->end    = 0;
  file->status &= (~DIRTYBIT);   /* reset the DIRTYBIT flag */
  Remain = file->bufsize;
  goto process;
}
