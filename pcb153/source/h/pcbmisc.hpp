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


/******************************************************************************/
/*                                                                            */
/*                                PCBMISC.HPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*              Data and functions for various PCB related needs              */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1994 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

#ifndef ___PCBMISC_HPP___

#define ___PCBMISC_HPP___

/******************************************************************************/

/* Pragmas */

/******************************************************************************/

/* Included Files */
#ifdef LIB

#include    "misc.h"

/*#ifdef  ___EXEC___         // commented out this line and */
#ifdef LIB  /* changed to LIB on 4/26/95 DWT */
#include    "pcbtools.h"
/*#include      "pcboard.h" */
/*#include      "newdata.h" */
/*#include      "users.h" */
#else
#include    "country.h"
#include    "dosfunc.h"
#endif

#else

#include    "pcb.h"
#include    "pcboard.h"
#include    "pcboard.ext"
#include    "pcbtext.h"
#include    "users.h"
#include    "misc.h"
#include    "screen.h"
#include    "country.h"
#include    "messages.h"
#include    "account.h"
#include    "ansi.h"

#endif

/******************************************************************************/

/* Defined Macros */



#ifdef LIB

#define URead           userrectype
#define UData           userrectype

#define UsersData       UserSys
#define UsersRec        UserSys

#define AliasSupport    UserSys.AliasSupport
#define AddressSupport  UserSys.AddressSupport
#define PasswordSupport UserSys.PasswordSupport
#define VerifySupport   UserSys.VerifySupport
#define StatsSupport    UserSys.StatsSupport
#define NotesSupport    UserSys.NotesSupport
/*#define AccountSupport  UserSys.AccountSupport */
/*#define QwkSupport      UserSys.QwkSupport */

#define DLDRMonth()     (ptrUData->DateLastDirRead.Month)
#define DLDRDay()       (ptrUData->DateLastDirRead.Day)
#define DLDRYear()      ((ptrUData->DateLastDirRead.Year)+80)

#ifdef ___COMP___

#define fmemcpy _fmemcpy
#define bmalloc malloc
#define bstrdup strdup
#define bfree   free

#define  MAXNAMELEN    66

#ifdef __OS2__
  #define  MAXHANDLES    30
  #define  BEGBUFSIZE    4096   /* first attempted buffer size             */
#else
  #define  MAXHANDLES    25
  #define  BEGBUFSIZE    2048   /* first attempted buffer size             */
#endif

#endif

#else

#define CNF_CUR         CUR
#define CNF_USR         USR
#define CNF_REG         REG
#define CNF_EXP         EXP
#define CNF_CON         CON
#define CNF_MFL         MFL
#define CNF_JOINED      JOINED
#define CNF_SCANNED     SCANNED
#define CNF_NET         NET

#define DLDRMonth() ((ptrUData->DateLastDirRead&0x01E0)>>5)
#define DLDRDay()   ((ptrUData->DateLastDirRead&0x001F))
#define DLDRYear()  (((ptrUData->DateLastDirRead&0xFE00)>>9)+80)
#define mask_num    mask_numbers
#endif

#define clearBuf(b) memset(&b,0,sizeof(b))


/* If subdir or volume bits are set then it's not a file */
#define fexist(f)   ((fileexist(f) & 0x18) == 0)

/******************************************************************************/

/* Types */

#ifdef LIB

/*typedef enum { SHOW,       HIDE }        showtype; */
typedef enum { NLOGOFF,    ALOGOFF }     logtype;
typedef enum { UPLOAD,     DOWNLOAD }    xfertype;
typedef enum { CHECKDIRTY, IGNOREDIRTY } dirtytype;

typedef struct {
    char     Status;
    bool     MailWaiting;
    unsigned Pager;
    char     Name[26];
    char     City[25];
    char     Operation[49];
    char     Message[79];
} nodetype;

typedef struct {
  char     Name[15];         /* Name of Application;                             */
  unsigned Version;          /* Version Number;                                  */
  unsigned SizeOfRec;        /* Size of Application Record information (0-65535) */
  unsigned SizeOfConfRec;    /* Size of each Application Conference Record       */
  char     KeyWord[9];       /* Keyword for executing Application from PCBoard   */
  long     Offset;           /* Offset in User Record where TPA record begins    */
} apptype;

/*typedef struct { */
/*  long HighMsgNum; */
/*  long LowMsgNum; */
/*  long NumActiveMsgs; */
/*} msgbasestattype; */
#endif

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

struct sSTATUS
{
    char DisplayText[80];
};

#endif

/******************************************************************************/

/* Constants */

/******************************************************************************/

/* Variables */

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

/*extern sSTATUS Status; */
extern char OpenFileNames[MAXHANDLES+1][MAXNAMELEN];

#endif

#if defined(LIB) && defined(___EXEC___)

extern URead * TempRead;
extern UData * TempData;

extern char  * TempReg;
extern long  * TempMsgPtr;

#endif

/******************************************************************************/

/* Function Prototypes */

#if defined(LIB) && defined(___EXEC___)

extern "C" {

#ifdef DEBUGSTACK
void   pascal checkstack(void);
#else
#define checkstack()
#endif

void   LIBENTRY substenvinfile          ( char * f, int l );
void   LIBENTRY checkdlfiles            ( int save );

bool   LIBENTRY filestuffinuse          ( void );
char   LIBENTRY bytesleftinkbdbuffer    ( void );
char   LIBENTRY bytesleftinpplbuffer    ( void );

long   LIBENTRY lowmsgnum               ( void );
long   LIBENTRY highmsgnum              ( void );

int    LIBENTRY setlanguage             ( int LangNum );

void   LIBENTRY clearpplbuffer          ( void );
void   LIBENTRY closeautostuff          ( void );

void   LIBENTRY getdisplaynames         ( void );

void   LIBENTRY entermessagefromfile    ( unsigned ConfNum, char * ToName, char * FromName, char * Subject, char Security, unsigned PackDate, bool ReqReceipt, bool Echo, char * FileName );

void   LIBENTRY joinconfcommand         ( int NumTokens );
int    LIBENTRY doquestionnaire         ( int Qnum );
int    LIBENTRY showblt                 ( int BltNum );
void   LIBENTRY dirmenu                 ( int NumTokens );
void   LIBENTRY stuffpplbuffer          ( char * Str );
void   LIBENTRY stuffkbdwithfile        ( char * FileName );
void   LIBENTRY byecommand              ( void );
void   LIBENTRY loguseroff              ( logtype Type );

void   LIBENTRY opendoor                ( int NumTokens );
void   LIBENTRY stuffbufferstr          ( char * Str );

void   LIBENTRY broadcast               ( int NumTokens );
void   LIBENTRY readusernetrecord       ( unsigned NodeNum, nodetype * Buf );
void   LIBENTRY updateusernetrecord     ( unsigned NodeNum, nodetype * Buf );

int    LIBENTRY allocatefilelist        ( xfertype Mode );
int    LIBENTRY addfiletolist           ( char * Name, char * Path, long Size, bool ForceIntoList, bool Showdupe );
void   LIBENTRY send                    ( int NumTokens );

void   LIBENTRY xlatetext               ( char * New, char * Old );

/*char * LIBENTRY countrydate             ( char * s ); */
/*char * LIBENTRY countrydate2            ( char * s ); */
/*char * LIBENTRY uncountrydate           ( char * s ); */
/*char * LIBENTRY uncountrydate2          ( char * s ); */

/*void   LIBENTRY openmodem               ( showtype Show ); */
/*void   LIBENTRY closemodem              ( bool TurnOffDTR ); */

void   LIBENTRY printcom                ( char * s );

int    LIBENTRY pcbcopyfile             ( char * Srce, char * Dest, bool CheckForEOF, bool ShowActivity, char * LockDrives, bool CopyAttributes );

int    LIBENTRY tempuseralloc           ( bool AllocMsgPtrs );
void   LIBENTRY tempuserdealloc         ( void );
void   LIBENTRY puttemprecord           ( long RecNo, dirtytype Flag );
int    LIBENTRY gettemprecord           ( long RecNo );
void   LIBENTRY convertdatatoread       ( UData * pud, URead * pur );
void   LIBENTRY convertreadtodata       ( URead * pur, UData * pud );

long   LIBENTRY numrecs                 ( int Hand, int Size );

void   LIBENTRY updateuserinformation   ( void );
void   LIBENTRY makepcboardsys          ( void );

int    LIBENTRY setnewpassword          ( userrectype * urp, char * NewPwrd );
int    LIBENTRY setnewalias             ( userrectype * urp, char * Alias, long urn );
char * LIBENTRY meganum                 ( int Num );
void   LIBENTRY setripmouseregion       ( int Num, int X1, int Y1, int X2, int Y2, int FontX, int FontY, bool Invert, bool Clear, char * Text );
int    LIBENTRY checkscreenforfilenames ( int StartLine, char * FileName );

/*bool   LIBENTRY insearch                ( void ); */
/*int    LIBENTRY tokenscan               ( char * Input, char * CodedInput, bool CaseSensitive ); */
/*bool   LIBENTRY parsersearch            ( char * Text, int TextLen, char * CodedInput, bool RetVal, int LastToken ); */
/*void   LIBENTRY stopsearch              ( void ); */
void   LIBENTRY printfoundtext          ( char * Str );

int    LIBENTRY gettpaheader            ( char * KeyWord, apptype * AppHeader );
int    LIBENTRY gettparec               ( apptype * AppHeader, long RecNum, void * Rec );
int    LIBENTRY puttparec               ( apptype * AppHeader, long RecNum, void * Rec );
int    LIBENTRY gettpaconfrec           ( apptype * AppHeader, long RecNum, unsigned ConfNum, void * Rec );
int    LIBENTRY puttpaconfrec           ( apptype * AppHeader, long RecNum, unsigned ConfNum, void * Rec );
bool   LIBENTRY canbeavailable          ( void );
int    LIBENTRY msgbasestats            ( msgbasestattype *MsgStats );
}

void   LIBENTRY removecodes             ( char * s );

#endif

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

void   LIBENTRY newline                 ( void );
void   LIBENTRY print                   ( char * s );
void   LIBENTRY println                 ( char * s );
void   LIBENTRY printxlated             ( char * s );
#endif

int    LIBENTRY isATX                   ( char * s );
int    LIBENTRY getATX                  ( char * s );

void   LIBENTRY clearBiosKbd            ( void );

void   LIBENTRY ttyforward              ( int x    );

bool   LIBENTRY lastChar                ( char * s, char c );
void   LIBENTRY dispString              ( char * s, int nt );

char * LIBENTRY pcbstrstr               ( char * s1, char * s2 );
char * LIBENTRY pcbstrchr               ( char * s, char c );
char * LIBENTRY pcbstrdup               ( char * s );
#ifdef ___EXEC___
char * LIBENTRY bstrdup                 ( char * s );
#endif

/******************************************************************************/

#endif

