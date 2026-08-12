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
#include <sys/stat.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif


/***************************************************************************
 ***************************************************************************/
int pascal zmGetReceiverInit(void) {
  int         i;
  unsigned    SaveBlockLength;
  struct stat f;

  for (i = 0; i < 10; i++) {
    switch (zmGetHeader(rxHeader)) {
      case ZCHALLENGE : zmStoreHeader(rxPosition);
                        zmSendHexHeader(4, ZACK, txHeader);
                        continue;
      case ZCOMMAND   : zmStoreHeader(0L);
                        zmSendHexHeader(4, ZRQINIT, txHeader);
                        continue;
      case ZRINIT     : rxFlags = 0xFF & rxHeader[ZF0];

//  On 12/30/93 the line below was found to cause problems with Robocomm
//  version 4.1.  I have no idea why, but REM'ing it out cures the problem.
//
//                      VariableLengthHeaders = rxHeader[ZF1] & CANVHDR;


                        tx32fcs = (want32fcs && (rxFlags & CANFC32));
                        ControlEncode |= rxFlags & TESCCTL;
                        rxBufLength = (0xFF & rxHeader[ZP0]) + ((0xFF & rxHeader[ZP1]) << 8);
                        if (!(rxFlags & CANFDX))
                          txWindow = 0;
                        if (rxBufLength && (rxBufLength > txFrameLength) && (txFrameLength > 32))
                          rxBufLength = txFrameLength;
                        if (!rxBufLength && (txFrameLength >= 32) && (txFrameLength <= 1024))
                          rxBufLength = txFrameLength;

                        if (!Command) {
                          fstat(in.handle, &f);    //lint !e534
                          if ((f.st_mode & S_IFMT) != S_IFREG) {
                            CanSeek = -1;
//                          txWindow = TXBSIZE - 1024;
                            txWSpace = TXBSIZE / 4;
                          }
                        }

                        if (BlockLength < 1024) {
                          if (EffectiveBaud >= 300)
                            BlockLength = 256;
                          if (EffectiveBaud >= 1200)
                            BlockLength = 512;
                          if (EffectiveBaud >= 2400)
                            BlockLength = 1024;
                        }

                        zmDisplay(72, 5, ZM_DISPLAY, "%-4d", BlockLength);

                        SaveBlockLength = BlockLength;
                        if (rxBufLength && BlockLength > rxBufLength)
                          BlockLength = rxBufLength;
                        if (BlockOpt && BlockLength > BlockOpt)
                          BlockLength = BlockOpt;

                        if (BlockLength != SaveBlockLength)
                          zmDisplay(72, 5, ZM_DISPLAY, "%-4d", BlockLength);

                        if (lzTransport == ZTRLE && (rxFlags & CANRLE))
                          tx32fcs = 2;
                        else
                          lzTransport = 0;

                        return zmSendInit();
      case ZCAN       : return ERROR;
      case TIMEOUT    : return ERROR;
      case ZRQINIT    : if (rxHeader[ZF0] == ZCOMMAND)
      default         : zmSendHexHeader(4, ZNAK, txHeader);
                        continue;
    }
  }

  return ERROR;
}
