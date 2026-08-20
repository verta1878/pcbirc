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
 *** Calculate the number of bytes of free disk space **********************/
static  long    _Cdecl  zmFreeSpace(void)
{
    return ~0L;
}


/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */
int     _Cdecl  zmTry(void)
{
    register    Header,
                n;
#ifdef  ZMODEM_COMMAND
    register    cmdzack1flg;
#endif

    for (n = 0; n < 15;) {
        /*** Set buffer length (0) and capability flags ********************/
        zmStoreHeader(0L);

        txHeader[ZF0] = CANFC32|CANFDX|CANOVIO;
        if (ControlEncode)
            txHeader[ZF0] |= TESCCTL;

// try it without RLE because I'm no so sure RLE is working (12/31/94)
//      txHeader[ZF0] |= CANRLE;

        if (Asy.Telnet) {
          // the lines below were an attempt to force zmodem to escape control
          // characters for compatibility with the internet
//        txHeader[ZF0] = ESCCTL|CANFC32|CANFDX|CANOVIO|TESCCTL|CANRLE;

// try it without RLE because I'm no so sure RLE is working (12/31/94)
          txHeader[ZF0] = ESCCTL|CANFC32|CANFDX|CANOVIO|TESCCTL;
        }

        txHeader[ZF1] = CANVHDR;
        /*** tryzhdrtype may == ZRINIT ******/
        zmSendHexHeader(4, zmHeaderType, txHeader);
        if (zmHeaderType == ZSKIP)          /*** Don't skip too far *******/
            zmHeaderType = ZRINIT;          /*** CAF 8-21-87 **************/
Again:
        if (kbdinkey() == 0x1B) {
            return ERROR;
        }
        switch (zmGetHeader(rxHeader)) {
            case ZRQINIT:
                    if (rxHeader[ZF3] & 0x80)   /*** we can var header ****/
                        VariableLengthHeaders = 1;
                continue;
            case ZEOF:
                continue;
            case TIMEOUT:
                continue;
            case ZFILE:
                    zConv = rxHeader[ZF0];
                    zManag = rxHeader[ZF1];
                    zTrans = rxHeader[ZF2];
                    if (rxHeader[ZF3] & ZCANVHDR)
                        VariableLengthHeaders = TRUE;
                    zmHeaderType = ZRINIT;
                    Header = zmReceiveData(SecBuf, BlockLength);
                    if (Header == GOTCRCW)
                        return ZFILE;
                    zmSendHexHeader(4, ZNAK, txHeader);
                    zmError(1,"Sending NAK");
                goto Again;
            case ZSINIT:
                    ControlEncode = TESCCTL & rxHeader[ZF0];
                    if (zmReceiveData(Attn, ZATTNLEN) == GOTCRCW) {
                        zmStoreHeader(1L);
                        zmSendHexHeader(4, ZACK, txHeader);
                        goto Again;
                    }
                    zmSendHexHeader(4, ZNAK, txHeader);
                goto Again;
            case ZFREECNT:
                    zmStoreHeader(zmFreeSpace());
                    zmSendHexHeader(4, ZACK, txHeader);
                goto Again;
            case ZCOMMAND:
#ifdef  ZMODEM_COMMAND
                    cmdzack1flg = rxHeader[ZF0];
                    if (zmReceiveData(SecBuf, BlockLength) == GOTCRCW) {
                        if (cmdzack1flg & ZCACK1)
                            zmStoreHeader(0L);
                        else
                            zmStoreHeader((long)sys2(SecBuf));
                        purgeline();    /* dump impatient questions */
                        do {
                            zmSendHexHeader(4, ZCOMPL, txHeader);
                        } while (++Errors < 20 && zmGetHeader(rxHeader) != ZFIN);
                        zmGoodByeACK();
                        if (cmdzack1flg & ZCACK1)
                            exec2(SecBuf);
                        return ZCOMPL;
                    }
                    zmSendHexHeader(4, ZNAK, txHeader);
#endif
                goto Again;
            case ZCOMPL:
                goto Again;
            case ZFIN:
                    zmGoodByeACK();
                return ZCOMPL;
            case ZCAN:
                return ERROR;
            default:
                if (zmAbort)
                  return(ERROR);
                continue;
        }
    }

    return 0;
}
