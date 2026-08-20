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
 *** Recover a long integer from Header ************************************/
long    _Cdecl  zmRecoverHeader(register char *Header)
{
        register long   l;

        l = (Header[ZP3] & 0xFF);
        l = (l << 8) | (Header[ZP2] & 0xFF);
        l = (l << 8) | (Header[ZP1] & 0xFF);
        l = (l << 8) | (Header[ZP0] & 0xFF);

        return l;
}
