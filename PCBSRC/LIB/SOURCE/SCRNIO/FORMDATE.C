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
#include <scrnio.h>
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
 *
 *  Function:  formatdate()
 *
 *  This function formats a date string into a standard format.  For example,
 *
 *   1/1/87   becomes 01/01/87
 *   1-01-87  becomes 01/01/87
 *   010187   becomes 01/01/87
 *   1/1/xx   becomes 01/01/xx
 */

#define isdatechar(c)  (c == 'X' || (c >= '0' && c <= '9'))

void LIBENTRY formatdate(char *Str) {
  char New[9];
  int  X,Y;

  for (X = 0; X < 8; X++)
    New[X] = '0';
  New[8] = 0;

  for (X = 7; ! isdatechar(Str[X]); X--);

  for (Y = 7; X >= 0; X--) {
    if (isdatechar(Str[X])) {
      New[Y] = Str[X];
      Y--;
    } else {
      if (Y >= 5)
        Y = 4;
      else
        if (Y == 2 || Y == 3)
          Y = 2;
    }

    if (Y == 2 || Y == 5) {
      New[Y] = '-';
      Y--;
    }
    if (Y < 0)
      break;
  }
  memcpy(Str,New,8);
}
