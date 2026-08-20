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

#include <pcbtools.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

/***************************************************************************
 *** Read a character from the port with timeout.  Eat all XON/XOFF ********
 *** characters as well as parity. *****************************************/
int pascal noXReadByte(void) {
  int c;

  while (TRUE) {
    if ((c = zmGetByte(rxTimeout)) < 0)
      return c;
    switch (c &= 0x7F) {
      case XON  : continue;
      case XOFF : continue;
      default   : if (ControlEncode && !(c & 0x60))
                    continue;
                  return(c);
      case '\r' : return(c);
      case '\n' : return(c);
      case ZDLE : return(c);
    }
  }

/*** unreachable code ******************************************************
    return c;
 ***************************************************************************/
}
