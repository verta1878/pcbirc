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

static void near pascal zmSendNulls(void) {
  static char Nulls[8] = {0,0,0,0,0,0,0,0};
  int X;

  for (X = ZNulls; X > 8; ) {
    zmSpoolStr(Nulls,8);
    X -= 8;
  }

  if (X > 0)
    zmSpoolStr(Nulls,X);
}

/***************************************************************************
 ***************************************************************************/
void pascal zmSendBinaryHeader(int Length, int Type, char *Header) {
  char static    buf[2] = {ZPAD,ZDLE};
  int            n;
  unsigned short crc;

  if (Type == ZDATA)
    zmSendNulls();

  zmSpoolStr(buf,2);

  switch (tx32crc = tx32fcs) {
      case 2 :
              zmSendBinaryHeader32(Length, Header, Type,
                          VariableLengthHeaders ? ZVBINR32 : ZBINR32);
          break;
      case 1 :
              zmSendBinaryHeader32(Length, Header, Type,
                          VariableLengthHeaders ? ZVBIN32 : ZBIN32);
          break;
      default :

              if (VariableLengthHeaders) {
                  zmXPutByte(ZVBIN);
                  zmZPutByte(Length);
              } else {
                  zmXPutByte(ZBIN);
              }
              zmZPutByte(Type);
              crc = updcrc16(Type, 0);      //lint !e572

              for (n = Length; --n >= 0; ++Header) {
                  zmZPutByte(*Header);
                  crc = updcrc16((0xFF & *Header), crc);
              }
              crc = updcrc16(0, updcrc16(0, crc));
              zmZPutByte(crc >> 8);
              zmZPutByte(crc);
          break;
  }
  if (Type != ZDATA) {
      /*** flush (not necessary) *****************************************/
  }
}
