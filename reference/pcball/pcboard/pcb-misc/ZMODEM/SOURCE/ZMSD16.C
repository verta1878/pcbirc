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
void    zmSendData16(register char *Buf, int Length, int FrameEnd)
{
        extern int                              zmAbort;
        register unsigned short crc;

        crc = 0;
        for (; --Length >= 0; ++Buf) {
                zmZPutByte(*Buf);
                crc = updcrc16((*Buf & 0xFF), crc);
        }
        zmXPutByte(ZDLE);
        zmXPutByte(FrameEnd);
        crc = updcrc16(FrameEnd, crc);
        crc = updcrc16(0, updcrc16(0, crc));
        zmZPutByte(crc >> 8);
        zmZPutByte(crc);
    if (kbdinkey() == 0x1B)
                zmAbort = TRUE;

        return;
}
