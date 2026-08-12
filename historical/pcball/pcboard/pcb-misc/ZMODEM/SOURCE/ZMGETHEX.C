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
 *** Decode two lower case hex digits into an 8-bit byte value *************/
int             _Cdecl  zmGetHex(void)
{
        register        HexByte,
                                UpperNybble;

        if ((HexByte = noXReadByte()) < 0)              /* fetch upper nybble of byte */
                return HexByte;
        UpperNybble = HexByte - '0';
        if (UpperNybble > 9)
                UpperNybble -= ('a' - ':');
        if (UpperNybble & ~0x0F)
                return ERROR;

        if ((HexByte = noXReadByte()) < 0)              /* fetch lower nybble of byte */
                return HexByte;
        HexByte -= '0';
        if (HexByte > 9)
                HexByte -= ('a' - ':');
        HexByte += (UpperNybble << 4);

        return HexByte;
}
