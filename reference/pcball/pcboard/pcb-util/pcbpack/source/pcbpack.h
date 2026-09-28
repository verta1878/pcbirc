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


/***************************************************************************
 *
 * Copyright (c) 1992, 1993.  All Rights Reserved.
 * Clark Development Company, Inc.
 *
 * $Revision:   1.7  $
 *
 * $Log:   D:/proj/pcbpack/vcs/pcbpack.h_v  $
 *
 *    Rev 1.7   22 Feb 1996 09:52:32   DWT
 * - Removed the definition for TEAR_LINE since it is no longer used.
 * - Added an extern for LineSeparator
 *
 *    Rev 1.6   06 Oct 1995 13:20:28   DWT
 * Made changes to facilitate porting to OS/2
 *
 *    Rev 1.5   22 Mar 1995 15:29:30   DWT
 * Added Compress.
 *
 *    Rev 1.4   04 Feb 1995 15:07:12   DWT
 * - Added a variable for IgnorePackoutDate.
 * - Added definitions for errorlevel return values.
 *
 *    Rev 1.3   18 Dec 1993 11:05:54   DWT
 * Added support for the /RANGE command line parameter .. which was documented but
 * never implemented until now.
 *
 *    Rev 1.2   30 Nov 1993 10:39:06   DWT
 * Added MinimumPackThreshold variable.
 *
 *    Rev 1.1   11 Nov 1993 10:49:04   DWT
 * - Added a few new prototypes
 * - Changed existing prototypes to LIBENTRY
 * - Fixed tab problems
 *
 *    Rev 1.0   19 Aug 1993 19:33:46   LDZ
 * Initial revision.
 *
 ***************************************************************************/

#ifndef PCBPACK_H
#define PCBPACK_H
#define PCBPACK     "PCBPack v15.0"

#define ALLOCMODE
#define BLOCKMODE
#define CHECKDUPS
#define PACKATTACH
#define PACKGAPS
#define REFNUM
#define RENUMBER
#define REPORT
#define USECONFIG
#define WARN

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif

#ifdef __OS2__
  #define _HUGE_
  #define farmalloc malloc
  #define farfree   free
#else
  #define _HUGE_ huge
#endif


/*** PCBPack File Extensions ***********************************************/
# define    EXT_BACKUP      "BAK"
# define    EXT_TEMP        "TMP"
# define    EXT_CRC         "CRC"
# define    EXT_IDX         "IDX"
# define    EXT_IDXBAK      "IBK"
# define    EXT_IDXTEMP     "III"
# define    EXT_NDX         "NDX"
# define    EXT_NDXBAK      "NBK"
# define    EXT_NDXTEMP     "NNN"

# define    EXTHDRID        0x40FF
# define    EXTFUNCLEN      7
# define    EXTDESCLEN      60

# define    MAXEXTHDRS      99
# define    MAX_DATELEN     10
# define    MAX_FILENAME    65
# define    MAX_MSGNUM      16777216L
# define    MIN_MSGNUM      0L

# define    MSG_RCVR        '*'     /* receiver only message - not read       */
# define    MSG_RCVR_READ   '+'     /* receiver only message - read           */
# define    MSG_CMNT        '~'     /* comment to sysop      - not read       */
# define    MSG_CMNT_READ   '`'     /* comment to sysop      - read           */
# define    MSG_SPWD        '%'     /* sender pwrd message   - not read       */
# define    MSG_SPWD_READ   '^'     /* sender pwrd message   - read           */
# define    MSG_GPWD        '!'     /* group pwrd message    - not read       */
# define    MSG_GPWD_READ   '#'     /* group pwrd message    - read           */
# define    MSG_GPWD_ALL    '$'     /* group pwrd message to all              */
# define    MSG_PBLC        ' '     /* msg to all, or public msg not yet read */
# define    MSG_PBLC_READ   '-'     /* public msg that has been read          */

# define    MSG_ACTIVE      0xE1
# define    MSG_INACTIVE    0xE2
# define    MSG_EOL         0xE3

# define    MSG_REMOVED     0
# define    MSG_KEPT        1

#define LOGRECLEN      64
#define LOGTEXTLEN     LOGRECLEN-2
typedef enum {LEFTJUSTIFY, SPACERIGHT } padtype;

/*** PCBPack Error Levels **************************************************/
# define    PCK_OK          0
# define    PCK_ABORT       1
# define    PCK_ESCAPE      100
# define    PCK_HELP        254
# define    PCK_ERRINIT     255

# define    PACK_TIMER      7
# define    PACK_TIMEOUT    60

#ifdef __OS2__
  #define    SIXTYSECONDS    (60000)
  #define    ONESECOND       (1000)
#else
  #define    SIXTYSECONDS    ((60 * 182) / 10)
  #define    ONESECOND       ((1 * 182) / 10)
#endif

# define    TMP_FILENAME    "$$XXXXXX"

# define    LINE_3          (160*3)
# define    COLUMN_28       (28*2)

/*** PCBPack Macro Definitions *********************************************/
# define    SWAP(x,y)       { x ^= y; y ^= x; x ^= y; }

enum {
        PBLC,
        RCVR,
        CMNT,
        SPWD,
        GPWD
};

typedef enum {
        ABORTMSG,
        SAVEMSG,
        NEXTMSG,
        KILLMSG
} savetype;

typedef enum {
        READONLY,
        RDWR,
        RDWRLOCK
} openmsgtype;

enum    {
        HDR_TO=1,
        HDR_FROM=2,
        HDR_SUBJ=4,
        HDR_CARB=8,
        HDR_FILE=16,
        HDR_RCPT=64,
        HDR_XTND=128
};

enum    {
        EXTSTAT_NONE='N',
        EXTSTAT_READ='R'
};

typedef enum    {
        EXTHDR_TO,
        EXTHDR_FROM,
        EXTHDR_SUBJECT,
        EXTHDR_ATTACH,
        EXTHDR_LIST,
        EXTHDR_ROUTE,
        EXTHDR_ORIGIN,
        EXTHDR_REQRR,
        EXTHDR_ACKRR,
        EXTHDR_ACKNAME,
        EXTHDR_PACKOUT,
        EXTHDR_TO2,
        EXTHDR_FROM2,
        EXTHDR_TOTAL
} exthdrtype;

#pragma pack(1)
typedef union {
        char    OkayToRead;
        struct {
                unsigned Okay    : 1;
                unsigned ToYou   : 1;
                unsigned FromYou : 1;
        } Msg;
} readaccesstype;
#pragma pack()

#pragma pack(1)
typedef struct {
        unsigned Type : 3;            /* types: PBLC,RCVR,CMNT,SPWD,GPWD */
        unsigned Read : 1;            /* true or false */
} msgstattype;

#ifdef BASREAL
#pragma pack(1)
typedef struct {
        bassngl HighMsgNum;
        bassngl LowMsgNum;
        bassngl NumActiveMsgs;
        bassngl NumCallers;
        char    LockStatus[6];
} msgbasediskstattype;
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
        char    Reserved[6];
} msgheadertype;
#pragma pack()
#define MSGHEADERTYPE

#pragma pack(1)
typedef struct {
        bassngl Offset;
} msgindextype;
#pragma pack()
#endif  /* ifdef BASREAL */

#pragma pack(1)
typedef struct {
        long    HighMsgNum;
        long    LowMsgNum;
        long    NumActiveMsgs;
} msgbasestattype;
#pragma pack()

#pragma pack(1)
typedef struct  {
        long     Offset;
        long     Num;
        char     To[25];
        char     From[25];
        char     Status;
        unsigned short Date;
        char     Reserved[3];
} newindextype;
#pragma pack()

#pragma pack(1)
typedef struct {
        short   Ident;                  /*  a string = NULL-@ = 16384 decimal = 4000 hex */
        char    Function[EXTFUNCLEN];   /*  TO, FROM, SUBJECT, ATTACH, LIST, ROUTE, ORIGIN */
        char    Colon;                  /*  always set to ":" */
        char    Descript[EXTDESCLEN];   /*  user name, subject, filename */
        char    Status;                 /*  NONE, READ (if LIST), DOWNLOADED (if ATTACH) */
        char    CarReturn;              /*  0xE3 or 0x0D */
} msgextendtype;
#pragma pack()

#pragma pack(1)
typedef struct {                        /* this structure is identical to msgextendtype */
        short   Ident;                  /* except that Description has been split up for */
        char    Function[EXTFUNCLEN];   /* use by the CARBON header allowing access to */
        char    Colon;                  /* the date and time read fields */
        char    Descript[EXTDESCLEN-10];
        char    DateRead[6];
        char    TimeRead[4];
        char    Status;
        char    CarReturn;
} msgcarbontype;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned short Num;
    long           LastMsgNum;
} crchdrtype;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned long   CRC;
    long            MsgNumber;
    unsigned short  Date;
} crctype;
#pragma pack()

#pragma pack(4)
typedef struct  {
    char    Normal;
    char    Intense;
    char    Warning;
    char    Status;
    char    StatusHighlight;
    char    Duplicate;
    char    Win;
    char    WinHighlight;
    char    WinPurge;
    char    Help;
} colorpalette;
#pragma pack()

/*** External Variable Declarations ****************************************/
extern  bool            AbortPack;

# ifdef EXECNET
extern  bool            BackTrack;
# endif

extern  char            LineSeparator;
extern  bool            CallerLog;
extern  bool            ConfFixUp;
extern  bool            DisplayHelp;
extern  bool            ForcePack;
extern  bool            IndexOnly;
extern  int             KeepUnreceived;
extern  bool            NoSizeCheck;
extern  bool            OldIndex;
extern  bool            PurgeReceived;
extern  int             PurgeReceivedAfterDays;
extern  bool            QuietPack;
extern  bool            RemoveAll;
extern  bool            RemoveBackup;
extern  bool            RemoveDuplicates;
extern  bool            UpdateOnly;
extern  bool            ToUpperCase;
extern  bool            IgnorePackoutDate;
extern  bool            Compress;
extern  char            Areas[100];
extern  char            CapFile[MAX_FILENAME];
extern  char            Today[MAX_DATELEN];
extern  char            ExtHdrFunctions[EXTHDR_TOTAL][EXTFUNCLEN+1];
extern  unsigned int    CRCDays;
extern  unsigned int    Days;
extern  unsigned int    DebugLevel;
extern  unsigned int    FixUpLevel;
extern  unsigned int    MaxMessages;
extern  unsigned int    MinMessages;
extern  unsigned int    Timeout;
extern  unsigned int    MinimumPackThreshold;

#ifdef DOSFILE
extern  DOSFILE         Capture;
#ifdef  DEBUGMODE
extern  DOSFILE         debugfile;
#endif
#endif

#ifdef  ARCHIVE
extern  bool            Archive;
#endif
#ifdef  BLOCKMODE
extern  bool            BlockMode;
#endif
#ifdef  CLEANUP
extern  bool            CleanUp;
#endif
#ifdef  PACKGAPS
extern  unsigned        MsgGap;
#endif
#ifdef  RENUMBER
extern  long            Renumber;
#endif
#ifdef  REPORT
extern  long            Report;
#endif
extern  char            *DatFile;
#ifdef pcbdattype
extern  pcbdattype      PcbData;
#endif
#ifdef  USECONFIG
extern  char            *ConfigName;
#endif

extern long            RangeLowMsg;
extern long            RangeHighMsg;
extern bool            PackRange;

extern  colorpalette    ColorList[2];
extern  colorpalette    *ScreenColor;

/*** Function Prototypes ***************************************************/
extern  void            DisplayLine(bool, char, char *, ...);

#ifdef  DEBUGMODE
extern  void            DebugDisplayLine(char, char *, ...);
#endif

#ifdef H_NEWDATA
extern  int  LIBENTRY     DisplayConference(unsigned, pcbconftype *);
#endif

#ifdef MSGHEADERTYPE
extern  void LIBENTRY     DisplayMessage(msgheadertype *);
extern  void LIBENTRY     DisplayStatus(unsigned, msgheadertype *);
extern  int  LIBENTRY     ValidHeader(msgheadertype *Header, long *oMsgNum, unsigned *oDate);
#endif /* ifdef MSGHEADERTYPE */

extern  void LIBENTRY     EndPack(void);
extern  void LIBENTRY     ShowHelp(void);
extern  int  LIBENTRY     AddExtension(char *, char *);
extern  int  LIBENTRY     CreateIndex(unsigned);
extern  int  LIBENTRY     Escape(void);
extern  int  LIBENTRY     waitonesecond(void);
extern  int  LIBENTRY     FixConf(unsigned);
extern  int  LIBENTRY     GenerateReport(unsigned);
extern  bool LIBENTRY     LoadConfig(char *);
extern  int  LIBENTRY     PackConference(unsigned);
extern  int  LIBENTRY     ValidConf(unsigned);
extern  int               CompareCRC(const void *, const void *);
extern  int               CompareDate(const void _HUGE_ *p, const void _HUGE_ *q);
extern  int  LIBENTRY     UpdateIndex(unsigned);
extern  void LIBENTRY     DisplayGraph(unsigned long, unsigned long);

extern  int  LIBENTRY     removefile(char *FileName);
extern  int  LIBENTRY     renamefile(char *OldName,char *NewName);

extern "C" {
long LIBENTRY settimer(int Which, long Ticks);
long LIBENTRY gettimer(int Which);
}

void LIBENTRY closecallerlog(void);
void LIBENTRY openlog(void);
void LIBENTRY repositionlogpointer(void);
void LIBENTRY writelog(char *Str, padtype Pad);
void LIBENTRY writedebugrecord(char *Str);

void LIBENTRY setpacktimer(long Seconds);
bool LIBENTRY packtimerexpired(int Pause);

#endif
