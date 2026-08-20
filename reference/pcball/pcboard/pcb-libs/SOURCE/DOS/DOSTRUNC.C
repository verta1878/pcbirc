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


#ifdef __OS2__
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

#include <io.h>
#include <stdio.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function:  dostrunc()
*
*  Desc    :  Accepts a file handle of a file to truncate
*             If offset = -1, truncates at current location
*             If offset != -1, seeks to offset first, then truncates
*
*  Returns :  the same error status as returned by doswrite()
*/

#pragma argsused
int LIBENTRY dostrunc(int handle, long offset DOS2ERROR) {
  #ifdef __OS2__

    if (offset == -1)
      offset = doslseek(handle,0,SEEK_CUR);
    return(DosSetFileSize(handle,offset) != 0 ? -1 : 0);

  #else

    if (offset != -1)
      doslseek(handle,offset,SEEK_SET);

    if (doswrite(handle,"",0 POS2ERROR2) == -1)
      return(-1);

    return(doscommit(handle));
  #endif
}
