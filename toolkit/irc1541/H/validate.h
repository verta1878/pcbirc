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


#ifndef H_VALIDATE
#define H_VALIDATE

#include <stdio.h>

#ifndef LIBENTRY
  #ifdef __WATCOMC__
    #define LIBENTRY
  #else
    #define LIBENTRY pascal
  #endif
#endif

#define VALIDATE_FILE  0
#define VALIDATE_DIR   1
#define CHECK_FILE     2
#define CHECK_DIR      3

#ifndef __OS2__  /* this needs to be finished */
int LIBENTRY validatepath(FILE *Out, char *Path, char *ResultPath, char Choice);
#endif

#endif
