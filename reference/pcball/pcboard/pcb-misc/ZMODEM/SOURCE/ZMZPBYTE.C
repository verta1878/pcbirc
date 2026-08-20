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
 ***************************************************************************/
void pascal zmZPutByte(int Byte) {

  if (Byte & 0x60) {
    LastSent = Byte;
    zmXPutByte(Byte);
  } else {
    switch (Byte & 0xFF) {
      case ZDLE : zmXPutByte(ZDLE);
                  Byte ^= 0x40;
                  LastSent = Byte;
                  zmXPutByte(Byte);
                  break;
      case 0x0D :
      case 0x8D : if (ControlEncode && (LastSent & 0x7F) != '@') {
                    LastSent = Byte;
                    zmXPutByte(Byte);
                    break;
                  }
                  //lint !e616  fall thru, but are we supposed to?
      case 0x10 :
      case 0x11 :
      case 0x13 :
      case 0x90 :
      case 0x91 :
      case 0x93 : zmXPutByte(ZDLE);
                  Byte ^= 0x40;
                  LastSent = Byte;
                  zmXPutByte(Byte);
                  break;

      default :   if (ControlEncode && !(Byte & 0x60)) {
                    zmXPutByte(ZDLE);
                    Byte ^= 0x40;
                  }
                  LastSent = Byte;
                  zmXPutByte(Byte);
                  break;
    }
  }
}
