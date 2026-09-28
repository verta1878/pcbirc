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

void LIBENTRY dosrewind(DOSFILE *file) {
  if ((file->status & OPEN_WRIT+OPEN_RDWR) && (file->status & DIRTYBIT))
    writecheck(file->handle,file->buffer,file->offset);

  doslseek(file->handle,0L,SEEK_SET);  /* move the DOS read pointer back    */
  file->curpos  = 0;                   /* set file read/write ptr to zero   */
  file->offset  = 0;                   /* set offset into file to zero      */
  file->end     = 0;                   /* set end of buffer to zero         */
  file->last_op = OP_NONE;             /* set last operation to nothing     */
  file->status &= (char) (~EOFBIT);    /* reset the EOFBIT flag   */
  file->status &= (char) (~DIRTYBIT);  /* reset the DIRTYBIT flag */

  #ifndef __OS2__
    ExtendedError = 0;
  #endif
}
