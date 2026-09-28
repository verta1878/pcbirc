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

int LIBENTRY dosfread(void *Buffer, unsigned NumChars, DOSFILE *file) {
  unsigned CharsCopied;
  unsigned BytesRead;

  #ifdef DEBUG
    if (file->handle == 0 ||            /* handle isn't likely to be 0       */
        file->buffer == NULL ||         /* buffer should *never* be NULL     */
        file->buffer != FileBuffers[file->handle] || /* buffer must be equal */
        (file->bufsize & 1) != 0 ||     /* size is always multiple of 2      */
        file->offset > file->bufsize || /* offset can't be bigger than size  */
        file->end    > file->bufsize || /* end can't be bigger than size     */
        file->curpos < 0 ||             /* curpos can't be negative          */
        file->last_op > OP_WRIT ||      /* only OP_NONE to OP_WRIT are valid */
        Buffer == NULL)                 /* Buffer should *never* be NULL     */
      return(-1);
  #endif

  if (file->last_op == OP_WRIT)
    dosflush(file);

  file->last_op = OP_READ;

  CharsCopied = 0;

  while (1) {
    if (file->offset >= file->end) {
      if ((file->status & EOFBIT) != 0)
        return(CharsCopied);
      if ((BytesRead = readcheck(file->handle,file->buffer,file->bufsize)) == 0xffff)
        return(-1);
#ifdef __OS2__
      if (BytesRead != file->bufsize) {
#else
      if (ExtendedError == 40) { /* check for read past end */
#endif
        file->status |= EOFBIT;
        if (BytesRead == 0) {
          if (CharsCopied != 0)
            return(CharsCopied);
          return(-1);
        }
      }
      file->end    = BytesRead;
      file->offset = 0;
    } else
      BytesRead = file->end - file->offset;

    if (BytesRead >= NumChars) {
      memcpy(Buffer,&file->buffer[file->offset],NumChars);
      file->offset += NumChars;
      file->curpos += NumChars;
      CharsCopied  += NumChars;
      return(CharsCopied);
    } else {
      memcpy(Buffer,&file->buffer[file->offset],BytesRead);
      file->curpos     += BytesRead;
      NumChars         -= BytesRead;
      CharsCopied      += BytesRead;
      Buffer            = (char *) Buffer + BytesRead;
      file->end         = 0;             /* force it to read more data */
      file->offset      = 0;
    }
  }

  #ifndef __cplusplus
    return(-1);  /* we'll never really get here anyway! */
  #endif
}
