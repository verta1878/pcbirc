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
int pascal zmReceiveData16(char *Buf, int Length) {
    register char           *End;
    register unsigned short crc;
    register int            c,
                            d;

    crc = rxCount = 0;
    End = Buf + Length;
    while (Buf <= End) {
        if ((c = zmReadByte()) & ~0xFF) {
crcfoo:
            switch (c) {
                case GOTCRCE:
                case GOTCRCG:
                case GOTCRCQ:
                case GOTCRCW:
                        crc = updcrc16((d = c) & 0xFF, crc);
                        if ((c = zmReadByte()) & ~0xFF)
                            goto crcfoo;
                        crc = updcrc16(c, crc);
                        if ((c = zmReadByte()) & ~0xFF)
                            goto crcfoo;
                        crc = updcrc16(c, crc);
                        if (crc & 0xFFFF) {
                            zmError(1,"Bad CRC");
                            return ERROR;
                        }
                        rxCount = Length - (int)(End - Buf);
                    return d;
                case GOTCAN:
                        zmError(1,"Sender Cancelled");
                    return ZCAN;
                case TIMEOUT:
                        zmError(1,"TIMEOUT");
                    return c;
                default:
                        zmError(1,"Garbled data subpacket");
                    return c;
            }
        }
        *Buf++ = c;
        crc = updcrc16(c, crc);
    }
    zmError(1,"Garbled data subpacket");

    return ERROR;
}
