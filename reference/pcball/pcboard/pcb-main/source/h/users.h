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


#ifndef H_USERS
#define H_USERS

#define CUR              (0)
#define USR              (ConfByteLen)
#define REG              (ConfByteLen * 2)
#define EXP              (ConfByteLen * 3)
#define CON              (ConfByteLen * 4)
#define MFL              (ConfByteLen * 5)
#define NET              (ConfByteLen * 6)
#define TOTCONFREGFLAGS  (ConfByteLen * 7)
#define JOINED           (0)
#define SCANNED          (ConfByteLen)
#define TOTCONFFLAGS     (ConfByteLen * 2)

typedef enum {CHECKDIRTY, IGNOREDIRTY } dirtytype;

#pragma pack(1)
typedef struct {
  unsigned short UserRec;
  char     UserName[25];    /* padded with spaces and no terminating NULL */
} IndexType;
#pragma pack()

#pragma pack(1)
typedef struct {
  long     UserRec;
  char     UserName[25];    /* padded with spaces and no terminating NULL */
} IndexType2;
#pragma pack()

#pragma pack(1)
#if defined(_MSC_VER) || defined(__WATCOMC__)
typedef struct {
  char Dirty:1;
  char MsgClear:1;
  char HasMail:1;
  char DontAskFSE:1;
  char FSEDefault:1;
  char ScrollMsgBody:1;
  char ShortHeader:1;
  char WideEditor:1;
} reservedbyte;
typedef struct {
  char UnAvailable:1;
  char SingleLines:1;
  char Reserved:6;
} reservedbyte2;
#else
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
#endif
#pragma pack()

#pragma pack(1)
#ifndef EXTENDED_USER_INFO
#define EXTENDED_USER_INFO
typedef struct {
  char  Street[2][50];
  char  City[25];
  char  State[10];
  char  Zip[10];
  char  Country[15];
} addresstype;
#pragma pack()

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
  char     Previous[3][12];
  unsigned short LastChange;
  unsigned short TimesChanged;
  unsigned short ExpireDate;
} passwordtype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char     Previous[3][13];
  unsigned short LastChange;
  unsigned short TimesChanged;
  unsigned short ExpireDate;
} passwordtypez;
#pragma pack()

#pragma pack(1)
typedef struct {
  char  Line[5][60];
} notestype;
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

#ifdef PCB152
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
  char DropSecLevel;
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
#endif /* ifdef PCB152 */
#endif /* ifndef EXTENDED_USER_INFO */

#pragma pack(1)
typedef struct {
  char Name[25];                    /* 000-024 */
  char City[24];                    /* 025-048 */
  char Password[12];                /* 049-060 */
  char BusDataPhone[13];            /* 061-073 */
  char HomeVoicePhone[13];          /* 074-086 */
  char LastDateOn[6];               /* 087-092 */ /* modified v14.0 was LastDateOn[8] */
  char LastTimeOn[5];               /* 093-097 */
  char ExpertMode;                  /* 098-098 */
  char Protocol;                    /* 099-099 */
  reservedbyte PackedFlags;         /* 100-100 */ /* was AutoRejoin in v12.x */
  char DateLastDirRead[6];          /* 101-106 */
  char SecurityLevel;               /* 107-107 */
  unsigned short NumTimesOn;        /* 108-109 */
  unsigned char PageLen;            /* 110-110 */
  unsigned short NumUploads;        /* 111-112 */
  unsigned short NumDownloads;      /* 113-114 */
  unsigned char DailyDnldBytes[8];  /* 115-122 */ /* renamed in 14.0 was DownloadedBytes[8] */
  char UserComment[30];             /* 123-152 */
  char SysopComment[30];            /* 153-182 */
  short ElapsedTimeOn;              /* 183-184 */
  char RegExpDate[6];               /* 185-190 */
  char ExpSecurityLevel;            /* 191-191 */
  char LastConference;              /* 192-192 */
  char RegConf[5];                  /* 193-197 */ /* new 14.0 - bit map replaces RegInConference[9] */
  char ExpRegConf[5];               /* 198-202 */ /* new 14.0 - bit map replaces ExpRegInConference[9] */
  char UsrRegConf[5];               /* 203-207 */ /* new 14.0 - bit map */
  unsigned char TotDnldBytes[8];    /* 208-215 */ /* new 14.0 */
  unsigned char TotUpldBytes[8];    /* 216-223 */ /* new 14.0 */
  char DeleteFlag;                  /* 224-224 */
  unsigned char LastMsgRead[40][4]; /* 225-384 */ /* changed 14.0 was LastMsgRead[10] */
  long RecNum;                      /* 385-388 */ /* added in v14.5 to point to USERS.INF */
  reservedbyte2 Flags;              /* 389-389 */ /* reserved in v14.5a */
  char Reserved[8];                 /* 390-397 */ /* reduced in v14.5 from 15 to 9, in v15.0 from 9 to 8 */
  unsigned short LastConf2;         /* 398-399 */ /* added in v14.5 for high conferences */
} URead;
#pragma pack()

/*
    information from Sam Smith:

    390-931  Extended User Pointer
    392      Prodoor Flags
    394      Scratch Byte
    395      Counter (# of times in Prodoor)
    396-397  Earned Credits for uploads
*/

#pragma pack(1)
typedef struct {
  char           Name[26];
  char           City[25];
  char           Password[13];
  char           BusDataPhone[14];
  char           HomeVoicePhone[14];
  unsigned short LastDateOn;
  char           LastTimeOn[6];
  bool           ExpertMode;
  char           Protocol;
  reservedbyte   PackedFlags;
  unsigned short DateLastDirRead;
  short          SecurityLevel;
  unsigned short NumTimesOn;
  unsigned char  PageLen;
  unsigned short NumUploads;
  unsigned short NumDownloads;
  long           DailyDnldBytes;    /* must be "signed" for credits to work */
  char           UserComment[31];
  char           SysopComment[31];
  short          ElapsedTimeOn;
  unsigned short RegExpDate;
  short          ExpSecurityLevel;
  unsigned short LastConference;
  unsigned long  ulTotDnldBytes;
  unsigned long  ulTotUpldBytes;
  bool           DeleteFlag;
  long           RecNum;
  reservedbyte2  Flags;
  char           Reserved[8];      /* bytes 390-397 from the USERS file */
  unsigned long  MsgsRead;         /* pulled from users.inf */
  unsigned long  MsgsLeft;         /* pulled from users.inf */
  bool           AliasSupport;
  char           Alias[26];        /* pulled from users.inf */
  bool           AddressSupport;
  addresstypez   Address;          /* pulled from users.inf */
  bool           PasswordSupport;
  passwordtypez  PwrdHistory;      /* pulled from users.inf */
  bool           VerifySupport;
  char           Verify[26];       /* pulled from users.inf */
  bool           StatsSupport;
  callerstattype Stats;            /* pulled from users.inf */
  bool           NotesSupport;
  notestypez     Notes;            /* pulled from users.inf */
#ifdef PCB152
  bool           AccountSupport;
  accounttype    Account;          /* pulled from users.inf */
  bool           QwkSupport;
  qwkconfigtype  QwkConfig;        /* pulled from users.inf */
#endif
#ifdef PCB153
  double         TotDnldBytes;     /* converted to IEEE Double */
  double         TotUpldBytes;     /* converted to IEEE Double */
#endif
} UData;
#pragma pack()

#pragma pack(1)
typedef struct {
  unsigned short Version;          /* PCBoard Version Number;                           */
  unsigned short NumOfConf;        /* Number of Extended Conferences (0-65495)          */
  unsigned short SizeOfRec;        /* Size of the PCBoard User Record information       */
  long           SizeOfConf;       /* Total Size of PCBoard Conference Information      */
  unsigned short NumOfApps;        /* Number of Third Party Apps adding onto the record */
  long           TotalRecSize;     /* Total Record Size (PCB and all TPA components)    */
} hdrtype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char           Name[15];         /* Name of Application;                             */
  unsigned short Version;          /* Version Number;                                  */
  unsigned short SizeOfRec;        /* Size of Application Record information (0-65535) */
  unsigned short SizeOfConfRec;    /* Size of each Application Conference Record       */
  char           KeyWord[9];       /* Keyword for executing Application from PCBoard   */
  long           Offset;           /* Offset in User Record where TPA record begins    */
} apptype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char          Name[25];    /* User Name (in case connection to USERS is lost) */
  unsigned long MsgsRead;    /* Number of messages the user has read in PCBoard */
  unsigned long MsgsLeft;    /* Number of messages the user has left in PCBoard */
} rectype;
#pragma pack()

extern UData          _FARDATA_ UsersData;
extern URead          _FARDATA_ UsersRead;
extern addresstype    _FARDATA_ AddrRec;
extern passwordtype   _FARDATA_ PwrdRec;
extern callerstattype _FARDATA_ StatsRec;
extern notestype      _FARDATA_ NotesRec;
#ifdef PCB152
extern accounttype    _FARDATA_ Account;
extern qwkconfigtype  _FARDATA_ QwkConfig;
#endif

extern DOSFILE  DosUsersInfFile;

extern URead    *TempRead;
extern UData    *TempData;
extern int      UsersFile;
extern int      UsersInfFile;
extern unsigned ConfByteLen;
extern unsigned ExtConfLen;
extern unsigned MsgPtrLen;
extern hdrtype  Hdr;
extern rectype  UsersRec;
extern rectype  TempInfRec;
extern apptype  AppHeader;
extern long     InfHeaderSize;

extern bool     AliasSupport;
extern long     AliasOffset;
extern char     ReadAlias[26];

extern bool     VerifySupport;
extern long     VerifyOffset;
extern char     ReadVerify[26];

extern bool         AddressSupport;
extern long         AddressOffset;

extern bool         PasswordSupport;
extern long         PasswordOffset;

extern bool           StatsSupport;
extern long           StatsOffset;

extern bool           NotesSupport;
extern long           NotesOffset;

extern bool           AccountSupport;
extern long           AccountOffset;

extern bool           QwkSupport;
extern long           QwkOffset;

extern long _FAR_ *MsgReadPtr;
extern long _FAR_ *MsgReadPtrBackup;
extern long _FAR_ *TempMsgPtr;
extern char _FAR_ *ConfReg;
extern char _FAR_ *TempReg;
extern char _FAR_ *ConfFlags;

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

/* recycle.c */
void LIBENTRY updateuserrecord(void);

void     LIBENTRY usersalloc(void);
void     LIBENTRY usersdealloc(void);
int      LIBENTRY tempuseralloc(bool AllocMsgPtrs);
void     LIBENTRY tempuserdealloc(void);
void     LIBENTRY yymmddtostr(char *Dest, char *Srce);
void     LIBENTRY strtoyymmdd(char *Dest, char *Srce);
void     LIBENTRY yymmddtommddyy(char *Dest, char *Srce);
void     LIBENTRY mmddyytoyymmdd(char *Dest, char *Srce);
void     LIBENTRY decryptusersrec(URead *Rec);
void     LIBENTRY convertreadtoconf(char _FAR_ *Conf, URead *Rec);
void     LIBENTRY convertreadtoconfandmsgs(URead *Rec, char _FAR_ *Conf, long _FAR_ *MsgPtr);
void     LIBENTRY convertreadtodata(URead *pur, UData *pud);
void     LIBENTRY encryptusersrec(URead *Rec);
void     LIBENTRY moveback(char *Dest, char *Srce, int Size);
void     LIBENTRY convertconftoread(URead *Rec, char _FAR_ *Conf);
void     LIBENTRY convertconfandmsgstoread(URead *Rec, char _FAR_ *Conf, long _FAR_ *MsgPtr);
void     LIBENTRY convertdatatoread(UData *pud, URead *pur);
void     LIBENTRY closeusersfile(void);
void     LIBENTRY closeusersinffile(void);
int      LIBENTRY openusersfile(void);
int      LIBENTRY openusersinffile(void);
long     LIBENTRY numrecs(const int Handle, const int RecSize);
void     LIBENTRY puttemprecord(long RecNo, dirtytype Flag);
void     LIBENTRY putuserrecord(dirtytype Flag);
int      LIBENTRY gettemprecord(long RecNo);
void     LIBENTRY getuserrecord(bool CheckForUserSys, bool LoggedOff);
void     LIBENTRY getdisplaynames(void);
void     LIBENTRY processuserrecord(void);
void     LIBENTRY newuserdefaults(void);
long     LIBENTRY adduserrecord(URead *Rec, rectype *ExtRec, char _FAR_ *Conf, long _FAR_ *MsgPtr);
void     LIBENTRY getsysopname(void);
void     LIBENTRY updateuserinformation(void);
void     LIBENTRY togglesysopprivileges(void);
void     LIBENTRY increasetime(void);
void     LIBENTRY decreasetime(void);
void     LIBENTRY increasesecurity(void);
void     LIBENTRY decreasesecurity(void);
bool     LIBENTRY lockedout(char _FAR_ *Conf, unsigned ConfNum);
bool     LIBENTRY isregistered(char _FAR_ *Conf, unsigned ConfNum);
bool     LIBENTRY checkuserreginconf(unsigned ConfNum, URead *Rec, char *Today);
bool     LIBENTRY userreginconf(unsigned ConfNum, long RecNo);
void     LIBENTRY putmessagewaiting(unsigned ConfNum, long RecNo);
void     LIBENTRY scanusersputmessagewaiting(unsigned ConfNum);
void     LIBENTRY zeromailflags(void);
bool     LIBENTRY checkmailflags(void);
void     LIBENTRY convertaddrtoaddrz(addresstype *pReadAddr, addresstypez *pDataAddr);
void     LIBENTRY convertaddrztoaddr(addresstypez *pDataAddr, addresstype *pReadAddr);
void     LIBENTRY convertpwrdtopwrdz(passwordtype *pReadPwrd, passwordtypez *pDataPwrd);
void     LIBENTRY convertpwrdztopwrd(passwordtypez *pDataPwrd, passwordtype *pReadPwrd);
bool     LIBENTRY isrealname(long RecNo, char *Name);
bool     LIBENTRY scanmailflags(void);
char *   LIBENTRY usercity(void);


/* userscan.c */
void LIBENTRY scanusersfile(void (LIBENTRY *sub)(URead *p, long ScanRecNo));
void LIBENTRY displayname(URead *p, long Num);
void LIBENTRY displayrec(URead *p, long Num);
void LIBENTRY listusersinconf(URead *p, long ScanRecNo);
void LIBENTRY listusers(URead *p, long ScanRecNo);
void LIBENTRY viewprintusers(int NumTokens);
void LIBENTRY scanuserrecords(int NumTokens);
void LIBENTRY listuserrecords(void);
void LIBENTRY scanforusers(int NumTokens);
void LIBENTRY listconfmembers(void);

/* usersys.c */
void LIBENTRY makeusersys(void);
void LIBENTRY readusersys(bool UserLoggedOff);

bool LIBENTRY badpassword(UData *pud, char *Str, bool ForceNewPassword);
void LIBENTRY getnewpassword(UData *pud, bool ForceNewPassword);
int  LIBENTRY setnewpassword(UData *pud, char *NewPwrd);
int  LIBENTRY setnewalias(UData *pud, char *NewAlias, long UserRecNo);
void LIBENTRY getalias(UData *pud, long UserRecNo);

int LIBENTRY gettpaheader(char *KeyWord, apptype *AppHeader);
int LIBENTRY gettparec(apptype *AppHeader, long RecNum, void *Rec);
int LIBENTRY puttparec(apptype *AppHeader, long RecNum, void *Rec);
int LIBENTRY gettpaconfrec(apptype *AppHeader, long RecNum, unsigned ConfNum,void *Rec);
int LIBENTRY puttpaconfrec(apptype *AppHeader, long RecNum, unsigned ConfNum,void *Rec);

void LIBENTRY validateinfoffset(long Offset, char *Operation);

long LIBENTRY ppladduser(char *Name);

#ifdef FIDO
void LIBENTRY resetuser(void);
#endif

/*
#ifdef __cplusplus
}
#endif
*/

#endif  /* #ifndef H_USERS */
