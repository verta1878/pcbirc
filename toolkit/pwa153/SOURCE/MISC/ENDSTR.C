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
#include <misc.h>
#ifdef DEBUG
  #include <memcheck.h>
#endif

/* this string returns a pointer to the last "MaxLen" characters in a string */
/*                                                                           */
/* Example:  endofstring("ABCDEFG",3) returns EFG                            */
/*           endofstring("ABC",10)    returns ABC                            */

char * LIBENTRY endofstring(char *Str, int MaxLen) {
  int Len;

  Len = strlen(Str);
  if (Len <= MaxLen)
    return(Str);

  return(Str + (Len-MaxLen));
}
