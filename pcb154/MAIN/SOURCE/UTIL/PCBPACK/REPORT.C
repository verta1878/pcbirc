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
 * Copyright (c) 1992, 1993.  All Rights Reserved.
 * Clark Development Company, Inc.
 *
 * $Revision:   1.4  $
 *
 * $Log:   E:/tc/pcbpack/vcs/report.c_v  $
 * 
 *    Rev 1.4   06 Oct 1995 13:26:14   DWT
 * Ported to OS/2
 *
 *    Rev 1.3   04 Feb 1995 15:06:36   DWT
 * Made changes to the reporting in order to handle long conference names.
 *
 *    Rev 1.2   11 Nov 1993 10:49:34   DWT
 * changed to LIBENTRY
 *
 *    Rev 1.1   13 Oct 1993 09:26:16   DWT
 * changed the way memcheck is used
 *
 *    Rev 1.0   19 Aug 1993 19:34:40   LDZ
 * Initial revision.
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <dosfunc.h>
#include <misc.h>
#include <pcb.h>
#include "pcbpack.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

msgbasestattype ReportInfo;
extern  char    RptFile[MAX_FILENAME];


static void _NEAR_ LIBENTRY confname(char *Str, char *Name) {
  int   X;
  char *p;

  strcpy(Str,Name);
  stripright(Str,' ');

  X = strlen(Str);
  p = &Str[X];
  X = 40 - X;

  for (; X > 0; X--) {
    *p = (X & 1 ? '.' : ' ');
    p++;
  }
  *p = 0;
}


/***************************************************************************
 *** Generate a report for information regarding the conference ************/
int LIBENTRY GenerateReport(unsigned Conf) {
    int                     LIndex = IndexOnly;
    DOSFILE                 mfp,
                            rfp;            /*** the report output file ****/
    char                    ConfName[80];
    char                    Str[100];       /*** scratch string ************/
    msgbasediskstattype     MsgDiskStats;
    msgbasestattype         MsgStats;
    pcbconftype             ConfRec;

    /*** get conference record from the CNAMES file ************************/
    getconfrecord(Conf, &ConfRec);

    /*** make sure the conference is valid *********************************/
    if (ConfRec.MsgFile[0] == 0 || fileexist(ConfRec.MsgFile) == 255 ||
            ConfRec.Name[0] == 0)
        return FALSE;

    IndexOnly = TRUE;
    DisplayLine(FALSE, ScreenColor->Intense, "Reporting on Conference (%u) %s", Conf, ConfRec.Name);
    DisplayLine(FALSE, ScreenColor->Intense, "");

    if (dosfopen(ConfRec.MsgFile, OPEN_READ|OPEN_DENYNONE, &mfp) == -1)
        return FALSE;

    if (dosfopen(RptFile, OPEN_RDWR|OPEN_APPEND|OPEN_DENYNONE, &rfp) == -1) {
        dosfclose(&mfp);
        return FALSE;
    }

    dosfread(&MsgDiskStats, sizeof(msgbasediskstattype), &mfp);
    ReportInfo.LowMsgNum        += MsgStats.LowMsgNum       =
                bassngltolong(MsgDiskStats.LowMsgNum);
    ReportInfo.HighMsgNum       += MsgStats.HighMsgNum      =
                bassngltolong(MsgDiskStats.HighMsgNum);
    ReportInfo.NumActiveMsgs    += MsgStats.NumActiveMsgs   =
                bassngltolong(MsgDiskStats.NumActiveMsgs);

    confname(ConfName,ConfRec.Name);
    sprintf(Str, "(%5d) %-40.40s%10ld%10ld%10ld\r\n", Conf, ConfName,
                MsgStats.LowMsgNum, MsgStats.HighMsgNum,
                MsgStats.NumActiveMsgs);
    dosfputs(Str, &rfp);

    IndexOnly = (bool) LIndex;

    dosfclose(&rfp);
    dosfclose(&mfp);

    return TRUE;
}
