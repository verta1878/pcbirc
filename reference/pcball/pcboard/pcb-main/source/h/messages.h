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


#ifndef H_MESSAGES
#define H_MESSAGES

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

enum { PBLC,RCVR,CMNT,SPWD,GPWD };

typedef enum {ABORTMSG,SAVEMSG,NEXTMSG,KILLMSG} savetype;
typedef enum {READONLY,RDWR,RDWRLOCK} openmsgtype;


#define EXTHDRID    0x40FF
#define EXTFUNCLEN  7
#define EXTDESCLEN  60
#define MAXEXTHDRS  99

enum {HDR_TO=1, HDR_FROM=2, HDR_SUBJ=4, HDR_CARB=8, HDR_FILE=16, HDR_RCPT=64, HDR_XTND=128};
enum {EXTSTAT_NONE='N',EXTSTAT_READ='R'};

typedef enum {EXTHDR_TO,EXTHDR_FROM,EXTHDR_SUBJECT,EXTHDR_ATTACH,EXTHDR_LIST,
              EXTHDR_ROUTE,EXTHDR_ORIGIN,EXTHDR_REQRR,EXTHDR_ACKRR,
              EXTHDR_ACKNAME,EXTHDR_PACKOUT,EXTHDR_TO2,EXTHDR_FROM2,
              EXTHDR_FORWARD,EXTHDR_UFOLLOW,EXTHDR_UNEWSGR,EXTHDR_SUBJ2,
              EXTHDR_TOTAL
} exthdrtype;

extern char ExtHdrFunctions[EXTHDR_TOTAL][EXTFUNCLEN+1];

#pragma pack(4)
typedef union {
  char OkayToRead;
  struct {
    unsigned Okay:1;
    unsigned ToYou:1;
    unsigned FromYou:1;
    unsigned List:1;
    unsigned Generic:1;
  } Msg;
} readaccesstype;
#pragma pack()

#pragma pack(4)
typedef struct {
  unsigned Type: 3;          /* types: PBLC,RCVR,CMNT,SPWD,GPWD */
  unsigned Rsvd: 4;          /* reserved */
  unsigned Read: 1;          /* true or false */
} msgstattype;
#pragma pack()

#pragma pack(1)
typedef struct {
  bassngl HighMsgNum;
  bassngl LowMsgNum;
  bassngl NumActiveMsgs;
  bassngl NumCallers;
  char    LockStatus[6];
} msgbasediskstattype;
#pragma pack()

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
  char    NetTag;
} msgheadertype;
#pragma pack()

#pragma pack(1)
typedef struct {
  short Ident;                /* a string = NULL-@ = 16384 decimal = 4000 hex */
  char  Function[EXTFUNCLEN]; /* TO, FROM, SUBJECT, ATTACH, LIST, ROUTE, ORIGIN */
  char  Colon;                /* always set to ":" */
  char  Descript[EXTDESCLEN]; /* user name, subject, filename */
  char  Status;               /* NONE, READ (if LIST), DOWNLOADED (if ATTACH) */
  char  CarReturn;            /* 0xE3 or 0x0D */
} msgextendtype;
#pragma pack()

#pragma pack(1)
typedef struct {              /* this structure is identical to msgextendtype  */
  short Ident;                /* except that Description has been split up for */
  char  Function[EXTFUNCLEN]; /* use by the CARBON header allowing access to   */
  char  Colon;                /* the date and time read fields                 */
  char  Descript[EXTDESCLEN-10];
  char  DateRead[6];
  char  TimeRead[4];
  char  Status;
  char  CarReturn;
} msgcarbontype;
#pragma pack()

#pragma pack(1)
typedef struct {
  bassngl Offset;
} msgindextype;
#pragma pack()

#pragma pack(4)
typedef struct {
  unsigned short   ConfNum;
  int              Index;
  DOSFILE          Msgs;
  msgbasestattype  Stats;
  msgheadertype    Header;
  char             *Body;
  int              BodySize;
  int              OpenMode;
} msgbasetype;
#pragma pack()

#pragma pack(1)
typedef struct {
  long           Offset;
  long           Num;
  char           To[25];
  char           From[25];
  char           Status;
  unsigned short Date;
  char           Reserved[3];
} newindextype;
#pragma pack()


#define     QWK_HEADER          "QWK Packet Produced by PCBoard v15.0"
#define     QWK_SIZE            128
#define     QWK_INDEX_SIZE      5

#pragma pack(1)
typedef struct {
  char           Status;
  char           Number[7];
  char           Date[8];
  char           Time[5];
  char           ToName[25];
  char           FromName[25];
  char           Subject[25];
  char           Password[12];
  char           ReferenceNum[8];
  char           NumBlocks[6];
  char           SpecialFlag;
  unsigned short ConfNum;
  unsigned short PacketMsgNum;
  char           NetTag;
} qwkmsgtype;
#pragma pack()

#pragma pack(1)
typedef struct {
  bassngl  RecordNumber;
  char     Extra;
} qwkndxtype;
#pragma pack()

extern msgextendtype *ExtHdr[MAXEXTHDRS]; /* dynamically alloc extended headers */
extern int           NumExtHdrs;          /* number of extended headers         */
extern char          ExtendedStatus;      /* used to build the status value     */
extern bool          CreateReceipt;       /* TRUE if creating a RETURN RECEIPT  */

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

int  LIBENTRY lockmessagebase(int MsgFile, msgbasestattype *MsgStats);
void LIBENTRY unlockmessagebase(int MsgFile, msgbasestattype *MsgStats);
long LIBENTRY numcallers(void);
long LIBENTRY addcaller(void);
bool LIBENTRY checkformail(void);
char * LIBENTRY getqwkroot(void);
void LIBENTRY qwkcommand(int NumTokens);
void LIBENTRY openedmsgbasestats(int MsgFile, msgbasestattype *MsgStats);
void LIBENTRY openedmsgbasestreamstats(DOSFILE *File, msgbasestattype *MsgStats);
int  LIBENTRY msgbasestats(msgbasestattype *MsgStats);
long LIBENTRY freemsgs(msgbasestattype *MsgStats);
long LIBENTRY checkfreemsgs(void);
int  LIBENTRY openmessagebase(unsigned short Num, pcbconftype *Conf, msgbasetype *MsgBase, openmsgtype OpenType);
void LIBENTRY closemessagebase(msgbasetype *MsgBase);
void LIBENTRY updatemessageheader(newindextype *Index, msgheadertype *Header, msgbasestattype *MsgStats);
readaccesstype LIBENTRY readstatus(newindextype *Index, msgstattype Msg);
msgstattype LIBENTRY getmsgstatus(char Status);
char LIBENTRY getnewmsgstatus(msgstattype MsgStatus);
void LIBENTRY setreplydate(msgheadertype *Header, msgstattype Status, bool RepliedTo);
bool LIBENTRY gettoname(int PcbTextNum, char *To);
void LIBENTRY createreceipt(msgbasetype *Old);
savetype LIBENTRY enterreply(msgbasetype *Old, msgstattype MsgStatus, readaccesstype Access, bool AskOther);
savetype LIBENTRY editmessage(msgbasetype *Old, long MsgNum, long Offset);
savetype LIBENTRY modifyextheader(msgbasetype *Old, long MsgNum, long Offset, exthdrtype Which, char *Desc, char Mask);
int  LIBENTRY savemessage(unsigned short ConfNum, msgheadertype *MsgHeader, char *MsgBody, bool Copying);
char * LIBENTRY getextendedheader(exthdrtype Which, char *Body);
char * LIBENTRY skipextendedheader(char *Body);
void LIBENTRY updatemsgindex(void);
void LIBENTRY updateindex(newindextype *Index, msgheadertype *Header, msgbasestattype *MsgStats);
void LIBENTRY updateoldindex(pcbconftype *Conf, long Offset, long Num, long LowMsgNum);
int LIBENTRY updatemessagebody(long Offset, char *Body, int NumBlocks);
void LIBENTRY readqwkreplies(char *Msgs, bool SetLMRs);
char * LIBENTRY carbonedtoyou(char *Body);
long LIBENTRY fixlmrpointer(unsigned short ConfNum, long LowMsgNum, long HighMsgNum);
int LIBENTRY getmessageheader(long MsgNumber, msgbasetype *MsgBase, long *IdxOffset, newindextype *Index);
int LIBENTRY putmessageheader(msgbasetype *MsgBase, long *IdxOffset, long MsgOffset, newindextype *Index);
void LIBENTRY setreplystatus(long MsgNum);
void LIBENTRY forwardmessage(unsigned short MoveConfNum, msgbasetype *MsgBase);
void LIBENTRY kill(long MsgNumber, bool InvalidateAttachment);
int LIBENTRY savetomsgbase(unsigned short ConfNum, pcbconftype *Conf, msgbasetype *MsgBase, msgheadertype *Header, char *Body, long OldMsgNumber, long OldOffset, void (LIBENTRY *showsaving)(unsigned short ConfNum, long MsgNum, long MsgOffset));
char * LIBENTRY loadextendedheaders(char *Body);
char * LIBENTRY checkextendedheaders(exthdrtype Which, char *Desc);
void * LIBENTRY buildgenextheader(char *Hdr, char *Desc);
void LIBENTRY buildextheader(exthdrtype Which, char *Desc, char BitMask);
void LIBENTRY removeexthdr(exthdrtype Which, char BitMask);
int LIBENTRY getmsgbasestats(msgbasestattype *MsgStats, char *FileName);
void LIBENTRY freehdrs(void);
void LIBENTRY makemsgheader(msgheadertype *Header, char Status, unsigned short NumBytes, char *Date, char *Time, char *To, long RefNum, unsigned short ReadDate, char *ReadTime, char ReadStatus, char *From, char *Subject, char *Password, bool Echo);

/*
#ifdef __cplusplus
}
#endif
*/

#endif /* #ifndef H_MESSAGES */
