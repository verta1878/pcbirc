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


#ifdef OLDFUNCTIONS
#include <io.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function:  fprintfcheck()
*
*  Desc    :
*
*  Returns :
*/

int LIBENTRY fprintfcheck(FILE *handle) {
  if (ferror(handle) != 0) {
    showerror(OpenFileNames[fileno(handle)],"Writing",FALSE);
    return(-1);
  }
  return(0);
}
#endif