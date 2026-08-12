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


#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function:  writecheck()
*
*  Desc    :
*
*  Returns :
*/

unsigned LIBENTRY writecheck(int handle, void *buffer, unsigned count) {
  unsigned Retry;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  Retry = 0;

j1:
  if (doswrite(handle,buffer,count POS2ERROR) != count) {
    if (handle < 0 || handle > MAXHANDLES || OpenFileNames[handle][0] == 0) {
      #ifdef __OS2__
        Os2Error.ExtendedAction = 8; /* special case! */
      #else
        ExtendedAction = 8; /* special case! */
      #endif
    }
    if ((Retry = retrycount(Retry,OpenFileNames[handle],"Writing" POS2ERROR)) != 0xFFFF)
      goto j1;
    return(-1);
  }
  return(0);
}
