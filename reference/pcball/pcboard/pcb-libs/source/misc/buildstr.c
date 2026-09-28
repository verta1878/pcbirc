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
#include <stdarg.h>
#include <stdlib.h>
#include <misc.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function:  buildstr()
*
*  Desc    :  Accepts a variable number of arguments with the 2nd thru the
*             last as pointers to strings which should be concatenated onto
*             the pointer to the first string.
*/

void buildstr(char *Dest, ...) {
  int     Len;
  char   *q;
  va_list p;

  *Dest = 0;
  va_start(p,Dest);

  while ((q = va_arg(p,char *)) != NULL) {
    Len = strlen(q);
    memcpy(Dest,q,Len+1);
    Dest += Len;
  }
  va_end(p);
}
