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
void pascal zmSendData32RLE(char *Buf, int Length, int FrameEnd) {
  int           c;
  int           l;
  int           n;
  unsigned long crc;

  crc = 0xFFFFFFFFL;
  l = *Buf++ & 0xFF;
  if (Length == 1) {
    zmZPutByte(l);
    crc = updcrc32(l, crc);
    if (l == ZRESC) {
      zmZPutByte(1);
      crc = updcrc32(1, crc);
    }
  } else {
    for (n = 0; --Length >= 0; ++Buf) {
      if ((c = *Buf & 0xFF) == l && n < 126 && Length > 0) {
        ++n;
        continue;
      }
      switch (n) {
        case 0 : zmZPutByte(l);
                 crc = updcrc32(l, crc);
                 if (l == ZRESC) {
                         zmZPutByte(0x40);
                         crc = updcrc32(0x40, crc);
                 }
                 l = c;
                 break;
        case 1 : if (l != ZRESC) {
                   zmZPutByte(l);
                   zmZPutByte(l);
                   crc = updcrc32(l, crc);
                   crc = updcrc32(l, crc);
                   n = 0;
                   l = c;
                   break;
                 }
                 //lint !e616  falls thru --- is it supposed to???
        default: zmZPutByte(ZRESC);
                 crc = updcrc32(ZRESC, crc);
                 if (l == 0x20 && n < 34) {
                   n += 0x1E;
                   zmZPutByte(n);
                   crc = updcrc32(n, crc);
                 } else {
                   n += 0x41;
                   zmZPutByte(n);
                   crc = updcrc32(n, crc);
                   zmZPutByte(l);
                   crc = updcrc32(l, crc);
                 }
                 n = 0;
                 l = c;
                 break;
      }
    }
  }

  zmXPutByte(ZDLE);
  zmXPutByte(FrameEnd);
  crc = updcrc32(FrameEnd, crc);

  crc = ~crc;
  for (Length = 4; --Length >= 0; ) {
    zmZPutByte((int)crc);
    crc >>= 8;
  }

  if (kbdinkey() == 0x1B)
    zmAbort = TRUE;
}
