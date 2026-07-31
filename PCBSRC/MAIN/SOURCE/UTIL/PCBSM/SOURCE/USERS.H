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


#ifndef H_PCBSM_USERS
#define H_PCBSM_USERS

#define USR   (0)
#define REG   (ConfByteLen)
#define EXP   (ConfByteLen * 2)
#define CON   (ConfByteLen * 3)
#define MFL   (ConfByteLen * 4)

typedef struct {
  unsigned UserRec;
  char     UserName[25];    /* padded with spaces and no terminating NULL */
} IndexType;

typedef struct {
  long     UserRec;
  char     UserName[25];    /* padded with spaces and no terminating NULL */
} IndexType2;

typedef struct {
  int Dirty:1;
  int MsgClear:1;
  int HasMail:1;
  int DontAskFSE:1;
  int FSEDefault:1;
  int ScrollMsgBody:1;
  int ShortHeader:1;
  int WideEditor:1;
} reservedbyte;

typedef struct {
  int UnAvailable:1;
  int SingleLines:1;
  int Reserved:6;
} reservedbyte2;

typedef struct {
  char Name[25];           /* 000-024 */
  char City[24];           /* 025-048 */
  char Password[12];       /* 049-060 */
  char BusDataPhone[13];   /* 061-073 */
  char HomeVoicePhone[13]; /* 074-086 */
  char LastDateOn[6];      /* 087-092 */ /* modified v14.0 was LastDateOn[8] */
  char LastTimeOn[5];      /* 093-097 */
  char ExpertMode;         /* 098-098 */
  char Protocol;           /* 099-099 */
  reservedbyte PackedFlags;/* 100-100 */ /* was AutoRejoin in v12.x */
  char DateLastDirRead[6]; /* 101-106 */
  char SecurityLevel;      /* 107-107 */
  unsigned NumTimesOn;     /* 108-109 */
  char PageLen;            /* 110-110 */
  unsigned NumUploads;     /* 111-112 */
  unsigned NumDownloads;   /* 113-114 */
  char DailyDnldBytes[8];  /* 115-122 */ /* renamed in 14.0 was DownloadedBytes[8] */
  char UserComment[30];    /* 123-152 */
  char SysopComment[30];   /* 153-182 */
  int  ElapsedTimeOn;      /* 183-184 */
  char RegExpDate[6];      /* 185-190 */
  char ExpSecurityLevel;   /* 191-191 */
  char LastConference;     /* 192-192 */
  char RegConf[5];         /* 193-197 */ /* new 14.0 - bit map replaces RegInConference[9] */
  char ExpRegConf[5];      /* 198-202 */ /* new 14.0 - bit map replaces ExpRegInConference[9] */
  char UsrRegConf[5];      /* 203-207 */ /* new 14.0 - bit map */
  char TotDnldBytes[8];    /* 208-215 */ /* new 14.0 */
  char TotUpldBytes[8];    /* 216-223 */ /* new 14.0 */
  char DeleteFlag;         /* 224-224 */
  char LastMsgRead[40][4]; /* 225-384 */ /* changed 14.0 was LastMsgRead[10] */
  long RecNum;             /* 385-388 */ /* added in v14.5 to point to USERS.INF */
  reservedbyte2 Flags;     /* 389-389 */ /* reserved in v14.5a */
  char Reserved[8];        /* 390-397 */ /* reduced in v14.5 from 15 to 9, in v15.0 from 9 to 8 */
  unsigned LastConf2;      /* 398-399 */ /* added in v14.5 for high conferences */
} URead;

typedef struct {
  char     Name[26];
  char     City[25];
  char     Password[13];
  char     BusDataPhone[14];
  char     HomeVoicePhone[14];
  unsigned LastDateOn;
  char     LastTimeOn[6];
  bool     ExpertMode;
  char     Protocol;
  reservedbyte PackedFlags;
  unsigned DateLastDirRead;
  int      SecurityLevel;
  unsigned NumTimesOn;
  char     PageLen;
  unsigned NumUploads;
  unsigned NumDownloads;
  long     DailyDnldBytes;    /* must be "signed" for credits to work */
  char     UserComment[31];
  char     SysopComment[31];
  int      ElapsedTimeOn;
  unsigned RegExpDate;
  int      ExpSecurityLevel;
  unsigned LastConference;
  unsigned long ulTotDnldBytes;
  unsigned long ulTotUpldBytes;
  bool     DeleteFlag;
  long     RecNum;
  reservedbyte2  Flags;
  char      Reserved[8];      /* bytes 390-397 from the USERS file */
  double    TotDnldBytes;     /* converted from Basic Double in USERS */
  double    TotUpldBytes;     /* converted from Basic Double in USERS */
} UData;

typedef struct {
  unsigned Version;          /* PCBoard Version Number;                           */
  unsigned NumOfConf;        /* Number of Extended Conferences (0-65495)          */
  unsigned SizeOfRec;        /* Size of the PCBoard User Record information       */
  long     SizeOfConf;       /* Total Size of PCBoard Conference Information      */
  unsigned NumOfApps;        /* Number of Third Party Apps adding onto the record */
  long     TotalRecSize;     /* Total Record Size (PCB and all TPA components)    */
} hdrtype;

typedef struct {
  char     Name[15];         /* Name of Application;                             */
  unsigned Version;          /* Version Number;                                  */
  unsigned SizeOfRec;        /* Size of Application Record information (0-65535) */
  unsigned SizeOfConfRec;    /* Size of each Application Conference Record       */
  char     KeyWord[9];       /* Keyword for executing Application from PCBoard   */
  long     Offset;           /* Offset in User Record where TPA record begins    */
} apptype;

typedef struct {
  char          Name[25]; /* User Name (in case connection to USERS is lost) */
  unsigned long MsgsRead; /* Number of messages the user has read in PCBoard */
  unsigned long MsgsLeft; /* Number of messages the user has left in PCBoard */
} rectype;

typedef struct {
  char  Street[2][50];
  char  City[25];
  char  State[10];
  char  Zip[10];
  char  Country[15];
} addresstype;

typedef struct {
  char  Street[2][51];
  char  City[26];
  char  State[11];
  char  Zip[11];
  char  Country[16];
} addresstypez;

typedef struct {
  char  Line[5][60];
} notestype;

typedef struct {
  char  Line[5][61];
} notestypez;

typedef struct {
  unsigned FirstDateOn;
  unsigned NumSysopPages;
  unsigned NumGroupChats;
  unsigned NumComments;
  unsigned Num300;
  unsigned Num1200;
  unsigned Num2400;
  unsigned Num9600;
  unsigned Num14400;
  unsigned NumSecViol;
  unsigned NumNotReg;
  unsigned NumReachDnldLim;
  unsigned NumFileNotFound;
  unsigned NumPwrdErrors;
  unsigned NumVerifyErrors;
} callerstattype;

typedef struct {
  char     Previous[3][12];
  unsigned LastChange;
  unsigned TimesChanged;
  unsigned ExpireDate;
} passwordtype;

typedef struct {
  char     Previous[3][13];
  unsigned LastChange;
  unsigned TimesChanged;
  unsigned ExpireDate;
} passwordtypez;

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
  char DropSecLevel;
} accounttype;

typedef struct {
  unsigned MaxMsgs;
  unsigned MaxMsgsPerConf;
  long     PersonalAttachLimit;
  long     PublicAttachLimit;
  long     NewBltLimit;
  bool     NewFiles;
  char     Reserved[13];
} qwkconfigtype;

/*---------------------------------------------------------------------------
 * personal_psa_t — new in 15.4.  Layout recovered from PCBSM.EXE 15.4b field
 * label table at data offset 0x26b21 (see v0.017 CHANGES entry and USERS154.H).
 *
 * On-disk PSA record installed via PCBSetup keyword "PERSONAL".  Fields
 * exposed to PPL scripts as U_GENDER, U_BIRTHDATE, U_EMAIL, U_WEB.
 *
 * The initial value for Gender must be ' ' (space) — see the 15.4 fix note
 * in USERINFO.C.  A value of 'M' would represent "male", not "unset".
 */
#pragma pack(1)
typedef struct {
  char Gender;             /* ' ', 'M', or 'F' — 15.4 default is ' ' */
  char Birthdate[6];       /* MMDDYY packed date (same as LastDateOn) */
  char Email[60];          /* Internet email, null-terminated */
  char Web[60];            /* Personal WEB URL, null-terminated */
} personal_psa_t;
#pragma pack()

extern UData    UsersData;
extern URead    UsersRead;
extern hdrtype  Header;
extern rectype  UsersRec;
extern apptype  AppHeader;
extern unsigned ConfByteLen;
extern unsigned ExtConfLen;
extern long far *MsgReadPtr;
extern char far *ConfReg;
extern char far *QwkConfFlags;
extern long     InfHeaderSize;
extern char     BackFileName[40];
extern char     TempFileName[40];
extern char     BackInfFileName[40];
extern char     TempInfFileName[40];
extern int      UsersFile;
extern int      TempFile;

extern bool     AliasSupport;
extern char     Alias[26];
extern long     AliasOffset;

extern bool     VerifySupport;
extern char     Verify[26];

extern bool         AddressSupport;
extern addresstype  AddrRec;
extern addresstypez Address;

extern bool          PasswordSupport;
extern passwordtype  PwrdRec;
extern passwordtypez Password;

extern bool           StatsSupport;
extern callerstattype CallerStats;

extern bool           NotesSupport;
extern notestype      NotesRec;
extern notestypez     Notes;

extern bool           AccountSupport;
extern accounttype    Account;

extern bool           QwkSupport;
extern qwkconfigtype  QwkConfig;

/* Personal PSA — new in 15.4 */
extern bool           PersonalSupport;
extern bool           BankSupport;
extern personal_psa_t Personal;

#endif  /* H_PCBSM_USERS */
