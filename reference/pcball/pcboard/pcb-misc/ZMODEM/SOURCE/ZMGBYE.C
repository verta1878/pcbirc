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
 *** Say goodbye to the receiver / "over and out" **************************/
void    _Cdecl  zmGoodBye(void) {
  while (TRUE) {
    zmStoreHeader(0L);
    zmSendHexHeader(4, ZFIN, txHeader);
    switch (zmGetHeader(rxHeader)) {
      case ZFIN    : zmXPutByte('O');
                     zmXPutByte('O');
                     return;
      case ZCAN    : return;
      case TIMEOUT : return;
    }
  }
}
