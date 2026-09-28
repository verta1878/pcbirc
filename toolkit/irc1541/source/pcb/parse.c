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
#include <stdio.h>
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: parse()
*
*  Desc    : This function is similar to the strtok() function included in the
*            C library.  However, it takes only a single argument and assumes
*            that the delimiter is a ",".  Also, if two commas are found
*            together (like ",,") it will return a zero length string.
*
*            NOTE: it does NOT return a NULL pointer at the end.
*
*  Calls   : strchr()
*/


char * LIBENTRY parse(char *Srce) {
  char static *p;
  char static *q;
  char static *end;

  if (Srce != NULL) {
    p = Srce;
    end = &Srce[strlen(Srce)];
  } else {
    if (q == end)
      return(end);
    p = q + 1;
  }

  if ((q = strchr(p,',')) == NULL) {
    q = end;
    return(p);
  } else {
    *q = 0;
    return(p);
  }
}
