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

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <borland.h>
#endif

#include "messages.h"

extern struct ffblk DTA;  /* declared in exist.c */


/********************************************************************
*
*  Function:  numcallers()
*
*  Desc    :  reads the header in the main board messsage base to determine
*             the number of callers to the system
*
*  Returns :  the number of callers, or 0 if an error occurs
*/

long LIBENTRY numcallers(void) {
  int                 RetVal;
  int                 MsgFile;
  msgbasediskstattype Header;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if ((MsgFile = dosopen(Status.MainMsgFile,OPEN_READ|OPEN_DENYNONE POS2ERROR)) == -1) {
    #ifdef __OS2__
      int ExtendedError = Os2Error.ExtendedError;
    #endif
    if (ExtendedError == 32)
      displaypcbtext(TXT_MSGBASEINUSE,NEWLINE|LFBEFORE|LOGIT);
    return(0);
  }

  RetVal = readcheck(MsgFile,&Header,sizeof(Header)-sizeof(Header.LockStatus));
  dosclose(MsgFile);
  return(RetVal == -1 ? 0 : bassngltolong(Header.NumCallers));
}


/********************************************************************
*
*  Function:  addcaller()
*
*  Desc    :  adjusts the "caller number" in the main board message base
*/

long LIBENTRY addcaller(void) {
  int             MsgFile;
  msgbasestattype MsgStats;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if (Asy.Online == LOCAL && PcbData.ExcludeLocals)
    return(numcallers());

  if ((MsgFile = dosopen(Status.MainMsgFile,OPEN_RDWR|OPEN_DENYNONE POS2ERROR)) == -1)
    return(0);

  if (lockmessagebase(MsgFile,&MsgStats) != -1) {
    if (MsgStats.NumCallers < 0)
      MsgStats.NumCallers = 0;
    MsgStats.NumCallers++;
    unlockmessagebase(MsgFile,&MsgStats);
  } else
    MsgStats.NumCallers = 0;

  dosclose(MsgFile);
  return(MsgStats.NumCallers);
}


/********************************************************************
*
*  Function:  msgbasestats()
*
*  Desc    :  reads the message base header and stores the translated header
*             information in the MsgStats variable passed to it.
*
*  Returns :  0 if okay or -1 if unable to open the message base
*/

int LIBENTRY msgbasestats(msgbasestattype *MsgStats) {
  if (getmsgbasestats(MsgStats,Status.CurConf.MsgFile) == -1) {
    #ifndef __OS2__
      if (ExtendedError == 32)
        displaypcbtext(TXT_MSGBASEINUSE,NEWLINE|LFBEFORE|LOGIT);
    #endif
    return(-1);
  }
  return(0);
}


long LIBENTRY lowmsgnum(void) {
  msgbasestattype Stats;
  return(msgbasestats(&Stats) == -1 ? 0 : Stats.LowMsgNum);
}

long LIBENTRY highmsgnum(void) {
  msgbasestattype Stats;
  return(msgbasestats(&Stats) == -1 ? 0 : Stats.HighMsgNum);
}


/********************************************************************
*
*  Function:  freemsgs()
*
*  Desc    :  When passed the message base statistics it calculates the
*             number of 'free' slots left in the message base index
*/

long LIBENTRY freemsgs(msgbasestattype *MsgStats) {
  long NumDefined;

  NumDefined = ((long) Status.CurConf.MsgBlocks) << 10;
  return(NumDefined - (MsgStats->HighMsgNum - MsgStats->LowMsgNum) - 1);
}


/********************************************************************
*
*  Function:  checkfreemsgs()
*
*  Desc    :  This function differs slightly from the above in that it first
*             opens the current conference message base to get the statistics
*             and then calls the above function to return the number of
*             'free' slots left in the message base index.
*/

long LIBENTRY checkfreemsgs(void) {
  msgbasestattype MsgStats;

  if (msgbasestats(&MsgStats) == -1)
    return(-1);
  return(freemsgs(&MsgStats));
}


/********************************************************************
*
*  Function:  updatemessageheader()
*
*  Desc    :  Performs a quick open of the message base in WRITE mode set
*             to DENYNONE.  It then seeks to the location of the message
*             header and updates it without locking the record.
*
* NOTE     :  There is no "file locking" nor "record locking" being done here
*             because I feel fairly certain that no two people will be updating
*             the reply status at the same time for the simple reason that
*             the message is only addressed to ONE person and only that one
*             person can be on at a time.  Of course, more than one person
*             might try to update an @USER@ message - the the reply date and
*             reply time in this case is nearly useless anyway .. and would
*             essentially be the same date and time of done from another node
*             in any case.
*/

void LIBENTRY updatemessageheader(newindextype *Index, msgheadertype *Header, msgbasestattype *MsgStats) {
  int File;

  if ((File = dosopencheck(Status.CurConf.MsgFile,OPEN_RDWR|OPEN_DENYNONE)) != -1) {
    doslseek(File,Index->Offset,SEEK_SET);
    writecheck(File,Header,sizeof(msgheadertype));    /*lint !e534 */
    dosclose(File);
  }

  updateindex(Index,Header,MsgStats);
}


static bool _NEAR_ LIBENTRY seclevelmatch(char *Str) {
  int Sec;

  if (*Str < '0' || *Str > '9' || strchr(Str,'@') == NULL)
    return(FALSE);

  Sec = atoi(Str);
  if (Status.CurSecLevel < Sec)                  /* too low?  return false */
    return(FALSE);

  if ((Str = strchr(Str,'-')) != NULL)
    return((bool) (Status.CurSecLevel <= atoi(Str+1)));   /* between low and high? */
  else
    return((bool) (Status.CurSecLevel == Sec));           /* equal to? return true */
}


/********************************************************************
*
*  Function:  readstatus()
*
*  Desc    :  checks to see if it's okay for a user to read a message
*
*             To make sure that name comparisons are done properly, it will
*             first convert the name to uppercase letters just in case some
*             OTHER software has written it in lowercase letters.
*
*  Returns :  okay to read and flags to indicate if it is TO or FROM the user
*/

readaccesstype LIBENTRY readstatus(newindextype *Index, msgstattype Msg) {
  readaccesstype Access;
  char           Save;
  char           AliasName[26];
  #ifdef PHILIP
  char           Str[80];
  static char    Types[5][5] = {"PBLC", "RCVR", "CMNT", "SPWD", "GPWD"};
  #endif

  Save = Index->Status;
  Index->Status = 0;
  strupr(Index->To);
  Index->Status = Save;

  #ifdef PHILIP
    sprintf(Str,"Msg=%ld, Type=%s, RA=%d, RC=%d, Cur=%d",
            Index->Num,
            Types[Msg.Type],
            PcbData.SysopSec[SEC_READALLMAIL],
            PcbData.SysopSec[SEC_READCOMMENTS],
            Status.CurSecLevel);
    writelog(Str,SPACERIGHT);
    sprintf(Str,"TO: %-25.25s",Index->To);
    writelog(Str,SPACERIGHT);
    sprintf(Str,"FROM: %-25.25s",Index->From);
    writelog(Str,SPACERIGHT);
  #endif

  if (AliasSupport && Status.CurConf.AllowAliases && UsersData.Alias[0] != 0)
    moveback(AliasName,UsersData.Alias,sizeof(UsersRead.Name));
  else
    AliasName[0] = 0;

  Access.OkayToRead = FALSE;
  if (memcmp(UsersRead.Name,Index->To,sizeof(UsersRead.Name)) == 0)
    Access.Msg.ToYou = TRUE;
  else if (Status.UserRecNo == 1 && memcmp(SysopName,Index->To,25) == 0)
    Access.Msg.ToYou = TRUE;
  else if (AliasName[0] != 0 && memcmp(AliasName,Index->To,sizeof(UsersRead.Name)) == 0)
    Access.Msg.ToYou = TRUE;
  else if (Index->To[0] == '@') {
    if (memcmp("USER@",&Index->To[1],5) == 0) {
      Access.Msg.ToYou = TRUE;
      Access.Msg.Generic = TRUE;
    } else if (memcmp("LIST@",&Index->To[1],5) == 0) {
      Access.Msg.ToYou = TRUE;  /* but MAYBE NOT! the main code will verify */
      Access.Msg.List  = TRUE;  /* this tells it to verify the LIST         */
    } else if (seclevelmatch(&Index->To[1]))
      Access.Msg.ToYou = TRUE;
    else
      Msg.Type = RCVR;
  }

  if (memcmp(UsersRead.Name,Index->From,sizeof(UsersRead.Name)) == 0)
    Access.Msg.FromYou = TRUE;
  else if (Status.UserRecNo == 1 && memcmp(SysopName,Index->From,25) == 0)
    Access.Msg.FromYou = TRUE;
  else if (AliasName[0] != 0 && memcmp(AliasName,Index->From,sizeof(UsersRead.Name)) == 0)
    Access.Msg.FromYou = TRUE;
  else if (Status.CurSecLevel < PcbData.SysopSec[SEC_SEERETRCPT] && memcmp("RETURN RECEIPT           ",Index->From,25) == 0 && ! Access.Msg.ToYou) {
    /* message is a RETURN RECEIPT and is NOT addressed to the caller, so */
    /* let's avoid showing it at all                                      */
    Access.OkayToRead = FALSE;
    return(Access);
  }


  switch (Msg.Type) {
    case PBLC  :
    case SPWD  :
    case GPWD  : Access.Msg.Okay = TRUE;
                 break;
    case RCVR  : if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READALLMAIL])
                   Access.Msg.Okay = TRUE;
                 break;
    case CMNT  : if (Status.CurSecLevel >= PcbData.SysopSec[SEC_READCOMMENTS])
                   Access.Msg.Okay = TRUE;
                 break;
  }

  #ifdef PHILIP
    sprintf(Str,"Access=%d",Access.OkayToRead);
    writelog(Str,SPACERIGHT);
  #endif
  return(Access);
}


/********************************************************************
*
*  Function:  getmsgstatus()
*
*  Desc    :  determines the type of message based on the Status flag
*
*  Returns :  the type of message and read status in one variable
*/

msgstattype LIBENTRY getmsgstatus(char StatusFromHeader) {
  msgstattype MsgStatus;

  MsgStatus.Type = PBLC;    /* give some defaults to initial Msg */
  MsgStatus.Read = FALSE;

  switch (StatusFromHeader) {
    case MSG_PBLC     : /* take the defaults */                       break;
    case MSG_PBLC_READ: MsgStatus.Read = TRUE;                        break;
    case MSG_SPWD     : MsgStatus.Type = SPWD;                        break;
    case MSG_SPWD_READ: MsgStatus.Type = SPWD; MsgStatus.Read = TRUE; break;
    case MSG_GPWD     : MsgStatus.Type = GPWD;                        break;
    case MSG_GPWD_READ: MsgStatus.Type = GPWD; MsgStatus.Read = TRUE; break;
    case MSG_GPWD_ALL : MsgStatus.Type = GPWD;                        break;
    case MSG_RCVR     : MsgStatus.Type = RCVR;                        break;
    case MSG_RCVR_READ: MsgStatus.Type = RCVR; MsgStatus.Read = TRUE; break;
    case MSG_CMNT     : MsgStatus.Type = CMNT;                        break;
    case MSG_CMNT_READ: MsgStatus.Type = CMNT; MsgStatus.Read = TRUE; break;
/*  default           : MsgStatus.Type = NODISP;                      break; */
  }
  return(MsgStatus);
}


/********************************************************************
*
*  Function:  getnewmsgstatus()
*
*  Desc    :  gets a new "Status" character based on current readtype
*
*  Returns :  the status character
*/

char LIBENTRY getnewmsgstatus(msgstattype MsgStatus) {
  char static Read[GPWD+1] = {MSG_PBLC_READ,MSG_RCVR_READ,MSG_CMNT_READ,MSG_SPWD_READ,MSG_GPWD_READ};
  char static UnRead[GPWD+1] = {MSG_PBLC,MSG_RCVR,MSG_CMNT,MSG_SPWD,MSG_GPWD};

  if (MsgStatus.Read)
    return(Read[MsgStatus.Type]);
  else
    return(UnRead[MsgStatus.Type]);
}


/********************************************************************
*
*  Function:  setreplydate()
*
*  Desc    :  takes the message header and stuffs the current time and date
*             into the ReplyTime and ReplyDate fields - and if the message was
*             actually being replied to (instead of just read) then it sets the
*             ReplyStatus to 'R'.
*/

void LIBENTRY setreplydate(msgheadertype *Header, msgstattype MsgStatus, bool RepliedTo) {
  char Temp[9];

  #pragma warn -stv
  Header->Status = getnewmsgstatus(MsgStatus);
  #pragma warn +stv
  memcpy(Header->ReplyTime,timestr2(Temp),5);

  strtoyymmdd(Temp,datestr(Temp));
  Temp[6] = 0;
  longtobassngl(Header->ReplyDate,atol(Temp));

  if (RepliedTo)
    Header->ReplyStatus = 'R';
}


void LIBENTRY updatemsgindex(void) {
  bool            Reset;
  bool            Displayed;
  long            Offset;
  long            Num;
  long            LastNum;
  long            Size;
  long            BytesNeeded;
  char            Date[9];
  msgbasestattype MsgStats;
  DOSFILE         IdxFile;
  DOSFILE         MsgFile;
  newindextype    Index;
  newindextype    Zeroes;
  char            Name[66];
  msgheadertype   Header;

  if (Status.CurConf.MsgFile[0] == 0 || fileexist(Status.CurConf.MsgFile) == 255)
    return;

  if (dosfopen(Status.CurConf.MsgFile,OPEN_READ|OPEN_DENYNONE,&MsgFile) == -1)
    return;

  openedmsgbasestreamstats(&MsgFile,&MsgStats);
  strcpy(Name,Status.CurConf.MsgFile);
  strcat(Name,".IDX");

  Size = (fileexist(Name) != 255 ? DTA.ff_fsize : 0);
  BytesNeeded = (((MsgStats.HighMsgNum - MsgStats.LowMsgNum) + 1) * sizeof(newindextype)) - Size;

  if (BytesNeeded == 0 || dosfopen(Name,OPEN_RDWR|OPEN_DENYNONE,&IdxFile) == -1) {
    dosfclose(&MsgFile);
    return;
  }

  showactivity(ACTBEGIN);

  Reset = FALSE;
  Offset = 128;
  LastNum = MsgStats.LowMsgNum - 1;

  if (Size == 0 || BytesNeeded < 0)
    Reset = TRUE;
  else {
    dosfread(&Index,sizeof(Index),&IdxFile);               /*lint !e534 */
    dosfseek(&MsgFile,128,SEEK_SET);
    dosfread(&Header,sizeof(msgheadertype),&MsgFile);      /*lint !e534 */
    if (Index.Num == bassngltolong(Header.MsgNumber)) {
      dosfseek(&IdxFile,Size-sizeof(Index),SEEK_SET);
      dosfread(&Index,sizeof(Index),&IdxFile);             /*lint !e534 */

      if (Index.Offset < 0)
        Index.Offset = -Index.Offset;

      dosfseek(&MsgFile,Index.Offset,SEEK_SET);
      dosfread(&Header,sizeof(msgheadertype),&MsgFile);    /*lint !e534 */
      if (Index.Num == bassngltolong(Header.MsgNumber)) {
        Offset = Index.Offset + (128 * Header.NumBlocks);
        LastNum = Index.Num;
      } else
        Reset = TRUE;
    } else
      Reset = TRUE;
  }

  if (Reset || BytesNeeded > 50000U) {
    if (Reset) {
      LastNum = MsgStats.LowMsgNum - 1;
      dosrewind(&IdxFile);
    }
    displaypcbtext(TXT_UPDATINGINDEX,LFBEFORE);
    Displayed = TRUE;
  } else
    Displayed = FALSE;

  memset(&Index,0,sizeof(Index));
  Zeroes = Index;

  dossetbuf(&MsgFile,2048);
  dossetbuf(&IdxFile,32768U);
  dosfseek(&MsgFile,Offset,SEEK_SET);

  Date[8] = 0;

  while (dosfread(&Header,sizeof(msgheadertype),&MsgFile) == sizeof(msgheadertype)) {
    memcpy(Date,Header.Date,8);

    /* skip over invalid headers */
    if ((Header.ActiveFlag == MSG_ACTIVE || Header.ActiveFlag == MSG_INACTIVE) &&
        Header.ToField[0] >= ' ' &&
        Header.FromField[0] >= ' ' &&
        Header.NumBlocks >= 2 &&
        (Index.Date = datetojulian(Date)) > 0 &&
        (Num = bassngltolong(Header.MsgNumber)) > LastNum &&
        Num <= MsgStats.HighMsgNum) {

      if (Num > LastNum+1) {
        for (; LastNum < Num-1; LastNum++) {
          showactivity(ACTSHOW);
          if (dosfwrite(&Zeroes,sizeof(Zeroes),&IdxFile) == -1)
            break;
        }
      }

      LastNum      = Num;
      Index.Num    = Num;
      Index.Offset = (Header.ActiveFlag == MSG_ACTIVE ? Offset : -Offset);
      Index.Status = Header.Status;
      memcpy(Index.To  ,Header.ToField  ,25);
      memcpy(Index.From,Header.FromField,25);

      if (dosfwrite(&Index,sizeof(Index),&IdxFile) == -1)
        break;

      Offset += sizeof(msgheadertype) * Header.NumBlocks;
      dosfseek(&MsgFile,Offset,SEEK_SET);
    } else
      Offset += sizeof(msgheadertype);

    showactivity(ACTSHOW);
  }

  while (LastNum < MsgStats.HighMsgNum) {
    if (dosfwrite(&Zeroes,sizeof(Zeroes),&IdxFile) == -1)
      break;
    LastNum++;
    showactivity(ACTSHOW);
  }

  dosftrunc(&IdxFile,-1);  /*lint !e534 truncate the file */

  showactivity(ACTEND);
  if (Displayed)
    newline();

  dosfclose(&IdxFile);
  dosfclose(&MsgFile);
}


void LIBENTRY updateindex(newindextype *Index, msgheadertype *Header, msgbasestattype *MsgStats) {
  int   IdxFile;
  char  Date[9];
  char  Name[66];

  strcpy(Name,Status.CurConf.MsgFile);
  strcat(Name,".IDX");

  if (Header->ActiveFlag == MSG_ACTIVE   && Index->Offset < 0 ||
      Header->ActiveFlag == MSG_INACTIVE && Index->Offset > 0)
    Index->Offset = - Index->Offset;

  Index->Status = Header->Status;
  memcpy(Index->To,Header->ToField,25);
  memcpy(Index->From,Header->FromField,25);

  memcpy(Date,Header->Date,8);  Date[8] = 0;
  Index->Date = datetojulian(Date);

  if ((IdxFile = dosopencheck(Name,OPEN_RDWR|OPEN_DENYNONE)) != -1) {
    doslseek(IdxFile,(Index->Num - MsgStats->LowMsgNum) * sizeof(newindextype),SEEK_SET);
    writecheck(IdxFile,Index,sizeof(newindextype));   /*lint !e534 */
    dosclose(IdxFile);
  }
}


char * LIBENTRY carbonedtoyou(char *Body) {
  msgextendtype *p;
  char           Alias[26];

  if (UsersData.Alias[0] == 0)
    Alias[0] = 0;
  else
    moveback(Alias,UsersData.Alias,sizeof(UsersRead.Name));

  p = (msgextendtype *) Body;
  while (p->Ident == EXTHDRID) {
    if (memcmp(p->Function,ExtHdrFunctions[EXTHDR_LIST],EXTFUNCLEN) == 0) {
      if (memicmp(p->Descript,UsersRead.Name,sizeof(UsersRead.Name)) == 0 ||
          (Status.UserRecNo == 1 && memicmp(p->Descript,SysopName,25) == 0) ||
          (Alias[0] != 0 && memicmp(p->Descript,Alias,sizeof(UsersRead.Name)) == 0))
        return((char *) p);
    }
    p++;
  }

  return(NULL);
}


static bool _NEAR_ LIBENTRY carbonlistmsgismine(char *MsgFile, long Offset) {
  bool          RetVal;
  char          *Body;
  unsigned      Size;
  DOSFILE       File;
  msgheadertype Header;

  if (dosfopen(MsgFile,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(FALSE);

  dosfseek(&File,Offset,SEEK_SET);
  if (dosfread(&Header,sizeof(Header),&File) != sizeof(Header)) {
    dosfclose(&File);
    return(FALSE);
  }

  if ((Header.ExtendedStatus & HDR_CARB) == 0) {
    dosfclose(&File);
    return(FALSE);
  }

  Size = (Header.NumBlocks-1) * 128;
  if ((Body = (char *) malloc(Size)) == NULL) {
    dosfclose(&File);
    return(FALSE);
  }

  RetVal = FALSE;
  if (dosfread(Body,Size,&File) == Size)
    RetVal = (bool) (carbonedtoyou(Body) != NULL);

  free(Body);
  dosfclose(&File);
  return(RetVal);
}


long LIBENTRY fixlmrpointer(unsigned short ConfNum, long LowMsgNum, long HighMsgNum) {
  long LastRead;

  LastRead = MsgReadPtr[ConfNum];

  /* if the LastRead pointer is LESS than the low message, then correct it */
  /* by moving it forward to the low message number                        */
  if (LastRead < LowMsgNum) {
    if (LowMsgNum > 0) {
      /* in moving the LastRead pointer forward, we don't want to skip the */
      /* first message in the message base, so back it up one.             */
      LowMsgNum--;
    }
    MsgReadPtr[ConfNum] = LastRead = LowMsgNum;
    return(LastRead);
  }

  /* if the LastRead pointer is GREATER than the high message, then correct */
  /* it by moving it back to the high message number                        */
  if (LastRead > HighMsgNum)
    MsgReadPtr[ConfNum] = LastRead = HighMsgNum;

  return(LastRead);
}


static bool _NEAR_ LIBENTRY verifynewmail(unsigned short ConfNum) {
  int             Handle;
  long            LastRead;
  msgbasestattype MsgStats;
  DOSFILE         File;
  char            AliasName[26];
  newindextype    Index;
  char            Name[66];
  pcbconftype     Conf;

  getconfrecord(ConfNum,&Conf);
  if (Conf.MsgFile[0] == 0)
    return(FALSE);

  if (OldIndex || Conf.OldIndex)
    updatemsgindex();

  if ((Handle = dosopencheck(Conf.MsgFile,OPEN_READ|OPEN_DENYNONE)) == -1)
    return(FALSE);

  openedmsgbasestats(Handle,&MsgStats);
  dosclose(Handle);

  /* are there any new messages at all?  if not, don't bother searching */
  if (MsgReadPtr[ConfNum] >= MsgStats.HighMsgNum)
    return(FALSE);

  strcpy(Name,Conf.MsgFile);
  strcat(Name,".IDX");
  if (dosfopen(Name,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(FALSE);

  LastRead = fixlmrpointer(ConfNum,MsgStats.LowMsgNum,MsgStats.HighMsgNum);
  LastRead++;  /* we want to begin scanning AFTER the last message read */

  dossetbuf(&File,8192);
  dosfseek(&File,(LastRead - MsgStats.LowMsgNum) * sizeof(newindextype),SEEK_SET);

  if (AliasSupport && Conf.AllowAliases && UsersData.Alias[0] != 0) {
    memcpy(AliasName,ReadAlias,25);
  } else
    AliasName[0] = 0;

  while (dosfread(&Index,sizeof(newindextype),&File) == sizeof(newindextype)) {
    if (Index.Offset > 0) {
      switch (Index.Status) {
        case MSG_RCVR:
        case MSG_CMNT:
        case MSG_SPWD:
        case MSG_GPWD:
        case MSG_PBLC: break;      /* msg has not been read, let it thru  */
        default      : continue;   /* message has been read, skip over it */
      }
      if (memicmp(UsersRead.Name,Index.To,sizeof(UsersRead.Name)) == 0)
        goto found;
      if (Status.UserRecNo == 1 && memicmp(SysopName,Index.To,25) == 0)
        goto found;
      if (AliasName[0] == Index.To[0] && memicmp(&AliasName[1],&Index.To[1],24) == 0)
        goto found;
      if (Index.To[0] == '@') {
        if (memcmp("USER@",&Index.To[1],5) == 0)
          goto found;
        if (seclevelmatch(&Index.To[1]))
          goto found;
        if (memcmp("LIST@",&Index.To[1],5) == 0) {
          if (carbonlistmsgismine(Conf.MsgFile,Index.Offset))
            goto found;
        }
      }
    }
  }

  dosfclose(&File);
  return(FALSE);

found:
  dosfclose(&File);
  return(TRUE);
}


bool LIBENTRY checkformail(void) {
  static char    mask[] = { 1,'A'};
  bool           FirstTime;
  bool           FirstAnnounce;
  bool           ReadMail;
  unsigned short X;
  unsigned short CurConf;
  char           Str[80];

  ReadMail = FALSE;
  if (Status.CurSecLevel < PcbData.UserLevels[SEC_R])
    goto end;

  FirstTime = TRUE;
  FirstAnnounce = TRUE;

  for (X = 0; X < PcbData.NumAreas; X++) {
    if (isset(&ConfReg[MFL],X)) {
      if (X != Status.Conference && Status.CurSecLevel < PcbData.UserLevels[SEC_J])
        continue;
      if (isregistered(ConfReg,X)) {
        if (FirstAnnounce) {
          freshline();
          displaypcbtext(TXT_SCANNINGFORMAIL,LFBEFORE);
          FirstAnnounce = FALSE;
          showactivity(ACTBEGIN);
        }
        if (verifynewmail(X)) {
          showactivity(ACTSUSPEND);
          if (FirstTime) {
            backupcleareol(awherex());
            displaypcbtext(TXT_MAILWAITINGIN,NEWLINE|BELL);
            printdefcolor();
            FirstTime = FALSE;
          }
          makeconfstr(Str,X);
          print(Str);
          println("...");
          showactivity(ACTRESUME);
        } else
          unsetbit(&ConfReg[MFL],X);
      } else
        unsetbit(&ConfReg[MFL],X);
      if (! FirstAnnounce)
        showactivity(ACTSHOW);
    }
  }

  if (! FirstAnnounce) {          /* we DID scan for mail  */
    showactivity(ACTEND);         /* get rid of the dots   */
    if (FirstTime) {              /* did we find any mail? */
      backupcleareol(awherex());  /* no, get rid of the scan for mail msg */
      UsersData.PackedFlags.HasMail = FALSE;
    }
  }

  if (PcbData.PromptToReadMail && ! FirstTime) {
    if (Status.ForceReadMail) {
      displaypcbtext(TXT_FORCEREADMAIL,NEWLINE|LFBEFORE|LFAFTER);
      moreprompt(PRESSENTER);
      strcpy(Str,"R Y O U WAIT");
    } else {
      Str[0] = NoChar;
      Str[1] = 0;
      inputfield(Str,TXT_READMAILNOW,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE|AUTO,NOHELP,mask);
      if (Str[0] == YesChar)
        strcpy(Str,"R Y O U WAIT");
      else if (Str[0] == 'A')
        strcpy(Str,"R WAIT");
      else
        Str[0] = 0;
    }

    if (Str[0] != 0) {
      CurConf = Status.Conference;
      messageread(tokenize(Str));
      newline();
      ReadMail = TRUE;
      // if reading mail left us in another conference, back back to the
      // original where we were...
      if (CurConf != Status.Conference) {
        getconfrecord(CurConf,&Status.CurConf);
        showconfjoined(CurConf,&Status.CurConf,FALSE,FALSE);
      }
      // if we captured mail for download then download it now
      if (Status.XferCapture)
        send(0);
    }
  }

end:
  Status.CheckForMail = FALSE;
  UsersData.PackedFlags.HasMail = FALSE;
  return(ReadMail);
}


/********************************************************************
*
*  Function:  updatemessagebody()
*
*  Desc    :  1) opens the message base and locks it for writing
*             2) writes the new msg body over top of the existing msg body
*
*             This function is used to simply update information IN PLACE
*             with no chance for the body of the message to grow in size or
*             change in any way.  This function is called when an extended
*             header has been updated - changing the STATUS of the extended
*             header for example (i.e. to show that a Carbon List message
*             has been read).  It does NOT attempt to update the message header
*             itself.  It does not attempt to lock the message base.  It has
*             no need for the index files.  It simply updates the message body.
*
*  Returns :  -1 if an error occured or 0 if all went well
*/

int LIBENTRY updatemessagebody(long Offset, char *Body, int NumBlocks) {
  int File;

  if ((File = dosopencheck(Status.CurConf.MsgFile,OPEN_RDWR|OPEN_DENYNONE)) == -1)
    return(-1);

  /* skip over the message header, jump right to the body of the message */
  doslseek(File,Offset+sizeof(msgheadertype),SEEK_SET);

  if (writecheck(File,Body,(NumBlocks-1)*128) == (unsigned) -1) {
    dosclose(File);
    return(-1);
  }

  dosclose(File);
  return(0);
}


char * LIBENTRY getqwkroot(void) {
  if (PcbData.QwkFile[0] != 0)
    return(PcbData.QwkFile);
  if (PcbData.CapFile[0] != 0)
    return(PcbData.CapFile);
  return("UNNAMED");
}


void LIBENTRY qwkcommand(int NumTokens) {
  bool Bye;
  bool Net;
  bool SetLMRs;
  char Direction;
  char *p;
  char Name[66];
  char Msgs[66];
  char Command[256];

  if (NumTokens == 0) {
    inputfield(Command,TXT_QWKCOMMANDS,sizeof(Command)-1,UPCASE|STACKED|NEWLINE|LFBEFORE,HLP_QWK,mask_msgread);
    NumTokens = tokenize(Command);
    if (NumTokens == 0)
      return;
  }

  Bye     = FALSE;
  Net     = FALSE;
  SetLMRs = FALSE;

  for (Direction = 0, Status.DefaultProtocol = 0; NumTokens > 0; NumTokens--) {
    p = getnexttoken();
    if (p[1] == 0) {
      if (Direction == 0 && (*p == 'D' || *p == 'U'))
        Direction = *p;
      else
        Status.DefaultProtocol = *p;
    } else {
      if (strcmp(p,"BYE") == 0)
        Bye = TRUE;
      else if (QwkSupport && strcmp(p,"NET") == 0)
        Net = TRUE;
      else if (strcmp(p,"LMR") == 0)
        SetLMRs = TRUE;
    }
  }

  if (Direction == 'D') {
    Status.ReplyCommand = FALSE;
    strcpy(Command,"A QWK");

    if (Bye) {
      addchar(Command,' ');
      strcat(Command,"BYE");
    }

    if (Net) {
      addchar(Command,' ');
      strcat(Command,"NET");
    }

    NumTokens = tokenize(Command);
    dispatch("R",PcbData.UserLevels[SEC_R],NumTokens+1,messageread);

    if (Status.XferCapture)
      send(0);
    else if (Bye)                    /* if there was nothing to transfer */
      autogoodbye(10);               /* still do the auto-logoff         */
  } else if (Direction == 'U') {
    Status.EnteringMessage = TRUE;   /* these two settings will force    */
    Status.FileAttach      = FALSE;  /* MSGS###.$$$ to be received       */

    Status.MsgUpload = TRUE;
    Status.QwkUpload = TRUE;
    receive(0);
    Status.MsgUpload = FALSE;
    Status.QwkUpload = FALSE;
    Status.EnteringMessage = FALSE;

    sprintf(Name,"MSG%d.$$$",PcbData.NodeNum);
    if (fileexist(Name) == 255) {
      sprintf(Command,"%s NOT FOUND!",Name);
      writelog(Command,SPACERIGHT);
      usernetavailable();
    } else {
      buildstr(Msgs,PcbData.TmpLoc,getqwkroot(),".MSG",NULL);
      if (extractmessages(Name,Msgs) != -1)
        readqwkreplies(Msgs,SetLMRs);
    }

    if (Bye)
      autogoodbye(10);
  }

  Status.DefaultProtocol = 0;
}


int LIBENTRY putmessageheader(msgbasetype *MsgBase, long *IdxOffset, long MsgOffset, newindextype *Index) {
  doslseek(MsgBase->Msgs.handle,MsgOffset,SEEK_SET);
  if (writecheck(MsgBase->Msgs.handle,&MsgBase->Header,sizeof(MsgBase->Header)) != (unsigned) -1) {
    doslseek(MsgBase->Index,*IdxOffset,SEEK_SET);
    if (writecheck(MsgBase->Index,Index,sizeof(newindextype)) != (unsigned) -1) {
      updateindex(Index,&MsgBase->Header,&MsgBase->Stats);
      updateoldindex(&Status.CurConf,Index->Offset,Index->Num,MsgBase->Stats.LowMsgNum);
    }
    return(0);
  }
  return(-1);
}


void LIBENTRY setreplystatus(long MsgNum) {
  long         IdxOffset;
  msgstattype  MsgStatus;
  newindextype Index;
  msgbasetype  WriteMsgBase;

  if (openmessagebase(Status.Conference,&Status.CurConf,&WriteMsgBase,RDWR) != -1) {
    if (getmessageheader(MsgNum,&WriteMsgBase,&IdxOffset,&Index) != -1 && Index.Offset >= 0) {
      MsgStatus = getmsgstatus(Index.Status);
      setreplydate(&WriteMsgBase.Header,MsgStatus,TRUE);
      putmessageheader(&WriteMsgBase,&IdxOffset,Index.Offset,&Index);  /*lint !e534 */
    }
    closemessagebase(&WriteMsgBase);
  }
}
