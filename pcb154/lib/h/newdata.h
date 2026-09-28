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


#ifndef H_NEWDATA
#define H_NEWDATA

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

enum {SEC_A=0,SEC_B,SEC_C,SEC_D,SEC_E,SEC_F,SEC_G,SEC_H,SEC_I,SEC_J,SEC_K,
SEC_L,SEC_M,SEC_N,SEC_O,SEC_P,SEC_Q,SEC_R,SEC_S,SEC_T,SEC_U,SEC_V,SEC_W,SEC_X,
SEC_Y,SEC_Z,SEC_OPEN,SEC_CHAT,SEC_REG,SEC_NONREG,SEC_USER,SEC_WHO,SEC_BATCH,
SEC_EDIT,SEC_TEST,USERSEC };

enum {SEC_READCOMMENTS=0,SEC_READALLMAIL,SEC_SYSOPLEVEL,SEC_MOVEMSG,SEC_1,
SEC_2,SEC_3,SEC_4,SEC_5,SEC_6,SEC_7,SEC_8,SEC_9,SEC_10,SEC_11,SEC_12,SEC_13,
SEC_14,SEC_SUBS,SEC_EDITALL,SEC_READONLY,SEC_15,SEC_BROADCAST,SEC_VIEWPRIV,
SEC_GENERICMSGS,SEC_EDITHDR,SEC_PROTECT,SEC_OVERWRITE,SEC_UNUSED,SEC_KEEPMSG,
SEC_SEERETRCPT,SYSOPSEC };

#pragma pack(4)
typedef struct {
  char Version[52];        /* pcboard version                                */

  char Sysop[16];          /* sysop display name                             */
  char Password[14];       /* sysop local password                           */
  bool UseRealName;        /* true if use sysop real name instead of 'SYSOP' */
  bool Graphics;           /* graphics in local mode                         */
  short SysopSec[SYSOPSEC]; /* sysop function security levels                */ /* 2 new level for v14.5 */

  char HlpLoc[32];         /* location of help files                         */
  char SecLoc[32];         /* location of security message files             */
  char ChtLoc[32];         /* location of chat files                         */
  char TxtLoc[32];         /* location of pcbtext files                      */
  char NdxLoc[32];         /* location of index files                        */
  char TmpLoc[32];         /* location of temporary files                    */ /* v14.5 */
  char UsrFile[32];        /* name of users file                             */
  char InfFile[32];        /* name of users info file                        */ /* v14.5 */
  char ClrFile[32];        /* name and location of callers file              */
  char CnfFile[32];        /* name and location of conference data file      */
  char PwdFile[32];        /* name and location of pwrd file                 */
  char FscFile[32];        /* name and location of fsec file                 */
  char UscFile[32];        /* name and location of upsec file                */
  char TcnFile[32];        /* name and location of tcan file                 */
  char WlcFile[32];        /* name and location of welcome file              */
  char NewFile[32];        /* name and location of newuser file              */
  char ClsFile[32];        /* name and location of closed file               */
  char WrnFile[32];        /* name and location of warning file              */
  char ExpFile[32];        /* name and location of expired file              */

  char NetFile[32];        /* name and location of USERNET.DAT file          */
  char CnfMenu[32];        /* name and location of conference join menu      */
  char RegFile[32];        /* name and location of newreg file questions     */
  char AnsFile[32];        /* name and location of non-reg user's answer file*/
  char TrnFile[32];        /* name and location of protocol data file        */
  char DldFile[32];        /* name and location of download summary file     */
  char LogOffScr[32];      /* name and loc of logoff script questionnaire    */
  char LogOffAns[32];      /* name and loc of logoff script answers          */
  char MultiLang[32];      /* name and loc of pcbml.dat file                 */
  char GroupChat[32];      /* name and loc of group chat topic/intro file    */
  char ColorFile[32];      /* name and loc of PCBFILER.DEF file              */ /* v14.5 */

  unsigned char Seconds;   /* seconds to wait for connect                    */
  char ModemPort[8];       /* modem comm port (COM1:, COM2: or NONE)         */
  long ModemSpeed;         /* modem baud rate                                */
  bool LockSpeed;          /* lock in the modem port speed                   */
  char ModemInit[42];      /* modem initialization string                    */
  char ModemInit2[42];     /* modem initialization string #2                 */
  char ModemOff[42];       /* modem off-hook string                          */
  char ModemAns[42];       /* modem answer string                            */
  char ModemDial[42];      /* modem dial string                              */
  short NumRedials;        /* # of redials before aborting dialing sequence  */
  short MaxTries;          /* max # of attempts to connect                   */
  bool DisableCTS;         /* disable CTS/RTS checking                       */
  bool Hayes9600;          /* using a Hayes 9600                             */
  bool FastComm;           /* using a FastComm                               */
  bool ResetModem;         /* perform modem reset during recycle             */
  bool OffHook;            /* take modem off-hook during recycle             */
  bool Packet;             /* running under a packet switch network          */
  bool AnswerRing;         /* answer on true ring detect                     */
  bool Allow7E1;           /* allow 7,E,1 callers                            */
  bool AllowLowBaud;       /* allow low baud callers                         */
  char AllowLowStrt[6];    /* start time of low baud                         */
  char AllowLowStop[6];    /* stop time of low baud                          */
  long LowBaudLimit;       /* highest low baudrate denied                    */ /* v14.5 */
  short ModemDelay;        /* modem delay time                               */ /* v14.5 */
  char RingsRequired;      /* number of rings required before picking up     */ /* v14.5 */
  char IrqNum;             /* IRQ number for non-standard comm ports         */ /* v14.5 */
  short BaseAddress;       /* BASE address for non-standard comm ports       */ /* v14.5 */
  bool LeaveDtrUp;         /* leave DTR up on exit to DOS                    */ /* v14.5 */
  bool ShareIRQs;          /* share IRQs on MCA buses                        */ /* v14.5 */

  bool DisableScan;        /* disable message base scan prompt               */
  bool DisableCls;         /* disable screen clear after 3 minutes           */
  bool DisableEdits;       /* disable registration edits                     */
  bool DisableFilter;      /* disable high order ascii filter                */
  bool DisableQuick;       /* disable quick logon                            */
  bool DisablePassword;    /* disable password check on return from DOS      */
  bool MultiLingual;       /* support multi-lingual operation                */
  bool AllowPwrdOnly;      /* allow only password changes                    */
  bool ClosedBoard;        /* run system as a closed board                   */
  bool NonGraphics;        /* run in non-graphics mode only                  */
  bool ExitToDos;          /* exit to dos after each caller                  */
  bool IncludeCity;        /* include user's city/state in node chat display */
  bool EliminateSnow;      /* eliminate screen snow                          */
  bool SubscriptMode;      /* run in subscription mode                       */
  bool AllowEscCodes;      /* allow ESC codes in messages                    */
  bool AllowCCs;           /* allow CC: messages                             */
  bool Validate;           /* valide TO: name                                */
  bool LastReadUpdate;     /* update Last Read pointer afer new msg          */
  bool EnforceTime;        /* enforce daily time limit                       */
  char DisplayNews;        /* display news only if changed since last call   */
  bool DisableCTSdrop;     /* disable CTS drop during disk writes            */
  char KbdTimeout;         /* num of minutes for keyboard timeout            */
  short MaxScrollBack;     /* maximum lines in scrollback buffer             */ /* v14.5 */
  bool FastText;           /* pre-load pcbtext file                          */ /* v14.5 */
  bool FastCnames;         /* pre-load cnames file                           */ /* v14.5 */
  bool UploadBy;           /* include Upload By: in upload description       */ /* v14.5 */
  bool TestUploads;        /* test files after upload                        */
  bool ShowSettings;       /* include settings display on login              */ /* v14.5 */
  bool AllowShell;         /* allow shelling to DOS                          */ /* v14.5 */
  bool Slaves;             /* use Slave Card updating                        */ /* v14.5 */
  short SubscriptionDays;  /* default days in new subscription period        */ /* v14.5 */
  short SubscriptWarning;  /* days prior to subscription expiration          */ /* v14.5 */

  bool Network;            /* running under a network                        */
  short NodeNum;           /* node number of this node                       */
  char NetTimeout;         /* network timeout delay (20-99 seconds)          */
  char ChatDelay;          /* node chat delay inveral when idle (5-99 sec)   */

  char BoardName[66];      /* name of board                                  */
  char CapFile[9];         /* name of message capture file                   */
  char ViewBatch[32];      /* name and loc of batch file to view archives    */
  char ViewExt[5];         /* archive extension                              */
  char FuncKeys[10][62];   /* function key definitions                       */

/*char PubConf[42];*/      /* public conference string                       */
  unsigned short NumConf;  /* number of conferences (1 thru 65534)           */
  unsigned short NumAreas; /* number of conference areas (Main=1 thru 65535) */
  short MaxMsgLines;       /* maximum message lines allowed (1-400)          */
  char DefaultColor;       /* default foreground color                       */
  char DefaultIntensity;   /* default intensity (0 or 1)                     */
  bool EventActive;        /* timed event active                             */
  char EventTime[8];       /* time to activate event '00:00'                 */
  char EventSuspend;       /* minutes to suspend board activity before event */
  bool EventStopUplds;     /* stop uploads initiation during suspense time   */
  bool EventSlide;         /* slide event if user online                     */
  char UploadBufSize;      /* upload buffer size (4-32)                      */
  bool DisableDriveCheck;  /* disable upload drive size check                */
  char ParallelPortNum;    /* parallel port number (1-3 or 0)                */
  unsigned StopFreeSpace;  /* stop uploads when free space < #K              */
  unsigned MaxTotalMsgs;       /* total maximum messages in a capture file   */ /* v14.5 */
  unsigned MaxConfMsgs;        /* maximum messages per conference in capture */ /* v14.5 */
  bool QuickScan;          /* default to 'Y Q' message scanning              */ /* v14.5 */
  bool ScanAll;            /* default to 'Y A' message scanning              */ /* v14.5 */
  char MinPriorToEvent;    /* stop upload minutes prior to an event          */ /* v14.5 */
  bool UseNewAskFile;      /* use NEWASK plus built-in questions             */ /* v14.5 */
  bool AllowOneName;       /* allow one-name users to log into system        */ /* v14.5 */
  char DefExpiredLevel;    /* default expired security level                 */ /* v14.5 */
  bool StopClockOnCap;     /* stop the clock during capture file download    */ /* v14.5 */
  char SysopStart[6];      /* start time to allow sysop page                 */ /* v14.5 */
  char SysopStop[6];       /* stop  time to allow sysop page                 */ /* v14.5 */
  bool LogCallerNumber;    /* log caller number to disk                      */ /* v14.5 */
  bool LogConnectStr;      /* log connect string to disk                     */ /* v14.5 */
  bool LogSecLevel;        /* log security level to disk                     */ /* v14.5 */
  bool ConfPwrdAdjust;     /* re-read PWRD upon joining a conference         */ /* v14.5 */
  bool ConfirmCaller;      /* confirm caller name                            */ /* v14.5 */
  bool AllowPwrdComment;   /* allow password failure comment                 */ /* v14.5 */
  bool GuardLogoff;        /* guard logoff command                           */ /* v14.5 */
  char NumDescLines;       /* number of upload description lines             */ /* v14.5 */

  char UserLevels[USERSEC];/* user security levels                           */ /* v15.0 */

  char ChatFile[32];       /* name and location of chat.dat                  */ /* v15.0 */
  char StatsFile[33];      /* name and location of pcbstats.dat              */ /* v15.0 */
  char ChatMenu[33];       /* name and location of CHAT menu                 */ /* v15.0 */
  char NoAnsi[33];         /* name and location of NOANSI file               */ /* v15.0 */
  char SlowDrives[27];     /* drive letters for slow drives                  */ /* v15.0 */
  char SwapPath[33];       /* path to swap file                              */ /* v15.0 */
  bool Swap;               /* swap out during normal shell?                  */ /* v15.0 */
  bool NoBatchUp;          /* don't allow batch uploads / force first name   */ /* v15.0 */
  bool ForceMain;          /* force comments to main board                   */ /* v15.0 */
  bool Foreign;            /* LineSeparator and tilde changes                */ /* v15.0 */
  bool MonitorModem;       /* watch for incompleted connections              */ /* v15.0 */
  bool No16550;            /* force 16550 to be treated as a 16450           */ /* v15.0 */
  bool Force16550A;        /* force 16450 to be treated as a 16550           */ /* v15.0 */
  bool AutoReset;          /* reset modem every 5 minutes while waiting      */ /* v15.0 */
  bool Float;              /* float node number                              */ /* v15.0 */
  bool PromoteBatch;       /* promote D and U commands to DB and UB          */ /* v15.0 */
  bool AutoRegConf;        /* automatically register new user in all confs   */ /* v15.0 */
  bool ScanBlts;           /* scan bulletins when joining a conference       */ /* v15.0 */
  bool AutoMakeMsgs;       /* create msgs file if missing                    */ /* v15.0 */
  bool VerifyCDLoss;       /* verify cd loss before recycling                */ /* v15.0 */
  bool Encrypt;            /* encrypt users file                             */ /* v15.0 */
  short UploadCredit;      /* time credit (times 10, i.e. 25 = 2.5 credit)   */ /* v15.0 */
  short ByteCredit;        /* byte credit (times 10, i.e. 25 = 2.5 credit)   */ /* v15.0 */
  char NewDefaultColor;    /* color code for default color                   */ /* v15.0 */
  char MsgHdr1Color;       /* color for DATE line of message header          */ /* v15.0 */
  char MsgHdr2Color;       /* color for TO   line of message header          */ /* v15.0 */
  char MsgHdr3Color;       /* color for FROM line of message header          */ /* v15.0 */
  char MsgHdr4Color;       /* color for SUBJ line of message header          */ /* v15.0 */
  char MsgHdr5Color;       /* color for READ line of message header          */ /* v15.0 */
  char MsgHdr6Color;       /* color for CONF line of message header          */ /* v15.0 */
  char Origin[61];         /* ORIGIN information                             */ /* v15.0 */
  char EventDatFile[33];   /* name and location of EVENT.DAT                 */ /* v15.0 */
  char EventFiles[33];     /* location of event files                        */ /* v15.0 */
  char CmdLst[32];         /* name/location of CMD.LST to use for defaults   */ /* v15.0 */
  bool RequirePwrdToExit;  /* Require Local Password to drop PCBoard to DOS  */ /* v15.0 */
  short EnvSize;           /* requested size for DOS's environment           */ /* v15.0 */
  char LowBaudSecOverride; /* Security Level to override low baud limit      */ /* v15.0 */
  char AllFilesList[33];   /* name and location of all files DLPATH.LST      */ /* v15.0 */
  bool EnableFido;         /* enable FIDO setup and processing               */ /* v15.0 */
  bool FidoProcessIn;      /* TRUE allows the node to process incoming PKT   */
  bool FidoProcessOrphan;  /* TRUE allows the node to process orphan PKT     */
  bool FidoProcessOut;     /* TRUE allows the node to export mail            */
  bool FidoDialOut;        /* TRUE allows the node to dial out               */
  short FidoPktFreq;       /* Minutes between processing incoming PKT        */
  short FidoExportFreq;    /* Minutes between checking to export mail        */
  short FidoMailFreq;      /* Minutes between checking for need to dial out  */
  short FidoDefaultZone;
  short FidoDefaultNet;
  char FidoConfig[33];     /* FIDO import/export configuration file          */ /* v15.0 */ /* no longer used as of v15.22 */
  char FidoQueue[33];      /* FIDO queue filename                            */ /* v15.0 */ /* no longer used as of v15.22 */
  bool PromptToReadMail;   /* ask to read mail waiting                       */ /* v15.0 */
  char LogOnScr[32];       /* name and loc of logon script questionnaire     */ /* v15.0 */
  char LogOnAns[32];       /* name and loc of logon script answers           */ /* v15.0 */
  char QwkFile[9];         /* name of qwk packet file                        */ /* v15.0 */
  char FileTcan[32];       /* name of File Trashcan for stopping uploads     */ /* v15.0 */
  bool SwapDuringBat;      /* TRUE if $$logon or $$logoff.bat should swap    */ /* v15.0 */
  bool UserSysDuringBat;   /* TRUE if users.sys should be made for $$logon   */ /* v15.0 */
  bool DefaultGraphics;    /* TRUE to default to graphics mode on login      */ /* v15.0 */
  char SlowDriveBat[32];   /* batch file to run after slow-drive copy        */ /* v15.0 */
  bool AllowAliasChange;   /* TRUE if ALIAS can be changed after setting it  */ /* v15.0 */
  bool ExcludeLocals;      /* TRUE if local logins should be ignored         */ /* v15.0 */
  short PwrdUpdate;        /* Number of days PWRD is valid before expiring   */ /* v15.0 */
  short PwrdWarn;          /* Number of days prior to WARN of PWRD expiring  */ /* v15.0 */
  bool RecordGroupChat;    /* TRUE if group chat should be captured to disk  */ /* v15.0 */
  bool ShowAlias;          /* TRUE if sysop wants ALIAS shown in USERNET     */ /* v15.0 */
  char MinPwrdLen;         /* Minimum Password Length when PSA is installed  */ /* v15.0 */
  bool AllowHandles;       /* TRUE if handles are allowed in Group Chat      */ /* v15.0 */
  char CmdLoc[32];         /* location of command display files              */ /* v15.1 */
  bool ForceIntro;         /* TRUE if forcing INTRO to be displayed          */ /* v15.1 */
  bool SkipProtocol;       /* TRUE to skip protocol selection for new users  */ /* v15.1 */
  bool SkipAlias;          /* TRUE to skip alias selection for new users     */
  bool NoCarrierExit;      /* TRUE to exit to DOS if no carrier after connect*/ /* v15.1 */
  bool OS2Driver;          /* TRUE if set to use OS/2 COMM Driver            */

  bool EnableAccounting;   /* TRUE if accounting features are enabled        */ /* v15.2 */
  bool ShowCurrency;       /* TRUE if currency values should be shown        */ /* v15.2 */
  bool Concurrent;         /* TRUE if charges run concurrently               */ /* v15.2 */
  bool IgnoreDropSecLevel; /* TRUE if the DropSecLevel should not be used    */ /* v15.2 */
  char PeakStart[6];       /* Start of peak usage hours                      */ /* v15.2 */
  char PeakEnd[6];         /* End of peak usage hours                        */ /* v15.2 */
  char PeakDays;           /* bit encoded flags (bit0=sun, bit1=mon, etc     */ /* v15.2 */

  char HolidaysFile[32];   /* name/loc of Peak Holidays File                 */ /* v15.2 */
  char AccountConfig[32];  /* name/loc of Account Rate File                  */ /* v15.2 */
  char AccountInfo[32];    /* name/loc of Account Info File                  */ /* v15.2 */
  char AccountWarn[32];    /* name/loc of Account Warning File               */ /* v15.2 */
  char AccountTrack[32];   /* name/loc of Account Tracking File              */ /* v15.2 */
  char AccountLogoff[32];  /* name/loc of Account Logoff Info File           */ /* v15.2 */

  char uucpPath[33];       /* location for UUCP files                        */ /* v15.2 */
  char uucpSpoolPath[33];  /* location for UUCP Spool files                  */ /* v15.2 */
  char uucpLogPath[33];    /* location for UUCP Log files                    */ /* v15.2 */
  char CompBatFile[33];    /* name/loc of compress batch file for UUCP       */ /* v15.2 */
  char DeCompBatFile[33];  /* name/loc of decompress batch file for UUCP     */ /* v15.2 */
  char uucpName[17];       /* UUCP Name                                      */ /* v15.2 */
  char uucpDomainName[33]; /* UUCP Domain Name                               */ /* v15.2 */
  char uucpEmailHost[17];  /* UUCP Email Host Name                           */ /* v15.2 */
  char uucpNewsHost[17];   /* UUCP News Host Name                            */ /* v15.2 */
  char uucpDefDist[17];    /* UUCP Default News Distribution                 */ /* v15.2 */
  char uucpTimeZone[6];    /* Time Zone - offset from GMT                    */ /* v15.2 */
  char uucpSeparator[2];   /* UUCP Name Separator                            */ /* v15.2 */
  char Organization[65];   /* Organization Name                              */ /* v15.2 */
  char uucpHighAscii;      /* S = Strip, R = Replace, C = Convert            */ /* v15.2 */
  unsigned uucpEmailConf;  /* number of UUCP Email Conference                */ /* v15.2 */
  unsigned uucpJunkConf;   /* number of UUCP Junk Conference                 */ /* v15.2 */
  bool uucpBangDomain;     /* Bang Domain                                    */ /* v15.2 */
  bool uucpSubDomain;      /* Sub Domain                                     */ /* v15.2 */
  char uucpModFile[33];    /* List of UUCP Moderators                        */ /* v15.2 */
  bool FidoImportAfterXfer;/* TRUE if import takes place after file xfer     */ /* v15.21 */
  short FidoCrashSec;      /* security level required for crash mail         */ /* v15.21 */
  bool FidoLogLevel;       /* controls the amount of logging performed       */ /* v15.21 */
  bool FidoCreateMsg;      /* create MSG files                               */ /* v15.21 */
  bool FidoEnableRouting;  /* enable fido routing                            */ /* v15.21 */

  char FidoLoc[33];        /* FIDO configuration files location              */ /* v15.22 */
  bool FidoSecure;         /* Store msgs to unknown users in secure location */ /* v15.22 */
  bool FidoSysopChange;    /* Change sysop to FIDO_SYSOP on import           */ /* v15.22 */
  bool FidoCheckDupePath;  /* Check for dupes using message path             */ /* v15.22 */
  bool FidoCheckDupeMsgId; /* Check for dupes using msg id                   */ /* v15.22 */
  long FidoNumMsgsToTrack; /* Number of messages per conference to track     */ /* v15.22 */
  bool FidoMakeResponse;   /* Generate response messages                     */ /* v15.22 */
  bool FidoEnablePassThru; /* Enable pass throughs                           */ /* v15.22 */
  bool FidoEnableAreaFix;  /* Enable areafix forwarding                      */ /* v15.22 */
  bool FidoAutoAdd;        /* Automatically add fido areas                   */ /* v15.22 */
  bool FidoReAddress;      /* Re-Address Routed Packets                      */ /* v15.22 */
  bool FidoRouteEchoMail;  /* Route Echo Mail                                */ /* v15.22 */

                           /* 0=Nothing, 1=Minimize Self, 2=Minimize Shell   */
  ubyte MinimizeLogOnOff;  /* $$LOGON/LOGOFF batch file                      */ /* v15.22 */
  ubyte MinimizeProtocols; /* External Protocols                             */ /* v15.22 */
  ubyte MinimizeDoors;     /* Doors                                          */ /* v15.22 */
  ubyte MinimizeFileTest;  /* Upload verification process                    */ /* v15.22 */
  ubyte MinimizeFileView;  /* File View                                      */ /* v15.22 */
  ubyte MinimizeCompress;  /* PCBQWK and PCBCMPRS batch files                */ /* v15.22 */
  ubyte MinimizeShells;    /* All Other Shells                               */ /* v15.22 */
  short PriorityNormal;    /* Priority for regular processing                */ /* v15.22 */
  short PriorityProtocols; /* Priority for external protocols                */ /* v15.22 */
  short PriorityCompress;  /* Priority for PCBQWK and PCBCMPRS batch files   */ /* v15.22 */
  short PriorityShells;    /* Priority for all other shells                  */ /* v15.22 */
  short PriorityFidoIn;    /* Priority for fido import                       */ /* v15.22 */
  short PriorityFidoOut;   /* Priority for fido export                       */ /* v15.22 */

  char NetCopy[32];        /* command for performing a network file copy     */ /* v15.3  */
} pcbdattype;
#pragma pack()


/* this was the old CNAMES.@@@ structure - it is left unchanged in order to  */
/* maintain compatibility with other software that accesses CNAMES.@@@.      */

#pragma pack(1)
typedef struct {
  char Name[14];           /* conference name (SDR : Part 1 of 2)            */
  bool PublicConf;         /* new user default registration                  */
  bool AutoRejoin;         /* auto-rejoin for this conference                */
  bool ViewMembers;        /* allow viewing conference members               */
  bool PrivUplds;          /* make uploads private                           */
  bool PrivMsgs;           /* make all messages private                      */
  bool EchoMail;           /* is this an Echo-Mail conference                */
  short ReqSecLevel;        /* security required to join PUBLIC conference    */
  short  AddSec;             /* additional security level added after join     */
  short  AddTime;            /* additional time added after join               */
  char MsgBlocks;          /* number of message base index blocks            */
  char MsgFile[32];        /* name and location of messages file             */
  char UserMenu[32];       /* name and location of Users Menu                */
  char SysopMenu[32];      /* name and location of Sysops Menu               */
  char NewsFile[32];       /* name and location of Conference News           */
  char PubUpldSort;        /* sort type for public upload DIR file           */
  char UpldDir[29];        /* name and location of public upload DIR file    */
  char PubUpldLoc[26];     /* public upload location                         */
  char PrvUpldSort;        /* sort type for private upload DIR file          */
  char PrivDir[29];        /* name and location of private upload DIR file   */
  char PrvUpldLoc[26];     /* private upload location                        */
  char DrsMenu[29];        /* name and location of conference doors menu     */
  char DrsFile[33];        /* name and location of DOORS.LST file            */
  char BltMenu[29];        /* name and location of conference blt menu       */
  char BltNameLoc[33];     /* name and location of BLT.LST file              */
  char ScrMenu[29];        /* name and location of conference scripts menu   */
  char ScrNameLoc[33];     /* name and location of SCRIPT.LST                */
  char DirMenu[29];        /* name and location of directories menu          */
  char DirNameLoc[33];     /* name and location of DIR.LST file              */
  char PthNameLoc[33];     /* name and location of DLPATH.LST file           */
} oldconftype;
#pragma pack()

/* these are the new fields found inside of the CNAMES.ADD file */

#define ADDCONFUNUSED      64
#define N2_SIZE            48
#define ADDCONFRESERVE     (ADDCONFUNUSED+N2_SIZE) /* PCBSM skips FilebaseFlags+Name2   */
#pragma pack(1)
typedef struct {
  bool ForceEcho;          /* turn off echo question, force all msgs to echo */
  bool ReadOnly;           /* do not allow ANY msgs to be entered in conf    */
  bool NoPrivateMsgs;      /* do not allow PRIVATE msgs to be entered        */
  char RetReceiptLevel;    /* level required to request return receipts      */
  bool RecordOrigin;       /* Record ORIGIN in messages                      */
  bool PromptForRouting;   /* Prompt user for ROUTING information            */
  bool AllowAliases;       /* Allow aliases to be used                       */
  bool ShowIntroOnRA;      /* Show the Conf INTRO in the middle of R A scan  */
  char ReqLevelToEnter;    /* Security Level required to enter messages      */
  char Password[13];       /* password reqd to join if private               */
  char Intro[32];          /* name/location of conference INTRO file         */
  char AttachLoc[32];      /* location for file attachment storage           */
  char RegFlags[4];        /* RXS flags for automatic conf registration      */
  char AttachLevel;        /* level required to attach files to messages     */
  char CarbonLimit;        /* max number of names in carbon list             */
  char CmdLst[32];         /* name/location of CMD.LST use instead of default*/
  bool OldIndex;           /* maintain old MSGS indexes?                     */
  bool LongToNames;        /* allow long TO: names to be entered             */
  char CarbonLevel;        /* level required to enter @LIST@ messages        */
  char ConfType;           /* one byte code for conference type (see below)  */
  long ExportPtr;          /* a pointer to the last message number exported  */
  float ChargeTime;
  float ChargeMsgRead;
  float ChargeMsgWrite;
  char FilebaseFlags[ADDCONFUNUSED]; /* Per-filebase bit flags (pcb_conf_ext)  */
                                 /*   1 bit per filebase, 8 flags per filebase  */
                                 /*   FilebaseFlags[N/8] bit (N%8) = filebase N */
                                 /*   Bit 0: FBFLAG_ISO (ISO-backed filebase)   */
                                 /*   Bits 1-7: reserved for future flags       */
                                 /*   Supports up to 512 filebases/conference   */
                                 /*   Zero-initialized by PCBoard. Read by PCBISO*/
                                 /*   Works on PCBoard 15.4+                    */
  char Name2[N2_SIZE];     /* conf name (SDR : 2 of 2 : internet compat)     */
} addconftype;
#pragma pack()

    /**********************************************************************/
    /*                                                                    */
    /* The ConfType Field Explained                                       */
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                       */
    /* This field has five defined values; all others are reserved.       */
    /*                                                                    */
    /*     ConfType = 0 = Normal PCBoard Conference                       */
    /*     ConfType = 1 = Internet Email (Import & Export)                */
    /*     ConfType = 2 = Internet/Usenet Junk (Import Only)              */
    /*     ConfType = 3 = Usenet Moderated Newsgroup                      */
    /*                    (Import Normally /                              */
    /*                     Export Public Messages as Email to Moderator / */
    /*                     Export Private Messages as Email to TO:)       */
    /*     ConfType = 4 = Usenet Public Newsgroup                         */
    /*                    (Import Normally /                              */
    /*                     Export Public Messages as Articles /           */
    /*                     Export Private Messages as Email to TO:)       */
    /*                                                                    */
    /**********************************************************************/

#define ADDCONFSIZE   (sizeof(addconftype) - ADDCONFRESERVE)

/* this structure is a combination of both the oldconftype and addconftype */
/* minus the reserved portion of addconftype.  it is named pcbconftype so  */
/* that all of the previously existing high-level code can continue to use */
/* the pcbconftype while suddenly gaining access to the new fields         */

#pragma pack(1)
typedef struct {
  char Name[61];
  bool PublicConf;
  bool AutoRejoin;
  bool ViewMembers;
  bool PrivUplds;
  bool PrivMsgs;
  bool EchoMail;
  short ReqSecLevel;
  short AddSec;
  short AddTime;
  char MsgBlocks;
  char MsgFile[32];
  char UserMenu[32];
  char SysopMenu[32];
  char NewsFile[32];
  char PubUpldSort;
  char UpldDir[29];
  char PubUpldLoc[26];
  char PrvUpldSort;
  char PrivDir[29];
  char PrvUpldLoc[26];
  char DrsMenu[29];
  char DrsFile[33];
  char BltMenu[29];
  char BltNameLoc[33];
  char ScrMenu[29];
  char ScrNameLoc[33];
  char DirMenu[29];
  char DirNameLoc[33];
  char PthNameLoc[33];
  bool ForceEcho;
  bool ReadOnly;
  bool NoPrivateMsgs;
  char RetReceiptLevel;
  bool RecordOrigin;
  bool PromptForRouting;
  bool AllowAliases;
  bool ShowIntroOnRA;
  char ReqLevelToEnter;
  char Password[13];
  char Intro[32];
  char AttachLoc[32];
  char RegFlags[4];
  char AttachLevel;
  char CarbonLimit;
  char CmdLst[32];
  bool OldIndex;
  bool LongToNames;
  char CarbonLevel;
  char ConfType;
  long ExportPtr;
  float ChargeTime;
  float ChargeMsgRead;
  float ChargeMsgWrite;
} pcbconftype;
#pragma pack()


#if defined(__LARGE__) || defined(__COMPACT__)
extern pcbdattype far PcbData;
#else
extern pcbdattype PcbData;
#endif
extern bool CommDrv;

#endif /* ifndef H_NEWDATA  */
