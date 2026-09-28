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


/*  USERS.SYS HEADER STRUCTURE
  --------------------------
  NOTE:  type "bool" is a character field with a non-zero value meaning
  TRUE and zero meaning FALSE. */

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


/*  USERS.SYS FIXED USER RECORD STRUCTURES
  -------------------------------------- */
#pragma pack(1)
#ifdef _MSC_VER
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
  char SingleLines:1;          /* display filenames using single lines */
  char Reserved:6;
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
  int SingleLines:1;           /* display filenames using single lines */
  int Reserved:6;
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
  long           DailyDnldBytes;     /* Number of BYTES downloaded so far today */
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

extern syshdrtype  UserSysHdr;
extern userrectype UserSys;
#define UsersData  UserSys         /* override existing UsersData variables! */
#define UsersRec   UserSys         /* override existing UsersRec  variables! */

