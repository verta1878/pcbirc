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

#include <ctype.h>
#include <pcbtools.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

long pascal dosgetlongtime(void);
long LIBENTRY doselapsedtime(long StartTime);

extern  void    pascal  scrollup(int, int, int, int, char);

extern  int         DebugMode;
extern  int         zmTransferStatus;


static void near pascal sendcps(long StartTime, long BytesSent) {
  int  InTransit;
  long Time;

  if ((Time = doselapsedtime(StartTime)) < 100)
    Time = 100;

  InTransit = OutBytes + SpoolBytes;
  if (InTransit > BytesSent)
    InTransit = (int) BytesSent;

  CPS = ((BytesSent - InTransit) * 100) / Time;   //lint !e573

  if (CPS >= 100000L)  // limit it to 5 digits
    CPS = 99999L;

  zmDisplay(71, 4, ZM_DISPLAY, "%5ld", CPS);
}


/***************************************************************************
 ***************************************************************************/
int pascal zmSendFileData(void) {
  static int      XOffset = 0;
  int             JunkCount = 0;
  int             key;
  int             c,
                  e,
                  NumBytes = 0;
  int             NewCount;
//int             CPSLimit;
  long            StartTime;
  long            StartPos;

//CPSLimit = (int) (Asy.ModemSpeed / 8);
  StartTime = dosgetlongtime();

  zmDisplay(45, 3, ZM_DISPLAY, "%s", tx32crc ? "32" : "16");
  BeenHere = FALSE;
  EndOfFile = FALSE;

  // first time into this function we've already received the ZRPOS
  // it appears that if we drop down to the zmGetInSync() function below that
  // we end up forcing it to WAIT FOR THE NEXT one.  So the three lines
  // below cheat and jump over the routine that picks up the next header
  // and allows it to continue on as if the header had already been received
  JunkCount = 0;
  c = ZRPOS;
  StartPos = txPosition;
  goto GotACK;
  //////////////////////////////////////////////////////////////////////////


SomeMore:
  if (InBytes) {

WaitACK:
    JunkCount = 0;
    c = zmGetInSync(0);

GotACK:
    switch (c) {
      case ERROR :
      default    :
      case ZCAN  : dosfclose(&in);
                   zmAbort = TRUE;
                   zmCancelTransfer();
                   return ERROR;
      case ZSKIP : dosfclose(&in);
                   return c;
      case ZACK  :
      case ZRPOS : break;
      case ZRINIT: dosfclose(&in);
                   return OK;
    }

    while (InBytes) {
      switch (zmGetByte(10)) {    // get a byte with 10 second timeout
        case CAN       : if (zmAbort) {
                           dosfclose(&in);
                           zmAbort = TRUE;
                           zmCancelTransfer();
                           return ERROR;
                         }
                         // fall thru
        case ZPAD      : c = zmGetInSync(1);
                         goto GotACK;
        case XOFF      :
        case XOFF|0x80 : zmGetByte(10); //lint !e534  // get a byte with 10 second timeout
      }
    }
  }

  NewCount = rxBufLength;
  txWCount = 0;
  zmStoreHeader(txPosition);
  zmSendBinaryHeader(4, ZDATA, txHeader);


  do {

/*** fill the transmit buffer **********************************************/
    NumBytes = dosfread(txBuffer, BlockLength, &in);
    if (NumBytes < BlockLength || txPosition+NumBytes >= FileSize) {
      e = ZCRCE;
      EndOfFile = TRUE;
    } else if (JunkCount > 3) {
      zmError(1,"Garbled Data Packet");
      e = ZCRCW;
    } else if (ByteCount == LastSync) {
      e = ZCRCW;
    } else if (rxBufLength && (NewCount -= NumBytes) <= 0) {
      e = ZCRCW;
    } else if (txWindow && (txWCount += NumBytes) >= txWSpace) {
      txWCount = 0;
      e = ZCRCQ;
    } else {
      e = ZCRCG;
    }

    if (NumBytes > 0) {
      zmSendData(txBuffer, NumBytes, e);
      ByteCount = txPosition += NumBytes;

      // keywords:  leech, leach, slugbait, cheat
      //
      // the code below is supposed to avoid leeches by checking to see if
      // the data sent so far is within 1K of the last byte of the file and
      // if so, we set the transfer status to SUCCESS regardless of what the
      // final outcome really is.

      if (FileSize > 4096 && txPosition > FileSize - 1024)
        zmTransferStatus = ZM_SUCCESS;
    }

    if (zmAbort) {
      zmCancelTransfer();
      break;
    }

    zmDisplay(68, 7, ZM_DISPLAY, "%8ld", ByteCount);

    zmGraph(ByteCount);
    sendcps(StartTime,ByteCount-StartPos);

    if (e == ZCRCW)
      goto WaitACK;

    while (InBytes) {
      switch (key = zmGetByte(10)) {   // get a byte with 10 second timeout
        case CAN      : if (zmAbort) {
                          dosfclose(&in);
                          zmAbort = TRUE;
                          zmCancelTransfer();
                          return ERROR;
                        }
                        // fall thru
        case ZPAD     : c = zmGetInSync(1);
                        if (c == ZACK)
                          break;
                        zmSendData(txBuffer, 0, ZCRCE);
                        goto GotACK;
        case XON      : if (c == ZACK) {     //lint !e644    I think 'c' was initialized up above - but are we sure???
                          if (DebugMode) {
                            zmDisplay(4, 24, 0x1F, " ZACK w/XON after ");
                          }
                        }
                        break;
        case XOFF     :
        case XOFF|0x80: if (zmGetByte(10) == XON) {   // get a byte with 10 second timeout
                          if (DebugMode) {
                            zmDisplay(4, 24, 0x1F, " XON after XOFF   ");
                          }
                        }
                        break;
        default       : ++JunkCount;
                        if (DebugMode) {
                          zmDisplay((XOffset * 3) + 4 , 23, 0x1B, "%02X", key);
                          zmDisplay(XOffset + 62, 23, 0x1B, "%c", key);
                          if (++XOffset == 16) {
                            scrollup(0, 18, 79, 23, 0x1B);
                            XOffset = 0;
                          }
                        }

      }
    }

    if (txWindow) {
      while ((txPosition - lrxPosition) >= txWindow) {
        if (e != ZCRCQ) {
          zmSendData(txBuffer, 0, e = ZCRCQ);
        }
        c = zmGetInSync(1);
        if (c != ZACK) {
          zmSendData(txBuffer, 0, ZCRCE);
          goto GotACK;
        }
      }
    }
  } while (!EndOfFile);

  // If we're this far then we've sent the entire file.
  //
  // If we're sending the file via telnet, then there may be buffers between
  // us as big as 100K in size which means that the receiver may be 100K
  // behind us.
  //
  // To avoid experiencing a timeout, we need to allow the receiver time
  // sufficient to receive the entire file before calling a timeout.  We'll
  // calculate how much time we should wait, by taking 100K and dividing it
  // by 1/2 of the CPS rate that we have detected thus far.  That will give
  // us the number of seconds to wait for the receiver to receive the file.
  // Then we'll add the standard 60 second timeout on top of that.

  if (CPS > 0) {
    long BytesInBigBuffer;

    if (txPosition < 100000L)
      BytesInBigBuffer = txPosition;
    else
      BytesInBigBuffer = 100000L;

    rxTimeout = 60 + (int) ((BytesInBigBuffer / CPS) * 2);
  }

  for (;;) {
    zmTransferStatus = ZM_SUCCESS;
    zmStoreHeader(txPosition);
    zmSendBinaryHeader(4, ZEOF, txHeader);

    sendcps(StartTime,txPosition-StartPos);
    zmError(0,"<END OF FILE>");

    switch (zmGetInSync(0)) {
      case ZACK  : continue;
      case ZRPOS : if (rxPosition == FileSize) {
                     sendcps(StartTime,txPosition-StartPos);
                     dosfclose(&in);
                     return(OK);
                   }
                   zmError(1,"got a ZRPOS");
                   goto SomeMore;
      case ZRINIT: sendcps(StartTime,txPosition-StartPos);
                   dosfclose(&in);
                   return OK;
      case ZSKIP : dosfclose(&in);
                   zmError(0,"File skipped by receiver");
                   return c;      //lint !e644   are we REALLY supposed to return 'c' here?
      default    : zmError(1,"EOF : Got %d.", FrameTypes[c]);  //lint !e644   was 'c' REALLY initialized?
                   dosfclose(&in);
                   // this is the last block, mark the transfer as failed,
                   // however, realize that this opens up the possibility
                   // for leeches!!!!
                   zmTransferStatus = ZM_FAIL;
                   return ERROR;
    }
  }
}
