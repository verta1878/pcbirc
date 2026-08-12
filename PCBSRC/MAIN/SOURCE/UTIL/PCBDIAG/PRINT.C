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


#include <dir.h>
#include <alloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <misc.h>
#include <event.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define printdiffstr(str,x)  { if (strcmp(Conf.x,OldConf.x) != 0) printstr(str,Conf.x); }
#define printdiffnum(str,x)  { if (Conf.x != OldConf.x) printnum(str,Conf.x); }
#define printdifflng(str,x)  { if (Conf.x != OldConf.x) printlng(str,Conf.x); }
#define printdifflog(str,x)  { if (Conf.x != OldConf.x) printlog(str,Conf.x); }
#define printdiffval(str,x)  { if (Conf.x != OldConf.x) printval(str,Conf.x); }

#define MAXFILES 300

typedef struct {
  char     FileName[40];
  char     Desc[25];
  unsigned ConfNum;
} filelisttype;

int          ColSize1;
int          ColSize2;
filelisttype *FileList = NULL;
int          NumFiles = 0;
bool         Error = FALSE;

/********************************************************************
*
*  Function: openprint()
*
*  Desc    : Asks the users for the name of the device or file to send printer
*            output to.
*/

void  openprint(void) {
  int  Len;
  bool Save;
  int  Okay;

  dosfclose(&prn);
  Okay = -1;
  while (Okay == -1) {
    setcursor(CUR_NORMAL);
    showkeystatus();
    boxcls(3,16,76,22,Colors[OUTBOX],SINGLE);
    fastcenter(18,"Enter the name of a file or print device to send output to:",Colors[DISPLAY]);
    inputstr(5,20,35,"e.g. FILENAME.TXT or LPT1 or PRN",smConfig.Dev,smConfig.Dev,ALLCHAR,INPUT_CAPS,0);
    stripright(smConfig.Dev,' ');
    Len = strlen(smConfig.Dev);
    if (smConfig.Dev[Len-1] == ':') {
      Len--;
      smConfig.Dev[Len] = 0;
    }
    if ((memcmp(smConfig.Dev,"LPT",3) == 0 && Len == 4) || strcmp(smConfig.Dev,"PRN") == 0)
      Okay = dosfopen(smConfig.Dev,OPEN_WRIT,&prn);
    else
      Okay = dosfopen(smConfig.Dev,OPEN_WRIT|OPEN_APPEND|OPEN_DENYNONE,&prn);
  }

  Error = FALSE;
  clsbox(4,20,75,20,Colors[OUTBOX]);
  Save = TRUE;
  inputnum(22,20,1,"Save printer definition to disk",&Save,vBOOL,0);
  if (Save && KeyFlags != ESC)
    writeconfig(&smConfig,ConfigName);
}


static int near  sendtoprinter(char *Str) {
  if (Error || dosfputs(Str,&prn) == -1) {
    Error = TRUE;
    return(-1);
  }
  return(0);
}


/********************************************************************
*
*  Function: printtop()
*
*  Desc    : Prints an underlined heading on the printer
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printtop(char *Heading) {
  int  Len;
  char UnderLine[80];
  char Temp[200];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
    mc_register(UnderLine,sizeof(UnderLine));
  #endif

  Len = strlen(Heading);
  if (Len > 79) {
    Heading[79] = 0;
    Len = 79;
  }
  UnderLine[Len] = 0;
  memset(UnderLine,'-',Len);

  sprintf(Temp,"\r\n%s\r\n%s\r\n",Heading,UnderLine);
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
      mc_unregister(UnderLine);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
    mc_unregister(UnderLine);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printstr()
*
*  Desc    : Prints the string Desc[] left justified in a field of ColSize1
*            characters in length followed by Str[] on the printer
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printstr(char *Desc, char *Str) {
  char Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  sprintf(Temp,"%-*s: %s\r\n",ColSize1,Desc,(Str[0] == 0 ? "(blank)" : Str));
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printnum()
*
*  Desc    : Prints the string Desc[] left justified in a field of ColSize1
*            characters in length followed by a Number in a field of ColSize2
*            characters in length.
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printnum(char Desc[], int Num) {
  char Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  sprintf(Temp,"%-*s: %*d\r\n",ColSize1,Desc,ColSize2,Num);
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


int  printval(char Desc[], double Num) {
  char Temp[128];
  char Val[20];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  dcomma(Val,Num);
  sprintf(Temp,"%-*s: %s\r\n",ColSize1,Desc,Val);
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printhex()
*
*  Desc    : Prints the string Desc[] left justified in a field of ColSize1
*            characters in length followed by a Number in a field of ColSize2
*            characters in length.  The number is printed in HEX format.
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printhex(char Desc[], int Num) {
  char Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  sprintf(Temp,"%-*s: %*X\r\n",ColSize1,Desc,ColSize2,Num);
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printlng()
*
*  Desc    : Prints the string Desc[] left justified in a field of ColSize1
*            characters in length followed by a Number in a field of ColSize2
*            characters in length.
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printlng(char Desc[], long Num) {
  char Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  sprintf(Temp,"%-*s: %*ld\r\n",ColSize1,Desc,ColSize2,Num);
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printlog()
*
*  Desc    : Prints the string Desc[] left justified in a field of ColSize1
*            characters in length followed by a Logical value (Y or N).
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printlog(char Desc[], bool Value) {
  char Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  sprintf(Temp,"%-*s: %c\r\n",ColSize1,Desc,(Value ? 'Y' : 'N'));
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printkey()
*
*  Desc    : Used to print the value of a Function Key as defined in the
*            PCBOARD.DAT file.
*
*  Returns : -1 if an error occurred, otherwise 0
*/

int  printkey(char Desc[], char Str[]) {
  char Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  sprintf(Temp,"(shift)+F%-*s: %s\r\n",ColSize1-9,Desc,Str);
  if (sendtoprinter(Temp) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printdatafile()
*
*  Desc    : Prints all of the values in the PCBOARD.DAT file.
*
*/

void  printdatafile(void) {
  int  X;
  char Str[10];
  char UploadCredit[10];
  char ByteCredit[10];
  char Q[20];

  #ifdef DEBUG
    mc_register(Q,sizeof(Q));
    mc_register(Str,sizeof(Str));
    mc_register(UploadCredit,sizeof(UploadCredit));
    mc_register(ByteCredit,sizeof(ByteCredit));
  #endif

  ColSize1 = 26;
  printstr("Version of PCBOARD.DAT"   ,PcbData.Version);
  printstr("Sysop's Display Name"     ,PcbData.Sysop);
  printstr("Local Logon Password"     ,"Not printed for security reasons");
  printlog("Require Password to Exit" ,PcbData.RequirePwrdToExit);
  printlog("Use Real Name"            ,PcbData.UseRealName);
  printlog("Graphics on in Local Mode",PcbData.Graphics);

  printtop("File Locations : System Files");
  printstr("Conference Data"       ,PcbData.CnfFile);
  printstr("User File"             ,PcbData.UsrFile);
  printstr("User Info File"        ,PcbData.InfFile);
  printstr("Caller Log"            ,PcbData.ClrFile);
  printstr("Group CHAT File"       ,PcbData.ChatFile);
  printstr("Group CHAT .CAP Files" ,PcbData.ChtLoc);
  printstr("Statistics File"       ,PcbData.StatsFile);
  printstr("USERNET.XXX File"      ,PcbData.NetFile);
  printstr("Transfer Summary File" ,PcbData.DldFile);
  printstr("Swap File"             ,PcbData.SwapPath);
  printstr("PCBTEXT Files"         ,PcbData.TxtLoc);
  printstr("USERS Index Files"     ,PcbData.NdxLoc);
  printstr("Temporary Work Files"  ,PcbData.TmpLoc);
  printstr("Help Files"            ,PcbData.HlpLoc);
  printstr("Security Message Files",PcbData.SecLoc);
  printstr("Command Display Files" ,PcbData.CmdLoc);

  printtop("File Locations : Configuration Files");
  printstr("PWRD  File"            ,PcbData.PwdFile);
  printstr("FSEC  File"            ,PcbData.FscFile);
  printstr("UPSEC File"            ,PcbData.UscFile);
  printstr("TCAN  File"            ,PcbData.TcnFile);
  printstr("Protocol Data File"    ,PcbData.TrnFile);
  printstr("Multi-Lang. Data File" ,PcbData.MultiLang);
  printstr("Color Definition File" ,PcbData.ColorFile);
  printstr("Default CMD.LST File"  ,PcbData.CmdLst);
  printstr("All-Files DLPATH.LST"  ,PcbData.AllFilesList);
  printstr("Upload File Trashcan"  ,PcbData.FileTcan);
  printstr("View Batch File"       ,PcbData.ViewBatch);
  printstr("View Extension"        ,PcbData.ViewExt);

  printtop("File Locations : Display Files");
  printstr("WELCOME File"          ,PcbData.WlcFile);
  printstr("NEWUSER File"          ,PcbData.NewFile);
  printstr("CLOSED  File"          ,PcbData.ClsFile);
  printstr("WARNING File"          ,PcbData.WrnFile);
  printstr("EXPIRED File"          ,PcbData.ExpFile);
  printstr("Conference Join Menu"  ,PcbData.CnfMenu);
  printstr("Group Chat Intro File" ,PcbData.GroupChat);
  printstr("Group Chat Menu"       ,PcbData.ChatMenu);
  printstr("NOANSI Warning"        ,PcbData.NoAnsi);

  printtop("File Locations : Questionnaires");
  printstr("New Reg Questionnaire",PcbData.RegFile);
  printstr("Answers to New Reg."  ,PcbData.AnsFile);
  printstr("Logon  Questionnaire" ,PcbData.LogOnScr);
  printstr("Logon  Answers"       ,PcbData.LogOnAns);
  printstr("Logoff Questionnaire" ,PcbData.LogOffScr);
  printstr("Logoff Answers"       ,PcbData.LogOffAns);

  ColSize1 = 28;
  printtop("Modem Information : Setup");
  sprintf(Str,"%X",PcbData.BaseAddress);
  printnum("Seconds to wait for carrier"  ,PcbData.Seconds);
  printstr("Comm Driver"                  ,(strchr(PcbData.ModemPort,'F') != NULL ? "FOSSIL" : (PcbData.ModemPort[0] == 'P' ? "COMM-DRV" : "ASYNC")));
  printstr("Comm Port"                    ,PcbData.ModemPort);
  printnum("IRQ Number"                   ,PcbData.IrqNum);
  printstr("Base Address"                 ,Str);
  printlng("Opening Baud Rate"            ,PcbData.ModemSpeed);
  printlog("Lock in Opening Baud Rate"    ,PcbData.LockSpeed);
  printstr("Modem Initialization String 1",PcbData.ModemInit);
  printstr("Modem Initialization String 2",PcbData.ModemInit2);
  printstr("Modem Off-Hook String"        ,PcbData.ModemOff);
  printstr("Modem Answer String"          ,PcbData.ModemAns);
  printstr("Modem Dialout String"         ,PcbData.ModemDial);
  printnum("Max # of Redials on Busy"     ,PcbData.NumRedials);
  printnum("Max # of Attempts to Connect" ,PcbData.MaxTries);

  ColSize1 = 30;
  printtop("Modem Information : Switches");
  printlog("Disable CTS/RTS Checking"     ,PcbData.DisableCTS);
  printlog("Disable RTS-Drop During Write",PcbData.DisableCTSdrop);
  printlog("Using a FastComm 9600"        ,PcbData.FastComm);
  printlog("Reset While Idle"             ,PcbData.AutoReset);
  printlog("Reset During Recycle"         ,PcbData.ResetModem);
  printlog("Off-Hook During Recycle"      ,PcbData.OffHook);
  printnum("Delay During Recycle"         ,PcbData.ModemDelay);
  printlog("Packet-Switch Network"        ,PcbData.Packet);
  printlog("Verify CD Loss"               ,PcbData.VerifyCDLoss);
  printlog("Leave DTR Up at Drop to DOS"  ,PcbData.LeaveDtrUp);
  printlog("Drop to DOS on Missed Connect",PcbData.NoCarrierExit);
  printlog("Answer on True Ring Detect"   ,PcbData.AnswerRing);
  printnum("Number of Rings Required"     ,PcbData.RingsRequired);
  printlog("Monitor Missed Connections"   ,PcbData.MonitorModem);
  printlog("Force NON-16550"              ,PcbData.No16550);
  printlog("Force 16550A"                 ,PcbData.Force16550A);
  printlog("Share IRQ's on MCA Buses"     ,PcbData.ShareIRQs);

  ColSize1 = 28;
  printtop("Modem Information : Allowed Access");
  printlog("Allow Callers at 7,E,1"       ,PcbData.Allow7E1);
  printlng("Lowest Baud Rate Desired"     ,PcbData.LowBaudLimit);
  printlog("Allow Lower Speed Callers"    ,PcbData.AllowLowBaud);
  printstr("Low Speed Caller Start Time"  ,PcbData.AllowLowStrt);
  printstr("Low Speed Caller Stop Time"   ,PcbData.AllowLowStop);
  printnum("Security Level Override"      ,PcbData.LowBaudSecOverride);

  ColSize1 = 32;
  printtop("Node Information");
  printstr("Board Name"                      ,PcbData.BoardName);
  printstr("Origin"                          ,PcbData.Origin);
  printnum("Parallel Port Num"               ,PcbData.ParallelPortNum);
  printlog("Running Under a Network"         ,PcbData.Network);
  printnum("Node Number if on a Network"     ,PcbData.NodeNum);
  printlog("Float Node Number"               ,PcbData.Float);
  printnum("Network Timeout (20-99 secs)"    ,PcbData.NetTimeout);
  printnum("Node Chat Frequency (5-99 secs)" ,PcbData.ChatDelay);
  printlog("Include City in WHO Display"     ,PcbData.IncludeCity);
  printlog("Show Alias in WHO Display"       ,PcbData.ShowAlias);
  printlog("Using Slave Cards"               ,PcbData.Slaves);

  printtop("Event Information");
  printlog("Is Timed-Event Active"           ,PcbData.EventActive);
  printstr("Name/Loc of EVENT.DAT"           ,PcbData.EventDatFile);
  printstr("Location of Event Files"         ,PcbData.EventFiles);
  printnum("Min. to Suspend Prior to Event"  ,PcbData.EventSuspend);
  printlog("Disallow Uploads Prior to Event" ,PcbData.EventStopUplds);
  printnum("Min. Prior to Disallow Uploads"  ,PcbData.MinPriorToEvent);

  ColSize1 = 33;
  printtop("Subscription Information");
  printlog("Enable Subscription Mode"        ,PcbData.SubscriptMode   );
  printnum("Default Subscription Length"     ,PcbData.SubscriptionDays);
  printnum("Default 'Expired' Security Level",PcbData.DefExpiredLevel );
  printnum("Warning Days Prior to Expiration",PcbData.SubscriptWarning);

  ColSize1 = 30;
  printtop("Configuration Options : Messages");
  printnum("Highest Number Conference"    ,PcbData.NumConf);
  printnum("Max Lines in Msg Editor"      ,PcbData.MaxMsgLines);
  printnum("Capture - Max Messages"       ,PcbData.MaxTotalMsgs);
  printnum("Capture - Max Per Conf"       ,PcbData.MaxConfMsgs);
  printstr("Name of Capture File"         ,PcbData.CapFile);
  printstr("Name of QWK Packet"           ,PcbData.QwkFile);
  printlog("Stop Clock On Msg Download"   ,PcbData.StopClockOnCap);
  printlog("Disable Message Scan Prompt"  ,PcbData.DisableScan);
  printlog("Allow ESC Codes in Messages"  ,PcbData.AllowEscCodes);
  printlog("Allow Carbon-Copy Messages"   ,PcbData.AllowCCs);
  printlog("Validate TO: Name in Messages",PcbData.Validate);
  printlog("Force COMMENTS into Main"     ,PcbData.ForceMain);
  printlog("Double-Byte Characters"       ,PcbData.Foreign);
  printlog("Create MSGS if Missing"       ,PcbData.AutoMakeMsgs);
  printlog("Default to Quick Scan"        ,PcbData.QuickScan);
  printlog("Default to Scan ALL Confs"    ,PcbData.ScanAll);
  printlog("Prompt to Read New Mail"      ,PcbData.PromptToReadMail);

  printtop("Configuration Options : File Transfers");
  decimal(UploadCredit,PcbData.UploadCredit);
  decimal(ByteCredit  ,PcbData.ByteCredit);
  printlog("Disallow BATCH Uploads"       ,PcbData.NoBatchUp);
  printlog("Promote to BATCH Transfers"   ,PcbData.PromoteBatch);
  printstr("Upload Credit for Time"       ,UploadCredit);
  printstr("Upload Credit for Bytes"      ,ByteCredit);
  printlog("Include 'Uploaded By' in Desc",PcbData.UploadBy);
  printlog("Verify Uploaded Files"        ,PcbData.TestUploads);
  printlog("Disable Drive Size Check"     ,PcbData.DisableDriveCheck);
  printnum("Upload Buffer Size"           ,PcbData.UploadBufSize);
  printstr("List of Slow Drives"          ,PcbData.SlowDrives);
  printstr("Slow Drive Batch File"        ,PcbData.SlowDriveBat);
  printnum("Stop Uploads when Free Space is less than (K)",PcbData.StopFreeSpace);

  ColSize1 = 31;
  printtop("Configuration Options : System Control");
  printlog("Disable NS Logon"                ,PcbData.DisableQuick);
  printlog("Disable Password Check (DOS)"    ,PcbData.DisablePassword);
  printlog("Multi-Lingual Operation"         ,PcbData.MultiLingual);
  printlog("Disable Full Record Updating"    ,PcbData.AllowPwrdOnly);
  printlog("Allow Alias Change"              ,PcbData.AllowAliasChange);
  printlog("Run System as a Closed Board"    ,PcbData.ClosedBoard);
  printlog("Enforce Daily Time Limit"        ,PcbData.EnforceTime);
  printlog("Allow One Name Users"            ,PcbData.AllowOneName);
  printlog("Allow Password Failure Comment"  ,PcbData.AllowPwrdComment);
  printlog("Warning on Logoff Command"       ,PcbData.GuardLogoff);
  printlog("Allow Local SHELL to DOS"        ,PcbData.AllowShell);
  printlog("Use NEWASK+Standard Questions"   ,PcbData.UseNewAskFile);
  printlog("Skip Protocol when Registering"  ,PcbData.SkipProtocol);
  printlog("Skip Alias when Registering"     ,PcbData.SkipAlias);
  printlog("Read PWRD on Conference Join"    ,PcbData.ConfPwrdAdjust);
  printlog("Confirm Caller Name / Address"   ,PcbData.ConfirmCaller);
  printlog("Auto Reg in Public Conferences"  ,PcbData.AutoRegConf);
  printlog("Encrypt Users File"              ,PcbData.Encrypt);

  ColSize1 = 32;
  printtop("Configuration Options : Switches");
  printlog("Disable 3-minute Screen Clear"   ,PcbData.DisableCls);
  printlog("Disable Registration Edits"      ,PcbData.DisableEdits);
  printlog("Disable High-ASCII Filter"       ,PcbData.DisableFilter);
  printlog("Default to Graphics at Login"    ,PcbData.DefaultGraphics);
  printlog("Use Non-Graphics Mode Only"      ,PcbData.NonGraphics);
  printlog("Exclude Local Logins from Stats" ,PcbData.ExcludeLocals);
  printlog("Exit to DOS After Each Call"     ,PcbData.ExitToDos);
  printlog("Eliminate Screen Snow in PCB"    ,PcbData.EliminateSnow);
  printlog("Display NEWS only if changed"    ,PcbData.DisplayNews);
  printlog("Display User Info at Login"      ,PcbData.ShowSettings);
  printlog("Force INTRO Display on Join"     ,PcbData.ForceIntro);
  printlog("Pre-load PCBTEXT File"           ,PcbData.FastText);
  printlog("Pre-load CNAMES File"            ,PcbData.FastCnames);
  printlog("Scan for New Bulletins"          ,PcbData.ScanBlts);
  printlog("Swap Out During Shell"           ,PcbData.Swap);
  printlog("Swap Out During $$LOGON/LOGOFF"  ,PcbData.SwapDuringBat);
  printlog("Create USERS.SYS for $$LOGON"    ,PcbData.UserSysDuringBat);
  printlog("Capture GROUP CHAT to Disk"      ,PcbData.RecordGroupChat);
  printlog("Allow Handles in Group Chat"     ,PcbData.AllowHandles);

  ColSize1 = 27;
  printtop("Configuration Options : Logging Options");
  printlog("Log Caller Number to Disk"       ,PcbData.LogCallerNumber);
  printlog("Log Connect String to Disk"      ,PcbData.LogConnectStr);
  printlog("Log Security Level to Disk"      ,PcbData.LogSecLevel);

  ColSize1 = 32;
  printtop("Configuration Options : Limits");
  printnum("Keyboard Timeout in Minutes"     ,PcbData.KbdTimeout);
  printnum("Num of Upload Description Lines" ,PcbData.NumDescLines);
  printnum("Max Lines in Scrollback"         ,PcbData.MaxScrollBack);
  printnum("DOS Environment Size"            ,PcbData.EnvSize);
  printnum("Num of Days Before Pwrd Change"  ,PcbData.PwrdUpdate);
  printnum("Num of Days to Warn"             ,PcbData.PwrdWarn);
  printnum("Minimum Password Length"         ,PcbData.MinPwrdLen);
  printstr("Allow Sysop Page Start Time"     ,PcbData.SysopStart);
  printstr("Allow Sysop Page Stop Time"      ,PcbData.SysopStop);

  ColSize1 = 30;
  printtop("Configuration Options : Colors");
  printhex("Default Color"                   ,PcbData.NewDefaultColor);
  printhex("Color for Message Header DATE"   ,PcbData.MsgHdr1Color);
  printhex("Color for Message Header TO"     ,PcbData.MsgHdr2Color);
  printhex("Color for Message Header FROM"   ,PcbData.MsgHdr3Color);
  printhex("Color for Message Header SUBJ"   ,PcbData.MsgHdr4Color);
  printhex("Color for Message Header READ"   ,PcbData.MsgHdr5Color);
  printhex("Color for Message Header CONF"   ,PcbData.MsgHdr6Color);

  ColSize1 = 10;
  printtop("Configuration Options : Function Keys");
  strcpy(Q,"F-Key #0 ");
  for (X = 0; X < 9; X++) {
    Q[7]++;
    printstr(Q,PcbData.FuncKeys[X]);
  }
  Q[7] = '1'; Q[8] = '0';
  printstr(Q,PcbData.FuncKeys[9]);

  ColSize1 = 28;
  printtop("Security Levels : Sysop Functions");
  printnum(" (1) View/Print Callers Log"                  ,PcbData.SysopSec[SEC_1 ]);
  printnum(" (2) View/Print Users List"                   ,PcbData.SysopSec[SEC_2 ]);
  printnum(" (3) Pack Renumber Messages"                  ,PcbData.SysopSec[SEC_3 ]);
  printnum(" (4) Recover Killed Message"                  ,PcbData.SysopSec[SEC_4 ]);
  printnum(" (5) List Message Headers"                    ,PcbData.SysopSec[SEC_5 ]);
  printnum(" (6) View Any File"                           ,PcbData.SysopSec[SEC_6 ]);
  printnum(" (7) User Maintenance"                        ,PcbData.SysopSec[SEC_7 ]);
  printnum(" (8) Pack Users File"                         ,PcbData.SysopSec[SEC_8 ]);
  printnum(" (9) Exit to DOS remote"                      ,PcbData.SysopSec[SEC_9 ]);
  printnum("(10) Shelled DOS functions"                   ,PcbData.SysopSec[SEC_10]);
  printnum("(11) View Other Nodes"                        ,PcbData.SysopSec[SEC_11]);
  printnum("(12) Logoff Alternate Node"                   ,PcbData.SysopSec[SEC_12]);
  printnum("(13) View Alt Node Callers"                   ,PcbData.SysopSec[SEC_13]);
  printnum("(14) Drop Alt Node to DOS"                    ,PcbData.SysopSec[SEC_14]);
  printnum("(15) Drop to DOS/Recycle"                     ,PcbData.SysopSec[SEC_15]);

  ColSize1 = 35;
  printtop("Security Levels : Sysop Commands");
  printnum("Sysop Level (Sysop Menu/TempSysop)"  ,PcbData.SysopSec[SEC_SYSOPLEVEL  ]);
  printnum("Read All Comments"                   ,PcbData.SysopSec[SEC_READCOMMENTS]);
  printnum("Read All Mail Except Comments"       ,PcbData.SysopSec[SEC_READALLMAIL ]);
  printnum("Copy or Move Messages"               ,PcbData.SysopSec[SEC_MOVEMSG     ]);
  printnum("Enter @-variables in Messages"       ,PcbData.SysopSec[SEC_SUBS        ]);
  printnum("Edit any Message"                    ,PcbData.SysopSec[SEC_EDITALL     ]);
  printnum("NOT Update Msg Read Status"          ,PcbData.SysopSec[SEC_READONLY    ]);
  printnum("Use the BROADCAST Command"           ,PcbData.SysopSec[SEC_BROADCAST   ]);
  printnum("View the Private Upload Directory"   ,PcbData.SysopSec[SEC_VIEWPRIV    ]);
  printnum("Enter Generic Messages"              ,PcbData.SysopSec[SEC_GENERICMSGS ]);
  printnum("Edit Message Headers"                ,PcbData.SysopSec[SEC_EDITHDR     ]);
  printnum("Protect/Unprotect a Message"         ,PcbData.SysopSec[SEC_PROTECT     ]);
  printnum("Overwrite Files on Uploads"          ,PcbData.SysopSec[SEC_OVERWRITE   ]);
  printnum("Set the Pack-Out Date on Messages"   ,PcbData.SysopSec[SEC_KEEPMSG     ]);
  printnum("See All Return Receipt Messages"     ,PcbData.SysopSec[SEC_SEERETRCPT  ]);

  ColSize1 = 24;
  ColSize2 =  3;
  printtop("Security Levels : UserLevels");
  printnum("A) Abandon Conference"  ,PcbData.UserLevels[SEC_A]);
  printnum("B) Bulletin Listings"   ,PcbData.UserLevels[SEC_B]);
  printnum("C) Comment to Sysop"    ,PcbData.UserLevels[SEC_C]);
  printnum("D) Download a File"     ,PcbData.UserLevels[SEC_D]);
  printnum("E) Enter a Message"     ,PcbData.UserLevels[SEC_E]);
  printnum("F) File Directory"      ,PcbData.UserLevels[SEC_F]);
  printnum("H) Help Functions"      ,PcbData.UserLevels[SEC_H]);
  printnum("I) Initial Welcome"     ,PcbData.UserLevels[SEC_I]);
  printnum("J) Join a Conference"   ,PcbData.UserLevels[SEC_J]);
  printnum("K) Kill a Message"      ,PcbData.UserLevels[SEC_K]);
  printnum("L) Locate File Name"    ,PcbData.UserLevels[SEC_L]);
  printnum("M) Mode (graphics)"     ,PcbData.UserLevels[SEC_M]);
  printnum("N) New Files Scan"      ,PcbData.UserLevels[SEC_N]);
  printnum("O) Operator Page"       ,PcbData.UserLevels[SEC_O]);
  printnum("P) Page Length"         ,PcbData.UserLevels[SEC_P]);
  printnum("Q) Quick Message Scan"  ,PcbData.UserLevels[SEC_Q]);
  printnum("R) Read Messages"       ,PcbData.UserLevels[SEC_R]);
  printnum("S) Script Questionnaire",PcbData.UserLevels[SEC_S]);
  printnum("T) Transfer Protocol"   ,PcbData.UserLevels[SEC_T]);
  printnum("U) Upload a File"       ,PcbData.UserLevels[SEC_U]);
  printnum("V) View Settings"       ,PcbData.UserLevels[SEC_V]);
  printnum("W) Write User Info."    ,PcbData.UserLevels[SEC_W]);
  printnum("X) Expert Mode Toggle"  ,PcbData.UserLevels[SEC_X]);
  printnum("Y) Your Personal Mail"  ,PcbData.UserLevels[SEC_Y]);
  printnum("Z) Zippy DIR Scan"      ,PcbData.UserLevels[SEC_Z]);
  printnum("Group CHAT"             ,PcbData.UserLevels[SEC_CHAT]);
  printnum("OPEN a DOOR"            ,PcbData.UserLevels[SEC_OPEN]);
  printnum("TEST a File"            ,PcbData.UserLevels[SEC_TEST]);
  printnum("USER Search/Display"    ,PcbData.UserLevels[SEC_USER]);
  printnum("WHO is On Another Node" ,PcbData.UserLevels[SEC_WHO]);
  printnum("Batch Transfers"        ,PcbData.UserLevels[SEC_BATCH]);
  printnum("EDIT Your Own Messages" ,PcbData.UserLevels[SEC_EDIT]);
  printnum("Agree  to Register"     ,PcbData.UserLevels[SEC_REG]);
  printnum("Refuse to Register"     ,PcbData.UserLevels[SEC_NONREG]);

  ColSize1 = 33;
  undogetdays(Str,&PcbData.PeakDays);
  printtop("Accounting Configuration");
  printlog("Enable Accounting Features"      ,PcbData.EnableAccounting);
  printlog("Display Money instead of Credits",PcbData.ShowCurrency);
  printlog("Concurrent Tracking of Charges"  ,PcbData.Concurrent);
  printlog("Ignore Empty Security Level"     ,PcbData.IgnoreDropSecLevel);
  printstr("Peak Usage Start Time"           ,PcbData.PeakStart);
  printstr("Peak Usage End Time"             ,PcbData.PeakEnd);
  printstr("Peak Days of Week"               ,Str);
  printstr("Peak Holidays List File"         ,PcbData.HolidaysFile);
  printstr("Account Configuration File"      ,PcbData.AccountConfig);
  printstr("Account Tracking File"           ,PcbData.AccountInfo);
  printstr("Account INFO File"               ,PcbData.AccountWarn);
  printstr("Account WARNING File"            ,PcbData.AccountTrack);
  printstr("Account LOGOFF File"             ,PcbData.AccountLogoff);

  ColSize1 = 33;
  printtop("Fido Processing Configuration");
  printlog("Enable Fido Processing"          ,PcbData.EnableFido);
  printstr("Import/Export Configuration File",PcbData.FidoConfig);
  printstr("Fido Queue"                      ,PcbData.FidoQueue);
  printlog("Import After File Transfer"      ,PcbData.FidoImportAfterXfer);
  printlog("Process Incoming Packets"        ,PcbData.FidoProcessIn);
  printnum("Inbound Scan Frequency"          ,PcbData.FidoPktFreq);
  printlog("Export Mail"                     ,PcbData.FidoProcessOut);
  printnum("Export Scan Frequency"           ,PcbData.FidoExportFreq);
  printlog("Dial Out"                        ,PcbData.FidoDialOut);
  printnum("Dial Out Frequency"              ,PcbData.FidoMailFreq);
  printnum("Fido Default Zone"               ,PcbData.FidoDefaultZone);
  printnum("Fido Default Net"                ,PcbData.FidoDefaultNet);
  printnum("Security Req'd for +C and +D"    ,PcbData.FidoCrashSec);
  printnum("Fido Logging Level"              ,PcbData.FidoLogLevel);
  printlog("Create MSG with Outbound PKTs"   ,PcbData.FidoCreateMsg);
  printlog("Enable Inbound Routing"          ,PcbData.FidoEnableRouting);

  ColSize1 = 23;
  ColSize2 =  5;
  Str[0] = PcbData.uucpHighAscii;
  Str[1] = 0;
  printtop("UUCP Configuration");
  printstr("Organization"          ,PcbData.Organization);
  printstr("Base UUCP Path"        ,PcbData.uucpPath);
  printstr("Spool Directory"       ,PcbData.uucpSpoolPath);
  printstr("Log File Directory"    ,PcbData.uucpLogPath);
  printstr("Decompress Batch"      ,PcbData.DeCompBatFile);
  printstr("Moderator List"        ,PcbData.uucpModFile);
  printstr("UUCP Name"             ,PcbData.uucpName);
  printstr("UUCP Domain Name"      ,PcbData.uucpDomainName);
  printstr("UUCP Email Host"       ,PcbData.uucpEmailHost);
  printstr("UUCP News Host"        ,PcbData.uucpNewsHost);
  printstr("News Distribution"     ,PcbData.uucpDefDist);
  printstr("Time Zone Offset"      ,PcbData.uucpTimeZone);
  printstr("Name Separator"        ,PcbData.uucpSeparator);
  printnum("Email Conference"      ,PcbData.uucpEmailConf);
  printnum("Junk Conference"       ,PcbData.uucpJunkConf);
  printlog("Bang Domain"           ,PcbData.uucpBangDomain);
  printlog("Sub Domain"            ,PcbData.uucpSubDomain);
  printstr("High Ascii"            ,Str);

  #ifdef DEBUG
    mc_unregister(Q);
    mc_unregister(Str);
    mc_unregister(UploadCredit);
    mc_unregister(ByteCredit);
  #endif
}


/********************************************************************
*
*  Function: printconffiles()
*
*  Desc    : Prints the values in the CNAMES file for conference information
*
*  Returns :
*/

void  printconffiles(void) {
  unsigned    Counter;
  char        Str[120];

  #ifdef DEBUG
    mc_register(Str,sizeof(Str));
  #endif

  ColSize1 = 28;
  ColSize2 = 3;

  memset(&OldConf,0xFF,sizeof(pcbconftype));
  OldConf.ChargeTime     = -1;
  OldConf.ChargeMsgRead  = -1;
  OldConf.ChargeMsgWrite = -1;

  for (Counter = 0; Counter < PcbData.NumAreas; Counter++) {
    getconfrecord(Counter,&Conf);

    if (Conf.Name[0] != 0) {
      sprintf(Str,"Conference #%u %-60.60s",Counter,Conf.Name);
      stripright(Str,' ');
      printtop(Str);

      printdifflog("Public Conference",PublicConf);
      printdiffnum("Required Security if Public",ReqSecLevel);
      printdiffstr("Password to Join if Public",Password);
      printdiffstr("Auto Register Flags",RegFlags);
      printdiffnum("Number of Message Blocks",MsgBlocks);
      printdiffstr("MSGS File",MsgFile);
      printdiffstr("Users Menu",UserMenu);
      printdiffstr("Sysops Menu",SysopMenu);
      printdiffstr("NEWS File",NewsFile);
      printdiffstr("Conf Intro",Intro);
      printdiffstr("Attachment Location",AttachLoc);
      printdiffnum("Public  Upload Sort Type",PubUpldSort);
      printdiffstr("Public  Upld DIR",UpldDir);
      printdiffstr("Public  Uploads",PubUpldLoc);
      printdiffnum("Private Upload Sort Type",PrvUpldSort);
      printdiffstr("Private Upld DIR",PrivDir);
      printdiffstr("Private Uploads",PrvUpldLoc);
      printdiffstr("Doors Menu",DrsMenu);
      printdiffstr("Doors Listing",DrsFile);
      printdiffstr("Bulletins Menu",BltMenu);
      printdiffstr("Bulletins Listing",BltNameLoc);
      printdiffstr("Scripts Menu",ScrMenu);
      printdiffstr("Scripts Listing",ScrNameLoc);
      printdiffstr("DIR Menu",DirMenu);
      printdiffstr("DIR Listing",DirNameLoc);
      printdiffstr("Download Path Listing",PthNameLoc);

      printdifflog("Auto Rejoin",AutoRejoin);
      printdifflog("Allow Viewing Conf Members",ViewMembers);
      printdifflog("Make All Uploads Private",PrivUplds);
      printdifflog("Make all Messages Private",PrivMsgs);
      printdifflog("Echo Mail in Conference",EchoMail);
      printdifflog("Force Echo",ForceEcho);
      printdiffnum("Type of NetMail",ConfType);
      printdifflog("Allow Long TO: Names",LongToNames);
      printdifflog("Make Conference Read-Only",ReadOnly);
      printdifflog("Disallow Private",NoPrivateMsgs);
      printdifflog("Place ORIGIN in Msg",RecordOrigin);
      printdifflog("Prompt for ROUTE Info",PromptForRouting);
      printdifflog("Allow Aliases",AllowAliases);
      printdifflog("Show INTRO during R;A",ShowIntroOnRA);
      printdifflog("Maintain Old Index",OldIndex);
      printdiffstr("Conf-Specific CMD.LST",CmdLst);
      printdiffnum("Additional Conf Security",AddSec);
      printdiffnum("Additional Conf Time",AddTime);
      printdiffnum("Level for Attachments",AttachLevel);
      printdiffnum("Level to Enter a Msg",ReqLevelToEnter);
      printdiffnum("Level for Ret Receipt",RetReceiptLevel);
      printdiffnum("Level for Carbon List",CarbonLevel);
      printdiffnum("Carbon List Limit",CarbonLimit);
      printdiffval("Charge Per Minute",ChargeTime);
      printdiffval("Charge Per Msg Read",ChargeMsgRead);
      printdiffval("Charge Per Msg Written",ChargeMsgWrite);

      OldConf = Conf;
    }
  }

  #ifdef DEBUG
    mc_unregister(Str);
  #endif
}


/********************************************************************
*
*  Function:  openfilename()
*
*  Desc    :  1- Checks to see if the Name is blank, if so prints an error
*                message and returns
*             2- Checks to see if the file has already been printed, if so it
*                indicates which conference the file was in and returns
*             3- Otherwise it saves the name, description and conference
*
*  Returns :  -1 on error or duplication else 0 (in which case *File is
*             filled with the file information)
*/

int  openfilename(char *Name, char *Desc, char ConfNum, DOSFILE *File) {
  filelisttype *p;
  char         Temp[256];
  pcbconftype  Conf;

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  if (Name[0] == 0) {
    sprintf(Temp,"\r\n(%s is blank)\r\n",Desc);
    sendtoprinter(Temp);
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  strupr(Name);
  stripright(Name,' ');

  if (FileList != NULL) {
    for (p = FileList; p < &FileList[NumFiles]; p++) {
      if (strcmp(p->FileName,Name) == 0) {
        getconfrecord(p->ConfNum,&Conf);
        sprintf(Temp,"%s was printed already in (%d) %-60.60s",
                Name,p->ConfNum,Conf.Name);
        stripright(Temp,' ');
        strcat(Temp,"\r\n");
        sendtoprinter(Temp);
        #ifdef DEBUG
          mc_unregister(Temp);
        #endif
        return(-1);
      }
    }
  }

  if (fileexist(Name) == 255) {
    sprintf(Temp,"\r\n%s is missing!\r\n",Name);
    sendtoprinter(Temp);
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  if (dosfopen(Name,OPEN_READ|OPEN_DENYNONE,File) == -1) {
    sprintf(Temp,"\r\nunable to open %s!\r\n",Name);
    sendtoprinter(Temp);
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return(-1);
  }

  if (FileList != NULL && NumFiles < MAXFILES) {
    strcpy(p->FileName,Name);
    strcpy(p->Desc,Desc);
    p->ConfNum = ConfNum;
    NumFiles++;
  }

  clsbox(18,20,77,20,Colors[OUTBOX]);
  fastprint(18,20,Desc,Colors[STATUS]);
  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
  return(0);
}


void  freefilelist(void) {
  if (FileList != NULL) {
    free(FileList);
    FileList = NULL;
  }
}


void  resetfilenames(void) {
  if (FileList == NULL)
    if ((FileList = (filelisttype *) malloc(sizeof(filelisttype)*MAXFILES)) == NULL)
      return;
  memset(FileList,0,sizeof(filelisttype)*MAXFILES);
  NumFiles = 0;
}


/********************************************************************
*
*  Function: printtextfile()
*
*  Desc    : Prints the text file to the printer as specified in Path and Name
*/

int  printtextfile(char *Name, char *Desc, char ConfNum) {
  DOSFILE File;
  char    Buf[256];

  #ifdef DEBUG
    mc_register(Buf,sizeof(Buf));
  #endif

  if (openfilename(Name,Desc,ConfNum,&File) != -1) {
    printtop(Name);
    while(dosfgets(Buf,sizeof(Buf),&File) == 0) {
      if (sendtoprinter(Buf) == -1 || sendtoprinter("\r\n") == -1) {
        dosfclose(&File);
        #ifdef DEBUG
          mc_unregister(Buf);
        #endif
        return(-1);
      }
    }
    dosfclose(&File);
  }

  #ifdef DEBUG
    mc_unregister(Buf);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: printblt()
*
*  Desc    : Prints the BLT.LST file to the printer
*/

void  printblt(char *FileName, char ConfNum) {
  char    Name[31];
  DOSFILE File;

  if (openfilename(FileName,"BLT LIST FILE",ConfNum,&File) != -1) {
    printtop(FileName);
    while(dosfread(Name,30,&File) > 0) {
      Name[30] = 0;
      stripright(Name,26);
      if (sendtoprinter(Name) == -1 || sendtoprinter("\r\n") == -1)
        break;
    }
    dosfclose(&File);
  }
}


/********************************************************************
*
*  Function: printscr()
*
*  Desc    : Prints the SCRIPT.LST file to the printer
*/

void  printscr(char *FileName, char ConfNum) {
  char    Name1[31];
  char    Name2[31];
  DOSFILE File;
  char    Temp[100];

  if (openfilename(FileName,"SCRIPT LIST FILE",ConfNum,&File) != -1) {
    printtop(FileName);
    Name1[30] = 0;
    Name2[30] = 0;
    while(dosfread(Name1,30,&File) > 0) {
      if (dosfread(Name2,30,&File) > 0) {
        sprintf(Temp,"%-30.30s  %s\r\n",Name1,Name2);
        if (sendtoprinter(Temp) == -1)
          break;
      } else {
        if (sendtoprinter(Name1) == -1 || sendtoprinter("\r\n") == -1)
          break;
        break;
      }
    }
    dosfclose(&File);
  }
}


/********************************************************************
*
*  Function: printdir()
*
*  Desc    : Prints the DIR.LST file to the printer
*/

void  printdir(char *FileName, char ConfNum) {
  DirListType2 Rec;
  DirListType  Print;
  DOSFILE      File;
  char         Temp[200];

  if (openfilename(FileName,"DIR LIST FILE",ConfNum,&File) != -1) {
    printtop(FileName);
    Print.DirPath[30] = 0;
    Print.DskPath[30] = 0;
    Print.DirDesc[35] = 0;
    while(dosfread(&Rec,sizeof(DirListType2),&File) > 0) {
      memcpy(Print.DirPath,Rec.DirPath,30); stripright(Print.DirPath,' ');
      memcpy(Print.DskPath,Rec.DskPath,30); stripright(Print.DskPath,' ');
      memcpy(Print.DirDesc,Rec.DirDesc,35); stripright(Print.DirDesc,' ');
      sprintf(Temp,"%-30.30s %-30.30s %d %s\r\n",Print.DirPath,Print.DskPath,Rec.SortType,Print.DirDesc);
      if (sendtoprinter(Temp) == -1)
        break;
    }
    dosfclose(&File);
  }
}


/********************************************************************
*
*  Function: printdoorfiles()
*
*  Desc    : Prints each of the files specified in the DOORS.DAT file
*/

void  printdoorfiles(char *FileName, char ConfNum) {
  DOSFILE File;
  char    *p;
  char    Temp[80];

  if (openfilename(FileName,"DOOR FILES",ConfNum,&File) != -1) {
    while (dosfgets(Temp,80,&File) != -1) {
      stripright(Temp,' ');
      if ((p = strchr(Temp,',')) != NULL) {
        *p = 0;
        if (printtextfile(Temp,"DOOR FILE",ConfNum) == -1)
          break;
      }
    }
    dosfclose(&File);
  }
}


/********************************************************************
*
*  Function: printeventfiles()
*
*  Desc    : Prints the EVENT.DAT file and each of the EVENT files
*/

int  printeventbatfiles(char *Name) {
  char         *p;
  struct ffblk ffblk;
  char         Mask[66];
  char         FileName[66];

  sprintf(Mask,"%s%s.*",PcbData.EventFiles,Name,".*");

  if (findfirst(Mask,&ffblk,0) != -1) {
    do {
      if ((p = strrchr(ffblk.ff_name,'.')) == NULL || atoi(p+1) > 0) {
        sprintf(FileName,"%s%s",PcbData.EventFiles,ffblk.ff_name);
        if (printtextfile(FileName,"EVENT FILE",0) == -1)
          return(-1);
      }
    } while (findnext(&ffblk) != -1);
  }

  return(0);
}


void  printeventfiles(void) {
  int       MaxEvents;
  int       EventCount;
  int       DayCount;
  char      ActiveEventList[MAXACTIVEEVENTSIZE];
  DOSFILE   File;
  EventType Buffer;
  char      Days[8];
  char      Temp[128];

  #ifdef DEBUG
    mc_register(Temp,sizeof(Temp));
  #endif

  if (openfilename(PcbData.EventDatFile,"EVENT.DAT FILE",0,&File) == -1) {
    #ifdef DEBUG
      mc_unregister(Temp);
    #endif
    return;
  }

  printtop(PcbData.EventDatFile);

  if (dosfread(&MaxEvents, MAXEVENTSIZE, &File) != MAXEVENTSIZE)
    goto end;

  if (dosfread(ActiveEventList,ACTIVEEVENTSIZE, &File) != ACTIVEEVENTSIZE)
    goto end;

  for (EventCount = 0, Days[7] = 0; EventCount < MaxEvents; EventCount++) {
    if (dosfread(&Buffer, sizeof(Buffer), &File) != sizeof(Buffer))
      break;

    for (DayCount = 0; DayCount < 7; DayCount++)
      Days[DayCount] = (isset(&Buffer.Days,DayCount) ? 'Y' : 'N');

    if (Buffer.BatchFile[0] > ' ') {
      sprintf(Temp,"%c %c %-8.8s %-5.5s %-5.5s %s %-8.8s\r\n",
                   (isset(ActiveEventList,EventCount) ? 'Y' : 'N'),
                   Buffer.Mode,
                   Buffer.BatchFile,
                   Buffer.BeginTime,
                   Buffer.EndTime,
                   Days,
                   Buffer.Date);
      if (sendtoprinter(Temp) == -1)
        goto end;
    }
  }

  dosfseek(&File,EVENTRECOFFSET,SEEK_SET);

  for (EventCount = 0; EventCount < MaxEvents; EventCount++) {
    if (dosfread(&Buffer, sizeof(Buffer), &File) != sizeof(Buffer))
      break;
    if (isset(ActiveEventList,EventCount)) {
      memcpy(Temp,Buffer.BatchFile,8);
      Temp[8] = 0;
      stripright(Temp,' ');
      if (printeventbatfiles(Temp) == -1)
        break;
    }
  }

end:
  dosfclose(&File);

  #ifdef DEBUG
    mc_unregister(Temp);
  #endif
}


/********************************************************************
*
*  Function: printconfig()
*
*  Desc    : Prints each of the PCBoard configuration files
*
*/

void  printconfig(void) {
  unsigned X;
  char Line[100];
  char Str[128];
  char Tmp[200];

  #ifdef DEBUG
    mc_register(Tmp,sizeof(Tmp));
    mc_register(Str,sizeof(Str));
    mc_register(Line,sizeof(Line));
  #endif

  fastprint(18,20,"pcboard.dat",Colors[STATUS]);   printdatafile();
  fastprint(18,20,"conferences",Colors[STATUS]);   printconffiles();

  if ((FileList = (filelisttype *) malloc(sizeof(filelisttype)*MAXFILES)) == NULL) {
    #ifdef DEBUG
      mc_unregister(Tmp);
      mc_unregister(Str);
      mc_unregister(Line);
    #endif
    return;
  }

  memset(&OldConf,0xFF,sizeof(pcbconftype));

  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&Conf);
    if (X == 0)
      strcpy(Str,"Main Board Configuration Files");
    else
      sprintf(Str,"%d) %s Conference Configuration Files:",X,Conf.Name);
    memset(Line,'=',strlen(Str));
    Line[strlen(Str)] = 0;
    sprintf(Tmp,"\r\n\r\n%s\r\n%s\r\n",Str,Line);
    sendtoprinter(Tmp);
    if (strcmp(Conf.DrsFile,OldConf.DrsFile) != 0)
      printtextfile(Conf.DrsFile,"DOORS LIST FILE",X);

    if (strcmp(Conf.BltNameLoc,OldConf.BltNameLoc) != 0)
      printblt(Conf.BltNameLoc,X);

    if (strcmp(Conf.ScrNameLoc,OldConf.ScrNameLoc) != 0)
      printscr(Conf.ScrNameLoc,X);

    if (strcmp(Conf.DirNameLoc,OldConf.DirNameLoc) != 0)
      printdir(Conf.DirNameLoc,X);

    if (strcmp(Conf.PthNameLoc,OldConf.PthNameLoc) != 0)
      printtextfile(Conf.PthNameLoc,"DNLD PATH LIST FILE",X);

    if (strcmp(Conf.DrsFile,OldConf.DrsFile) != 0)
      printdoorfiles(Conf.DrsFile,X);

    OldConf = Conf;
  }
  sendtoprinter("\r\n");

  printtextfile(PcbData.FscFile   ,"FSEC",0);
  printtextfile(PcbData.UscFile   ,"USEC",0);
  printtextfile(PcbData.PwdFile   ,"PWRD",0);
  printtextfile(PcbData.TrnFile   ,"PCBPROT.DAT",0);
  printtextfile(RemoteFile        ,"REMOTE.SYS",0);
  printeventfiles();
  printtextfile(CompressFile      ,"PCBCMPRS.BAT",0);
  printtextfile(QwkCompressFile   ,"PCBQWK.BAT",0);
  printtextfile("C:\\CONFIG.SYS"  ,"",0);
  printtextfile("C:\\AUTOEXEC.BAT","",0);
  printtextfile(BoardBatFile      ,"BOARD.BAT",0);
  freefilelist();

  #ifdef DEBUG
    mc_unregister(Tmp);
    mc_unregister(Str);
    mc_unregister(Line);
  #endif
}
