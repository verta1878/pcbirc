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


#include <dir.h>
#include <pcbtools.h>

#ifdef  MEMCHECK
# include <memcheck.h>
#endif

#include "zmodem.h"

extern  struct ffblk    DTA;


/***************************************************************************
 *** Count files left and bytes left ***************************************/
int     _Cdecl  zmCountFiles(int argc, char **argv)
{
    int             Exist;
    register        i;
    DOSFILE         list;

    for ((TotalLeft = FilesLeft = 0), i = 1; i < argc; i++) {
        switch (argv[i][0]) {
            case '-' :
            case '/' :
                break;
            case '@' :
                    if (dosfopen(argv[i] + 1, OPEN_READ|OPEN_DENYNONE, &list) == -1) {
                        return 0;
                    }
                    while (dosfgets(FileName, MAX_FILENAME, &list) != -1) {
                        Exist = fileexist(FileName);
                        if (Exist != 0xFF && ((Exist & (0x10|0x40)) == 0x00)) {
                            ++FilesLeft;
                            TotalLeft += DTA.ff_fsize;
                        }
                    }
                    dosfclose(&list);
                break;
            default :
                    Exist = fileexist(argv[i]);
                    if (Exist != 0xFF && ((Exist & (0x10|0x40)) == 0x00)) {
                        ++FilesLeft;
                        TotalLeft += DTA.ff_fsize;
                    }
                break;
        }
    }

    TotalFiles = FilesLeft;

    return TotalFiles;
}
