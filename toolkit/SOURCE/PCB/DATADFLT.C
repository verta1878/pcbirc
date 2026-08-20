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
#else
  #include <dos.h>
  #include <direct.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <validate.h>
#include <dosfunc.h>
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define TRUE   1
#define FALSE  0

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


/*
char static defPCBGEN[]  = "C:\\PCB\\GEN\\";
char static defPCBHELP[] = "C:\\PCB\\HELP\\";
char static defPCBMAIN[] = "C:\\PCB\\MAIN\\";
*/

/*
   In the following variable declarations:

      2 bytes for the drive letter and colon                       ("C:")
     20 bytes for the path (magic number from the install utility) ("\\PCB")
      5 bytes for "\\GEN\\"                   (PCBGEN only)
      6 bytes for "\\HELP\\" or "\\MAIN\\"    (PCBMAIN and PCBHELP only)
      1 byte  for 0 terminator for the string
*/

static char PCBGEN  [2+20+5+1];
static char PCBHELP [2+20+6+1];
static char PCBMAIN [2+20+6+1];
static char PCBUUCP [2+20+6+1];

static void _NEAR_ LIBENTRY combineg(char *Dest,char *Srce) {
  strcpy(Dest,PCBGEN);
  strcat(Dest,Srce);
}


static void _NEAR_ LIBENTRY combinem(char *Dest,char *Srce) {
  strcpy(Dest,PCBMAIN);
  strcat(Dest,Srce);
}

static void _NEAR_ LIBENTRY combineu(char *Dest,char *Srce) {
  strcpy(Dest,PCBUUCP);
  strcat(Dest,Srce);
}


static void _NEAR_ LIBENTRY initializesysopsec(void) {
  int X;

  for (X = 0; X < sizeof(PcbData.SysopSec)/sizeof(short); X++)
    PcbData.SysopSec[X] = 110;

  PcbData.SysopSec[SEC_SYSOPLEVEL] = 100;
}

/* this buildstr macro is used instead of the buildstr function for this module */
#define buildstr(d,s1,s2,s3,s4) strcpy(d,s1); \
                                strcat(d,s2); \
                                strcat(d,s3)

void LIBENTRY pcbdefaults(char Drive, char * Path) {
  char FileName[66];
  pcbconftype Conf;

  buildstr(PCBGEN, "C:",Path,"\\GEN\\", NULL);
  buildstr(PCBHELP,"C:",Path,"\\HELP\\",NULL);
  buildstr(PCBMAIN,"C:",Path,"\\MAIN\\",NULL);
  buildstr(PCBUUCP,"C:",Path,"\\UUCP\\",NULL);

  memset(&PcbData,0,sizeof(PcbData));
  memset(&Conf,0,sizeof(pcbconftype));

/* lines that are commented-out are "implied" by the two lines above */

  strcpy(PcbData.Version,"*** PCBoard Version 14.5 & 15.0 data file ***");
  strcpy(PcbData.Sysop,"SYSOP");
/*PcbData.Password[0] = 0;*/
/*PcbData.UseRealName = FALSE;*/
  PcbData.Graphics    = TRUE;

  initializesysopsec();

  strcpy(PcbData.HlpLoc   ,PCBHELP);
  strcpy(PcbData.SecLoc   ,PCBGEN);
  strcpy(PcbData.ChtLoc   ,PCBMAIN);
  strcpy(PcbData.TxtLoc   ,PCBGEN);
  strcpy(PcbData.NdxLoc   ,PCBMAIN);
  strcpy(PcbData.TmpLoc   ,"WORK\\");
  combinem(PcbData.UsrFile,"USERS");
  combinem(PcbData.InfFile,"USERS.INF");
  combinem(PcbData.ClrFile,"CALLER");
  combinem(PcbData.CnfFile,"CNAMES");
  combinem(PcbData.PwdFile,"PWRD");
  combinem(PcbData.FscFile,"FSEC");
  combinem(PcbData.UscFile,"UPSEC");
  combinem(PcbData.TcnFile,"TCAN");
  combineg(PcbData.WlcFile,"WELCOME");
  combineg(PcbData.NewFile,"NEWUSER");
  combineg(PcbData.ClsFile,"CLOSED");
  combineg(PcbData.WrnFile,"WARNING");
  combineg(PcbData.ExpFile,"EXPIRED");

  combineg(PcbData.TrnFile  ,"PCBPROT.DAT");
  combinem(PcbData.NetFile  ,"USERNET.XXX");
  combineg(PcbData.CnfMenu  ,"CNFN");
  combinem(PcbData.RegFile  ,"NEWASK");
  combinem(PcbData.AnsFile  ,"NEWREG");
  combinem(PcbData.DldFile  ,"DOWNLOAD.TXT");
  combineg(PcbData.LogOffScr,"SCRIPT0");
  combinem(PcbData.LogOffAns,"ANSWER0");
  combineg(PcbData.MultiLang,"PCBML.DAT");
  strcpy(PcbData.ColorFile  ,"PCBFILER.DEF");

  PcbData.Seconds      = 30;
  strcpy(PcbData.ModemPort,"NONE");
  PcbData.ModemSpeed   = 2400;
/*PcbData.LockSpeed    = FALSE;*/
  strcpy(PcbData.ModemInit,"ATE0Q0V1X1S0=0H0");
  strcpy(PcbData.ModemOff ,"ATH1");
  strcpy(PcbData.ModemAns ,"ATA");
  strcpy(PcbData.ModemDial,"ATDT");

  PcbData.NumRedials   =  10;
  PcbData.MaxTries     =  10;

/*PcbData.DisableCTS   = FALSE;*/
/*PcbData.Hayes9600    = FALSE;*/
/*PcbData.FastComm     = FALSE;*/
/*PcbData.ResetModem   = FALSE;*/
/*PcbData.OffHook      = FALSE;*/
/*PcbData.Packet       = FALSE;*/
/*PcbData.AnswerRing   = FALSE;*/
/*PcbData.Allow7E1     = FALSE;*/
/*PcbData.AllowLowBaud = FALSE;*/
  strcpy(PcbData.AllowLowStrt,"00:01");
  strcpy(PcbData.AllowLowStop,"24:00");
  PcbData.LowBaudLimit  = 1200;
/*PcbData.ModemDelay    = 0;*/
  PcbData.RingsRequired = 1;
/*PcbData.IrqNum = 0; */
/*PcbData.BaseAddress = 0; */
  PcbData.LeaveDtrUp = TRUE;
/*PcbData.ShareIRQs = FALSE; */

/*PcbData.DisableScan     = FALSE;*/
/*PcbData.DisableCls      = FALSE;*/
/*PcbData.DisableEdits    = FALSE;*/
/*PcbData.DisableFilter   = FALSE;*/
/*PcbData.DisableQuick    = FALSE;*/
  PcbData.DisablePassword = TRUE;
/*PcbData.MultiLingual    = FALSE;*/
/*PcbData.AllowPwrdOnly   = FALSE;*/
/*PcbData.ClosedBoard     = FALSE;*/
/*PcbData.NonGraphics     = FALSE;*/
/*PcbData.ExitToDos       = FALSE;*/
  PcbData.IncludeCity     = TRUE;
/*PcbData.EliminateSnow   = FALSE;*/
/*PcbData.SubscriptMode   = FALSE;*/
/*PcbData.AllowEscCodes   = FALSE;*/
  PcbData.AllowCCs        = TRUE;
  PcbData.Validate        = TRUE;
/*PcbData.LastReadUpdate  = FALSE;*/
  PcbData.EnforceTime     = TRUE;
  PcbData.DisplayNews     = 'Y';
/*PcbData.DisableCTSdrop  = FALSE;*/
  PcbData.KbdTimeout      = 5;
  PcbData.MaxScrollBack   = 100;
/*PcbData.FastText        = FALSE;*/
/*PcbData.FastCnames      = FALSE;*/
  PcbData.UploadBy        = TRUE;
/*PcbData.TestUploads     = FALSE;*/
/*PcbData.ShowSettings    = FALSE;*/
  PcbData.AllowShell      = TRUE;
/*PcbData.Slaves          = FALSE;*/
  PcbData.SubscriptionDays= 365;
  PcbData.SubscriptWarning= 30;

/*PcbData.Network         = FALSE;*/
  PcbData.NodeNum         = 1;
  PcbData.NetTimeout      = 20;
  PcbData.ChatDelay       = 5;

  strcpy(PcbData.BoardName,"®® PCBoard Professional Bulletin Board ¯¯");
/*PcbData.CapFile[0] = 0 */
  strcpy(PcbData.ViewBatch,"PCBVIEW.BAT");
  strcpy(PcbData.ViewExt  ,"ZIP");

/*PcbData.NumConf            = 0;*/
  PcbData.NumAreas           = 1;
  PcbData.MaxMsgLines        = 99;
  PcbData.DefaultColor       = 37;
/*PcbData.DefaultIntensity   = 0;*/
/*PcbData.EventActive        = FALSE;*/
  strcpy(PcbData.EventTime,"02:00");
/*PcbData.EventSuspend       = 0;*/
/*PcbData.EventStopUplds     = FALSE;*/
/*PcbData.EventSlide         = FALSE;*/
  PcbData.UploadBufSize      = 8;
/*PcbData.DisableDriveCheck  = FALSE;*/
  PcbData.ParallelPortNum    = 1;
  PcbData.StopFreeSpace      = 500;
  PcbData.MaxTotalMsgs       = 600;
  PcbData.MaxConfMsgs        = 200;
/*PcbData.MinPriorToEvent    = 0;*/
  PcbData.QuickScan          = TRUE;
  PcbData.ScanAll            = TRUE;
/*PcbData.UseNewAskFile      = FALSE;*/
/*PcbData.AllowOneName       = FALSE;*/
/*PcbData.StopClockOnCap     = FALSE;*/

  strcpy(PcbData.SysopStart,"00:00");
  strcpy(PcbData.SysopStop ,"23:59");

  PcbData.LogCallerNumber  = FALSE;
  PcbData.LogConnectStr    = FALSE;
  PcbData.LogSecLevel      = FALSE;
/*PcbData.ConfPwrdAdjust   = FALSE;*/
/*PcbData.ConfirmCaller    = FALSE;*/
/*PcbData.AllowPwrdComment = FALSE;*/
/*PcbData.GuardLogoff      = FALSE;*/
  PcbData.NumDescLines     = 10;

  PcbData.DefExpiredLevel    = 10;
  memset(PcbData.UserLevels,10,sizeof(PcbData.UserLevels));
  PcbData.UserLevels[6]  = 0;
  PcbData.UserLevels[29] = 0;

  strcpy(Conf.Name,"Main Board");
  Conf.PublicConf  = TRUE;
  Conf.AutoRejoin  = TRUE;
/*Conf.ViewMembers = FALSE;*/
/*Conf.PrivUplds   = FALSE;*/
/*Conf.PrivMsgs    = FALSE;*/
/*Conf.EchoMail    = FALSE;*/
/*Conf.ReqSecLevel = 0;*/
/*Conf.AddSec      = 0;*/
/*Conf.AddTime     = 0;*/
  Conf.MsgBlocks   = 4;
  combinem(Conf.MsgFile   ,"MSGS");
  combineg(Conf.UserMenu  ,"BRDM");
  combineg(Conf.SysopMenu ,"BRDS");
  combineg(Conf.NewsFile  ,"NEWS");
  Conf.PubUpldSort = 2;
  combinem(Conf.UpldDir   ,"PUBLIC");

  buildstr(Conf.PubUpldLoc,"C:",Path,"\\UPLOAD\\",NULL);   /* Modified by SDR */

  Conf.PrvUpldSort = 2;
  combinem(Conf.PrivDir   ,"PRIVATE");

  buildstr(Conf.PrvUpldLoc,"C:",Path,"\\PRIVUPLD\\",NULL); /* Modified by SDR */

  combineg(Conf.DrsMenu   ,"DOORS");
  combineg(Conf.DrsFile   ,"DOORS.LST");
  combineg(Conf.BltMenu   ,"BLT");
  combineg(Conf.BltNameLoc,"BLT.LST");
  combineg(Conf.ScrMenu   ,"SCRIPT");
  combineg(Conf.ScrNameLoc,"SCRIPT.LST");
  combineg(Conf.DirMenu   ,"DIR");
  combineg(Conf.DirNameLoc,"DIR.LST");
  combineg(Conf.PthNameLoc,"DLPATH.LST");

  buildstr(Conf.AttachLoc, "C:",Path,"\\ATTACH\\",NULL);   /* Modified by SDR */

/*----------------------------------------------------------------------*/
/* the following are new or changed for v15.0                           */
/*----------------------------------------------------------------------*/

  combinem(PcbData.StatsFile,"PCBSTATS.DAT");
  combinem(PcbData.ChatFile,"CHAT.DAT");
  combinem(PcbData.EventDatFile,"EVENT.DAT");
  combineg(PcbData.GroupChat,"GROUP");
  strcpy(PcbData.SwapPath,".\\PCBSWAP.$$$");
  strcpy(PcbData.EventFiles,PCBMAIN);
/*PcbData.SlowDrives[0] = 0; */
  combineg(PcbData.ChatMenu,"CHTM");
  combineg(PcbData.NoAnsi,"NOANSI");
  combineg(PcbData.CmdLst,"CMD.LST");
/*PcbData.LogOnScr[0]        = 0;*/
/*PcbData.LogOnAns[0]        = 0;*/
/*PcbData.QwkFile[0]         = 0;*/
/*PcbData.AllFilesList[0]    = 0;*/
/*PcbData.SlowDriveBat[0]    = 0;*/
/*PcbData.FileTcan[0]        = 0;*/
/*PcbData.LowBaudSecOverride = 0;*/
/*PcbData.OldIndex           = FALSE;*/
  PcbData.Swap               = TRUE;
/*PcbData.NoBatchUp          = FALSE;*/
/*PcbData.ForceMain          = FALSE;*/
/*PcbData.Foreign            = FALSE;*/
/*PcbData.MonitorModem       = FALSE;*/
/*PcbData.No16550            = FALSE;*/
/*PcbData.Force16550A        = FALSE;*/
/*PcbData.AutoReset          = FALSE;*/
/*PcbData.Float              = FALSE;*/
/*PcbData.Encrypt            = FALSE;*/
/*PcbData.EnableFido         = FALSE;*/
/*PcbData.SwapDuringBat      = FALSE;*/
/*PcbData.UserSysDuringBat   = FALSE;*/
/*PcbData.DefaultGraphics    = FALSE;*/
/*PcbData.AllowAliasChange   = FALSE;*/
/*PcbData.ExcludeLocals      = FALSE;*/
/*PcbData.PwrdUpdate         = 0;*/
/*PcbData.PwrdWarn           = 0;*/
/*PcbData.ByteCredit         = 0;*/
/*PcbData.RecordGroupChat    = FALSE;*/
/*PcbData.AllowHandles       = FALSE;*/
/*PcbData.ShowAlias          = FALSE;*/
  PcbData.MinPwrdLen         = 6;
  PcbData.PromptToReadMail   = TRUE;
  PcbData.PromoteBatch       = TRUE;
  PcbData.AutoRegConf        = TRUE;
  PcbData.ScanBlts           = TRUE;
  PcbData.AutoMakeMsgs       = TRUE;
  PcbData.VerifyCDLoss       = TRUE;
  PcbData.UploadCredit       = 10;
  PcbData.EnvSize            = 768;
  combinem(PcbData.FidoConfig,"PCBFIDO.CFG");
  combinem(PcbData.FidoQueue ,"FIDOQUE.DAT");

  PcbData.NewDefaultColor = 0x07;
  PcbData.MsgHdr1Color    = 0x1F;
  PcbData.MsgHdr2Color    = 0x3F;
  PcbData.MsgHdr3Color    = 0x3F;
  PcbData.MsgHdr4Color    = 0x3F;
  PcbData.MsgHdr5Color    = 0x3E;
  PcbData.MsgHdr6Color    = 0x3E;

/*----------------------------------------------------------------------*/
/* the following are new or changed for v15.1                           */
/*----------------------------------------------------------------------*/

  buildstr(PcbData.CmdLoc,"C:",Path,"\\CMDFILES\\", NULL);
/*PcbData.ForceIntro         = FALSE;*/
/*PcbData.SkipProtocol       = FALSE;*/
/*PcbData.SkipAlias          = FALSE;*/
/*PcbData.NoCarrierExit      = FALSE;*/
/*PcbData.OS2Driver          = FALSE;*/

/*----------------------------------------------------------------------*/
/* the following are new or changed for v15.2                           */
/*----------------------------------------------------------------------*/

/*PcbData.EnableAccounting         = FALSE;*/
/*PcbData.ShowCurrency             = FALSE;*/
/*PcbData.Concurrent               = FALSE;*/
/*PcbData.PeakDays                 = 0;*/

  strcpy(PcbData.PeakStart,"00:00");
  strcpy(PcbData.PeakEnd  ,"00:00");

  combinem(PcbData.HolidaysFile ,"HOLIDAYS.CFG");
  combinem(PcbData.AccountConfig,"ACCOUNT.CFG");
  combineg(PcbData.AccountInfo  ,"ACTINFO");
  combineg(PcbData.AccountWarn  ,"ACTWARN");
  combinem(PcbData.AccountTrack ,"TRACKING.TXT");
  combineg(PcbData.AccountLogoff,"ACTBYE");

  combineu(PcbData.uucpPath     ,"");
  combineu(PcbData.uucpSpoolPath,"SPOOL\\");
  combineu(PcbData.uucpLogPath  ,"LOGS\\");
  combineu(PcbData.CompBatFile  ,"COMP.BAT");
  combineu(PcbData.DeCompBatFile,"DECOMP.BAT");
  combineu(PcbData.uucpModFile  ,"USENET.MOD");

  /* PcbData.uucpEmailConf     = 0;      */
  /* PcbData.uucpJunkConf      = 0;      */
  /* PcbData.uucpName[0]       = 0;      */
  /* PcbData.uucpDomainName[0] = 0;      */
  /* PcbData.uucpBangDomain    = FALSE;  */
  /* PcbData.uucpSubDomain     = FALSE;  */
  /* PcbData.uucpEmailHost[0]  = 0;      */
  /* PcbData.uucpNewsHost[0]   = 0;      */
  /* PcbData.Organization[0]   = 0;      */

  strcpy(PcbData.uucpDefDist,"world");
  strcpy(PcbData.uucpTimeZone,"-0700");
  strcpy(PcbData.uucpSeparator,".");
  PcbData.uucpHighAscii = 'C';

/*----------------------------------------------------------------------*/
/* the following are new or changed for v15.21                          */
/*----------------------------------------------------------------------*/

  /* PcbData.FidoProcessIn       = FALSE;*/
  /* PcbData.FidoProcessOrphan   = FALSE;*/
  /* PcbData.FidoProcessOut      = FALSE;*/
  /* PcbData.FidoDialOut         = FALSE;*/
  /* PcbData.FidoImportAfterXfer = FALSE;*/
  /* PcbData.FidoDefaultZone     = 0;    */
  /* PcbData.FidoDefaultNet      = 0;    */
     PcbData.FidoPktFreq         = 1;
     PcbData.FidoExportFreq      = 1;
     PcbData.FidoMailFreq        = 1;
     PcbData.FidoCrashSec        = 100;
  /* PcbData.FidoLogLevel        = 0; */
  /* PcbData.FidoCreateMsg       = FALSE; */
  /* PcbData.FidoEnableRouting   = FALSE; */

/*PcbData.IgnoreDropSecLevel       = FALSE;*/

/*----------------------------------------------------------------------*/
/* the following are new or changed for v15.22                          */
/*----------------------------------------------------------------------*/

  strcpy(PcbData.FidoLoc,PCBMAIN);
  /* PcbData.FidoSecure         = FALSE; */
  /* PcbData.FidoSysopChange    = FALSE; */
  /* PcbData.FidoCheckDupePath  = FALSE; */
  /* PcbData.FidoCheckDupeMsgId = FALSE; */
  /* PcbData.FidoNumMsgsToTrack = FALSE; */
     PcbData.FidoMakeResponse   = TRUE;
  /* PcbData.FidoEnablePassThru = FALSE; */
  /* PcbData.FidoEnableAreaFix  = FALSE; */
  /* PcbData.FidoAutoAdd        = FALSE; */
  /* PcbData.FidoReAddress      = FALSE; */
  /* PcbData.FidoRouteEchoMail  = FALSE; */

  /* PcbData.MinimizeLogOnOff   = 0; */
  /* PcbData.MinimizeProtocols  = 0; */
  /* PcbData.MinimizeDoors      = 0; */
  /* PcbData.MinimizeFileTest   = 0; */
  /* PcbData.MinimizeFileView   = 0; */
  /* PcbData.MinimizeCompress   = 0; */
  /* PcbData.MinimizeShells     = 0; */
  /* PcbData.PriorityNormal     = 0; */
  /* PcbData.PriorityProtocols  = 0; */
  /* PcbData.PriorityCompress   = 0; */
  /* PcbData.PriorityShells     = 0; */
  /* PcbData.PriorityFidoIn     = 0; */
  /* PcbData.PriorityFidoOut    = 0; */
  /* PcbData.NetCopy[0]         = 0; */

/*----------------------------------------------------------------------*/
/* customize drive settings                                             */
/*----------------------------------------------------------------------*/

  PcbData.HlpLoc[0]         =
  PcbData.SecLoc[0]         =
  PcbData.TxtLoc[0]         =
  PcbData.NdxLoc[0]         =
  PcbData.ChtLoc[0]         =
  PcbData.FidoConfig[0]     =
  PcbData.FidoQueue[0]      =
  PcbData.EventDatFile[0]   =
  PcbData.EventFiles[0]     =
  PcbData.ChatFile[0]       =
  PcbData.ChatMenu[0]       =
  PcbData.NoAnsi[0]         =
  PcbData.CmdLst[0]         =
  PcbData.StatsFile[0]      =
  PcbData.UsrFile[0]        =
  PcbData.InfFile[0]        =
  PcbData.ClrFile[0]        =
  PcbData.CnfFile[0]        =
  PcbData.PwdFile[0]        =
  PcbData.FscFile[0]        =
  PcbData.UscFile[0]        =
  PcbData.TcnFile[0]        =
  PcbData.WlcFile[0]        =
  PcbData.NewFile[0]        =
  PcbData.ClsFile[0]        =
  PcbData.WrnFile[0]        =
  PcbData.ExpFile[0]        =
  PcbData.TrnFile[0]        =
  PcbData.NetFile[0]        =
  PcbData.CnfMenu[0]        =
  PcbData.RegFile[0]        =
  PcbData.AnsFile[0]        =
  PcbData.DldFile[0]        =
  PcbData.LogOffScr[0]      =
  PcbData.LogOffAns[0]      =
  PcbData.MultiLang[0]      =
  PcbData.GroupChat[0]      =
  PcbData.HolidaysFile[0]   =
  PcbData.AccountConfig[0]  =
  PcbData.AccountInfo[0]    =
  PcbData.AccountWarn[0]    =
  PcbData.AccountTrack[0]   =
  PcbData.AccountLogoff[0]  =

  PcbData.CmdLoc[0]         =

  PcbData.uucpPath[0]       =
  PcbData.uucpSpoolPath[0]  =
  PcbData.uucpLogPath[0]    =
  PcbData.CompBatFile[0]    =
  PcbData.DeCompBatFile[0]  =
  PcbData.uucpModFile[0]    =
  PcbData.FidoLoc[0]        =

  Conf.MsgFile[0]     =
  Conf.PubUpldLoc[0]  =
  Conf.PrvUpldLoc[0]  =
  Conf.UpldDir[0]     =
  Conf.PrivDir[0]     =
  Conf.UserMenu[0]    =
  Conf.SysopMenu[0]   =
  Conf.NewsFile[0]    =
  Conf.DrsMenu[0]     =
  Conf.DrsFile[0]     =
  Conf.BltMenu[0]     =
  Conf.BltNameLoc[0]  =
  Conf.ScrMenu[0]     =
  Conf.ScrNameLoc[0]  =
  Conf.DirMenu[0]     =
  Conf.DirNameLoc[0]  =
  Conf.PthNameLoc[0]  =
  Conf.AttachLoc[0]   =  Drive;

  strcpy(FileName,PcbData.CnfFile);
  strcat(FileName,".@@@");
  checkexistence(FileName,CHECK_FILE,NULL,TRUE);

  if (fileexist(FileName) != 255) {
    PcbData.NumAreas = (short) ((DTA.ff_fsize - sizeof(short)) / sizeof(oldconftype));
    PcbData.NumConf  = (short) (PcbData.NumAreas - 1);
    loadcnames(TRUE);
  } else {
    loadcnames(TRUE);
    putconfrecord(0,&Conf);
  }
}


void LIBENTRY askshare(char *Desc, char *ConfLoc, char *Menu, char *List, char *MainMenu, char *MainList, char *MenuName, char *ListName) {
  int  Choice;
  char Temp[80];

  boxcls(1,4,78,23,Colors[OUTBOX],SINGLE);
  sprintf(Temp,"%s File Definitions",Desc);
  fastcenter(06,Temp,Colors[HEADING]);

  fastprint(3, 8,"These definitions are very flexible and can be configured any way you like.",Colors[DISPLAY]);
  fastprint(3, 9,"Below are three `default' choices that you may use to get your conference"  ,Colors[DISPLAY]);
  fastprint(3,10,"set up fast and may be modified manually later if you wish."                ,Colors[DISPLAY]);
  fastprint(3,12,"1) Use the default conference definitions directly.  If you make any"       ,Colors[DISPLAY]);
  fastprint(3,13,"   changes it will affect all conferences sharing the same definitions."    ,Colors[DISPLAY]);
  fastprint(3,15,"2) Copy the default conference definitions into a conference-specific file" ,Colors[DISPLAY]);
  fastprint(3,16,"   which may then be modified without affecting any other conference."      ,Colors[DISPLAY]);
  fastprint(3,18,"3) Start out with an empty conference-specific file which may be modified"  ,Colors[DISPLAY]);
  fastprint(3,19,"   without affecting any other conference."                                 ,Colors[DISPLAY]);

  Choice = 1;
  sprintf(Temp,"Enter (1, 2 or 3) for the %s file definitions",Desc);
  inputnum(4,21,1,Temp,&Choice,vINT,0);
  if (KeyFlags != ESC) {
    switch (Choice) {
      case 1: if (MainMenu != NULL)
                strcpy(Menu,MainMenu);
              strcpy(List,MainList);
              break;
      case 2: /* continue */
      case 3: if (MenuName != NULL) {
                strcpy(Menu,ConfLoc);  strcat(Menu,MenuName);
              }
              strcpy(List,ConfLoc);  strcat(List,ListName);
              if (Choice == 2) {
                if (MainMenu != NULL && MainMenu[0] != 0) {
                  checkexistence(Menu,VALIDATE_FILE,NULL,FALSE);
                  checkexistence(MainMenu,VALIDATE_FILE,NULL,FALSE);
                  copyfile(MainMenu,Menu,FALSE);
                }
                if (MainList[0] != 0) {
                  checkexistence(List,VALIDATE_FILE,NULL,FALSE);
                  checkexistence(MainList,VALIDATE_FILE,NULL,FALSE);
                  copyfile(MainList,List,FALSE);
                }
              }
              break;
    }
  }
}


void LIBENTRY askshareupspec(char *ConfLoc, pcbconftype *Main, pcbconftype *New) {
  int  Choice;

  boxcls(1,4,78,18,Colors[OUTBOX],SINGLE);
  fastcenter(06,"Upload Definitions",Colors[HEADING]);

  fastprint(3, 8,"These definitions are very flexible and can be configured any way you like.",Colors[DISPLAY]);
  fastprint(3, 9,"Below are three `default' choices that you may use to get your conference"  ,Colors[DISPLAY]);
  fastprint(3,10,"set up fast and may be modified manually later if you wish."                ,Colors[DISPLAY]);
  fastprint(3,12,"1) Copy the default upload paths and DIR file specifications."              ,Colors[DISPLAY]);
  fastprint(3,13,"2) Create new upload paths and DIR files for this conference."              ,Colors[DISPLAY]);
  fastprint(3,14,"3) Leave the upload path and DIR file specifications blank."                ,Colors[DISPLAY]);

  Choice = 1;
  inputnum(4,16,1,"Enter (1, 2 or 3) for the upload definitions",&Choice,vINT,0);
  if (KeyFlags != ESC) {
    switch (Choice) {
      case 1: strcpy(New->UpldDir   ,Main->UpldDir);
              strcpy(New->PubUpldLoc,Main->PubUpldLoc);
              strcpy(New->PrivDir   ,Main->PrivDir);
              strcpy(New->PrvUpldLoc,Main->PrvUpldLoc);
              break;
      case 2: strcpy(New->UpldDir   ,ConfLoc);  strcat(New->UpldDir   ,"UPLD");
              strcpy(New->PubUpldLoc,ConfLoc);  strcat(New->PubUpldLoc,"UPLOAD\\");
              strcpy(New->PrivDir   ,ConfLoc);  strcat(New->PrivDir   ,"PRIVATE");
              strcpy(New->PrvUpldLoc,ConfLoc);  strcat(New->PrvUpldLoc,"PRIVUPLD\\");
              break;
      case 3: break;
    }
  }
}


static void _NEAR_ LIBENTRY removeinvalidchars(char *Str) {
  stripall(Str,' ');      /* get rid of any spaces (not valid in names)  */
  stripall(Str,'+');      /* get rid of any +'s (not valid in filenames) */
  stripall(Str,'.');      /* get rid of any .'s (multiple .'s not valid) */

  Str[8] = 0;             /* limit the SUBDIRECTORY NAME to 8 characters */
  strupr(Str);
}


extern char ALLCONF[];  /* found in PCBSetup's INIT.C */
extern char ALLFILE[];  /* found in PCBSetup's INIT.C */

#define COL_LEFT    2
#define COL_RIGHT   77
#define ROW_TOP     2
#define ROW_BOTTOM  23
#define TXT_LEFT    COL_LEFT + 2
#define TXT_LINE    ROW_TOP  + 2

void LIBENTRY confdefaults(unsigned CurConfNum, pcbconftype *q) {
  bool          Okay;
  unsigned      CloneNum;
  int           DefaultNum;
  char         *p;
  char          Clone[6];
  char          ConfLoc[21];
  char          OldConfLoc[21];
  char          ConfName[80];
  char          Temp[80];
  char          Old[80];
  pcbconftype   Conf;
  savescrntype  ScrnBuf;

  savescreen(&ScrnBuf);

  boxcls(COL_LEFT,ROW_TOP,COL_RIGHT,ROW_BOTTOM,Colors[OUTBOX],SINGLE);
  fastcenter(TXT_LINE,"Setting up a NEW conference",Colors[HEADING]);
  Okay = TRUE;
  inputnum(TXT_LEFT,TXT_LINE+2,1,"Do you want PCBSetup to provide `defaults' for you",&Okay,vBOOL,0);
  if (Okay && KeyFlags != ESC) {
    ConfName[0] = 0;
    inputstr(TXT_LEFT,TXT_LINE+4,sizeof(Conf.Name)-1,"Conf. Name",ConfName,ConfName,ALLCONF,INPUT_DEFAULT,0);
    if (KeyFlags == ESC)
      goto end;

    stripright(ConfName,' ');
    if (ConfName[0] == 0)
      goto end;

    strcpy(Temp,ConfName);
    removeinvalidchars(Temp);

    fastprint(TXT_LEFT,TXT_LINE+6,"Cloning an existing conference can save a lot of time if the new",Colors[DISPLAY]);
    fastprint(TXT_LEFT,TXT_LINE+7,"conference will be very similar to an existing conference."      ,Colors[DISPLAY]);

retry:
    Clone[0] = 0;
    inputstr(TXT_LEFT,TXT_LINE+9,5,"Number of conference to clone (leave blank to skip)",Clone,Clone,ALLNUM,INPUT_DEFAULT,0);
    if (KeyFlags == ESC)
      goto end;

    stripall(Clone,' ');
    if (Clone[0] != 0) {
      CloneNum = atoi(Clone);
      if (CloneNum > PcbData.NumConf || CloneNum == CurConfNum) {
        beep();
        goto retry;
      }

      getconfrecord(CloneNum,&Conf);
      stripright(Conf.MsgFile,' ');
      if (Conf.MsgFile[0] == 0) {
        beep();
        memset(&MsgData,0,sizeof(MsgData));
        MsgData.AutoBox   = TRUE;
        MsgData.Save      = TRUE;
        MsgData.Msg1      = "The conference you selected is not properly set up.";
        MsgData.Line1     = 17;
        MsgData.Color1    = Colors[HEADING];
        MsgData.Msg2      = "You'll need to select a different conference.";
        MsgData.Line2     = 18;
        MsgData.Color2    = Colors[HEADING];
        showmessage();
        goto retry;
      }

      strcpy(Old,Conf.Name);
      removeinvalidchars(Old);

      strcpy(Conf.Name,ConfName);

      maxstrcpy(OldConfLoc,Conf.MsgFile,sizeof(OldConfLoc));
      if ((p = strrchr(OldConfLoc,'\\')) != NULL)
        *(p+1) = 0;

      strcpy(ConfLoc,OldConfLoc);
      substitute(ConfLoc,Old,Temp,sizeof(ConfLoc));
      inputstr(TXT_LEFT,TXT_LINE+11,sizeof(ConfLoc)-1,"Default Conference Location",ConfLoc,ConfLoc,ALLFILE,INPUT_CAPS,0);
      if (KeyFlags == ESC)
        goto end;
      stripright(ConfLoc,' ');
      addbackslash(ConfLoc,sizeof(ConfLoc));

      substitute(Conf.MsgFile   ,OldConfLoc,ConfLoc,sizeof(Conf.MsgFile   ));
      substitute(Conf.UserMenu  ,OldConfLoc,ConfLoc,sizeof(Conf.UserMenu  ));
      substitute(Conf.SysopMenu ,OldConfLoc,ConfLoc,sizeof(Conf.SysopMenu ));
      substitute(Conf.NewsFile  ,OldConfLoc,ConfLoc,sizeof(Conf.NewsFile  ));
      substitute(Conf.UpldDir   ,OldConfLoc,ConfLoc,sizeof(Conf.UpldDir   ));
      substitute(Conf.PubUpldLoc,OldConfLoc,ConfLoc,sizeof(Conf.PubUpldLoc));
      substitute(Conf.PrivDir   ,OldConfLoc,ConfLoc,sizeof(Conf.PrivDir   ));
      substitute(Conf.PrvUpldLoc,OldConfLoc,ConfLoc,sizeof(Conf.PrvUpldLoc));
      substitute(Conf.DrsMenu   ,OldConfLoc,ConfLoc,sizeof(Conf.DrsMenu   ));
      substitute(Conf.DrsFile   ,OldConfLoc,ConfLoc,sizeof(Conf.DrsFile   ));
      substitute(Conf.BltMenu   ,OldConfLoc,ConfLoc,sizeof(Conf.BltMenu   ));
      substitute(Conf.BltNameLoc,OldConfLoc,ConfLoc,sizeof(Conf.BltNameLoc));
      substitute(Conf.ScrMenu   ,OldConfLoc,ConfLoc,sizeof(Conf.ScrMenu   ));
      substitute(Conf.ScrNameLoc,OldConfLoc,ConfLoc,sizeof(Conf.ScrNameLoc));
      substitute(Conf.DirMenu   ,OldConfLoc,ConfLoc,sizeof(Conf.DirMenu   ));
      substitute(Conf.DirNameLoc,OldConfLoc,ConfLoc,sizeof(Conf.DirNameLoc));
      substitute(Conf.PthNameLoc,OldConfLoc,ConfLoc,sizeof(Conf.PthNameLoc));
      substitute(Conf.Intro     ,OldConfLoc,ConfLoc,sizeof(Conf.Intro     ));
      substitute(Conf.AttachLoc ,OldConfLoc,ConfLoc,sizeof(Conf.AttachLoc ));
      *q = Conf;
    } else {
      fastprint(TXT_LEFT,TXT_LINE+11,"If the new conference will have many similarities to an existing",Colors[DISPLAY]);
      fastprint(TXT_LEFT,TXT_LINE+12,"conference, without being a clone, you can still save time by"   ,Colors[DISPLAY]);
      fastprint(TXT_LEFT,TXT_LINE+13,"letting PCBSetup provide defaults for you."                      ,Colors[DISPLAY]);

retry2:
      DefaultNum = 0;
      inputnum(TXT_LEFT,TXT_LINE+15,5,"Enter the Conference # on which to base the defaults",&DefaultNum,vINT,0);
      if (DefaultNum > PcbData.NumConf) {
        beep();
        goto retry2;
      }

      getconfrecord(DefaultNum,&Conf);

      maxstrcpy(OldConfLoc,Conf.MsgFile,sizeof(OldConfLoc));
      if ((p = strrchr(OldConfLoc,'\\')) != NULL)
        *(p+1) = 0;

      strcpy(ConfLoc,OldConfLoc);
      inputstr(TXT_LEFT,TXT_LINE+17,sizeof(ConfLoc)-1,"Default Conference Location",ConfLoc,ConfLoc,ALLFILE,INPUT_CAPS,0);
      if (KeyFlags == ESC)
        goto end;
      stripright(ConfLoc,' ');
      addbackslash(ConfLoc,sizeof(ConfLoc));
    }

    restorescreen(&ScrnBuf);

    if (Clone[0] == 0) {
      memset(q,0,sizeof(pcbconftype));
  /*  lines that are commented-out are "implied" by the line above */

      strcpy(q->Name,ConfName);
  /*  q->PublicConf  = FALSE;*/
  /*  q->AutoRejoin  = FALSE;*/
  /*  q->ViewMembers = FALSE;*/
  /*  q->PrivUplds   = FALSE;*/
  /*  q->PrivMsgs    = FALSE;*/
  /*  q->EchoMail    = FALSE;*/
  /*  q->ReqSecLevel = 0; */
  /*  q->AddSec      = 0;*/
  /*  q->AddTime     = 0;*/
      q->MsgBlocks   = 4;
      strcpy(q->MsgFile  ,ConfLoc);  strcat(q->MsgFile,"MSGS");
      strcpy(q->UserMenu ,Conf.UserMenu);
      strcpy(q->SysopMenu,Conf.SysopMenu);
      strcpy(q->NewsFile ,ConfLoc);  strcat(q->NewsFile,"NEWS");

      q->PubUpldSort = 2;
      q->PrvUpldSort = 2;

      askshareupspec(ConfLoc,&Conf,q);

      askshare("DOORS"         ,ConfLoc,q->DrsMenu,q->DrsFile   ,Conf.DrsMenu,Conf.DrsFile   ,"DOORS" ,"DOORS.LST");
      restorescreen(&ScrnBuf);
      askshare("BULLETINS"     ,ConfLoc,q->BltMenu,q->BltNameLoc,Conf.BltMenu,Conf.BltNameLoc,"BLT"   ,"BLT.LST");
      restorescreen(&ScrnBuf);
      askshare("SCRIPTS"       ,ConfLoc,q->ScrMenu,q->ScrNameLoc,Conf.ScrMenu,Conf.ScrNameLoc,"SCRIPT","SCRIPT.LST");
      restorescreen(&ScrnBuf);
      askshare("DIR Listing"   ,ConfLoc,q->DirMenu,q->DirNameLoc,Conf.DirMenu,Conf.DirNameLoc,"DIR"   ,"DIR.LST");
      restorescreen(&ScrnBuf);
      askshare("Download Paths",ConfLoc,NULL      ,q->PthNameLoc,NULL              ,Conf.PthNameLoc,NULL    ,"DLPATH.LST");

      /*------------------------------------------------------------------*/
      /* the following are new for v15.0                                  */
      /*------------------------------------------------------------------*/
  /*  q->ForceEcho        = FALSE; */
  /*  q->ReadOnly         = FALSE; */
  /*  q->NoPrivateMsgs    = FALSE; */
  /*  q->RetReceiptLevel  = 0;     */
  /*  q->RecordOrigin     = FALSE; */
  /*  q->PromptForRouting = FALSE; */
  /*  q->AllowAliases     = FALSE; */
  /*  q->ShowIntroOnRA    = FALSE; */
  /*  q->AttachLevel      = 0;     */
      q->CarbonLimit      = 5;
  /*  q->ReqLevelToEnter  = 0;     */
  /*  q->Password[0]      = 0;     */
  /*  q->Intro[0]         = 0;     */

      if ((p = strrchr(q->MsgFile,'\\')) != NULL)
        memcpy(q->AttachLoc,q->MsgFile,(int) (p - q->MsgFile) + 1);
    }
  }

end:
  restorescreen(&ScrnBuf);
}
