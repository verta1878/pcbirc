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


#include <ctype.h>
#include "misc.h"


/********************************************************************
*
*  Function:  alldigits()
*
*  Desc    :  checks the string to see if all characters in it are digits 0-9
*
*  Returns :  TRUE if they're all digits, FALSE otherwise
*/

bool LIBENTRY alldigits(char *p) {
  for (; *p != 0; p++)
    if (! isdigit(*p))
      return(FALSE);
  return(TRUE);
}
