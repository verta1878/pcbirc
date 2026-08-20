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
int     _Cdecl  zmReadByte(void)
{
    register    c;

Again:
    if ((c = zmGetByte(rxTimeout)) & 0x60)
        return c;

    if (zmAbort)
      return(GOTCAN);

    switch (c) {
        case ZDLE :
            break;
        case 0x13 :
        case 0x93 :
        case 0x11 :
        case 0x91 :
            goto Again;
        default :
                if (ControlEncode && !(c & 0x60)) {
                    goto Again;
                }
            return c;
    }
Again2:
    if ((c = zmGetByte(rxTimeout)) < 0)
        return c;

    if (zmAbort)
      return(GOTCAN);

    if (c == CAN && (c = zmGetByte(rxTimeout)) < 0)
        return c;
    if (c == CAN && (c = zmGetByte(rxTimeout)) < 0)
        return c;
    if (c == CAN && (c = zmGetByte(rxTimeout)) < 0)
        return c;
    switch (c) {
        case CAN :
            return GOTCAN;
        case ZCRCE :
        case ZCRCG :
        case ZCRCQ :
        case ZCRCW :
            return (c | GOTOR);
        case ZRUB0 :
            return 0x1F;
        case ZRUB1 :
            return 0xFF;
        case 0x13 :
        case 0x93 :
        case 0x11 :
        case 0x91 :
            goto Again2;
        default :
                if (ControlEncode && !(c & 0x60))
                    goto Again2;
                if ((c & 0x60) == 0x40)
                    return c ^ 0x40;
            break;
    }
    if (Verbose > 1)
        zmError(1,"Bad escape sequence %x", c);

    return ERROR;
}
