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
int     _Cdecl  zmSendInit(void)
{
    register    c;

    if (Attn[0] == 0 && (!ControlEncode || (rxFlags & TESCCTL)))
        return OK;

    Errors = 0;
    while (TRUE) {
        zmStoreHeader(0L);
        if (ControlEncode) {
            txHeader[ZF0] |= TESCCTL;
            zmSendHexHeader(4, ZSINIT, txHeader);
        } else {
            zmSendBinaryHeader(4, ZSINIT, txHeader);
        }
        zmSendData(Attn, ZATTNLEN, ZCRCW);
        c = zmGetHeader(rxHeader);

        switch (c) {
            case ZCAN :
                return ERROR;
            case ZACK :
                return OK;
            default :
                if (++Errors > 19)
                    return ERROR;
                continue;
        }
    }

/*** unreachable code ******************************************************
    return ERROR;
 ***************************************************************************/
}
