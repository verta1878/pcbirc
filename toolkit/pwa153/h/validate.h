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

/* The prototype used to be hidden behind #ifndef __OS2__ with the note
   "this needs to be finished".  It is finished: MISC\VALIDATE.C, which
   holds the definition, compiles clean for OS/2 with wpp386 -bt=os2v2,
   and the function is target-independent - it walks a path string.
   Hiding the prototype only stopped its callers (PCB\DATA120.C and
   FIDOUTIL's FIDONET.CPP) from building.  -- pcbirc crew, 2026-09-23 */
int LIBENTRY validatepath(FILE *Out, char *Path, char *ResultPath, char Choice);

#endif
