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
 *** ACK a ZFIN packet then get outta here *********************************/
void  _Cdecl zmGoodByeACK(void) {
  int n;

  zmStoreHeader(0L);
  for (n = 0; n < 3; n++) {

    /*** I think the following is equivalent to 'purgeline()' ********/
//  clearinbuf();

    zmSendHexHeader(4, ZFIN, txHeader);
    switch (zmGetByte(5)) {   // get a byte with 5 second timeout
      case 'O'    : zmGetByte(1); /* discard the 2nd 'O' */ //lint !e534
                    return;
      case RCDO   : return;
      case TIMEOUT: break;
      default     : break;
    }
  }
}
