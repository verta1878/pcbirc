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
 * $Revision:   1.3  $
 *
 * $Log:   E:/tc/pcbpack/vcs/repair.c_v  $
 * 
 *    Rev 1.3   06 Oct 1995 13:22:20   DWT
 * Made changes for porting to OS/2
 *
 *    Rev 1.2   11 Nov 1993 10:49:20   DWT
 * Changed to LIBENTRY
 *
 *    Rev 1.1   13 Oct 1993 09:24:34   DWT
 * Changed the way memcheck is used.
 *
 *    Rev 1.0   19 Aug 1993 19:34:20   LDZ
 * Initial revision.
 *
 ***************************************************************************/

#ifdef __BORLANDC__
  #include <dir.h>
#else
  #include <direct.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dosfunc.h>
#include <misc.h>
#include "newdata.h"
#include "pcb.h"
#include <screen.h>
#include "pcbpack.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define max(a,b)    (((a) > (b)) ? (a) : (b))


/***************************************************************************
 *** Fix a conference's Number of Active messages. *************************/
int LIBENTRY FixConf(unsigned Conf) {
    bool                IndexStat = IndexOnly;
    char                str[100],
                        Date[10],
                        Time[10];
    int                 Blocks;
    unsigned            MsgDate;
    register            i;
    long                MsgNum;
    long                ThrowAwayNum;
    unsigned long       BadBlocks = 0L,
                        NumActive = 0L;
    DOSFILE             lfp,
                        mfp;
    msgbasediskstattype MsgDiskStats;
    msgbasestattype     MsgStats;
    msgheadertype       Header;
    pcbconftype         ConfRec;
    extern char         *FixLog;

    IndexOnly = TRUE;

#ifdef  TOOLKIT
    if (getconfrecord(Conf, &ConfRec) == 0) {
    }
#else
    getconfrecord(Conf, &ConfRec);
#endif

#ifndef EXEC_FIX
    if (fileexist(ConfRec.MsgFile) == 255) {
        return FALSE;
    }

    /*** open the message base *********************************************/
    if (dosfopen(ConfRec.MsgFile, OPEN_RDWR|OPEN_DENYNONE, &mfp) == -1) {
        IndexOnly = IndexStat;
        return FALSE;
    }

    /*** read the message base header **************************************/
    if (dosfread(&MsgDiskStats, sizeof(msgbasediskstattype), &mfp) != sizeof(msgbasediskstattype)) {
        dosfclose(&mfp);
        IndexOnly = IndexStat;
        return FALSE;
    }
    dosfseek(&mfp, sizeof(msgheadertype) - sizeof(msgbasediskstattype), SEEK_CUR);
    MsgStats.LowMsgNum      = bassngltolong(MsgDiskStats.LowMsgNum);
    MsgStats.HighMsgNum     = bassngltolong(MsgDiskStats.HighMsgNum);
    MsgStats.NumActiveMsgs  = bassngltolong(MsgDiskStats.NumActiveMsgs);

    if (FixLog != NULL) {
        if (dosfopen(FixLog, OPEN_RDWR|OPEN_APPEND|OPEN_DENYNONE, &lfp) == -1) {
            FixLog = NULL;
        } else {
            sprintf(str, "Date: %s  Time: %s\r\n", datestr(Date), timestr2(Time));
            dosfputs(str, &lfp);
            sprintf(str, "Conference: (%u) %s\r\n", Conf, ConfRec.Name);
            dosfputs(str, &lfp);
        }
    }

    /*** read the messages *************************************************/
    while ((dosfread(&Header, sizeof(msgheadertype), &mfp) > 0) && (AbortPack == FALSE)) {

        // check for a valid header, plus get the current message number & date
        if (ValidHeader(&Header,&MsgNum,&MsgDate)) {
            NumActive++;

            DisplayLine(FALSE, ScreenColor->Intense, "Message number: %8ld", MsgNum);

            /*** Press ESC to abort ****************************************/
            if (Escape()) {
                AbortPack = TRUE;
                goto Abort;
            }

#ifndef WRONG_BLOCKS
            Blocks = Header.NumBlocks;
            for (i = 0; i < Blocks - 1; i++) {
                if (dosfread(&Header, sizeof(msgheadertype), &mfp) > 0) {
                    // check for a valid header, plus get the current message number & date - but don't bother using them
                    if (ValidHeader(&Header,&ThrowAwayNum,&MsgDate)) {
                        if (FixLog != NULL) {
                            sprintf(str, "Valid message header found %d blocks into the message.\r\n", i);
                            dosfputs(str, &lfp);
                            sprintf(str, "Number of message blocks: %d\r\n", Header.NumBlocks);
                            dosfputs(str, &lfp);
                        }
                        dosfseek(&mfp, -sizeof(msgheadertype), SEEK_CUR);
                        break;
                    }
                }
                /*** Press ESC to abort ****************************************/
                if (Escape()) {
                    AbortPack = TRUE;
                    goto Abort;
                }
            }
#else
            dosfseek(&mfp, (Header.NumBlocks - 1) * sizeof(msgheadertype), SEEK_CUR);
#endif

        } else {
            DisplayLine(FALSE, ScreenColor->Intense, "Invalid message block");
            BadBlocks++;
        }
    }

    if (FixLog != NULL) {
        sprintf(str, "Number of invalid message blocks : %ld\r\n", BadBlocks);
        dosfputs(str, &lfp);
    }

    /*** store the new number of active messages ***************************/
    longtobassngl(MsgDiskStats.LowMsgNum, MsgStats.LowMsgNum);
    longtobassngl(MsgDiskStats.HighMsgNum, max(MsgStats.HighMsgNum, MsgNum));
    longtobassngl(MsgDiskStats.NumActiveMsgs, MsgStats.NumActiveMsgs);
    dosrewind(&mfp);
    dosfwrite(&MsgDiskStats, sizeof(msgbasediskstattype), &mfp);

Abort:
    if (FixLog != NULL) {
        dosfclose(&lfp);
    }
    dosfclose(&mfp);

#else
    spawnlp(P_WAIT, "MSGFIX.EXE", "MSGFIX.EXE", ConfRec.MsgFile, NULL);
#endif

    IndexOnly = IndexStat;

    return TRUE;
}
