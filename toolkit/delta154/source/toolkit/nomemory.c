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


#include <pcbtools.h>

void LIBENTRY initbmalloc(void) {
}

#pragma argsused
void * LIBENTRY bmalloc(int Size) {
  return(NULL);
}

#pragma argsused
void LIBENTRY bfree(void *p) {
}

void LIBENTRY bfreeall(void) {
}

#pragma argsused
void _FAR_ * LIBENTRY fbmalloc(long Size) {
  return(NULL);
}

#pragma argsused
void LIBENTRY fbfree(void _FAR_ *p) {
}

void LIBENTRY fbfreeall(void) {
}
