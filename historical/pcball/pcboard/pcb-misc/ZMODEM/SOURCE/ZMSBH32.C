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
void    _Cdecl  zmSendBinaryHeader32(int Length, register char *Header, int Type, int Flavor)
{
        register                                n;
        register unsigned long  crc;

        zmXPutByte(Flavor);
        if (VariableLengthHeaders)
                zmZPutByte(Length);
        zmZPutByte(Type);
        crc = 0xFFFFFFFFL;
        crc = updcrc32(Type, crc);

        for (n = Length; --n >= 0; ++Header) {
                crc = updcrc32((0xFF & *Header), crc);
                zmZPutByte(*Header);
        }

        crc = ~crc;
        for (n = 4; --n >= 0; ) {
                zmZPutByte((int) crc);
                crc >>= 8;
        }

        return;
}
