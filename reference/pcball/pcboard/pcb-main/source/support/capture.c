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


#include "project.h"
#pragma hdrstop

#ifdef __WATCOMC__
  #include <io.h>
#endif

bool static OldCap   = FALSE;
bool static OldPrint = FALSE;


void LIBENTRY toggleprinter(void) {
   Status.Printer = (bool) (Status.Printer || OldPrint || prn <= 0 ? FALSE : TRUE);
   OldPrint = Status.Printer;
   redisplaystatusline();
}


void LIBENTRY closecapturefile(void) {
  OldCap = FALSE;
  dosfclose(&Status.CapFile);
  Status.Capture = FALSE;
  Status.CapFileExport = FALSE;
  redisplaystatusline();
}


int LIBENTRY opencapturefile(char *FileName) {
  char Date[9];
  char Time[9];
  char Str[80];

  if (FileName[0] == 0) {
    if (PcbData.CapFile[0] == 0) {
      lascii(Str,Status.CallerNumber);
      Str[7] = 0; /* limit it to 7 digits */
      sprintf(Status.CapFileName,"%s%s%c.CAP",PcbData.TmpLoc,Str,Status.MsgCapCount+'A');
    } else {
      strcpy(Str,PcbData.CapFile);
      if (Status.MsgCapCount > 0) {
        if (strlen(Str) == 8) {
          if (Str[7] + Status.MsgCapCount < 'Z')
            Str[7] += Status.MsgCapCount;
          else
            Str[7] = 'Z';
        } else {
          if (Status.MsgCapCount + '@' <= 'Z')
            addchar(Str,(char) (Status.MsgCapCount + '@'));
          else
            addchar(Str,'Z');
        }
      }
      sprintf(Status.CapFileName,"%s%s.CAP",PcbData.TmpLoc,Str);
    }
    unlink(Status.CapFileName);  /* remove it if it's already there */
  } else strcpy(Status.CapFileName,FileName);

  /* Note:  OPEN_APPEND is used - but it only affects captures to files that */
  /* are initiated by the sysop using the ALT-F command.  Read captures are  */
  /* deleted above even though they shouldn't already exist                  */

  if (dosfopen(Status.CapFileName,OPEN_RDWR|OPEN_APPEND|OPEN_DENYRDWR,&Status.CapFile) == -1) {
    memset(&Status.CapFile,0,sizeof(Status.CapFile));
    return(-1);
  }

  Status.Capture = TRUE;
  OldCap = TRUE;
  if (FileName[0] == 0) {
    dosfputs(PcbData.BoardName,&Status.CapFile);   /*lint !e534 */
    if (PcbData.Network) {
      sprintf(Str,"  Node %d ",PcbData.NodeNum);
      dosfputs(Str,&Status.CapFile);               /*lint !e534 */
    }
    sprintf(Str," %s %s\r\nName: %s\r\n",countrydate(datestr(Date)),timestr2(Time),Status.DisplayName);
    dosfputs(Str,&Status.CapFile);                 /*lint !e534 */
  }
  return(0);
}


void LIBENTRY setcapture(bool On) {
  if (On) {
    Status.Capture |= (bool) (OldCap && (Status.CapFile.handle != 0));  /*lint !e514  leave Status.Capture on if OldCap was on or if the capture file handle is non-zero */
    Status.Printer |= OldPrint;
  } else {
    if (Status.Capture || Status.Printer) {
      OldCap   = Status.Capture;
      OldPrint = Status.Printer;
      Status.Capture = FALSE;
      Status.Printer = FALSE;
    }
  }
}


void LIBENTRY openprinter(void) {
  char static Printer[] = "LPTx";
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if (PcbData.ParallelPortNum != 0) {
    Printer[3] = (char) (PcbData.ParallelPortNum+'0');
    if ((prn = dosopen(Printer,OPEN_WRIT|OPEN_DENYNONE POS2ERROR)) == -1)
      PcbData.ParallelPortNum = 0;

    /* NOTE:  the call to closedosopenfiles() in RECYCLE.C will close this */
  }
}
