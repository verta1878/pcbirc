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


#ifndef __PCBTOOLS_H
#define __PCBTOOLS_H

/* need to pull in TYPES.HPP */

#include "types.hpp"

/* #define bool  char */
/* #define TRUE  1    */
/* #define FALSE 0    */

#ifndef SEEK_CUR
#define SEEK_CUR        1
#define SEEK_END        2
#define SEEK_SET        0
#endif

/* use byte alignment for all structure in this header file */
#if defined _MSC_VER || defined __WATCOMC__
#pragma pack(1)
#elif __TURBOC__ >= 0x400
#pragma option -a-
#endif

#ifdef __OS2__
  /* based on 1/1000th of a second - used by settimer()/gettimer() */
  #define TENTHSECOND        100
  #define QUARTERSECOND      250
  #define HALFSECOND         500
  #define ONESECOND         1000
  #define TWOSECONDS        2000
  #define THREESECONDS      3000
  #define FOURSECONDS       4000
  #define FIVESECONDS       5000
  #define SIXSECONDS        6000
  #define TENSECONDS       10000
  #define TWENTYSECONDS    20000
  #define THIRTYSECONDS    30000
  #define SIXTYSECONDS     60000
  #define NINETYSECONDS    90000
  #define ONEMINUTE        60000
  #define TWOMINUTES      120000
  #define THREEMINUTES    180000
  #define FIVEMINUTES     300000
  #define TICKSPERSECOND    1000
#else
  /* based on 1/18.2 of a second - used by settimer()/gettimer() */
  #define TENTHSECOND       2
  #define QUARTERSECOND     4
  #define HALFSECOND        9
  #define ONESECOND        18
  #define TWOSECONDS       36
  #define THREESECONDS     54
  #define FOURSECONDS      73
  #define FIVESECONDS      91
  #define SIXSECONDS      109
  #define TENSECONDS      182
  #define TWENTYSECONDS   (2 * 182)
  #define THIRTYSECONDS   (3 * 182)
  #define SIXTYSECONDS    (6 * 182)
  #define NINETYSECONDS   (9 * 182)
  #define ONEMINUTE       (1 * 6 * 182)
  #define TWOMINUTES      (2 * 6 * 182)
  #define THREEMINUTES    (3 * 6 * 182)
  #define FIVEMINUTES     (5 * 6 * 182)
  #define TICKSPERSECOND  (182 / 10)
#endif

#define  OPEN_DENYRDWR 0x10     /* share mode: deny read/write */
#define  OPEN_DENYWRIT 0x20     /* share mode: deny write access */
#define  OPEN_DENYREAD 0x30     /* share mode: deny read access */
#define  OPEN_DENYNONE 0x40     /* share mode: deny none */
#define  OPEN_READ     0x00     /* open for read access */
#define  OPEN_WRIT     0x01     /* open for write access */
#define  OPEN_RDWR     0x02     /* open for read/write access */
#define  OPEN_APPEND   0x04     /* added to OPEN_WRIT, opens for APPEND */
#define  OPEN_CREATE   0x08     /* added to OPEN_WRIT. makes a NEW file */
#define  OPEN_ALREADY  0x100    /* used to dup an opened file handle */
#define  OPEN_NORMAL   0x00     /* create normal attribute file */
#define  OPEN_RDONLY   0x01     /* create read-only file */
#define  OPEN_HIDDEN   0x02     /* create hidden file */
#define  OPEN_SYSTEM   0x04     /* create system file */
#define  OPEN_VOLUME   0x08     /* create volume label */

#define CUR_BLANK   0
#define CUR_NORMAL  1
#define CUR_BLOCK   2
#define CUR_INSERT  3

#define CNF_USR      0
#define CNF_REG      ConfByteLen
#define CNF_EXP      ConfByteLen * 2
#define CNF_CON      ConfByteLen * 3
#define CNF_MFL      ConfByteLen * 4
#define CNF_NET      ConfByteLen * 5
#define CNF_JOINED   0
#define CNF_SCANNED  ConfByteLen

#define TRIGGER1   0
#define TRIGGER4   64
#define TRIGGER8   128
#define TRIGGER14  192

#ifndef PCBTOOLS_HDR
#define PCBTOOLS_HDR


enum {SEC_A=0,SEC_B,SEC_C,SEC_D,SEC_E,SEC_F,SEC_G,SEC_H,SEC_I,SEC_J,SEC_K,
SEC_L,SEC_M,SEC_N,SEC_O,SEC_P,SEC_Q,SEC_R,SEC_S,SEC_T,SEC_U,SEC_V,SEC_W,SEC_X,
SEC_Y,SEC_Z,SEC_OPEN,SEC_CHAT,SEC_REG,SEC_NONREG,SEC_USER,SEC_WHO,SEC_BATCH,
SEC_EDIT,SEC_TEST,USERSEC };

enum {SEC_READCOMMENTS=0,SEC_READALLMAIL,SEC_SYSOPLEVEL,SEC_MOVEMSG,SEC_1,
SEC_2,SEC_3,SEC_4,SEC_5,SEC_6,SEC_7,SEC_8,SEC_9,SEC_10,SEC_11,SEC_12,SEC_13,
SEC_14,SEC_SUBS,SEC_EDITALL,SEC_READONLY,SEC_15,SEC_BROADCAST,SEC_VIEWPRIV,
SEC_GENERICMSGS,SEC_EDITHDR,SEC_PROTECT,SEC_OVERWRITE,SEC_UNUSED,SEC_KEEPMSG,
SEC_SEERETRCPT,SYSOPSEC };

enum {PCB_BLUE=9, PCB_GREEN, PCB_CYAN, PCB_RED, PCB_MAGENTA, PCB_YELLOW, PCB_WHITE };
enum {NOHELP,HLP_MORE};

typedef enum {WATCHKBD=1, WATCHSESSION=2, READUSERSYS=4, SHOWSTATUSLINE=8, LMRS=16, CONFFLAGS=32, TPA=64, NOCLS=128} openstatus;
typedef enum {NONE, BLANKLINE, HELP, NAME, CONFREG, COMMENT, MODEMSTATS } statlinetype;
typedef enum {OFFLINE, LOCAL, REMOTE } onlinetype;
#if defined(H_SCRNIO) || defined(__WATCOMC__)
#define _DISPLAYTYPE_DEFINED
#ifdef __WATCOMC__
#pragma push_macro("DEFAULTS")
#pragma push_macro("WORDWRAP")
#undef DEFAULTS
#undef WORDWRAP
#ifdef __WATCOMC__
typedef enum {DEFAULTS=0,ECHODOTS=1,FIELDLEN=2,UPCASE=4,STACKED=8,ERASELINE=16,NEWLINE=32,LFBEFORE=64,LFAFTER=128,LOGIT=256,LOGITLEFT=512,GUIDE=1024,WORDWRAP=2048,_PCBYESNO=2049,NOCLEAR=4096,BELL=8192,HIGHASCII=16384,AUTO=32768,NOTBLANK=32768} displaytype;
#define YESNO _PCBYESNO
#else
typedef enum {DEFAULTS=0,ECHODOTS=1,FIELDLEN=2,UPCASE=4,STACKED=8,ERASELINE=16,NEWLINE=32,LFBEFORE=64,LFAFTER=128,LOGIT=256,LOGITLEFT=512,GUIDE=1024,WORDWRAP=2048,YESNO=2049,NOCLEAR=4096,BELL=8192,HIGHASCII=16384,AUTO=32768,NOTBLANK=32768} displaytype;
#endif
#ifdef __WATCOMC__
#pragma pop_macro("WORDWRAP")
#pragma pop_macro("DEFAULTS")
#endif
#endif
/* Some functions which take ENUM'd parameters and OR them together declare  */
/* their parameter types using the typedef'd name of the enum values.  I     */
/* like this, but Watcom does not.  These following lines of code help solve */
/* this problem and keep the compiler happy.                                 */
#if defined(__cplusplus) || defined(__WATCOMC__)
  #define SCANDLTYPE      int
  #define DISPLAYTYPE     int
  #define DISPLAYFILETYPE int
  #define VERIFYTYPE      int
#else
  #define SCANDLTYPE      scandltype
  #define DISPLAYTYPE     displaytype
  #define DISPLAYFILETYPE displayfiletype
  #define VERIFYTYPE      verifytype
#endif

#ifndef BASREAL
  #define BASREAL  1
#ifndef _BASSNGL_DEFINED
#define _BASSNGL_DEFINED
  typedef unsigned char bassngl[4];
#endif
  typedef unsigned char basdble[8];
#endif

#pragma pack(4)
#ifndef _DOSFILE_DEFINED
#define _DOSFILE_DEFINED
typedef struct {      /* struct used with dosfopen, dosfgets, & dosfclose */
  int      handle;    /* dos file handle */
  char     *buffer;   /* pointer to buffer space */
  unsigned bufsize;   /* size of the buffer associated with this file */
  unsigned offset;    /* offset of current pointer in buffer space */
  unsigned end;       /* last valid character in buffer space */
  char     status;    /* file status indicator */
  long     curpos;    /* current read/write position in the file */
  char     last_op;   /* last operation (OP_NONE, OP_READ, OP_WRIT) */
} DOSFILE;
#endif /* DOSFILE guard */
#pragma pack()

#pragma pack(4)
typedef struct {
  bool         PrintLog;               /* TRUE if the 'log' printer is turned on */
  bool         Alarm;                  /* TRUE if the alarm is toggled on */
  bool         ForceScreenOff;         /* TRUE if the screen was FORCED off by pressing F9 */
  bool         LoggingUserOff;         /* TRUE if we're in the process of logging the user off */
  bool         TimeAdjustedForEvent;   /* TRUE if the time online has been adjusted */
  bool         Kbd;                    /* TRUE if last char was input from the keyboard */
  bool         InChat;                 /* TRUE if we are currently in sysop chat */
  bool         Poff;                   /* TRUE if @POFF@ was issued */
  bool         KbdTimedOut;            /* TRUE if keyboard timer expired */
  bool         SessionTimedOut;        /* TRUE if session timer expired */
  bool         UseAlias;               /* TRUE if user wants to use his alias */
  bool         DisableColor;           /* TRUE if @X-code interpretation should be turned off */
  char         SysopFlag;              /* 'N' of Sysop Next, 'X' if Exit to DOS or ' ' */
  char         FirstName[26];          /* the first name of the caller */
  char         DisplayName[26];        /* real name or sysop name */
  char         LastDateOnStr[9];       /* from users file - last date on */
  statlinetype StatLine;               /* type of help displayed on status line */
  statlinetype OldStatLine;            /* previous value of StatLine */
  long         UserRecNo;              /* record number in the users file */
  short        SuspendMinute;          /* time when activity should be suspended */
  short        EventMinute;            /* time when event should be run */
  short        EndEventMinute;         /* last minute possible for event to run */
  short        LogonMinute;            /* time when the caller logged on in minutes since midnight */
  short        CreditMinutes;          /* time credit for uploads and sysop chat */
  unsigned short JulianLogonDate;      /* date when the caller logged on (julian) */
  char         LogonDate[9];           /* date when the caller logged on (MM/DD/YY) */
  char         LogonTime[9];           /* time when the caller logged on (HH:MM:SS) */
  unsigned short Conference;           /* current conference number */
  char         CurSecLevel;            /* current security level */
  bool         WatchForStopChar;       /* used to signal when StopChar is monitored */
  char         StatusLine1;            /* line number of top status line */
  char         StatusLine2;            /* line number of top status line */
  short        ConfAddSec;             /* added security from joining a conference */
  short        ConfAddTime;            /* added time from joining a conference */
  short        SysLimit;               /* caller's time limit from pcboard.sys */
  short        OverrideLimit;          /* SessionLimit passed to initdoor() */
  char         Language;               /* number of language chosen */
  char         MultiLangExt[5];        /* filename extension for multilingual operation */
  bool         AppendText;             /* indicate DisplayText should be appended */
  char         DisplayText[80];        /* special @-variable display text */
} statustype;
#pragma pack()

#pragma pack(4)
typedef struct {
  bool     Screen;            /* TRUE if the screen should be displayed */
  bool     GraphicsMode;      /* TRUE if caller requested graphics */
  bool     RipMode;           /* TRUE if caller requested RIP graphics */
  bool     WatchKbdClock;     /* FALSE if we want to ignore the KEYBOARD clock */
  bool     WatchSessionClock; /* FALSE if we want to ignore the SESSION clock */
  unsigned KbdTimer;          /* Number of clock ticks for keyboard timer */
  char     WarnMinute;        /* Minute to warn on prior to session timeout */
} controltype;
#pragma pack()

#pragma pack(4)
typedef struct {
  char       ComPortNumber;     /* e.g. COM1 = 1 */
  char       DataBits;          /* e.g. 8 or 7 */
  onlinetype Online;            /* type of online connection */
  long       CarrierSpeed;      /* modem to modem speed */
  long       ConnectSpeed;      /* connect speed displayed to caller */
  long       ModemSpeed;        /* pc to modem speed */
  bool       LostCarrier;       /* TRUE if carrier was lost */
  bool       IgnoreCDLoss;      /* TRUE if we need to process something after CD is lost */
  bool       ErrorCorrected;    /* TRUE if caller logged on in error corrected mode */
  bool       HstMode;           /* TRUE if we detected an HST modem */
  bool       Telnet;            /* TRUE if we detected a TELNET session */
} asytype;
#pragma pack()

#pragma pack(4)
typedef struct {
  bool         AbortPrintout;    /* TRUE if system should abort a printout */
  bool         CountLines;       /* TRUE if system is currently counting lines displayed */
  unsigned char NumLinesPrinted; /* a count of the number of lines displayed since last pause */
  bool         AutoMore;         /* TRUE if auto answer more? prompt after 10 seconds */
  bool         WasNonStop;       /* TRUE if last printout was in non-stop mode */
  bool         WasAborted;       /* TRUE if last printout was aborted */
  unsigned char PageLen;         /* caller's current page length */
  bool         ShowOnScreen;     /* TRUE if data should be shown on screen */
  bool         Break;            /* TRUE if current display can be interrupted at the More Prompt */
  char         DefaultColor;     /* default caller as chosen by the sysop */
  char         NumScrnLines;     /* number of lines on the local display screen */
} disptype;
#pragma pack()

#pragma pack(4)
typedef struct {
  char Version[52];        /* pcboard version */
  char Sysop[16];          /* sysop display name */
  char Password[14];       /* sysop local password */
  bool UseRealName;        /* true if use sysop real name instead of 'SYSOP' */
  bool Graphics;           /* graphics in local mode */
  short SysopSec[SYSOPSEC]; /* sysop function security levels */
  char HlpLoc[32];         /* location of help files */
  char SecLoc[32];         /* location of security message files */
  char ChtLoc[32];         /* location of chat files */
  char TxtLoc[32];         /* location of pcbtext files */
  char NdxLoc[32];         /* location of index files */
  char TmpLoc[32];         /* location of temporary files */
  char UsrFile[32];        /* name of users file */
  char InfFile[32];        /* name of users info file */
  char ClrFile[32];        /* name and location of callers file */
  char CnfFile[32];        /* name and location of conference data file */
  char PwdFile[32];        /* name and location of pwrd file */
  char FscFile[32];        /* name and location of fsec file */
  char UscFile[32];        /* name and location of upsec file */
  char TcnFile[32];        /* name and location of tcan file */
  char WlcFile[32];        /* name and location of welcome file */
  char NewFile[32];        /* name and location of newuser file */
  char ClsFile[32];        /* name and location of closed file */
  char WrnFile[32];        /* name and location of warning file */
  char ExpFile[32];        /* name and location of expired file */
  char NetFile[32];        /* name and location of USERNET.DAT file */
  char CnfMenu[32];        /* name and location of conference join menu */
  char RegFile[32];        /* name and location of newreg file questions */
  char AnsFile[32];        /* name and location of non-reg user's answer file*/
  char TrnFile[32];        /* name and location of protocol data file */
  char DldFile[32];        /* name and location of download summary file */
  char LogOffScr[32];      /* name and loc of logoff script questionnaire */
  char LogOffAns[32];      /* name and loc of logoff script answers */
  char MultiLang[32];      /* name and loc of pcbml.dat file */
  char GroupChat[32];      /* name and loc of group chat topic/intro file */
  char ColorFile[32];      /* name and loc of PCBFILER.DEF file */
  unsigned char Seconds;   /* seconds to wait for connect */
  char ModemPort[8];       /* modem comm port (COM1:, COM2: or NONE) */
  long ModemSpeed;         /* modem baud rate */
  bool LockSpeed;          /* lock in the modem port speed */
  char ModemInit[42];      /* modem initialization string */
  char ModemInit2[42];     /* modem initialization string #2 */
  char ModemOff[42];       /* modem off-hook string */
  char ModemAns[42];       /* modem answer string */
  char ModemDial[42];      /* modem dial string */
  short NumRedials;        /* # of redials before aborting dialing sequence */
  short MaxTries;          /* max # of attempts to connect */
  bool DisableCTS;         /* disable CTS/RTS checking */
  bool NotUsed;            /* no longer used - undefined */
  bool FastComm;           /* using a FastComm */
  bool ResetModem;         /* perform modem reset during recycle */
  bool OffHook;            /* take modem off-hook during recycle */
  bool Packet;             /* running under a packet switch network */
  bool AnswerRing;         /* answer on true ring detect */
  bool Allow7E1;           /* allow 7,E,1 callers */
  bool AllowLowBaud;       /* allow low baud callers */
  char AllowLowStrt[6];    /* start time of low baud */
  char AllowLowStop[6];    /* stop time of low baud */
  long LowBaudLimit;       /* highest low baudrate denied */
  short ModemDelay;        /* modem delay time */
  char RingsRequired;      /* number of rings required before picking up */
  char IrqNum;             /* IRQ number for non-standard comm ports */
  short BaseAddress;       /* BASE address for non-standard comm ports */
  bool LeaveDtrUp;         /* leave DTR up on exit to DOS */
  bool ShareIRQs;          /* share IRQs on MCA buses */
  bool DisableScan;        /* disable message base scan prompt */
  bool DisableCls;         /* disable screen clear after 3 minutes */
  bool DisableEdits;       /* disable registration edits */
  bool DisableFilter;      /* disable high order ascii filter */
  bool DisableQuick;       /* disable quick logon */
  bool DisablePassword;    /* disable password check on return from DOS */
  bool MultiLingual;       /* support multi-lingual operation */
  bool AllowPwrdOnly;      /* allow only password changes */
  bool ClosedBoard;        /* run system as a closed board */
  bool NonGraphics;        /* run in non-graphics mode only */
  bool ExitToDos;          /* exit to dos after each caller */
  bool IncludeCity;        /* include user's city/state in node chat display */
  bool EliminateSnow;      /* eliminate screen snow */
  bool SubscriptMode;      /* run in subscription mode */
  bool AllowEscCodes;      /* allow ESC codes in messages */
  bool AllowCCs;           /* allow CC: messages */
  bool Validate;           /* valide TO: name */
  bool LastReadUpdate;     /* update Last Read pointer afer new msg */
  bool EnforceTime;        /* enforce daily time limit */
  char DisplayNews;        /* display news only if changed since last call */
  bool DisableCTSdrop;     /* disable CTS drop during disk writes */
  char KbdTimeout;         /* num of minutes for keyboard timeout */
  short MaxScrollBack;     /* maximum lines in scrollback buffer */
  bool FastText;           /* pre-load pcbtext file */
  bool FastCnames;         /* pre-load cnames file */
  bool UploadBy;           /* include Upload By: in upload description */
  bool TestUploads;        /* test files after upload */
  bool ShowSettings;       /* include settings display on login */
  bool AllowShell;         /* allow shelling to DOS */
  bool Slaves;             /* use Slave Card updating */
  short SubscriptionDays;  /* default days in new subscription period */
  short SubscriptWarning;  /* days prior to subscription expiration */
  bool Network;            /* running under a network */
  short NodeNum;           /* node number of this node */
  char NetTimeout;         /* network timeout delay (20-99 seconds) */
  char ChatDelay;          /* node chat delay inveral when idle (5-99 sec) */
  char BoardName[66];      /* name of board */
  char CapFile[9];         /* name of message capture file */
  char ViewBatch[32];      /* name and loc of batch file to view archives */
  char ViewExt[5];         /* archive extension */
  char FuncKeys[10][62];   /* function key definitions */
  unsigned short NumConf;  /* number of conferences (1 thru 65534) */
  unsigned short NumAreas; /* number of conference areas (Main=1 thru 65535) */
  short MaxMsgLines;       /* maximum message lines allowed (1-255) */
  char DefaultColor;       /* default foreground color */
  char DefaultIntensity;   /* default intensity (0 or 1) */
  bool EventActive;        /* timed event active */
  char EventTime[8];       /* time to activate event '00:00' */
  char EventSuspend;       /* minutes to suspend board activity before event */
  bool EventStopUplds;     /* stop uploads initiation during suspense time */
  bool EventSlide;         /* slide event if user online */
  char UploadBufSize;      /* upload buffer size (4-32) */
  bool DisableDriveCheck;  /* disable upload drive size check */
  char ParallelPortNum;    /* parallel port number (1-3 or 0) */
  unsigned StopFreeSpace;  /* stop uploads when free space < #K */
  unsigned MaxTotalMsgs;   /* total maximum messages in a capture file */
  unsigned MaxConfMsgs;    /* maximum messages per conference in capture */
  bool QuickScan;          /* default to 'Y Q' message scanning */
  bool ScanAll;            /* default to 'Y A' message scanning */
  char MinPriorToEvent;    /* stop upload minutes prior to an event */
  bool UseNewAskFile;      /* use NEWASK plus built-in questions */
  bool AllowOneName;       /* allow one-name users to log into system */
  char DefExpiredLevel;    /* default expired security level */
  bool StopClockOnCap;     /* stop the clock during capture file download */
  char SysopStart[6];      /* start time to allow sysop page */
  char SysopStop[6];       /* stop  time to allow sysop page */
  bool LogCallerNumber;    /* log caller number to disk */
  bool LogConnectStr;      /* log connect string to disk */
  bool LogSecLevel;        /* log security level to disk */
  bool ConfPwrdAdjust;     /* re-read PWRD upon joining a conference */
  bool ConfirmCaller;      /* confirm caller name */
  bool AllowPwrdComment;   /* allow password failure comment */
  bool GuardLogoff;        /* guard logoff command */
  char NumDescLines;       /* number of upload description lines */
  char UserLevels[USERSEC];/* user security levels */
  char ChatFile[32];       /* name and location of chat.dat */
  char StatsFile[33];      /* name and location of pcbstats.dat */
  char ChatMenu[33];       /* name and location of CHAT menu */
  char NoAnsi[33];         /* name and location of NOANSI file */
  char SlowDrives[27];     /* drive letters for slow drives */
  char SwapPath[33];       /* path to swap file */
  bool Swap;               /* swap out during normal shell? */
  bool NoBatchUp;          /* don't allow batch uploads / force first name */
  bool ForceMain;          /* force comments to main board */
  bool Foreign;            /* LineSeparator and tilde changes */
  bool MonitorModem;       /* watch for incompleted connections */
  bool No16550;            /* force 16550 to be treated as a 16450 */
  bool Force16550A;        /* force 16450 to be treated as a 16550 */
  bool AutoReset;          /* reset modem every 5 minutes while waiting */
  bool Float;              /* float node number */
  bool PromoteBatch;       /* promote D and U commands to DB and UB */
  bool AutoRegConf;        /* automatically register new user in all confs */
  bool ScanBlts;           /* scan bulletins when joining a conference */
  bool AutoMakeMsgs;       /* create msgs file if missing */
  bool VerifyCDLoss;       /* verify cd loss before recycling */
  bool Encrypt;            /* encrypt users file */
  short UploadCredit;      /* time credit (times 10, i.e. 25 = 2.5 credit) */
  short ByteCredit;        /* byte credit (times 10, i.e. 25 = 2.5 credit) */
  char NewDefaultColor;    /* color code for default color */
  char MsgHdr1Color;       /* color for DATE line of message header */
  char MsgHdr2Color;       /* color for TO   line of message header */
  char MsgHdr3Color;       /* color for FROM line of message header */
  char MsgHdr4Color;       /* color for SUBJ line of message header */
  char MsgHdr5Color;       /* color for READ line of message header */
  char MsgHdr6Color;       /* color for CONF line of message header */
  char Origin[61];         /* ORIGIN information */
  char EventDatFile[33];   /* name and location of EVENT.DAT */
  char EventFiles[33];     /* location of event files */
  char CmdLst[32];         /* name/location of CMD.LST to use for defaults */
  bool RequirePwrdToExit;  /* Require Local Password to drop PCBoard to DOS */
  short EnvSize;           /* requested size for DOS's environment */
  char LowBaudSecOverride; /* Security Level to override low baud limit */
  char AllFilesList[33];   /* name and location of all files DLPATH.LST */
  bool EnableFido;         /* enable FIDO setup and processing */
  bool FidoProcessIn;      /* TRUE allows the node to process incoming PKT */
  bool FidoProcessOrphan;  /* TRUE allows the node to process orphan PKT */
  bool FidoProcessOut;     /* TRUE allows the node to export mail */
  bool FidoDialOut;        /* TRUE allows the node to dial out */
  short FidoPktFreq;       /* Minutes between processing incoming PKT */
  short FidoExportFreq;    /* Minutes between checking to export mail */
  short FidoMailFreq;      /* Minutes between checking for need to dial out */
  short FidoDefaultZone;
  short FidoDefaultNet;
  char FidoConfig[33];     /* FIDO import/export configuration file */
  char FidoQueue[33];      /* FIDO queue filename */
  bool PromptToReadMail;   /* ask to read mail waiting */
  char LogOnScr[32];       /* name and loc of logon script questionnaire */
  char LogOnAns[32];       /* name and loc of logon script answers */
  char QwkFile[9];         /* name of qwk packet file */
  char FileTcan[32];       /* name of File Trashcan for stopping uploads */
  bool SwapDuringBat;      /* TRUE if $$logon or $$logoff.bat should swap */
  bool UserSysDuringBat;   /* TRUE if users.sys should be made for $$logon */
  bool DefaultGraphics;    /* TRUE to default to graphics mode on login */
  char SlowDriveBat[32];   /* batch file to run after slow-drive copy */
  bool AllowAliasChange;   /* TRUE if ALIAS can be changed after setting it */
  bool ExcludeLocals;      /* TRUE if local logins should be ignored */
  short PwrdUpdate;        /* Number of days PWRD is valid before expiring */
  short PwrdWarn;          /* Number of days prior to WARN of PWRD expiring */
  bool RecordGroupChat;    /* TRUE if group chat should be captured to disk */
  bool ShowAlias;          /* TRUE if sysop wants ALIAS shown in USERNET */
  char MinPwrdLen;         /* Minimum Password Length when PSA is installed */
  bool AllowHandles;       /* TRUE if handles are allowed in Group Chat */
  char CmdLoc[32];         /* location of command display files */
  bool ForceIntro;         /* TRUE if forcing INTRO to be displayed */
  bool SkipProtocol;       /* TRUE to skip protocol selection for new users */
  bool SkipAlias;          /* TRUE to skip alias selection for new users */
  bool NoCarrierExit;      /* TRUE to exit to DOS if no carrier after connect */
  bool OS2Driver;          /* TRUE to use OS/2 COMM Driver */
  bool EnableAccounting;   /* TRUE if accounting features are enabled        */
  bool ShowCurrency;       /* TRUE if currency values should be shown        */
  bool Concurrent;         /* TRUE if charges run concurrently               */
  bool IgnoreDropSecLevel; /* TRUE if the DropSecLevel should not be used    */
  char PeakStart[6];       /* Start of peak usage hours                      */
  char PeakEnd[6];         /* End of peak usage hours                        */
  char PeakDays;           /* bit encoded flags (bit0=sun, bit1=mon, etc     */
  char HolidaysFile[32];   /* name/loc of Peak Holidays File                 */
  char AccountConfig[32];  /* name/loc of Account Rate File                  */
  char AccountInfo[32];    /* name/loc of Account Info File                  */
  char AccountWarn[32];    /* name/loc of Account Warning File               */
  char AccountTrack[32];   /* name/loc of Account Tracking File              */
  char AccountLogoff[32];  /* name/loc of Account Logoff Info File           */
  char uucpPath[33];       /* location for UUCP files                        */
  char uucpSpoolPath[33];  /* location for UUCP Spool files                  */
  char uucpLogPath[33];    /* location for UUCP Log files                    */
  char CompBatFile[33];    /* name/loc of compress batch file for UUCP       */
  char DeCompBatFile[33];  /* name/loc of decompress batch file for UUCP     */
  char uucpName[17];       /* UUCP Name                                      */
  char uucpDomainName[33]; /* UUCP Domain Name                               */
  char uucpEmailHost[17];  /* UUCP Email Host Name                           */
  char uucpNewsHost[17];   /* UUCP News Host Name                            */
  char uucpDefDist[17];    /* UUCP Default News Distribution                 */
  char uucpTimeZone[6];    /* Time Zone - offset from GMT                    */
  char uucpSeparator[2];   /* UUCP Name Separator                            */
  char Organization[65];   /* Organization Name                              */
  char uucpHighAscii;      /* S = Strip, R = Replace, C = Convert            */
  unsigned uucpEmailConf;  /* number of UUCP Email Conference                */
  unsigned uucpJunkConf;   /* number of UUCP Junk Conference                 */
  bool uucpBangDomain;     /* Bang Domain                                    */
  bool uucpSubDomain;      /* Sub Domain                                     */
  char uucpModFile[33];    /* List of UUCP Moderators                        */
  bool FidoImportAfterXfer;/* TRUE if import takes place after file xfer     */
  short FidoCrashSec;      /* security level required for crash mail         */
  bool FidoLogLevel;       /* controls the amount of logging performed       */
  bool FidoCreateMsg;      /* create MSG files                               */
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

#pragma pack(1)
typedef struct {
  char Name[61];           /* conference name */
  bool PublicConf;         /* new user default registration */
  bool AutoRejoin;         /* auto-rejoin for this conference */
  bool ViewMembers;        /* allow viewing conference members */
  bool PrivUplds;          /* make uploads private */
  bool PrivMsgs;           /* make all messages private */
  bool EchoMail;           /* is this an Echo-Mail conference */
  short ReqSecLevel;       /* security required to join PUBLIC conference */
  short AddSec;            /* additional security level added after join */
  short AddTime;           /* additional time added after join */
  char MsgBlocks;          /* number of message base index blocks */
  char MsgFile[32];        /* name and location of messages file */
  char UserMenu[32];       /* name and location of Users Menu */
  char SysopMenu[32];      /* name and location of Sysops Menu */
  char NewsFile[32];       /* name and location of Conference News */
  char PubUpldSort;        /* sort type for public upload DIR file */
  char UpldDir[29];        /* name and location of public upload DIR file */
  char PubUpldLoc[26];     /* public upload location */
  char PrvUpldSort;        /* sort type for public upload DIR file */
  char PrivDir[29];        /* name and location of private upload DIR file */
  char PrvUpldLoc[26];     /* private upload location */
  char DrsMenu[29];        /* name and location of conference doors menu */
  char DrsFile[33];        /* name and location of DOORS.LST file */
  char BltMenu[29];        /* name and location of conference blt menu */
  char BltNameLoc[33];     /* name and location of BLT.LST file */
  char ScrMenu[29];        /* name and location of conference scripts menu */
  char ScrNameLoc[33];     /* name and location of SCRIPT.LST */
  char DirMenu[29];        /* name and location of directories menu */
  char DirNameLoc[33];     /* name and location of DIR.LST file */
  char PthNameLoc[33];     /* name and location of DLPATH.LST file */
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
  char ConfType;           /* 0=Normal, 1=Inet E-mail, 2=Inet Junk, 3=Mod Nwsgrp, 4=Pub Nwsgrp, 5=Fido */
  long ExportPtr;          /* last message exported for fido                 */
  float ChargeTime;        /* amount to charge for time while in conference  */
  float ChargeMsgRead;     /* amount to charge for reading msgs in conference*/
  float ChargeMsgWrite;    /* amount to charge for writing msgs in conference*/
} pcbconftype;
#pragma pack()

#pragma pack(1)
typedef struct {
  unsigned short Version;           /* PCBoard version number (i.e. 145, 150 or 152) */
  long           RecNo;             /* Record number from USER's file */
  unsigned short SizeOfRec;         /* Size of "fixed" user record */
  unsigned short NumOfAreas;        /* Number of conference areas (Main=1 thru 65535) */
  unsigned short NumOfBitFields;    /* Number of Bit Map fields for conferences */
  unsigned short SizeOfBitFields;   /* Size of each Bit Map field */
  char           AppName[15];       /* Name of the Third Party Application (if any) */
  unsigned short AppVersion;        /* Version number for the application (if any) */
  unsigned short AppSizeOfRec;      /* Size of a "fixed length" record (if any) */
  unsigned short AppSizeOfConfRec;  /* Size of each conference record (if any) */
  long           AppRecOffset;      /* Offset of AppRec into USERS.INF record (if any) */
  bool           Updated;           /* TRUE if the USERS.SYS file has been updated */
} syshdrtype;
#pragma pack()


#pragma pack(1)
#ifdef _MSC_VER
/* MSC makes structures that are as small as the smallest type used.  So    */
/* with an "int Dirty:1" it forces the entire structure to be TWO BYTES in  */
/* size even though the total element size is only 8 bits.  Therefore, MSC  */
/* is compiled with "char" as the type instead of "int".  However, Turbo C  */
/* does not like using "char".  AARGH!  So we test for the MSC compiler and */
/* compile using "char" for MSC, and "int" for Turbo C */

typedef struct {               /* Bit packed flags in the users file */
  char Dirty    :1;            /* Dirty Flag (meaning file has been updated) */
  char MsgClear :1;            /* User's choice for screen clear after messages */
  char HasMail  :1;            /* Indicates if NEW mail has been left for user */
  char DontAskFSE:1;           /* Used with FSEDefault, if set, don't prompt caller */
  char FSEDefault:1;           /* Use Full Screen Editor */
  char ScrollMsgBody:1;        /* Scroll message body when reading */
  char ShortHeader:1;          /* Display short message header */
  char WideEditor:1;           /* Use wide editor when entering messages */
} packedbyte;
typedef struct {
  char UnAvailable:1;          /* Unavailable for chat */
  char Reserved:7;
} packedbyte2;
#else
typedef struct {               /* Bit packed flags in the users file */
  int Dirty    :1;             /* Dirty Flag (meaning file has been updated) */
  int MsgClear :1;             /* User's choice for screen clear after messages */
  int HasMail  :1;             /* Indicates if NEW mail has been left for user */
  int DontAskFSE:1;            /* Used with FSEDefault, if set, don't prompt caller */
  int FSEDefault:1;            /* Use Full Screen Editor */
  int ScrollMsgBody:1;         /* Scroll message body when reading */
  int ShortHeader:1;           /* Display short message header */
  int WideEditor:1;            /* Use wide editor when entering messages */
} packedbyte;
typedef struct {
  int UnAvailable:1;           /* Unavailable for chat */
  int Reserved:7;
} packedbyte2;
#endif
#pragma pack()

#pragma pack(1)
typedef struct {               /* DOS format for bit packed date fields */
  int  Day   :5;               /* 5 bit integer representing the Day */
  int  Month :4;               /* 4 bit integer representing the Month */
  int  Year  :7;               /* 7 bit integer representing the Year MINUS 80 */
} datetype;
#pragma pack()

#ifndef EXTENDED_USER_INFO
#define EXTENDED_USER_INFO
#pragma pack(1)
typedef struct {
  char  Street[2][51];
  char  City[26];
  char  State[11];
  char  Zip[11];
  char  Country[16];
} addresstypez;
#pragma pack()

#pragma pack(1)
typedef struct {
  char           Previous[3][13];
  unsigned short LastChange;
  unsigned short TimesChanged;
  unsigned short ExpireDate;
} passwordtypez;
#pragma pack()

#pragma pack(1)
typedef struct {
  char  Line[5][61];
} notestypez;
#pragma pack()

#pragma pack(1)
typedef struct {
  unsigned short FirstDateOn;
  unsigned short NumSysopPages;
  unsigned short NumGroupChats;
  unsigned short NumComments;
  unsigned short Num300;
  unsigned short Num1200;
  unsigned short Num2400;
  unsigned short Num9600;
  unsigned short Num14400;
  unsigned short NumSecViol;
  unsigned short NumNotReg;
  unsigned short NumReachDnldLim;
  unsigned short NumFileNotFound;
  unsigned short NumPwrdErrors;
  unsigned short NumVerifyErrors;
} callerstattype;
#pragma pack()

#pragma pack(1)
typedef struct {
  double StartingBalance;
  double StartThisSession;
  double DebitCall;
  double DebitTime;
  double DebitMsgRead;
  double DebitMsgReadCapture;
  double DebitMsgWrite;
  double DebitMsgWriteEchoed;
  double DebitMsgWritePrivate;
  double DebitDownloadFile;
  double DebitDownloadBytes;
  double DebitGroupChat;
  double DebitTPU;
  double DebitSpecial;
  double CreditUploadFile;
  double CreditUploadBytes;
  double CreditSpecial;
  char   DropSecLevel;
} accounttype;
#pragma pack()

#pragma pack(1)
typedef struct {
  unsigned short MaxMsgs;
  unsigned short MaxMsgsPerConf;
  long           PersonalAttachLimit;
  long           PublicAttachLimit;
  long           NewBltLimit;
  bool           NewFiles;
  char           Reserved[13];
} qwkconfigtype;
#pragma pack()
#endif //#ifdef EXTENDED_USER_INFO

#pragma pack(1)
typedef struct {
  char           Name[26];           /* Name (NULL terminated) */
  char           City[25];           /* City (NULL terminated) */
  char           Password[13];       /* Password (NULL terminated) */
  char           BusDataPhone[14];   /* Business or Data Phone (NULL terminated) */
  char           HomeVoicePhone[14]; /* Home or Voice Phone (NULL terminated) */
  unsigned short LastDateOn;         /* Julian date for the Last Date On */
  char           LastTimeOn[6];      /* Last Time On (NULL Terminated) */
  bool           ExpertMode;         /* 1=Expert, 0=Novice */
  char           Protocol;           /* Protocol (A thru Z) */
  packedbyte     PackedFlags;        /* Bit packed flags */
  datetype       DateLastDirRead;    /* Date for Last DIR Scan (most recent file) */
  short          SecurityLevel;      /* Security Level */
  unsigned short NumTimesOn;         /* Number of times the caller has connected */
  unsigned char  PageLen;            /* Page Length when display data on the screen */
  unsigned short NumUploads;         /* Total number of FILES uploaded */
  unsigned short NumDownloads;       /* Total number of FILES downloaded */
  long           DailyDnldBytes;     /* Number of BYTES downloaded so _FAR_ today */
  char           UserComment[31];    /* Comment field #1 (NULL terminated) */
  char           SysopComment[31];   /* Comment field #1 (NULL terminated) */
  short          ElapsedTimeOn;      /* Number of minutes online */
  unsigned short RegExpDate;         /* Julian date for Registration Expiration Date */
  short          ExpSecurityLevel;   /* Expired Security Level */
  unsigned short LastConference;     /* Number of the conference the caller was in */
  unsigned long  ulTotDnldBytes;     /* Total number of BYTES downloaded */
  unsigned long  ulTotUpldBytes;     /* Total number of BYTES uploaded */
  bool           DeleteFlag;         /* 1=delete this record, 0=keep */
  long           RecNum;             /* Record Number in USERS.INF file */
  packedbyte2    Flags;
  char           Reserved[8];        /* Bytes 390-397 from the USERS file */
  unsigned long  MsgsRead;           /* Number of messages the user has read in PCB */
  unsigned long  MsgsLeft;           /* Number of messages the user has left in PCB */
  bool           AliasSupport;
  char           Alias[26];
  bool           AddressSupport;
  addresstypez   Address;
  bool           PasswordSupport;
  passwordtypez  PwrdHistory;
  bool           VerifySupport;
  char           Verify[26];
  bool           StatsSupport;
  callerstattype Stats;
  bool           NotesSupport;
  notestypez     Notes;
  bool           AccountSupport;
  accounttype    Account;
  bool           QwkSupport;
  qwkconfigtype  QwkConfig;
  double         TotDnldBytes;     /* converted to IEEE Double */
  double         TotUpldBytes;     /* converted to IEEE Double */
} userrectype;
#pragma pack()

#pragma pack(1)
typedef struct {
  int  DateFormat;
  char DollarSym[5];
  char ThousandSep[2];
  char FractionSep[2];
  char DateSep[2];
  char TimeSep[2];
  char CurrencyFmt;
  char SigDigits;
  char TimeFmt;
  char (_FAR_ *upchar)(char Al);
  char DataSep[2];
  char Reserved[10];
} countrytype;
#pragma pack()

#endif

/* Forward type declarations for pcboard.h enum types (Watcom needs these
   when PCBTOOLS.H is included without pcboard.h) */
#ifdef __WATCOMC__
#ifndef _PCBOARD_H_TYPES_
#define _PCBOARD_H_TYPES_
typedef int acttype;
typedef int moretype;
typedef int padtype;
typedef int startdisplaytype;
typedef int timereasontype;
typedef int shelltype;
typedef int modemverifytype;
/* Forward enum values for pcboard.h types */
#define NOCHANGE 0
#define FORCENONSTOP 1
#define FORCECOUNTLINES 2
#define ACTSHOW 0
#define ACTBEGIN 1
#define ACTEND 2
#define ACTSUSPEND 3
#define ACTRESUME 4
#define MOREPROMPT 0
#define PRESSENTER 1
/* modemverifytype values */
#define WAITFOROKAY 1
#define NOWAIT 0
/* displayfiletype values */
#define NOALTERNATE 0
#define GRAPHICS 1
#define SECURITY 2
#define LANGUAGE 4
#define WELCOME 8
#define RUNMENU 16
#define RUNPPL 32
#define CMDFILE 64
#define DISABLESUBFILES 128
/* padtype values */
#define SPACELEFT 0
#define SPACERIGHT 1
#define NOSPACE 2
#define DONTWAIT 0
#endif
#endif

extern asytype     Asy;
extern char        B16550;
extern char        B16550A;
extern char        B16650;
extern char        B8250;
extern char        NO16550;
extern char        Allow16650;
extern char        FORCE16550A;
extern char        VerifyCDLoss;
extern char _FAR_ *  ConfReg;
extern char _FAR_ *  ConfFlags;
extern long _FAR_ *  MsgReadPtr;
extern unsigned    ConfByteLen;
extern controltype Control;
extern disptype    Display;
extern char *      DoorParams;
extern bool        ErrorLevel;
extern bool        ForceUpdate;
extern char        RTSoff;
extern char        PcbDir[];
extern bool        UseAnsi;
extern syshdrtype  UserSysHdr;
extern userrectype UserSys;
extern void *      TPAstatic;
extern void *      TPAdynamic;
extern char        mask_alphanum[];
extern char        mask_yesno[];
extern char        mask_num[];
extern char        mask_atoz[];
extern long        Scrn_Addr;
extern char        Scrn_Mode;
extern char        YesChar;
extern char        NoChar;
extern char        SystemYesChar;
extern char        SystemNoChar;
extern countrytype Country;
extern char        LineSeparator;
extern bool        OldIndex;

#if defined(__LARGE__) || defined(__COMPACT__)
extern pcbdattype _FAR_ PcbData;
extern statustype _FAR_ Status;
#else
extern pcbdattype PcbData;
extern statustype Status;
#endif


#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif

#ifdef __OS2__
  #define fmemcpy(dest,src,len) memcpy(dest,src,len)
#else
  void LIBENTRY fmemcpy(void _FAR_ *dest,void _FAR_ *src,unsigned len);
#endif

#ifdef __OS2__
void LIBENTRY settimer(int Which, int Hundredths);
#else
void LIBENTRY settimer(int Which, long Ticks);
#endif

long LIBENTRY gettimer(int Which);
void LIBENTRY giveup(void);
void LIBENTRY mydelay(int Hundredths);

char LIBENTRY curcolor(void);
void LIBENTRY ansi(char *Str);
char LIBENTRY awherex(void);
char LIBENTRY awherey(void);
void LIBENTRY agotoxy(char X, char Y);
void LIBENTRY asetcolor(char NewColor);
void LIBENTRY bell(void);

#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif


/*
#if defined(__cplusplus) || defined(__WATCOMC__)
extern "C" {
#endif
*/

extern int  (LIBENTRY *ringdetect)(void);
extern int  (LIBENTRY *ctsokay)(void);
extern int  (LIBENTRY *inbytes)(void);
extern int  (LIBENTRY *outbytes)(void);
extern int  (LIBENTRY *framingerrors)(void);
extern int  (LIBENTRY *overrunerrors)(void);
extern int  (LIBENTRY *parityerrors)(void);
extern int  (LIBENTRY *cdstillup)(void);
extern int  (LIBENTRY *online)(void);
extern int  (LIBENTRY *bauddivisor)(long PortSpeed);
extern void (LIBENTRY *setport)(int BaudDivisor,int DataBits);
extern void (LIBENTRY *turnoffdtr)(void);
extern void (LIBENTRY *turnondtr)(void);
extern void (LIBENTRY *turnoffrts)(void);
extern void (LIBENTRY *turnonrts)(void);
extern void (LIBENTRY *turnonxmit)(void);
extern void (LIBENTRY *clearoutbuf)(void);
extern void (LIBENTRY *clearinbuf)(void);
extern void (LIBENTRY *commgo)(void);
extern void (LIBENTRY *commstop)(void);
extern void (LIBENTRY *commpause)(void);
extern int  (LIBENTRY *checkcomm)(void);
extern int  (LIBENTRY *comminkey)(void);
extern int  (LIBENTRY *cgetstr)(char *pStr, int StrLen);
extern int  (LIBENTRY *cgetbuf)(char *Buf, int BufLen);
extern void (LIBENTRY *csendbyte)(unsigned char ByteToSend);
extern void (LIBENTRY *csendstr)(char *pStr, int StrLen);
#define RingDetect ringdetect()
#define CTSokay ctsokay()
#define InBytes inbytes()
#define OutBytes outbytes()
#define FramingErrors framingerrors()
#define OverrunErrors overrunerrors()
#define ParityErrors parityerrors()

int  LIBENTRY initdoor(char *Name, int SessionLimit, int KbdTimeout, openstatus OpenStatus);
void LIBENTRY closedoor(bool UpdateUserSys);
void LIBENTRY goodbye(bool UpdateUserSys);
void LIBENTRY println(char *Str);

void LIBENTRY dosinit(void);
void LIBENTRY pcbinit(void);
char * LIBENTRY strnchr(char *Str, char C, int Len);

#ifdef __OS2__
  #pragma pack(4)
  typedef struct {
    int           ExtendedError;
    unsigned long ExtendedAction;
    unsigned long ExtendedClass;
    unsigned long ExtendedLocus;
  } os2errtype;
  #pragma pack()
  /* the OS/2 functions require passing the extended error information    */
  /* because if multiple threads are used, the individual error states    */
  /* for each thread need to be handled.  DOS2ERROR declares the Os2Error */
  /* variable in the function prototype, while POS2ERROR passes a pointer */
  /* to the Os2Error variable.                                            */
  #define DOS2ERROR    ,os2errtype *Os2Error
  #define POS2ERROR    ,&Os2Error
  #define POS2ERROR2   ,Os2Error
#else
  /* the DOS functions don't pass the extended error values because the */
  /* extended error variables are global variables.                     */
  #define DOS2ERROR
  #define POS2ERROR
  #define POS2ERROR2
  extern int      ExtendedError;
  extern char     ExtendedAction;
  extern char     ExtendedClass;
  extern char     ExtendedLocus;
  extern char     Int24Error;
  extern char     Int24Flags;
#endif

int  LIBENTRY dosappendcheck(char *path, int access);
int  LIBENTRY dosopencheck(char *path, int access);
int  LIBENTRY doscreatecheck(char *path, int access, unsigned mode);
unsigned LIBENTRY writecheck(int handle, void *buffer, unsigned count);
unsigned LIBENTRY readcheck(int handle, void *buffer, unsigned count);
int  LIBENTRY dosappend(char *FileName, int OpenFlags);
void LIBENTRY dosclose(int handle);
int  LIBENTRY doscommit(int Handle);
int  LIBENTRY doscreate(char *FileName, int OpenFlags, int Attributes DOS2ERROR);
int  LIBENTRY dosfclose(DOSFILE *file);
int  LIBENTRY dosfgets(char *Str, unsigned MaxChars, DOSFILE *file);
int  LIBENTRY dosfputs(char *Str, DOSFILE *file);
int  LIBENTRY dosflush(DOSFILE *file);
int  LIBENTRY dosfopen(char *FileName, int OpenFlags, DOSFILE *file);
int  LIBENTRY dosfread(void *Buffer, unsigned NumChars, DOSFILE *file);
int  LIBENTRY dosfwrite(void *Buffer, unsigned NumChars, DOSFILE *file);
long LIBENTRY dosfseek(DOSFILE *file, long distance, char method);
int  LIBENTRY doslockcheck(int handle, long offset, long length);
long LIBENTRY doslseek(int handle, long distance, char method);
int  LIBENTRY dosread(int handle, void *buf, unsigned len DOS2ERROR);
void LIBENTRY dosrewind(DOSFILE *file);
int  LIBENTRY dossetbuf(DOSFILE *file, unsigned BufSize);
int  LIBENTRY doswrite(int handle, void *buf, unsigned len DOS2ERROR);
int  LIBENTRY dosdup(int OldHandle DOS2ERROR);
int  LIBENTRY dosopen(char *FileName, int OpenFlags DOS2ERROR);



void LIBENTRY showactivity(acttype Type);
void LIBENTRY atclosing(void);
void LIBENTRY print(char *Str);
void LIBENTRY printcls(void);
void LIBENTRY moreprompt(moretype Type);
void LIBENTRY newline(void);
void LIBENTRY freshline(void);
void LIBENTRY println(char *Str);
void LIBENTRY printxlated(char *Str);
void LIBENTRY xlatetext(char *New, char *Old);
int  LIBENTRY strlenminuscolorcodes(char *Str);
void LIBENTRY printdefcolor(void);
void LIBENTRY printcolor(int ColorNum);
void LIBENTRY backupcleareol(int NumColumns);
void LIBENTRY backup(int NumColumns);
void LIBENTRY forward(int NumColumns);
void LIBENTRY movecursor(int X, int Y);
void LIBENTRY cleareol(void);
void LIBENTRY turnkbdtimeron(void);
void LIBENTRY inputfield(char *Buffer, int PcbTextNum, int MaxLen, DISPLAYTYPE Display, int HelpNum, char *Mask);
int  LIBENTRY inputfieldint(int Default, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum);
long LIBENTRY inputfieldlong(long Default, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum);
void LIBENTRY inputfieldstr(char *Buffer, char *Prompt, int Color, int MaxLen, DISPLAYTYPE Display, int HelpNum, char *Mask);
void LIBENTRY inputfieldreqstr(char *Buffer, char *Prompt, int Color, int MaxLen, int MinLen, DISPLAYTYPE Display, int HelpNum, char *Mask);
void LIBENTRY getalternatename(char *FileName, DISPLAYFILETYPE Type, int RecordSize);
int  LIBENTRY displayfile(char *Name, DISPLAYFILETYPE Type);
void LIBENTRY waitforempty(int Seconds);
void LIBENTRY sendbyte(char Byte);
void LIBENTRY sendstr(char *Str, int StrLen);
void LIBENTRY writelog(char *Str, padtype Pad);
void LIBENTRY addchar(char *Str, char Ch);
char * LIBENTRY stripleft(char *Str, char Ch);
char * LIBENTRY stripright(char *Str, char Ch);
void LIBENTRY setbit(void _FAR_ *BitStream, int BitNum);
unsigned short LIBENTRY datetojulian(char *DateStr);
unsigned short LIBENTRY getjuliandate(void);
char * LIBENTRY juliantodate(unsigned short JD);
void LIBENTRY redisplaystatusline(void);
void LIBENTRY turnonfifo(char TriggerLevel);
void LIBENTRY fmemset(void _FAR_ *dest,char c,unsigned len);
void LIBENTRY sysopchat();
void LIBENTRY checkdisplaystatus(void);
void LIBENTRY startdisplay(startdisplaytype Action);
void LIBENTRY clearkbdbuffer(void);
int  LIBENTRY commportinkey(void);
int  LIBENTRY kbdinkey(void);
int  LIBENTRY cinkey(void);
void LIBENTRY checkstatus(void);
void * LIBENTRY bmalloc(unsigned Size);
void LIBENTRY bfree(void *p);
void LIBENTRY bfreeall(void);
void _FAR_ * LIBENTRY fbmalloc(long Size);
void LIBENTRY fbfree(void _FAR_ *p);
void LIBENTRY fbfreeall(void);
short LIBENTRY minutesleft(void);
short LIBENTRY minutesonline(void);
void LIBENTRY addtime(long Seconds, timereasontype Reason);
void LIBENTRY subtime(long Seconds, timereasontype Reason);
void LIBENTRY ascii(char *Str, int Num);
void LIBENTRY asciispace(char *Str, int Num);
void LIBENTRY lascii(char *Str, long Num);
bool LIBENTRY substitute(char *Str, char *Search, char *Replace, int MaxLen);
void buildstr(char *Dest, ...);
void LIBENTRY tickdelay(int Ticks);
void LIBENTRY change(char *Str, char Old, char New);
void LIBENTRY displaypcbtext(int PcbTextNum, DISPLAYTYPE Display);
int  LIBENTRY savescrnbuffer(void);
void LIBENTRY restscrnbuffer(void);
void LIBENTRY turndisplayon(bool WriteToDisk);
void LIBENTRY turndisplayoff(bool WriteToDisk);
void LIBENTRY shelltodos(void);
int  LIBENTRY performshell(char *Command, char *Params, shelltype ShellType, int PriorityDelta, bool MinimizeSelf, bool MinimizeWindow, int WaitTime);
int  LIBENTRY tokenize(char *Command);
int  LIBENTRY tokenizestr(char *Command);
char * LIBENTRY getnexttoken(void);
void LIBENTRY addbackslash(char *Str, int MaxLen);
void LIBENTRY cls(void);
int  LIBENTRY getcursor(void);
void LIBENTRY setcursor(int CursorType);
char * LIBENTRY datestr(char *DateStr);  /* MM/DD/YY */
void LIBENTRY fastputc(int Loc,char C);
void LIBENTRY fastputch(int Loc, char C, char Color);
void LIBENTRY fastprint(int X,int Y,char Str[80],char Attr);
int  LIBENTRY xyvalue(int X, int Y);
char * LIBENTRY timestr1(char *TimeStr);  /* HH:MM:SS */
char * LIBENTRY timestr2(char *TimeStr);  /* HH:MM */
long LIBENTRY bassngltolong(bassngl Num);
long LIBENTRY basdbletolong(basdble Num);
void LIBENTRY longtobasdble(basdble New, long Num);
void LIBENTRY longtobassngl(bassngl New, long Num);
unsigned char LIBENTRY fileexist(char *FileName);
int  LIBENTRY isset(void _FAR_ *BitStream, int BitNum);
void LIBENTRY maxstrcpy(char *Dest, char *Srce, int Max);
void LIBENTRY padstr(char *Str, char C, int MaxLen);
void LIBENTRY unsetbit(void _FAR_ *BitStream, int BitNum);
int  LIBENTRY getconfrecord(unsigned ConfNum, pcbconftype *Conf);
int  LIBENTRY putconfrecord(unsigned ConfNum, pcbconftype *Conf);
int  LIBENTRY opencnames(void);
void LIBENTRY closecnames(void);
int  LIBENTRY displayhelpfile(int HelpNum);
void LIBENTRY customhelp(int HelpNum);
long LIBENTRY finduser(char *SrchName);
void LIBENTRY slowsendtomodem(char *Str);
int  LIBENTRY showmodem(char *InBuf, int BufSize, modemverifytype Verify);
int  LIBENTRY modemcommand(char *Command, modemverifytype Verify);
void LIBENTRY initport(void);

char * LIBENTRY countrydate(char *DateStr);
char * LIBENTRY countrydate2(char *DateStr);
char * LIBENTRY uncountrydate(char *DateStr);
char * LIBENTRY uncountrydate2(char *DateStr);
char * LIBENTRY dateformat(void);
char * LIBENTRY dateformat2(void);
char * LIBENTRY comma(char *Buffer, long Num);
char * LIBENTRY ucomma(char *Buffer, unsigned long Num);
char * LIBENTRY dcomma(char *Buffer, double Num);


#define MSG_RCVR        '*'    /* receiver only message - not read       */
#define MSG_RCVR_READ   '+'    /* receiver only message - read           */
#define MSG_CMNT        '~'    /* comment to sysop      - not read       */
#define MSG_CMNT_READ   '`'    /* comment to sysop      - read           */
#define MSG_SPWD        '%'    /* sender pwrd message   - not read       */
#define MSG_SPWD_READ   '^'    /* sender pwrd message   - read           */
#define MSG_GPWD        '!'    /* group pwrd message    - not read       */
#define MSG_GPWD_READ   '#'    /* group pwrd message    - read           */
#define MSG_GPWD_ALL    '$'    /* group pwrd message to all              */
#define MSG_PBLC        ' '    /* msg to all, or public msg not yet read */
#define MSG_PBLC_READ   '-'    /* public msg that has been read          */

#define MSG_ACTIVE      225
#define MSG_INACTIVE    226

typedef enum {READONLY,RDWR,RDWRLOCK} openmsgtype;

#define EXTHDRID    0x40FF
#define EXTFUNCLEN  7
#define EXTDESCLEN  60
#define MAXEXTHDRS  99
enum {HDR_TO=1, HDR_FROM=2, HDR_SUBJ=4, HDR_CARB=8, HDR_FILE=16, HDR_RCPT=64, HDR_XTND=128};
enum {EXTSTAT_NONE='N',EXTSTAT_READ='R'};
typedef enum {EXTHDR_TO,EXTHDR_FROM,EXTHDR_SUBJECT,EXTHDR_ATTACH,EXTHDR_LIST,EXTHDR_ROUTE,EXTHDR_ORIGIN,EXTHDR_REQRR,EXTHDR_ACKRR,EXTHDR_ACKNAME,EXTHDR_PACKOUT,EXTHDR_TO2,EXTHDR_FROM2,EXTHDR_FORWARD,EXTHDR_SUBJ2,EXTHDR_TOTAL} exthdrtype;

extern char ExtHdrFunctions[EXTHDR_TOTAL][EXTFUNCLEN];

#pragma pack(4)
typedef struct {
  long HighMsgNum;
  long LowMsgNum;
  long NumActiveMsgs;
  long NumCallers;
} msgbasestattype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char    Status;
  bassngl MsgNumber;
  bassngl RefNumber;
  char    NumBlocks;
  char    Date[8];
  char    Time[5];
  char    ToField[25];
  bassngl ReplyDate;
  char    ReplyTime[5];
  char    ReplyStatus;
  char    FromField[25];
  char    SubjField[25];
  char    Password[12];
  char    ActiveFlag;
  char    EchoFlag;
  char    Reserved[4];
  char    ExtendedStatus;
  char    Reserved2;
} msgheadertype;
#pragma pack()

#pragma pack(1)
typedef struct {
  int   Ident;                /* a string = NULL-@ = 16384 decimal = 4000 hex */
  char  Function[EXTFUNCLEN]; /* TO, FROM, SUBJECT, ATTACH, LIST, ROUTE, ORIGIN */
  char  Colon;                /* always set to ":" */
  char  Descript[EXTDESCLEN]; /* user name, subject, filename */
  char  Status;               /* NONE, READ (if LIST), DOWNLOADED (if ATTACH) */
  char  CarReturn;            /* 0xE3 or 0x0D */
} msgextendtype;
#pragma pack()

#pragma pack(4)
typedef struct {
  unsigned         ConfNum;
  int              Index;
  DOSFILE          Msgs;
  msgbasestattype  Stats;
  msgheadertype    Header;
  char             *Body;
  int              BodySize;
  int              OpenMode;
} msgbasetype;
#pragma pack()


int  LIBENTRY openmessagebase(unsigned short Num, pcbconftype *Conf, msgbasetype *MsgBase, openmsgtype OpenType);
void LIBENTRY closemessagebase(msgbasetype *MsgBase);
int  LIBENTRY savetomsgbase(unsigned short ConfNum, pcbconftype *Conf, msgbasetype *MsgBase, msgheadertype *Header, char *Body, long OldMsgNumber, long OldOffset, void (LIBENTRY *showsaving)(unsigned short ConfNum, long MsgNum, long MsgOffset));
char * LIBENTRY loadextendedheaders(char *Body);
char * LIBENTRY checkextendedheaders(exthdrtype Which, char *Desc);
void * LIBENTRY buildgenextheader(char *Hdr, char *Desc);
void LIBENTRY buildextheader(exthdrtype Which, char *Desc, char BitMask);
int  LIBENTRY copyexthdrs(int Handle);
void LIBENTRY removeexthdr(exthdrtype Which, char BitMask);
int  LIBENTRY getmsgbasestats(msgbasestattype *MsgStats, char *FileName);
void LIBENTRY freehdrs(void);
void LIBENTRY makemsgheader(msgheadertype *Header, char Status, unsigned short NumBytes, char *Date, char *Time, char *To, long RefNum, unsigned short ReadDate, char *ReadTime, char ReadStatus, char *From, char *Subject, char *Password, bool Echo);
void LIBENTRY makefilenameunique(char *NewName, char *Original);
char * LIBENTRY findstartofname(char *Path);
int  LIBENTRY copyfiletomessage(char *To, char *From, char *Subject, char MsgStatus, bool EchoFlag, unsigned short ConfNum, char *SourceFile, char *FileToAttach, void (LIBENTRY *showsaving)(unsigned short,long,long));

int  LIBENTRY openusersfile(void);
int  LIBENTRY openusersinffile(void);
void LIBENTRY closeusersfile(void);
void LIBENTRY closeusersinffile(void);
void LIBENTRY putmessagewaiting(unsigned ConfNum, long RecNo);

typedef enum {SHOW, HIDE } showtype;

void LIBENTRY openmodem(showtype Show);
void LIBENTRY closemodem(bool TurnOffDTR);
int  LIBENTRY initmodem(char *ComPort, int Num, int Irq, int Addr, long Speed, void (LIBENTRY *pshow)(char *Str));

#define COPYFILE_ERROPENSRCE  1
#define COPYFILE_ERROPENDEST  2
#define COPYFILE_ERRMEMALLOC  3
#define COPYFILE_ERRREADSRCE  4
#define COPYFILE_ERRWRITDEST  5

int LIBENTRY copyfile(char *Srce, char *Dest, bool CheckForEOF);


/****************************************************************************/

// Semaphore classes and definitions for OS/2 threads
#ifdef __OS2__
const int SEM_WAIT_NOTIMEOUT = -1;
const int SEM_TIMEDOUT       =  0;
const int SEM_INTERRUPTED    =  1;

#pragma pack(4)
class CSemaphore {
  public:
    CSemaphore();
    virtual ~CSemaphore();

    bool   LIBENTRY create(const char *SemName);
    bool   LIBENTRY createunique(const char *SemName);
    bool   LIBENTRY open(const char *SemName);
    void   LIBENTRY close(void);
    int    LIBENTRY waitforevent(int Timeout);
    void   LIBENTRY postevent(void);
    int    LIBENTRY reset(void);
    char * LIBENTRY getname(void);

    inline bool     opened(void) { return Opened; }

  protected:
    long  Semaphore;
    bool  Opened;

  private:
    char  Name[32];
};
#pragma pack()

#pragma pack(4)
class CUpdateSemaphore : public CSemaphore {
  public:
    CUpdateSemaphore();
    ~CUpdateSemaphore();

    bool LIBENTRY create(const char *SemName);
    bool LIBENTRY createunique(const char *SemName);
    bool LIBENTRY open(const char *SemName);
    void LIBENTRY close(void);
    void LIBENTRY enabletimer(const int Time);
    void LIBENTRY disabletimer(void);
    int  LIBENTRY waitforevent(void);

    inline bool isenabled(void) { return(Enabled); }

  private:
    int   Interval;
    bool  Enabled;
};
#pragma pack(1)

#pragma pack(4)
class CTimeoutSemaphore : public CSemaphore{
  public:
    CTimeoutSemaphore();
    ~CTimeoutSemaphore();

    bool LIBENTRY open(const char *SemName);
    void LIBENTRY close(void);
    void LIBENTRY settimer(const int Time);
    bool LIBENTRY timerexpired(const int PauseTime);
    void LIBENTRY stoptimer(void);

  private:
    void *Timer;
};
#pragma pack(1)

#pragma pack(4)
class CMutexSemaphore {
  public:
    CMutexSemaphore();
    ~CMutexSemaphore();

    bool LIBENTRY create(const char *SemName);
    bool LIBENTRY createunique(const char *SemName);
    bool LIBENTRY open(const char *SemName);
    void LIBENTRY close(void);
    void LIBENTRY acquire(void);
    void LIBENTRY release(void);

  private:
    long  Semaphore;
    bool  Opened;
    char  Name[32];
};
#pragma pack()

extern CTimeoutSemaphore WaitKeySem;
extern CSemaphore        KbdTimer;

extern "C" void LIBENTRY releasekbdblock(int TimeToWait);
#endif /* ifdef __OS2__ */


#ifdef __OS2__
void LIBENTRY updatelinesnow(void);
void LIBENTRY setscreenupdateinterval(int Interval);
#else
  #define updatelinesnow()
  #define setscreenupdateinterval(i)
#endif

/****************************************************************************/

// these definitions/function are used only in the REPTXT.OBJ module

#pragma pack(1)
typedef struct {
  char Color;                          /* color to be displayed with text */
  char Str[80];                        /* text brought in from PCBTEXT */
} pcbtexttype;
#pragma pack()

typedef void (LIBENTRY pcbtextcb)(int TextNum, pcbtexttype *Buf);
void LIBENTRY setpcbtext(int TextNum, pcbtextcb *Func);

/****************************************************************************/


/*
#if defined(__cplusplus) || defined(__WATCOMC__)
}
#endif
*/

#endif  /* __PCBTOOLS_H */
#endif
