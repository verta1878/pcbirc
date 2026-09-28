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
#include "crc.h"
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

/***************************************************************************
 ***************************************************************************/
void pascal zmSendData32(char *Buf, int Length, int FrameEnd) {
  int           c;
  unsigned long crc;

  crc = 0xFFFFFFFFL;
  for (; --Length >= 0; ++Buf) {
    c = *Buf & 0xFF;

    if (c & 0x60) {
      LastSent = c;
      zmXPutByte(c);
    } else
      zmZPutByte(c);

    crc = updcrc32(c, crc);
  }

  zmXPutByte(ZDLE);
  zmXPutByte(FrameEnd);

  crc = updcrc32(FrameEnd, crc);
  crc = ~crc;
  for (c = 4; --c >= 0; ) {
    zmZPutByte((int)crc);
    crc >>= 8;
  }

  if (kbdinkey() == 0x1B)
    zmAbort = TRUE;
}
