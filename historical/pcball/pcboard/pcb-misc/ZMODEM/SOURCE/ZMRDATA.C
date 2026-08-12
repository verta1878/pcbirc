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
int pascal zmReceiveData(char *Buf, int Length) {
    int c;

    switch (rx32crc) {
        case 1 :
                c = zmReceiveData32(Buf, Length);
                if (kbdinkey() == 0x1B) {
                    zmAbort = TRUE;
                    c = ERROR;
                }
            return c;
        case 2 :
            c = zmReceiveData32RLE(Buf, Length);
            if (kbdinkey() == 0x1B) {
                zmAbort = TRUE;
                c = ERROR;
            }
            return c;
        default :
                c = zmReceiveData16(Buf, Length);
                if (kbdinkey() == 0x1B) {
                    zmAbort = TRUE;
                    c = ERROR;
                }
            return c;
    }

}
