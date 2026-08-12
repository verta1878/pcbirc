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
#include <stdio.h>
#include <mem.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif


/***************************************************************************
 ***************************************************************************/
int pascal zmGetHeader(char *Header) {
  short  CancelCount;
  int    c,
         n;

  n = EffectiveBaud;
  rxFrameID = rxType = 0;

StartOver:
  CancelCount = 5;

Again:
  switch (c = zmGetByte(rxTimeout)) {
    case 0x11     : goto Again;
    case 0x91     : goto Again;
    case RCDO     : goto fifi;
    case TIMEOUT  : goto fifi;
    case CAN      :
GotCAN:
                    if (--CancelCount <= 0 || zmAbort) {
                      c = ZCAN;
                      goto fifi;
                    }
                    switch (c = zmGetByte(10)) {  // get a byte with 10 second timeout
                      case TIMEOUT : goto Again;
                      case ZCRCW   : switch (zmGetByte(10)) {   // get a byte with 10 second timeout
                                       case TIMEOUT : c = ERROR;
                                                      goto fifi;
                                       case RCDO    : goto fifi;
                                     }
                                     goto OnceAgain;
                      case RCDO    : goto fifi;
                      default      : break;
                      case CAN     : if (--CancelCount <= 0 || zmAbort) {
                                       c = ZCAN;
                                       goto fifi;
                                     }
                                     goto Again;
                    }
                    //lint !e616 fall thru   (is it really supposed to???)

    default       :
OnceAgain:
                    if (--n == 0) {
                      c = GCOUNT;
                      goto fifi;
                    }
                    goto StartOver;
    case ZPAD|0x80: Not8Bit = c;
                    break;
    case ZPAD     : break;
  }
  CancelCount = 5;

Splat:
  switch (c = noXReadByte()) {
    case ZPAD    : goto Splat;
    case RCDO    : goto fifi;
    case TIMEOUT : goto fifi;
    default      : goto OnceAgain;
    case ZDLE    : break;
  }

  rxHeaderLength = 4;
  rxFrameID = c = noXReadByte();

  switch (c) {
    case ZVBIN32 : if ((rxHeaderLength = c = zmReadByte()) < 0)
                     goto fifi;
                   if (c > ZMAXHLEN)
                     goto OnceAgain;
                   rx32crc = 1;
                   c = zmReceiveBinaryHeader32(Header);
                   break;
    case ZBIN32  : if (VariableLengthHeaders)
                     goto OnceAgain;
                   rx32crc = 1;
                   c = zmReceiveBinaryHeader32(Header);
                   break;
    case ZVBINR32: if ((rxHeaderLength = c = zmReadByte()) < 0)
                     goto fifi;
                   if (c > ZMAXHLEN)
                       goto OnceAgain;
                   rx32crc = 2;
                   c = zmReceiveBinaryHeader32(Header);
                   break;
    case ZBINR32 : if (VariableLengthHeaders)
                     goto OnceAgain;
                   rx32crc = 2;
                   c = zmReceiveBinaryHeader32(Header);
                   break;
    case RCDO    : goto fifi;
    case TIMEOUT : goto fifi;
    case ZVBIN   : if ((rxHeaderLength = c = zmReadByte()) < 0)
                     goto fifi;
                   if (c > ZMAXHLEN)
                       goto OnceAgain;
                   rx32crc = 0;
                   c = zmReceiveBinaryHeader(Header);
                   break;
    case ZBIN    : if (VariableLengthHeaders)
                     goto OnceAgain;
                   rx32crc = 0;
                   c = zmReceiveBinaryHeader(Header);
                   break;
    case ZVHEX   : if ((rxHeaderLength = c = zmGetHex()) < 0)
                     goto fifi;
                   if (c > ZMAXHLEN)
                       goto OnceAgain;
                   rx32crc = 0;
                   c = zmReceiveHexHeader(Header);
                   break;
    case ZHEX    : if (VariableLengthHeaders)
                     goto OnceAgain;
                   rx32crc = 0;
                   c = zmReceiveHexHeader(Header);
                   break;
    case CAN     : goto GotCAN;
    default      : goto OnceAgain;
  }

  memset(&Header[rxHeaderLength],0,ZMAXHLEN-rxHeaderLength-1);
//memset(&Header[n],0,ZMAXHLEN-rxHeaderLength);
//the above line replaces these two lines
//for (n = rxHeaderLength; n++ < ZMAXHLEN; )
//    Header[n] = 0x00;

  rxPosition = Header[ZP3] & 0xFF;
  rxPosition = (rxPosition << 8) + (Header[ZP2] & 0xFF);
  rxPosition = (rxPosition << 8) + (Header[ZP1] & 0xFF);
  rxPosition = (rxPosition << 8) + (Header[ZP0] & 0xFF);

fifi:
  zmDisplay(4, 5, 0x1F, "Frame    : %-15s", FrameTypes[c + ZFTOFFSET]);
  switch (c) {
    case GOTCAN : c = ZCAN;
                  //lint !e616 fall thru
    case ZNAK   :
    case ZCAN   :
    case ERROR  :
    case TIMEOUT:
    case RCDO   :
    case GCOUNT : zmError(1,"Got %s", FrameTypes[c + ZFTOFFSET]);
  }

  if (c >= 0 && c <= FRAMETYPES && (rxFrameID & 0x20))
    VariableLengthHeaders = 1;

  return(c);
}
