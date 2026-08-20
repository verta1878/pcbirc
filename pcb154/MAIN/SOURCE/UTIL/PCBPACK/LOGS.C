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


/***************************************************************************
 *
 * Copyright (c) 1993   All Rights Reserved.
 * Clark Development Company, Inc.
 *
 * $Revision:   1.2  $
 *
 * $Log:   E:/tc/pcbpack/vcs/logs.c_v  $
 * 
 *    Rev 1.2   06 Oct 1995 12:44:44   DWT
 * - Added support for PCB= environment variable to get the node number when
 *   updating the caller log.
 * 
 * - Ported to OS/2
 *
 *    Rev 1.1   13 Oct 1993 09:25:30   DWT
 * changed the way memcheck is used
 *
 *    Rev 1.0   19 Aug 1993 19:33:22   LDZ
 * Initial revision.
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dosfunc.h>
#include <misc.h>
#include "pcb.h"
#include "pcbpack.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

static int CallersLog;


/***************************************************************************
 ***************************************************************************/
void    LIBENTRY  closecallerlog(void)
{
    if (CallersLog > 0) {
        dosclose(CallersLog);
        CallersLog = 0;
    }
}


/***************************************************************************
 ***************************************************************************/
static  void    _NEAR_    LIBENTRY  writelogrecord(char *Buf)
{
    int     Handle;
    #ifdef __OS2__
      os2errtype Os2Error;
    #endif

    if (PcbData.Slaves) {
        if ((Handle = dosdup(CallersLog POS2ERROR)) != -1) {
            writecheck(Handle, Buf, LOGRECLEN);
            dosclose(Handle);
        }
    } else {
        writecheck(CallersLog, Buf, LOGRECLEN);
    }

}


/***************************************************************************
 ***************************************************************************/
void    LIBENTRY  repositionlogpointer(void)
{
    long    NumRecs;

    /* just in case someone has messed with our files - let's go to the end  */
    /* get the length of the file - then make it an even RECORD SIZE by then */
    /* seeking to the record number start location thus rounding off any     */
    /* offset that may have crept into the file (it happens!)                */

    if (CallersLog > 0) {
        NumRecs = doslseek(CallersLog, 0, SEEK_END) / LOGRECLEN;
        doslseek(CallersLog, NumRecs * LOGRECLEN, SEEK_SET);
    }
}


/***************************************************************************
 ***************************************************************************/
void LIBENTRY openlog(void) {
    char   *p;
    char    Name[66];
    char    Str[80];
    #ifdef __OS2__
      os2errtype Os2Error;
    #endif

    if (CallersLog > 0)   /* don't reopen the file if it's already open! */
        return;

    if (PcbData.ClrFile[0] == 0)  /* don't open until PcbData has been read in */
        return;

    if ((p = getenv("PCB")) != NULL) {
      strupr(p);
      if ((p = strstr(p,"/NODE:")) != NULL)
        PcbData.NodeNum = (short) atoi(p+6);
    }

    if (PcbData.Network)
        sprintf(Name, "%s%d", PcbData.ClrFile, PcbData.NodeNum);
    else
        strcpy(Name, PcbData.ClrFile);

    if ((CallersLog = dosopen(Name, OPEN_RDWR|OPEN_DENYNONE POS2ERROR)) == -1)
        if ((CallersLog = doscreate(Name, OPEN_RDWR|OPEN_DENYNONE, OPEN_NORMAL POS2ERROR)) == -1) {
            sprintf(Str, "Can't create caller log: [%s]", Name);
                errorexittodos(Str);
        }

    repositionlogpointer();
}


/***************************************************************************
 ***************************************************************************/
void    LIBENTRY  writelog(char *Str, padtype Pad)
{
    bool    Open;
    char    Buffer[LOGRECLEN + 1];

    Open = TRUE;
    if (CallersLog == 0) {
        Open = FALSE;
        openlog();
        if (CallersLog == 0)
            return;
    }

    switch (Pad) {
        case LEFTJUSTIFY  :
                memcpy(Buffer, Str, LOGTEXTLEN);
            break;
        case SPACERIGHT   :
                memset(Buffer, ' ', 6);
                memcpy(&Buffer[6], Str, LOGTEXTLEN - 6);
            break;
    }

    Buffer[LOGTEXTLEN] = 0;
    padstr(Buffer, ' ', LOGTEXTLEN);
    Buffer[LOGRECLEN - 2] = 13;
    Buffer[LOGRECLEN - 1] = 10;
    writelogrecord(Buffer);
    if (!Open)
        closecallerlog();
}
