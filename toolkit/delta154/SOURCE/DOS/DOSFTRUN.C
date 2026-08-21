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

/********************************************************************
*
*  Function:  dosftrunc()
*
*  Desc    :  Accepts a file handle of a file to truncate
*             If offset = -1, truncates at current location
*             If offset != -1, seeks to offset first, then truncates
*
*  Returns :  the same error status as returned by doswrite()
*/

int LIBENTRY dosftrunc(DOSFILE *file, long offset) {
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  dosflush(file);

  if (offset != -1)
    dosfseek(file,offset,SEEK_SET);

/*return(doswrite(file->handle,"",0 POS2ERROR));*/
  return(dostrunc(file->handle,-1 POS2ERROR));
}
