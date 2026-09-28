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

int LIBENTRY validatesemantics(char *Path) {
  int  Pos;
  int  Count;
  bool Dot;
  bool Colon;

  for (Pos = 1, Count = 0, Dot = Colon = FALSE; *Path != 0; Pos++, Path++) {
    switch (*Path) {
      case ':' : if (Pos != 2)    /* colon should always the 2nd character  */
                   return(-1);    /* or not at all, otherwise it's an error */
                 Colon = TRUE;
                 Count = 0;
                 Dot = FALSE;
                 break;
      case '\\': if (Pos != 1) {               /* \ in first pos is okay */
                   if (Count == 0) {
                     if (Colon && Pos == 3) {  /* \ after colon is okay  */
                     } else
                       return(-1);             /* must preceed \ with chars */
                   }
                 }
                 Count = 0;
                 Dot = FALSE;
                 break;
      case '.' : /* a path such as "..\test" is okay */
                 if (*(Path+1) == '.' && *(Path+2) == '\\') {
                   /* skip over the next two characters, the . and \ */
                   Path += 2;
                   Pos  += 2;
                   Dot   = FALSE;
                   Count = 0;
                   break;
                 }
                 if (Count == 0)  /* must have at least 1 char in name */
                   return(-1);
                 if (Dot)         /* can't have two dots in a filename */
                   return(-1);
                 Dot = TRUE;
                 break;
      case '>' :
      case '<' :
      case '+' : return(-1);   /* these chars are never valid in a path */
      default  : Count++;
                 if (Dot) {
                   if (Count > 3)  /* extensions cannot be more than 3 chars */
                     return(-1);
                 } else {
                   if (Count > 8)  /* names cannot be more than 3 chars */
                     return(-1);
                 }
                 break;
    }
  }
  return(FALSE);
}
