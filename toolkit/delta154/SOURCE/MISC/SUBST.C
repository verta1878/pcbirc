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


#include <stdio.h>
#include <string.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function:  substitute()
*
*  Desc    :  Searchs *Str to find substring *Search within it.  If found, it
*             is replaced by the *Replace string.
*
*  Returns :  FALSE = no replacement made
*             TRUE  = replaced
*/

bool LIBENTRY substitute(char *Str, char *Search, char *Replace, int MaxLen) {
  char *p;
  int  SrchLen;
  int  RepLen;

  if ((p = strstr(Str,Search)) == NULL)
    return(FALSE);

  SrchLen = strlen(Search);
  RepLen  = strlen(Replace);

  /* don't let it try to replace the original with an identical string */
  if (SrchLen == RepLen && strcmp(Search,Replace) == 0)
    return(FALSE);

  /* don't let the substitution overrun the size of the original buffer used */
  if (strlen(Str) - SrchLen + RepLen > MaxLen - 1)
    return(FALSE);

  if (SrchLen >= RepLen) {
    memcpy(p,Replace,RepLen);
    strcpy(p+RepLen,p+SrchLen);
  } else {
    memmove(p+RepLen,p+SrchLen,strlen(p+SrchLen)+1);
    memcpy(p,Replace,RepLen);
  }

  MaxLen -= (int) (p - Search) + RepLen;
  substitute(p+RepLen,Search,Replace,MaxLen);
  return(TRUE);
}

