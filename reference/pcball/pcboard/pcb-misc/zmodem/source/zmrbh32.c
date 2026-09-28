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
int pascal zmReceiveBinaryHeader32(char *Header) {
    register                c,
                            n;
    register unsigned long  crc;

    if ((c = zmReadByte()) & ~0xFF)
        return c;
    rxType = c;
    crc = 0xFFFFFFFFL;
    crc = updcrc32(c, crc);

    for (n = rxHeaderLength; --n >= 0; ++Header) {
        if ((c = zmReadByte()) & ~0xFF)
            return c;
        crc = updcrc32(c, crc);
        *Header = c;
    }
    for (n = 4; --n >= 0; ) {
        if ((c = zmReadByte()) & ~0xFF)
            return c;
        crc = updcrc32(c, crc);
    }
    if (crc != 0xDEBB20E3L) {
        zmError(1,"Bad CRC");
        return ERROR;
    }
    ZModem = 1;

    return rxType;
}
