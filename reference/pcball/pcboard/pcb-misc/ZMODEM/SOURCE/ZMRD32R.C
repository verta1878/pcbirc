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
#include <stdio.h>
#include "crc.h"
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif


/***************************************************************************
 ***************************************************************************/
int pascal zmReceiveData32RLE(char *Buf, int Length) {
    register char           *End;
    register int            c,
                            d;
    register unsigned long  crc;

    crc = 0xFFFFFFFFL;
    rxCount = 0;
    End = Buf + Length;
    d = 0;                              /*** Use for RLE decoder state *****/
    while (Buf <= End) {
        if ((c = zmReadByte()) & ~0377) {
crcfoo:
            switch (c) {
                case GOTCRCE:
                case GOTCRCG:
                case GOTCRCQ:
                case GOTCRCW:
                    d = c;  c &= 0377;
                    crc = updcrc32(c, crc);
                    if ((c = zmReadByte()) & ~0377)
                        goto crcfoo;
                    crc = updcrc32(c, crc);
                    if ((c = zmReadByte()) & ~0377)
                        goto crcfoo;
                    crc = updcrc32(c, crc);
                    if ((c = zmReadByte()) & ~0377)
                        goto crcfoo;
                    crc = updcrc32(c, crc);
                    if ((c = zmReadByte()) & ~0377)
                        goto crcfoo;
                    crc = updcrc32(c, crc);
                    if (crc != 0xDEBB20E3L) {
                        zmError(1,"Bad CRC");
                        return ERROR;
                    }

                    rxCount = Length - (int)(End - Buf);

                    zmDisplay(72, 5, ZM_DISPLAY, "%-4d", rxCount);

                    return d;
                    case GOTCAN:
                        zmError(1,"Sender Cancelled");
                        return ZCAN;
                    case TIMEOUT:
                        zmError(1,"TIMEOUT");
                        return c;
                    default:
                        zmError(1,"Bad data subpacket");
                        return c;
            }
        }
        crc = updcrc32(c, crc);
        switch (d) {
            case 0:
                if (c == ZRESC) {
                    d = -1;  continue;
                }
                *Buf++ = c;  continue;
            case -1:
                if (c >= 040 && c < 0100) {
                    d = c - 035; c = 040;  goto spaces;
                }
                if (c == 0100) {
                    d = 0;
                    *Buf++ = ZRESC;  continue;
                }
                d = c;
                continue;
            default:
                d -= 0100;
                if (d < 1)
                    goto badpkt;
spaces:
                if ((Buf + d) > End)
                    goto badpkt;
                while ( --d >= 0)
                    *Buf++ = c;
                d = 0;
                continue;
        }
    }
badpkt:
    zmError(1,"Data subpacket too long %d/%d",Length - (int)(End - Buf),Length);
    return ERROR;
}
