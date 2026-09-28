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

static char hdr[3] = {ZPAD,ZPAD,ZDLE};
static char tlr[3] = {0x0D,0x8A,0x11};


/***************************************************************************
 ***************************************************************************/
void pascal zmSendHexHeader(int Length, int Type, char *Header) {
  unsigned short  crc;
  int      i;

  zmSpoolStr(hdr,3);

  if (VariableLengthHeaders) {
    zmPutByte(ZVHEX);
    zmPutHex(Length);
  } else {
    zmPutByte(ZHEX);
  }
  zmPutHex(Type);

  crc = updcrc16(Type, 0);   //lint !e572
  for (i = Length; --i >= 0; ++Header) {
    zmPutHex(*Header);
    crc = updcrc16((*Header & 0xFF), crc);
  }
  crc = updcrc16(0, updcrc16(0, crc));
  zmPutHex(crc >> 8);
  zmPutHex(crc);

  if (Type != ZFIN && Type != ZACK)
    zmSpoolStr(tlr,3);
  else
    zmSpoolStr(tlr,2);
}
