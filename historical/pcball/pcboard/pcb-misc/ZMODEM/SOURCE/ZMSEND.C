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


// #include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <pcbtools.h>

#include "zmodem.h"

#ifdef  MEMCHECK
# include <memcheck.h>
#endif

// extern void gotoxy(int, int);
// extern bool pascal istelnetviavmodem(void);

// this is a variable used by "C" to determine if
// daylight saving should be processed, we are going
// to permanently disable it down below.
#if __BORLANDC__ >= 0x500
extern int _daylight;
#define daylight _daylight
#else
extern int  daylight;
#endif



bool        BatchTransfer = FALSE;
char        UploadPath[MAX_FILENAME] = "";
int         zmTransferStatus;

static char    *ListFile = NULL;
static DOSFILE  list;

extern unsigned _stklen = 0x8000;

void pascal openport(void);


#ifdef MEMCHECK
void shutdownmemcheck(void) {
  mc_endcheck();
}
#endif



/***************************************************************************
 ***************************************************************************/
int main(int argc, char **argv) {
  bool ForceRemote;
  int  i;
  int  Color;
  int  Handle;

#ifdef  MEMCHECK
  mc_startcheck(erf_standard);
  atexit(shutdownmemcheck);
#endif

  daylight = 0;  // disable daylight savings time processing
  ForceRemote = FALSE;

  if (initdoor("ZMSend v1.00à", 0, 0, NOCLS) == -1)
    return(-1);

  for (i = 1; i < argc; i++) {
    switch (argv[i][0]) {
      case '-' :
      case '/' :
#ifdef  DEBUGMODE
                  if (strncmpi(argv[i]+1,"DEBUG",strlen(argv[i]+1)) == 0)
                    DebugMode = TRUE;
#endif
                  if (strncmpi(argv[i]+1,"FORCE",strlen(argv[i]+1)) == 0)
                    ForceRemote = TRUE;
//                if (strncmpi(argv[i]+1,"TELNET",strlen(argv[i]+1)) == 0)
//                  Asy.Telnet = TRUE;
                  break;
      case '@' :
                  ListFile = argv[i] + 1;
                  BatchTransfer = TRUE;
                  break;
      default :
                  strcpy(FileName, argv[i]);
                  break;
    }
  }

  if (Asy.Online != REMOTE) {
    if (ForceRemote) {
      Asy.Online = REMOTE;
      Asy.CarrierSpeed = Asy.ConnectSpeed = Asy.ModemSpeed = PcbData.ModemSpeed;
      initport();
      openport();
      online();
      if (! cdstillup())
        Asy.Online = LOCAL;
    }

    /* hopefully it has changed to REMOTE - so check again below */
    if (Asy.Online != REMOTE) {
      if ((i = doscreate(".\\PCBERR.FIL",OPEN_RDWR,OPEN_NORMAL)) != -1) {
        doswrite(i,"LOCAL CONNECTION\r\n",18);   //lint !e534
        dosclose(i);
      }
      doswrite(1,"LOCAL CONNECTION\r\n",18);   //lint !e534
      goto AbortZModem;
    }
  }

//if (PcbData.OS2Driver && istelnetviavmodem())
//  Asy.Telnet = TRUE;
  Asy.Telnet = FALSE;  // disable telnet for ZMSEND

  Asy.IgnoreCDLoss = TRUE;
  EffectiveBaud = (unsigned)Asy.CarrierSpeed;

  openusernet();   //lint !e534

  if (BatchTransfer) {
    if (dosfopen(ListFile, OPEN_READ|OPEN_DENYNONE, &list) == -1) {
//    goto AbortZModem;  // do we really want to abort completely?
      memset(&list,0,sizeof(list));
    }
  }

  if (getenv(DSZLOG) == NULL) {
    if ((i = doscreate(".\\PCBERR.FIL",OPEN_RDWR,OPEN_NORMAL)) != -1) {
      doswrite(i,"NO DSZLOG SETTING\r\n",19);   //lint !e534
      dosclose(i);
    }
    doswrite(1,"NO DSZLOG SETTING\r\n",19);   //lint !e534
    goto AbortZModem;
  }

  if ((Handle = doscreatecheck(getenv(DSZLOG),OPEN_WRIT,OPEN_NORMAL)) == -1) {
    if ((i = doscreate(".\\PCBERR.FIL",OPEN_RDWR,OPEN_NORMAL)) != -1) {
      doswrite(i,"CAN'T MAKE DSZLOG\r\n",19);   //lint !e534
      dosclose(i);
    }
    doswrite(1,"CAN'T MAKE DSZLOG\r\n",19);   //lint !e534
    goto AbortZModem;
  }

  dosclose(Handle);

  if (zmCountFiles(argc, argv) == 0) {
//  goto AbortZModem;   // do we really want to abort completely?
  }

  gotoxy(16, 5);
  setcursor(CUR_BLANK);
  if (Scrn_Mode)
    Color = ZM_DISPLAY;
  else
    Color = 0x07;

  fastprint(2, 2, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿", Color);
  fastprint(2, 3, "³ Sending  :                    ZModem CRC-16 (Batch)        File    of    ³", Color);
  fastprint(2, 4, "³ Status   : NONE                     Total Errors: 0   Average CPS:       ³", Color);
  fastprint(2, 5, "³                                                        Block Size:       ³", Color);
  fastprint(2, 6, "³   0...10...20...30...40...50...60...70...80...90...100%  Bytes         0 ³", Color);
  fastprint(2, 7, "³                                                          Done          0 ³", Color);
  fastprint(2, 8, "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ", Color);
#ifdef  DEBUGMODE
    if (DebugMode) {
      fastprint(0, 17, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿", Color);
      fastprint(0, 18, "³                                                           ³                  ³", Color);
      fastprint(0, 19, "³                                                           ³                  ³", Color);
      fastprint(0, 20, "³                                                           ³                  ³", Color);
      fastprint(0, 21, "³                                                           ³                  ³", Color);
      fastprint(0, 22, "³                                                           ³                  ³", Color);
      fastprint(0, 23, "³                                                           ³                  ³", Color);
      fastprint(0, 24, "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ", Color);
    }
#endif

  if (zmSendStartUp() != ERROR) {
    do {

      zmTransferStatus = ZM_FAIL;

      zmError(0,"NONE");
      zmDisplay(68, 3, Color, "%2d of %2d", (TotalFiles - FilesLeft) + 1, TotalFiles);

      if (BatchTransfer && list.handle > 0) {
        if (dosfgets(FileName, MAX_FILENAME, &list) == -1)
          break;
      }

      if (dosfopen(FileName, OPEN_READ|OPEN_DENYNONE, &in) == -1)
        continue;

      dossetbuf(&in,16384);
      EndOfFile = FALSE;
      vPos = 0;
      rxPosition = 0;
      Warnings = 0;
      if (zmSendFileName(FileName) == ERROR) {
        zmWriteLog(ZM_SEND, zmTransferStatus);
        continue;
      }

      showfileinusernet('D',FileName,FileSize);
//    clearinbuf();

      if (zmSendFileData() == ERROR) {
        zmWriteLog(ZM_SEND, zmTransferStatus);
        break;
      } else {
        zmWriteLog(ZM_SEND, zmTransferStatus);
      }

    } while (FilesLeft && !zmAbort);

    if (! zmAbort)
      zmGoodBye();
  }

  zmFlushSpooler();
  if (BatchTransfer && list.handle > 0)
    dosfclose(&list);

  setcursor(CUR_NORMAL);

AbortZModem:

  closeusernet();
  closedoor(FALSE);

  return ExitCode;
}
