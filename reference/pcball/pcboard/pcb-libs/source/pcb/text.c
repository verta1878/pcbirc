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


#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: getlog()
*
*  Desc    : Gets the next "token" in the ReadBuf[] buffer and converts it
*            to a boolean value.
*
*  Returns : Boolean value,  TRUE if the value was "-1" or FALSE otherwise
*/

bool LIBENTRY getlog(void) {
  return((bool) (atoi(parse(NULL)) == -1 ? TRUE : FALSE));
}


/********************************************************************
*
*  Function: getint()
*
*  Desc    : Gets the next "token" in the ReadBuf[] buffer and converts it
*            to an integer value.
*
*  Returns : Integer representation of ascii string
*/

int LIBENTRY getint(void) {
  return(atoi(parse(NULL)));
}


/********************************************************************
*
*  Function: getlong()
*
*  Desc    : Gets the next "token" in the ReadBuf[] buffer and converts it
*            to an long integer value.
*
*  Returns : Integer representation of ascii string
*/

long LIBENTRY getlong(void) {
  return(atol(parse(NULL)));
}


/********************************************************************
*
*  Function: getstr()
*
*  Desc    : Gets the next "token" in the ReadBuf[] buffer and copies it to
*            the Dest[] string.
*
*  Returns : None.  String pointed to on stack is filled with new value.
*/

void LIBENTRY getstr(char Dest[]) {
  strcpy(Dest,parse(NULL));
}
