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
 * $Revision:   1.5  $
 *
 * $Log:   E:/tc/pcbpack/vcs/index.c_v  $
 *
 *    Rev 1.5   20 Sep 1995 18:44:02   DWT
 * Changed to LIBENTRY
 *
 *    Rev 1.4   22 Mar 1995 15:27:02   DWT
 * - Commented out the "updating message" display during the indexing process.
 * - Changed the way the file is truncated.
 *
 *    Rev 1.3   04 Feb 1995 15:08:12   DWT
 * - Made a change to handle dates by nul-terminating them because of a change
 *   in the julian date functions which previously were restricted to 2-digit
 *   years which now allow 4 digits.
 *
 *    Rev 1.2   11 Nov 1993 10:44:10   DWT
 * changed to LIBENTRY
 *
 *    Rev 1.1   13 Oct 1993 09:25:20   DWT
 * changed the way memcheck is used
 *
 *    Rev 1.0   19 Aug 1993 19:33:04   LDZ
 * Initial revision.
 *
 ***************************************************************************/
/***************************************************************************
 ***************************************************************************/

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <direct.h>
  #include <dos.h>
#else
  #include <dir.h>
#endif

#include <stdio.h>
#include <string.h>
#include <dosfunc.h>
#include <misc.h>
#include <pcb.h>
#include "crc.h"
#include "pcbpack.h"

#if defined(__BORLANDC__) || defined(__TURBOC__)
  extern struct ffblk DTA;
#elif defined(__WATCOMC__)
  extern struct find_t DTA;
  #define ff_name   name
  #define ff_ftime  wr_time
  #define ff_fdate  wr_date
  #define ff_fsize  size
  #define ff_attrib attrib
#endif


#ifdef DEBUG
#include <memcheck.h>
#endif


/***************************************************************************
 *** Update an IDX file if not "up to date" ********************************/
int LIBENTRY UpdateIndex(unsigned Conf) {
    bool                Reset;
    long                Offset;
    long                Num;
    long                LastNum;
    long                Size;
    long                BytesNeeded;
    char                Date[9];
    msgbasestattype     MsgStats;
    msgbasediskstattype DiskMsgStats;
    DOSFILE             ifp;
    DOSFILE             mfp;
    newindextype        Index;
    newindextype        Zeroes;
    char                idxFile[MAX_FILENAME],
                        msgFile[MAX_FILENAME];
    msgheadertype       Header;
    pcbconftype         ConfRec;

    getconfrecord(Conf, &ConfRec);

    if (ConfRec.MsgFile[0] == 0 || fileexist(ConfRec.MsgFile) == 255 ||
            ConfRec.Name[0] == 0)
        return FALSE;

    if (dosfopen(ConfRec.MsgFile, OPEN_READ|OPEN_DENYNONE, &mfp) == -1)
        return FALSE;

    DisplayLine(TRUE, ScreenColor->Intense, "Updating Conference (%u) %s", Conf, ConfRec.Name);

    strcpy(msgFile, ConfRec.MsgFile);
    strcpy(idxFile, ConfRec.MsgFile);
    AddExtension(idxFile, EXT_IDX);

    dosfread(&DiskMsgStats, sizeof(msgbasediskstattype), &mfp);
    MsgStats.HighMsgNum     = bassngltolong(DiskMsgStats.HighMsgNum);
    MsgStats.LowMsgNum      = bassngltolong(DiskMsgStats.LowMsgNum);
    MsgStats.NumActiveMsgs  = bassngltolong(DiskMsgStats.NumActiveMsgs);
    dosrewind(&mfp);

    if (fileexist(idxFile) == 255)
      Size = 0;
    else
      Size = DTA.ff_fsize;

    BytesNeeded = (((MsgStats.HighMsgNum - MsgStats.LowMsgNum) + 1) * sizeof(newindextype)) - Size;

    if (Escape()) {
        dosfclose(&mfp);
        AbortPack = TRUE;
        DisplayLine(TRUE, ScreenColor->Intense, "ESC pressed.  Aborting...");
        DisplayLine(TRUE, ScreenColor->Intense, "");
        return FALSE;
    }

    if (BytesNeeded == 0 || dosfopen(idxFile, OPEN_RDWR|OPEN_DENYNONE, &ifp) == -1) {
        dosfclose(&mfp);
        DisplayLine(TRUE, ScreenColor->Intense, "No update necessary...");
        DisplayLine(TRUE, ScreenColor->Intense, "");
        return FALSE;
    }

    Reset = FALSE;
    Offset = 128;

    if (Size == 0 || BytesNeeded < 0)
        Reset = TRUE;
    else {
        dosfread(&Index, sizeof(Index), &ifp);
        dosfseek(&mfp, 128, SEEK_SET);
        dosfread(&Header, sizeof(msgheadertype), &mfp);
        if (Index.Num == bassngltolong(Header.MsgNumber)) {
            dosfseek(&ifp, Size - sizeof(Index), SEEK_SET);
            dosfread(&Index, sizeof(Index), &ifp);

            if (Index.Offset < 0)
                Index.Offset = -Index.Offset;

            dosfseek(&mfp, Index.Offset, SEEK_SET);
            dosfread(&Header, sizeof(msgheadertype), &mfp);
            if (Index.Num == bassngltolong(Header.MsgNumber)) {
                Offset = Index.Offset + (128 * Header.NumBlocks);
                LastNum = Index.Num;
            } else {
                Reset = TRUE;
            }
        } else {
            Reset = TRUE;
        }
    }

    if (Reset) {
        LastNum = MsgStats.LowMsgNum - 1;
        dosrewind(&ifp);
    }

    memset(&Index, 0, sizeof(Index));
    Zeroes = Index;

    dossetbuf(&mfp, 2048);
    dossetbuf(&ifp, 32768U);
    dosfseek(&mfp, Offset, SEEK_SET);

    Date[8] = 0;

    while (dosfread(&Header, sizeof(msgheadertype), &mfp) == sizeof(msgheadertype)) {
        if (Escape()) {
            AbortPack = TRUE;
            goto Abort;
        }

        memcpy(Date,Header.Date,8);

        /*** skip over invalid headers *************************************/
        if ((Header.ActiveFlag == MSG_ACTIVE || Header.ActiveFlag == MSG_INACTIVE) &&
                Header.ToField[0] >= ' ' &&
                Header.FromField[0] >= ' ' &&
                Header.NumBlocks >= 2 &&
                (Index.Date = datetojulian(Date)) > 0 &&
                (Num = bassngltolong(Header.MsgNumber)) > LastNum) {

            DisplayLine(TRUE, ScreenColor->Intense, "Updating Message: %ld", Num);

            if (Num > LastNum + 1) {
                for (; LastNum < Num - 1; LastNum++) {
                    if (dosfwrite(&Zeroes, sizeof(Zeroes), &ifp) == -1) {
//                      DisplayLine(TRUE, ScreenColor->Intense, "Updating Message: %ld", Num);
                        break;
                    }
                }
            }

            LastNum      = Num;
            Index.Num    = Num;
            Index.Offset = (Header.ActiveFlag == MSG_ACTIVE ? Offset : -Offset);
            Index.Status = Header.Status;

// don't force to uppercase because the SYSTEM's table for uppercase may not
// be the same table that was used by the caller while the caller was online
//          {
//            char ToName[26];
//            memcpy(ToName,Header.ToField,25); ToName[25] = 0;
//            strupr(ToName);
//            memcpy(Index.To,ToName,25);
//          }
            memcpy(Index.To,Header.ToField,25);

            memcpy(Index.From, Header.FromField, 25);

            if (dosfwrite(&Index, sizeof(Index), &ifp) == -1)
                break;

            Offset += sizeof(msgheadertype) * Header.NumBlocks;
            dosfseek(&mfp, Offset, SEEK_SET);
        } else {
            Offset += sizeof(msgheadertype);
        }
    }

    while (LastNum < MsgStats.HighMsgNum) {
        if (dosfwrite(&Zeroes, sizeof(Zeroes), &ifp) == -1) {
//          DisplayLine(TRUE, ScreenColor->Intense, "Updating Message: %ld", LastNum);
            break;
        }
        LastNum++;
    }

    DisplayLine(TRUE, ScreenColor->Intense, "");

Abort:
    dosftrunc(&ifp,-1);           /*** truncate the file *******************/
    dosfclose(&ifp);
    dosfclose(&mfp);

    return TRUE;
}
