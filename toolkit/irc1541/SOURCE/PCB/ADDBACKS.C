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
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: addbackslash()
*
*  Desc    : Takes *Str as input and adds a backslash "\" to it
*
*  NOTE    : if *Str is too long already then it won't add the backslash to
*            it - this will cause an ERROR in that a file path won't have the
*            backslash separate *but* such an error is preferable to allowing
*            C to overwrite it's data by having a string that's too long.
*/

void LIBENTRY addbackslash(char *Str, int MaxLen) {
  int L;

  L = strlen(Str);
  if (L >= MaxLen)
    return;
  if (L > 0 && Str[L-1] != '\\') {
    Str[L]   = '\\';
    Str[L+1] = 0;
  }
}
