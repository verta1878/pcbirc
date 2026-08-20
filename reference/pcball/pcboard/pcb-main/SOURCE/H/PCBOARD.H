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


#ifndef H_PCBOARD
#define H_PCBOARD

#ifdef PCB_DEMO
  #undef FIDO      /* don't allow FIDO in demo version */
#endif

#ifndef COMM
  #undef FIDO      /* don't allow FIDO in local-only version */
  #undef PCBCOMM
#endif

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif

#ifndef H_NEWDATA
  #include <newdata.h>
#endif

#ifndef H_DOSFUNC
  #include <dosfunc.h>
#endif

#if __TURBOC__ < 0x300
  #define _FARDATA_
#elif (defined(__LARGE__) || defined(__COMPACT__)) && ! defined(LIB)
  #define _FARDATA_ _FAR_
#else
  #define _FARDATA_
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


#define NUMNODES (PCB_MAXNODES+1)

enum {PCB_BLUE=9, PCB_GREEN, PCB_CYAN, PCB_RED, PCB_MAGENTA, PCB_YELLOW, PCB_WHITE };

#ifndef LIB
enum {NOHELP,HLP_A,HLP_B,HLP_C,HLP_D,HLP_E,HLP_F,HLP_G,HLP_H,HLP_I,HLP_J,HLP_K,HLP_L,HLP_M,HLP_N,HLP_O,HLP_P,HLP_Q,HLP_R,HLP_S,HLP_T,
  HLP_U,HLP_V,HLP_W,HLP_X,HLP_Y,HLP_Z,HLP_BROADCAST,HLP_CHAT,HLP_CMENU,HLP_MSG,HLP_ENDR,HLP_LANG,HLP_NEWS,HLP_OPEN,HLP_QWK,HLP_REP,HLP_SEC,HLP_MAIN,HLP_MORE,
  HLP_MOREXFER,HLP_FULLSCRN,HLP_SRCH,HLP_TS,HLP_SEL,HLP_FLAG,HLP_REG,HLP_RM,HLP_TEST,HLP_USER,HLP_WHO,HLP_REPEAT,HLP_ALIAS,
  HLP_1,HLP_2,HLP_3,HLP_4,HLP_5,HLP_6,HLP_7,HLP_8,HLP_9,HLP_10,HLP_11,HLP_12,HLP_13,HLP_14,HLP_15,HLP_TEXT };
#else
enum {NOHELP,HLP_MORE};
#endif

enum {EXIT_NORMAL=0,EXIT_EVENT,EXIT_DOOR,EXIT_REMOTEDOS,EXIT_PACK,EXIT_GOODBYE,
  EXIT_RECYCLE,EXIT_FAX,EXIT_NC,EXIT_FIDO,EXIT_PPLRECYCLE,EXIT_ERROR=99};

/* These enums are for the Status.LogginIn variable                          */
/* if the variable is anything but zero, then we're handling a log in        */
/* but we only want to check for fido if the WATCHFORFIDO bit is set and     */
/* likewise, we only want to check for MD5 if the WATCHFORMD5 bit is set.    */
/* if the Status.LoggingIn variable==NOTNOW, then either a caller is logged  */
/* in already, or we're simply not processing a login right now              */
enum {NOTNOW=0, LOGINNOW=1, WATCHFORFIDO=2, WATCHFORMD5=4};

enum {K_BKSPC=8,K_TAB,K_LFEED,K_RET=13,K_ESC=27,K_SPACE=32,K_SCLN=59,K_HOME=1071,K_UP,K_PGUP,K_LEFT=1075,K_RIGHT=1077,K_END=1079,K_DOWN,K_PGDN,K_INS,K_DEL,K_CLEFT=1115,K_CRIGHT,K_CEND=1117 };
enum {CTRL_A=1,CTRL_B,CTRL_C,CTRL_D,CTRL_E,CTRL_F,CTRL_G,CTRL_H,CTRL_I,CTRL_J,CTRL_K,CTRL_L,CTRL_M,CTRL_N,CTRL_O,CTRL_P,CTRL_Q,CTRL_R,CTRL_S,CTRL_T,CTRL_U,CTRL_V,CTRL_W,CTRL_X,CTRL_Y,CTRL_Z};
#define EXTLOWKEY  K_HOME
#define EXTHIGHKEY K_CEND

/* note: YESNO=2049 ... the input functions figure out that ECHODOTS and WORDWRAP would never be used together */
/* note: NOTBLANK=32768U, same as AUTO ... the displaypcbtext() function figures out this value would never be used as AUTO */
typedef enum {DEFAULTS=0,ECHODOTS=1,FIELDLEN=2,UPCASE=4,STACKED=8,ERASELINE=16,NEWLINE=32,LFBEFORE=64,LFAFTER=128,LOGIT=256,LOGITLEFT=512,GUIDE=1024,WORDWRAP=2048,YESNO=2049,NOCLEAR=4096,BELL=8192,HIGHASCII=16384,AUTO=32768U,NOTBLANK=32768U} displaytype;

typedef enum {NOCHANGE,FORCENONSTOP,FORCECOUNTLINES} startdisplaytype;
typedef enum {ACTSHOW, ACTBEGIN, ACTEND, ACTSUSPEND, ACTRESUME} acttype;
typedef enum {MOREPROMPT,PRESSENTER } moretype;
typedef enum {OFFLINE, LOCAL, REMOTE } onlinetype;
typedef enum {NONE, BLANKLINE, HELP, NAME,CONFREG, COMMENT, MODEMSTATS } statlinetype;
typedef enum {CALLWAIT_EXIT, CALLWAIT_SYSOP, CALLWAIT_LOCALUSER, CALLWAIT_CALLER, CALLWAIT_ONLINE, CALLWAIT_RECYCLE } calltype;
typedef enum {USER, SYSOP } usertype;
typedef enum {SHOW, HIDE } showtype;
typedef enum {INPUTOKAY, INPUTREFUSEANSWER } inputtype;
typedef enum {PROGRAM, USERSELECT } callfromtype;
typedef enum {LOGON, NLOGOFF, ALOGOFF, REMOTEDOS, FILEVIEWSHELL, DOOR, RUNSETUP, RUNFILER, RUNSM, RUNEVENT, RUNFAX} logtype;
typedef enum {LEFTJUSTIFY, SPACERIGHT } padtype;
typedef enum {FNAM, FSIZ, FDTE, FDSC, HEAD, TEXT, DUPE, DLTD, OFLN, NEWF} ColorType;
typedef enum {NEWTIME, CONFTIME, CREDTIME, PREVTIMEON, ACCTTIME } timereasontype;
typedef enum {SHELLDIRECT, SHELLVIACOMMAND } shelltype;
typedef enum {CHECKFSEC, CHECKUPSEC, CHECKVIEW, NOCHECKING } checksectype;
typedef enum {NOWILDSNOINFORM=0, INFORM=1, ALLOWWILDCARDS=2, ALLOWTEXTONLY=4, ALLOWSTARDOTSTAR=8} verifytype;
typedef enum {INCREASE,DECREASE } adjusttype;
typedef enum {NOALTERNATE=0,GRAPHICS=1,SECURITY=2,LANGUAGE=4,WELCOME=8,RUNMENU=16,RUNPPL=32,CMDFILE=64,DISABLESUBFILES=128 } displayfiletype;
typedef enum {UPDATE,LOGOFF,RESETSTATS,SETTYPE } statupdatetype;
typedef enum {SCANONLY, ALLCONF } choosetype;
typedef enum {NOUPDATE,UPDATEUSER} addsectype;
typedef enum {NOBUFFER,CHECKBUFFER} kbdhittype;
typedef enum {CHECKUPLDS=1,CHECKDNLDS=2,ADDFILE=4,REPORTONLY=8,IGNOREPRIVATE=16,SHOWDUPES=32} scandltype;
typedef enum {FAILED=0,UPOKAY,DNOKAY,REMOVE} trntype;
typedef enum {NOTCAP,MSGCAP,QWKCAP} captype;
typedef enum {DOWNLOAD,UPLOAD} xfertype;
typedef enum {KEEP,THROWAWAY} savekeep;
typedef enum {ACT_DISABLED=0,ACT_TRACKONLY,ACT_ENFORCE} actstattype;

typedef enum {
  NOCALLER=' ',AVAILABLE='A',OUTINDOS='B',CHATWITHSYSOP='C',INADOOR='D',
  ENTERMESSAGE='E',FILEVIEW='F',GROUPCHAT='G',HANDLEMAIL='H',LOGOFFPENDING='L',
  NODEMESSAGE='M',RUNNINGEVENT='N',LOGINTOSYSTEM='O',PAGINGSYSOP='P',
  RUNONCONNECT='Q',RECYCLEBBS='R',ANSWERSCRIPT='S',TRANSFER='T',
  UNAVAILABLE='U',DROPDOSDELAYED='W',DROPDOSNOW='X'
} netstatustype;


/* Some functions which take ENUM'd parameters and OR them together declare  */
/* their parameter types using the typedef'd name of the enum values.  I     */
/* like this, but Watcom does not.  These following lines of code help solve */
/* this problem and keep the compiler happy.                                 */
#ifdef __cplusplus
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

/* Used to get the status of the keyboard */
#define SCROLL  0x10

#ifdef LIB
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
#else
#pragma pack(4)
typedef struct {
  bool         PrintLog;               /* TRUE if the 'log' printer is turned on */
  bool         Printer;                /* TRUE if the 'capture' printer is turned on */
  bool         Alarm;                  /* TRUE if the alarm is toggled on */
  bool         PageBell;               /* TRUE if the page bell is toggled on */
  bool         ForceScreenOff;         /* TRUE if the screen was FORCED off by pressing F9 */
  bool         ScrollOn;               /* TRUE if the scrollback buffer is allocated */
  bool         InScrollBack;           /* TRUE if we're in the scrollback buffer (viewing it) */
  bool         Paged;                  /* TRUE if caller paged the sysop */
  bool         LoggingUserOff;         /* TRUE if we're in the process of logging the user off */
  bool         LoggingIn;              /* TRUE if we're in the process of loggin in */
  bool         TimeAdjustedForEvent;   /* TRUE if the time online has been adjusted */
  bool         BackFromDoor;           /* TRUE if just returned from a DOOR */
  bool         SwapDoor;               /* TRUE if SWAP enabled for DOOR shell */
  #ifdef __OS2__
  bool         Os2Door;                /* TRUE if DOOR.CMD should be used instead of DOOR.BAT */
  bool         DisableUserNetUpdate;   /* TRUE if usernet thread should stop updating temporarily */
  #endif
  bool         Kbd;                    /* TRUE if last char was input from the keyboard */
  bool         ColorizeDirFiles;       /* TRUE if directories are to be colored */
  bool         InChat;                 /* TRUE if we are currently in sysop chat */
  bool         OnlineUpdate;           /* TRUE if we are in the online update function */
  bool         EnteringMessage;        /* TRUE if we are inside the message editor */
  bool         FileAttach;             /* TRUE if receiving a file attachment for a msg */
  bool         InFileListing;          /* TRUE if currently reading a DIR file */
  bool         InFileListingLastMore;  /* TRUE if last more? prompt of file listing */
  bool         FileXfer;               /* TRUE if currently transfering a file */
  bool         HeaderScan;             /* TRUE if currently performing a 5)Header Scan */
  bool         QuickScan;              /* TRUE if currently performing a Q)uick Scan */
  bool         CmdPrompt;              /* TRUE if currently at the command prompt */
  bool         ReplyCommand;           /* TRUE if currently performing a REPLY command */
  bool         Batch;                  /* TRUE if currently performing a BATCH file transfer */
  bool         FullScreen;             /* TRUE if currently in Full Screen message editor */
  bool         Ringing;                /* TRUE if the phone is ringing during a non-busy session */
  bool         RingAlert;              /* TRUE if need to detect rings */
  bool         NewUser;                /* TRUE if caller is a new user */
  bool         LocalStats;             /* TRUE if statistics are node-specific */
  bool         MakeUserSys;            /* TRUE if USERS.SYS should be created */
  bool         SendBlts;               /* TRUE if sending BLT files */
  bool         ReadCapture;            /* TRUE if capturing messages */
  bool         XferCapture;            /* TRUE if need to transfer capture file */
  bool         Capture;                /* TRUE if currently capturing to disk */
  bool         CapBye;                 /* TRUE if auto-goodbye after capture */
  bool         CapAsk;                 /* TRUE if need to ask before downloading */
  bool         ZipCap;                 /* TRUE if need to zip cap file */
  bool         AutoLogoff;             /* TRUE if system is automatically logging a user off */
  bool         Poff;                   /* TRUE if @POFF@ was issued */
  bool         KbdTimedOut;            /* TRUE if keyboard timer expired */
  bool         SessionTimedOut;        /* TRUE if session timer expired */
  bool         TerseMode;              /* TRUE if in PCBTEXT terse mode */
  bool         MsgRead;                /* TRUE if reading messages */
  bool         CheckForMail;           /* TRUE if need to check for new mail */
  bool         QwkUpload;              /* TRUE if receiving .REP packet */
  bool         MsgUpload;              /* TRUE if receiving MSG#.$$$ file */
  bool         UseAlias;               /* TRUE if want alias when in alias conf */
  bool         DisableColor;           /* TRUE if @X-code interpretation should be turned off */
  bool         UseSingleLines;         /* TRUE if want to see only single-line file descriptions */
  bool         CapFileExport;          /* TRUE if capture file is ALT-F export */
  DOSFILE      CapFile;                /* file for capture */
  char         CapFileName[66];        /* name of file used for capture */
  char         SysopFlag;              /* 'N' of Sysop Next, 'X' if Exit to DOS or ' ' */
  char         PackFlag;               /* 254 = msgbase, 255 = users, ' ' = nothing */
/*char         PrnNumLinesPrinted; */  /* number of lines sent to printer */
  char         NumSecViolations;       /* the number of security violations */
  char         FirstName[26];          /* the first name of the caller */
  char         SysopName[26];          /* the first name of the sysop */
  char         DisplayName[26];        /* real name or sysop name */
  char         LastPassword[13];       /* last password entered by a user */
  char         LastDateOnStr[9];       /* from users file - last date on */
  char         DoorName[20];           /* Name of DOOR being executed */
  #ifdef __OS2__
  char         DoorPath[60];           /* path to DOOR being executed */
  #endif
  char         ErrorLevel;             /* DOS errorlevel: 0=ok, 1=event, 2=door, 3=remote dos, 4=pcbpack, 99=error */
  char         DefaultProtocol;        /* if non-zero, override user's default for xfer */
  long         LogoffFile;             /* size of logoff file if displayed */
  long         CallerNumber;           /* number of caller at login time */
  long         MemorizeNum;            /* number of message that is memorized */
  statlinetype StatLine;               /* type of help displayed on status line */
  statlinetype OldStatLine;            /* previous value of StatLine */
  logtype      Logoff;                 /* type of logoff that occured */
  usertype     User;                   /* sysop or user */
  long         UserRecNo;              /* record number in the users file */
  short        SuspendMinute;          /* time when activity should be suspended */
  short        EventMinute;            /* time when event should be run */
  short        EndEventMinute;         /* last minute possible for event to run */
  short        LogonMinute;            /* time when the caller logged on in minutes since midnight */
  short        CreditMinutes;          /* time credit for uploads and sysop chat */
  unsigned short JulianLogonDate;        /* date when the caller logged on (julian) */
  unsigned short CtodLogonDate;          /* date caller logged on in ctod() format */
  unsigned short LatestDate;             /* date of latest file found */
  long         LastDateTime;           /* combined last date & time on */
  char         LogonDate[9];           /* date when the caller logged on (MM/DD/YY) */
  char         LogonTime[9];           /* time when the caller logged on (HH:MM:SS) */
  unsigned short Conference;           /* current conference number */
  char         CurSecLevel;            /* current security level */
  bool         WatchForStopChar;       /* used to signal when StopChar is monitored */
  bool         Available;              /* usernet avail/nonavail status */
  char       * TaggedText;             /* pointer to text tagged in the scrollback buffer */
  char       * TagPointer;             /* pointer to next character in TaggedText */
  char       * TaggedFileNames;        /* pointer to filenames tagged in a DIR scan */
  char       * CurDirName;             /* pointer to the current DIR title */
  short        CurDirNum;              /* number of the current DIR file */
  short        NumTaggedFiles;         /* Number of tagged filenames in *TaggedFileNames */
  #ifndef PCB_DEMO
  long         FoundCrc;               /* CRC value */
  #endif
  char         StatusLine1;            /* line number of top status line */
  char         StatusLine2;            /* line number of top status line */
  unsigned short StartConf;            /* starting conference number for a Read All command */
  short        DirColors[NEWF+1];      /* array of colors used in coloring directories */
  short        TempAddSec;             /* added security from conf sysop, temp sysop, alt-f# changes */
  short        ConfAddSec;             /* added security from joining a conference */
  short        ConfAddTime;            /* added time from joining a conference */
  short        MaxTimeAllowed;         /* caller's maximum time available on the system */
  short        PwrdTimeAllowed;        /* Time allowed based on PWRD file */
  short        MaxKBytesAllowed;       /* the maximum number of Kbytes a caller can download */
  short        BatchLimit;             /* the maximum number of files that can be batch downloaded */
  long         BytesRemaining;         /* bytes left that caller can download */
  char         Language;               /* number of language chosen */
  char         MultiLangExt[5];        /* filename extension for multilingual operation */
  char         MainMsgFile[32];        /* name and path of the main board message base */
  char         Version[66];            /* text string describing PCBoard version */
  char         NodeStr[14];            /* text string describing PCBoard version */
  char         LastView[13];           /* name of last file viewed with "F V" */
  bool         AppendText;             /* indicate DisplayText should be appended */
  char         DisplayText[80];        /* special @-variable display text */
  char         SearchInput[80];        /* used for parsersearch() routine */
  char         SearchText[80];         /* used for parsersearch() routine */
  char         CmdLst[32];             /* current CMD.LST file */
  char         RipVersion[7];          /* RIP Version information */
  pcbconftype  CurConf;                /* current conference data */
  unsigned     FileRatioCredits;       /* credits against File Ratio */
  unsigned     KByteRatioCredits;      /* credits against Byte Ratio */
  long         FileRatio;              /* 0=disabled, div by 10 to get ratio */
  long         ByteRatio;              /* 0=disabled, div by 10 to get ratio */
  long         TotalFileLimit;         /* 0=disabled, max # of files to d/l  */
  long         TotalKByteLimit;        /* 0=disabled, max # of K-bytes to d/l*/
  bool         EnforceTimeLimit;       /* enforce DAILY time limit */
  bool         AllowAlias;             /* allow caller to change his name */
  bool         ForceReadMail;          /* force to read waiting maile */
  bool         MultipleLogins;         /* allow same name on 2 or more nodes */
  char         MsgCapCount;            /* num times C/D/Z capture downloaded */
  char         QwkCapCount;            /* num times QWK capture downloaded   */
  char         PwrdFailureCount;       /* num times failed conf pwrd */
  short        PrevTimeOn;             /* minutes used, previous same-day session */
  long         LastMsgNum;             /* number of last message saved */
  long         LastMsgOffset;          /* offset of last message saved */
#ifdef FIDO
  bool         FoundFido;              /* TRUE if FIDO was detected */
  bool         AllowHumanCallers;      /* TRUE if human callers can connect */
  bool         AllowFileRequests;      /* TRUE if file requests are allowed */
  unsigned long FidoEndEvent;          /* exact time of current fido window end */
  unsigned long FidoNextEvent;         /* exact time of next fido window start */
#endif
#ifdef PCB152
  actstattype  ActStatus;              /* OFF, TRACKING, ENFORCED */
  bool         UseDropSecLevel;        /* TRUE if need to use Account.DropSecLevel */
  double       Balance;                /* Current Account Balance */
  double       TempCharge;             /* Temporarily reduce balance by this much */
  short        FsecMultiplier;         /* FSEC multiplier */
  long         ConfJoinTime;           /* Time when last conf was joined */
  double       DoorChargePerUse;       /* Charge to use the door */
  double       DoorChargePerMin;       /* Charge per minute to use the door */
  char         SysopLevel;             /* the sysop's security level */
#endif
  long         HideAliasStartTime;     /* time when alias hidding started */
} statustype;
#pragma pack()
#endif

#pragma pack(4)
typedef struct {
  int  Internal:1;      /* TRUE if protocol is internal */
  int  Batch:1;         /* TRUE if capable of BATCH */
  int  ErrCorrReq:1;    /* TRUE if requires MNP */
  int  PortOpen:1;      /* TRUE if need to leave port open */
  int  LockLines:1;     /* TRUE if need to lock status lines */
  int  BiDirect:1;      /* TRUE if bi-directional */
  int  BlockSize;       /* block size of the protocol */
  char Desc[50];        /* description of the protocol */
} prottype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char LastCaller[54];
  char Time[6];
  long NewMsgs;
  long NewCalls;
  long TotalUp;
  long TotalDn;
  bool LocalStats;
} stattype;
#pragma pack()

#pragma pack(4)
typedef struct {
  char *Word;
} wordtype;
#pragma pack()

#pragma pack(1)
typedef struct {
  unsigned short Version;     /* PCBoard version number */
  unsigned short NumOfNodes;  /* Number of nodes supported in the USERNET.DAT */
  unsigned short SizeOfRec;   /* The size of each node record */
} usernethdrtype;
#pragma pack()

#pragma pack(1)
typedef union {
  long DateTime;
  struct {
    uint Time;            /* number of seconds past midnight divided by 2 */
    uint Date;            /* julian date */
  } Split;
} updttype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char     Status;        /* node status */
  bool     MailWaiting;   /* true if msg posted */
  uint     Pager;         /* node number of pager */
  char     Name[26];      /* caller's name */
  char     City[25];      /* caller's city */
  char     Operation[49]; /* current operational text */
  char     Message[80];   /* broadcast message text */
  char     Channel;       /* channel number of pager */
  updttype LastUpdate;    /* hour:min:sec of last update on this record */
} nodetype;
#pragma pack()

#define USERNETFLAGSIZE ((PCB_MAXNODES+7)/8)
#define USERNETSTART    (sizeof(usernethdrtype) + (USERNETFLAGSIZE*2))
#define USERNETFLAGS    (sizeof(usernethdrtype) + USERNETFLAGSIZE)

#pragma pack(1)
typedef struct {
  char Name[18];
  char Str[58];
  char CrLf[2];
} chattype;
#pragma pack()

#pragma pack(4)
typedef struct {
  char BoxColor;
  char TitleColor;
  char HiLite;
  char LoLite;
  char Shadow;
  char HitColor;
  char BarColor;
  char VersionColor;
  char MemColor;
  char HandleColor;
  char v14Color;
} colortype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char    FullPath[66];
  char    Name[13];
  long    Size;
  long    Seconds;
  unsigned short CPS;
  uint    Success;     // holds values of trntype
//trntype Success;
//#if defined(__OS2__) && defined(__BORLANDC__)
//  char  Filler1;
//#endif
  bool    HasDesc;
  bool    Checked;
  bool    Found;
  bool    FsecOkay;
  bool    FreeFile;
  bool    FreeTime;
  bool    MovedToWork;
  bool    Failed;
  char    OldName[13];
  bool    Duplicate;
  uint    CapType;      // holds values of captype
//captype CapType;
//#if defined(__OS2__) && defined(__BORLANDC__)
//  char  Filler2;
//#endif
  short   Multiplier;
  bool    NoCost;
  char    Reserved[11];
} spectype;
#pragma pack()

#pragma pack(4)
typedef struct {
  int      SuccessfulDnldFiles;
  int      SuccessfulUpldFiles;
  long     SuccessfulDnldBytes;
  long     SuccessfulUpldBytes;
  unsigned CPSdn;
  unsigned CPSup;
  unsigned CPSboth;
} xfervartype;
#pragma pack()

#pragma pack(4)
typedef struct {
  bool     Screen;            /* TRUE if the screen should be displayed */
  bool     GraphicsMode;      /* TRUE if caller requested graphics */
  bool     RipMode;           /* TRUE if caller requested RIP graphics */
  bool     WatchKbdClock;     /* FALSE if we want to ignore the KEYBOARD clock */
  bool     WatchSessionClock; /* FALSE if we want to ignore the SESSION clock */
  unsigned KbdTimer;          /* Number of clock ticks for keyboard timer */
  #ifndef LIB
  unsigned NetTimer;          /* Number of clock ticks for usernet timer */
  #endif
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
  #ifndef LIB
  char       ConnectString[128]; /* string returned by modem */
  char       CallerIDString[128];/* string returned by modem for CallerID */
  #endif
} asytype;
#pragma pack()

#pragma pack(4)
typedef struct {
  bool         AbortPrintout;    /* TRUE if system should abort a printout */
  bool         CountLines;       /* TRUE if system is currently counting lines displayed */
  unsigned char NumLinesPrinted; /* a count of the number of lines displayed since last pause */
  #ifndef LIB
  bool         NonStopDuringCmd; /* TRUE if current command should be in non-stop mode */
  #endif
  bool         AutoMore;         /* TRUE if auto answer more? prompt after 10 seconds */
  bool         WasNonStop;       /* TRUE if last printout was in non-stop mode */
  bool         WasAborted;       /* TRUE if last printout was aborted */
  unsigned char PageLen;         /* caller's current page length */
  bool         ShowOnScreen;     /* TRUE if data should be shown on screen */
  bool         Break;            /* TRUE if current display can be interrupted at the More Prompt */
  char         DefaultColor;     /* default caller as chosen by the sysop */
  char         NumScrnLines;     /* number of lines on the local display screen */
  #ifndef LIB
  bool         ClearScrnWhenFull;/* Clear the screen after a more? prompt */
  char         KeepLines;        /* number of lines to guard at the top */
  char         NumCharsPrinted;  /* keep track of # of chars on a line in capture file */
  #endif
} disptype;
#pragma pack()

#define LOGRECLEN      64
#define LOGTEXTLEN     LOGRECLEN-2
#define MAXLOGRECS     256
#define MAXSESSIONRECS 100

#pragma pack(1)
typedef struct {
  char Text[LOGRECLEN-2];
  char CrLf[2];
} logrectype;
#pragma pack()

/*
#ifdef __cplusplus
extern "C" {
#endif
*/


/* account.cpp */
bool LIBENTRY currentlyinpeak(void);
void LIBENTRY creditcurrency(char *Str, double Value);
void LIBENTRY calculatebalance(void);
bool LIBENTRY insufficientcredits(double Charge, double TempCharge);
void LIBENTRY checkaccountbalance(void);
long LIBENTRY minutesused(long StartTime);
void LIBENTRY chargeformessage(char MsgStatus, bool Echo, char *To);
void LIBENTRY chargetimeinconf(void);
void LIBENTRY chargetimeonline(int CurrentMinute, int MinutesOnline);
double LIBENTRY recordusage(char *Activity, char *SubActivity, double UnitCost, long Quantity, double *Storage);

/* blt.c */
int  LIBENTRY numblts(void);
void LIBENTRY checkbulletins(void);
int  LIBENTRY showblt(int BltNum);
void LIBENTRY bltmenu(int NumTokens);

/* callwait.c */
void     LIBENTRY processconnectstring(char *ComBuffer);
void     LIBENTRY logcalleridstring(void);
void     LIBENTRY offhook(void);
void     LIBENTRY showmodemdata(char *Str);
void     LIBENTRY initcommwindow(showtype Type);
void     LIBENTRY reseterror(void);
bool     LIBENTRY errorcorrected(char *Str);
calltype LIBENTRY callwait(void);

/* capture.c */
void LIBENTRY toggleprinter(void);
int  LIBENTRY opencapturefile(char *FileName);
void LIBENTRY closecapturefile(void);
void LIBENTRY setcapture(bool On);
void LIBENTRY openprinter(void);

/* chat.c */
#ifdef LIB
void LIBENTRY sysopchat(void);
#else
void LIBENTRY sysopchat(int NumTokens);
#endif

/* command.c */
void LIBENTRY command(void);

/* confrnce.c */
void LIBENTRY makeconfstr(char *Str, unsigned ConfNum);
void LIBENTRY makeconfstrsys(char *Str, unsigned ConfNum);
void LIBENTRY makeconfreadstr(char *Str, unsigned ConfNum);
void LIBENTRY confdetails(addsectype Action);
void LIBENTRY aliasnames(void);
void LIBENTRY processjoin(bool ShowNews, bool QuickLogon, bool FullStats);
void LIBENTRY showconfjoined(unsigned short Num, pcbconftype *Conf, bool ShowJoined, bool ShowIntro);
void LIBENTRY joinconfcommand(int NumTokens);
void LIBENTRY abandon(int NumTokens);
void LIBENTRY chooseconferences(int NumTokens, char _FAR_ *Conf, long _FAR_ *MsgPtr, choosetype Flag);
void LIBENTRY selectconf(int NumTokens);
void LIBENTRY tersemodeconfinfo(unsigned ConfNum);

/* copyfile.c */
int LIBENTRY pcbcopyfile(char *Srce, char *Dest, bool CheckForEOF, bool ShowActivity, char *LockDrives, bool CopyAttributes);
int LIBENTRY pcbmovefile(char *Srce, char *Dest);

/* crc32.c */
unsigned long LIBENTRY crc32buf(char *buf, unsigned len);
unsigned long LIBENTRY crc32file(char *file);

/* dir.c */
void LIBENTRY showfile(int NumTokens);
bool LIBENTRY needtoresyncdirindex(char *FileName, DOSFILE *IdxFile);
void LIBENTRY updatedirindex(char *FileName, DOSFILE *IdxFile, char *Name, char *Str, long Offset);
int  LIBENTRY numdirs(char *DirList);
void LIBENTRY dirmenu(int NumTokens);
void LIBENTRY filescan(int NumTokens);
void LIBENTRY newfilescan(int NumTokens);
void LIBENTRY zippyscan(int NumTokens);

/* diz.c */
bool LIBENTRY checkfordizfile(char *ZipFileName);

/* dlpath.c */
void LIBENTRY flagfiles(int NumTokens);
void LIBENTRY flagnamedfile(char *Path, char *NameOnDisk, char *NewName);
bool LIBENTRY filesflagged(void);
int  LIBENTRY scandlpaths(SCANDLTYPE Flags, char *FileNames);

/* doors.c */
void LIBENTRY remotedos(int NumTokens);
void LIBENTRY setpcbenv(DOSFILE *Out, bool PlaceInEnv);
int  LIBENTRY searchdoorlist(char *Text, int NumTokens);
void LIBENTRY opendoor(int NumTokens);
void LIBENTRY logindoor(void);
void LIBENTRY runshelldoor(void);
void LIBENTRY scandoorsforcharges(char *DoorName);

/* dostime.c */
long LIBENTRY dosgetlongtime(void);
long LIBENTRY doselapsedtime(long StartTime);
#ifdef __OS2__
void LIBENTRY settimer(int Which, int Hundredths);
int  LIBENTRY gettimer(int Which);
bool LIBENTRY timerexpired(int Which);
#endif


/* filelist.c */
int  LIBENTRY allocatefilelist(xfertype Mode);
void LIBENTRY reloadfilelist(void);
void LIBENTRY deallocatefilelist(savekeep Action);
int  LIBENTRY getfilespec(int Num, spectype *File);
int  LIBENTRY putfilespec(int Num, spectype *File);
void LIBENTRY removefilesfromlist(void);
void LIBENTRY movefilesinlist(void);
void LIBENTRY removefinishedfilesfromlist(void);
int  LIBENTRY findfilename(char *Name);
int  LIBENTRY addfiletolist(char *Name, char *Path, long Size, bool ForceIntoList, bool ShowDupe);
unsigned  LIBENTRY averagecps(long Bytes, trntype TranType);
char * LIBENTRY allocatefiledesc(void);
void LIBENTRY deallocatefiledesc(void);
int  LIBENTRY getfiledesc(int Num, char *Desc);
int  LIBENTRY putfiledesc(int Num, char *Desc);
char * LIBENTRY slowdrive(char *SlowDrives, char DriveLetter);

/* files.c */
void   LIBENTRY readpwrdfile(int PreviousTimeOn);
void   LIBENTRY readpwrdfileadjustsecurity(adjusttype);
bool   LIBENTRY readtcanfile(char *Name);
bool   LIBENTRY isprotvalid(char Letter);
int    LIBENTRY protoffset(int Letter);
bool   LIBENTRY isprotbatch(int Letter);
char * LIBENTRY protdesc(int Letter);
void   LIBENTRY readprotfile(void);
bool   LIBENTRY dosreservedword(char *Name);
int    LIBENTRY fsecokay(char *FullPath, char *Name, checksectype CheckSec, char *AltPath);
int    LIBENTRY checkforalternatelist(char *FoundName, char *OriginalName, int RecordSize);
void   LIBENTRY substenvinfile(char *FileName, int Len);
unsigned char LIBENTRY checkenvfileexist(char *FileName, int Len);
int    LIBENTRY displaycmdfile(char *Command);
bool   LIBENTRY findcmdfile(char *Command, char *FileName);
void   LIBENTRY flagfilefordownload(char *FileName);
int    LIBENTRY displayfile(char *Name, DISPLAYFILETYPE Type);
void   LIBENTRY getalternatename(char *FileName, DISPLAYFILETYPE Type, int RecordSize);

/* help.c */
int  LIBENTRY displayhelpfile(int HelpNum);
void LIBENTRY helpcommand(int NumTokens);

/* init.c */
#ifdef KBD3
void LIBENTRY setint3(void);
void LIBENTRY restoreint3(void);
#endif
void LIBENTRY setkbdtimer(int Minutes);
void LIBENTRY setdefaultpriority(void);
void LIBENTRY adjustpriority(int PriorityDelta);
void LIBENTRY initnontransient(void);
void LIBENTRY init(void);
void LIBENTRY backfromdos(void);
int  LIBENTRY findbat(char *FoundPath, char *SearchName);
void LIBENTRY stuffkbdforcaller(void);
calltype LIBENTRY reload(void);

/* inkey.c */
void LIBENTRY initkbdbuffers(void);
void LIBENTRY warntime(void);
void LIBENTRY renewkbdtimer(void);
void LIBENTRY turnkbdtimeron(void);
void LIBENTRY clearkbdbuffer(void);
void LIBENTRY clearpplbuffer(void);
void LIBENTRY stuffbuffer(int Key);
void LIBENTRY stuffbufferstr(char *Str);
bool LIBENTRY filestuffinuse(void);
char LIBENTRY bytesleftinkbdbuffer(void);
char LIBENTRY bytesleftinpplbuffer(void);
void LIBENTRY stuffpplbuffer(char *Str);
void LIBENTRY stuffkbdwithfile(char *FileName);
void LIBENTRY getlocalinput(char *Str, int TextNum, int FieldLen, char *Mask);
void LIBENTRY closeautostuff(void);
int  LIBENTRY callwaitinkey(char *ComBuffer);
int  LIBENTRY kbdhit(kbdhittype Action);
bool LIBENTRY typeahead(void);
bool LIBENTRY ppltypeahead(void);
void LIBENTRY gettypeaheadstr(char *Str, int MaxLen);
void LIBENTRY checktime(void);
void LIBENTRY watchsystemfunctions(void);
int  LIBENTRY commportinkey(void);
int  LIBENTRY kbdinkey(void);
int  LIBENTRY cinkey(void);
int  LIBENTRY waitforkey(void);
int  LIBENTRY waitforkey(int TimeToWait);
void LIBENTRY checkstatus(void);
void LIBENTRY watchstatuswhilesending(void);

/* input.c */
int  LIBENTRY wordbackward(char *Str, int Pos);
int  LIBENTRY wordforward(char *Str, int Pos, int FieldLen);

void LIBENTRY inputfield(char *Buffer, int PcbTextNum, int MaxLen, DISPLAYTYPE Display, int HelpNum, char *Mask);
void LIBENTRY inputfieldstr(char *Buffer, char *Prompt, int Color, int MaxLen, DISPLAYTYPE Display, int HelpNum, char *Mask);
int  LIBENTRY inputfieldint(int Default, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum);
long LIBENTRY inputfieldlong(long Default, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum);
inputtype LIBENTRY inputreqfield(char *Buffer, int PcbTextNum, int MaxLen, DISPLAYTYPE Display, int HelpNum, char *Mask);

/* index.c */
long LIBENTRY finduser(char *SrchName);
void LIBENTRY removeuser(char *SrchName);
int  LIBENTRY addusertoindex(char *Name, long RecNo);
long LIBENTRY findusersoundex(char *SrchName, char *FoundStr, long *StartRecord);

/* language.c */
void LIBENTRY getlanguage(int NumTokens);
int  LIBENTRY getdefaultlanguage(void);
int  LIBENTRY getsystemlanguage(void);
void LIBENTRY getlanguagespecifics(void);
int  LIBENTRY setlanguage(int LangNum);

/* log.c */
void LIBENTRY closecallerlog(void);
void LIBENTRY openlog(void);
void LIBENTRY repositionlogpointer(void);
void LIBENTRY writelog(char *Str, padtype Pad);
void LIBENTRY writedebugrecord(char *Str);
void LIBENTRY writenametolog(logtype Which);

/* logview.c */
void LIBENTRY viewcallerslog(int NumTokens);
void LIBENTRY nodeviewcallerslog(int NumTokens);
unsigned LIBENTRY searchlogfordoor(char *DoorName);

/* login.c */
void LIBENTRY logconnectstring(void);
void LIBENTRY login(void);

/* memory.c */
void LIBENTRY initbmalloc(void);
void * LIBENTRY bmalloc(unsigned Size);
void * LIBENTRY brealloc(void *p, unsigned NewSize);
void LIBENTRY bfree(void *p);
void LIBENTRY bfreeall(void);
void _FAR_ * LIBENTRY fbmalloc(long Size);
void LIBENTRY fbfree(void _FAR_ *p);
void LIBENTRY fbfreeall(void);

/* misc.c */
short LIBENTRY minutesleft(void);
short LIBENTRY minutesonline(void);
void LIBENTRY addtime(long Seconds, timereasontype Reason);
void LIBENTRY subtime(long Seconds, timereasontype Reason);
int  LIBENTRY numrandrecords(char *FileName, int RecSize);
void LIBENTRY checkpagelen(void);
int  LIBENTRY openfile(char *Name);
bool LIBENTRY passwordokay(char *Password, char *Input, int TextNum, int BadNum, int NumTries);
void LIBENTRY fileratio(char *Str);
void LIBENTRY byteratio(char *Str);
bool LIBENTRY asciionbinary(char *FileName);
void *LIBENTRY checkmalloc(unsigned Size, char *Str);
void LIBENTRY tickdelay(int Ticks);
bool LIBENTRY seclevelokay(char *Cmd, int SecLvlReq);
int  LIBENTRY option(wordtype *Options, char *Str, int NumOptions);
void LIBENTRY dispatch(char *Cmd, int SecLvlReq, int NumTokens, void (LIBENTRY *func)(int NumTokens));
void LIBENTRY addtext(char *Str, char *Text, int MaxChars);
bool LIBENTRY previousconfmatchfound(char *Str);
void LIBENTRY tersemodechecksum(char *Str, int Len);
void LIBENTRY queryfido(void);
void LIBENTRY checkforfidoresponse(void);
bool LIBENTRY invalidfilename(char *FileName, VERIFYTYPE Action);

/* msgenter.c */
void LIBENTRY removetokens(char *Str);
void LIBENTRY movebottom(void);
void LIBENTRY entermessage(int NumTokens);
void LIBENTRY entercomment(int NumTokens);
void LIBENTRY entermessagefromfile(unsigned short ConfNum, char *ToName, char *FromName, char *Subject, char Security, unsigned short PackDate, bool ReqReceipt, bool Echo, char *FileName);

/* msgread.c */
int  LIBENTRY finishqwkpacket(bool CreateControlDat);
void LIBENTRY removeqwkfiles(bool All);
void LIBENTRY cleanupmsgmemory(void);
void LIBENTRY backuppointers(void);
void LIBENTRY restorepointers(void);
void LIBENTRY killcommand(int NumTokens);
int  LIBENTRY killmessage(unsigned ConfNum, long Number);
void LIBENTRY unkill(int NumTokens);
long LIBENTRY curmsgnum(void);
long LIBENTRY highmsgnum(void);
long LIBENTRY lowmsgnum(void);
void LIBENTRY messageread(int NumTokens);

/* msgscan.c */
void LIBENTRY messagescan(int NumTokens);
void LIBENTRY scanmail(char *Command, bool LoginScan);
void LIBENTRY tersemodescanmail(void);

/* newchat.c */
void LIBENTRY newchat(void);
void LIBENTRY initchatfile(void);
void LIBENTRY exitchat(void);

/* node.c */
void LIBENTRY broadcast(int NumTokens);
void LIBENTRY displayusernet(int NumTokens);
void LIBENTRY chatcommand(int NumTokens);
void LIBENTRY logoffcommand(int NumTokens);
void LIBENTRY recyclecommand(int NumTokens);
void LIBENTRY dropdoscommand(int NumTokens);
bool LIBENTRY okaytohandlemail(void);
unsigned LIBENTRY foundinusernet(char *Name, char *Alias);
void LIBENTRY callcommand(int NumTokens);

/* cmds.c */
void LIBENTRY freecmds(void);
void LIBENTRY loadcmds(void);
bool LIBENTRY runcmds(char *Command, int NumTokens);
bool LIBENTRY runscriptwithparams(char *FileName);

/* ratio.cpp */
bool LIBENTRY checkratio(char *Name, unsigned long Up, unsigned long Dn, long New, long CheckRatio, int RatioTxt, int ExceededText);
bool LIBENTRY checkratio2(char *Name, double Up, double Dn, long New, long CheckRatio, int RatioTxt, int ExceededText);
long LIBENTRY totalbytesleft(void);

/* recycle.c */
void LIBENTRY loguseroff(logtype Type);
void LIBENTRY byecommand(void);
void LIBENTRY recycle(void);

/* screen.c */
int  LIBENTRY savescrnbuffer(void);
void LIBENTRY restscrnbuffer(void);
void LIBENTRY initdisplay(void);
void LIBENTRY turndisplayon(bool WriteToDisk);
void LIBENTRY turndisplayoff(bool WriteToDisk);
char * LIBENTRY meganum(int Num);
void LIBENTRY setripmouseregion(int Num, int X1, int Y1, int X2, int Y2, int FontX, int FontY, bool Invert, bool Clear, char *Text);
int  LIBENTRY checkscreenforfilenames(int StartLine, char *FileName);

/* script.c */
void LIBENTRY removecodes(char *Str);
int  LIBENTRY doquestionnaire(int Qnum);
void LIBENTRY scriptmenu(int NumTokens);
void LIBENTRY logoffscript(void);
void LIBENTRY logonscript(void);
void LIBENTRY newregscript(void);

/* scrlback.c */
void LIBENTRY turnoffscrollback(void);
void LIBENTRY turnonscrollback(void);
void LIBENTRY addtaggedtext(char *Str);
void LIBENTRY viewscrollback(void);

/* settings.c */
void LIBENTRY setpagelength(int NumTokens);
void LIBENTRY protfile(char Letter, bool BatchOnly);
void LIBENTRY setprotocol(int NumTokens);

/* shell.c */
char LIBENTRY spawndos(char *Command, char *Params, shelltype ShellType, int PriorityDelta, bool MinimizeSelf, bool MinimizeWindow, int WaitTime);
char LIBENTRY swapdos(char *Command, char *Params, shelltype ShellType, char *RetVal);
int  LIBENTRY performshell(char *Command, char *Params, shelltype ShellType, int PriorityDelta, bool MinimizeSelf, bool MinimizeWindow, int WaitTime);
void LIBENTRY savepath(void);
void LIBENTRY restorepath(void);
void LIBENTRY shelltodos(void);
void LIBENTRY runexportbat(char *FileName);
void LIBENTRY runcdlostbat(void);
void LIBENTRY checkforlogoffbat(void);
void LIBENTRY checkforlogonbat(void);
void LIBENTRY runslowdrivebat(char *FileName);
void LIBENTRY runfiler(void);
void LIBENTRY runmoni(void);
void LIBENTRY runsysmgr(void);
void LIBENTRY runsetup(void);
void LIBENTRY doscommand(int NumTokens);
int  LIBENTRY verifyfile(char *FullName, char *Name, char *UpDescName);
void LIBENTRY verifyfilecommand(int NumTokens);
int  LIBENTRY performfileview(char *FullPath, char *Name);
void LIBENTRY viewfile(int NumTokens);
void LIBENTRY zipcapfile(void);
int  LIBENTRY zipqwkfile(void);
int  LIBENTRY extractmessages(char *RepPacket, char *Msgs);
void LIBENTRY packmessages(int NumTokens);
void LIBENTRY packusers(int NumTokens);
void LIBENTRY shelltotextcommand(char *Str);

/* stats.c */
void LIBENTRY updatestats(int NewMsgs, int NewUpld, int NewDnld, statupdatetype Type);
void LIBENTRY getstatstype(void);
bool LIBENTRY checkstats(stattype *Stats, bool Refresh);
void LIBENTRY lastcaller(char *CallerName, bool Local);

/* status.c */
void LIBENTRY showstatusline(void);
void LIBENTRY redisplaystatusline(void);
void LIBENTRY showusernetscan(bool Scanning);
void LIBENTRY showphoneringing(void);

/* sys.c */
void  LIBENTRY makepcboardsys(void);
void  LIBENTRY readpcboardsys(void);
short LIBENTRY readpcbsysfornodenum(void);

/* token.c */
int    LIBENTRY tokenizestr(char *Command);
int    LIBENTRY tokenize(char *Command);
char * LIBENTRY getnexttoken(void);
void   LIBENTRY peekatnexttoken(char *Buffer, int MaxLen);
void   LIBENTRY savetokenpointer(char **Pointer);
void   LIBENTRY restoretokenpointer(char **Pointer);

/* transfer.c */
unsigned LIBENTRY numfilesflagged(void);
long LIBENTRY numbytesflagged(void);
int  LIBENTRY xferminutesleft(void);
long LIBENTRY numbyteslessignore(void);
long LIBENTRY bytesleft(void);
long LIBENTRY numsecsforxfer(long FileSize);
void LIBENTRY resetxfercounters(bool ForceReset);
long LIBENTRY freespace(char *Path);
void LIBENTRY shownamesizeandtime(int Num, int ProtocolBlockSize);
void LIBENTRY showbatchnames(int OldNumFiles, int BlockSize);
void LIBENTRY checkdlfiles(int StartNum);
void LIBENTRY autogoodbye(int Seconds);
void LIBENTRY send(int NumTokens);
void LIBENTRY receive(int NumTokens);
bool LIBENTRY proceedwithupload(void);
int  LIBENTRY xmodemsendfile(char *FileName);
int  LIBENTRY xmodemreceivefile(char *FileName);

/* usernet.c */
void LIBENTRY setattentionflags(unsigned NodeNum, bool On);
int  LIBENTRY writeusernetrecord(unsigned NodeNum, nodetype *Buf);
void LIBENTRY readusernetrecord(unsigned NodeNum, nodetype *Buf);
void LIBENTRY updateusernetrecord(unsigned NodeNum, nodetype *Buf);
bool LIBENTRY checkforusernetmatch(unsigned NodeNum, char *Name);
unsigned short LIBENTRY firstfreenode(unsigned StartNum);
bool LIBENTRY scanusernet(void);
void LIBENTRY clearusernet(void);
void LIBENTRY closeusernet(void);
void LIBENTRY createusernet(void);
int  LIBENTRY openusernet(void);
void LIBENTRY writeusernetstatus(char NetStatus, char *Operation);
void LIBENTRY hidealiasinusernet(void);
bool LIBENTRY canbeavailable(void);
void LIBENTRY usernetavailable(void);
void LIBENTRY setmsgflaginusernet(char *Name);

/* userstat.c */
void LIBENTRY showuserstatus(int NumTokens);

/* usrmaint.c */
void LIBENTRY onlineupdate(void);
void LIBENTRY modifyself(int NumTokens);
void LIBENTRY usermaint(int NumTokens);
void LIBENTRY tersemodeuserinfo(void);
void LIBENTRY getaddress(void);

/* version.c */
#ifdef PCB_DEMO
void LIBENTRY version(void);
#else
int  LIBENTRY version(int NumNodes, int Version);
#endif
long LIBENTRY serialnumber(void);
void LIBENTRY registrationcode(char *Str);
void LIBENTRY togglecheck(void);

/* display.c */
void LIBENTRY checkdisplaystatus(void);
void LIBENTRY startdisplay(startdisplaytype Action);
void LIBENTRY showactivity(acttype Type);
int  LIBENTRY sendtoprinter(char *Str, int StrLen);
#ifdef COMM
void LIBENTRY printcom(char *Str);
#endif
void LIBENTRY print(char *Str);
void LIBENTRY printnow(char *Str);
void LIBENTRY printcls(void);
void LIBENTRY moreprompt(moretype Type);
void LIBENTRY newline(void);
void LIBENTRY pagebreak(int NumLines);
void LIBENTRY freshline(void);
void LIBENTRY println(char *Str);
void LIBENTRY printdefcolor(void);
void LIBENTRY printcolor(int ColorNum);
void LIBENTRY backupdestructive(int NumColumns);
void LIBENTRY backupcleareol(int NumColumns);
void LIBENTRY backup(int NumColumns);
void LIBENTRY forward(int NumColumns);
void LIBENTRY cleareol(void);
void LIBENTRY checkforansi(void);
void LIBENTRY checkforrip(void);
void LIBENTRY showmessage(int TextNum);
void LIBENTRY movecursor(int X, int Y);

/* xlate.c */
void LIBENTRY printxlated(char *Str);
void LIBENTRY printfoundtext(char *Str);
void LIBENTRY xlatetext(char *New, char *Old);
int  LIBENTRY strlenminuscolorcodes(char *Str);

/* DOSxxxx.C */
void LIBENTRY registerdosopencb(void);
void LIBENTRY deregisterdosopencb(void);
void LIBENTRY registerdosdupcb(void);
void LIBENTRY deregisterdosdupcb(void);
void LIBENTRY registerdoscreatecb(void);
void LIBENTRY deregisterdoscreatecb(void);
void LIBENTRY registerdosreadcb(void);
void LIBENTRY deregisterdosreadcb(void);
void LIBENTRY registerdoswritecb(void);
void LIBENTRY deregisterdoswritecb(void);
void LIBENTRY registerdosclosecb(void);
void LIBENTRY deregisterdosclosecb(void);


#ifdef COMM
/* MODEM.C */
void LIBENTRY ASYNC_reopenport(void);
bool LIBENTRY sendmodemwaitokay(char *OutBuf, bool WatchKbd);
void LIBENTRY slowsendtomodem(char *Str);
int  LIBENTRY resetmodem(bool FullReset, bool WatchKbd);
void LIBENTRY waitforempty(int Seconds);
void LIBENTRY clearandwaitformodemempty(void);
void LIBENTRY sendbyte(char Byte);
void LIBENTRY sendstr(char *Str, int StrLen);
void LIBENTRY modemoffhook(void);
bool LIBENTRY initializemodem(showtype Show);
void LIBENTRY openmodem(showtype Show);
void LIBENTRY closemodem(bool TurnOffDTR);

#if defined(MULTIPORT) || defined(LIB)
#ifdef __OS2__
  #define turnonxmit()
#else
  extern void (LIBENTRY *turnonxmit)(void);
#endif
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
extern void (LIBENTRY *reopenport)(void);
extern void (LIBENTRY *disconnectmodem)(void);


#ifdef MULTIPORT
bool LIBENTRY istelnetviavmodem(void);
bool LIBENTRY isvmodem(void);
#endif


#ifndef LIB
extern void LIBENTRY (*reportcom)(long Speed);
#endif

#define RingDetect ringdetect()
#define CTSokay ctsokay()
#define InBytes inbytes()
#define OutBytes outbytes()
#define FramingErrors framingerrors()
#define OverrunErrors overrunerrors()
#define ParityErrors parityerrors()

#else  /* #if defined(MULTIPORT) || defined(LIB) */

#define RingDetect _RingDetect
#define CTSokay _CTSokay
#define InBytes _InBytes
#define OutBytes _OutBytes
#define FramingErrors _FramingErrors
#define OverrunErrors _OverrunErrors
#define ParityErrors _ParityErrors
#define cdstillup   ASYNC_cdstillup
#define bauddivisor ASYNC_bauddivisor
#define setport     ASYNC_setport
#define turnoffdtr  ASYNC_turnoffdtr
#define turnondtr   ASYNC_turnondtr
#define turnoffrts  ASYNC_turnoffrts
#define turnonrts   ASYNC_turnonrts
#define turnonxmit  ASYNC_turnonxmit
#define clearoutbuf ASYNC_clearoutbuf
#define clearinbuf  ASYNC_clearinbuf
#define commgo      ASYNC_commgo
#define commstop    ASYNC_commstop
#define commpause   ASYNC_commpause
#define checkcomm   ASYNC_checkcomm
#define comminkey   ASYNC_comminkey
#define cgetstr     ASYNC_cgetstr
#define cgetbuf     ASYNC_cgetbuf
#define csendbyte   ASYNC_csendbyte
#define csendstr    ASYNC_csendstr
#define online      ASYNC_online
#define reopenport  ASYNC_reopenport
#define disconnectmodem ASYNC_disconnectmodem

#ifndef LIB
#define reportcom   ASYNC_reportcom
#endif  /* LIB */
#endif  /* if defined(MULTIPORT) || defined(LIB)  */
#endif  /* ifdef COMM */

void LIBENTRY movestr(char *dest, char *srce, unsigned size);

void LIBENTRY getaltnum(void);

#ifdef DEBUGSTACK
  extern unsigned int StackCheck;
  void LIBENTRY checkstack(void);
#else
  #define checkstack()
#endif

/*
#ifdef __cplusplus
}
#endif
*/



#ifdef __cplusplus
extern "C" {
#endif

#ifdef COMM
/* ASYNCH.ASM */
#ifndef __OS2__
int  LIBENTRY ASYNC_cgetbuf(char *Buffer, int BufLen);
int  LIBENTRY ASYNC_cgetstr(char *Str, int StrLen);
void LIBENTRY ASYNC_csendbyte(unsigned char Byte);
void LIBENTRY ASYNC_csendstr(char *Str, int StrLen);
int  LIBENTRY ASYNC_checkcomm(void);
void LIBENTRY ASYNC_commpause(void);
void LIBENTRY ASYNC_commstop(void);
void LIBENTRY ASYNC_commgo(void);
void LIBENTRY ASYNC_sendbreak(void);
void LIBENTRY ASYNC_turnonrts(void);
void LIBENTRY ASYNC_turnoffrts(void);
void LIBENTRY ASYNC_turnondtr(void);
void LIBENTRY ASYNC_turnoffdtr(void);
void LIBENTRY ASYNC_turnonxmit(void);
void LIBENTRY ASYNC_turnonfifo(char TriggerLevel);
int  LIBENTRY ASYNC_online(void);
int  LIBENTRY ASYNC_cdstillup(void);
void LIBENTRY ASYNC_clearinbuf(void);
void LIBENTRY ASYNC_clearoutbuf(void);
void LIBENTRY ASYNC_init(char IrqNum, int Address, void _FAR_ *PtrToInBuf,void _FAR_ *PtrToOutBuf,int InSize,int OutSize,bool UseCTS,bool ShareIRQ);
void LIBENTRY ASYNC_setport(int BaudDivisor,int DataBits);
int  LIBENTRY ASYNC_opencom(int BaudDivisor,int DataBits);
void LIBENTRY ASYNC_closecom(void);
int  LIBENTRY ASYNC_comminkey(void);
void LIBENTRY ASYNC_reportcom(long Speed);
int  LIBENTRY ASYNC_bauddivisor(long PortSpeed);
void LIBENTRY ASYNC_disconnectmodem(void);
#endif  /* ifndef __OS2__ */
#endif  /* ifdef COMM */

/* BGKEY.ASM */
#ifndef __OS2__
  extern "C" short LIBENTRY bgetkey2(char Option);  /* used instead of bgetkey() from SYSTEM.LIB */
#endif

/* NOSCROLL.ASM */
#ifndef __OS2__
void LIBENTRY begnoscroll(void);
void LIBENTRY endnoscroll(void);
#endif

/* TIMER.ASM */
#ifndef __OS2__
long LIBENTRY settimer(int Which, long Ticks);
long LIBENTRY gettimer(int Which);
long LIBENTRY getticks(void);
#endif


#ifdef __OS2__
  #define fmemcpy(dest,src,len) memcpy(dest,src,len)
#else
  void LIBENTRY fmemcpy(void _FAR_ *dest,void _FAR_ *src,unsigned len);
  #if defined(CPU386) && defined(__LARGE__) && ! defined(DEBUG) && ! defined(__cplusplus)
    #define memcpy(dest,src,len)   fmemcpy(dest,src,len)
    #define strncpy(dest,src,len)  fmemcpy(dest,src,len)
  #endif
#endif

#ifdef __OS2__
  void LIBENTRY createheartbeatthread(void);
  void LIBENTRY destroyheartbeatthread(void);
  void LIBENTRY releasekbdblock(int TimeToWait);
  void LIBENTRY createusernetthread(void);
  void LIBENTRY destroyusernetthread(void);
  bool LIBENTRY needtoscanusernet(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef H_PCBOARD */
