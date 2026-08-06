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
  #include <direct.h>
#endif

#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"
#include "scrnio.h"
#include "scrnio.ext"
#include "misc.h"
#include "dosfunc.h"
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define BUFLEN   2048

static DOSFILE pcbfile;

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

static void _NEAR_ LIBENTRY getdays2(char *Days, char *Str) {
  int X;

  for (X = 0, *Days = 0; *Str != 0 && X < 7; X++, Str++) {
    if (*Str == 'Y')
      *Days += (char) (1 << X);
  }
}


static int _NEAR_ LIBENTRY fgetstr2(char *Str, int Len) {
  char T[BUFLEN+2];

  if (dosfgets(T,BUFLEN,&pcbfile) != -1) { /*allow lines up to BUFLEN characters long*/
    stripright(T,' ');
    maxstrcpy(Str,T,Len);                  /*but copy only the maximum of Len bytes  */
    return(0);
  } else {
    Str[0] = 0;                            /*if there was an error, return null string*/
    return(-1);
  }
}


static short _NEAR_ LIBENTRY fgethex2(void) {
  char Str[20];
  fgetstr2(Str,10);
  return((short) hextoint(Str));
}


static short _NEAR_ LIBENTRY fgetint2(void) {
  char Str[20];
  fgetstr2(Str,10);
  return((short) atoi(Str));
}


static long _NEAR_ LIBENTRY fgetlong2(void) {
  char Str[20];
  fgetstr2(Str,10);
  return(atol(Str));
}


static bool _NEAR_ LIBENTRY fgetlog2(void) {
  char Str[20];
  fgetstr2(Str,10);
  return(atoi(Str) == -1 ? (bool) TRUE : (bool) FALSE);
}


static void _NEAR_ LIBENTRY read150file2(void) {
  unsigned    x;
  int         Num;
  char        Temp[100];

  fgetstr2(PcbData.Sysop   ,sizeof(PcbData.Sysop));
  fgetstr2(PcbData.Password,sizeof(PcbData.Password));
  PcbData.UseRealName = fgetlog2();
  PcbData.Graphics    = fgetlog2();

  for (x = SEC_READCOMMENTS; x <= SEC_14; x++)
    PcbData.SysopSec[x] = fgetint2();

  fgetstr2(PcbData.HlpLoc ,sizeof(PcbData.HlpLoc));
  fgetstr2(PcbData.SecLoc ,sizeof(PcbData.SecLoc));
  fgetstr2(PcbData.ChtLoc ,sizeof(PcbData.ChtLoc));
  fgetstr2(PcbData.TxtLoc ,sizeof(PcbData.TxtLoc));
  fgetstr2(PcbData.NdxLoc ,sizeof(PcbData.NdxLoc));
  fgetstr2(PcbData.UsrFile,sizeof(PcbData.UsrFile));
  fgetstr2(PcbData.ClrFile,sizeof(PcbData.ClrFile));
  fgetstr2(PcbData.CnfFile,sizeof(PcbData.CnfFile));
  fgetstr2(PcbData.PwdFile,sizeof(PcbData.PwdFile));
  fgetstr2(PcbData.FscFile,sizeof(PcbData.FscFile));
  fgetstr2(PcbData.UscFile,sizeof(PcbData.UscFile));
  fgetstr2(PcbData.TcnFile,sizeof(PcbData.TcnFile));
  fgetstr2(PcbData.WlcFile,sizeof(PcbData.WlcFile));
  fgetstr2(PcbData.NewFile,sizeof(PcbData.NewFile));
  fgetstr2(PcbData.ClsFile,sizeof(PcbData.ClsFile));
  fgetstr2(PcbData.WrnFile,sizeof(PcbData.WrnFile));
  fgetstr2(PcbData.ExpFile,sizeof(PcbData.ExpFile));

  fgetstr2(PcbData.NetFile  ,sizeof(PcbData.NetFile  ));
  fgetstr2(PcbData.CnfMenu  ,sizeof(PcbData.CnfMenu  ));
  fgetstr2(PcbData.RegFile  ,sizeof(PcbData.RegFile  ));
  fgetstr2(PcbData.AnsFile  ,sizeof(PcbData.AnsFile  ));
  fgetstr2(PcbData.TrnFile  ,sizeof(PcbData.TrnFile  ));
  fgetstr2(PcbData.DldFile  ,sizeof(PcbData.DldFile  ));
  fgetstr2(PcbData.LogOffScr,sizeof(PcbData.LogOffScr));
  fgetstr2(PcbData.LogOffAns,sizeof(PcbData.LogOffAns));
  fgetstr2(PcbData.MultiLang,sizeof(PcbData.MultiLang));
  fgetstr2(PcbData.GroupChat,sizeof(PcbData.GroupChat));

  PcbData.Seconds              = fgetint2();
  fgetstr2(PcbData.ModemPort,sizeof(PcbData.ModemPort));
  PcbData.ModemSpeed           = fgetlong2();
  PcbData.LockSpeed            = fgetlog2();
  fgetstr2(PcbData.ModemInit,sizeof(PcbData.ModemInit));
  fgetstr2(PcbData.ModemOff ,sizeof(PcbData.ModemOff ));
  PcbData.DisableCTS           = fgetlog2();
  PcbData.Hayes9600            = fgetlog2();
  PcbData.FastComm             = fgetlog2();
  PcbData.ResetModem           = fgetlog2();
  PcbData.OffHook              = fgetlog2();
  PcbData.Packet               = fgetlog2();
  PcbData.AnswerRing           = fgetlog2();
  PcbData.Allow7E1             = fgetlog2();
  PcbData.AllowLowBaud         = fgetlog2();
  fgetstr2(PcbData.AllowLowStrt,sizeof(PcbData.AllowLowStrt));
  fgetstr2(PcbData.AllowLowStop,sizeof(PcbData.AllowLowStop));

  PcbData.DisableScan          = fgetlog2();
  PcbData.DisableCls           = fgetlog2();
  PcbData.DisableEdits         = fgetlog2();
  PcbData.DisableFilter        = fgetlog2();
  PcbData.DisableQuick         = fgetlog2();
  PcbData.DisablePassword      = fgetlog2();
  PcbData.MultiLingual         = fgetlog2();
  PcbData.AllowPwrdOnly        = fgetlog2();
  PcbData.ClosedBoard          = fgetlog2();
  PcbData.NonGraphics          = fgetlog2();
  PcbData.ExitToDos            = fgetlog2();
  PcbData.IncludeCity          = fgetlog2();
  PcbData.EliminateSnow        = fgetlog2();
  PcbData.SubscriptMode        = fgetlog2();
  PcbData.AllowEscCodes        = fgetlog2();
  PcbData.AllowCCs             = fgetlog2();
  PcbData.Validate             = fgetlog2();
  PcbData.LastReadUpdate       = fgetlog2();
  PcbData.EnforceTime          = fgetlog2();

  if ((Num = fgetint2()) == -1)
    PcbData.DisplayNews = 'Y';
  else if (Num == 1)
    PcbData.DisplayNews = 'A';
  else PcbData.DisplayNews = 'N';

  PcbData.DisableCTSdrop       = fgetlog2();
  PcbData.KbdTimeout           = fgetint2();
  PcbData.Network              = fgetlog2();
  PcbData.NodeNum              = fgetint2();
  PcbData.NetTimeout           = fgetint2();
  PcbData.ChatDelay            = fgetint2();

  fgetstr2(PcbData.BoardName,sizeof(PcbData.BoardName));

  fgetstr2(PcbData.ViewBatch,sizeof(PcbData.ViewBatch));
  fgetstr2(PcbData.ViewExt  ,sizeof(PcbData.ViewExt));
  stripleft(PcbData.ViewExt,'.');                       /* remove the '.' for internal use */

  for (x = 0; x < 10; x++)
    fgetstr2(PcbData.FuncKeys[x],sizeof(PcbData.FuncKeys[0]));

  fgetstr2(Temp,sizeof(Temp)); /* used to be Public Conference string */
  PcbData.NumConf            = fgetint2();
  PcbData.NumAreas           = (short) (PcbData.NumConf + 1);
  PcbData.MaxMsgLines        = fgetint2();
  PcbData.DefaultColor       = fgetint2();
  PcbData.DefaultIntensity   = fgetint2();
  PcbData.EventActive        = fgetlog2();
  fgetstr2(PcbData.EventTime,sizeof(PcbData.EventTime));
  PcbData.EventSuspend       = fgetint2();
  PcbData.EventStopUplds     = fgetlog2();
  PcbData.EventSlide         = fgetlog2();
  PcbData.UploadBufSize      = fgetint2();
  PcbData.DisableDriveCheck  = fgetlog2();
  PcbData.ParallelPortNum    = fgetint2();
  PcbData.StopFreeSpace      = fgetint2();

  for (x = SEC_A; x <= SEC_BATCH; x++)
    PcbData.UserLevels[x] = fgetint2();

  fgetstr2(PcbData.ColorFile,sizeof(PcbData.ColorFile));

  PcbData.LowBaudLimit          = fgetlong2();
  PcbData.ModemDelay            = fgetint2();
  PcbData.RingsRequired         = fgetint2();
  PcbData.IrqNum                = fgetint2();
  PcbData.BaseAddress           = fgethex2();
  PcbData.LeaveDtrUp            = fgetlog2();

  if (memcmp(PcbData.ModemPort,"COM1",4) == 0) {
    PcbData.IrqNum = 4;
    PcbData.BaseAddress = 0x3F8;
  } else if (memcmp(PcbData.ModemPort,"COM2",4) == 0) {
    PcbData.IrqNum = 3;
    PcbData.BaseAddress = 0x2F8;
  } else if (memcmp(PcbData.ModemPort,"PORT",4) == 0) {
    PcbData.IrqNum = 0;
    PcbData.BaseAddress = 0;
  }

  PcbData.MaxScrollBack         = fgetint2();
  PcbData.FastText              = fgetlog2();
  PcbData.FastCnames            = fgetlog2();
  PcbData.SysopSec[SEC_SUBS]    = fgetint2();
  PcbData.UploadBy              = fgetlog2();
  PcbData.ShowSettings          = fgetlog2();
  PcbData.AllowShell            = fgetlog2();
  PcbData.Slaves                = fgetlog2();
  PcbData.SubscriptionDays      = fgetint2();

  PcbData.MaxTotalMsgs          = fgetint2();
  PcbData.MaxConfMsgs           = fgetint2();
  PcbData.MinPriorToEvent       = fgetint2();

  PcbData.SysopSec[SEC_EDITALL] = fgetint2();
  PcbData.UserLevels[SEC_EDIT]  = fgetint2();
  PcbData.ShareIRQs             = fgetlog2();
  PcbData.ScanAll               = fgetlog2();
  PcbData.SysopSec[SEC_READONLY]= fgetint2();
  PcbData.UseNewAskFile         = fgetlog2();

  fgetstr2(PcbData.TmpLoc ,sizeof(PcbData.TmpLoc));
  fgetstr2(PcbData.InfFile,sizeof(PcbData.InfFile));

  PcbData.QuickScan        = fgetlog2();
  PcbData.SubscriptWarning = fgetint2();
  PcbData.AllowOneName     = fgetlog2();
  PcbData.DefExpiredLevel  = fgetint2();

  PcbData.UserLevels[SEC_TEST] = fgetint2();
  fgetstr2(PcbData.CapFile,sizeof(PcbData.CapFile));
  PcbData.TestUploads = fgetlog2();
  PcbData.StopClockOnCap = fgetlog2();

  fgetstr2(PcbData.SysopStart,sizeof(PcbData.SysopStart));
  fgetstr2(PcbData.SysopStop ,sizeof(PcbData.SysopStop));

  PcbData.LogCallerNumber  = fgetlog2();
  PcbData.LogConnectStr    = fgetlog2();
  PcbData.LogSecLevel      = fgetlog2();
  PcbData.ConfPwrdAdjust   = fgetlog2();
  PcbData.ConfirmCaller    = fgetlog2();
  PcbData.AllowPwrdComment = fgetlog2();
  PcbData.GuardLogoff      = fgetlog2();
  PcbData.NumDescLines     = fgetint2();
  if (PcbData.NumDescLines == 0)
    PcbData.NumDescLines = 1;

  #ifdef LIB
    /* The Toolkit must be capable of reading v14.5 files.  So if it is not */
    /* v15.0, then get out now before we run into errors reading the file   */
    if (strstr(PcbData.Version,"15.0") == NULL) {
      dosfclose(&pcbfile);
      return;
    }
  #endif

  fgetstr2(PcbData.ChatFile  ,sizeof(PcbData.ChatFile));
  fgetstr2(PcbData.StatsFile ,sizeof(PcbData.StatsFile));
  fgetstr2(PcbData.ChatMenu  ,sizeof(PcbData.ChatMenu));
  fgetstr2(PcbData.NoAnsi    ,sizeof(PcbData.NoAnsi));
  fgetstr2(PcbData.SlowDrives,sizeof(PcbData.SlowDrives));
  fgetstr2(PcbData.SwapPath  ,sizeof(PcbData.SwapPath));
  PcbData.Swap         = fgetlog2();
  PcbData.NoBatchUp    = fgetlog2();
  PcbData.ForceMain    = fgetlog2();
  PcbData.Foreign      = fgetlog2();
  PcbData.MonitorModem = fgetlog2();
  PcbData.No16550      = fgetlog2();
  PcbData.Force16550A  = fgetlog2();
  PcbData.AutoReset    = fgetlog2();
  PcbData.Float        = fgetlog2();
  PcbData.PromoteBatch = fgetlog2();
  PcbData.AutoRegConf  = fgetlog2();
  PcbData.ScanBlts     = fgetlog2();
  PcbData.AutoMakeMsgs = fgetlog2();
  PcbData.VerifyCDLoss = fgetlog2();
  PcbData.Encrypt      = fgetlog2();
  PcbData.UploadCredit = fgetint2();
  PcbData.ByteCredit   = fgetint2();
  PcbData.NewDefaultColor = fgetint2();
  PcbData.MsgHdr1Color = fgetint2();
  PcbData.MsgHdr2Color = fgetint2();
  PcbData.MsgHdr3Color = fgetint2();
  PcbData.MsgHdr4Color = fgetint2();
  PcbData.MsgHdr5Color = fgetint2();
  PcbData.MsgHdr6Color = fgetint2();
  fgetstr2(PcbData.Origin      ,sizeof(PcbData.Origin));
  fgetstr2(PcbData.EventDatFile,sizeof(PcbData.EventDatFile));
  fgetstr2(PcbData.EventFiles  ,sizeof(PcbData.EventFiles));
  fgetstr2(PcbData.CmdLst      ,sizeof(PcbData.CmdLst));
  PcbData.RequirePwrdToExit = fgetlog2();

  /* NOTE:  SEC_UNUSED used to be SEC_MOVE which was redundant as SEC_MOVEMSG */
  /* was already for that purpose.  But now we have to read in SEC_UNUSED     */
  /* anyway or we'll get out of sync.                                         */

  for (x = SEC_15; x <= SEC_UNUSED; x++)
    PcbData.SysopSec[x] = fgetint2();

  PcbData.EnvSize = fgetint2();
  PcbData.LowBaudSecOverride = fgetint2();
  fgetstr2(PcbData.AllFilesList,sizeof(PcbData.AllFilesList));

  PcbData.EnableFido = fgetlog2();
  fgetstr2(PcbData.FidoConfig,sizeof(PcbData.FidoConfig));
  fgetstr2(PcbData.FidoQueue,sizeof(PcbData.FidoQueue));

  /* during the v15.21 beta this needs to take place */
  /* change FIDOAREA.NDX to FIDOQUE.DAT */
  if (strstr(PcbData.FidoQueue,".NDX") != NULL) {
    strcpy(PcbData.FidoQueue,PcbData.EventFiles);
    strcat(PcbData.FidoQueue,"FIDOQUE.DAT");
  }

  PcbData.PromptToReadMail = fgetlog2();
  fgetstr2(PcbData.LogOnScr,sizeof(PcbData.LogOnScr));
  fgetstr2(PcbData.LogOnAns,sizeof(PcbData.LogOnAns));
  fgetstr2(PcbData.QwkFile ,sizeof(PcbData.QwkFile));
  fgetstr2(PcbData.FileTcan,sizeof(PcbData.FileTcan));

  PcbData.SysopSec[SEC_KEEPMSG]    = fgetint2();
  PcbData.SysopSec[SEC_SEERETRCPT] = fgetint2();

  PcbData.SwapDuringBat    = fgetlog2();
  PcbData.UserSysDuringBat = fgetlog2();
  PcbData.DefaultGraphics  = fgetlog2();

  fgetstr2(PcbData.SlowDriveBat,sizeof(PcbData.SlowDriveBat));

  PcbData.AllowAliasChange = fgetlog2();
  PcbData.ExcludeLocals    = fgetlog2();
  PcbData.PwrdUpdate       = fgetint2();
  PcbData.PwrdWarn         = fgetint2();
  PcbData.RecordGroupChat  = fgetlog2();
  PcbData.ShowAlias        = fgetlog2();
  PcbData.MinPwrdLen       = fgetint2();
  PcbData.AllowHandles     = fgetint2();

  fgetstr2(PcbData.CmdLoc ,sizeof(PcbData.CmdLoc));
  PcbData.ForceIntro       = fgetlog2();
  PcbData.SkipProtocol     = fgetlog2();
  PcbData.SkipAlias        = fgetlog2();
  PcbData.NoCarrierExit    = fgetlog2();
  PcbData.OS2Driver        = fgetlog2();

  PcbData.EnableAccounting  = fgetlog2();
  PcbData.ShowCurrency      = fgetlog2();
  PcbData.Concurrent        = fgetlog2();
  fgetstr2(PcbData.PeakStart,sizeof(PcbData.PeakStart));
  fgetstr2(PcbData.PeakEnd  ,sizeof(PcbData.PeakEnd));

  fgetstr2(PcbData.AccountConfig,sizeof(PcbData.AccountConfig));
  fgetstr2(PcbData.AccountInfo,sizeof(PcbData.AccountInfo));
  fgetstr2(PcbData.AccountWarn,sizeof(PcbData.AccountWarn));
  fgetstr2(PcbData.AccountTrack,sizeof(PcbData.AccountTrack));
  fgetstr2(PcbData.AccountLogoff,sizeof(PcbData.AccountLogoff));

  fgetstr2(PcbData.uucpPath,sizeof(PcbData.uucpPath));
  fgetstr2(PcbData.uucpSpoolPath,sizeof(PcbData.uucpSpoolPath));
  fgetstr2(PcbData.uucpLogPath,sizeof(PcbData.uucpLogPath));
  fgetstr2(PcbData.CompBatFile,sizeof(PcbData.CompBatFile));
  fgetstr2(PcbData.DeCompBatFile,sizeof(PcbData.DeCompBatFile));
  fgetstr2(PcbData.uucpName,sizeof(PcbData.uucpName));
  fgetstr2(PcbData.uucpDomainName,sizeof(PcbData.uucpDomainName));
  fgetstr2(PcbData.uucpEmailHost,sizeof(PcbData.uucpEmailHost));
  fgetstr2(PcbData.uucpNewsHost,sizeof(PcbData.uucpNewsHost));
  fgetstr2(PcbData.uucpDefDist,sizeof(PcbData.uucpDefDist));
  fgetstr2(PcbData.uucpTimeZone,sizeof(PcbData.uucpTimeZone));
  fgetstr2(PcbData.uucpSeparator,sizeof(PcbData.uucpSeparator));
  fgetstr2(PcbData.Organization,sizeof(PcbData.Organization));
  fgetstr2(Temp,sizeof(Temp));

  PcbData.uucpHighAscii  = Temp[0];
  PcbData.uucpEmailConf  = fgetint2();
  PcbData.uucpJunkConf   = fgetint2();
  PcbData.uucpBangDomain = fgetlog2();
  PcbData.uucpSubDomain  = fgetlog2();

  PcbData.FidoProcessIn     = fgetlog2();
  PcbData.FidoProcessOrphan = fgetlog2();
  PcbData.FidoProcessOut    = fgetlog2();
  PcbData.FidoDialOut       = fgetlog2();
  PcbData.FidoPktFreq       = fgetint2();
  PcbData.FidoExportFreq    = fgetint2();
  PcbData.FidoMailFreq      = fgetint2();
  PcbData.FidoDefaultZone   = fgetint2();
  PcbData.FidoDefaultNet    = fgetint2();

  fgetstr2(Temp,sizeof(Temp)); getdays2(&PcbData.PeakDays,Temp);
  fgetstr2(PcbData.HolidaysFile,sizeof(PcbData.HolidaysFile));

  fgetstr2(PcbData.uucpModFile,sizeof(PcbData.uucpModFile));

  fgetstr2(PcbData.ModemInit2,sizeof(PcbData.ModemInit2));
  fgetstr2(PcbData.ModemAns,sizeof(PcbData.ModemAns));
  if (PcbData.ModemAns[0] == 0)
    strcpy(PcbData.ModemAns,"ATA");

  PcbData.IgnoreDropSecLevel = fgetlog2();

  fgetstr2(PcbData.ModemDial,sizeof(PcbData.ModemDial));
  if (PcbData.ModemDial[0] == 0)
    strcpy(PcbData.ModemDial,"ATDT");

  PcbData.NumRedials = fgetint2();
  PcbData.MaxTries   = fgetint2();

  if (PcbData.NumRedials == 0)
    PcbData.NumRedials = 1;
  if (PcbData.MaxTries == 0)
    PcbData.MaxTries = 100;

  PcbData.FidoImportAfterXfer = fgetlog2();
  PcbData.FidoCrashSec        = fgetint2();
  PcbData.FidoLogLevel        = fgetint2();
  PcbData.FidoCreateMsg       = fgetlog2();
  PcbData.FidoEnableRouting   = fgetlog2();

  fgetstr2(PcbData.FidoLoc,sizeof(PcbData.FidoLoc));
  if (PcbData.FidoLoc[0] == 0 && PcbData.FidoConfig[0] != 0) {
    /* conversion from v15.21 to v15.22 */
    char *p = findstartofname(PcbData.FidoConfig);
    char Save = *p;
    *p = 0;
    maxstrcpy(PcbData.FidoLoc,PcbData.FidoConfig,sizeof(PcbData.FidoLoc));
    *p = Save;
  }

  PcbData.FidoSecure         = fgetlog2();
  PcbData.FidoSysopChange    = fgetlog2();
  PcbData.FidoCheckDupePath  = fgetlog2();
  PcbData.FidoCheckDupeMsgId = fgetlog2();
  PcbData.FidoNumMsgsToTrack = fgetlong2();
  PcbData.FidoMakeResponse   = fgetlog2();
  PcbData.FidoEnablePassThru = fgetlog2();
  PcbData.FidoEnableAreaFix  = fgetlog2();
  PcbData.FidoAutoAdd        = fgetlog2();
  PcbData.FidoReAddress      = fgetlog2();
  PcbData.FidoRouteEchoMail  = fgetlog2();

  PcbData.MinimizeLogOnOff   = (ubyte) fgetint2();
  PcbData.MinimizeProtocols  = (ubyte) fgetint2();
  PcbData.MinimizeDoors      = (ubyte) fgetint2();
  PcbData.MinimizeFileTest   = (ubyte) fgetint2();
  PcbData.MinimizeFileView   = (ubyte) fgetint2();
  PcbData.MinimizeCompress   = (ubyte) fgetint2();
  PcbData.MinimizeShells     = (ubyte) fgetint2();
  PcbData.PriorityNormal     = (short) fgetint2();
  PcbData.PriorityProtocols  = (short) fgetint2();
  PcbData.PriorityCompress   = (short) fgetint2();
  PcbData.PriorityShells     = (short) fgetint2();
  PcbData.PriorityFidoIn     = (short) fgetint2();
  PcbData.PriorityFidoOut    = (short) fgetint2();

  fgetstr2(PcbData.NetCopy,sizeof(PcbData.NetCopy));

  dosfclose(&pcbfile);


  #ifndef LIB
    /* If the date of the PCBOARD.DAT file is less than 11/10/94 then lets  */
    /* force the swap switch ON.  This is to help reduce tech support calls */
    /* by upgrading older systems to SWAP=TRUE automatically.  In order for */
    /* customers to force SWAP=FALSE, all they have to do is re-edit their  */
    /* PCBOARD.DAT file and save it again after 11/10/94.                   */
    fileexist(DatFile);
    if (DTA.ff_fdate <= ctod("11-10-94"))
      PcbData.Swap = TRUE;
  #endif
}


static char FoundPath[66];

void LIBENTRY readdatfile(void) {
  char *p;

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
  if (dosfopen(DatFile,OPEN_READ|OPEN_DENYNONE,&pcbfile) == -1)
    errorexittodos("Unable to open PCBOARD.DAT");

  memset(&PcbData,0,sizeof(PcbData));
  fgetstr2(PcbData.Version,sizeof(PcbData.Version));

  #ifdef LIB
    /* the Toolkit works with both v14.5 and v15.0 */
    if (strstr(PcbData.Version,"15.0") == NULL && strstr(PcbData.Version,"14.5") == NULL)
      errorexittodos("Wrong version of PCBOARD.DAT file");
  #else
    /* PCBoard, and its utilities, work only with v15.0 */
    if (strstr(PcbData.Version,"15.0") == NULL)
      errorexittodos("Wrong version of PCBOARD.DAT file");
  #endif

  read150file2();
}
