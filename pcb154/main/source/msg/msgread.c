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


#include "project.h"
#pragma hdrstop

#include "pcbmacro.h"
#include "account.h"
#include "messages.h"
#include <ctype.h>

#ifdef __WATCOMC__
  #include <io.h>
#endif

#define MAXGROUPS              20
#define IDXBUFSIZE            128
#define TOFIELDTHRUSUBJFIELD   85
#define HDR_SHORT               4
#define HDR_LONG                6

typedef enum {NOFUNC,REPLY,REPOTHER,KILL,EDITHDR,EDITMSG,PROT,UNPROT,MOVE,COPY,FINDFROM,FINDTO,GOODBYE,STOP, /*CONTINUE,*/ ENTERMSG,JOIN,QUICKSCAN,SKIPMSG,JUMPOUT,SELCONF,DESELCONF,REDISPLAY,XPORT,FLAGFILE,VIEWFILE,CHAT,WHOCMD,FORWARD} msgfunctype;
typedef enum {INSIDEREADLOOP, OUTSIDEREADLOOP} readlooptype;
typedef enum {READNEXT, REREAD, QUITREAD, QUITLOOP, SKIPEND, SKIPNEXT} readmsgtype;
typedef enum {FIND_NONE,FIND_TO=1,FIND_FROM=2,FIND_USER=4} usrsrchtype;

#ifdef __cplusplus
  #define max(a,b)    (((a) > (b)) ? (a) : (b))
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
  #define USRSRCHTYPE int
#else
  #define USRSRCHTYPE usrsrchtype
#endif

/***********************/
#define NUMMSGOPTIONS 42
wordtype static Options[NUMMSGOPTIONS] = {
 {"ALL"}, {"ALIAS"}, {"BYE"}, {"CHAT"}, {"COPY"}, {"DESELECT"}, {"EDIT"},
 {"FF"}, {"FT"}, {"FLAG"}, {"FORWARD"}, {"FROM"}, {"GB"}, {"HELP"}, {"JUMP"},
 {"KILL"}, {"LONG"}, {"MOVE"}, {"NEXT"}, {"NET"}, {"PREV"}, {"QWK"}, {"REPLY"},
 {"RM"}, {"RM+"}, {"RM-"}, {"RO"}, {"RR"}, {"RR+"}, {"RR-"}, {"SELECT"},
 {"SET"}, {"SHORT"}, {"SKIP"}, {"T+"}, {"T-"}, {"TO"}, {"TS"}, {"USER"},
 {"WAIT"}, {"WHO"}, {"YA"}
};

enum {O_ALL,O_ALIAS,O_BYE,O_CHAT,O_COPY,O_DESEL,O_EDIT,O_FINDF,O_FINDT,O_FLAG,
      O_FORW,O_FROM,O_GB,O_HELP,O_JUMP,O_KILL,O_LONG,O_MOVE,O_NEXT,O_NET,
      O_PREV,O_QWK,O_REPLY,O_RM,O_RMF,O_RMB,O_RO,O_RR,O_RRF,O_RRB,O_SEL,O_SET,
      O_SHORT,O_SKIP,O_TF,O_TB,O_TO,O_TS,O_USER,O_WAIT,O_WHO,O_YA};

/***********************/

typedef struct {
  char        DateNum[80];
  char        To[15];
  char        Replies[30];
  char        From[15];
  char        NA[15];
  char        NotRead[15];
  char        ReceiverOnly[30];
  char        GroupPwrd[30];
  char        SenderPwrd[30];
  char        Public[30];
  char        Refer[30];
  char        Read[15];
  char        Subject[15];
  char        Status[15];
  char        File[15];
  char        List[15];
  char        None[15];
  char        General[80];
  char        Thread[80];
  char        TextScan[80];
  char        ForYou[80];
  char        FromYou[80];
  char        ToOrFrom[80];
  char        Separator[15];
  char        AllName[30];
  char        RetRecName[30];
  char        Comment[30];
  char        Echo[15];
  char        AllConfIndicator[15];
  char        ReadOnlyIndicator[15];
  int         Left;
  int         Right;
  char        TextScanWidth;
  char        ToOrFromWidth;
  char        ForYouWidth;
  char        FromYouWidth;
  char        ThreadWidth;
  char        GeneralWidth;
} msgtexttype;

typedef struct {
  long First;
  long Last;
} msgseqtype;

typedef struct {
  char           Prompt[81];         // string containing tokenized commands
  char           ConfStr[80];        // Conference String (name and number)
  char           ConfUpStr[80];      // Conference String (uppercase)
  char           ConfUpStrLen;       // length of conference string
  char           UserNameTo[26];     // User Name to search for in TO or USER
  char           UserNameFrom[26];   // User Name to search for in FROM
  USRSRCHTYPE    UserSrch;           // FIND_NONE,FIND_TO,FIND_FROM,FIND_USER
  bool           AllConf;            // scan all conferences for mail
  bool           MailWaitConf;       // scan conferences that have mail waiting
  bool           Since;              // read mail since last message read
  bool           AnyMsgs;            // read any msgs (not just to/from)
  bool           FromMsgs;           // read any messages from you
  bool           YourMsgs;           // read any messages for you
  bool           MsgsToAll;          // read any messages to all
  bool           UnReadOnly;         // read only messages that are UNREAD
  bool           FullRange;          // read all messages (begin to end)
  bool           KeepGoing;          // check for any newly left msgs & go on
  bool           Forward;            // read messages in a forward direction
  bool           Backward;           // read messages in a backward direction
  bool           Threading;          // read messages in threaded order
  bool           UpdateMsgStatus;    // should Msg Read status be updated
  bool           UpdatePtrs;         // should MsgReadPtr be updated
  bool           CheckUserScan;      // yes if we should check user preference
  bool           MsgClear;           // true if user wants the screen cleared
  bool           StayInConf;         // set true so "R A" won't go to next conf
  bool           Capture;            // capture messages for download
  bool           PossibleNoScroll;   // stores most of scroll/noscroll status
  bool           QWK;                // set true when creating a QWK packet
  bool           Net;                // set true when creating a QWK "NET" packet
  bool           CaptureSingle;      // set true to capture a single message
  bool           Attached;           // true if ATTACHED.LST exists
  bool           PreMsg;             // true if a PREMSG file is found
  char           PreMsgFile[66];     // full path\name of PREMSG file, if found
  char           HeaderLen;          // 6 or 4 line header
  unsigned       FoundTotal;         // total number of messages (for capture)
  unsigned       Found;              // number of messages found
  unsigned       ActMsgsRead;        // count of messages read for accounting purposes
  int            NumSearches;        // <>0 = read only msgs matching text srch
  long           MsgNumber;          // current message number
  unsigned short StartConf;          // where we started the read from
  unsigned short Conf;               // cur conf # - used in Read All processing
  int            NumGroups;          // number of groups of messages to read
  int            CurGroup;           // current group being process (first=0)
  unsigned short NewDate;            // julian date to search for new msgs
  msgseqtype    *Numbers;            // list of numbers in the groups
  msgstattype    MsgStatus;          // current msg:  msg type / read/not read
  readaccesstype Access;             // current msg:  to/from/okay to read
  msgcarbontype *CarbonPtr;          // pointer to carbon extended header
} readtype;

typedef struct {
  qwkmsgtype MsgRecord;
  qwkndxtype NdxRecord;
  char       MsgName[66];
  char       NdxName[66];
  char       PerName[66];
  char       ListName[66];
  unsigned   RecordCount;
  unsigned   NdxCount;
  unsigned   PerCount;
  DOSFILE    File;
  DOSFILE    Index;
  DOSFILE    Personal;
  DOSFILE    ListFile;
  char      *ConfArray;
} qwktype;

unsigned short static MoveConfNum;  /* number of conference to move message to  */
msgtexttype    static *Text;        /* memory for this variable allocated below */
readtype       static Read;
msgbasetype    static MsgBase;
unsigned short static OriginalConference;
qwktype        static *QWK = NULL;
long           static SaveMsgReadCount;
double         static MsgReadCharge;


void LIBENTRY cleanupmsgmemory(void) {
  checkstack();
  if (Read.NumSearches > 0) {
    stopsearch();
    Read.NumSearches = 0;
    Status.SearchText[0] = 0;
  }
  closemessagebase(&MsgBase);
  if (Read.Numbers != NULL) {
    bfree(Read.Numbers);
    Read.Numbers = NULL;
  }
  if (Text != NULL) {
    bfree(Text);
    Text = NULL;
  }
  if (QWK != NULL) {
    dosfclose(&QWK->File);
    dosfclose(&QWK->Index);
    dosfclose(&QWK->Personal);
    bfree(QWK->ConfArray);
    bfree(QWK);
    QWK = NULL;
  }
  Status.MsgRead = FALSE;
  Display.ClearScrnWhenFull = FALSE;
}

/********************************************************************
*
*  Function:  backuppointers()
*
*  Desc    :  make a backup copy of the Last Message Read pointers
*/

void LIBENTRY backuppointers(void) {
  checkstack();
  fmemcpy(MsgReadPtrBackup,MsgReadPtr,MsgPtrLen);
  Status.ReadCapture = TRUE;
  SaveMsgReadCount = UsersData.MsgsRead;
}


/********************************************************************
*
*  Function:  restorepointers()
*
*  Desc    :  restore the Last Message Read pointers from the backup
*/

void LIBENTRY restorepointers(void) {
  checkstack();
  Status.ReadCapture = FALSE;
  fmemcpy(MsgReadPtr,MsgReadPtrBackup,MsgPtrLen);

  #ifdef PCB152
  {
    if (Status.ActStatus != ACT_DISABLED && UsersData.MsgsRead != SaveMsgReadCount && MsgReadCharge != 0) {
//    double Rate = AccountRates.ChargeForMsgReadCapture + Status.CurConf.ChargeMsgRead;
//    recordusage("MSG READ CAP","CREDIT",Rate,-1 * (UsersData.MsgsRead - SaveMsgReadCount),&UsersData.Account.DebitMsgReadCapture);
      recordusage("MSG READ CAP","CREDIT",-1,MsgReadCharge,&UsersData.Account.DebitMsgReadCapture);
    }
  }
  #endif

  UsersData.MsgsRead = SaveMsgReadCount;
  displaypcbtext(TXT_POINTERSRESTORED,NEWLINE|LOGIT);
}


static int _NEAR_ LIBENTRY setlastmessageread(int NumTokens) {
  char Str[10];
  char *p;

  checkstack();
  if (NumTokens) {
    p = getnexttoken();
    if (alldigits(p)) {
      NumTokens--;
      goto done;
    }
  }

  sprintf(Status.DisplayText,"%ld-%ld",MsgBase.Stats.LowMsgNum,MsgBase.Stats.HighMsgNum);
  lascii(Str,MsgReadPtr[Status.Conference]);
  freshline();
  inputfield(Str,TXT_SETLASTMSGREADPTR,sizeof(Str)-1,NEWLINE|LFBEFORE|FIELDLEN|GUIDE,NOHELP,mask_numbers);
  p = Str;

done:
  if (*p != 0) {
    lascii(Status.DisplayText,MsgReadPtr[Status.Conference] = min(atol(p),MsgBase.Stats.HighMsgNum));
    displaypcbtext(TXT_LASTMSGREADSETTO,NEWLINE|LFBEFORE);
  }
  return(NumTokens);
}


/********************************************************************
*
*  Function:  xport()
*
*  Desc    :  Exports the current message to a file and then shells out to
*             a batch file called export.bat.
*/

static void _NEAR_ LIBENTRY xport(void) {
  DOSFILE File;
  char    FileName[66];

  checkstack();
  buildstr(FileName,PcbData.TmpLoc,"EXPORT.MSG",NULL);
  if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&File) != -1) {
    if (dosfwrite(&MsgBase.Header,sizeof(MsgBase.Header),&File) != -1 &&
        dosfwrite(MsgBase.Body,MsgBase.BodySize * 128,&File) != -1) {
      dosfclose(&File);
      runexportbat(FileName);
    } else
      dosfclose(&File);
    unlink(FileName);
  }
}


/********************************************************************
*
*  Function:  unprotect()
*
*  Desc    :  unprotects the current message
*/

static void _NEAR_ LIBENTRY unprotect(newindextype *Index) {
  checkstack();
  if (Read.MsgStatus.Type != PBLC) {
    MsgBase.Header.Status = (Read.MsgStatus.Read ? MSG_PBLC_READ : MSG_PBLC);
    updatemessageheader(Index,&MsgBase.Header,&MsgBase.Stats);
  }
}


/********************************************************************
*
*  Function:  protect()
*
*  Desc    :  protects the current message
*/

static void _NEAR_ LIBENTRY protect(newindextype *Index) {
  checkstack();
  if (Read.MsgStatus.Type != RCVR) {
    MsgBase.Header.Status = (Read.MsgStatus.Read ? MSG_RCVR_READ : MSG_RCVR);
    updatemessageheader(Index,&MsgBase.Header,&MsgBase.Stats);
  }
}


/********************************************************************
*
*  Function:  kill()
*
*  Desc    :  kills the message defined by MsgNumber
*/

static int _NEAR_ LIBENTRY kill(long MsgNumber, bool InvalidateAttachment, bool IgnoreSecurity, bool PPL) {
  bool         OkayToDelete;
  long         IdxOffset;
  long         MsgOffset;
  char         ConfStr[80];
  newindextype Index;
  char         Str[80];
  msgbasetype  WriteMsgBase;

  checkstack();
  if (openmessagebase(Status.Conference,&Status.CurConf,&WriteMsgBase,RDWR) != -1) {
    if (getmessageheader(MsgNumber,&WriteMsgBase,&IdxOffset,&Index) != -1 && Index.Offset >= 0) {
      OkayToDelete   = FALSE;
      Read.MsgStatus = getmsgstatus(Index.Status);
      #pragma warn -stv
      Read.Access    = readstatus(&Index,Read.MsgStatus);
      #pragma warn +stv
      if (IgnoreSecurity)         // used by the MOVE command, for instance, to avoid the insuf sec to kill error
        OkayToDelete = TRUE;
      else if (Read.Access.OkayToRead && Status.CurSecLevel >= PcbData.UserLevels[SEC_K]) {
        if (Read.MsgStatus.Type == CMNT && Status.CurSecLevel >= PcbData.SysopSec[SEC_READCOMMENTS])
          OkayToDelete = TRUE;
        else if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READALLMAIL])
          OkayToDelete = TRUE;
        else if (Read.Access.Msg.ToYou && Read.MsgStatus.Type != SPWD && WriteMsgBase.Header.ToField[0] != '@')
          OkayToDelete = TRUE;
        else if (Read.Access.Msg.FromYou) {
          if (Read.MsgStatus.Type == SPWD || Read.MsgStatus.Type == GPWD) {
            Str[0] = 0;
            OkayToDelete = passwordokay(WriteMsgBase.Header.Password,Str,TXT_YOURPASSWORD,TXT_WRONGPWRDENTERED,2);
          } else OkayToDelete = TRUE;
        }
      }

      if (OkayToDelete) {
        if (InvalidateAttachment && (MsgBase.Header.ExtendedStatus & HDR_FILE) != 0)
          modifyextheader(&MsgBase,MsgNumber,Index.Offset,EXTHDR_ATTACH,"",0);  //lint !e534

        WriteMsgBase.Header.ActiveFlag = MSG_INACTIVE;
        MsgOffset = Index.Offset;
        Index.Offset = -Index.Offset;
        if (putmessageheader(&WriteMsgBase,&IdxOffset,MsgOffset,&Index) != -1) {
          makeconfstrsys(ConfStr,Status.Conference);
          sprintf(Status.DisplayText,"%s# %ld",ConfStr,MsgNumber);
          Status.AppendText = TRUE;
          if (PPL)
            logsystext(TXT_MSGKILLED,SPACERIGHT);
          else
            displaypcbtext(TXT_MSGKILLED,NEWLINE|LOGIT);
        }
      } else {
        lascii(Status.DisplayText,MsgNumber);
        Status.AppendText = TRUE;
        displaypcbtext(TXT_YOUCANNOTKILLMSG,NEWLINE);
      }
    } else {
      if (PPL) {
        closemessagebase(&WriteMsgBase);
        return(-1);
      }

      lascii(Str,MsgNumber);
      // allow @OPTEXT@ to be used even though the current PCBTEXT does not
      // make use of it, remove the println(Str) once the default has @OPTEXT@
      strcpy(Status.DisplayText,Str);
      displaypcbtext(TXT_NOSUCHMSGNUMBER,DEFAULTS);
      println(Str);
    }
    closemessagebase(&WriteMsgBase);
  }
  return(0);
}


/********************************************************************
*
*  Function:  unkill()
*
*  Desc    :  unkills a message
*/

void LIBENTRY unkill(int NumTokens) {
  char         *p;
  long         IdxOffset;
  long         MsgNumber;
  char         Str[40];
  char         ConfStr[80];
  newindextype Index;
  msgbasetype  WriteMsgBase;

  checkstack();
  if (NumTokens == 0) {
    inputfield(Str,TXT_MSGNUMTOACTIVATE,sizeof(Str)-1,NEWLINE|LFBEFORE,NOHELP,mask_numbers);
    if (Str[0] == 0)
      return;
    p = Str;
  } else p = getnexttoken();

  MsgNumber = atol(p);

  if (openmessagebase(Status.Conference,&Status.CurConf,&WriteMsgBase,RDWR) != -1) {
    if (getmessageheader(MsgNumber,&WriteMsgBase,&IdxOffset,&Index) != -1 && Index.Offset < 0) {
      Index.Offset = -Index.Offset;
      WriteMsgBase.Header.ActiveFlag = MSG_ACTIVE;
      if (putmessageheader(&WriteMsgBase,&IdxOffset,Index.Offset,&Index) != -1) {
        makeconfstr(ConfStr,Status.Conference);
        sprintf(Status.DisplayText,"%s# %ld",ConfStr,MsgNumber);
        Status.AppendText = TRUE;
        displaypcbtext(TXT_MSGRESTORED,NEWLINE|LOGIT);
      }
    } else {
      lascii(Str,MsgNumber);
      // allow @OPTEXT@ to be used even though the current PCBTEXT does not
      // make use of it, remove the println(Str) once the default has @OPTEXT@
      strcpy(Status.DisplayText,Str);
      displaypcbtext(TXT_NOSUCHMSGNUMBER,DEFAULTS);
      println(Str);
    }
    closemessagebase(&WriteMsgBase);
  }
}


/********************************************************************
*
*  Function:  killcommand()
*
*  Desc    :  Called from the 'command prompt' this function determines the
*             msg number to be called and then calls kill() to kill it.
*/

void LIBENTRY killcommand(int NumTokens) {
  char *p;
  long Number;
  char Str[10];

  checkstack();
  if (NumTokens == 0) {
    Str[0] = 0;
    inputfield(Str,TXT_MSGNUMBERTOKILL,sizeof(Str)-1,NEWLINE|LFBEFORE,HLP_K,mask_numbers);
    if (Str[0] == 0)
      return;
    p = Str;
  } else p = getnexttoken();

  Number = atol(p);
  kill(Number,FALSE,FALSE,FALSE);      //lint !e534
}


int LIBENTRY killmessage(unsigned ConfNum, long Number) {
  int         RetVal;
  unsigned    SaveConfNum;
  pcbconftype SaveConf;

  SaveConfNum = Status.Conference;
  SaveConf = Status.CurConf;
  if (Status.Conference != ConfNum) {
    Status.Conference = ConfNum;
    getconfrecord(ConfNum,&Status.CurConf);
  }

  RetVal = kill(Number,FALSE,TRUE,TRUE);

  if (ConfNum != SaveConfNum) {
    Status.CurConf = SaveConf;
    Status.Conference = SaveConfNum;
  }

  return(RetVal);
}


long LIBENTRY curmsgnum(void) {
  checkstack();
  return(Read.MsgNumber);
}


/********************************************************************
*
*  Function:  editheader()
*
*  Desc    :  allows the user to edit the message header
*/

static int _NEAR_ LIBENTRY editheader(int CurPos, newindextype *Index) {
  short          Temp;
  int            Len;
  unsigned short Date;
  char           Opt[2];
  char          *p;
  char           DateStr[9];
  char           Refer[10];
  char           Subj[(EXTDESCLEN*2)+1];
  char           To[(EXTDESCLEN*2)+1];
  char           From[(EXTDESCLEN*2)+1];
  char           Old[(EXTDESCLEN*2)+1];
  char           Str[(EXTDESCLEN*2)+1];

  checkstack();
  if ((p = getextendedheader(EXTHDR_TO,MsgBase.Body)) != NULL) {
    memcpy(To,p,EXTDESCLEN);
    To[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_TO2,MsgBase.Body)) != NULL) {
      memcpy(&To[EXTDESCLEN],p,EXTDESCLEN);
      To[EXTDESCLEN*2] = 0;
    }
  } else {
    movestr(To,MsgBase.Header.ToField,sizeof(UsersRead.Name));
  }

  if ((p = getextendedheader(EXTHDR_FROM,MsgBase.Body)) != NULL) {
    memcpy(From,p,EXTDESCLEN);
    From[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_FROM2,MsgBase.Body)) != NULL) {
      memcpy(&From[EXTDESCLEN],p,EXTDESCLEN);
      From[EXTDESCLEN*2] = 0;
    }
  } else {
    movestr(From,MsgBase.Header.FromField,sizeof(UsersRead.Name));
  }

  if ((p = getextendedheader(EXTHDR_SUBJECT,MsgBase.Body)) != NULL) {
    memcpy(Subj,p,EXTDESCLEN);
    Subj[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_SUBJ2,MsgBase.Body)) != NULL) {
      memcpy(&Subj[EXTDESCLEN],p,EXTDESCLEN);
      From[EXTDESCLEN*2] = 0;
    }
  } else {
    movestr(Subj,MsgBase.Header.SubjField,sizeof(UsersRead.Name));
  }

  stripright(To,' ');
  stripright(From,' ');
  stripright(Subj,' ');

  lascii(Refer,bassngltolong(MsgBase.Header.RefNumber));

  newline();
  displaypcbtext(TXT_TO  ,DEFAULTS);  println(To);
  displaypcbtext(TXT_FROM,DEFAULTS);  println(From);
  displaypcbtext(TXT_SUBJ,DEFAULTS);  println(Subj);

  Opt[0] = 0;
  if (Status.CurConf.EchoMail)
    inputfield(Opt,TXT_EDITHEADERECHO,1,FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_efnprst);
  else
    inputfield(Opt,TXT_EDITHEADER,1,FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_fnprst);

  Len = 25;

  switch(Opt[0]) {
    case  0 : return(CurPos);
    case 'E': MsgBase.Header.EchoFlag = (MsgBase.Header.EchoFlag == 'E' ? ' ' : 'E');
              goto write;
    case 'R': if (seclevelokay("R",PcbData.SysopSec[SEC_EDITHDR])) {
                if (! Read.MsgStatus.Read)
                  return(CurPos);
                Read.MsgStatus.Read = FALSE;
                #pragma warn -stv
                MsgBase.Header.Status = getnewmsgstatus(Read.MsgStatus);
                #pragma warn +stv
                goto write;
              }
              return(CurPos);
    case 'P': if (seclevelokay("P",PcbData.SysopSec[SEC_PROTECT])) {
                Len = 1;
                switch (Read.MsgStatus.Type) {
                  case PBLC: Str[0] = 'N'; break;
                  case CMNT:
                  case RCVR: Str[0] = 'R'; break;
                  case SPWD: Str[0] = 'S'; break;
                  case GPWD: Str[0] = 'G'; break;
                }
                Str[1] = 0;
              } else return(CurPos);
              break;
    case 'N': Len = 9; maxstrcpy(Str,Refer,sizeof(Str)); break;
    case 'T': maxstrcpy(Str,To,sizeof(Str));
              maxstrcpy(Old,Str,sizeof(Old));
              gettoname(TXT_NEWINFO,Str);
              goto process;
    case 'S': maxstrcpy(Str,Subj,sizeof(Str));
              Len = EXTDESCLEN;
              break;
    case 'F': Temp = PcbData.SysopSec[SEC_EDITHDR];
              if ((MsgBase.Header.ExtendedStatus & HDR_RCPT) != 0 && (p = getextendedheader(EXTHDR_ACKRR,MsgBase.Body)) != NULL)
                PcbData.SysopSec[SEC_EDITHDR] = 256;  // prevent changing return receipt
              if (seclevelokay("F",PcbData.SysopSec[SEC_EDITHDR])) {
                PcbData.SysopSec[SEC_EDITHDR] = Temp;
                maxstrcpy(Str,From,sizeof(Str));
              } else {
                PcbData.SysopSec[SEC_EDITHDR] = Temp;
                return(CurPos);
              }
              break;
  }

  maxstrcpy(Old,Str,sizeof(Old));

  if (Opt[0] == 'P') {
    mask_gnrsd[0] = (char) (Status.CurSecLevel >= PcbData.SysopSec[SEC_KEEPMSG] ? 5 : 4);
    inputfield(Str,TXT_MSGSECURITY,1,UPCASE|FIELDLEN|NEWLINE|LFBEFORE,HLP_SEC,mask_gnrsd);
  } else
    inputfield(Str,TXT_NEWINFO,Len,FIELDLEN|HIGHASCII|NEWLINE|LFBEFORE,NOHELP,mask_alphanum);

process:
  stripleft(Str,' ');
  if (Str[0] == 0)
    return(CurPos);

  if (strcmp(Old,Str) == 0)
    return(CurPos);

  if (Len == 25)
    padstr(Str,' ',25);

  switch(Opt[0]) {
    case 'T': if (! Status.CurConf.LongToNames)
                strupr(Str);

              memcpy(MsgBase.Header.ToField,Str,25);
              if (strlen(Str) > 25 || strlen(To) > 25) {
                if (strlen(Str) > 25)
                  memset(MsgBase.Header.ToField,' ',25);
                if (modifyextheader(&MsgBase,Read.MsgNumber,Index->Offset,EXTHDR_TO,Str,HDR_TO) == KILLMSG) {
                  kill(Read.MsgNumber,TRUE,TRUE,FALSE);    //lint !e534
                  return(-1);  // force a re-read of the index
                }
                return(CurPos);
              }
              break;
    case 'F': strupr(Str);
              if (strstr(Str,"@USER@") != NULL) {  /* don't allow @USER@ in FROM field */
                displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE);
                return(CurPos);
              }
              memcpy(MsgBase.Header.FromField,Str,25);
              break;
    case 'S': padstr(Str,' ',25);
              memcpy(MsgBase.Header.SubjField,Str,25);
              if (strlen(Str) > 25 || strlen(Subj) > 25) {
                if (modifyextheader(&MsgBase,Read.MsgNumber,Index->Offset,EXTHDR_SUBJECT,Str,HDR_SUBJ) == KILLMSG)
                  kill(Read.MsgNumber,TRUE,TRUE,FALSE);    //lint !e534
                return(-1);  // force a re-read of the index
              }
              break;
    case 'N': longtobassngl(MsgBase.Header.RefNumber,atol(Str));
              break;
    case 'P': strupr(Str);
              switch (Str[0]) {
                case 'N': MsgBase.Header.Status = MSG_PBLC; break;
                case 'R': MsgBase.Header.Status = MSG_RCVR; break;
                case 'S': MsgBase.Header.Status = MSG_SPWD;
                          Str[0] = 0;
                          inputfield(Str,TXT_SECURITYPASSWORD,12,FIELDLEN|UPCASE|NEWLINE|HIGHASCII,HLP_E,mask_alphanum);
                          padstr(Str,' ',12);
                          memcpy(MsgBase.Header.Password,Str,12);
                          break;
                case 'G': MsgBase.Header.Status = MSG_GPWD;
                          Str[0] = 0;
                          inputfield(Str,TXT_SECURITYPASSWORD,12,FIELDLEN|UPCASE|NEWLINE|HIGHASCII,HLP_E,mask_alphanum);
                          padstr(Str,' ',12);
                          memcpy(MsgBase.Header.Password,Str,12);
                          break;
                case 'D': if ((p = getextendedheader(EXTHDR_PACKOUT,MsgBase.Body)) != NULL) {
                            memcpy(DateStr,p,8);
                            DateStr[8] = 0;
                            stripright(DateStr,' ');
                            stripall(DateStr,'-');
                            countrydate2(DateStr);
                          } else
                            DateStr[0] = 0;

                          while (1) {
                            inputfield(DateStr,TXT_ENTERPACKDATE,6,FIELDLEN|GUIDE|NEWLINE|LFBEFORE,NOHELP,mask_numbers);
                            if (DateStr[0] == 0)
                              break;
                            uncountrydate2(DateStr);
                            if ((Date = datetojulian(DateStr)) != 0) {
                              strcpy(DateStr,juliantodate(Date));
                              if (modifyextheader(&MsgBase,Read.MsgNumber,Index->Offset,EXTHDR_PACKOUT,DateStr,0) == KILLMSG) {
                                kill(Read.MsgNumber,TRUE,TRUE,FALSE);  //lint !e534
                                return(-1);  // force a re-read of the index
                              }
                              break;
                            }
                          }
                          break;
              }
              break;
  }

write:
  updatemessageheader(Index,&MsgBase.Header,&MsgBase.Stats);
  return(CurPos);
}



static void _NEAR_ LIBENTRY write_index_value(void) {
  checkstack();
  if (! Read.Net) {
    longtobassngl(QWK->NdxRecord.RecordNumber,QWK->RecordCount);

    if(MsgBase.ConfNum<0xff)
      QWK->NdxRecord.Extra=(char)MsgBase.ConfNum;
    else
      QWK->NdxRecord.Extra=0xff;

    if (dosfwrite(&QWK->NdxRecord,QWK_INDEX_SIZE,&QWK->Index) == -1)
      return;

    QWK->NdxCount++;
    if (Read.Access.Msg.ToYou) {
      if (dosfwrite(&QWK->NdxRecord,QWK_INDEX_SIZE,&QWK->Personal) == -1)
        return;

      QWK->PerCount++;
    }
  }
}


static int _NEAR_ LIBENTRY write_file_header(DOSFILE *fptr) {
  char headerstring[QWK_SIZE+1];

  checkstack();
  strcpy(headerstring,QWK_HEADER);
  padstr(headerstring,0x20,QWK_SIZE);
  return(dosfwrite(headerstring,QWK_SIZE,fptr));
}


static int _NEAR_ LIBENTRY openqwkindex(void) {
  char Temp[15];

  checkstack();
  setbit(QWK->ConfArray,Read.Conf);

  if (! Read.Net) {
    dosfclose(&QWK->Index);
    if (QWK->NdxName[0] != 0) {
      if (QWK->NdxCount == 0)
        unlink(QWK->NdxName);
    }

    sprintf(Temp,(Read.Conf < 1000 ? "%03d.NDX" : Read.Conf < 10000 ? "%04d.NDX" : "%05d.NDX"),Read.Conf);
    buildstr(QWK->NdxName,PcbData.TmpLoc,Temp,NULL);

    QWK->NdxCount = 0;
    if (dosfopen(QWK->NdxName,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&QWK->Index) == -1)
      return(-1);

  }
  return(0);
}


static void _NEAR_ LIBENTRY writefiletolist(char *FileName) {
  char Temp[128];

  checkstack();
  buildstr(Temp,PcbData.TmpLoc,FileName,"\r\n",NULL);
  dosfputs(Temp,&QWK->ListFile);         //lint !e534
}


static int _NEAR_ LIBENTRY openqwkfile(void) {
  removeqwkfiles(TRUE);

  checkstack();

  Read.Attached = FALSE;

  if ((QWK = (qwktype *) checkmalloc(sizeof(qwktype),"QWK PACKET")) == NULL)
    return(-1);

  memset(QWK,0,sizeof(qwktype));

  if ((QWK->ConfArray = (char *) checkmalloc(ConfByteLen,"QWK PACKET")) == NULL) {
    bfree(QWK);
    return(-1);
  }

  memset(QWK->ConfArray,0,ConfByteLen);

  buildstr(QWK->MsgName,PcbData.TmpLoc,"MESSAGES.DAT",NULL);
  if (dosfopen(QWK->MsgName,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&QWK->File) == -1) {
    bfree(QWK->ConfArray);
    bfree(QWK);
    return(-1);
  }

  buildstr(QWK->ListName,PcbData.TmpLoc,"PCBQWK.LST",NULL);
  if (dosfopen(QWK->ListName,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&QWK->ListFile) == -1) {
    dosfclose(&QWK->File);
    unlink(QWK->MsgName);
    bfree(QWK->ConfArray);
    bfree(QWK);
    return(-1);
  }

  writefiletolist("MESSAGES.DAT");
  writefiletolist("CONTROL.DAT");
  writefiletolist("*.NDX");

  if (Read.Net)
    writefiletolist("NETFLAGS.DAT");
  else {
    buildstr(QWK->PerName,PcbData.TmpLoc,"PERSONAL.NDX",NULL);
    if (dosfopen(QWK->PerName,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&QWK->Personal) == -1) {
      dosfclose(&QWK->ListFile);
      unlink(QWK->ListName);
      dosfclose(&QWK->File);
      unlink(QWK->MsgName);
      bfree(QWK->ConfArray);
      bfree(QWK);
      return(-1);
    }
  }

  dossetbuf(&QWK->File,16384);
  QWK->RecordCount=2;
  if (write_file_header(&QWK->File) == 0 && openqwkindex() == 0)
    return(0);

//unneeded?  (dwt 12/31/93)
//QWK->MsgRecord.PacketMsgNum = 0;  // make sure it starts out as zero

  dosfclose(&QWK->Personal);
  unlink(QWK->PerName);
  dosfclose(&QWK->ListFile);
  unlink(QWK->ListName);
  dosfclose(&QWK->File);
  unlink(QWK->MsgName);
  bfree(QWK->ConfArray);
  bfree(QWK);
  return(-1);
}


static char * _NEAR_ LIBENTRY filenameonly(char *FullPath) {
  char *p;

  checkstack();
  if ((p = strrchr(FullPath,'\\')) == NULL)
    p = FullPath;
  else
    p++;

  return(p);
}


void LIBENTRY removeqwkfiles(bool All) {
  static char *Types[8] = {"MESSAGES.DAT","CONTROL.DAT","PCBQWK.LST","NETFLAGS.DAT","ATTACHED.LST","*.NDX","*.QWK","*.REP"};
  int  X;
  int  Last;
  char *p;
  char Temp[66];

  checkstack();
  strcpy(Temp,PcbData.TmpLoc);
  p = &Temp[strlen(Temp)];

  Last = (All ? 8 : 6);

  for (X = 0; X < Last; X++) {
    strcpy(p,Types[X]);
    deletefiles(Temp);
  }
}


int LIBENTRY finishqwkpacket(bool CreateControlDat) {
  char        Flag;
  unsigned    X;
  unsigned    ConfCount;
  DOSFILE     File;
  char        Name[66];
  char        Temp[256];
  pcbconftype Conf;

  checkstack();
  if (QWK == NULL || ! Read.QWK)
    return(0);

  if (Read.Attached)
    writefiletolist("ATTACHED.LST");

  dosfclose(&QWK->File);
  dosfclose(&QWK->Index);
  dosfclose(&QWK->Personal);
  dosfclose(&QWK->ListFile);

  if (QWK->NdxCount == 0)
    unlink(QWK->NdxName);

  if (QWK->PerCount == 0)
    unlink(QWK->PerName);

  if (! CreateControlDat) {
    removeqwkfiles(TRUE);
    return(0);
  }

  buildstr(Name,PcbData.TmpLoc,"CONTROL.DAT",NULL);
  if (dosfopen(Name,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&File) == -1)
    return(-1);

/******************************************************************************
The CONTROL.DAT file is a simple ASCII file.  Each line is terminated
with a carriage return and line feed combination.  All lines should
start on the first column.

Line #
 1   My BBS                   BBS name
 2   New York, NY             BBS city and state
 3   212-555-1212             BBS phone number
 4   John Doe, Sysop          BBS Sysop name
 5   20052,MYBBS              Mail door registration #, BBSID
 6   01-01-1991,23:59:59      Mail packet creation time
 7   JANE DOE                 User name (upper case)
 8                            Name of menu for Qmail, blank if none
 9   0 or -1                  -1 means the current user is the sysop
10   999                      Total number of messages in packet
11   121                      Total number of conference minus 1
12   0                        1st conf. number
13   Main Board               1st conf. name (13 characters or less)
14   1                        2nd conf. number
15   General                  2nd conf. name
..   3                        etc. onward until it hits max. conf.
..   123                      Last conf. number
..   Amiga_I                  Last conf. name
..   HELLO                    Welcome screen file
..   NEWS                     BBS news file
..   SCRIPT0                  Log off screen
******************************************************************************/

  sprintf(Temp,"%s\r\n"           // line  1, BBS Name
               "\r\n"             // line  2, BBS City
               "\r\n"             // line  3, BBS Phone Number
               "%s, Sysop\r\n"    // line  4, Sysop name
               "PCBOARD,%s\r\n",  // line  5, Mail Door
          PcbData.BoardName,
          Status.SysopName,
          getqwkroot());
  dosfputs(Temp,&File);  //lint !e534

  datestr(Temp);                  // line  6, Packet creation date
  Temp[8] = ',';
  timestr1(&Temp[9]);
  strcat(Temp,"\r\n");
  dosfputs(Temp,&File);           //lint !e534

  for (X = ConfCount = 0; X < PcbData.NumAreas; X++)
    if (isset(QWK->ConfArray,X))
      ConfCount++;

  sprintf(Temp,"%s\r\n"           // line  7, User's Name
               "\r\n"             // line  8, QMAIL menu name
               "%d\r\n"           // line  9, -1 if SYSOP, otherwise 0
               "%u\r\n"           // line 10, Total number of messages in packet
               "%d\r\n",          // line 11, Total number of conferences minus 1
          Status.DisplayName,
          (Status.User == SYSOP ? -1 : 0),
          Read.FoundTotal,
          ConfCount-1);
  dosfputs(Temp,&File);           //lint !e534

  for (X = 0; X < PcbData.NumAreas; X++) {
    if (isset(QWK->ConfArray,X)) {
      getconfrecord(X,&Conf);
      sprintf(Temp,"%u\r\n"       // conference number
                   "%s\r\n",      // conference name
                   X,Conf.Name);
      dosfputs(Temp,&File);       //lint !e534
      if (--ConfCount == 0)
        break;
    }
  }

  sprintf(Temp,"%s\r\n%s\r\n%s\r\n",
          filenameonly(PcbData.WlcFile),
          filenameonly(Status.CurConf.NewsFile),
          filenameonly(PcbData.LogOffScr));
  dosfputs(Temp,&File);           //lint !e534
  dosfclose(&File);

  if (Read.Net) {
    buildstr(Name,PcbData.TmpLoc,"NETFLAGS.DAT",NULL);
    if (dosfopen(Name,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&File) != -1) {
      for (X = 0; X < PcbData.NumAreas; X++) {
         Flag = (char) (isset(&ConfReg[NET],X) ? 1 : 0);
         dosfwrite(&Flag,1,&File);     //lint !e534
      }
      dosfclose(&File);
    } else
      Read.Net = FALSE;
  }

  return(0);
}


/********************************************************************
*
*  Function:  messagereadprompt()
*
*  Desc    :  prompts the user for the messages to read
*/

static int _NEAR_ LIBENTRY messagereadprompt(void) {
  int InputNum;
  int HelpNum;
  pcbtexttype Buf;

  checkstack();
  if (Status.QuickScan) {
    InputNum = (UsersData.ExpertMode ? TXT_MSGSCANCMDEXPERT : TXT_MSGSCANCOMMAND);
    HelpNum  = HLP_Q;
  } else if (Status.ReplyCommand) {
    InputNum = TXT_REPLYTOMSGS;
    HelpNum  = HLP_REP;
  } else {
    InputNum = (UsersData.ExpertMode ? TXT_MSGREADCMDEXPRT : TXT_MSGREADCOMMAND);
    HelpNum  = HLP_R;
    if (Display.NumLinesPrinted > 1) {
      getpcbtext(InputNum,&Buf);
      if (Buf.Str[0] == '%' || Buf.Str[0] == '!' || Buf.Str[0] == '$')
        moreprompt(PRESSENTER);
    }
  }

  Read.Prompt[0] = 0;
  Read.MsgNumber = MsgReadPtr[Status.Conference];  /* default to next msg to read */
  openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);

  sprintf(Status.DisplayText,"%ld-%ld",MsgBase.Stats.LowMsgNum,MsgBase.Stats.HighMsgNum);
  inputfield(Read.Prompt,InputNum,sizeof(Read.Prompt)-1,UPCASE|HIGHASCII|STACKED|NEWLINE|LFBEFORE,HelpNum,mask_msgread);
  return(tokenize(Read.Prompt));
}


/********************************************************************
*
*  Function:  isnumber()
*
*  Desc    :  This function is similar to alldigits() except that it allows
*             for a + or - at the end of the number as well.
*
*  NOTE    :  For + or - to be valid, an actual digit has to first be found.
*
*  Returns :  TRUE if it is a number, FALSE if it is not.
*/

static bool _NEAR_ LIBENTRY isnumber(char *p) {
  int Num;

  checkstack();
  for (Num = FALSE; *p != 0; p++) {
    if (isdigit(*p))
      Num = TRUE;
    else if (! (Num && (*p == '+' || *p == '-')))
      return(FALSE);
  }
  return(TRUE);
}


static void _NEAR_ LIBENTRY getallresumestatus(void) {
  char Str[2];

  Read.Conf = 0;
  if (Status.StartConf != 0) {
    Str[0] = YesChar;
    Str[1] = 0;
    inputfield(Str,TXT_RESUMEALL,1,NEWLINE|LFBEFORE|FIELDLEN|GUIDE|UPCASE|YESNO,NOHELP,mask_yesno);
    if (Str[0] == YesChar)
      Read.Conf = Status.StartConf;   /* get last beginning conf number */
    else
      Status.StartConf = 0;
  }
}


/********************************************************************
*
*  Function:  interpretcommand()
*
*  Desc    :  interprets the command line passed to the read message functions
*
*  Returns :  returns -1 if a memory error occurs
*/

static msgfunctype _NEAR_ LIBENTRY interpretcommand(int NumTokens, readlooptype Flag) {
  bool            DoTextSearch;
  bool            DoUserSearch;
  bool            OpenCapture;
  bool            OpenQWK;
  bool            ForwDirection;    /* for threading purposes */
  bool            NewMsgs;
  bool            ValidCmd;
  char            SaveByte;
  int             Opt;
  int             LastSrchCmd;
  msgseqtype      Num;
  msgfunctype     Func;
  char            *p;
  long            Temp;
  char            Input[15];
  char            Str[sizeof(Status.SearchText)];
  readtype        Backup;

  checkstack();
  if (Read.Capture) {
    closecapturefile();
    Read.Capture = FALSE;
    Status.ReadCapture = FALSE;
  }

  Backup = Read;

  /* defaults */
  ValidCmd           = FALSE;
  Func               = NOFUNC;
  Status.CapBye      = FALSE;
  Status.CapAsk      = TRUE;
  OpenCapture        = FALSE;
  OpenQWK            = FALSE;
  DoTextSearch       = FALSE;
  DoUserSearch       = FALSE;
  NewMsgs            = FALSE;
  Read.AllConf       = FALSE;
  Read.MailWaitConf  = FALSE;
  Read.Since         = FALSE;
  Read.FromMsgs      = FALSE;
  Read.YourMsgs      = FALSE;
  Read.MsgsToAll     = FALSE;
  Read.FullRange     = FALSE;
  Read.KeepGoing     = FALSE;
  Read.Threading     = FALSE;
  Read.CaptureSingle = FALSE;
  Read.AnyMsgs       = TRUE;
  Read.UnReadOnly    = FALSE;
  Read.StayInConf    = FALSE;
  Read.CheckUserScan = TRUE;
  Read.Found         = 0;
  Read.ActMsgsRead   = 0;
  Read.NewDate       = 0;
  SaveByte           = Status.SearchText[0];
  Status.SearchText[0] = 0;
  Read.UserNameTo[0]   = 0;
  Read.UserNameFrom[0] = 0;
  Read.UserSrch      = FIND_NONE;
  LastSrchCmd        = 0;
  Read.NumGroups     = 0;
  Read.CurGroup      = -1;
  ForwDirection      = TRUE;

top:
  for (;  NumTokens; NumTokens--) {
    p = getnexttoken();
    if (isnumber(p)) {
      if (NewMsgs && strlen(p) == 6 && alldigits(p)) {
        uncountrydate2(p);
        Read.NewDate = datetojulian(p);
      } else {
        Num.First = Num.Last = atol(p);
        if (strchr(p,'+') != NULL || (Status.QuickScan && strchr(p,'-') == NULL)) {
          Num.Last = 0x7FFFFFFFL;                    /* last message! */
          Read.KeepGoing = TRUE;
        }
        else if ((p = strchr(p,'-')) != NULL) {
          p++;
          if (*p == 0)
            Num.Last = 1;
          else if (*p >= '1' && *p <= '9')
            Num.Last = atol(p);
        }
        if (Read.NumGroups < MAXGROUPS) {
          Func = NOFUNC;
          Read.Numbers[Read.NumGroups] = Num;
        }
        Read.NumGroups++;
        ValidCmd = TRUE;
      }
    } else if (! Status.ReplyCommand) {
      if (*(p+1) == 0) {
        switch (*p) {
          case 'L': Flag = OUTSIDEREADLOOP;
                    /* fall thru */
          case '-': openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
                    Read.Numbers[Read.NumGroups].First = (Flag == INSIDEREADLOOP ? Read.MsgNumber - 1 : 0x7FFFFFFFL);
                    Read.Numbers[Read.NumGroups].Last  = 1;
                    Read.NumGroups++;
                    Func = NOFUNC;
                    ValidCmd = TRUE;
                    break;
          case '+': if (Flag == INSIDEREADLOOP) {
                      Read.Numbers[Read.NumGroups].First = (Flag == INSIDEREADLOOP ? Read.MsgNumber + 1 : 1);
                      Read.Numbers[Read.NumGroups].Last  = 0x7FFFFFFFL; /* last number */
                      Read.NumGroups++;
                      Func = NOFUNC;
                    } else Read.Since = TRUE;   /* else treat it like a Since command */
                    Read.KeepGoing = TRUE;
                    openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
                    ValidCmd = TRUE;
                    break;
          case 'A': if (Status.CurSecLevel < PcbData.UserLevels[SEC_J])
                       break;
                    ValidCmd = TRUE;
                    Read.AllConf = TRUE;
                    getallresumestatus();
                    if (Flag == INSIDEREADLOOP)        /* if we're already reading messages */
                      Read.StayInConf = TRUE;          /* then don't jump out just yet */
                    break;
          case 'Z': Status.ZipCap   = TRUE;   /* fall thru */
          case 'D': Status.CapAsk   = FALSE;  /* fall thru */
          case 'C': OpenCapture     = TRUE;
                    Read.FoundTotal = 0;
                    ValidCmd = TRUE;
                    if (Read.NumGroups != 0)
                      break;
                    if (Flag == INSIDEREADLOOP) {
                      Read.CaptureSingle = TRUE;
                      Func = REDISPLAY;
                      break;
                    }
                    /* else fall thru to do a READ SINCE with the capture */
          case '*':
          case 'S': Read.Since     = TRUE;
                    Read.KeepGoing = TRUE;
                    ValidCmd = TRUE;
                    openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
                    break;
          case 'E': Func = (Flag == INSIDEREADLOOP ? EDITHDR : ENTERMSG);
                    ValidCmd = TRUE;
                    break;
          case 'F': if (Flag == INSIDEREADLOOP)
                      Func = (Read.Access.Msg.FromYou ? FINDTO : FINDFROM);
                    else {
                      Read.AnyMsgs  = FALSE;
                      Read.FromMsgs = TRUE;
                    }
                    ValidCmd = TRUE;
                    break;
          case 'G': if (Func == NOFUNC) {
                      Func = GOODBYE;
                      ValidCmd = TRUE;
                    }
                    break;
          case 'H': displayhelpfile(HLP_R);
                    ValidCmd = TRUE;
                    break;
          case 'J': freshline();
                    dispatch(p,PcbData.UserLevels[SEC_J],NumTokens,joinconfcommand);
                    NumTokens = 1;  /* this will abort out of the loop */
                    Func = JOIN;
                    ValidCmd = TRUE;
                    break;
          case 'K': Func = KILL;
                    ValidCmd = TRUE;
                    break;
          case 'M': Status.MemorizeNum = Read.MsgNumber;
                    displaypcbtext(TXT_MSGNUMBERMEMORIZED,LFBEFORE);
//                  Func = CONTINUE;
                    ValidCmd = TRUE;
                    break;
          case 'N': if (Flag == INSIDEREADLOOP)
                      Func = STOP;
                    else {
                      NewMsgs = TRUE;
                      Read.Since = FALSE;
                    }
                    ValidCmd = TRUE;
                    break;
          case 'O': Read.UpdatePtrs = FALSE;
                    ValidCmd = TRUE;
                    if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READONLY])
                      Read.UpdateMsgStatus = FALSE;
                    break;
          case 'P': Func = PROT;
                    ValidCmd = TRUE;
                    break;
          case 'Q': if (seclevelokay(p,PcbData.UserLevels[SEC_Q])) {
                      Status.QuickScan = TRUE;
                      ValidCmd = TRUE;
                      if (Flag == INSIDEREADLOOP)
                        Func = QUICKSCAN;
                    }
                    break;
          case 'T': if (Flag == INSIDEREADLOOP) {
                      Read.Threading = TRUE;
                      ForwDirection  = TRUE;
                      ValidCmd = TRUE;
                    }
                    break;
          case 'U': if (Flag == INSIDEREADLOOP)
                      Func = UNPROT;
                    else
                      Read.UnReadOnly = TRUE;
                    ValidCmd = TRUE;
                    break;
          case 'V': Func = VIEWFILE;
                    ValidCmd = TRUE;
                    break;
          case 'X': if (Flag == INSIDEREADLOOP) {
                      Func = XPORT;
                      ValidCmd = TRUE;
                    }
                    break;
          case 'Y': Read.AnyMsgs  = FALSE;
                    Read.YourMsgs = TRUE;
                    ValidCmd = TRUE;
                    break;
          case '/': if (Flag == INSIDEREADLOOP) {
                      Func = REDISPLAY;
                      ValidCmd = TRUE;
                    }
                    break;
          default : switch (LastSrchCmd) {
                      case O_FROM: addtext(Read.UserNameFrom,p,sizeof(Read.UserNameFrom)-1);
                                   break;
                      case O_TO  : // fall thru - USER shares the same memory
                      case O_USER: addtext(Read.UserNameTo,p,sizeof(Read.UserNameTo)-1);
                                   break;
                      case O_TS  : // fall thru - default to TS search
                      default    : addtext(Status.SearchText,p,sizeof(Status.SearchText)-1);
                                   break;
                    }
        }
      } else {
        Opt = option(Options,p,NUMMSGOPTIONS);
        switch (Opt) {
          case O_ALL  : Read.CheckUserScan = FALSE;
                        ValidCmd = TRUE;
                        if (Status.CurSecLevel < PcbData.UserLevels[SEC_J])
                          break;
                        Read.AllConf = TRUE;
                        getallresumestatus();
                        if (Flag == INSIDEREADLOOP)        /* if we're already reading messages */
                          Read.StayInConf = TRUE;          /* then don't jump out just yet */
                        break;
          case O_ALIAS: if (AliasSupport) {
                          if (NumTokens > 1) {
                            p = getnexttoken();
                            if (strcmp(p,"ON") == 0)
                              Status.UseAlias = TRUE;
                            else if (strcmp(p,"OFF") == 0)
                              Status.UseAlias = FALSE;
                            else
                              Status.UseAlias = (bool) (! Status.UseAlias);
                          } else
                            Status.UseAlias = (bool) (! Status.UseAlias);
                          aliasnames();
                        }
                        if (Flag == INSIDEREADLOOP)
                          Func = REDISPLAY;
                        ValidCmd = TRUE;
                        break;
          case O_BYE  :
          case O_GB   : Status.CapAsk = FALSE;
                        Status.CapBye = TRUE;
                        ValidCmd = TRUE;
                        break;
          case O_HELP : displayhelpfile(HLP_ENDR);
                        ValidCmd = TRUE;
                        if (Flag == INSIDEREADLOOP)
                          Func = REDISPLAY;
                        break;
          case O_CHAT : Func = CHAT;
                        ValidCmd = TRUE;
                        break;
          case O_COPY : Func = COPY;
                        ValidCmd = TRUE;
                        MoveConfNum = 0xFFFF;
                        if (NumTokens > 1) {
                          NumTokens--;
                          p = getnexttoken();
                          if (alldigits(p)) {
                            MoveConfNum = (unsigned short) atoi(p);
                            if (MoveConfNum > PcbData.NumConf)
                              MoveConfNum = 0xFFFF;
                          }
                        }
                        break;
          case O_DESEL: Func = DESELCONF;
                        ValidCmd = TRUE;
                        break;
          case O_EDIT : Func = EDITMSG;
                        ValidCmd = TRUE;
                        break;
          case O_FINDF: Func = FINDFROM;
                        ValidCmd = TRUE;
                        break;
          case O_FINDT: Func = FINDTO;
                        ValidCmd = TRUE;
                        break;
          case O_FLAG : Func = FLAGFILE;
                        ValidCmd = TRUE;
                        break;
#ifdef PCB152
          case O_FORW : Func = FORWARD;
                        ValidCmd = TRUE;
                        MoveConfNum = 0xFFFF;
                        if (NumTokens > 1) {
                          NumTokens--;
                          p = getnexttoken();
                          if (alldigits(p)) {
                            MoveConfNum = (unsigned short) atoi(p);
                            if (MoveConfNum > PcbData.NumConf)
                              MoveConfNum = 0xFFFF;
                          }
                        }
                        break;
#endif
          case O_FROM : DoUserSearch   = TRUE;
                        LastSrchCmd    = O_FROM;
                        Read.UserSrch |= FIND_FROM;
                        ValidCmd = TRUE;
                        break;
          case O_JUMP : Func = JUMPOUT;
                        ValidCmd = TRUE;
                        break;
          case O_KILL : Func = KILL;
                        ValidCmd = TRUE;
                        break;
          case O_LONG : Read.HeaderLen = HDR_LONG;
                        if (Flag == INSIDEREADLOOP) {
                          Backup.HeaderLen = HDR_LONG;
                          Func = REDISPLAY;
                        }
                        Display.KeepLines = Read.HeaderLen;
                        ValidCmd = TRUE;
                        break;
          case O_MOVE : Func = MOVE;
                        ValidCmd = TRUE;
                        MoveConfNum = 0xFFFF;
                        if (NumTokens > 1) {
                          NumTokens--;
                          p = getnexttoken();
                          if (alldigits(p)) {
                            MoveConfNum = (unsigned short) atoi(p);
                            if (MoveConfNum > PcbData.NumConf)
                              MoveConfNum = 0xFFFF;
                          }
                        }
                        break;
          case O_PREV : Read.Numbers[Read.NumGroups].First = (Flag == INSIDEREADLOOP ? Read.MsgNumber - 1 : 0x7FFFFFFFL);
                        Read.Numbers[Read.NumGroups].Last  = 1;
                        Read.NumGroups++;
                        Func = NOFUNC;
                        ValidCmd = TRUE;
                        break;
          case O_NEXT : Read.Numbers[Read.NumGroups].First = (Flag == INSIDEREADLOOP ? Read.MsgNumber + 1 : 1);
                        Read.Numbers[Read.NumGroups].Last  = 0x7FFFFFFFL; /* last number */
                        Read.KeepGoing = TRUE;
                        Read.NumGroups++;
                        Func = NOFUNC;
                        ValidCmd = TRUE;
                        break;
          case O_NET  : if (QwkSupport)
                          Read.Net = TRUE;
                        ValidCmd = TRUE;
                        break;
          case O_QWK  : Read.QWK        = TRUE;
                        OpenQWK         = TRUE;
                        Status.ZipCap   = TRUE;
                        Status.CapAsk   = FALSE;
                        Read.FoundTotal = 0;

                        if (Read.NumGroups != 0 || NewMsgs)  // caller specified range?
                          break;

                        if (Flag == INSIDEREADLOOP) {  // single message?
                          Read.CaptureSingle = TRUE;
                          Func = REDISPLAY;
                          break;
                        }

                        // else fall thru to do a READ SINCE with the capture

                        Read.Since     = TRUE;
                        Read.KeepGoing = TRUE;
                        openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
                        ValidCmd = TRUE;
                        break;
          case O_REPLY: Func = REPLY;
                        ValidCmd = TRUE;
                        break;
          case O_RMF  : if (Status.MemorizeNum != 0) {
                          Read.KeepGoing = TRUE;
                          Temp = Status.MemorizeNum;
                          Read.Numbers[Read.NumGroups].Last  = 0x7FFFFFFFL; /* last number */
                          Read.Numbers[Read.NumGroups].First = Temp;
                          Read.NumGroups++;
                        } else
                          displaypcbtext(TXT_NOTMEMORIZED,NEWLINE|LFBEFORE);
                        Func = NOFUNC;
                        ValidCmd = TRUE;
                        break;
          case O_RMB  : if (Status.MemorizeNum != 0) {
                          Temp = Status.MemorizeNum;
                          Read.Numbers[Read.NumGroups].Last  = 1;
                          Read.Numbers[Read.NumGroups].First = Temp;
                          Read.NumGroups++;
                        } else
                          displaypcbtext(TXT_NOTMEMORIZED,NEWLINE|LFBEFORE);
                        Func = NOFUNC;
                        ValidCmd = TRUE;
                        break;
          case O_RM   : if (Status.MemorizeNum != 0) {
                          Temp = Status.MemorizeNum;
                          Read.Numbers[Read.NumGroups].Last  = Temp;
                          Read.Numbers[Read.NumGroups].First = Temp;
                          Read.NumGroups++;
                        } else
                          displaypcbtext(TXT_NOTMEMORIZED,NEWLINE|LFBEFORE);
                        Func = NOFUNC;
                        ValidCmd = TRUE;
                        break;
          case O_RO   : Func = REPOTHER;
                        ValidCmd = TRUE;
                        break;
          case O_RRF  :
          case O_RRB  :
          case O_RR   : Temp = bassngltolong(MsgBase.Header.RefNumber);
                        ValidCmd = TRUE;
                        if (Temp == 0) {     /* no refer found so re-display */
                          Func = REDISPLAY;  /* the current message          */
                        } else {
                          switch(Opt) {
                            case O_RRF: Read.KeepGoing = TRUE;
                                        Read.Numbers[Read.NumGroups].Last  = 0x7FFFFFFFL; /* last number */
                                        Read.Numbers[Read.NumGroups].First = Temp;
                                        break;
                            case O_RRB: Read.Numbers[Read.NumGroups].Last  = 1;
                                        Read.Numbers[Read.NumGroups].First = Temp;
                                        Read.NumGroups++;
                                        break;
                            case O_RR : Read.Numbers[Read.NumGroups].Last  = Temp;
                                        Read.Numbers[Read.NumGroups].First = Temp;
                                        break;
                          }
                          Read.NumGroups++;
                          if (Read.MsgNumber == MsgReadPtr[Status.Conference])
                            MsgReadPtr[Status.Conference]--;
                          Func = NOFUNC;
                        }
                        break;
          case O_SEL  : Func = SELCONF;
                        ValidCmd = TRUE;
                        break;
          case O_SET  : NumTokens = setlastmessageread(NumTokens);
                        Func = NOFUNC;
                        ValidCmd = TRUE;
                        break;
          case O_SHORT: Read.HeaderLen = HDR_SHORT;
                        if (Flag == INSIDEREADLOOP) {
                          Backup.HeaderLen = HDR_SHORT;
                          Func = REDISPLAY;
                        }
                        Display.KeepLines = Read.HeaderLen;
                        ValidCmd = TRUE;
                        break;
          case O_SKIP : Func = SKIPMSG;
                        ValidCmd = TRUE;
                        break;
          case O_TB   : if (Flag == INSIDEREADLOOP) {
                          Read.Threading = TRUE;
                          ForwDirection  = FALSE;
                          ValidCmd = TRUE;
                        }
                        break;
          case O_TF   : if (Flag == INSIDEREADLOOP) {
                          Read.Threading = TRUE;
                          ForwDirection  = TRUE;
                          ValidCmd = TRUE;
                        }
                        break;
          case O_TS   : DoTextSearch = TRUE;
                        LastSrchCmd  = O_TS;
                        ValidCmd = TRUE;
                        /* if we've already been searching then stop */
                        /* now so we can search for new stuff        */
                        if (Read.NumSearches != 0) {
                          stopsearch();
                          Read.NumSearches = 0;
                          Status.SearchText[0] = 0;
                        }
                        break;
          case O_TO   : DoUserSearch   = TRUE;
                        LastSrchCmd    = O_TO;
                        Read.UserSrch |= FIND_TO;
                        ValidCmd = TRUE;
                        break;
          case O_USER : DoUserSearch  = TRUE;
                        LastSrchCmd   = O_USER;
                        Read.UserSrch = FIND_USER;
                        ValidCmd = TRUE;
                        break;
          case O_WAIT : Read.CheckUserScan = FALSE;
                        Read.MailWaitConf  = TRUE;
                        Read.AllConf       = TRUE;
                        Read.Conf          = 0;
                        ValidCmd = TRUE;
                        break;
          case O_WHO  : Func = WHOCMD;
                        ValidCmd = TRUE;
                        break;
          case O_YA   : Read.AnyMsgs   = FALSE;
                        Read.YourMsgs  = TRUE;
                        Read.MsgsToAll = TRUE;
                        ValidCmd = TRUE;
                        break;
          default     : switch (LastSrchCmd) {
                          case O_FROM: addtext(Read.UserNameFrom,p,sizeof(Read.UserNameFrom)-1);
                                       break;
                          case O_TO  : // fall thru - USER shares the same memory
                          case O_USER: addtext(Read.UserNameTo,p,sizeof(Read.UserNameTo)-1);
                                       break;
                          case O_TS  : // fall thru - default to TS search
                          default    : addtext(Status.SearchText,p,sizeof(Status.SearchText)-1);
                                       break;
                        }
        }
      }
    }
  }

  if (Func != NOFUNC) {
    if (OpenCapture || OpenQWK) {
      if (Func != REDISPLAY)
        Func = NOFUNC;
      goto capture;
    }

    /* if the interpreter loop has deallocated the TS search memory then   */
    /* don't let the backup-restore process accidently make it appear that */
    /* we're still in a text search mode or it will crash when it tries to */
    /* use the *now* de-allocated memory.                                  */
    if (Read.NumSearches == 0)
      Backup.NumSearches = 0;

    // the following variables are carried across the Read=Backup assignment
    // in order to ensure that the single message capture process works.
    if (Read.CaptureSingle) {
      Backup.QWK           = Read.QWK;
      Backup.Capture       = Read.Capture;
      Backup.CaptureSingle = Read.CaptureSingle;
      Backup.Found         = 0;
      Backup.FoundTotal    = 0;
    }

    Status.SearchText[0] = SaveByte;
    Read = Backup;
    return(Func);
  }

  if (Read.Threading) {
    if (ForwDirection) {
      Read.Numbers[Read.NumGroups].First = (Flag == INSIDEREADLOOP ? Read.MsgNumber + 1 : 1);
      Read.Numbers[Read.NumGroups].Last  = 0x7FFFFFFFL; /* last number */
      Read.KeepGoing = TRUE;
      Read.NumGroups++;
    } else {
      Read.Numbers[Read.NumGroups].First = (Flag == INSIDEREADLOOP ? Read.MsgNumber - 1 : 0x7FFFFFFFL);
      Read.Numbers[Read.NumGroups].Last  = 1;
      Read.NumGroups++;
    }
    memcpy(Status.SearchText,MsgBase.Header.SubjField,25);
    Status.SearchText[25] = 0;
    // for internet we must ignore the "Re: " in order to keep the threads
    // together
    if (memicmp(Status.SearchText,"Re: ",4) == 0) {
      memmove(Status.SearchText,Status.SearchText+4,22);
      memset(&Status.SearchText[24-4],' ',4);
    }
    Func = NOFUNC;
  }

  if (! DoTextSearch && Read.NumSearches > 0) {
    stopsearch();
    Read.NumSearches = 0;
    Status.SearchText[0] = 0;
  }

  if (DoTextSearch && Read.NumSearches == 0) {
    if (Status.SearchText[0] == 0) {
      freshline();
      Str[0] = 0;
      inputfield(Str,TXT_TEXTTOSCANFOR,sizeof(Status.SearchText)-1,NEWLINE|UPCASE|LFBEFORE|HIGHASCII,HLP_SRCH,mask_alphanum);
      if (Str[0] == 0)
        return(STOP);
      strcpy(Status.SearchText,Str);
    }

    if ((Read.NumSearches = tokenscan(Status.SearchText,Status.SearchInput,FALSE)) == -1) {
      displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
      DoTextSearch = FALSE;
      Read.NumSearches = 0;
      Status.SearchText[0] = 0;
    }
  }

  if (DoUserSearch) {
    if ((Read.UserSrch & FIND_TO) && Read.UserNameTo[0] == 0) {
      freshline();
      inputfield(Read.UserNameTo,TXT_USERSEARCHTONAME,sizeof(UsersRead.Name),NEWLINE|UPCASE|HIGHASCII|FIELDLEN|LFBEFORE,NOHELP,mask_alphanum);
      if (Read.UserNameTo[0] == 0)
        return(STOP);
    }
    if ((Read.UserSrch & FIND_FROM) && Read.UserNameFrom[0] == 0) {
      freshline();
      inputfield(Read.UserNameFrom,TXT_USERSEARCHFROMNAME,sizeof(UsersRead.Name),NEWLINE|UPCASE|HIGHASCII|FIELDLEN|LFBEFORE,NOHELP,mask_alphanum);
      if (Read.UserNameFrom[0] == 0)
        return(STOP);
    }
    if ((Read.UserSrch & FIND_USER) && Read.UserNameTo[0] == 0) {
      freshline();
      inputfield(Read.UserNameTo,TXT_USERSEARCHNAME,sizeof(UsersRead.Name),NEWLINE|UPCASE|HIGHASCII|FIELDLEN|LFBEFORE,NOHELP,mask_alphanum);
      if (Read.UserNameTo[0] == 0)
        return(STOP);
    }
    padstr(Read.UserNameTo,' ',sizeof(UsersRead.Name));
    padstr(Read.UserNameFrom,' ',sizeof(UsersRead.Name));
  }

  if (Read.NumGroups == 0) {
    if (NewMsgs && Read.NewDate == 0) {
      strcpy(Input,Status.LastDateOnStr);
      stripall(Input,'-');
      countrydate2(Input);
      inputfield(Input,TXT_MSGSDATETOSEARCH,6,GUIDE|FIELDLEN|NEWLINE|LFBEFORE,NOHELP,mask_numbers);
      if (Input[0] == 0)
        return(STOP);
      uncountrydate2(Input);
      Read.NewDate = datetojulian(Input);
    }
    if ((! (Read.Since || Read.NewDate)) && (Read.NumSearches > 0 || DoUserSearch)) {
      freshline();
      openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
      sprintf(Status.DisplayText,"%ld-%ld",MsgBase.Stats.LowMsgNum,MsgBase.Stats.HighMsgNum);
      Input[0] = 0;
      inputfield(Input,TXT_MSGSEARCHFROM,sizeof(Input)-1,UPCASE|NEWLINE,NOHELP,mask_msgrange);
      if (Input[0] == 0)
        return(STOP);
      if (strchr(Input,'-') == NULL && strchr(Input,'+') == NULL && Input[0] >= '0' && Input[0] <= '9')
        addchar(Input,'+');
      NumTokens = tokenize(Input);
      goto top;
    }
    if (Read.AllConf && ! NewMsgs)
      Read.Since = TRUE;
    if (Read.Since || Read.NewDate || Read.FromMsgs || Read.YourMsgs || Read.UnReadOnly) {
      Read.FullRange = TRUE;
      Read.KeepGoing = TRUE;
      Read.NumGroups = 1;
    }
  } else if (Read.AllConf) {
    Read.NumGroups = 1;
    Read.CurGroup  = 0;
  }

  if (Func == NOFUNC && Read.NumGroups == 0 && ! Read.AllConf && ! Read.Since) {
    if (Status.CapBye) {  /* was a BYE command given without a "C/D/Z" command? */
      freshline();
      cleanupmsgmemory();
      loguseroff(NLOGOFF);
    }

    if (! ValidCmd) {
      displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE);
      Status.SearchText[0] = 0;
      Read.UserNameTo[0]   = 0;
      Read.UserNameFrom[0] = 0;
    }

    if ((NumTokens = messagereadprompt()) == 0)
      Func = STOP;
    else {
      Flag = OUTSIDEREADLOOP;
      goto top;
    }
  }

capture:
  if (OpenCapture || OpenQWK) {
    MsgReadCharge = 0;

    if (Status.QuickScan) {
      // don't allow captures with quick scan so turn them off
      OpenCapture   = FALSE;
      OpenQWK       = FALSE;
      Read.QWK      = FALSE;
      Status.ZipCap = FALSE;
      Status.CapAsk = FALSE;
    } else {
      if (OpenQWK)
        OpenCapture = FALSE;   // don't let QWK and D be used at the same time

      if (OpenCapture) {
        if (opencapturefile("") != -1) {
          setcapture(FALSE);
          freshline();
          displaypcbtext(TXT_ABORTKEYS,NEWLINE|LFAFTER);
          setcapture(TRUE);
          Read.Capture = TRUE;
          startdisplay(FORCENONSTOP);
          backuppointers();
          MsgBase.ConfNum = 0xFFFE;  /* force it to re-open the message base (don't use a -1) */
          Display.ShowOnScreen = FALSE;
        } else {
          OpenCapture = FALSE;
          Status.ZipCap = FALSE;
          Status.CapAsk = FALSE;
          Func = STOP;
        }
      }

      if (OpenQWK) {
        if (openqwkfile() != -1) {
          freshline();
          displaypcbtext(TXT_ABORTKEYS,NEWLINE);
          startdisplay(FORCENONSTOP);
          backuppointers();
          MsgBase.ConfNum = 0xFFFE;  /* force it to re-open the message base (don't use a -1) */
          Display.ShowOnScreen = FALSE;
        } else {
          OpenQWK = FALSE;
          Read.QWK = FALSE;
          Status.ZipCap = FALSE;
          Status.CapAsk = FALSE;
          Func = STOP;
        }
      }
    }
  }

  return(Func);
}


/********************************************************************
*
*  Function:  displaymsgline()
*
*  Desc    :  displays the message text lines
*
*             NOTE:  displaymsg() calls this routine instead of calling the
*             println() routine in case a text search is active.  If it is and
*             the text to be displayed contains the "found" text, then it
*             highlights the found text.
*/

static void _NEAR_ LIBENTRY displaymsgline(char *Str) {
  int X;

  checkstack();
  if (*Str == '-' && *(Str+1) == '>' && *(Str+2) <= ' ' && Control.GraphicsMode) {
    printcolor(PCB_CYAN);
    print("->");
    printdefcolor();
    Str += 2;
  }

  X = strlen(Str);
  if (X > 79) {
    if (Str[0] == '!' && Str[1] == '|') {
      // found RIPscrip code
    } else if (strnchr(Str,27,X) != NULL) {
      // found ANSI code
    } else if (findtoken(Str) != 0) {
      // if Found != 0 then findtoken() found a token.  Trouble is... it
      // sets value of the byte at the token start location to 0 so we need
      // to restore it back to an '@' character.
      Str[FindTokenStart] = '@';
    } else {
      // didn't find anything special on the line, so truncate it!
      Str[79] = 0;
    }
  }

  if (Read.NumSearches < 1)
    printxlated(Str);
  else
    printfoundtext(Str);
}


static void _NEAR_ LIBENTRY displaymsgheaderline(char *Str, bool Wrap) {
  char Temp[128];

  checkstack();
  if (Wrap) {
    memset(Temp,' ',Text->Left);
    strcpy(&Temp[Text->Left],&Str[79]);
    Str[79] = 0;
    displaymsgline(Str);
    newline();
    cleareol();
    displaymsgline(Temp);
  } else
    displaymsgline(Str);

  if ((curcolor() & 0xF0) != 0)
    printcolor(0x07);
}


#ifdef PCBCOMM2
static void _NEAR_ LIBENTRY displayterseline(char *Str) {
  checkstack();
  sendstr(Str,strlen(Str));
  sendbyte('\r');
}
#endif


static bool _NEAR_ LIBENTRY gettonamefromheader(char *Str) {
  checkstack();
  if (memcmp(MsgBase.Header.ToField,"ALL                      ",25) == 0) {
    strcpy(Str,Text->AllName);
    return(TRUE);
  } else {
    movestr(Str,MsgBase.Header.ToField,sizeof(UsersRead.Name));
    stripright(Str,' ');
  }
  return(FALSE);
}


static void _NEAR_ LIBENTRY getfromnamefromheader(char *Str) {
  checkstack();
  if (memcmp(MsgBase.Header.FromField,"RETURN RECEIPT           ",25) == 0) {
    strcpy(Str,Text->RetRecName);
  } else {
    movestr(Str,MsgBase.Header.FromField,sizeof(UsersRead.Name));
    stripright(Str,' ');
  }
}


static void _NEAR_ LIBENTRY getsubjfromheader(char *Str) {
  char Temp[30];

  checkstack();
  movestr(Temp,MsgBase.Header.SubjField,sizeof(MsgBase.Header.SubjField));
  stripright(Temp,' ');

  if (memcmp(MsgBase.Header.SubjField,"COMMENT ",8) == 0 && (Temp[8] == ' ' || Temp[8] == '(')) {
    strcpy(Str,Text->Comment);
    if (Temp[8] != ' ') {
      addchar(Str,' ');
      strcat(Str,&Temp[8]);
    }
  } else {
    strcpy(Str,Temp);
  }
}


static void _NEAR_ LIBENTRY copyconfname(char *Str, int Width) {
  int   Len;
  char *p;

  checkstack();
  Len = Read.ConfUpStrLen;
  p   = Read.ConfUpStr;

  if (Len > Width) {
    Len = Width;
    p += (Read.ConfUpStrLen - Width);
  }

  memcpy(Str,p,Len);

  for (Str += Len; Len < Width; Len++, Str++)
    *Str = ' ';
}


/********************************************************************
*
*  Function:  displaymsgheader()
*
*  Desc    :  displays the message header
*/

static void _NEAR_ LIBENTRY displaymsgheader(void) {
  bool  ToAll;
  bool  WrapLine;
  char  Width;
  char  *p;
  long  Num;
  char  FromName[26];
  pcbtexttype Buf;
  char  Addressee[256];
  char  Temp[256];
  char  Str[256];

  #ifdef PCBCOMM2
    bool  Terse;

    if ((Terse = Status.TerseMode & ! Status.Capture && Asy.Online == REMOTE && ! Asy.LostCarrier) != FALSE) {
      Asy.Online = LOCAL;  /* force LOCAL ONLY mode */
      sendstr("\rMSG\r",6);
    }
  #endif

  checkstack();
  // reset the line counter now so that the top of the message header will
  // move to the top of the screen on long messages.
  Display.NumLinesPrinted = 0;

  if (Read.MsgClear && ! Status.Capture)
    printcls();
  else {
    freshline();  // get to a fresh line and make sure
    newline();    // we have a blank one in between
  }

  if (Read.PreMsg)
    displayfile(Read.PreMsgFile,NOALTERNATE);

  // Get the TO NAME information first just in case the message is addressed
  // to ALL because if it is, then the READ status needs to be N/A.
  WrapLine = FALSE;
  if ((MsgBase.Header.ExtendedStatus & HDR_TO) != 0 && (p = getextendedheader(EXTHDR_TO,MsgBase.Body)) != NULL) {
    memcpy(Temp,p,EXTDESCLEN);
    Temp[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_TO2,MsgBase.Body)) != NULL) {
      memcpy(&Temp[EXTDESCLEN],p,EXTDESCLEN);
      Temp[EXTDESCLEN*2] = 0;
    }
    stripright(Temp,' ');
    if (MsgBase.Header.ToField[0] != ' ') {
      strcat(Temp,", ");
      ToAll = gettonamefromheader(Str);
      strcat(Temp,Str);
      stripright(Temp,' ');
    } else
      ToAll = FALSE;

    if (strlen(Temp) + Text->Left > 79) {
      WrapLine = TRUE;
      Read.PossibleNoScroll = TRUE;
    }
  } else {
    // check for @USER@, @##@ or @LIST@ in TO field
    if (MsgBase.Header.ToField[0] == '@' && Read.Access.Msg.ToYou) {
      if (Read.Access.Msg.List) {
        maxstrcpy(Temp,Status.DisplayName,sizeof(Temp));
      } else {
        if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READALLMAIL]) {
          getpcbtext(TXT_GENERICMESSAGE,&Buf);
          maxstrcpy(Temp,Buf.Str,sizeof(Temp));
        } else {
          maxstrcpy(Temp,Status.DisplayName,sizeof(Temp));
        }
      }
      Read.MsgStatus.Read = FALSE;  // Read date & time is n/a on generic msgs
      ToAll = FALSE;
    } else {
      if (memcmp(MsgBase.Header.ToField,"@LIST@",6) == 0) {
        getpcbtext(TXT_CARBONLISTMSG,&Buf);
        maxstrcpy(Temp,Buf.Str,sizeof(Temp));
        Read.MsgStatus.Read = FALSE;  // Read date & time is n/a on generic msgs
        ToAll = FALSE;
      } else
        ToAll = gettonamefromheader(Temp);
    }
  }
  buildstr(Addressee,Text->To," ",Temp,NULL);

  // If the message is to you, and if you are not capturing mail, then make
  // the bell sound.
  //
  // NOTE: If you are doing an R Y command it won't make the bell sound (but
  // will if you do an R YA command) because by definition an R Y command
  // will find *only* mail that is TO you, and that means every message found
  // would sound the alarm, making the alarm redundant.

  if (Read.Access.Msg.ToYou && ! Read.Capture) {
    if (Read.MsgsToAll || ! Read.YourMsgs) {
      #ifdef PCBCOMM2
      if (! Terse) {  // if in terse mode, don't sound alarm
      #endif
        bell();
      #ifdef PCBCOMM2
      }
      #endif
    }
  }

  // end of TO NAME processing


  // start of date line
  if (Control.GraphicsMode) {
    printcolor(PcbData.MsgHdr1Color);  cleareol();
  }
  maxstrcpy(Str,Text->DateNum,sizeof(Str));
  sprintf(Temp,"%8.8s (%5.5s)",MsgBase.Header.Date,MsgBase.Header.Time);
  countrydate(Temp);
  memcpy(&Str[Text->Left],Temp,16);
  if (Read.HeaderLen == HDR_SHORT) {
    if (! ToAll) {
      if (Read.MsgStatus.Read && bassngltolong(MsgBase.Header.ReplyDate) != 0)
        Str[Text->Left+18] = '*';
    }
    if (MsgBase.Header.ReplyStatus == 'R' && MsgBase.Header.ToField[0] != '@')
      Str[Text->Left+20] = 'R';
  }
  print(Str);
  sprintf(Str," %ld %s %ld ",Read.MsgNumber,Text->Separator,MsgBase.Stats.HighMsgNum);
  print(Str);
  if ((Num = bassngltolong(MsgBase.Header.RefNumber)) != 0)
    sprintf(Temp,"(%s %ld)",Text->Refer,Num);
  else
    sprintf(Temp,"(%s %s)",Text->Refer,Text->None);
  print(Temp);
  if ((curcolor() & 0xF0) != 0)
    printcolor(0x07);
  newline();
  // end of DATE line


  // start of TO line
  if (Control.GraphicsMode) {
    printcolor(PcbData.MsgHdr2Color);  cleareol();
  }
  displaymsgheaderline(Addressee,WrapLine);
  newline();
  // end of TO line


  // start of FROM line
  if (Control.GraphicsMode) {
    printcolor(PcbData.MsgHdr3Color);  cleareol();
  }
  WrapLine = FALSE;
  if ((MsgBase.Header.ExtendedStatus & HDR_FROM) != 0 && (p = getextendedheader(EXTHDR_FROM,MsgBase.Body)) != NULL) {
    memcpy(Temp,p,EXTDESCLEN);
    Temp[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_FROM2,MsgBase.Body)) != NULL) {
      memcpy(&Temp[EXTDESCLEN],p,EXTDESCLEN);
      Temp[EXTDESCLEN*2] = 0;
    }
    stripright(Temp,' ');
    if (MsgBase.Header.FromField[0] != ' ') {
      getfromnamefromheader(FromName);
      sprintf(Str,"%s %s, %-25.25s",Text->From,Temp,FromName);
    } else
      sprintf(Str,"%s %s",Text->From,Temp);
    stripright(Str,' ');
    if (strlen(Str) > 79) {
      WrapLine = TRUE;
      Read.PossibleNoScroll = TRUE;
    }
  } else {
    getfromnamefromheader(FromName);
    sprintf(Str,"%s %-25.25s",Text->From,FromName);
  }
  stripright(Str,' ');
  displaymsgheaderline(Str,WrapLine);
  newline();
  // end of FROM line


  // start of SUBJECT line
  if (Control.GraphicsMode) {
    printcolor(PcbData.MsgHdr4Color);  cleareol();
  }
  WrapLine = FALSE;
  if ((MsgBase.Header.ExtendedStatus & HDR_SUBJ) != 0 && (p = getextendedheader(EXTHDR_SUBJECT,MsgBase.Body)) != NULL) {
    memcpy(Temp,p,EXTDESCLEN);
    Temp[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_SUBJ2,MsgBase.Body)) != NULL) {
      memcpy(&Temp[EXTDESCLEN],p,EXTDESCLEN);
      Temp[EXTDESCLEN*2] = 0;
    }
    stripright(Temp,' ');
    buildstr(Str,Text->Subject," ",Temp,NULL);
    if (strlen(Str) > 79) {
      WrapLine = TRUE;
      Read.PossibleNoScroll = TRUE;
    }
  } else {
    getsubjfromheader(Temp);
    sprintf(Str,"%s %s",Text->Subject,Temp);
  }
  stripright(Str,' ');
  displaymsgheaderline(Str,WrapLine);
  if ((curcolor() & 0xF0) != 0)
    printcolor(0x07);
  newline();
  // end of SUBJECT line


  if (Read.HeaderLen == HDR_LONG) {
    // start of READ/STATUS line
    if (Control.GraphicsMode) {
      printcolor(PcbData.MsgHdr5Color);  cleareol();
    }
    if (ToAll)
      p = Text->NA;
    else {
      if (Read.MsgStatus.Read) {
        if ((Num = bassngltolong(MsgBase.Header.ReplyDate)) != 0) {
          lascii(Temp,Num);
          yymmddtostr(Str,Temp);
          countrydate(Str);
          sprintf(Temp,"%8.8s (%5.5s)",Str,MsgBase.Header.ReplyTime);
          p = Temp;
        } else p = Text->NotRead;
      } else
        p = Text->NotRead;
    }
    strcpy(Str,Text->Read);
    addchar(Str,' ');
    strcat(Str,p);
    padstr(Str,' ',Text->Right);
    strcpy(&Str[Text->Right-strlen(Text->Status)-1],Text->Status);
    switch (Read.MsgStatus.Type) {
      case RCVR:
      case CMNT: p = Text->ReceiverOnly; break;
      case SPWD: p = Text->SenderPwrd;   break;
      case GPWD: p = Text->GroupPwrd;    break;
      case PBLC: p = Text->Public;       break;
    }
    addchar(Str,' ');
    strcat(Str,p);

    if (MsgBase.Header.EchoFlag == 'E')
      strcat(&Str[Text->Right],Text->Echo);

    if ((MsgBase.Header.ExtendedStatus & HDR_FILE) != 0 && (p = getextendedheader(EXTHDR_ATTACH,MsgBase.Body)) != NULL)
      strcat(&Str[Text->Right],Text->File);

    Str[79] = 0;           // make sure it doesn't go over 79 characters.
    stripright(Str,' ');

    displaymsgline(Str);
    if ((curcolor() & 0xF0) != 0)
      printcolor(0x07);
    newline();
    // end of READ/STATUS line


    // start of CONF/TYPE line
    if (Control.GraphicsMode) {
      printcolor(PcbData.MsgHdr6Color);  cleareol();
    }
    if (Read.NumSearches > 0) {
      p = Text->TextScan;
      Width = Text->TextScanWidth;
    } else if (Read.YourMsgs) {
      if (Read.FromMsgs) {
        p = Text->ToOrFrom;
        Width = Text->ToOrFromWidth;
      } else {
        p = Text->ForYou;
        Width = Text->ForYouWidth;
      }
    } else if (Read.FromMsgs) {
      p = Text->FromYou;
      Width = Text->FromYouWidth;
    } else if (Read.Threading) {
      p = Text->Thread;
      Width = Text->ThreadWidth;
    } else {
      p = Text->General;
      Width = Text->GeneralWidth;
    }
    copyconfname(&p[Text->Left],Width);
//  memcpy(&p[Text->Left],Read.ConfUpStr,25);
    print(p);

    Str[0] = 0;
    if (Read.AllConf)
      strcpy(Str,Text->AllConfIndicator);
    if (! Read.UpdateMsgStatus)
      strcat(Str,Text->ReadOnlyIndicator);
    if (Read.Forward)
      strcat(Str," (+)");
    else if (Read.Backward)
      strcat(Str," (-)");
    print(Str);

    if (MsgBase.Header.ReplyStatus == 'R' && MsgBase.Header.ToField[0] != '@') {
      if (awherex() + strlen(Text->Replies)+1 > 79) {
        Str[0] = ' ';
        strcpy(Str + 1,Text->Replies);
        Str[79 - awherex()] = 0;
        print(Str);
      } else {
        print(" ");
        print(Text->Replies);
      }
    }
    if ((curcolor() & 0xF0) != 0)
      printcolor(0x07);
    newline();
    // end of CONF/TYPE line
  }


  #ifdef PCBCOMM2
    if (Terse)
      Asy.Online = REMOTE;
  #endif
}


/********************************************************************
*
*  Function:  displaymsgbody()
*
*  Desc    :  displays the body
*/

static void _NEAR_ LIBENTRY displaymsgbody(void) {
  char  *p;
  char  *q;
  char  *r;
  int   Bytes;
  char  Str[EXTDESCLEN];

  #ifdef PCBCOMM2
    if (Status.TerseMode && ! Status.Capture) {
      startdisplay(FORCENONSTOP);
      Status.Poff = TRUE;          /* this will turn non-stop off down below */
    }
  #endif

  checkstack();
  printdefcolor();
  newline();

  Bytes = (MsgBase.Header.NumBlocks-1)*128; /* subtract 1 from NumBlocks to get Body length */

  #ifdef PCB152
  // display FORWARD extended header
  if ((p = getextendedheader(EXTHDR_FORWARD,MsgBase.Body)) != NULL) {
    maxstrcpy(Str,p,EXTDESCLEN);
    if ((p = strchr(Str,' ')) != NULL) {
      *p = 0;
      makeconfstr(Status.DisplayText,atoi(Str));
      displaypcbtext(TXT_FORWARDFROM,NEWLINE);
      maxstrcpy(Status.DisplayText,p+1,sizeof(Status.DisplayText));
      displaypcbtext(TXT_FORWARDBY,NEWLINE|LFAFTER);
      printdefcolor();
    }
  }
  #endif

  // skip over any EXTENDED HEADERS found inside the body of the message
  p = skipextendedheader(MsgBase.Body);
  Bytes -= ((int) (p - MsgBase.Body));

  // if this is an internet conference, then disable color interpretation
  if (Status.CurConf.LongToNames)
    Status.DisableColor = TRUE;

  // Begin display of the message.  Each line MUST be terminated by either a
  // 0x0D or a 0xE3.  This is the LineSeparator character.  If a line is not
  // terminated in that fashion then the end of the message has been reached.
  // Additionally, if the number of bytes runs down to zero then it is also
  // considered to have reached the end of the message.

  if ((q = strnchr(p,LineSeparator,Bytes)) != NULL) {
    while (1) {
      Bytes -= ((int) (q-p+1));
      r = strnchr(q+1,LineSeparator,Bytes);
      *q = 0;
      stripright(p,' ');

      if (*p != CTRL_A)            // don't display FIDO's CTRL-A lines
        displaymsgline(p);

      *q = LineSeparator;
      if (Bytes <= 0 || r == NULL)
        break;

      if (*p != CTRL_A)            // don't display FIDO's CTRL-A lines
        newline();

      if (Display.AbortPrintout)
        break;

      p = q + 1;
      q = r;
    }
  }

  // in case color was disabled, re-enable it right now
  Status.DisableColor = FALSE;

  if (Status.Poff && ! Read.Capture) { // did someone use the @POFF@ command?
    startdisplay(FORCECOUNTLINES);     // if so, force it back off to avoid
    Status.Poff = FALSE;               // a non-stop display of all messages
  }
}


static void _NEAR_ LIBENTRY skiptolastmsg(void) {
  checkstack();
  if (Read.UpdatePtrs) {
    freshline();
    lascii(Status.DisplayText,MsgBase.Stats.HighMsgNum);
    displaypcbtext(TXT_LASTMSGREADSETTO,NEWLINE|LFBEFORE);
    MsgReadPtr[Status.Conference] = MsgBase.Stats.HighMsgNum;
  }
}


static void _NEAR_ LIBENTRY writemsgheader(void) {
  char tempbuff[26];       /* temporary */
  long temp;               /* buffers   */

  checkstack();
  QWK->MsgRecord.Status=MsgBase.Header.Status;      /* copy message status  */

  sprintf(tempbuff,"%-7ld",Read.MsgNumber);
  memcpy(QWK->MsgRecord.Number,tempbuff,7);

  memcpy(QWK->MsgRecord.Date,MsgBase.Header.Date,8);        // copy date
  memcpy(QWK->MsgRecord.Time,MsgBase.Header.Time,5);        // copy time
  memcpy(QWK->MsgRecord.FromName,MsgBase.Header.FromField,25); // copy from name
  memcpy(QWK->MsgRecord.Subject,MsgBase.Header.SubjField,25);   // copy subject
  memcpy(QWK->MsgRecord.Password,MsgBase.Header.Password,12);   // copy password

  /* check for @USER@ in TO field */
  if (MsgBase.Header.ToField[0] == '@' && Read.Access.Msg.ToYou) {
    if (Read.Access.Msg.List) {
      memset(QWK->MsgRecord.ToName,' ',25);
      memcpy(QWK->MsgRecord.ToName,Status.DisplayName,strlen(Status.DisplayName)); // copy to name
    } else {
      if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READALLMAIL])
        memcpy(QWK->MsgRecord.ToName,MsgBase.Header.ToField,25); // copy to name
      else {
        memset(QWK->MsgRecord.ToName,' ',25);
        memcpy(QWK->MsgRecord.ToName,Status.DisplayName,strlen(Status.DisplayName)); // copy to name
      }
    }
  } else {
    memcpy(QWK->MsgRecord.ToName,MsgBase.Header.ToField,25); // copy to name
  }

  temp=bassngltolong(MsgBase.Header.RefNumber);
  sprintf(tempbuff,"%-8ld",temp);                           // copy ref number
  memcpy(QWK->MsgRecord.ReferenceNum,tempbuff,8);

  sprintf(tempbuff,"%-6d",MsgBase.Header.NumBlocks);
  memcpy(QWK->MsgRecord.NumBlocks,tempbuff,6);              // copy # of blocks

  switch (MsgBase.Header.NetTag) {
    case   0:
    case ' ':
    case 255: QWK->MsgRecord.NetTag = 0;   break;
    default : QWK->MsgRecord.NetTag = '*'; break;
  }

  QWK->MsgRecord.SpecialFlag=MSG_ACTIVE;                              // msg must be active
  QWK->MsgRecord.ConfNum=MsgBase.ConfNum;                             // copy conf number
  QWK->MsgRecord.PacketMsgNum++;                                      // count total # of msgs
  if (dosfwrite(&QWK->MsgRecord,sizeof(qwkmsgtype),&QWK->File) != -1) // now write it out
    QWK->RecordCount++;                                               // update index count
}


static void LIBENTRY writemsgbody(DOSFILE *fptr) {
  int BodyLen;

  checkstack();
  BodyLen = 128 * (MsgBase.Header.NumBlocks-1);

  if (Status.CurConf.ConfType != 0) {
    int   Len;
    int   LineLen;
    int   NumReplacedChars;
    char *p;
    char *q;

    // search for CTRL-A lines and blank them out
    Len = BodyLen;
    NumReplacedChars = 0;
    for (p = MsgBase.Body; Len > 0 && (q = strnchr(p,1,Len)) != NULL; ) {
      // subtract the # of bytes "skipped" from the total Len left to process
      Len -= (int) (q - p);

      // point p at the CTRL-A character
      p = q;

      // locate the next line separator so that we know how long the text is
      // that belongs to the CTRL-A line.  If there is *NO* line separator,
      // then we must be done so break out
      if ((q = strnchr(p,LineSeparator,Len)) == NULL)
        break;

      // q now points to the line separator
      q++;                          // skip over the line separator
      LineLen  = (int) (q - p);     // calculate the length of this line
      Len     -= LineLen;           // subtract # of bytes to be processed
      NumReplacedChars += LineLen;  // but remember how many we replaced
      // if there are bytes left in the body that haven't been processed yet
      // then move them down towards the beginning, copying from the byte
      // that starts the NEXT line into the position where the CTRL-A was
      // found
      if (Len > 0)
        memmove(p,q,Len);
    }

    // if we replaced any bytes at all, then we need to fill up the rest of
    // the message body (after what has become the true "end of message") with
    // spaces.
    if (NumReplacedChars != 0)
      memset(MsgBase.Body+BodyLen-NumReplacedChars,' ',NumReplacedChars);
  }

  if (dosfwrite(MsgBase.Body,BodyLen,fptr) != -1)
    QWK->RecordCount += (MsgBase.Header.NumBlocks-1);       // update index count
}


static void _NEAR_ LIBENTRY selectconference(bool Select) {
  checkstack();
  if (Select) {
    displaypcbtext(TXT_SELECTED,NEWLINE|LFBEFORE);
    setbit(&ConfReg[USR],Status.Conference);
  } else {
    displaypcbtext(TXT_DESELECTED,NEWLINE|LFBEFORE);
    unsetbit(&ConfReg[USR],Status.Conference);
  }
}


static int _NEAR_ LIBENTRY replycommand(int *CurPos) {
  checkstack();
  newline();
  #pragma warn -stv
  switch (enterreply(&MsgBase,Read.MsgStatus,Read.Access,FALSE)) {
    case KILLMSG:  kill(Read.MsgNumber,FALSE,FALSE,FALSE);  //lint !e534
                   *CurPos = -1; /* force a re-read of the index */
                   break;
    case ABORTMSG: return(-1);
  }
  #pragma warn +stv
  return(0);
}


static void _NEAR_ LIBENTRY quickscan(void) {
  char static Start[4] = "A  ";
  char *p;
  char *q;
  long RefNum;
  char RefStr[8];
  char Str[80];

  checkstack();
  newline();
  printcolor(PCB_CYAN);
  if (Status.HeaderScan) {
    Start[0] = (MsgBase.Header.ActiveFlag == MSG_INACTIVE ? 'I' : 'A');
    print(Start);
  }

  if ((RefNum = bassngltolong(MsgBase.Header.RefNumber)) != 0)
    sprintf(RefStr,"%-7ld",RefNum);
  else
    strcpy(RefStr,"       ");

  /* check for @USER@ in TO field */
  if (MsgBase.Header.ToField[0] == '@' && Read.Access.Msg.ToYou) {
    if (Read.Access.Msg.List) {
      p = Status.DisplayName;
    } else {
      if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READALLMAIL])
        p = MsgBase.Header.ToField;
      else
        p = Status.DisplayName;
    }
  } else {
    if (MsgBase.Header.ToField[0] > ' ')
      p = MsgBase.Header.ToField;
    else if ((MsgBase.Header.ExtendedStatus & HDR_TO) == 0 || (p = getextendedheader(EXTHDR_TO,MsgBase.Body)) == NULL)
      p = MsgBase.Header.ToField;
  }

  if (MsgBase.Header.FromField[0] > ' ')
    q = MsgBase.Header.FromField;
  else if ((MsgBase.Header.ExtendedStatus & HDR_FROM) == 0 || (q = getextendedheader(EXTHDR_FROM,MsgBase.Body)) == NULL)
    q = MsgBase.Header.FromField;

  sprintf(Str,"%c%-7ld %s %-15.15s %-15.15s %-25.25s",
          MsgBase.Header.Status,
          Read.MsgNumber,
          RefStr,
          p,
          q,
          MsgBase.Header.SubjField);
  stripright(Str,' ');
  print(Str);
}


static void _NEAR_ LIBENTRY writefiletoattachlist(char *ViewName, char *StoredName, unsigned ConfNum, long MsgNum) {
  DOSFILE AttachList;
  char    FileName[66];
  char    Output[256];

  buildstr(FileName,PcbData.TmpLoc,"ATTACHED.LST",NULL);
  if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE|OPEN_APPEND,&AttachList) == -1)
    return;

  sprintf(Output,"%u,%ld,%s,%s\r\n",ConfNum,MsgNum,StoredName,ViewName);
  dosfputs(Output,&AttachList);
  dosfclose(&AttachList);
  Read.Attached = TRUE;
}


static void _NEAR_ LIBENTRY showattachment(void) {
  int  NumTokens;
  char *p;
  long Size;
  long Limit;
  char StoredName[13];
  char ViewName[13];
  char FullStoredPath[66];
  char FullCopyPath[66];
  char Str[EXTDESCLEN];

  checkstack();
  if ((MsgBase.Header.ExtendedStatus & HDR_FILE) == 0 || (p = getextendedheader(EXTHDR_ATTACH,MsgBase.Body)) == NULL)
    return;

  maxstrcpy(Str,p,sizeof(Str));
  NumTokens = tokenizestr(Str);

  if (NumTokens == 0)
    return;

  maxstrcpy(ViewName,getnexttoken(),sizeof(ViewName));
  maxstrcpy(Status.DisplayText,ViewName,sizeof(Status.DisplayText));
  displaypcbtext(TXT_ATTACHEDFILE,LFBEFORE);

  if (--NumTokens != 0) {
    p = getnexttoken();             // get (####) size parameter
    Size = atol(p+1);               // skip over left paren to get size
    comma(Status.DisplayText,Size); // display with country-specific commas
    displaypcbtext(TXT_FILESIZE,DEFAULTS);

    if (QwkSupport && Read.QWK) {
      Limit = (Read.Access.Msg.ToYou ? UsersData.QwkConfig.PersonalAttachLimit : UsersData.QwkConfig.PublicAttachLimit);
      if (Size <= Limit) {
        maxstrcpy(StoredName,getnexttoken(),sizeof(StoredName));
        buildstr(FullCopyPath,PcbData.TmpLoc,StoredName,NULL);
        buildstr(FullStoredPath,Status.CurConf.AttachLoc,StoredName,NULL);
        if (pcbcopyfile(FullStoredPath,FullCopyPath,FALSE,FALSE,"",FALSE) == 0) {
          writefiletolist(StoredName);
          writefiletoattachlist(ViewName,StoredName,Read.Conf,Read.MsgNumber);
        }
      }
    }
  }
  newline();

  // if Display.CountLines is turned off, then we are capturing to a file
  // so don't bother displaying the attach commands.

  if (Display.CountLines)
    displaypcbtext(TXT_SHOWATTACHCMDS,LFBEFORE);
}


static void _NEAR_ LIBENTRY showcarbonnames(void) {
  bool          FirstTime;
  int           Limit;
  int           Len;
  char          TimeStr[6];
  char          DateStr[9];
  msgcarbontype *p;

  checkstack();
  if ((MsgBase.Header.ExtendedStatus & HDR_CARB) == 0)
    return;

  FirstTime = TRUE;
  p = (msgcarbontype *) MsgBase.Body;
  while (p->Ident == EXTHDRID) {
    if (memcmp(p->Function,ExtHdrFunctions[EXTHDR_LIST],EXTFUNCLEN) == 0) {
      if (FirstTime) {
        newline();
        FirstTime = FALSE;
      }

      // find the first space *after* the end of the name
      for (Len = sizeof(p->Descript)-1; p->Descript[Len] == ' '; Len--);

      if (Len <= 25)
        Limit = 26;
      else if (Len > 79-20)
        Limit = 79 - 20;
      else
        Limit = Len + 1;

      memcpy(Status.DisplayText,p->Descript,Limit);
      Status.DisplayText[Limit] = 0;

      if (p->Status == EXTSTAT_READ) {
        DateStr[0] = p->DateRead[0];
        DateStr[1] = p->DateRead[1];
        DateStr[3] = p->DateRead[2];
        DateStr[4] = p->DateRead[3];
        DateStr[6] = p->DateRead[4];
        DateStr[7] = p->DateRead[5];
        DateStr[2] = '/';
        DateStr[5] = '/';
        DateStr[8] = 0;
        strcat(Status.DisplayText,countrydate(DateStr));
        addchar(Status.DisplayText,' ');
        TimeStr[0] = p->TimeRead[0];
        TimeStr[1] = p->TimeRead[1];
        TimeStr[3] = p->TimeRead[2];
        TimeStr[4] = p->TimeRead[3];
        TimeStr[2] = ':';
        TimeStr[5] = 0;
        strcat(Status.DisplayText,TimeStr);
      } else
        stripright(Status.DisplayText,' ');

      displaypcbtext(TXT_CARBONNAME,LFBEFORE);
    }
    p++;
  }
}


static void _NEAR_ LIBENTRY flagorview(msgfunctype Func, int NumTokens) {
  char *p;
  char StoredName[13];
  char ViewName[13];
  char FullStoredPath[66];
  char FullViewPath[66];
  char Str[EXTDESCLEN];

  checkstack();
  if ((MsgBase.Header.ExtendedStatus & HDR_FILE) == 0 || (p = getextendedheader(EXTHDR_ATTACH,MsgBase.Body)) == NULL) {
    freshline();
    switch (Func) {
      case FLAGFILE: flagfiles(NumTokens-1);
                     break;
      case VIEWFILE: viewfile(NumTokens-1);
                     Status.LastView[0] = 0;
                     break;
    }
    return;
  }

  maxstrcpy(Str,p,sizeof(Str));
  NumTokens = tokenizestr(Str);
  if (NumTokens == 0)
    return;

  maxstrcpy(ViewName,getnexttoken(),sizeof(ViewName));

  getnexttoken();   //lint !e534  throw away file size
  NumTokens -= 2;   // decrement for ViewName & File Size

  if (NumTokens != 0)
    maxstrcpy(StoredName,getnexttoken(),sizeof(StoredName));
  else
    strcpy(StoredName,ViewName);

  buildstr(FullViewPath,Status.CurConf.AttachLoc,ViewName,NULL);
  buildstr(FullStoredPath,Status.CurConf.AttachLoc,StoredName,NULL);

  freshline();
  switch (Func) {
    case FLAGFILE: flagnamedfile(Status.CurConf.AttachLoc,StoredName,ViewName);
                   break;
    case VIEWFILE: if (rename(FullStoredPath,FullViewPath) == -1) {
                     strcpy(Status.DisplayText,ViewName);
                     displaypcbtext(TXT_ERRORVIEWINGFILE,NEWLINE|LFBEFORE|LOGIT|BELL);
                   } else {
                     performfileview(FullViewPath,ViewName);  //lint !e534
                     rename(FullViewPath,FullStoredPath);     //lint !e534
                     Status.LastView[0] = 0;
                   }
                   break;
  }
}


static readmsgtype _NEAR_ LIBENTRY readmessage(int *CurPos, newindextype *Index) {
  int           NumTokens;
  savetype      RetVal;
  msgfunctype   Func;
  char          *p;
  char          Str[81];

  checkstack();
  Status.MsgRead = TRUE;
  if (Read.MsgStatus.Type == GPWD && Status.CurSecLevel < PcbData.SysopSec[SEC_READALLMAIL]) {
    if (Read.Capture || Read.QWK)  // don't try to get a password if we are
      return(READNEXT);            // capturing the messages for download
    displaymsgheader();
    Str[0] = 0;
    if (! passwordokay(MsgBase.Header.Password,Str,TXT_PASSWORDTOREADMSG,TXT_WRONGPWRDENTERED,2))
      return(READNEXT);
  } else {
    if (Read.QWK) {
      write_index_value();                /* update qwk index */
      writemsgheader();                    /* write qwk header */
    } else
      displaymsgheader();
  }

  if (Read.QWK) {
    writemsgbody(&QWK->File);              /* write qwk body  */
  } else {
    Display.ClearScrnWhenFull = (bool) (Read.PossibleNoScroll && (! Status.Capture));
    displaymsgbody();

    if ((MsgBase.Header.ExtendedStatus & HDR_RCPT) != 0 && (p = getextendedheader(EXTHDR_ACKRR,MsgBase.Body)) != NULL) {
      maxstrcpy(Str,p,EXTDESCLEN);
      if (tokenizestr(Str) == 9) {
        getnexttoken();      //lint !e534   ignore token
        getnexttoken();      //lint !e534   ignore token
        getnexttoken();      //lint !e534   ignore token
        getnexttoken();      //lint !e534   ignore token

        displaypcbtext(TXT_YOURMSGNUM,DEFAULTS);
        println(getnexttoken());

        if ((MsgBase.Header.ExtendedStatus & HDR_FROM) != 0 && (p = getextendedheader(EXTHDR_FROM,MsgBase.Body)) != NULL) {
          displaypcbtext(TXT_ADDRESSEDTO,DEFAULTS);
          p[25] = 0;
          println(p);
        }

        getnexttoken();      //lint !e534   ignore token
        getnexttoken();      //lint !e534   ignore token
        displaypcbtext(TXT_RECEIVEDON,DEFAULTS);
        print(countrydate(getnexttoken()));
        print("  ");
        print(getnexttoken());
      }
    }
  }

  if (Read.Capture) {
    newline();
    println("<<<>>>");
    if (Display.CountLines)         // did someone use @PON@ in a message?
      startdisplay(FORCENONSTOP);   // if so, resume non-stop display
  }

  Status.MsgRead = FALSE;

  if (Read.MsgNumber > MsgReadPtr[Status.Conference]) {
    // if the message is NOT from you, then count count it
    if (! Read.Access.Msg.FromYou) {
      UsersData.MsgsRead++;
      // Also, count the message for accounting purposes
      Read.ActMsgsRead++;
    }

    // if we're updating LMRs, then move the LMR forward.
    if (Read.UpdatePtrs)
      MsgReadPtr[Status.Conference] = Read.MsgNumber;
  }

  // if message is TO you, and we're updating message status
  if (Read.Access.Msg.ToYou && Read.UpdateMsgStatus) {
    // if a return receipt is being requested
    if ((MsgBase.Header.ExtendedStatus & HDR_RCPT) != 0 && getextendedheader(EXTHDR_REQRR,MsgBase.Body) != NULL) {
      // if this is either NOT an @LIST@ messsage, or, if it is @LIST@, if it
      // has NOT been read by you, then generate the return receipt
      if (! Read.Access.Msg.List || Read.CarbonPtr->Status != EXTSTAT_READ) {
        displaypcbtext(TXT_RETRECEIPTREQ,LFBEFORE);
        displaypcbtext(TXT_GENERATERECEIPT,LFBEFORE);
        createreceipt(&MsgBase);
        // if it is NOT an @LIST@ message, then lets get rid of the return
        // receipt request right now so that we don't generated another one
        if (! Read.Access.Msg.List) {
          MsgBase.Header.ExtendedStatus &= ~HDR_RCPT;
          updatemessageheader(Index,&MsgBase.Header,&MsgBase.Stats);
        }
        openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
      }
    }

    if (Read.Access.Msg.List)
      Read.MsgStatus.Read = (Read.CarbonPtr->Status == EXTSTAT_READ);

    if (! Read.MsgStatus.Read) {
      // if the message is not a generic message and it is addressed to you and
      // it has not yet been read - then set the 'read flag' and update the msg
      if (MsgBase.Header.ToField[0] != '@') {
        Read.MsgStatus.Read = TRUE;
        #pragma warn -stv
        setreplydate(&MsgBase.Header,Read.MsgStatus,FALSE);
        #pragma warn +stv
        updatemessageheader(Index,&MsgBase.Header,&MsgBase.Stats);
      }

      // if it is a carboned message then update the extended header
      if (Read.Access.Msg.List) {
        datestr(Str);
        Read.CarbonPtr->DateRead[0] = Str[0];
        Read.CarbonPtr->DateRead[1] = Str[1];
        Read.CarbonPtr->DateRead[2] = Str[3];
        Read.CarbonPtr->DateRead[3] = Str[4];
        Read.CarbonPtr->DateRead[4] = Str[6];
        Read.CarbonPtr->DateRead[5] = Str[7];
        timestr2(Str);
        Read.CarbonPtr->TimeRead[0] = Str[0];
        Read.CarbonPtr->TimeRead[1] = Str[1];
        Read.CarbonPtr->TimeRead[2] = Str[3];
        Read.CarbonPtr->TimeRead[3] = Str[4];
        Read.CarbonPtr->Status = EXTSTAT_READ;
        updatemessagebody(Index->Offset,MsgBase.Body,MsgBase.Header.NumBlocks);  //lint !e534
      }
    }
  }

  if (! Display.AbortPrintout && ! Read.QWK) {
    if (Read.Access.Msg.List)
      showcarbonnames();

    if (Status.CurSecLevel >= PcbData.SysopSec[SEC_KEEPMSG] && (p = getextendedheader(EXTHDR_PACKOUT,MsgBase.Body)) != NULL) {
      memcpy(Status.DisplayText,p,8);
      Status.DisplayText[8] = 0;
      countrydate(Status.DisplayText);
      displaypcbtext(TXT_DATETOPACKOUT,LFBEFORE);
    }
  }

  // we want a blank line in between the last line of the message and the
  // End of Message Command prompt ... but if the blank line would cause a
  // "more?" prompt to be issued then forget it, skip the blank line!
  if (Display.NumLinesPrinted+1 <= Display.PageLen)
    newline();

  // Did they just interrupt one message?  If so, re-enable display.
  // Otherwise, if the display was non-stop, then allow the display to
  // abort completely.
  if (Display.CountLines && Display.AbortPrintout)
    Display.AbortPrintout = FALSE;

  showattachment();

  if (Display.CountLines) {
getinput:
    Display.ClearScrnWhenFull = FALSE;

    if (Display.NumLinesPrinted == Display.PageLen) {
      Display.NumLinesPrinted = 0;
      newline();
    } else if (Display.NumLinesPrinted != 0)
      newline();
    else
      freshline();

    Str[0] = 0;
    sprintf(Status.DisplayText,"%ld-%ld",MsgBase.Stats.LowMsgNum,MsgBase.Stats.HighMsgNum);
    inputfield(Str,(UsersData.ExpertMode ? TXT_ENDOFMSGEXPERT : TXT_ENDOFMESSAGE),80,UPCASE|STACKED,HLP_ENDR,mask_msgread);
    NumTokens = tokenize(Str);
    if (NumTokens) {
      if (Str[0] == 'N' && Str[1] == 0) {
        newline();
        Display.AbortPrintout = TRUE;
        return(QUITREAD);
      }
      Func = interpretcommand(NumTokens,INSIDEREADLOOP);
      #pragma warn -stv
      switch(Func) {
        case JUMPOUT  : return(SKIPNEXT);
        case SKIPMSG  : return(SKIPEND);
        case DESELCONF: selectconference(FALSE);
                        return(READNEXT);
        case SELCONF  : selectconference(TRUE);
                        return(READNEXT);
        case JOIN     : Read.StartConf = Status.Conference;
                        //lint !e616   fall thru
        case STOP     : freshline();
                        Display.AbortPrintout = TRUE;
                        return(QUITREAD);
        case NOFUNC   : return(QUITLOOP);
        case GOODBYE  : freshline();
                        closemessagebase(&MsgBase);
                        byecommand();
                        Display.AbortPrintout = TRUE;
                        return(QUITREAD);
        case REDISPLAY: return(REREAD);
        case REPOTHER :
        case REPLY    : newline();
                        if (seclevelokay("REPLY",PcbData.UserLevels[SEC_E])) {
                          RetVal = enterreply(&MsgBase,Read.MsgStatus,Read.Access,(bool) (Func == REPOTHER));
                          displaycmdfile("R");
                          if (RetVal == ABORTMSG)
                            return(REREAD);
                          #pragma warn -stv
                          setreplydate(&MsgBase.Header,Read.MsgStatus,TRUE);
                          #pragma warn +stv
                          updatemessageheader(Index,&MsgBase.Header,&MsgBase.Stats);
                          openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
                          switch (RetVal) {
                            case SAVEMSG : return(REREAD);
                            case KILLMSG : kill(Read.MsgNumber,FALSE,FALSE,FALSE);  //lint !e534
                                           *CurPos = -1; /* force a re-read of the index */
                            case NEXTMSG : break;
                          }
                        }
                        break;
        case FINDFROM :
        case FINDTO   : newline();
                        if (seclevelokay("F",PcbData.SysopSec[SEC_7])) {
                          Str[0] = 255; /* create a dummy command string */
                          Str[1] = ' ';
                          memcpy(&Str[2],(Func == FINDTO ? MsgBase.Header.ToField : MsgBase.Header.FromField),sizeof(MsgBase.Header.FromField));
                          Str[2+sizeof(MsgBase.Header.FromField)] = 0;
                          NumTokens = tokenize(Str);
                          usermaint(NumTokens);
                        }
                        return(REREAD);
        case EDITHDR  : newline();
                        if (seclevelokay("E",PcbData.UserLevels[SEC_E])) {
                          if (Status.CurSecLevel >= PcbData.SysopSec[SEC_EDITALL] || Read.Access.Msg.FromYou)
                            *CurPos = editheader(*CurPos,Index);
                          else
                            displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE);
                        }
                        return(REREAD);
        case ENTERMSG : dispatch("E",PcbData.UserLevels[SEC_E],1,entermessage);
                        displaycmdfile("R");
                        return(QUITLOOP);
        case EDITMSG  : newline();
                        if (seclevelokay("EDIT",PcbData.UserLevels[SEC_EDIT])) {
                          if (Status.CurSecLevel >= PcbData.SysopSec[SEC_EDITALL] || Read.Access.Msg.FromYou) {
                            switch (editmessage(&MsgBase,Read.MsgNumber,Index->Offset)) {
                              case ABORTMSG:
                              case SAVEMSG :
                                   return(REREAD);
                              case KILLMSG :
                                   // KILLMSG indicates that the edited message
                                   // has GROWN in size and the original needs
                                   // to be removed.

                                   // now get rid of the original
                                   kill(Read.MsgNumber,TRUE,TRUE,FALSE);  //lint !e534

                                   // force a re-read of the index
                                   *CurPos = -1;
                                   break;
                              case NEXTMSG :
                                   break;
                            }
                            dosflush(&MsgBase.Msgs);
                          } else
                            seclevelokay("EDIT",256); /* force security message */ //lint !e534
                        }
                        break;
        case KILL     : newline();
                        if (seclevelokay("K",PcbData.UserLevels[SEC_K])) {
                          kill(Read.MsgNumber,FALSE,FALSE,FALSE);    //lint !e534
                          *CurPos = -1;  /* force a re-read of the index */
                        }
                        break;
        case UNPROT   : newline();
                        if (seclevelokay("U",PcbData.SysopSec[SEC_PROTECT]))
                          unprotect(Index);
                        return(REREAD);
        case XPORT    : if (seclevelokay("X",PcbData.SysopSec[SEC_READALLMAIL]))
                          xport();
                        return(REREAD);
        case PROT     : newline();
                        if (seclevelokay("P",PcbData.SysopSec[SEC_PROTECT]))
                          protect(Index);
                        return(REREAD);
#ifdef PCB152
        case FORWARD  : newline();

                        // check to see if they at least have enough security
                        // to enter a message
                        if (Status.CurSecLevel < PcbData.UserLevels[SEC_E] || Status.CurSecLevel < Status.CurConf.ReqLevelToEnter) {
                          // insufficient security, show an error message
                          seclevelokay("FORWARD",PcbData.UserLevels[SEC_E]);  //lint !e534
                          break;
                        }

                        // check to see if the conference is read-only
                        if (Status.CurConf.ReadOnly) {
                          displaypcbtext(TXT_CONFISREADONLY,NEWLINE|LFBEFORE|BELL);
                          break;
                        }

                        // they have enough security to enter a message...
                        if (Read.Access.Msg.ToYou && ! Read.Access.Msg.Generic) {
                          // if so, then no special processing, everyone can
                          // forward their own *personal* mail
                        } else {
                          // now, since this is NOT a personal message, then
                          // do they have the ability to MOVE messages?
                          if (Status.CurSecLevel < PcbData.SysopSec[SEC_MOVEMSG]) {
                            // insufficient security, show an error message
                            seclevelokay("FORWARD",PcbData.SysopSec[SEC_MOVEMSG]);   //lint !e534
                            break;
                          }
                        }

                        // don't offer the ability to move the message
                        // unless the caller has sufficient security, but
                        // also, don't give an error message if he doesn't
                        // have sufficient security, so manually check the
                        // level instead of using seclevelokay()
                        if (Status.CurSecLevel >= PcbData.SysopSec[SEC_MOVEMSG]) {
                          if (MoveConfNum == 0xFFFF) {
                            Str[0] = 0;
                            inputfield(Str,TXT_MOVEMESSAGETOCONF,5,NEWLINE|FIELDLEN|GUIDE|LFBEFORE,HLP_ENDR,mask_numbers);
                            if (Str[0] == 0)
                              break;
                            else {
                              MoveConfNum = (unsigned short) atoi(Str);
                              if (MoveConfNum > PcbData.NumConf)
                                MoveConfNum = Status.Conference;
                            }
                          }
                        } else {
                          // caller didn't have security to move the message
                          // to another conference, so only allow it to be
                          // forwarded in this conference
                          MoveConfNum = Status.Conference;
                        }
                        forwardmessage(MoveConfNum,&MsgBase);
                        *CurPos = -1;  /* force a re-read of the index */
                        return(REREAD);
#endif
        case MOVE     : newline();
                        if (seclevelokay("MOVE",PcbData.SysopSec[SEC_MOVEMSG])) {
                          if (MoveConfNum == 0xFFFF) {
                            Str[0] = 0;
                            inputfield(Str,TXT_MOVEMESSAGETOCONF,5,NEWLINE|LFBEFORE,HLP_ENDR,mask_numbers);
                            if (Str[0] == 0)
                              break;
                            MoveConfNum = (unsigned short) atoi(Str);
                            if (MoveConfNum > PcbData.NumConf)
                              break;
                          }
                          if (savemessage(MoveConfNum,&MsgBase.Header,MsgBase.Body,FALSE) != -1)
                            kill(Read.MsgNumber,(bool) (MoveConfNum == Status.Conference),TRUE,FALSE);  //lint !e534
                          *CurPos = -1;  /* force a re-read of the index */
                        }
                        break;
        case CHAT     : freshline();
                        if (PcbData.Network) {
                          dispatch("CHAT",PcbData.UserLevels[SEC_CHAT],NumTokens,chatcommand);
                          startdisplay(FORCECOUNTLINES);
                          moreprompt(PRESSENTER);
                          displaycmdfile("R");
                        } else
                          displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                        return(REREAD);
        case WHOCMD   : freshline();
                        if (PcbData.Network) {
                          dispatch("WHO",PcbData.UserLevels[SEC_WHO],NumTokens,displayusernet);
                          moreprompt(PRESSENTER);
                          displaycmdfile("R");
                        } else
                          displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                        return(REREAD);
        case COPY     : newline();
                        if (seclevelokay("COPY",PcbData.SysopSec[SEC_MOVEMSG])) {
                          if (MoveConfNum == 0xFFFF) {
                            Str[0] = 0;
                            inputfield(Str,TXT_COPYMESSAGETOCONF,5,NEWLINE|LFBEFORE,HLP_ENDR,mask_numbers);
                            if (Str[0] == 0)
                              break;
                            MoveConfNum = (unsigned short) atoi(Str);
                            if (MoveConfNum > PcbData.NumConf)
                              break;
                          }
                          savemessage(MoveConfNum,&MsgBase.Header,MsgBase.Body,TRUE);  //lint !e534
                          return(REREAD);
                        }
                        break;
        case FLAGFILE :
        case VIEWFILE : flagorview(Func,NumTokens);
                        goto getinput;
      }
      #pragma warn +stv
    }
  }
  return(READNEXT);
}


/********************************************************************
*
*  Function:  readconference()
*
*  Desc    :  This is the main "engine" for message reading, message scanning
*             and header scanning (functions 'R', 'Q' and '5').
*/

static int _NEAR_ LIBENTRY readconference(void) {
  bool          MaxConfMsgsReached;
  bool          MaxMsgsReached;
  unsigned      MaxMsgs;
  unsigned      MaxMsgsPerConf;
  unsigned      Bytes;
  int           CurPos;
  int           NewPos;
  int           RetVal;
  long          X;
  long          First;
  long          Last;
  long          LastMsgRead;
  newindextype  *IdxPtr;
  msgseqtype    Num;
  char          Str[80];
  newindextype  IdxBuffer[IDXBUFSIZE];

  checkstack();
  MaxConfMsgsReached = MaxMsgsReached = FALSE;

  MaxMsgs = PcbData.MaxTotalMsgs;
  if (QwkSupport && UsersData.QwkConfig.MaxMsgs != 0) {
    if (UsersData.QwkConfig.MaxMsgs < MaxMsgs)
      MaxMsgs = UsersData.QwkConfig.MaxMsgs;
  }

  MaxMsgsPerConf = PcbData.MaxConfMsgs;
  if (QwkSupport && UsersData.QwkConfig.MaxMsgsPerConf != 0) {
    if (UsersData.QwkConfig.MaxMsgsPerConf < MaxMsgsPerConf)
      MaxMsgsPerConf = UsersData.QwkConfig.MaxMsgsPerConf;
  }

  RetVal = 0;
  checkstatus();
  if (Display.AbortPrintout)
    return(-1);

  /* if we're reading messages then  reset the line counter now so that if  */
  /* the conference contains no messages to read it won't cause the display */
  /* to pause - (when scanning, however, we want it to pause)               */
  if (! Status.QuickScan)
    Display.NumLinesPrinted = 0;
  else {
    displaypcbtext(Status.HeaderScan ? TXT_FIVESCANHEADER : TXT_QUICKSCANHEADER,LFBEFORE);
    printcolor(PCB_CYAN);
  }

  /* fix the last message read pointer if it's gone too high or too low */
  LastMsgRead = fixlmrpointer(Status.Conference,MsgBase.Stats.LowMsgNum,MsgBase.Stats.HighMsgNum);

  if (Read.Since) {
    Num.First     = LastMsgRead + 1;
    Num.Last      = max(MsgBase.Stats.HighMsgNum,Num.First);
    Read.Forward  = TRUE;
    Read.Backward = FALSE;
  } else if (Read.FullRange) {
    Num.First     = MsgBase.Stats.LowMsgNum;
    Num.Last      = MsgBase.Stats.HighMsgNum;
    Read.Forward  = TRUE;
    Read.Backward = FALSE;
  } else {
    Num = Read.Numbers[Read.CurGroup];
    Read.Forward  = (bool) (Num.First < Num.Last);
    Read.Backward = (bool) (Num.First > Num.Last);
  }

  if (Read.Forward) {
    if (dossetbuf(&MsgBase.Msgs,4096) == -1)
      goto exit;
  } else if (Read.Backward) {
    if (dossetbuf(&MsgBase.Msgs,1024) == -1)
      goto exit;
  }

  setbit(&ConfFlags[SCANNED],Status.Conference);
  if (MsgBase.BodySize == 0) {
    if ((MsgBase.Body = (char *) checkmalloc(10*128,"MSG READ")) == NULL) {
      MsgBase.BodySize = 0;
      goto exit;
    }
    MsgBase.BodySize = 10;
  }

  if (Num.First > MsgBase.Stats.HighMsgNum) {
    if (Read.Backward) Num.First = MsgBase.Stats.HighMsgNum; else goto exit;
  }
  if (Num.First < MsgBase.Stats.LowMsgNum) {
    if (Read.Forward) Num.First = MsgBase.Stats.LowMsgNum; else goto exit;
  }
  if (Num.Last  > MsgBase.Stats.HighMsgNum) {
    if (Read.Forward) Num.Last = MsgBase.Stats.HighMsgNum; else goto exit;
  }
  if (Num.Last  < MsgBase.Stats.LowMsgNum) {
    if (Read.Backward) Num.Last = MsgBase.Stats.LowMsgNum; else goto exit;
  }

  First  = (Num.First - MsgBase.Stats.LowMsgNum);
  Last   = (Num.Last  - MsgBase.Stats.LowMsgNum);
  X      = First;
  CurPos = -1;

  showactivity(ACTBEGIN);

  while (TRUE) {
    if (Display.AbortPrintout)
      goto exit;

    showactivity(ACTSHOW);

    NewPos = (int) ((X*sizeof(newindextype)) / sizeof(IdxBuffer));
    if (NewPos != CurPos) {
      IdxPtr = &IdxBuffer[(int) (X % IDXBUFSIZE)];
      CurPos = NewPos;
      doslseek(MsgBase.Index,(long)CurPos*sizeof(IdxBuffer),SEEK_SET);
      if (readcheck(MsgBase.Index,IdxBuffer,sizeof(IdxBuffer)) == (unsigned) -1)
        break;
    }

    if (IdxPtr->Offset <= 0) {  //lint !e644  IdxPtr is initialized when NewPos != CurPos, which happens at least the first time!
      if (IdxPtr->Offset == 0)
        goto next;
      if (Status.HeaderScan)
        IdxPtr->Offset = -IdxPtr->Offset;
      else
        goto next;  /* if not header scan then skip to the next message */
    }

evaluateaccess:
//  if (! Status.HeaderScan) {
      Read.MsgStatus = getmsgstatus(IdxPtr->Status);
      #pragma warn -stv
      Read.Access    = readstatus(IdxPtr,Read.MsgStatus);
      #pragma warn +stv
      if (! Read.Access.OkayToRead)
        goto next;
      if (! (Read.AnyMsgs ||
             (Read.YourMsgs && Read.Access.Msg.ToYou) ||
             (Read.FromMsgs && Read.Access.Msg.FromYou) ||
             (Read.MsgsToAll && memcmp(IdxPtr->To,"ALL                      ",25) == 0)))
        goto next;
      if (Read.UnReadOnly && Read.MsgStatus.Read)
        goto next;
//  }

    if (IdxPtr->Date < Read.NewDate)
      goto next;

    if (Read.UserSrch) {
      if ((Read.UserSrch & FIND_TO) && memcmp(IdxPtr->To,Read.UserNameTo,25) != 0)
        goto next;
      if ((Read.UserSrch & FIND_FROM) && memcmp(IdxPtr->From,Read.UserNameFrom,25) != 0)
        goto next;
      if ((Read.UserSrch == FIND_USER) && memcmp(IdxPtr->To,Read.UserNameTo,25) != 0 &&
                                          memcmp(IdxPtr->From,Read.UserNameTo,25) != 0)
        goto next;
    }

    dosfseek(&MsgBase.Msgs,IdxPtr->Offset,SEEK_SET);
    if (dosfread(&MsgBase.Header,sizeof(msgheadertype),&MsgBase.Msgs) != sizeof(msgheadertype))
      goto exit;

    /* check for a valid header on the message and if invalid skip to next */

    if (MsgBase.Header.ToField[0] < ' ' ||
        MsgBase.Header.FromField[0] < ' ' ||
        MsgBase.Header.NumBlocks < 2)
      goto next;

    // In case the NUM value in the header and index do not agree, write a
    // message to the caller's log and then exit the read command.

    if (bassngltolong(MsgBase.Header.MsgNumber) != IdxPtr->Num) {
      newline();
      printcolor(PCB_RED);
      sprintf(Str,"Corrupted IDX File (%u = %ld : %ld)",Status.Conference,bassngltolong(MsgBase.Header.MsgNumber),IdxPtr->Num);
      println(Str);
      writelog(Str,SPACERIGHT);
      goto exit;
    }

    Read.MsgNumber = IdxPtr->Num;

    // In case the STATUS byte in the header has been updated by OTHER software
    // leaving the index status byte unchanged, update it now

    if (MsgBase.Header.Status != IdxPtr->Status) {
      updateindex(IdxPtr,&MsgBase.Header,&MsgBase.Stats);
      goto evaluateaccess;
    }

    // In case a message has been killed by OTHER software leaving the index
    // unchanged, update it now and then skip the message

    if (MsgBase.Header.ActiveFlag != MSG_ACTIVE) {
      if (! (Status.HeaderScan && MsgBase.Header.ActiveFlag == MSG_INACTIVE)) {
        updateindex(IdxPtr,&MsgBase.Header,&MsgBase.Stats);
        goto next;
      }
    }

    if (Read.Net) {
      // if the message is TO YOU, or if it is flagged to be ECHOED and it is
      // NOT a private message, then everything is okay, otherwise, skip over
      // this message
      if (Read.Access.Msg.ToYou || (MsgBase.Header.EchoFlag == 'E' && Read.MsgStatus.Type == PBLC)) {
      } else
        goto next;   // it either wasn't a public echoed message, or it wasn't to you
    }

    if (Read.Threading) {
      int Offset;
      Offset = (memicmp(MsgBase.Header.SubjField,"Re: ",4) == 0 ? 4 : 0);
      if (memicmp(Status.SearchText,MsgBase.Header.SubjField+Offset,25-Offset) != 0)
        goto next;
    }

    #ifndef __OS2__
      /* if we've read a BIG message and this message isn't nearly as big */
      /* then let's cause the message buffer to shrink and be reallocated */
      if (MsgBase.BodySize > 40 && MsgBase.Header.NumBlocks < 30)
        MsgBase.BodySize = 1;
    #endif

    /* if our message read buffer isn't big enough to hold the next message */
    /* then let's de-allocate our current buffer and get a new one that is  */
    if (MsgBase.BodySize < MsgBase.Header.NumBlocks-1) {
      if (MsgBase.BodySize != 0)
        bfree(MsgBase.Body);
      MsgBase.BodySize = MsgBase.Header.NumBlocks - 1; /* subtract 1 to get Body Length */

      Bytes = (unsigned) MsgBase.BodySize * 128U;
      if ((MsgBase.Body = (char *) checkmalloc(Bytes,"MSG READ")) == NULL) {
        MsgBase.BodySize = 0;
        goto next;
      }
    }

    /* Now read the body - subtract 1 from NumBlocks to get Body Length */
    if (dosfread(MsgBase.Body,(MsgBase.Header.NumBlocks-1)*128,&MsgBase.Msgs) != (MsgBase.Header.NumBlocks-1)*128)
      goto next;

    if (Read.NumSearches > 0  /* search header and body for text */
        && ! parsersearch(MsgBase.Header.ToField,TOFIELDTHRUSUBJFIELD,Status.SearchInput,FALSE,0)
        && ! parsersearch(MsgBase.Body,(MsgBase.Header.NumBlocks-1)*128,Status.SearchInput,FALSE,0)) // subtract 1 from NumBlocks to get Body length
      goto next;

    // a carbon list (@LIST@) message has been found - check to see if the
    // message is TRULY addressed to the caller, it might NOT be even though
    // the Read.Access.Msg.ToYou says it is.

    if (Read.Access.Msg.List && (Read.CarbonPtr = (msgcarbontype *) carbonedtoyou(MsgBase.Body)) == NULL) {
      // this message is not really addressed to the caller, but we should
      // let the sysop (or the sender) read it if he so desires.  Just turn off
      // the ToYou variable so that it will show up as a generic message.
      Read.Access.Msg.ToYou = FALSE;

      // Turning off the ToYou flag affects the OkayToRead flag.
      //
      // This message isn't addressed to the caller, so unless the message is
      // cleared for the caller to read, let's skip over it.
      if (! (Read.Access.Msg.Okay || Read.Access.Msg.FromYou))
        goto next;

      // Also, since the message is no longer TO YOU, we need to once again
      // verify that, if reading mail TO YOU, this message does not show up
      if (Read.YourMsgs)
        goto next;
    }

    if (Read.Capture || Read.QWK) {
      if (Read.Found >= MaxMsgsPerConf) {
        MaxConfMsgsReached = TRUE;
        goto exit;
      }
      if (Read.FoundTotal >= MaxMsgs) {
        MaxMsgsReached = TRUE;
        RetVal = -1;
        goto exit;
      }
      if (Read.FoundTotal >= 1 && Read.CaptureSingle)
        goto exit;
      Read.FoundTotal++;
    } else
      showactivity(ACTSUSPEND);

    Read.Found++;

    if (Status.QuickScan)
      quickscan();
    else if (Status.ReplyCommand) {
      if (replycommand(&CurPos) == -1) {
        RetVal = -1;
        showactivity(ACTEND);
        freshline();
        return(RetVal);
      }
    } else {
      switch (readmessage(&CurPos,IdxPtr)) {
        case REREAD  :  dosflush(&MsgBase.Msgs); continue;
        case QUITREAD:  RetVal = -1;
                        //lint !e616  fall thru
        case QUITLOOP:  showactivity(ACTEND);
                        if (Read.Found != 0)
                          goto alldone;
                        return(RetVal);
        case READNEXT:  break;
        case SKIPEND :  skiptolastmsg();  /* fall thru */
        case SKIPNEXT:  RetVal = 0;
                        if (Read.Found != 0)
                          goto alldone;
                        return(RetVal);
      }
    }

    if (! Status.Capture)
      showactivity(ACTRESUME);

    if (!Read.Forward && !Read.Backward)
      goto exit;

next:
    // go to the next message in the display sequence
    if (Read.Forward) {
      if (++X > Last) {
        if (Read.KeepGoing) {
          if (! PcbData.Network) {
            /* if SHARE isn't loaded the close the message base and re-open */
            /* it so that DOS can properly inform us as to whether or not   */
            /* there are more messages to read                              */
            closemessagebase(&MsgBase);
            if (openmessagebase(Status.Conference,&Status.CurConf,&MsgBase,READONLY) == -1)
              goto exit;
          }
          openedmsgbasestreamstats(&MsgBase.Msgs,&MsgBase.Stats);
          Last = (unsigned) (MsgBase.Stats.HighMsgNum - MsgBase.Stats.LowMsgNum);
          if (X > Last) {
            unsetbit(&ConfReg[MFL],Status.Conference);
            break;
          }
          CurPos = -1; /* we'll need to re-read the index */
        } else break;
      }
      IdxPtr++;
    } else if (Read.Backward) {
      if (--X == -1)
        break;
      IdxPtr--;
    } else
      break;
  }

exit:
  showactivity(ACTEND);

  if (Read.Capture || Read.QWK) {
    if (Read.Capture)
      setcapture(FALSE);

    Display.ShowOnScreen = TRUE;
    if (Display.AbortPrintout) {
      if (Read.Capture) {
        closecapturefile();
        unlink(Status.CapFileName);
        Read.Capture = FALSE;
      } else {
        finishqwkpacket(FALSE);   //lint !e534
        Read.QWK = FALSE;
      }
      checkdisplaystatus();
      newline();
      restorepointers();
      RetVal = -1;
    } else {
      if (Read.Found == 0) {
        backupcleareol(awherex());
      } else {
        printcolor(PCB_YELLOW);
        sprintf(Str,"-> %u",Read.Found);
        print(Str);
        if (MaxMsgsReached) {
          print("  ");
          displaypcbtext(TXT_MAXMSGS,LFBEFORE);
        } else if (MaxConfMsgsReached) {
          print("  ");
          displaypcbtext(TXT_MAXMSGSPERCONF,LFBEFORE);
        }
      }
      Display.ShowOnScreen = FALSE;
      setcapture(TRUE);
    }
  }

alldone:
  showactivity(ACTEND);
  freshline();
  if (Read.Found == 0)
    displaypcbtext(TXT_NOMAILFOUND,NEWLINE);
  else {
    #ifdef PCB152
    {
      double Rate;

      if (Status.ActStatus != ACT_DISABLED) {
        Rate = Status.CurConf.ChargeMsgRead;
        if (Read.Capture || Read.QWK) {
          Rate += AccountRates.ChargeForMsgReadCapture;
          MsgReadCharge += recordusage("MSG READ CAP","",Rate,Read.ActMsgsRead,&UsersData.Account.DebitMsgReadCapture);
          checkaccountbalance();
        } else {
          Rate += AccountRates.ChargeForMsgRead;
          MsgReadCharge += recordusage("MSG READ","",Rate,Read.ActMsgsRead,&UsersData.Account.DebitMsgRead);
          checkaccountbalance();
        }
      }
    }
    #endif
  }
  return(RetVal);
}


static int _NEAR_ LIBENTRY opennewmessagebase(void) {
  checkstack();
  closemessagebase(&MsgBase);
  if (openmessagebase(Status.Conference,&Status.CurConf,&MsgBase,READONLY) == -1)
    return(-1);
  makeconfstr(Read.ConfStr,Status.Conference);
  makeconfreadstr(Read.ConfUpStr,Status.Conference);
  Read.ConfUpStrLen = (char) strlen(Read.ConfUpStr);
  return(0);
}


static int _NEAR_ LIBENTRY newconference(void) {
  bool           Abort;
  unsigned short TempConf;

  checkstack();
  checkstatus();
  if (Display.AbortPrintout)
    return(-1);

  // if we're in Net Status mode, then skip any conferences which are
  // not set up with Net Status
  if (Read.Net && ! isset(&ConfReg[NET],Read.Conf))
    return(-1);

  if (Status.Conference != MsgBase.ConfNum) {
    updatemsgindex();
    if (opennewmessagebase() == -1)
      return(-1);

    if (Read.QWK)
      if (openqwkindex() == -1)
        return(-1);

    if (Read.Capture || Read.QWK)
      Display.ShowOnScreen = TRUE;       // turn display back on

    freshline();                         // only LF if something was on screen
    displaypcbtext(TXT_SCANNING,DEFAULTS);
    print(Read.ConfStr);

    // if we got this far, and Read.Net is set, then this must be a
    // Net Status conference, so display that status to the caller
    if (Read.Net)
      displaypcbtext(TXT_NETSTATUS,DEFAULTS);

    if (Read.Capture || Read.QWK)
      Display.ShowOnScreen = FALSE;

    if (Read.NumSearches > 0) {
      maxstrcpy(Status.DisplayText,Status.SearchText,sizeof(Status.DisplayText));
      displaypcbtext(TXT_SEARCHINGFOR,LFBEFORE);
    }

    if (Status.Conference != Read.StartConf) {
      TempConf = Status.Conference;          /* Remember where we are */
      Status.Conference = Read.StartConf;    /* then pretend we just came from where we started */
      Read.StartConf = TempConf;             /* and set our starting point to this conference */
      checkstatus();
      Abort = Display.AbortPrintout;
      showconfjoined(TempConf,&Status.CurConf,FALSE,(bool) (Status.CurConf.ShowIntroOnRA && ! (Read.Capture || Read.QWK)));
      loadcmds();
      if (Abort || Display.AbortPrintout || Display.WasAborted) {
        Display.AbortPrintout = TRUE;
        return(-1);
      }
    }
  } else if (! PcbData.Network) {
    /* if not a network then SHARE might not be loaded so let's close and */
    /* reopen the message base so that we can flush the buffers and get   */
    /* any new messages that may have been written to disk                */
    if (opennewmessagebase() == -1)
      return(-1);
  }
  return(0);
}


static int _NEAR_ LIBENTRY nextconference(int Abort) {
  checkstack();
  if (Display.AbortPrintout || Abort == -1)
    return(-1);

  Status.StartConf = ++Read.Conf;       /* increment the local Conf pointer */
  if (Read.Conf > PcbData.NumConf) {
    Status.StartConf = 0;
    return(-1);
  }
  return(0);
}


static bool _NEAR_ LIBENTRY okaytoreadthisconference(void) {
  checkstack();
  /* test for mail waiting scan */
  if (Read.MailWaitConf && ! isset(&ConfReg[MFL],Read.Conf))
    return(FALSE);

  if (Read.Conf != OriginalConference) {
    /* test for security clearance to JOIN this conference */
    if (Status.CurSecLevel < PcbData.UserLevels[SEC_J])
      return(FALSE);

    /* test for desired scan of this conference */
    if (Read.CheckUserScan && ! isset(&ConfReg[USR],Read.Conf))
      return(FALSE);

    /* test for being registered in this conference */
    if (! isregistered(ConfReg,Read.Conf))
      return(FALSE);
  }

  /* let's see if the conference message base is defined */
  getconfrecord(Read.Conf,&Status.CurConf);
  if (Status.CurConf.Name[0] == 0 || Status.CurConf.MsgFile[0] == 0) {
    getconfrecord(Status.Conference,&Status.CurConf);
    return(FALSE);
  }

  Status.Conference = Read.Conf;
  confdetails(UPDATEUSER);

  /* now that we've joined do we still have clearance to READ in here? */
  if (Status.CurSecLevel < PcbData.UserLevels[SEC_R])
    return(FALSE);

//if (Read.QWK)
//  openqwkindex();

  return(TRUE);
}


static int _NEAR_ LIBENTRY mainmessageprompt(int NumTokens) {
  checkstack();
retry:
  if (NumTokens == 0)
    if ((NumTokens = messagereadprompt()) == 0)
      return(-1);

  switch(interpretcommand(NumTokens,OUTSIDEREADLOOP)) {
    case QUICKSCAN: if (Read.NumGroups == 0 || Read.CurGroup >= Read.NumGroups) {
                      NumTokens = 0;
                      goto retry;
                    }
                    return(-1);
    case ENTERMSG : dispatch("E",PcbData.UserLevels[SEC_E],1,entermessage);
                    NumTokens = 0;
                    goto retry;
    case GOODBYE  : freshline();
                    byecommand();
                    return(-1);
    case KILL     : dispatch("K",PcbData.UserLevels[SEC_K],NumTokens,killcommand);
                    NumTokens = 0;
                    goto retry;
    case JOIN     : OriginalConference = Status.Conference;
                    /* fall thru */
    case STOP     : return(-1);
    case SKIPMSG  : skiptolastmsg();
                    NumTokens = 0;
                    goto retry;
    case DESELCONF: selectconference(FALSE);
                    return(-1);
    case SELCONF  : selectconference(TRUE);
                    return(-1);
    case CHAT     : freshline();
                    if (PcbData.Network) {
                      dispatch("CHAT",PcbData.UserLevels[SEC_CHAT],NumTokens,chatcommand);
                      startdisplay(FORCECOUNTLINES);
                      displaycmdfile("R");
                    } else
                      displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                    return(-1);
    case WHOCMD   : freshline();
                    if (PcbData.Network) {
                      dispatch("WHO",PcbData.UserLevels[SEC_WHO],NumTokens,displayusernet);
                      displaycmdfile("R");
                    } else
                      displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                    return(-1);
  }
  return(0);
}


static void _NEAR_ LIBENTRY getprompt(int Num, char *Str, int Len) {
  pcbtexttype Buf;

  checkstack();
  getpcbtext(Num,&Buf);
  maxstrcpy(Str,Buf.Str,Len);
}


static int _NEAR_ LIBENTRY countspaces(char *Str, int Left) {
  int X;

  checkstack();
  for (X = 0, Str += Left+2; *Str == ' '; Str++, X++);
  return(X-1);
}


static int _NEAR_ LIBENTRY locatecolon(char *Str, int Count) {
  int Pos;

  checkstack();
  for (Pos = 0; *Str != 0; Str++) {
    if (*Str == ':') {
      Count--;
      if (Count == 0)
        return(Pos);
    }
    Pos++;
  }
  return(0);
}


static void _NEAR_ LIBENTRY padleft(char *Str) {
  int X;
  int Distance;

  checkstack();
  X = locatecolon(Str,1);
  if ((Distance = Text->Left - X) > 0) {
    memmove(&Str[Distance],Str,strlen(Str)+1);
    memset(Str,' ',Distance);
  }
}


static void _NEAR_ LIBENTRY padright(char *Str) {
  int X;
  int Y;
  int Distance;

  checkstack();
  X = locatecolon(Str,2);
  if ((Distance = Text->Right - X) > 0) {
    Y = locatecolon(Str,1) + 1;
    memmove(&Str[Distance+Y],&Str[Y],strlen(&Str[Y])+1);
    memset(&Str[Y],' ',Distance);
  }
}



static void _NEAR_ LIBENTRY loadmsgprompts(void) {
  int Left;
  int Right;
  int X;

  checkstack();
  getprompt(TXT_MSGDATENUM       ,Text->DateNum          ,sizeof(Text->DateNum));
  getprompt(TXT_MSGTOLINE        ,Text->To               ,sizeof(Text->DateNum));
  getprompt(TXT_MSGREPLIES       ,Text->Replies          ,sizeof(Text->DateNum));
  getprompt(TXT_MSGFROM          ,Text->From             ,sizeof(Text->DateNum));
  getprompt(TXT_MSGNA            ,Text->NA               ,sizeof(Text->DateNum));
  getprompt(TXT_MSGNOTREAD       ,Text->NotRead          ,sizeof(Text->DateNum));
  getprompt(TXT_MSGRCVRONLY      ,Text->ReceiverOnly     ,sizeof(Text->DateNum));
  getprompt(TXT_MSGGROUPPWRD     ,Text->GroupPwrd        ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSENDERPWRD    ,Text->SenderPwrd       ,sizeof(Text->DateNum));
  getprompt(TXT_MSGPUBLIC        ,Text->Public           ,sizeof(Text->DateNum));
  getprompt(TXT_MSGREFERNUM      ,Text->Refer            ,sizeof(Text->DateNum));
  getprompt(TXT_MSGREAD          ,Text->Read             ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSUBJLINE      ,Text->Subject          ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSTATUS        ,Text->Status           ,sizeof(Text->DateNum));
  getprompt(TXT_MSGFILE          ,Text->File             ,sizeof(Text->DateNum));
  getprompt(TXT_MSGLIST          ,Text->List             ,sizeof(Text->DateNum));

  getprompt(TXT_NONE             ,Text->None             ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSGENERAL      ,Text->General          ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSTHREAD       ,Text->Thread           ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSTEXTSCAN     ,Text->TextScan         ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSREADFORYOU   ,Text->ForYou           ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSREADFROMYOU  ,Text->FromYou          ,sizeof(Text->DateNum));
  getprompt(TXT_MSGSREADTOORFROM ,Text->ToOrFrom         ,sizeof(Text->DateNum));
  getprompt(TXT_SEPARATOR        ,Text->Separator        ,sizeof(Text->DateNum));

  getprompt(TXT_ALLNAME          ,Text->AllName          ,sizeof(Text->DateNum));
  getprompt(TXT_FROMRETRECEIPT   ,Text->RetRecName       ,sizeof(Text->DateNum));
  getprompt(TXT_COMMENT          ,Text->Comment          ,sizeof(Text->DateNum));
  getprompt(TXT_ECHO             ,Text->Echo             ,sizeof(Text->DateNum));
  getprompt(TXT_ALLCONFIND       ,Text->AllConfIndicator ,sizeof(Text->DateNum));
  getprompt(TXT_READONLYIND      ,Text->ReadOnlyIndicator,sizeof(Text->DateNum));

  Left = locatecolon(Text->DateNum,1);
  if ((X = locatecolon(Text->General,1)) > Left) Left = X;
  if ((X = locatecolon(Text->To     ,1)) > Left) Left = X;
  if ((X = locatecolon(Text->From   ,1)) > Left) Left = X;
  if ((X = locatecolon(Text->Subject,1)) > Left) Left = X;

//Right = locatecolon(Text->DateNum,2);
//if ((X = locatecolon(Text->General ,2)) > Right) Right = X;

  Right = locatecolon(Text->General,2);
  if ((X = locatecolon(Text->Thread  ,2)) > Right) Right = X;
  if ((X = locatecolon(Text->TextScan,2)) > Right) Right = X;
  if ((X = locatecolon(Text->ForYou  ,2)) > Right) Right = X;
  if ((X = locatecolon(Text->FromYou ,2)) > Right) Right = X;
  if ((X = locatecolon(Text->ToOrFrom,2)) > Right) Right = X;

  Text->Left  = Left;
  Text->Right = Right;

  padleft(Text->DateNum);
  padleft(Text->To);
  padleft(Text->From);
  padleft(Text->Subject);
  padleft(Text->General);
  padleft(Text->Thread);
  padleft(Text->TextScan);
  padleft(Text->ForYou);
  padleft(Text->FromYou);
  padleft(Text->ToOrFrom);
  padleft(Text->Read);

//padright(Text->DateNum);
  padright(Text->General);
  padright(Text->Thread);
  padright(Text->TextScan);
  padright(Text->ForYou);
  padright(Text->FromYou);
  padright(Text->ToOrFrom);

  Text->Left  += 2;
  Text->Right += 2;

  Text->TextScanWidth  = (char) countspaces(Text->TextScan,Left);
  Text->ToOrFromWidth  = (char) countspaces(Text->ToOrFrom,Left);
  Text->ForYouWidth    = (char) countspaces(Text->ForYou  ,Left);
  Text->FromYouWidth   = (char) countspaces(Text->FromYou ,Left);
  Text->ThreadWidth    = (char) countspaces(Text->Thread  ,Left);
  Text->GeneralWidth   = (char) countspaces(Text->General ,Left);


  strupr(Text->None);
}



/********************************************************************
*
*  Function: messageread()
*
*  Desc    : sets up to read the message bases by doing the following:
*
*  1) Allocates memory for the "text displays" as well as the
*     number-list used for message reading
*
*  2) If there were no command line parameters it gets them by
*     displaying the "message read" prompt
*
*  3) It gets the PCBTEXT information in a "ready-to-use" form
*
*  4) Interprets the tokens passed to it (cmnd line or read prompt)
*
*  5) Reads using the following flow:
*
*  Record the current conference
*  Pad the name for "name matching" purposes
*  Set the MsgBase.ConfNum to -1 for "open and close" purposes
*
*  While not aborted do {
*    While not aborted do {
*      If reading ALL conferences {
*        Add 1 to the conference number
*        If the Current Conference > the Number of Conferences {
*          Reset our "starting conference" number back to -1
*          Break out of inner While Loop
*        }
*        Note that no messages have been found in this conference yet
*        If we are not registered in this conference {
*          Then loop back to the top of the inner While Loop
*        }
*        If we are not set to SCAN this conference {
*          Then loop back to the top of the inner While Loop
*        }
*        Get the current conference definitions
*        If the conf name or filename is blank {
*          Then loop back to the top of the inner While Loop
*        }
*        Find out our "security level" while in this conference
*        If we now have insufficient security to READ messages {
*          Then loop back to the top of the inner While Loop
*        }
*      } else (Note: if not scanning ALL conferences) {
*        Add 1 to the current Group Pointer
*      }
*      If the Current Group Pointer >= the Number of Groups {
*        Break out of inner While Loop
*      }
*      If the Current Confe is not equal to the MsgBase Conf Num {
*        Close previous message base (if it was open)
*        Open new message base
*        If there's an error opening the message base {
*          Loop back to the top of the inner While Loop
*        }
*        Create a "Conference String" for display purposes
*        Create a "Conference Read String" for display purposes
*        Indicate that we are scanning the conference
*      }
*      Read the messages in the "current group" in the "current conf"
*      Record our "starting conference" as this conference minus 1
*
*      If we were forced to stop OR we're not scanning ALL conf's {
*        If we're NOT in the conference we started in {
*          Reset the display status (in case it was aborted)
*          Join this conference now
*          Show that we joined the conference
*          If we were forced to stop {
*            Then exit now
*          }
*        }
*      }
*      Note: Loop back to the top of the inner While Loop
*    }
*
*    If we were in a Valid Conf AND we did not abort out {
*      Then display a new message read prompt
*      If there's nothing to do {
*         Then exit the outer While Loop
*      }
*      Interpret the commands given to us
*    }
*    Note: loop back to the top of the outer While Loop
*  }
*
*  If we had any "searches" allocated {
*    Free the memory allocated to them
*  }
*
*  Close the message base we were in
*  Free the Number Groups
*  Free the Text messages
*/

void LIBENTRY messageread(int NumTokens) {
  bool ScannedConf;
  int  RetVal;
  char Str[10];

  checkstack();
  memset(&Read,0,sizeof(Read));

  Status.ReadCapture   = FALSE;
  Status.XferCapture   = FALSE;
  Status.CapBye        = FALSE;
  Status.CapAsk        = TRUE;
  Read.StartConf       = Status.Conference;
  Read.Conf            = Status.Conference;
  OriginalConference   = Status.Conference;
  Read.UpdatePtrs      = TRUE;  /* set the default here INSTEAD of in interpretcommand() so that it sticks across Read functions */
  Read.UpdateMsgStatus = TRUE;  /* set the default here INSTEAD of in interpretcommand() so that it sticks across Read functions */
  Read.HeaderLen       = (char) (UsersData.PackedFlags.ShortHeader ? HDR_SHORT : HDR_LONG);
  Display.KeepLines    = Read.HeaderLen;
  ScannedConf          = FALSE;

  Read.PossibleNoScroll = FALSE;
  Read.MsgClear = FALSE;

  if (UsersData.PackedFlags.MsgClear) {
    Read.MsgClear = TRUE;
    if (UseAnsi && ! UsersData.PackedFlags.ScrollMsgBody)
      Read.PossibleNoScroll = TRUE;
  }

  if (Status.Poff) {                /* did someone use the @POFF@ command? */
    startdisplay(FORCECOUNTLINES);  /* if so, force it back off to avoid   */
    Status.Poff = FALSE;            /* a non-stop display of all messages  */
  }

  if ((Text = (msgtexttype *) checkmalloc(sizeof(msgtexttype),"MSG READ")) == NULL)
    goto final;

  if ((Read.Numbers = (msgseqtype *) checkmalloc(MAXGROUPS * sizeof(msgseqtype),"MSG READ")) == NULL)
    goto final;

  if (opennewmessagebase() == -1)
    goto final;


  loadmsgprompts();
  Read.PreMsg = findcmdfile("PREMSG",Read.PreMsgFile);

  if (mainmessageprompt(NumTokens) == -1)
    goto final;

  closemessagebase(&MsgBase);
  MsgBase.ConfNum = 0xFFFF;

  if (! Read.AllConf && newconference() == -1)
    goto exit;

  /* loop for overall read menu and prompts */
  while (! Display.AbortPrintout) {

    /* loop to read "groups" or read conferences in a "R A" command */
    while (! Display.AbortPrintout) {
      if (Read.AllConf) {
        Read.Found = 0;
        Read.ActMsgsRead = 0;
        if (Read.Conf != Read.StartConf || Read.MailWaitConf) {
          if (! okaytoreadthisconference()) {     /* not okay to read         */
            if (nextconference(0) == 0)           /* go to next conference    */
              continue;                           /*   success, loop back up  */

            // at this point, there are no more "next conferences" to join
            // down below, we are checking to see if we have successfully
            // SCANNED a conference *OR* if we were scanning MAIL WAIT
            // conferences *OR* if we were scanning for conferences with
            // NET status.  If any of these three conditions are true, then
            // it goes back to the read menu.  But what OTHER conditions
            // exist for which returning back to the read menu is NOT desired?
            // this should be documented as soon as it is understood!

            if (ScannedConf || Read.MailWaitConf || Read.Net) /* we've read some msgs so  */
              goto readmenu;                      /*   go back to read menu   */
            Read.Conf = 0;                        /* else start over at main  */
            continue;
          }
        }
      } else {
        Read.CurGroup++;
        if (Read.CurGroup >= Read.NumGroups)
          goto readmenu;
      }

      /* make sure we've opened the right conference message base */
      if (newconference() == -1) {
        if (Display.AbortPrintout)
          goto exit;
        if (Read.AllConf && nextconference(0) == 0)
          continue;
        goto readmenu;
      }

      RetVal = readconference();
      ScannedConf = TRUE;

      /* were we capturing messages?  And if so, have we captured all */
      /* that we are allowed to capture?  If so, get out now          */
      if ((Read.Capture || Read.QWK) && Read.FoundTotal >= PcbData.MaxTotalMsgs)
        goto exit;

      /* did the user exit QuickScan prematurely?  If so, simply turn  */
      /* QuickScan off and send him to the message prompt              */
/*    if (Status.QuickScan && (Read.AllConf || Display.AbortPrintout)) { */
      if (Status.QuickScan && Display.AbortPrintout) {
        Display.AbortPrintout = Status.QuickScan = FALSE;
        OriginalConference = Status.Conference;
        goto readmenu;
      }

      /* were we forced to stop in another conference?                      */
      /* or are we not *now* reading allconf and yet we're in another conf? */
      /* if so, join the conference now                                     */
      if (Status.Conference != OriginalConference && (RetVal == -1 || ! Read.AllConf)) {
        OriginalConference = Status.Conference;
        if (RetVal == -1)
          goto exit;
      }

      if (Read.AllConf) {
        if (Read.StayInConf)
          Read.StayInConf = FALSE;
        else
          if (nextconference(RetVal) == -1)
            goto readmenu;
      }
    }

readmenu:
    freshline();
    if (! Display.AbortPrintout) {
      if (Status.Conference != OriginalConference) {
        Read.Conf = OriginalConference;
        getconfrecord(OriginalConference,&Status.CurConf);
        showconfjoined(OriginalConference,&Status.CurConf,FALSE,FALSE);
        loadcmds();
        confdetails(UPDATEUSER);
      }
      if (opennewmessagebase() == -1)  /* make sure the msg base is open */
        goto exit;
      Status.HeaderScan = Status.QuickScan = FALSE;  /* turn off quick scan */
      if (Read.Capture || Read.QWK)
        break;
      if (mainmessageprompt(0) == -1)
        goto exit;
    }
  }


exit:
  if (Read.NumSearches > 0) {
    stopsearch();
    Read.NumSearches = 0;
    Status.SearchText[0] = 0;
  }

  if (Read.Capture || Read.QWK) {
    Read.Capture        = FALSE;
    Display.ShowOnScreen = TRUE;
    newline();
    if (Read.QWK)
      finishqwkpacket((bool) (Read.FoundTotal > 0 ? TRUE : FALSE));  //lint !e534
    else
      closecapturefile();

    if (Read.FoundTotal == 0) {
      displaypcbtext(TXT_CAPISEMPTY,NEWLINE);
      unlink(Status.CapFileName);
      Status.CapFileName[0] = 0;
      Status.XferCapture = FALSE;
    } else {
      displaypcbtext(TXT_TOTALMSGSINCAPTURE,DEFAULTS);
      sprintf(Str," %u",Read.FoundTotal);
      println(Str);
      Status.XferCapture = TRUE;
      if (Read.QWK) {
        if (zipqwkfile() == -1) {
          Status.XferCapture = FALSE;
          restorepointers();
        }
        Read.QWK = FALSE;
        removeqwkfiles(FALSE);
      } else if (Status.ZipCap) {
        zipcapfile();
        Status.ZipCap = FALSE;
      }
    }
  }

final:
  cleanupmsgmemory();
  Status.HeaderScan = Status.QuickScan = Status.ReplyCommand = FALSE;
}
