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

#if __BORLANDC__ >= 0x0410 || __OS2__
#pragma option -Oi
#endif

bool LIBENTRY fileisopen(char *FileName) {
  int X;

  for (X = 1; X <= MAXHANDLES; X++)
    if (strcmp(OpenFileNames[X],FileName) == 0)
      return(TRUE);

  return(FALSE);
}


char * LIBENTRY getopenfilename(int Handle) {
  if (Handle <= 0 || Handle > MAXHANDLES)
    return("");

  if (OpenFileNames[Handle][0] == 0)
    return("");

  return(OpenFileNames[Handle]);
}


#if __BORLANDC__ >= 0x0410 || __OS2__
#pragma option -O-i
#endif
