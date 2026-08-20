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
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/*****************************************************************************
 STREAMBUFSTART is the physical location within the file that corresponds to
                where the first byte in the stream buffer is located.  In
                other words, if the offset into the stream buffer is 0 on a
                read, then the first byte read would correspond to the byte
                found on disk at offset STREAMBUFSTART within the file.  On a
                write, the first byte written out of the buffer will go into
                the file at an offset specified by STREAMBUFSTART.

 STREAMBUFEND   is the physical location within the file that corresponds to
                where the last byte in the stream buffer is located.
******************************************************************************/

#define STREAMBUFSTART       (file->curpos - file->offset)
#define STREAMBUFEND         (file->curpos + (file->end - file->offset))
#define DIRTYBITSET(f)       ((f->status) & (DIRTYBIT))


long LIBENTRY dosfseek(DOSFILE *file, long distance, char method) {

  file->status &= (char) (~EOFBIT);   /* reset the EOFBIT flag   */

  switch (method) {
    case SEEK_CUR: /* turn the relative distance into an absolute position */
                   /* and then fall through to the SEEK_SET method for     */
                   /* handling absolute movement within the file           */

                   distance = file->curpos + distance;
                   /* fall through to SEEK_SET */

    case SEEK_SET: /* if we're moving to the same position where we are, */
                   /* then don't bother moving, just return the position */

                   if (distance == file->curpos)
                     return(file->curpos);

                   /* Check to see if the offset into the file (distance) is */
                   /* a position that is inside of the stream buffer.        */

                   if (distance >= STREAMBUFSTART && distance <= STREAMBUFEND) {
                     /* move the read/write pointer relative to the current  */
                     /* position by taking the offset (distance) minus the   */
                     /* current read/write position and adding it to the     */
                     /* offset into the stream buffer                        */
                     file->offset += (int) (distance - file->curpos);
                     return(file->curpos = distance);
                   }

                   /* before moving the read/write pointer, need to flush   */
                   /* any "writes" to disk to avoid creating a whole in the */
                   /* stream buffer on a write operation                    */

                   if (DIRTYBITSET(file))
                     dosflush(file);
                   else {
                     file->offset = 0; /* flush the buffer */
                     file->end    = 0;
                   }
                   return(file->curpos = doslseek(file->handle,distance,SEEK_SET));

    case SEEK_END: /* before moving the read/write pointer, need to flush   */
                   /* any "writes" to disk to avoid creating a whole in the */
                   /* stream buffer on a write operation                    */

                   if (DIRTYBITSET(file))
                     dosflush(file);
                   else {
                     file->offset = 0; /* flush the buffer */
                     file->end    = 0;
                   }
                   return(file->curpos = doslseek(file->handle,distance,SEEK_END));
  }

  return(0);
}
