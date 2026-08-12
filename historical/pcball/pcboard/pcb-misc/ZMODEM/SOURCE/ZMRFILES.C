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
int             _Cdecl  zmReceiveFiles(void)
{
        register        Header;

        while (TRUE) {
                switch (Header = zmReceiveFile()) {
                        case ZEOF :
                        case ZSKIP :
                                        switch (zmTry()) {
                                                case ZCOMPL :
                                                        return OK;
                                                case ZFILE :
                                                        break;
                                                default :
                                                        return ERROR;
                                        }
                                continue;
                        default :
                                return Header;
/*** LDZ -- 12/10/92 *******************************************************
 *** The following switch option is not necessary due to the fact that *****
 *** the above option will take care of just such an option. ***************
                        case ERROR :
                                return ERROR;
 ***************************************************************************/
                }
        }

/*** unreachable code ******************************************************
        return ERROR;
 ***************************************************************************/
}

