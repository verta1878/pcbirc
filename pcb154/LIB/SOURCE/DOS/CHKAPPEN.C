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


#include <string.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function:  dosappendcheck()
*
*  Desc    :
*
*  Returns :
*/

int LIBENTRY dosappendcheck(char *path, int access) {
  int      Handle;
  unsigned Retry;
  #ifdef __OS2__
  os2errtype Os2Error;
  #endif

  Retry = 0;

j1:
  if ((Handle = dosappend(path,access POS2ERROR)) == -1) {
    if ((Retry = retrycount(Retry,path,"Opening" POS2ERROR)) != 0xFFFF)
      goto j1;
  }
  return(Handle);
}
