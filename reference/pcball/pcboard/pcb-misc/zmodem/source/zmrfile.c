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

#include <io.h>
#include <pcbtools.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

typedef union {
    struct ftime    fTime;
    long            uTime;
} TimeJunk;

extern int      zmSkipFile;
extern TimeJunk tj;
long static     StartTime;
long static     Time;


long pascal dosgetlongtime(void);
long LIBENTRY doselapsedtime(long StartTime);

static void near pascal abortfile(void) {
  zmWriteLog(ZM_RECV, ZM_FAIL);
  dosfclose(&in);
  unlink(FileName);
}

/***************************************************************************
 *** Receive a file using the ZModem protocol.  Assumes that filename ******
 *** will be found in SecBuf. **********************************************/
int pascal zmReceiveFile(void) {
  int         Header;
  int         n;
//int         CPSLimit;

  EndOfFile = FALSE;
  n = 20;
  rxBytes = 0L;

//CPSLimit = (int) (Asy.ModemSpeed / 8);

  if ((Header = zmProcessHeader(SecBuf)) != 0)
    return (zmHeaderType = Header);

  zmDisplay(45, 3, ZM_DISPLAY, "%s", rx32crc ? "32" : "16");
  showfileinusernet('U',FileName,FileSize);

  StartTime = dosgetlongtime();
  zmError(0,"NONE                   ");
  Warnings = 0;

  while (TRUE) {
    zmStoreHeader(rxBytes);
    zmSendHexHeader(4, ZRPOS, txHeader);
NextHeader:
    if (zmAbort) {
      zmError(0,"Operator abort!");
      abortfile();
      return ERROR;
    }
    switch (Header = zmGetHeader(rxHeader)) {
      case ZCAN   : zmError(1,"Sender cancelled");
                    abortfile();
                    return ERROR;
      case ZNAK   : if ( --n < 0) {
                      zmError(1,"Received ZNAK");
                      abortfile();
                      return ERROR;
                    }
                    continue;
      case TIMEOUT: if ( --n < 0) {
                      zmError(1,"TIMEOUT");
                      abortfile();
                      return ERROR;
                    }
                    continue;
      case ZFILE  : zmReceiveData(SecBuf, BlockLength);    //lint !e534
                    continue;
      case ZEOF   :
                    // Check to see if the size of the file is the same as
                    // what the sender told us we were to receive - if it
                    // is not, then ignore this header and wait for more
                    if (zmRecoverHeader(rxHeader) != rxBytes) {
                       // ignore EOF if it's at wrong place - force a timeout
                       // because the EOF might have gone out before we sent
                       // our ZRPOS.
                       Errors = 0;
                       goto NextHeader;
                     }

                    zmError(0,"<END OF FILE>");
                    zmDisplay(71, 4, ZM_DISPLAY, "%5ld", CPS);
                    zmDisplay(69, 7, ZM_DISPLAY, "%7ld", rxBytes);
                    zmGraph(rxBytes);

                    dosflush(&in);
                    if (dosfclose(&in) == -1) {     /*** Error closing *****/
                      zmHeaderType = ZFERR;
                      abortfile();
                      return ERROR;
                    }

                    if ((Time = doselapsedtime(StartTime)) < 100)
                      Time = 100;

                    if (rxBytes < 20000000L)
                      CPS = (rxBytes * 100) / Time;  //lint !e573
                    else
                      CPS = (rxBytes / Time) * 100;  //lint !e573

//                  if (CPS > CPSLimit)
//                    CPS = CPSLimit;

                    zmWriteLog(ZM_RECV, ZM_SUCCESS);
                    return Header;
      case ERROR  : /*** Too much garbage in header search error ***/
                    if ( --n < 0) {
                        zmError(1,"CRC Error");
                        abortfile();
                        return ERROR;
                    }
                    zmPutStr(Attn);
                    continue;
      case ZSKIP  : zmError(0,"File skipped by sender");
                    dosflush(&in);
                    setftime(in.handle, &tj.fTime);
                    dosfclose(&in);
                    return Header;
      case ZDATA  : if (zmRecoverHeader(rxHeader) != rxBytes) {
                      if (--n < 0) {
                        abortfile();
                        return ERROR;
                      }
                      zmPutStr(Attn);
                      continue;
                    }
MoreData:
                    if (zmAbort) {
                      zmError(0,"Operator abort!");
                      abortfile();
                      return ERROR;
                    } else if (zmSkipFile) {
                      zmSendHexHeader(4, ZSKIP, txHeader);
                      zmError(0,"Skipping file!");
                      zmSkipFile = FALSE;
                      continue;
                    }
                    Header = zmReceiveData(SecBuf, BlockLength);
                    zmDisplay(69, 7, ZM_DISPLAY, "%7ld", rxBytes);
                    switch (Header) {
                      case ZCAN   : abortfile();
                                    return ERROR;
                      case ERROR  : /* CRC error */
                                    if (--n < 0) {
                                      zmError(1,"CRC Error");
                                      abortfile();
                                      return ERROR;
                                    }
                                    zmPutStr(Attn);
                                    continue;
                      case TIMEOUT: if (--n < 0) {
                                      zmError(1,"TIMEOUT");
                                      abortfile();
                                      return ERROR;
                                    }
                                    continue;
                      case GOTCRCW: n = 20;
                                    if (zmPutSector(SecBuf, rxCount) == -1) {
                                       zmError(1,"ERROR WRITING FILE");
                                       abortfile();
                                       return ERROR;
                                    }
                                    rxBytes += rxCount;
                                    if ((Time = doselapsedtime(StartTime)) < 100)
                                        Time = 100;

                                    if (rxBytes < 20000000L)
                                      CPS = (rxBytes * 100) / Time;  //lint !e573
                                    else
                                      CPS = (rxBytes / Time) * 100;  //lint !e573

//                                  if (CPS > CPSLimit)
//                                    CPS = CPSLimit;

                                    zmStoreHeader(rxBytes);
                                    zmSendHexHeader(4, ZACK, txHeader);
                                    zmPutByte(XON);
                                    zmDisplay(71, 4, ZM_DISPLAY, "%5ld", CPS);
                                    zmGraph(rxBytes);
                                    goto NextHeader;
                      case GOTCRCQ: n = 20;
                                    if (zmPutSector(SecBuf, rxCount) == -1) {
                                       zmError(1,"ERROR WRITING FILE");
                                       abortfile();
                                       return ERROR;
                                    }
                                    rxBytes += rxCount;
                                    if ((Time = doselapsedtime(StartTime)) < 100)
                                        Time = 100;

                                    if (rxBytes < 20000000L)
                                      CPS = (rxBytes * 100) / Time;  //lint !e573
                                    else
                                      CPS = (rxBytes / Time) * 100;  //lint !e573

//                                  if (CPS > CPSLimit)
//                                    CPS = CPSLimit;

                                    zmStoreHeader(rxBytes);
                                    zmSendHexHeader(4, ZACK, txHeader);
                                    zmDisplay(71, 4, ZM_DISPLAY, "%5ld", CPS);
                                    zmGraph(rxBytes);
                                    goto MoreData;
                      case GOTCRCG: n = 20;
                                    if (zmPutSector(SecBuf, rxCount) == -1) {
                                       zmError(1,"ERROR WRITING FILE");
                                       abortfile();
                                       return ERROR;
                                    }
                                    rxBytes += rxCount;
                                    if ((Time = doselapsedtime(StartTime)) < 100)
                                        Time = 100;

                                    if (rxBytes < 20000000L)
                                      CPS = (rxBytes * 100) / Time;  //lint !e573
                                    else
                                      CPS = (rxBytes / Time) * 100;  //lint !e573

//                                  if (CPS > CPSLimit)
//                                    CPS = CPSLimit;

                                    zmDisplay(71, 4, ZM_DISPLAY, "%5ld", CPS);
                                    zmGraph(rxBytes);
                                    goto MoreData;
                      case GOTCRCE: n = 20;
                                    if (zmPutSector(SecBuf, rxCount) == -1) {
                                       zmError(1,"ERROR WRITING FILE");
                                       abortfile();
                                       return ERROR;
                                    }
                                    rxBytes += rxCount;
                                    if ((Time = doselapsedtime(StartTime)) < 100)
                                        Time = 100;

                                    if (rxBytes < 20000000L)
                                      CPS = (rxBytes * 100) / Time;  //lint !e573
                                    else
                                      CPS = (rxBytes / Time) * 100;  //lint !e573

//                                  if (CPS > CPSLimit)
//                                    CPS = CPSLimit;

                                    zmDisplay(71, 4, ZM_DISPLAY, "%5ld", CPS);
                                    zmGraph(rxBytes);
                                    goto NextHeader;
                    }
                    break;  // added this break on 9/15/94 - is it needed?
      default     : zmError(1,"Wrong header [%s]", FrameTypes[Header]);
                    if (--n < 0) {
                      abortfile();
                      return ERROR;
                    }
                    continue;
    }
  }

/*** unreachable code ******************************************************
    return ERROR;
 ***************************************************************************/
}
