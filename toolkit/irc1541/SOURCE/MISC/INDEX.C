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


#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*    function:  index()
*
*  parameters:  takes as input a pointer to a string and a character
*
*      return:  returns the index (offset) into the string where the
*               character was found, or -1 if not found.
*/

int LIBENTRY index(char *s,char c) {
  int i;

  for (i = 0; *s; i++,s++)
    if (*s == c)
      return(i);

  return(-1);
}
