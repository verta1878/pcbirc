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


#if defined(__BORLANDC__) || defined(__TURBOC__)
  #include <dir.h>
  #include <alloc.h>
#else
  #include <direct.h>
  #include <malloc.h>
#endif

#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <system.h>
#include <misc.h>
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifndef min
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define BUFLEN 2048

extern DOSFILE pcbfile;

char *DatFile = "PCBOARD.DAT";
char VerChange;

static char ansicolors[] = "04261537";

static void _NEAR_ LIBENTRY versionchange(char *Old) {
  char Str[80];
  bool Ans;

  boxcls(18,11,61,20,Colors[OUTBOX],SINGLE);
  sprintf(Str," VERSION CHANGE:  converting version %s ",Old);
  fastprint(19,13,Str,Colors[DESC]);
  fastprint(19,14," values in memory to version 15.0 format  ",Colors[DESC]);
  fastprint(19,15," (changes will not be made to disk until  ",Colors[DESC]);
  fastprint(19,16," saved at the end of the program).        ",Colors[DESC]);
  Ans = TRUE;
  ShowClock = NOCLOCK;
  inputnum(32,18,1,"Continue?  (Y/N)",&Ans,vBOOL,0);

  if (Ans == FALSE || KeyFlags == ESC) {
    cls();
    fastprint(0,0,"Files left unchanged...",0x07);
    gotoxy(0,2);
    exit(0);
  }
}


static char * _NEAR_ LIBENTRY getswitch(char *Str, char *Search) {
  char *p;
  bool SaveUpdateStatus;

  if ((p = strstr(Str,Search)) != NULL) {
    SaveUpdateStatus = UpdateKbdStatus;
    UpdateKbdStatus  = FALSE;

    boxcls(04,11,74,21,Colors[OUTBOX],SINGLE);
    fastcenter(13,Search,Colors[HEADING]);
    fastcenter(15,"The above PCBoard environment variable is not used by PCBoard v15.0.",Colors[QUESTION]);
    fastcenter(16,"After exiting PCBSetup (and SAVING the configuration), you",Colors[QUESTION]);
    fastcenter(17,"may remove it from the SET PCB= line inside of BOARD.BAT.",Colors[QUESTION]);
    beep();
    fastprintmove(27,19,"Press any key to continue",Colors[HELPKEY]);
    bgetkey(0);

    UpdateKbdStatus  = SaveUpdateStatus;
    return(p);
  }
  return(NULL);
}


static char * _NEAR_ LIBENTRY getpcbenv(void) {
  char *p;

  if ((p = getenv("PCB")) == NULL)
    return(NULL);

  strupr(p);
  return(p);
}


static void _NEAR_ LIBENTRY readenvironment(void) {
  char *p;
  char *q;

  PcbData.MaxScrollBack    = 0;
  PcbData.FastText         = FALSE;
  PcbData.FastCnames       = FALSE;
  PcbData.ShowSettings     = FALSE;
  PcbData.UploadBy         = FALSE;
  PcbData.TestUploads      = FALSE;
  PcbData.LeaveDtrUp       = TRUE;
  PcbData.ShareIRQs        = FALSE;
  PcbData.QuickScan        = FALSE;
  PcbData.ScanAll          = FALSE;
  PcbData.UseNewAskFile    = FALSE;
  PcbData.AllowOneName     = FALSE;
  PcbData.Slaves           = FALSE;
  PcbData.LowBaudLimit     = 1200;
  PcbData.AllowShell       = TRUE;
  PcbData.MaxConfMsgs      = 200;
  PcbData.MaxTotalMsgs     = 600;
  PcbData.StopClockOnCap   = FALSE;
  PcbData.ModemDelay       = 0;
  PcbData.RingsRequired    = 1;
  PcbData.IrqNum           = 0;
  PcbData.BaseAddress      = 0;
  PcbData.SubscriptionDays = 365;
  PcbData.SubscriptWarning = 30;
  PcbData.MinPriorToEvent  = 0;
  PcbData.DefExpiredLevel  = PcbData.UserLevels[SEC_REG];
  PcbData.UserLevels[SEC_EDIT]   = PcbData.UserLevels[SEC_K];
  PcbData.UserLevels[SEC_TEST]   = 0;
  PcbData.SysopSec[SEC_EDITALL]  = PcbData.SysopSec[SEC_READCOMMENTS];
  PcbData.SysopSec[SEC_READONLY] = PcbData.SysopSec[SEC_READCOMMENTS];
  PcbData.CapFile[0]             = 0;

  strcpy(PcbData.SysopStart,"00:00");
  strcpy(PcbData.SysopStop ,"23:59");

  strcpy(PcbData.ColorFile,"PCBFILER.DEF");
  maxstrcpy(PcbData.InfFile,PcbData.UsrFile,sizeof(PcbData.InfFile)-4);
  strcat(PcbData.InfFile,".INF");

  if ((p = getpcbenv()) == NULL)
    return;

  if ((q = getswitch(p,"/MAX:")) != NULL)
    PcbData.MaxScrollBack = (short) min(atoi(q+5),250);
  if ((q = getswitch(p,"/BAUD:")) != NULL)
    PcbData.LowBaudLimit = atol(q+6);
  if (getswitch(p,"/NOS") != NULL)
    PcbData.SysopSec[SEC_SUBS] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  else
    PcbData.SysopSec[SEC_SUBS] = 0;
  if (getswitch(p,"/INFO") != NULL)
    PcbData.ShowSettings = TRUE;
  if ((q = getswitch(p,"/DELAY:")) != NULL)
    PcbData.ModemDelay = (short) atoi(q+7);
}


static void _NEAR_ LIBENTRY read145environment(void) {
  char *p;
  char *q;

  strcpy(PcbData.SwapPath,".\\PCBSWAP.$$$");
  PcbData.SlowDrives[0] = 0;

  if ((p = strrchr(PcbData.ChtLoc,'\\')) != NULL) {
    /* check to see if the \ is the last character in the string to be */
    /* sure that it is safe to append a filename to it                 */
    if (*(p+1) == 0) {  /* was the \ the last character in the string? */
      if (PcbData.Network)
        strcpy(PcbData.StatsFile,PcbData.ChtLoc);
      else
        PcbData.StatsFile[0] = 0;
      strcat(PcbData.StatsFile,"PCBSTATS.DAT");
      strcpy(PcbData.ChatFile,PcbData.ChtLoc);
      strcat(PcbData.ChatFile,"CHAT.DAT");
    }
  } else {
    strcpy(PcbData.ChatFile,"CHAT.DAT");
    strcpy(PcbData.StatsFile,"PCBSTATS.DAT");
  }

  strcpy(PcbData.ChatMenu,PcbData.TxtLoc);
  strcpy(PcbData.NoAnsi  ,PcbData.TxtLoc);
  strcat(PcbData.ChatMenu,"CHTM");
  strcat(PcbData.NoAnsi  ,"NOANSI");

  strcpy(PcbData.EventDatFile,PcbData.ChtLoc);
  strcat(PcbData.EventDatFile,"EVENT.DAT");
  strcpy(PcbData.EventFiles  ,PcbData.ChtLoc);
  PcbData.CmdLst[0] = 0;
  PcbData.FileTcan[0] = 0;

  PcbData.EnableFido = FALSE;
  strcpy(PcbData.FidoConfig,PcbData.ChtLoc);
  strcat(PcbData.FidoConfig,"PCBFIDO.CFG");
  strcpy(PcbData.FidoQueue ,PcbData.ChtLoc);
  strcat(PcbData.FidoQueue ,"FIDOQUE.DAT");

  strcpy(PcbData.QwkFile,PcbData.CapFile);

  if (PcbData.DefaultColor >= 30 && PcbData.DefaultColor <= 37) {
    PcbData.NewDefaultColor = (char) (ansicolors[PcbData.DefaultColor - 30] - '0');
    if (PcbData.DefaultIntensity == 1)
      PcbData.NewDefaultColor += (char) 8;
  }

  PcbData.MsgHdr1Color  = 0x1F;
  PcbData.MsgHdr2Color  = 0x3F;
  PcbData.MsgHdr3Color  = 0x3F;
  PcbData.MsgHdr4Color  = 0x3F;
  PcbData.MsgHdr5Color  = 0x3E;
  PcbData.MsgHdr6Color  = 0x3E;

  PcbData.PromptToReadMail = FALSE;
  PcbData.Swap             = FALSE;
  PcbData.NoBatchUp        = FALSE;
  PcbData.ForceMain        = FALSE;
  PcbData.Foreign          = FALSE;
  PcbData.MonitorModem     = FALSE;
  PcbData.No16550          = FALSE;
  PcbData.Force16550A      = FALSE;
  PcbData.AutoReset        = FALSE;
  PcbData.Float            = FALSE;
  PcbData.Encrypt          = FALSE;
  PcbData.SwapDuringBat    = FALSE;
  PcbData.UserSysDuringBat = FALSE;
  PcbData.DefaultGraphics  = FALSE;
  PcbData.AllowAliasChange = FALSE;
  PcbData.ExcludeLocals    = FALSE;
  PcbData.RecordGroupChat  = FALSE;
  PcbData.AllowHandles     = FALSE;
  PcbData.ShowAlias        = FALSE;
  PcbData.MinPwrdLen       = 6;
  PcbData.PromoteBatch     = TRUE;
  PcbData.AutoRegConf      = TRUE;
  PcbData.ScanBlts         = TRUE;
  PcbData.AutoMakeMsgs     = TRUE;
  PcbData.VerifyCDLoss     = TRUE;
  PcbData.ByteCredit       = 0;
  PcbData.UploadCredit     = 10;
  PcbData.EnvSize          = 768;
  PcbData.LowBaudSecOverride = 0;
  PcbData.PwrdUpdate      = 0;
  PcbData.PwrdWarn        = 0;
  PcbData.AllFilesList[0] = 0;
  PcbData.SlowDriveBat[0] = 0;

  PcbData.SysopSec[SEC_15         ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_BROADCAST  ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_VIEWPRIV   ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_GENERICMSGS] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_EDITHDR    ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_PROTECT    ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_OVERWRITE  ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
/*PcbData.SysopSec[SEC_UNUSED     ] = PcbData.SysopSec[SEC_SYSOPLEVEL];*/
  PcbData.SysopSec[SEC_KEEPMSG    ] = PcbData.SysopSec[SEC_SYSOPLEVEL];
  PcbData.SysopSec[SEC_SEERETRCPT ] = PcbData.SysopSec[SEC_SYSOPLEVEL];

  if ((p = getpcbenv()) == NULL)
    return;

  if (getswitch(p,"/NOBATCHUP") != NULL)
    PcbData.NoBatchUp = TRUE;

  if (getswitch(p,"/MAIN") != NULL)
    PcbData.ForceMain = TRUE;

  if (getswitch(p,"/FOREIGN") != NULL)
    PcbData.Foreign = TRUE;

  if (getswitch(p,"/MONITORMODEM") != NULL)
    PcbData.MonitorModem = TRUE;

  if ((q = getswitch(p,"/UPCRED:")) != NULL)
    PcbData.UploadCredit = (short) timesten(q+8);

  if ((q = getswitch(p,"/BYTECRED:")) != NULL)
    PcbData.ByteCredit = (short) timesten(q+10);

  if (getswitch(p,"/NOPROMOTE") != NULL)
    PcbData.PromoteBatch = FALSE;

  if (getswitch(p,"/NOREG") != NULL)
    PcbData.AutoRegConf = FALSE;

  if (getswitch(p,"/NOBLTSCAN") != NULL)
    PcbData.ScanBlts = FALSE;

  if (getswitch(p,"/NOMSGS") != NULL)
    PcbData.AutoMakeMsgs = FALSE;

  if (getswitch(p,"/NO16550") != NULL)
    PcbData.No16550 = TRUE;

  if (getswitch(p,"/FORCE16550A") != NULL)
    PcbData.Force16550A = TRUE;

  if (getswitch(p,"/NOCDVERIFY") != NULL)
    PcbData.VerifyCDLoss = FALSE;

  if (getswitch(p,"/RESET") != NULL)
    PcbData.AutoReset = TRUE;

  if (getswitch(p,"/FLOAT") != NULL)
    PcbData.Float = TRUE;

  if (getswitch(p,"/ENCRYPT") != NULL)
    PcbData.Encrypt = TRUE;

  if ((q = getswitch(p,"/SWAP")) != NULL) {
    PcbData.Swap = TRUE;
    if (*(q+5) == ':') {
      maxstrcpy(PcbData.SwapPath,q+6,sizeof(PcbData.SwapPath));
      if ((q = strchr(PcbData.SwapPath,' ')) != NULL)
        *q = 0;
      if ((q = strchr(PcbData.SwapPath,'/')) != NULL)
        *q = 0;
    }
  }
}


static char FoundPath[66];

void LIBENTRY readpcbdat(char *DefaultLoc) {
  bool Ans;
  char Str[80];
  char *p;
  #ifdef __WATCOMC__
  unsigned CurDrive;
  #endif

  if (fileexist(DatFile) == 255) {
    #if ! (defined(_MSC_VER) || defined(__WATCOMC__))
      strcpy(FoundPath,_argv[0]);
      if ((p = strrchr(FoundPath,'\\')) != NULL || (p = strrchr(FoundPath,':')) != NULL) {
        strcpy(p+1,"PCBOARD.DAT");
        if (fileexist(FoundPath) != 255) {
          DatFile = FoundPath;
          goto open;
        }
      }
    #endif
    strcpy(FoundPath,"PCBOARD.DAT");
    if (srchpath(FoundPath) != -1) {
      DatFile = FoundPath;
      goto open;
    }
  }

open:
  if (DefaultLoc[0] != 0 || dosfopen(DatFile,OPEN_READ|OPEN_DENYNONE,&pcbfile) == -1) {
    if (DefaultLoc[0] == 0) {
      #ifndef __OS2__
        if (ExtendedError != 2) {
          sprintf(Str,"Unable to open file [%s]",DatFile);
          errorexittodos(Str);
        }
      #endif
      boxcls(18,8,61,20,Colors[OUTBOX],SINGLE);
      fastprint(19,10,"            MISSING PCBOARD.DAT           ",Colors[DESC]);
      fastprint(19,11,"                                          ",Colors[DESC]);
      fastprint(19,12," If desired, a NEW setup (pcboard.dat and ",Colors[DESC]);
      fastprint(19,13," cnames.*) will be created for you.       ",Colors[DESC]);
      fastprint(19,14,"                                          ",Colors[DESC]);
      fastprint(19,15," Enter the Drive Letter where you want    ",Colors[DESC]);
      fastprint(19,16," the PCBoard configuration files stored.  ",Colors[DESC]);
      #ifdef __WATCOMC__
        _dos_getdrive(&CurDrive);
        Ans = (char) (CurDrive + 'A');
      #else
        Ans = (char) (getdisk()+'A');
      #endif
      inputnum(22,18,1,"Drive Letter (or ESC to abort)",&Ans,vCHAR,0);
      if (KeyFlags == ESC)
        exit(0);
      strcpy(Str,"\\PCB");
    } else {
      if (DefaultLoc[1] == ':') {
        Ans = DefaultLoc[0];
        maxstrcpy(Str,DefaultLoc+2,sizeof(Str));
      } else {
        #ifdef __WATCOMC__
          _dos_getdrive(&CurDrive);
          Ans = (char) (CurDrive + 'A');
        #else
          Ans = (char) (getdisk()+'A');
        #endif
        maxstrcpy(Str,DefaultLoc,sizeof(Str));
      }
    }
    pcbdefaults(Ans,Str);
    return;
  }

  fgetstr(PcbData.Version,sizeof(PcbData.Version));
  if (strstr(PcbData.Version,"15.0") != NULL) {
    read150file();
    return;
  }

  if (strstr(PcbData.Version,"12.") != NULL) {
    versionchange("12.x");
    VerChange = 120;
    read120file();
    readenvironment();
    read145environment();
    return;
  }

  if (strstr(PcbData.Version,"14.") != NULL) {
    read150file();
    versionchange("14.x");
    VerChange = 145;
    if (strstr(PcbData.Version,"14.0") != NULL) {
      readenvironment();
      VerChange  = 140;
    }
    read145environment();
    return;
  }

  fastprint(0,1,"Cannot use current copy of PCBOARD.DAT file ... first line is:",0x0F);
  fastprint(0,2,PcbData.Version,0x07);
  gotoxy(0,4);
  exit(1);
}
