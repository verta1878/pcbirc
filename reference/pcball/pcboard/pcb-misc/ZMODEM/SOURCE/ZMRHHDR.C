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
int pascal zmReceiveHexHeader(char *Header) {
  int             c;
  int             n;
  int             crc;

  if ((c = zmGetHex()) < 0)
      return c;
  rxType = c;
  crc = updcrc16(c, 0);    //lint !e572
  for (n = rxHeaderLength; --n >= 0; ++Header) {
    if ((c = zmGetHex()) < 0)
      return c;
    crc = updcrc16(c, crc);
    *Header = c;
  }

  if ((c = zmGetHex()) < 0)
    return c;

  crc = updcrc16(c, crc);
  if ((c = zmGetHex()) < 0)
    return c;

  crc = updcrc16(c, crc);
  if (crc & 0xFFFF) {
    zmError(1,"Bad CRC");
    return ERROR;
  }

  switch (c = zmGetByte(10)) {    // get a byte with 10 second timeout
    case 0x9D : Not8Bit = c;
                //lint !e616  fall thru
    case 0x0D : switch (c = zmGetByte(10)) {  // get a byte with 10 second timeout
                  case 0x0A : Not8Bit |= c;
                }
  }

  ZModem = 1;
  if (c < 0)
    return c;

  return rxType;
}
