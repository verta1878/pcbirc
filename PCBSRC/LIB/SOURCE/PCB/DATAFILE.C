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
#include <misc.h>
#include <dosfunc.h>
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define BUFLEN   2048

DOSFILE pcbfile;
bool    OldCnames;

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

int LIBENTRY fgetstr(char *Str, int Len) {
  char T[BUFLEN+2];

  if (dosfgets(T,BUFLEN,&pcbfile) != -1) { /*allow lines up to BUFLEN characters long*/
    stripright(T,' ');
    maxstrcpy(Str,T,Len);                  /*but copy only the maximum of Len bytes  */
    return(0);
  } else {
    Str[0] = 0;                            /*if there was an error, return empty string*/
    return(-1);
  }
}


static short _NEAR_ LIBENTRY fgethex(void) {
  char Str[20];
  fgetstr(Str,10);
  return((short) hextoint(Str));
}


static short _NEAR_ LIBENTRY fgetint(void) {
  char Str[20];
  fgetstr(Str,10);
  return((short) atoi(Str));
}


static long _NEAR_ LIBENTRY fgetlong(void) {
  char Str[20];
  fgetstr(Str,10);
  return(atol(Str));
}


static bool _NEAR_ LIBENTRY fgetlog(void) {
  char Str[20];
  fgetstr(Str,10);
  return(atoi(Str) == -1 ? (bool) TRUE : (bool) FALSE);
}


void LIBENTRY read150file(void) {
  char static ALLFILE[11] = {10, '$','#','!','&', 0,'-',':', 0,'A','z' };
  unsigned    x;
  int         Num;
  char        *p;
  char        PublicConf[41];
  char        File[66];
  char        Temp[100];
  pcbconftype Conf;

  fgetstr(PcbData.Sysop   ,sizeof(PcbData.Sysop));
  fgetstr(PcbData.Password,sizeof(PcbData.Password));
  PcbData.UseRealName = fgetlog();
  PcbData.Graphics    = fgetlog();

  for (x = SEC_READCOMMENTS; x <= SEC_14; x++)
    PcbData.SysopSec[x] = fgetint();

  fgetstr(PcbData.HlpLoc ,sizeof(PcbData.HlpLoc));
  fgetstr(PcbData.SecLoc ,sizeof(PcbData.SecLoc));
  fgetstr(PcbData.ChtLoc ,sizeof(PcbData.ChtLoc));
  fgetstr(PcbData.TxtLoc ,sizeof(PcbData.TxtLoc));
  fgetstr(PcbData.NdxLoc ,sizeof(PcbData.NdxLoc));
  fgetstr(PcbData.UsrFile,sizeof(PcbData.UsrFile));
  fgetstr(PcbData.ClrFile,sizeof(PcbData.ClrFile));
  fgetstr(PcbData.CnfFile,sizeof(PcbData.CnfFile));
  fgetstr(PcbData.PwdFile,sizeof(PcbData.PwdFile));
  fgetstr(PcbData.FscFile,sizeof(PcbData.FscFile));
  fgetstr(PcbData.UscFile,sizeof(PcbData.UscFile));
  fgetstr(PcbData.TcnFile,sizeof(PcbData.TcnFile));
  fgetstr(PcbData.WlcFile,sizeof(PcbData.WlcFile));
  fgetstr(PcbData.NewFile,sizeof(PcbData.NewFile));
  fgetstr(PcbData.ClsFile,sizeof(PcbData.ClsFile));
  fgetstr(PcbData.WrnFile,sizeof(PcbData.WrnFile));
  fgetstr(PcbData.ExpFile,sizeof(PcbData.ExpFile));

  fgetstr(PcbData.NetFile  ,sizeof(PcbData.NetFile  ));
  if ((p = strstr(PcbData.NetFile,".DAT")) != NULL)
    memcpy(p,".XXX",4);

  fgetstr(PcbData.CnfMenu  ,sizeof(PcbData.CnfMenu  ));
  fgetstr(PcbData.RegFile  ,sizeof(PcbData.RegFile  ));
  fgetstr(PcbData.AnsFile  ,sizeof(PcbData.AnsFile  ));
  fgetstr(PcbData.TrnFile  ,sizeof(PcbData.TrnFile  ));
  fgetstr(PcbData.DldFile  ,sizeof(PcbData.DldFile  ));
  fgetstr(PcbData.LogOffScr,sizeof(PcbData.LogOffScr));
  fgetstr(PcbData.LogOffAns,sizeof(PcbData.LogOffAns));
  fgetstr(PcbData.MultiLang,sizeof(PcbData.MultiLang));
  fgetstr(PcbData.GroupChat,sizeof(PcbData.GroupChat));

  PcbData.Seconds              = fgetint();
  fgetstr(PcbData.ModemPort,sizeof(PcbData.ModemPort));
  PcbData.ModemSpeed           = fgetlong();
  PcbData.LockSpeed            = fgetlog();
  fgetstr(PcbData.ModemInit,sizeof(PcbData.ModemInit));
  fgetstr(PcbData.ModemOff ,sizeof(PcbData.ModemOff ));
  PcbData.DisableCTS           = fgetlog();
  PcbData.Hayes9600            = fgetlog();
  PcbData.FastComm             = fgetlog();
  PcbData.ResetModem           = fgetlog();
  PcbData.OffHook              = fgetlog();
  PcbData.Packet               = fgetlog();
  PcbData.AnswerRing           = fgetlog();
  PcbData.Allow7E1             = fgetlog();
  PcbData.AllowLowBaud         = fgetlog();
  fgetstr(PcbData.AllowLowStrt,sizeof(PcbData.AllowLowStrt));
  fgetstr(PcbData.AllowLowStop,sizeof(PcbData.AllowLowStop));

  PcbData.DisableScan          = fgetlog();
  PcbData.DisableCls           = fgetlog();
  PcbData.DisableEdits         = fgetlog();
  PcbData.DisableFilter        = fgetlog();
  PcbData.DisableQuick         = fgetlog();
  PcbData.DisablePassword      = fgetlog();
  PcbData.MultiLingual         = fgetlog();
  PcbData.AllowPwrdOnly        = fgetlog();
  PcbData.ClosedBoard          = fgetlog();
  PcbData.NonGraphics          = fgetlog();
  PcbData.ExitToDos            = fgetlog();
  PcbData.IncludeCity          = fgetlog();
  PcbData.EliminateSnow        = fgetlog();
  PcbData.SubscriptMode        = fgetlog();
  PcbData.AllowEscCodes        = fgetlog();
  PcbData.AllowCCs             = fgetlog();
  PcbData.Validate             = fgetlog();
  PcbData.LastReadUpdate       = fgetlog();
  PcbData.EnforceTime          = fgetlog();

  if ((Num = fgetint()) == -1)
    PcbData.DisplayNews = 'Y';
  else if (Num == 1)
    PcbData.DisplayNews = 'A';
  else PcbData.DisplayNews = 'N';

  PcbData.DisableCTSdrop       = fgetlog();
  PcbData.KbdTimeout           = fgetint();
  PcbData.Network              = fgetlog();
  PcbData.NodeNum              = fgetint();
  PcbData.NetTimeout           = fgetint();
  PcbData.ChatDelay            = fgetint();

  fgetstr(PcbData.BoardName,sizeof(PcbData.BoardName));

  fgetstr(PcbData.ViewBatch,sizeof(PcbData.ViewBatch)); /* used to be COMMENT TO SYSOP     */
  fgetstr(PcbData.ViewExt  ,sizeof(PcbData.ViewExt));   /* used to be USER PROMPT          */
  stripleft(PcbData.ViewExt,'.');                       /* remove the '.' for internal use */

  for (x = 0; x < 10; x++)
    fgetstr(PcbData.FuncKeys[x],sizeof(PcbData.FuncKeys[0]));

  memset(PublicConf,0,sizeof(PublicConf));
  fgetstr(PublicConf,sizeof(PublicConf));

  PcbData.NumConf            = fgetint();
  PcbData.NumAreas           = (short) (PcbData.NumConf + 1);
  PcbData.MaxMsgLines        = fgetint();
  PcbData.DefaultColor       = fgetint();
  PcbData.DefaultIntensity   = fgetint();
  PcbData.EventActive        = fgetlog();
  fgetstr(PcbData.EventTime,sizeof(PcbData.EventTime));
  PcbData.EventSuspend       = fgetint();
  PcbData.EventStopUplds     = fgetlog();
  PcbData.EventSlide         = fgetlog();
  PcbData.UploadBufSize      = fgetint();
  PcbData.DisableDriveCheck  = fgetlog();
  PcbData.ParallelPortNum    = fgetint();
  PcbData.StopFreeSpace      = fgetint();

  for (x = SEC_A; x <= SEC_NONREG; x++)
    PcbData.UserLevels[x] = fgetint();

  if (strstr(PcbData.Version,"14.0") == NULL) {
    for (; x <= SEC_BATCH; x++)
      PcbData.UserLevels[x] = fgetint();

    fgetstr(PcbData.ColorFile,sizeof(PcbData.ColorFile));

    PcbData.LowBaudLimit          = fgetlong();
    PcbData.ModemDelay            = fgetint();
    PcbData.RingsRequired         = fgetint();
    PcbData.IrqNum                = fgetint();
    PcbData.BaseAddress           = fgethex();
    PcbData.LeaveDtrUp            = fgetlog();

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

    PcbData.MaxScrollBack         = fgetint();
    PcbData.FastText              = fgetlog();
    PcbData.FastCnames            = fgetlog();
    PcbData.SysopSec[SEC_SUBS]    = fgetint();
    PcbData.UploadBy              = fgetlog();
    PcbData.ShowSettings          = fgetlog();
    PcbData.AllowShell            = fgetlog();
    PcbData.Slaves                = fgetlog();
    PcbData.SubscriptionDays      = fgetint();

    PcbData.MaxTotalMsgs          = fgetint();
    PcbData.MaxConfMsgs           = fgetint();
    PcbData.MinPriorToEvent       = fgetint();

    PcbData.SysopSec[SEC_EDITALL] = fgetint();
    PcbData.UserLevels[SEC_EDIT]  = fgetint();
    PcbData.ShareIRQs             = fgetlog();
    PcbData.ScanAll               = fgetlog();
    PcbData.SysopSec[SEC_READONLY]= fgetint();
    PcbData.UseNewAskFile         = fgetlog();

    fgetstr(PcbData.TmpLoc ,sizeof(PcbData.TmpLoc));
    fgetstr(PcbData.InfFile,sizeof(PcbData.InfFile));

    PcbData.QuickScan        = fgetlog();
    PcbData.SubscriptWarning = fgetint();
    PcbData.AllowOneName     = fgetlog();
    PcbData.DefExpiredLevel  = fgetint();

    PcbData.UserLevels[SEC_TEST] = fgetint();
    fgetstr(PcbData.CapFile,sizeof(PcbData.CapFile));
    PcbData.TestUploads = fgetlog();
    PcbData.StopClockOnCap = fgetlog();

    fgetstr(PcbData.SysopStart,sizeof(PcbData.SysopStart));
    fgetstr(PcbData.SysopStop ,sizeof(PcbData.SysopStop));

    PcbData.LogCallerNumber  = fgetlog();
    PcbData.LogConnectStr    = fgetlog();
    PcbData.LogSecLevel      = fgetlog();
    PcbData.ConfPwrdAdjust   = fgetlog();
    PcbData.ConfirmCaller    = fgetlog();
    PcbData.AllowPwrdComment = fgetlog();
    PcbData.GuardLogoff      = fgetlog();
    PcbData.NumDescLines     = fgetint();
    if (PcbData.NumDescLines == 0)
      PcbData.NumDescLines = 1;
  }

  fgetstr(PcbData.ChatFile  ,sizeof(PcbData.ChatFile));
  fgetstr(PcbData.StatsFile ,sizeof(PcbData.StatsFile));
  fgetstr(PcbData.ChatMenu  ,sizeof(PcbData.ChatMenu));
  fgetstr(PcbData.NoAnsi    ,sizeof(PcbData.NoAnsi));
  fgetstr(PcbData.SlowDrives,sizeof(PcbData.SlowDrives));
  fgetstr(PcbData.SwapPath  ,sizeof(PcbData.SwapPath));
  PcbData.Swap         = fgetlog();
  PcbData.NoBatchUp    = fgetlog();
  PcbData.ForceMain    = fgetlog();
  PcbData.Foreign      = fgetlog();
  PcbData.MonitorModem = fgetlog();
  PcbData.No16550      = fgetlog();
  PcbData.Force16550A  = fgetlog();
  PcbData.AutoReset    = fgetlog();
  PcbData.Float        = fgetlog();
  PcbData.PromoteBatch = fgetlog();
  PcbData.AutoRegConf  = fgetlog();
  PcbData.ScanBlts     = fgetlog();
  PcbData.AutoMakeMsgs = fgetlog();
  PcbData.VerifyCDLoss = fgetlog();
  PcbData.Encrypt      = fgetlog();
  PcbData.UploadCredit = fgetint();
  PcbData.ByteCredit   = fgetint();
  PcbData.NewDefaultColor = fgetint();
  PcbData.MsgHdr1Color = fgetint();
  PcbData.MsgHdr2Color = fgetint();
  PcbData.MsgHdr3Color = fgetint();
  PcbData.MsgHdr4Color = fgetint();
  PcbData.MsgHdr5Color = fgetint();
  PcbData.MsgHdr6Color = fgetint();
  fgetstr(PcbData.Origin      ,sizeof(PcbData.Origin));
  fgetstr(PcbData.EventDatFile,sizeof(PcbData.EventDatFile));
  fgetstr(PcbData.EventFiles  ,sizeof(PcbData.EventFiles));
  fgetstr(PcbData.CmdLst      ,sizeof(PcbData.CmdLst));
  PcbData.RequirePwrdToExit = fgetlog();

  /* NOTE:  SEC_UNUSED used to be SEC_MOVE which was redundant as SEC_MOVEMSG */
  /* was already for that purpose.  But now we have to read in SEC_UNUSED     */
  /* anyway or we'll get out of sync.                                         */

  for (x = SEC_15; x <= SEC_UNUSED; x++)
    PcbData.SysopSec[x] = fgetint();

  PcbData.EnvSize = fgetint();
  PcbData.LowBaudSecOverride = fgetint();

  fgetstr(PcbData.AllFilesList,sizeof(PcbData.AllFilesList));

  PcbData.EnableFido = fgetlog();
  fgetstr(PcbData.FidoConfig,sizeof(PcbData.FidoConfig));
  fgetstr(PcbData.FidoQueue,sizeof(PcbData.FidoQueue));

  /* during the v15.21 beta this needs to take place */
  /* change FIDOAREA.NDX to FIDOQUE.DAT */
  if (strstr(PcbData.FidoQueue,".NDX") != NULL) {
    strcpy(PcbData.FidoQueue,PcbData.EventFiles);
    strcat(PcbData.FidoQueue,"FIDOQUE.DAT");
  }



  PcbData.PromptToReadMail = fgetlog();
  fgetstr(PcbData.LogOnScr,sizeof(PcbData.LogOnScr));
  fgetstr(PcbData.LogOnAns,sizeof(PcbData.LogOnAns));
  fgetstr(PcbData.QwkFile ,sizeof(PcbData.QwkFile));
  fgetstr(PcbData.FileTcan,sizeof(PcbData.FileTcan));

  PcbData.SysopSec[SEC_KEEPMSG]    = fgetint();
  PcbData.SysopSec[SEC_SEERETRCPT] = fgetint();

  PcbData.SwapDuringBat    = fgetlog();
  PcbData.UserSysDuringBat = fgetlog();
  PcbData.DefaultGraphics  = fgetlog();

  fgetstr(PcbData.SlowDriveBat,sizeof(PcbData.SlowDriveBat));

  PcbData.AllowAliasChange = fgetlog();
  PcbData.ExcludeLocals    = fgetlog();
  PcbData.PwrdUpdate       = fgetint();
  PcbData.PwrdWarn         = fgetint();
  PcbData.RecordGroupChat  = fgetlog();
  PcbData.ShowAlias        = fgetlog();
  PcbData.MinPwrdLen       = fgetint();
  PcbData.AllowHandles     = fgetint();

  fgetstr(PcbData.CmdLoc ,sizeof(PcbData.CmdLoc));
  PcbData.ForceIntro       = fgetlog();
  PcbData.SkipProtocol     = fgetlog();
  PcbData.SkipAlias        = fgetlog();
  PcbData.NoCarrierExit    = fgetlog();
  PcbData.OS2Driver        = fgetlog();

  PcbData.EnableAccounting  = fgetlog();
  PcbData.ShowCurrency      = fgetlog();
  PcbData.Concurrent        = fgetlog();
  fgetstr(PcbData.PeakStart,sizeof(PcbData.PeakStart));
  fgetstr(PcbData.PeakEnd  ,sizeof(PcbData.PeakEnd));

  fgetstr(PcbData.AccountConfig,sizeof(PcbData.AccountConfig));
  fgetstr(PcbData.AccountInfo,sizeof(PcbData.AccountInfo));
  fgetstr(PcbData.AccountWarn,sizeof(PcbData.AccountWarn));
  fgetstr(PcbData.AccountTrack,sizeof(PcbData.AccountTrack));
  fgetstr(PcbData.AccountLogoff,sizeof(PcbData.AccountLogoff));

  fgetstr(PcbData.uucpPath,sizeof(PcbData.uucpPath));
  fgetstr(PcbData.uucpSpoolPath,sizeof(PcbData.uucpSpoolPath));
  fgetstr(PcbData.uucpLogPath,sizeof(PcbData.uucpLogPath));
  fgetstr(PcbData.CompBatFile,sizeof(PcbData.CompBatFile));
  fgetstr(PcbData.DeCompBatFile,sizeof(PcbData.DeCompBatFile));
  fgetstr(PcbData.uucpName,sizeof(PcbData.uucpName));
  fgetstr(PcbData.uucpDomainName,sizeof(PcbData.uucpDomainName));
  fgetstr(PcbData.uucpEmailHost,sizeof(PcbData.uucpEmailHost));
  fgetstr(PcbData.uucpNewsHost,sizeof(PcbData.uucpNewsHost));
  fgetstr(PcbData.uucpDefDist,sizeof(PcbData.uucpDefDist));
  fgetstr(PcbData.uucpTimeZone,sizeof(PcbData.uucpTimeZone));
  fgetstr(PcbData.uucpSeparator,sizeof(PcbData.uucpSeparator));
  fgetstr(PcbData.Organization,sizeof(PcbData.Organization));
  fgetstr(Temp,sizeof(Temp));

  PcbData.uucpHighAscii  = Temp[0];
  PcbData.uucpEmailConf  = fgetint();
  PcbData.uucpJunkConf   = fgetint();
  PcbData.uucpBangDomain = fgetlog();
  PcbData.uucpSubDomain  = fgetlog();

  PcbData.FidoProcessIn     = fgetlog();
  PcbData.FidoProcessOrphan = fgetlog();
  PcbData.FidoProcessOut    = fgetlog();
  PcbData.FidoDialOut       = fgetlog();
  PcbData.FidoPktFreq       = fgetint();
  PcbData.FidoExportFreq    = fgetint();
  PcbData.FidoMailFreq      = fgetint();
  PcbData.FidoDefaultZone   = fgetint();
  PcbData.FidoDefaultNet    = fgetint();

  fgetstr(Temp,sizeof(Temp));  getdays(&PcbData.PeakDays,Temp);
  fgetstr(PcbData.HolidaysFile,sizeof(PcbData.HolidaysFile));

  fgetstr(PcbData.uucpModFile,sizeof(PcbData.uucpModFile));

  fgetstr(PcbData.ModemInit2,sizeof(PcbData.ModemInit2));
  fgetstr(PcbData.ModemAns,sizeof(PcbData.ModemAns));
  if (PcbData.ModemAns[0] == 0)
    strcpy(PcbData.ModemAns,"ATA");

  PcbData.IgnoreDropSecLevel = fgetlog();

  fgetstr(PcbData.ModemDial,sizeof(PcbData.ModemDial));
  if (PcbData.ModemDial[0] == 0)
    strcpy(PcbData.ModemDial,"ATDT");

  PcbData.NumRedials = fgetint();
  PcbData.MaxTries   = fgetint();

  if (PcbData.NumRedials == 0)
    PcbData.NumRedials = 1;
  if (PcbData.MaxTries == 0)
    PcbData.MaxTries = 100;

  PcbData.FidoImportAfterXfer = fgetlog();
  PcbData.FidoCrashSec        = fgetint();
  PcbData.FidoLogLevel        = fgetint();
  PcbData.FidoCreateMsg       = fgetlog();
  PcbData.FidoEnableRouting   = fgetlog();

  fgetstr(PcbData.FidoLoc,sizeof(PcbData.FidoLoc));
  if (PcbData.FidoLoc[0] == 0 && PcbData.FidoConfig[0] != 0) {
    /* conversion from v15.21 to v15.22 */
    char Save;
    p = findstartofname(PcbData.FidoConfig);
    Save = *p;
    *p = 0;
    maxstrcpy(PcbData.FidoLoc,PcbData.FidoConfig,sizeof(PcbData.FidoLoc));
    *p = Save;
  }

  PcbData.FidoSecure         = fgetlog();
  PcbData.FidoSysopChange    = fgetlog();
  PcbData.FidoCheckDupePath  = fgetlog();
  PcbData.FidoCheckDupeMsgId = fgetlog();
  PcbData.FidoNumMsgsToTrack = fgetlong();
  PcbData.FidoMakeResponse   = fgetlog();
  PcbData.FidoEnablePassThru = fgetlog();
  PcbData.FidoEnableAreaFix  = fgetlog();
  PcbData.FidoAutoAdd        = fgetlog();
  PcbData.FidoReAddress      = fgetlog();
  PcbData.FidoRouteEchoMail  = fgetlog();

  PcbData.MinimizeLogOnOff   = (ubyte) fgetint();
  PcbData.MinimizeProtocols  = (ubyte) fgetint();
  PcbData.MinimizeDoors      = (ubyte) fgetint();
  PcbData.MinimizeFileTest   = (ubyte) fgetint();
  PcbData.MinimizeFileView   = (ubyte) fgetint();
  PcbData.MinimizeCompress   = (ubyte) fgetint();
  PcbData.MinimizeShells     = (ubyte) fgetint();
  PcbData.PriorityNormal     = (short) fgetint();
  PcbData.PriorityProtocols  = (short) fgetint();
  PcbData.PriorityCompress   = (short) fgetint();
  PcbData.PriorityShells     = (short) fgetint();
  PcbData.PriorityFidoIn     = (short) fgetint();
  PcbData.PriorityFidoOut    = (short) fgetint();

  fgetstr(PcbData.NetCopy,sizeof(PcbData.NetCopy));

  dosfclose(&pcbfile);


  /* If the date of the PCBOARD.DAT file is less than 11/10/94 then lets  */
  /* force the swap switch ON.  This is to help reduce tech support calls */
  /* by upgrading older systems to SWAP=TRUE automatically.  In order for */
  /* customers to force SWAP=FALSE, all they have to do is re-edit their  */
  /* PCBOARD.DAT file and save it again after 11/10/94.                   */
  fileexist(DatFile);
  if (DTA.ff_fdate <= ctod("11-10-94"))
    PcbData.Swap = TRUE;


  /*********************************************************************/


  do {
    strcpy(File,PcbData.CnfFile);
    strcat(File,".@@@");
    if (fileexist(PcbData.CnfFile) != 255 || fileexist(File) != 255)
      break;

    beep();
    boxcls(4,14,75,22,Colors[OUTBOX],SINGLE);
    fastcenter(16,"Unable to locate the Conference Data File",Colors[HEADING]);
    fastcenter(18,"Press ESC to continue or enter a new filename and path below",Colors[DISPLAY]);
    inputstr(39-((sizeof(PcbData.CnfFile)-1+17) >> 1),20,sizeof(PcbData.CnfFile)-1,"New path\\filename",PcbData.CnfFile,PcbData.CnfFile,ALLFILE,INPUT_CAPS,0);
    clsbox(4,14,75,22,0);
    stripright(PcbData.CnfFile,' ');
  } while (KeyFlags != ESC);

  if (KeyFlags == ESC) {
    PcbData.NumConf  = 0;
    PcbData.NumAreas = 1;
    return;
  }

  loadcnames(TRUE);

  if (OldCnames && dosfopen(PcbData.CnfFile,OPEN_READ|OPEN_DENYNONE,&pcbfile) != -1) {
    dossetbuf(&pcbfile,8192);
    resetconffile();
    for (x = 0; x < PcbData.NumAreas; x++) {
      memset(&Conf,0,sizeof(pcbconftype));
      if (fgetstr(Conf.Name,sizeof(Conf.Name)) == -1)
        break;                             /* ah oh, went to far - get out now! */

      if (x == 0 && strcmp(Conf.Name,"Main Board") != 0) {
        if (Conf.Name[0] == 0) {
          strcpy(Conf.Name,"Main Board");
        } else if (strchr(Conf.Name,',') != NULL) {   /* check for commas in name because old format used multiple params on a line */
          cls();
          fastprint(0,0,"PCBOARD.DAT is formatted for v14.0 however CNAMES is not!",0x0F);
          gotoxy(0,2);
          exit(99);
        }
      }

      Conf.AutoRejoin        = fgetlog();
      Conf.ViewMembers       = fgetlog();
      Conf.PrivUplds         = fgetlog();
      Conf.PrivMsgs          = fgetlog();
      Conf.EchoMail          = fgetlog();
      Conf.AddSec            = fgetint();
      Conf.AddTime           = fgetint();
      Conf.MsgBlocks         = fgetint();
      fgetstr(Conf.MsgFile   ,sizeof(Conf.MsgFile   ));
      fgetstr(Conf.UserMenu  ,sizeof(Conf.UserMenu  ));
      fgetstr(Conf.SysopMenu ,sizeof(Conf.SysopMenu ));
      fgetstr(Conf.NewsFile  ,sizeof(Conf.NewsFile  ));
      Conf.PubUpldSort       = fgetint();
      fgetstr(Conf.UpldDir   ,sizeof(Conf.UpldDir   ));
      fgetstr(Conf.PubUpldLoc,sizeof(Conf.PubUpldLoc));
      Conf.PrvUpldSort       = fgetint();
      fgetstr(Conf.PrivDir   ,sizeof(Conf.PrivDir   ));
      fgetstr(Conf.PrvUpldLoc,sizeof(Conf.PrvUpldLoc));
      Conf.PublicConf        = fgetlog();
      fgetstr(Conf.DrsMenu   ,sizeof(Conf.DrsMenu   ));
      fgetstr(Conf.DrsFile   ,sizeof(Conf.DrsFile   ));
      Conf.ReqSecLevel       = fgetint();                /* was NumBull ... now Join Sec    */
      fgetstr(Conf.BltMenu   ,sizeof(Conf.BltMenu   ));
      fgetstr(Conf.BltNameLoc,sizeof(Conf.BltNameLoc));
      fgetstr(Temp           ,sizeof(Temp         ));  /* used to get NumScripts */
      fgetstr(Conf.ScrMenu   ,sizeof(Conf.ScrMenu   ));
      fgetstr(Conf.ScrNameLoc,sizeof(Conf.ScrNameLoc));
      fgetstr(Temp           ,sizeof(Temp         ));  /* used to get NumTextDirs */
      fgetstr(Conf.DirMenu   ,sizeof(Conf.DirMenu   ));
      fgetstr(Conf.DirNameLoc,sizeof(Conf.DirNameLoc));
      fgetstr(Temp           ,sizeof(Temp         ));  /* used to get NumPaths */
      fgetstr(Conf.PthNameLoc,sizeof(Conf.PthNameLoc));

      /* override the values found for the Main Board to force it to be public */
      if (x == 0) {
        Conf.ReqSecLevel = 0;
        Conf.PublicConf  = TRUE;
        Conf.EchoMail    = FALSE;
        Conf.ViewMembers = FALSE;
        Conf.AddSec      = 0;
        Conf.AddTime     = 0;
      }

      if (x < 40 && PublicConf[x] == 'X')  /* this will convert PRE-14.5 */
        Conf.PublicConf = TRUE;            /* public conference strings! */

      writeconfrecord(&Conf);
    }

    if (x > 0) {
      PcbData.NumConf  = (short) (x - 1);
      PcbData.NumAreas = (short) x;
    }

    resetconffile();
    dosfclose(&pcbfile);
  }
}


int LIBENTRY checkdatfile(char Ver[]) {
  #ifndef __OS2__
    char Str[80];
  #endif

  if (dosfopen(DatFile,OPEN_READ|OPEN_DENYNONE,&pcbfile) == -1) {
    #ifndef __OS2__
      if (ExtendedError != 2) {
        sprintf(Str,"Unable to open file [%s]",DatFile);
        errorexittodos(Str);
      }
    #endif
    return(-1);  /* return -1 to indicate file-not-found */
  }
  fgetstr(PcbData.Version,sizeof(PcbData.Version));
  if (strstr(PcbData.Version,Ver) == NULL) {
    cls();
    fastprint( 0,0,"PCBoard Data :",0x0F);
    fastprint(15,0,PcbData.Version ,0x0F);
    fastprint( 0,1,"Searching for:",0x07);
    fastprint(15,1,Ver             ,0x07);
    gotoxy(0,3);
    exit(1);
  }

  read150file();
  return(0);
}
