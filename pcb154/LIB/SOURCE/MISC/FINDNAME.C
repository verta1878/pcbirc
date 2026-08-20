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


#include <stdlib.h>
#include <string.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

char * LIBENTRY findstartofname(char *Path) {
  char *p;

  /* find the first character of the filename, skipping over any path */
  /* information                                                      */

  if ((p = strrchr(Path,'\\')) != NULL || (p = strrchr(Path,':')) != NULL)
    return(p+1);  /* point to first character of filename */

  return(Path);   /* or point to first character of complete path */
}
