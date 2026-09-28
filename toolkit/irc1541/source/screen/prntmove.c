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
#include "screen.h"

/********************************************************************
 *
 *  Function:  fastprintmove()
 *
 *  Print a string of characters on the screen at a given location with a
 *  specified attribute for the entire string and move the cursor to the end
 *  of the line afterwards
 */

void LIBENTRY fastprintmove(int X, int Y, char Str[80], char Attr) {
  fastprint(X,Y,Str,Attr);
  gotoxy(X+strlen(Str),Y);
}
