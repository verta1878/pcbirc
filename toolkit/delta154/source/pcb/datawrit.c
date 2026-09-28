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
  #include <alloc.h>
#else
  #include <malloc.h>
#endif

#include <io.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <validate.h>
#include <dosfunc.h>
#include <stdlib.h>
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define logical(c) (c ? -1 : 0)

#define DIRNAME_LENGTH   33      /* length of longest paths to check      */

static bool PerformOldValue;
static char *OldDatF = "pcboard.old";
/* static char *PathListing = NULL; */
/* static int  NumNonDupes; */

pcbconftype WriteConf;

#ifdef __cplusplus
extern "C" {
#endif
/* these functions come from LIST.C which is compiled into PCBSETUP-ONLY. */
/* therefore, the only way this module can be used is with PCBSETUP or if */
/* another program were to supply these functions for DATAWRIT.C's usage  */
bool LIBENTRY allocatelist(int Size);
bool LIBENTRY foundinlist(char *Str);
void LIBENTRY deallocatelist(void);
#ifdef __cplusplus
}
#endif


static void _NEAR_ LIBENTRY validate(char Choice, char *Str, char *Reference) {
  if (PerformValidation && ! foundinlist(Str)) {
    checkexistence(Str,Choice,Reference,TRUE);
    if (! PerformValidation)     /* did user turn validation off?   */
      PerformOldValue = FALSE;   /* if so, force it off permanently */
  }
}

static char * _NEAR_ LIBENTRY strip(char *Str) {
  stripright(Str,' ');
  return(Str);
}

static char * _NEAR_ LIBENTRY strippath(char *Str) {
  stripall(Str,' ');
  return(Str);
}

static char * _NEAR_ LIBENTRY backs(char *Str, int MaxLen) {
  addbackslash(strippath(Str),MaxLen);
  validate(CHECK_DIR,Str,Str);
  return(Str);
}

static char * _NEAR_ LIBENTRY backs2(char *Str, char *Reference, int MaxLen) {
  addbackslash(strippath(Str),MaxLen);
  validate(CHECK_DIR,Str,Reference);
  return(Str);
}

static char * _NEAR_ LIBENTRY stripfile(char *Str) {
  strippath(Str);
  validate(CHECK_FILE,Str,Str);
  return(Str);
}

static char * _NEAR_ LIBENTRY stripfile2(char *Str, char *Reference) {
  strippath(Str);
  validate(CHECK_FILE,Str,Reference);
  return(Str);
}


static void _NEAR_ LIBENTRY validateblt(char *ConfName, char *FileNameLoc) {
  char    Buffer[31];
  DOSFILE File;

  if (ConfName[0] != 0 && FileNameLoc[0] != 0)
    if (dosfopen(FileNameLoc,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
      while (dosfread(Buffer,30,&File) == 30) {
        Buffer[30] = 0;
        stripfile2(Buffer,FileNameLoc);
      }
      dosfclose(&File);
    }
}

static void _NEAR_ LIBENTRY validatedir(char *ConfName, char *FileNameLoc) {
  char    Temp[31];
  char    Buffer[96];
  DOSFILE File;

  if (ConfName[0] != 0 && FileNameLoc[0] != 0)
    if (dosfopen(FileNameLoc,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
      while (dosfread(Buffer,96,&File) == 96) {
        memcpy(Temp,&Buffer[ 0],30); Temp[30] = 0;
        stripfile2(Temp,FileNameLoc);
        memcpy(Temp,&Buffer[30],30); Temp[30] = 0;
        backs2(Temp,FileNameLoc,sizeof(Temp));
      }
      dosfclose(&File);
    }
}

static void _NEAR_ LIBENTRY validatepth(char *ConfName, char *FileNameLoc) {
  DOSFILE File;
  char    Buffer[66];

  if (ConfName[0] != 0 && FileNameLoc[0] != 0)
    if (dosfopen(FileNameLoc,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
      while (dosfgets(Buffer,sizeof(Buffer)-1,&File) == 0) {
        if (Buffer[0] == '%')
          stripfile2(&Buffer[1],FileNameLoc);
        else
          backs2(Buffer,FileNameLoc,sizeof(Buffer));
      }
      dosfclose(&File);
    }
}

static void _NEAR_ LIBENTRY validatescr(char *ConfName, char *FileNameLoc) {
  char    Buffer2[31];
  char    Buffer[66];
  DOSFILE File;

  if (ConfName[0] != 0 && FileNameLoc[0] != 0)
    if (dosfopen(FileNameLoc,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
      while (dosfread(Buffer,60,&File) == 60) {
        memcpy(Buffer2,&Buffer[30],30);
        Buffer[30] = Buffer2[30] = 0;
        stripfile2(Buffer ,FileNameLoc);
        stripfile2(Buffer2,FileNameLoc);
      }
      dosfclose(&File);
    }
}

static bool _NEAR_ LIBENTRY asktochecklist(char *Name) {
  char Quest[80];

  if (! PerformValidation)
    return(FALSE);

  strcpy(Quest,"Check all paths in all ");
  strcat(Quest,Name);
  strcat(Quest," List files");

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.X1        = 4;
  MsgData.Y1        = 18;
  MsgData.X2        = 74;
  MsgData.Y2        = 22;
  MsgData.QuestLine = 20;
  MsgData.Quest     = Quest;
  MsgData.Save      = TRUE;
  MsgData.Answer[0] = 'Y';
  MsgData.Answer[1] = 0;
  MsgData.Mask      = YESNO;
  showmessage();

  if (KeyFlags == ESC || MsgData.Answer[0] != 'Y')
    MsgData.Answer[0] = FALSE;

  return(MsgData.Answer[0]);
}


void LIBENTRY writedatafile(void) {
  int     x;
  DOSFILE pcbfile;
  char    Temp[35];
  char    Buffer[2048];

  #ifdef DEBUG
    mc_register(Buffer,sizeof(Buffer));
    mc_register(Temp,sizeof(Temp));
  #endif

  checkexistence(DatFile,CHECK_FILE,NULL,FALSE);

/*don't delete any more, instead, truncate the file down below */
/*unlink(DatFile);*/

  if (dosfopen(DatFile,OPEN_WRIT|OPEN_DENYRDWR,&pcbfile) == -1) {
    #ifdef DEBUG
      mc_unregister(Buffer);
      mc_unregister(Temp);
    #endif
    return;
  }

  dosftrunc(&pcbfile,0);          /* truncate the file at 0 bytes */

  sprintf(Buffer,"*** PCBoard Version 14.5 & 15.0 data file ***\r\n");
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%d\r\n%d\r\n",
          strip(PcbData.Sysop),
          strip(PcbData.Password),
          logical(PcbData.UseRealName),
          logical(PcbData.Graphics));
  dosfputs(Buffer,&pcbfile);

  for (x = SEC_READCOMMENTS; x <= SEC_14; x++) {
    sprintf(Buffer,"%d\r\n",PcbData.SysopSec[x]);
    dosfputs(Buffer,&pcbfile);
  }

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          backs(PcbData.HlpLoc,sizeof(PcbData.HlpLoc)),
          backs(PcbData.SecLoc,sizeof(PcbData.SecLoc)),
          backs(PcbData.ChtLoc,sizeof(PcbData.ChtLoc)),
          backs(PcbData.TxtLoc,sizeof(PcbData.TxtLoc)),
          backs(PcbData.NdxLoc,sizeof(PcbData.NdxLoc)));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          stripfile(PcbData.UsrFile),
          stripfile(PcbData.ClrFile),
          stripfile(PcbData.CnfFile),
          stripfile(PcbData.PwdFile),
          stripfile(PcbData.FscFile),
          stripfile(PcbData.UscFile),
          stripfile(PcbData.TcnFile),
          stripfile(PcbData.WlcFile));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          stripfile(PcbData.NewFile),
          stripfile(PcbData.ClsFile),
          stripfile(PcbData.WrnFile),
          stripfile(PcbData.ExpFile),
          stripfile(PcbData.NetFile),
          stripfile(PcbData.CnfMenu),
          stripfile(PcbData.RegFile),
          stripfile(PcbData.AnsFile));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          stripfile(PcbData.TrnFile),
          stripfile(PcbData.DldFile),
          stripfile(PcbData.LogOffScr),
          stripfile(PcbData.LogOffAns),
          stripfile(PcbData.MultiLang),
          stripfile(PcbData.GroupChat));
  dosfputs(Buffer,&pcbfile);


  strcpy(Temp,PcbData.ModemPort);
  if (Temp[0] == 'C') {            /* then it must be a COMM port so add */
    Temp[4] = ':';                 /* a colon to the name of the port    */
    Temp[5] = 0;
  } else
    strip(Temp);


  sprintf(Buffer,"%d\r\n%s\r\n%ld\r\n%d\r\n%s\r\n%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%s\r\n%s\r\n",
          PcbData.Seconds,
          Temp,
          PcbData.ModemSpeed,
          logical(PcbData.LockSpeed),
          strip(PcbData.ModemInit),
          strip(PcbData.ModemOff),
          logical(PcbData.DisableCTS),
          logical(PcbData.Hayes9600),
          logical(PcbData.FastComm),
          logical(PcbData.ResetModem),
          logical(PcbData.OffHook),
          logical(PcbData.Packet),
          logical(PcbData.AnswerRing),
          logical(PcbData.Allow7E1),
          logical(PcbData.AllowLowBaud),
          strip(PcbData.AllowLowStrt),
          strip(PcbData.AllowLowStop));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          logical(PcbData.DisableScan),
          logical(PcbData.DisableCls),
          logical(PcbData.DisableEdits),
          logical(PcbData.DisableFilter),
          logical(PcbData.DisableQuick),
          logical(PcbData.DisablePassword),
          logical(PcbData.MultiLingual),
          logical(PcbData.AllowPwrdOnly),
          logical(PcbData.ClosedBoard),
          logical(PcbData.NonGraphics),
          logical(PcbData.ExitToDos),
          logical(PcbData.IncludeCity),
          logical(PcbData.EliminateSnow));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          logical(PcbData.SubscriptMode),
          logical(PcbData.AllowEscCodes),
          logical(PcbData.AllowCCs),
          logical(PcbData.Validate),
          logical(PcbData.LastReadUpdate),
          logical(PcbData.EnforceTime),
          (PcbData.DisplayNews == 'Y' ? -1 : (PcbData.DisplayNews == 'A' ? 1 : 0)),
          logical(PcbData.DisableCTSdrop),
          PcbData.KbdTimeout,
          logical(PcbData.Network),
          PcbData.NodeNum,
          PcbData.NetTimeout,
          PcbData.ChatDelay);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n.%s\r\n",       /* NOTE:  '.' added on purpose */
          strip(PcbData.BoardName),
          strippath(PcbData.ViewBatch),
          strippath(PcbData.ViewExt));
  dosfputs(Buffer,&pcbfile);

  for (x = 0; x < 10; x++) {
    sprintf(Buffer,"%s\r\n",strip(PcbData.FuncKeys[x]));
    dosfputs(Buffer,&pcbfile);
  }

  sprintf(Buffer,"\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%u\r\n",
          PcbData.NumConf,
          PcbData.MaxMsgLines,
          PcbData.DefaultColor,
          PcbData.DefaultIntensity,
          logical(PcbData.EventActive),
          strip(PcbData.EventTime),
          PcbData.EventSuspend,
          logical(PcbData.EventStopUplds),
          logical(PcbData.EventSlide),
          PcbData.UploadBufSize,
          logical(PcbData.DisableDriveCheck),
          PcbData.ParallelPortNum,
          PcbData.StopFreeSpace);
  dosfputs(Buffer,&pcbfile);

  for (x = SEC_A; x <= SEC_BATCH; x++) {
    sprintf(Buffer,"%d\r\n",PcbData.UserLevels[x]);
    dosfputs(Buffer,&pcbfile);
  }

  sprintf(Buffer,"%s\r\n%ld\r\n%d\r\n%d\r\n%d\r\n%X\r\n%d\r\n",
          stripfile(PcbData.ColorFile),
          PcbData.LowBaudLimit,
          PcbData.ModemDelay,
          PcbData.RingsRequired,
          PcbData.IrqNum,
          PcbData.BaseAddress,
          logical(PcbData.LeaveDtrUp));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          PcbData.MaxScrollBack,
          logical(PcbData.FastText),
          logical(PcbData.FastCnames),
          PcbData.SysopSec[SEC_SUBS],
          logical(PcbData.UploadBy),
          logical(PcbData.ShowSettings),
          logical(PcbData.AllowShell),
          logical(PcbData.Slaves),
          PcbData.SubscriptionDays);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n",
          PcbData.MaxTotalMsgs,
          PcbData.MaxConfMsgs,
          PcbData.MinPriorToEvent);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%s\r\n%s\r\n",
          PcbData.SysopSec[SEC_EDITALL],
          PcbData.UserLevels[SEC_EDIT],
          logical(PcbData.ShareIRQs),
          logical(PcbData.ScanAll),
          PcbData.SysopSec[SEC_READONLY],
          logical(PcbData.UseNewAskFile),
          backs(PcbData.TmpLoc,sizeof(PcbData.TmpLoc)),
          stripfile(PcbData.InfFile));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%s\r\n%d\r\n%d\r\n%s\r\n%s\r\n",
          logical(PcbData.QuickScan),
          PcbData.SubscriptWarning,
          logical(PcbData.AllowOneName),
          PcbData.DefExpiredLevel,
          PcbData.UserLevels[SEC_TEST],
          strippath(PcbData.CapFile),
          logical(PcbData.TestUploads),
          logical(PcbData.StopClockOnCap),
          PcbData.SysopStart,
          PcbData.SysopStop);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          logical(PcbData.LogCallerNumber),
          logical(PcbData.LogConnectStr),
          logical(PcbData.LogSecLevel),
          logical(PcbData.ConfPwrdAdjust),
          logical(PcbData.ConfirmCaller),
          logical(PcbData.AllowPwrdComment),
          logical(PcbData.GuardLogoff),
          PcbData.NumDescLines);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          stripfile(PcbData.ChatFile),
          stripfile(PcbData.StatsFile),
          stripfile(PcbData.ChatMenu),
          stripfile(PcbData.NoAnsi),
          strip(PcbData.SlowDrives),
          stripfile(PcbData.SwapPath));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          logical(PcbData.Swap),
          logical(PcbData.NoBatchUp),
          logical(PcbData.ForceMain),
          logical(PcbData.Foreign),
          logical(PcbData.MonitorModem),
          logical(PcbData.No16550),
          logical(PcbData.Force16550A),
          logical(PcbData.AutoReset),
          logical(PcbData.Float),
          logical(PcbData.PromoteBatch),
          logical(PcbData.AutoRegConf),
          logical(PcbData.ScanBlts),
          logical(PcbData.AutoMakeMsgs),
          logical(PcbData.VerifyCDLoss),
          logical(PcbData.Encrypt),
          PcbData.UploadCredit,
          PcbData.ByteCredit,
          PcbData.NewDefaultColor,
          PcbData.MsgHdr1Color,
          PcbData.MsgHdr2Color,
          PcbData.MsgHdr3Color,
          PcbData.MsgHdr4Color,
          PcbData.MsgHdr5Color,
          PcbData.MsgHdr6Color);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%d\r\n",
          strip(PcbData.Origin),
          stripfile(PcbData.EventDatFile),
          backs(PcbData.EventFiles,sizeof(PcbData.EventFiles)),
          stripfile(PcbData.CmdLst),
          logical(PcbData.RequirePwrdToExit));
  dosfputs(Buffer,&pcbfile);

  /* NOTE:  SEC_UNUSED used to be SEC_MOVE which was redundant as SEC_MOVEMSG */
  /* was already for that purpose.  But now we have to write out SEC_UNUSED   */
  /* anyway or we'll get out of sync.                                         */

  for (x = SEC_15; x <= SEC_UNUSED; x++) {
    sprintf(Buffer,"%d\r\n",PcbData.SysopSec[x]);
    dosfputs(Buffer,&pcbfile);
  }

  sprintf(Buffer,"%d\r\n%d\r\n%s\r\n%d\r\n%s\r\n%s\r\n%d\r\n",
          PcbData.EnvSize,
          PcbData.LowBaudSecOverride,
          stripfile(PcbData.AllFilesList),
          logical(PcbData.EnableFido),
          strip(PcbData.FidoConfig),        /* don't use stripfile() because they MIGHT NOT be using FIDO! */
          PcbData.FidoQueue,                /* I think this is no longer used */
          logical(PcbData.PromptToReadMail));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n",
          stripfile(PcbData.LogOnScr),
          stripfile(PcbData.LogOnAns),
          strippath(PcbData.QwkFile),
          stripfile(PcbData.FileTcan));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          PcbData.SysopSec[SEC_KEEPMSG],
          PcbData.SysopSec[SEC_SEERETRCPT],
          logical(PcbData.SwapDuringBat),
          logical(PcbData.UserSysDuringBat),
          logical(PcbData.DefaultGraphics),
          stripfile(PcbData.SlowDriveBat),
          logical(PcbData.AllowAliasChange),
          logical(PcbData.ExcludeLocals),
          PcbData.PwrdUpdate,
          PcbData.PwrdWarn,
          logical(PcbData.RecordGroupChat),
          logical(PcbData.ShowAlias),
          PcbData.MinPwrdLen,
          logical(PcbData.AllowHandles));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          backs(PcbData.CmdLoc,sizeof(PcbData.CmdLoc)),
          logical(PcbData.ForceIntro),
          logical(PcbData.SkipProtocol),
          logical(PcbData.SkipAlias),
          logical(PcbData.NoCarrierExit),
          logical(PcbData.OS2Driver));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%s\r\n%s\r\n",
          logical(PcbData.EnableAccounting),
          logical(PcbData.ShowCurrency),
          logical(PcbData.Concurrent),
          strip(PcbData.PeakStart),
          strip(PcbData.PeakEnd));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          stripfile(PcbData.AccountConfig),
          stripfile(PcbData.AccountInfo),
          stripfile(PcbData.AccountWarn),
          stripfile(PcbData.AccountTrack),
          stripfile(PcbData.AccountLogoff));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          backs(PcbData.uucpPath,sizeof(PcbData.uucpPath)),
          backs(PcbData.uucpSpoolPath,sizeof(PcbData.uucpSpoolPath)),
          backs(PcbData.uucpLogPath,sizeof(PcbData.uucpLogPath)),
          PcbData.CompBatFile,  /* don't use stripfile() for this line because we're no longer using it */
          stripfile(PcbData.DeCompBatFile));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          strip(PcbData.uucpName),
          strip(PcbData.uucpDomainName),
          strip(PcbData.uucpEmailHost),
          strip(PcbData.uucpNewsHost),
          strip(PcbData.uucpDefDist),
          strip(PcbData.uucpTimeZone),
          strip(PcbData.uucpSeparator),
          strip(PcbData.Organization));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%c\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          PcbData.uucpHighAscii,
          PcbData.uucpEmailConf,
          PcbData.uucpJunkConf,
          logical(PcbData.uucpBangDomain),
          logical(PcbData.uucpSubDomain));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          logical(PcbData.FidoProcessIn),
          logical(PcbData.FidoProcessOrphan),
          logical(PcbData.FidoProcessOut),
          logical(PcbData.FidoDialOut),
          PcbData.FidoPktFreq,
          PcbData.FidoExportFreq,
          PcbData.FidoMailFreq,
          PcbData.FidoDefaultZone,
          PcbData.FidoDefaultNet);
  dosfputs(Buffer,&pcbfile);

  undogetdays(Temp,&PcbData.PeakDays);
  sprintf(Buffer,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%d\r\n",
          Temp,
          stripfile(PcbData.HolidaysFile),
          stripfile(PcbData.uucpModFile),
          strip(PcbData.ModemInit2),
          strip(PcbData.ModemAns),
          logical(PcbData.IgnoreDropSecLevel));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          strip(PcbData.ModemDial),
          PcbData.NumRedials,
          PcbData.MaxTries,
          logical(PcbData.FidoImportAfterXfer),
          PcbData.FidoCrashSec,
          PcbData.FidoLogLevel,
          logical(PcbData.FidoCreateMsg),
          logical(PcbData.FidoEnableRouting));
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%ld\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          strippath(PcbData.FidoLoc),
          logical(PcbData.FidoSecure),
          logical(PcbData.FidoSysopChange),
          logical(PcbData.FidoCheckDupePath),
          logical(PcbData.FidoCheckDupeMsgId),
          PcbData.FidoNumMsgsToTrack,
          logical(PcbData.FidoMakeResponse),
          logical(PcbData.FidoEnablePassThru),
          logical(PcbData.FidoEnableAreaFix),
          logical(PcbData.FidoAutoAdd),
          logical(PcbData.FidoReAddress),
          logical(PcbData.FidoRouteEchoMail));
  dosfputs(Buffer,&pcbfile);


  sprintf(Buffer,"%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          PcbData.MinimizeLogOnOff,
          PcbData.MinimizeProtocols,
          PcbData.MinimizeDoors,
          PcbData.MinimizeFileTest,
          PcbData.MinimizeFileView,
          PcbData.MinimizeCompress,
          PcbData.MinimizeShells,
          PcbData.PriorityNormal,
          PcbData.PriorityProtocols,
          PcbData.PriorityCompress,
          PcbData.PriorityShells,
          PcbData.PriorityFidoIn,
          PcbData.PriorityFidoOut);
  dosfputs(Buffer,&pcbfile);

  sprintf(Buffer,"%s\r\n",
          PcbData.NetCopy);
  dosfputs(Buffer,&pcbfile);

  dosfclose(&pcbfile);

  #ifdef DEBUG
    mc_unregister(Buffer);
    mc_unregister(Temp);
  #endif
}


static void _NEAR_ LIBENTRY createmsgbase(char *MsgsFile, int NumBlocks, bool OldIndex) {
  int  File;
  int  X;
  char *p;
  char Index[66];
  char Block[4096];

  if ((File = doscreatecheck(MsgsFile,OPEN_WRIT,OPEN_NORMAL)) == -1)
    return;

  memset(Block,0,12);
  memset(&Block[12],' ',116);
  writecheck(File,Block,128);
  dosclose(File);

  strcpy(Index,MsgsFile);
  p = &Index[strlen(Index)];

  strcpy(p,".IDX");
  if ((File = doscreatecheck(Index,OPEN_WRIT,OPEN_NORMAL)) != -1) {
    memset(Block,0,64);
    writecheck(File,Block,64);
    dosclose(File);
  }

  strcpy(p,".NDX");
  if (OldIndex && (File = doscreatecheck(Index,OPEN_WRIT,OPEN_NORMAL)) != -1) {
    memset(Block,0,4096);
    for (X = 0; X < NumBlocks; X++)
      writecheck(File,Block,4096);
    dosclose(File);
  }
}


void LIBENTRY writeconffile(void) {
  bool        OldIndex;
  unsigned    X;
  char        *p;
  DOSFILE     pcbfile;
  char        Buffer[2048];

  #ifdef DEBUG
    mc_register(Buffer,sizeof(Buffer));
  #endif

  OldIndex = FALSE;
  if ((p = getenv("PCB")) != NULL)
    OldIndex = (bool) (strstr(p,"/OLDINDEX") != NULL);

  checkexistence(PcbData.CnfFile,CHECK_FILE,NULL,TRUE);
  if (dosfopen(PcbData.CnfFile,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&pcbfile) == -1)
    return;

  PerformOldValue = PerformValidation;
  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&WriteConf);
    strip(WriteConf.Name);
    PerformValidation = (bool) (PerformOldValue & (WriteConf.Name[0] != 0 ? TRUE : FALSE));
    sprintf(Buffer,"%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%s\r\n",
            WriteConf.Name,
            logical(WriteConf.AutoRejoin),
            logical(WriteConf.ViewMembers),
            logical(WriteConf.PrivUplds),
            logical(WriteConf.PrivMsgs),
            logical(WriteConf.EchoMail),
            WriteConf.AddSec,
            WriteConf.AddTime,
            WriteConf.MsgBlocks,
            stripfile(WriteConf.MsgFile));
    dosfputs(Buffer,&pcbfile);

    sprintf(Buffer,"%s\r\n%s\r\n%s\r\n",
            stripfile(WriteConf.UserMenu),
            stripfile(WriteConf.SysopMenu),
            stripfile(WriteConf.NewsFile));
    dosfputs(Buffer,&pcbfile);

    sprintf(Buffer,"%d\r\n%s\r\n%s\r\n%d\r\n%s\r\n%s\r\n",
            WriteConf.PubUpldSort,
            stripfile(WriteConf.UpldDir),
            backs(WriteConf.PubUpldLoc,sizeof(WriteConf.PubUpldLoc)),
            WriteConf.PrvUpldSort,
            stripfile(WriteConf.PrivDir),
            backs(WriteConf.PrvUpldLoc,sizeof(WriteConf.PrvUpldLoc)));
    dosfputs(Buffer,&pcbfile);

    sprintf(Buffer,"%d\r\n%s\r\n%s\r\n%d\r\n%s\r\n%s\r\n",
            logical(WriteConf.PublicConf),
            stripfile(WriteConf.DrsMenu),
            stripfile(WriteConf.DrsFile),
            WriteConf.ReqSecLevel,
            stripfile(WriteConf.BltMenu),
            stripfile(WriteConf.BltNameLoc));
    dosfputs(Buffer,&pcbfile);

    sprintf(Buffer,"\r\n%s\r\n%s\r\n\r\n%s\r\n%s\r\n\r\n%s\r\n",
/*          WriteConf.NumScripts, */
            stripfile(WriteConf.ScrMenu),
            stripfile(WriteConf.ScrNameLoc),
/*          WriteConf.NumTextDirs, */
            stripfile(WriteConf.DirMenu),
            stripfile(WriteConf.DirNameLoc),
/*          WriteConf.NumPaths, */
            stripfile(WriteConf.PthNameLoc));
    dosfputs(Buffer,&pcbfile);

    if (PerformValidation)
      stripfile(WriteConf.Intro);

    if (PerformValidation)
      backs(WriteConf.AttachLoc,sizeof(WriteConf.AttachLoc));

    if (PerformValidation)
      stripfile(WriteConf.CmdLst);

    if (PerformValidation && WriteConf.MsgFile[0] > 0 && fileexist(WriteConf.MsgFile) == 255)
      createmsgbase(WriteConf.MsgFile,WriteConf.MsgBlocks,WriteConf.OldIndex|OldIndex);
  }
  dosfclose(&pcbfile);

  #ifdef DEBUG
    mc_unregister(Buffer);
  #endif
}



int LIBENTRY writefile(exitsavetype SaveType) {
  char static YNQ[4] = { 3, 'Y','N','Q'};
  char Ext;
  int  X;
  char Temp[66];

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.X1 = 14;
  MsgData.X2 = 64;
  MsgData.Y2 = 22;

  if (VerChange) {
    MsgData.Y1 = 15;
    MsgData.Line1  = 17;
    MsgData.Msg1   = " NOTE: Saving files in a NEW format,  old files  ";
    MsgData.Color1 = Colors[DESC];
    MsgData.Line2  = 18;
    MsgData.Msg2   = " will be saved as:  PCBOARD.OLD  and  CNAMES.OLD ";
    MsgData.Color2 = Colors[DESC];
  } else {
    MsgData.Y1 = 18;
  }

  switch (SaveType) {
    case ASKTOCHECK      : MsgData.QuestLine = 20;
                           MsgData.Quest     = "Save configuration files (Y/N/Q=Quick Save)";
                           MsgData.Answer[0] = 'Y';
                           MsgData.Answer[1] = 0;
                           MsgData.Mask      = YNQ;
                           showmessage();
                           if (MsgData.Answer[0] == 'N' || KeyFlags == ESC)
                             return(0);
                           PerformValidation = (bool) ((MsgData.Answer[0] == 'Y' ? TRUE : FALSE));
                           break;
    case SAVEWITHOUTCHECK: PerformValidation = FALSE;
                           break;
    case SAVEWITHCHECK   : PerformValidation = TRUE;
                           break;
  }


  if (VerChange) {
    unlink(OldDatF);
    rename(DatFile,OldDatF);               /* save OLD pcboard.dat pcbfile */
    strcpy(Temp,PcbData.CnfFile);
    strcat(Temp,".old");
    unlink(Temp);
    rename(PcbData.CnfFile,Temp);             /* save OLD cnames pcbfile */
  }

  cls();
  gotoxy(0,0);
  ShowClock = NOCLOCK;

  if (PerformValidation) {
    if (! allocatelist(DIRNAME_LENGTH))
      PerformValidation = FALSE;
  }

  if (PerformValidation)
    printf("Checking directories while saving files... ");
  else
    printf("Saving files...\n");

  writedatafile();
  resetconffile();

  if (PerformValidation)  /* don't bother writing the CNAMES text file out */
    writeconffile();      /* if the sysop typed 'Q' for QUICK save         */

  PerformValidation = PerformOldValue;

  if (PerformValidation) { /* start all over on the dupe checking */
    deallocatelist();
    if (! allocatelist(DIRNAME_LENGTH))
      PerformValidation = FALSE;
  }

  if (asktochecklist("Bulletin")) {
    printf("\nChecking BLT.LST file(s) ... ");
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&WriteConf);
      if (WriteConf.Name[0] != 0 && ! foundinlist(WriteConf.BltNameLoc))
        validateblt(WriteConf.Name,WriteConf.BltNameLoc);
    }
  }

  if (asktochecklist("Directory")) {
    printf("\nChecking DIR.LST file(s) ... ");
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&WriteConf);
      if (WriteConf.Name[0] != 0 && ! foundinlist(WriteConf.DirNameLoc))
        validatedir(WriteConf.Name,WriteConf.DirNameLoc);
    }
  }

  if (asktochecklist("Script")) {
    printf("\nChecking SCRIPT.LST file(s) ... ");
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&WriteConf);
      if (WriteConf.Name[0] != 0 && ! foundinlist(WriteConf.ScrNameLoc))
        validatescr(WriteConf.Name,WriteConf.ScrNameLoc);
    }
  }

  if (asktochecklist("Download Path")) {
    printf("\nChecking DLPATH.LST file(s) ... ");
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&WriteConf);
      if (WriteConf.Name[0] != 0 && ! foundinlist(WriteConf.PthNameLoc))
        validatepth(WriteConf.Name,WriteConf.PthNameLoc);
    }
  }

  if (wherey() != 1) {
    gotoxy(0,24);
    printf("press any key to continue...");
    Ext = inkey(&Ext,NOCLOCK);
  }

  deallocatelist();
  /*
  if (PathListing != NULL) {
    free(PathListing);
    PathListing = NULL;
  }
  */
  return(1);
}
