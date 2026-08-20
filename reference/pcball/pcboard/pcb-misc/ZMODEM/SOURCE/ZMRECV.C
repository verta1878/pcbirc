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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <screen.h>
#include "zmodem.h"
#ifdef MEMCHECK
#include <memcheck.h>
#endif


extern  void pascal closecallerlog(void);
extern  void pascal openlog(void);

extern  void pascal  makepcboardsys(void);
extern  bool pascal  istelnetviavmodem(void);


pcbconftype     CurConf;
bool            UseConfInfo;
bool            BatchTransfer = FALSE;
bool            ScanDlPath;
char            *ListFile = NULL;
char            UploadPath[MAX_FILENAME];
DOSFILE         list;
extern unsigned     _stklen = 0x8000;

void pascal initport(void);
void pascal openport(void);


#ifdef MEMCHECK
void shutdownmemcheck(void) {
  mc_endcheck();
}
#endif


void pascal atclosing(void) {
  char *p;
  char  Str[80];

  if (PcbData.ModemPort[0] == 'C' && ! PcbData.OS2Driver) {
    if (OverrunErrors != 0 || ParityErrors != 0 || FramingErrors != 0) {
      if (B16550A) {
        if (B16650)
          p = "16650 FIFO-32";
        else
          p = "16550A FIFO";
      } else if (B16550)
        p = "16550 FIFO";
      else if (B8250)
        p = "8250 (old)";
      else
        p = "8250A/16450";

      sprintf(Str,"Overrun Errors (Data Lost From UART): %d",OverrunErrors);
      fastprint(2,10,Str,0x0F);
      sprintf(Str,"Parity Errors                       : %d",ParityErrors);
      fastprint(2,11,Str,0x0F);
      sprintf(Str,"Framing Errors                      : %d",FramingErrors);
      fastprint(2,12,Str,0x0F);
      sprintf(Str,"UART Type                           : %s",p);
      fastprint(2,13,Str,0x0F);

      tickdelay(36);
    }
  }
}


/***************************************************************************
 *** Program Entry Point ***************************************************/
int main(int argc, char **argv) {
  bool       ForceRemote;
  register   Header,
             c,
             i;
  int        Color;
  int        Handle;

#ifdef  MEMCHECK
  mc_startcheck(erf_standard);
  atexit(shutdownmemcheck);
#endif

  if (initdoor("ZMRecv v1.00á", 0, 0, NOCLS) == -1)
    return(-1);

  ScanDlPath = TRUE;
  UseConfInfo = FALSE;
  if (opencnames() == 0) {
    if (getconfrecord(Status.Conference,&CurConf) == 0 && CurConf.Name[0] != 0)
      UseConfInfo = TRUE;
    closecnames();
  }

  gotoxy(16, 5);
  setcursor(CUR_BLANK);
    if (Scrn_Mode)
      Color = ZM_DISPLAY;
    else
      Color = 0x07;

  zmHeaderType = ZRINIT;
//BlockLength = 1024;
  BlockLength = 8192;

  for (i = 1; i < argc; i++) {
    switch (argv[i][0]) {
      case '-' :
      case '/' :
                  if (strncmpi(argv[i]+1,"FORCE",strlen(argv[i]+1)) == 0)
                    ForceRemote = TRUE;

                  if (strncmpi(argv[i]+1,"NODLPATH",strlen(argv[i]+1)) == 0)
                    ScanDlPath = FALSE;

                  if (strncmpi(argv[i]+1,"TELNET",strlen(argv[i]+1)) == 0)
                    Asy.Telnet = TRUE;
                  break;

      case '@' :  break;

      default :   strcpy(UploadPath, argv[i]);
                  stripright(UploadPath,'\\');
                  c = fileexist(UploadPath);
                  if (c == 255)
                    BatchTransfer = FALSE;
                  else if (c & 0x10)
                    BatchTransfer = TRUE;
                  else
                    goto AbortZModem;
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
        doswrite(i,"LOCAL CONNECTION\r\n",18);
        dosclose(i);
      }
      doswrite(1,"LOCAL CONNECTION\r\n",18);
      goto AbortZModem;
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

  if (PcbData.OS2Driver && istelnetviavmodem())
    Asy.Telnet = TRUE;

  fastprint(2, 2, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿", Color);
  fastprint(2, 3, "³ Receiving:                    ZModem CRC-16 (Batch)        File    of    ³", Color);
  fastprint(2, 4, "³ Status   : NONE                     Total Errors: 0   Average CPS:       ³", Color);
  fastprint(2, 5, "³                                                        Block Size:       ³", Color);
  fastprint(2, 6, "³   0...10...20...30...40...50...60...70...80...90...100%  Bytes         0 ³", Color);
  fastprint(2, 7, "³                                                          Done          0 ³", Color);
  fastprint(2, 8, "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ", Color);

  openlog();
  openusernet();

  Asy.IgnoreCDLoss = TRUE;

  // start out showing the current block length
  zmDisplay(72, 5, ZM_DISPLAY, "%-4d", BlockLength);

  if ((Header = zmTry()) != 0) {
    if (Header == ZCOMPL)
      goto done;
    if (Header == ERROR)
      goto ScrewedUp;
    if ((Header = zmReceiveFiles()) != 0)
      goto ScrewedUp;
  }

done:
  setcursor(CUR_NORMAL);
  closeusernet();
  closecallerlog();
  closedoor(FALSE);
  return OK;

ScrewedUp:
  setcursor(CUR_NORMAL);
  zmCancelTransfer();

  if (Status.SysopFlag != ' ')
    makepcboardsys();

AbortZModem:
  setcursor(CUR_NORMAL);
  closeusernet();
  closecallerlog();
  closedoor(FALSE);
  return ERROR;
}
