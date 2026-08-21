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
#include <stdlib.h>
#include <pcboard.h>
#include <pcboard.ext>
#ifdef DEBUG
#include <memcheck.h>
#endif


void LIBENTRY initport(void) {
  if (memcmp(PcbData.ModemPort,"COM",3) == 0)
    Asy.ComPortNumber = (char) atoi(&PcbData.ModemPort[3]);
  else if (memcmp(PcbData.ModemPort,"PORT",4) == 0)
    Asy.ComPortNumber = (char) atoi(&PcbData.ModemPort[4]);
  else
    Asy.ComPortNumber = 0;
}


void LIBENTRY openport(void) {
  if (Asy.Online == REMOTE) {
    openmodem(HIDE);
    if (Asy.Online == REMOTE) {
      CDokay = 0x80;
      online();
    }
  }
}
