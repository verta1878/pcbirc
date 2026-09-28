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

/********************************************************************
*
*  Function:  dosfgets()
*
*  Desc    :  reads the disk and returns a NULL-terminated string in Str
*
*             Str will be at most MaxChars long but may be shorter if a
*             carriage return is encountered in the file prior to MaxChars
*             being reached
*
*  Returns :  -1 on error
*             0  if the Str was CR/LF delimited
*             1  if either the MaxChars was hit or no CR/LF was found
*/

static unsigned int _NEAR_ LIBENTRY readdisk(DOSFILE *file) {
  unsigned BytesRead;
  char     *ptr_to_eof;

  if ((BytesRead = readcheck(file->handle,file->buffer,file->bufsize)) == (unsigned) -1)
    return((unsigned int) -1);

  if ((ptr_to_eof = strnchr(file->buffer,26,BytesRead)) != NULL) {
    *ptr_to_eof = 0;
    BytesRead = (unsigned) (ptr_to_eof - file->buffer);
    file->status |= EOFBIT;
  } else {
    #ifdef __OS2__
    if (BytesRead != file->bufsize) {
    #else
    if (ExtendedError == 40) { /* check for read past end */
    #endif
      file->status |= EOFBIT;
      if (BytesRead == 0)      /* if we were unable to read ANYTHING */
        return(0xffff);        /* then return an error NOW!          */
    }
  }

  file->end    = BytesRead;
  file->offset = 0;
  return(BytesRead);
}


int LIBENTRY dosfgets(char *Str, unsigned MaxChars, DOSFILE *file) {
  char *ptr_to_head;
  char *ptr_to_crlf;
  unsigned BytesRead;
  unsigned OldOffset;

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

  MaxChars--;         /* make room for the NULL terminator */

  if (file->last_op == OP_WRIT)
    dosflush(file);

  file->last_op = OP_READ;

  OldOffset = file->offset;

  while (1) {
    if (file->offset >= file->end) {
      if ((file->status & EOFBIT) != 0)
        return(-1);

      if ((BytesRead = readdisk(file)) == 0xffff)
        return(-1);

      OldOffset = 0;
    } else
      BytesRead = file->end - file->offset;

    ptr_to_head = &file->buffer[file->offset];
    if (*ptr_to_head == '\n') {  /* in case of a remaining LF, remove it now */
      ptr_to_head++;
      file->offset++;
    }

    if ((ptr_to_crlf = strnchr(ptr_to_head,'\r',BytesRead)) != NULL) {
      BytesRead = (unsigned) (ptr_to_crlf - ptr_to_head);
      if (BytesRead >= MaxChars) {
        file->offset += MaxChars;
        memcpy(Str,ptr_to_head,MaxChars);
        Str[MaxChars] = 0;
        file->curpos += file->offset - OldOffset;
        return(BytesRead > MaxChars ? 1 : 0);
      } else {
        file->offset += BytesRead;
        memcpy(Str,ptr_to_head,BytesRead);
        Str[BytesRead] = 0;
        file->offset++;                     /* get rid of CR by adding 1 */
        file->curpos += file->offset - OldOffset;

        if (file->offset == file->bufsize) {
          /* BOUNDARY CONDITION!  We've found a \r at the very end of the */
          /* buffer.  We can't check for a \n without going to read more  */
          /* data off the disk, so we'll do that now.  Then we check to   */
          /* see if the FIRST byte in the buffer is a \n and, if it is,   */
          /* we push the offset pointer past the \n character.            */

          if ((BytesRead = readdisk(file)) == 0xffff)
            return(-1);

          if (file->buffer[0] == '\n') {
            file->offset++;
            file->curpos++;
          }

        } else {
          if (*(ptr_to_crlf+1) == '\n') {
            file->offset++;                   /* get rid of LF by adding 1 */
            file->curpos++;
          }
        }
      }

      return(0);
    }

    if (BytesRead >= MaxChars) {
      memcpy(Str,ptr_to_head,MaxChars); Str[MaxChars] = 0;
      file->offset += MaxChars;
      file->curpos += file->offset - OldOffset;
      return(1);
    }

    memcpy(Str,ptr_to_head,BytesRead);
    file->end = 0;               /* force it to try to get more data next time */
    if (file->status & EOFBIT) { /* if we've read it all then get out now */
      Str[BytesRead] = 0;
      return(1);
    }
    MaxChars     -= BytesRead;
    Str          += BytesRead;
    file->curpos += BytesRead;
  }

  #ifndef __cplusplus
    return(-1);   /* it will never get here anyway */
  #endif
}
