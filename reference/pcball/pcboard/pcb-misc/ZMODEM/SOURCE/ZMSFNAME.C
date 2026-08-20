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
#include <string.h>
#include <sys/stat.h>

#ifdef  MEMCHECK
# include <memcheck.h>
#endif

#include "zmodem.h"


/***************************************************************************
 *** Send a file name block ************************************************/
int pascal zmSendFileName(char *Name) {
    char            *p;
    int             c;
    int             Len;
    struct stat     f;

    memset(txBuffer, 0x00, TXBSIZE);

    stripright(Name,'\\');  // strip any TRAILING backslashes first

    // then strip everything to the left of the last backslash leaving
    // just the filename in the txBuffer

    if ((p = strrchr(Name,'\\')) != NULL)
      p++;
    else if (Name[1] == ':')
      p = &Name[2];
    else
      p = Name;

    Len = strlen(p) + 1;
    memcpy(txBuffer,p,Len);
    p = &txBuffer[Len];

    zmDisplay(15, 3, ZM_DISPLAY, "%-15s", txBuffer);    /*** display the name **/

    strlwr(txBuffer);                       /*** now lower-case it *********/

    if (*Name) {
      if (fstat(in.handle, &f) != -1) {
        sprintf(p, "%lu %lo 0 0 %d %ld", f.st_size, f.st_mtime - 18000, /* f.st_mode, */
                   FilesLeft, TotalLeft);
        Len += strlen(p) + 1;
      }
      FileSize = f.st_size;
      TotalLeft -= f.st_size;
    } else
      memset(&f,0,sizeof(f));  // initialize 'f' to a known value...

    zmDisplay(68, 6, ZM_DISPLAY, "%8ld", f.st_size);

    if (--FilesLeft <= 0)
        TotalLeft = FilesLeft = 0;

    if (TotalLeft < 0)
        TotalLeft = 0;

    if (txBuffer[125]) {
        BlockLength = 1024;
    } else {
        txBuffer[127] = (f.st_size + 127) >> 7;
        txBuffer[126] = (f.st_size + 127) >> 15;
    }

    for (Errors = 0; Errors < 11; Errors++) {

        if (kbdinkey() == 0x1B) {
            Errors = 255;
            continue;
        }

        txHeader[ZF0] = lzConversion;
        txHeader[ZF1] = lzManagement;
        txHeader[ZF2] = lzTransport;
        txHeader[ZF3] = 0;

        zmSendBinaryHeader(4, ZFILE, txHeader);
        zmSendData(txBuffer, Len, ZCRCW);
//      zmSendData(txBuffer, (int)(1 + strlen(p) + (p - txBuffer)), ZCRCW);

Again:
        c = zmGetHeader(rxHeader);

        switch (c) {
            case ZRINIT :
                continue;
            case TIMEOUT :
                    zmError(1,"Timeout");
                return ERROR;
            case ZCAN :
            case ZABORT :
            case ZFIN :
                return ERROR;
            case ZCRC :
                goto Again;
            case ZFERR :
            case ZSKIP :
                return c;
            case ZRPOS :
                    dosflush(&in);
                    if (rxPosition > FileSize)
                      rxPosition = 0;
                    if ((dosfseek(&in, rxPosition, SEEK_SET)) == -1) {
                        return ERROR;
                    }
                    if (rxPosition < FileSize)
                        LastSync = (ByteCount = txPosition = lrxPosition = rxPosition) - 1;
                    else {
//                      LastSync = ByteCount = txPosition = lrxPosition = rxPosition;
                        return ERROR;
                    }
                return OK;
            case ERROR :
            case ZNAK :
            default :
                continue;
        }
    }

    return ERROR;
}
