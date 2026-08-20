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

#include <io.h>
#include "messages.h"

#ifdef _MSC_VER
#include <borland.h>
#endif

char ExtHdrFunctions[EXTHDR_TOTAL][EXTFUNCLEN+1] = {
  "TO     ",
  "FROM   ",
  "SUBJECT",
  "ATTACH ",
  "LIST   ",
  "ROUTE  ",
  "ORIGIN ",
  "REQRR  ",
  "ACKRR  ",
  "ACKNAME",
  "PACKOUT",
  "TO2    ",
  "FROM2  ",
  "FORWARD",
  "UFOLLOW",
  "UNEWSGR",
};


msgextendtype *ExtHdr[MAXEXTHDRS]; /* dynamically alloc extended headers */
int            NumExtHdrs = 0;     /* number of extended headers         */
char           ExtendedStatus = 0; /* used to build the status value     */
bool           CreateReceipt;      /* TRUE if creating a RETURN RECEIPT  */



/********************************************************************
*
*  Function:  hdrtostat()
*
*  Desc    :  translates the message base header into the msgbasestat format
*/

static void _NEAR_ LIBENTRY hdrtostat(msgbasediskstattype *MsgBaseDiskStats, msgbasestattype *MsgStats) {
  checkstack();
  MsgStats->NumCallers    = bassngltolong(MsgBaseDiskStats->NumCallers);
  MsgStats->HighMsgNum    = bassngltolong(MsgBaseDiskStats->HighMsgNum);
  MsgStats->NumActiveMsgs = bassngltolong(MsgBaseDiskStats->NumActiveMsgs);
  MsgStats->LowMsgNum     = (MsgStats->NumActiveMsgs == 0 ? MsgStats->HighMsgNum : bassngltolong(MsgBaseDiskStats->LowMsgNum));
}


/********************************************************************
*
*  Function:  stattohdr()
*
*  Desc    :  translates the msgbasestat format into the header format
*/

static void _NEAR_ LIBENTRY stattohdr(msgbasestattype *MsgStats, msgbasediskstattype *MsgBaseDiskStats) {
  checkstack();
  longtobassngl(MsgBaseDiskStats->HighMsgNum   ,MsgStats->HighMsgNum);
  longtobassngl(MsgBaseDiskStats->LowMsgNum    ,MsgStats->LowMsgNum);
  longtobassngl(MsgBaseDiskStats->NumActiveMsgs,MsgStats->NumActiveMsgs);
  longtobassngl(MsgBaseDiskStats->NumCallers   ,MsgStats->NumCallers);
}


/********************************************************************
*
*  Function:  lockmessagebase()
*
*  Desc    :  performs a DOS lock on the "header" portion of the message base
*
*  Returns :  0 if successfully locked, or -1 if an error occurs
*/

int LIBENTRY lockmessagebase(int MsgFile, msgbasestattype *MsgStats) {
  msgbasediskstattype Header;

  checkstack();
  if (PcbData.Network) {
    if (doslockcheck(MsgFile,16,6) == -1)
      return(-1);
  }

  doslseek(MsgFile,0,SEEK_SET);
  if (readcheck(MsgFile,&Header,sizeof(Header)-sizeof(Header.LockStatus)) == (unsigned) -1)
    return(-1);

  hdrtostat(&Header,MsgStats);
  return(0);
}


/********************************************************************
*
*  Function:  unlockmessagebase()
*
*  Desc    :  updates the message status block
*             then unlocks the message base
*/

void LIBENTRY unlockmessagebase(int MsgFile, msgbasestattype *MsgStats) {
  msgbasediskstattype Header;

  checkstack();
  doslseek(MsgFile,0,SEEK_SET);
  stattohdr(MsgStats,&Header);
  memset(Header.LockStatus,' ',6);
  writecheck(MsgFile,&Header,sizeof(Header));    /*lint !e534 */
  if (PcbData.Network)
    unlock(MsgFile,16,6);
}



/********************************************************************
*
*  Function:  openedmsgbasestats()
*
*  Desc    :  reads the message base header and stores the translated header
*             information in the MsgStats variable passed to it.
*/

void LIBENTRY openedmsgbasestats(int MsgFile, msgbasestattype *MsgStats) {
  msgbasediskstattype Header;

  checkstack();
  doslseek(MsgFile,0,SEEK_SET);
  if (readcheck(MsgFile,&Header,sizeof(Header)-sizeof(Header.LockStatus)) != (unsigned) -1)
    hdrtostat(&Header,MsgStats);
  else {
    MsgStats->HighMsgNum    = 0;
    MsgStats->LowMsgNum     = 0;
    MsgStats->NumActiveMsgs = 0;
  }
}

/* keep the stream buffer synchronized while using the low-level handle to   */
/* obtain the message base statistics using the existing function to read it */
void LIBENTRY openedmsgbasestreamstats(DOSFILE *File, msgbasestattype *MsgStats) {
  checkstack();
  openedmsgbasestats(File->handle,MsgStats);
  dosflush(File);
}


int LIBENTRY getmsgbasestats(msgbasestattype *MsgStats, char *FileName) {
  int MsgFile;

  checkstack();
  if (FileName[0] == 0 || fileexist(FileName) == 255) {
    memset(MsgStats,0,sizeof(msgbasestattype));
    return(-1);
  }

  if ((MsgFile = dosopencheck(FileName,OPEN_READ|OPEN_DENYNONE)) == -1) {
    memset(MsgStats,0,sizeof(msgbasestattype));
    return(-1);
  }

  openedmsgbasestats(MsgFile,MsgStats);
  dosclose(MsgFile);
  return(0);
}


/********************************************************************
*
*  Function:  openmessagebase()
*
*  Desc    :  Opens the message base for the conference specified by "Num".
*             It then opens up the message base index file and if in RDWR mode
*             it locks the message base.
*
*  Returns :  0 if successful, -1 if an error occurs
*/

int LIBENTRY openmessagebase(unsigned short Num, pcbconftype *Conf, msgbasetype *MsgBase, openmsgtype OpenType) {
  int  OpenMode;
  int  Retry;
  char FileName[66];

  checkstack();
  if (Conf->MsgFile[0] == 0) {
#ifndef LIB
    if (DebugLevel >= 1) {
      sprintf(FileName,"ERROR: Conf #%d MSGS is blank",Num);
      writedebugrecord(FileName);
    }
#endif
    return(-1);
  }

  switch (OpenType) {
    case RDWR    :
    case RDWRLOCK: OpenMode = OPEN_RDWR|OPEN_DENYNONE; break;
    case READONLY: OpenMode = OPEN_READ|OPEN_DENYNONE; break;
    default      : MsgBase->ConfNum  = 0xFFFF; return(-1);
  }

  MsgBase->ConfNum  = Num;
  MsgBase->OpenMode = OpenType;
  MsgBase->Body     = NULL;
  strcpy(FileName,Conf->MsgFile);
  Retry = 0;

retry:
  if (fileexist(FileName) == 255) {
    if (Retry > 4) {
      MsgBase->ConfNum = 0xFFFF;
      return(-1);
    }
    Retry++;
    tickdelay(QUARTERSECOND);
    goto retry;
  }

  if (dosfopen(FileName,OpenMode,&MsgBase->Msgs) == -1) {
    MsgBase->ConfNum  = 0xFFFF;
    return(-1);
  }

  strcat(FileName,".IDX");
  if ((MsgBase->Index = dosopencheck(FileName,OpenMode)) == -1) {
    dosfclose(&MsgBase->Msgs);
    MsgBase->ConfNum  = 0xFFFF;
    return(-1);
  }

  if (OpenType == RDWRLOCK) {
   if (lockmessagebase(MsgBase->Msgs.handle,&MsgBase->Stats) == -1) {
      dosclose(MsgBase->Index);
      dosfclose(&MsgBase->Msgs);
      MsgBase->ConfNum  = 0xFFFF;
      return(-1);
    }
    return(0);
  }

  openedmsgbasestreamstats(&MsgBase->Msgs,&MsgBase->Stats);
  return(0);
}


/********************************************************************
*
*  Function:  closemessagebase()
*
*  Desc    :  If the message base was in RDWR mode it unlocks the message base.
*             It then closes the message base and index files and frees up any
*             memory that may have been allocated for the message base.
*/

void LIBENTRY closemessagebase(msgbasetype *MsgBase) {
  checkstack();
  if (MsgBase->ConfNum != 0xFFFF && MsgBase->Msgs.handle > 0) {
    switch (MsgBase->OpenMode) {
      case READONLY : dosfclose(&MsgBase->Msgs);
                      if (MsgBase->BodySize != 0) {
                        bfree(MsgBase->Body);
                        MsgBase->Body = NULL;
                      }
                      break;
      case RDWR     : dosflush(&MsgBase->Msgs);
                      dosfclose(&MsgBase->Msgs);
                      break;
      case RDWRLOCK : dosflush(&MsgBase->Msgs);
                      unlockmessagebase(MsgBase->Msgs.handle,&MsgBase->Stats);
                      dosfclose(&MsgBase->Msgs);
                      break;
    }
    dosclose(MsgBase->Index);
    MsgBase->ConfNum  = 0xFFFF;
    MsgBase->BodySize = 0;
  }
}


char * LIBENTRY getextendedheader(exthdrtype Which, char *Body) {
  msgextendtype *p;

  checkstack();
  for (p = (msgextendtype *) Body; p->Ident == EXTHDRID; p++) {
    if (memcmp(p->Function,ExtHdrFunctions[Which],EXTFUNCLEN) == 0)
      return(p->Descript);
  }
  return(NULL);
}


char * LIBENTRY skipextendedheader(char *Body) {
  msgextendtype *p;

  checkstack();
  p = (msgextendtype *) Body;
  while (p->Ident == EXTHDRID)
    p++;

  return((char *) p);
}


int LIBENTRY getmessageheader(long MsgNumber, msgbasetype *MsgBase, long *IdxOffset, newindextype *Index) {
  long MsgOffset;

  checkstack();
  if (MsgNumber < MsgBase->Stats.LowMsgNum ||
      MsgNumber > MsgBase->Stats.HighMsgNum)
    return(-1);

  *IdxOffset = (MsgNumber - MsgBase->Stats.LowMsgNum) * sizeof(newindextype);
  doslseek(MsgBase->Index,*IdxOffset,SEEK_SET);
  if (readcheck(MsgBase->Index,Index,sizeof(newindextype)) == (unsigned) -1)
    return(-1);

  if (Index->Offset < 0)
    MsgOffset = -Index->Offset;
  else
    MsgOffset = Index->Offset;

  doslseek(MsgBase->Msgs.handle,MsgOffset,SEEK_SET);
  if (readcheck(MsgBase->Msgs.handle,&MsgBase->Header,sizeof(MsgBase->Header)) == (unsigned) -1)
    return(-1);

  if (bassngltolong(MsgBase->Header.MsgNumber) != MsgNumber)
    return(-1);

  return(0);
}


/********************************************************************
*
*  Function:  loadextendedheaders()
*
*  Desc    :  This function takes an existing "packed" message body and reads
*             the extended headers out of the message body.  In doing so, it
*             allocates memory to hold each of the headers found, and uses an
*             array, called ExtHdrs, to store the pointers to each extended
*             header.
*
*             In addition, "bit values" are stored in an integer called
*             ExtendedStatus to indicate which headers were found.
*
*  Returns :  A pointer to the first character AFTER the extended headers.
*             In other words, a pointer to the actual "body" of the message.
*/

char * LIBENTRY loadextendedheaders(char *Body) {
  msgextendtype *p;
  msgextendtype *q;
  char          *Func;

  checkstack();
  p = (msgextendtype *) Body;
  while (p->Ident == EXTHDRID) {
    if (NumExtHdrs < MAXEXTHDRS) {
      if ((q = (msgextendtype *) bmalloc(sizeof(msgextendtype))) != NULL) {
//      *q = *p;
        memcpy(q,p,sizeof(*q));
        ExtHdr[NumExtHdrs++] = q;
        Func = q->Function;
        if (memcmp(Func,ExtHdrFunctions[EXTHDR_TO],sizeof(p->Function)) == 0)
          ExtendedStatus |= HDR_TO;
        else if (memcmp(Func,ExtHdrFunctions[EXTHDR_FROM],sizeof(p->Function)) == 0)
          ExtendedStatus |= HDR_FROM;
        else if (memcmp(Func,ExtHdrFunctions[EXTHDR_SUBJECT],sizeof(p->Function)) == 0)
          ExtendedStatus |= HDR_SUBJ;
        else if (memcmp(Func,ExtHdrFunctions[EXTHDR_LIST],sizeof(p->Function)) == 0)
          ExtendedStatus |= HDR_CARB;
        else if (memcmp(Func,ExtHdrFunctions[EXTHDR_ATTACH],sizeof(p->Function)) == 0)
          ExtendedStatus |= HDR_FILE;
        else if (memcmp(Func,ExtHdrFunctions[EXTHDR_REQRR],sizeof(p->Function)) == 0)
          ExtendedStatus |= HDR_RCPT;
        else if (memcmp(Func,ExtHdrFunctions[EXTHDR_ACKRR],sizeof(p->Function)) == 0)
          CreateReceipt = TRUE;
      }
    }
    p++;
  }

  return((char *) p);
}


/********************************************************************
*
*  Function:  checkextendedheaders()
*
*  Desc    :  This function scans the existing Extended Headers, found via the
*             ExtHdrs array, to see if a particular extended header is in use.
*
*  Returns :  If the extended header is found, a pointer to the contents of
*             the header is returned.  Otherwise, NULL is returned.
*/

char * LIBENTRY checkextendedheaders(exthdrtype Which, char *Desc) {
  int  X;
  char Descript[EXTDESCLEN];

  checkstack();
  memset(Descript,' ',EXTDESCLEN);
  if (Desc != NULL)
    memcpy(Descript,Desc,strlen(Desc));

  for (X = 0; X < NumExtHdrs; X++) {
    if (memcmp(ExtHdr[X]->Function,ExtHdrFunctions[Which],EXTFUNCLEN) == 0) {
      if (Desc == NULL)
        return(ExtHdr[X]->Descript);
      if (memcmp(ExtHdr[X]->Descript,Descript,EXTDESCLEN) == 0)
        return(ExtHdr[X]->Descript);
    }
  }
  return(NULL);
}


/********************************************************************
*
*  Function:  buildextheader()
*
*  Desc    :  This function builds a new header and adds it into the ExtHdrs
*             array after allocating memory to hold the new header.
*
*             Some extended headers also have a bit value associated with them.
*             The BitMask value is OR'd with the current ExtendedStatus value
*             in order to keep track of which headers are in use.
*/

void * LIBENTRY buildgenextheader(char *HdrTitle, char *Desc) {
  msgextendtype *p;
  unsigned      Len;

  checkstack();
  if (NumExtHdrs < MAXEXTHDRS) {
    if ((p = (msgextendtype *) bmalloc(sizeof(msgextendtype))) != NULL) {
      ExtHdr[NumExtHdrs++] = p;
      memset(p,' ',sizeof(msgextendtype));

      Len = strlen(HdrTitle);
      if (Len > sizeof(p->Function))
        Len = sizeof(p->Function);
      memcpy(p->Function,HdrTitle,Len);

      Len = strlen(Desc);
      if (Len > sizeof(p->Descript))
        Len = sizeof(p->Descript);
      memcpy(p->Descript,Desc,Len);

      p->Ident = EXTHDRID;
      p->Colon = ':';
      p->Status = EXTSTAT_NONE;
      p->CarReturn = LineSeparator;
      return(p);
    }
  }

  return(NULL);
}


void LIBENTRY buildextheader(exthdrtype Which, char *Desc, char BitMask) {
  checkstack();
  if (buildgenextheader(ExtHdrFunctions[Which],Desc) != NULL)
    ExtendedStatus |= BitMask;
}


/********************************************************************
*
*  Function:  copyexthdrs()
*
*  Desc    :  This function copies extended headers out of the ExtHdr array
*             directly to disk.  A handle to an open file is provided.
*
*  Returns :  0 if no extended headers were written, otherwise it returns the
*             number of bytes taken up by the extended headers that were
*             written out to disk.
*/

static int _NEAR_ LIBENTRY copyexthdrs(int Handle) {
  int Bytes;
  int X;

  checkstack();
  if (NumExtHdrs == 0)
    return(0);

  for (X = 0, Bytes = 0; X < NumExtHdrs; X++) {
    if (ExtHdr[X] != NULL) {
      if (writecheck(Handle,ExtHdr[X],sizeof(msgextendtype)) == (unsigned) -1)
        return(-1);
      Bytes += sizeof(msgextendtype);
    }
  }
  return(Bytes);
}


/********************************************************************
*
*  Function:  removeexthdr()
*
*  Desc    :  This function removes an existing extended header from the ExtHdr
*             array.  If the header exists, its memory is freed and the ExtHdr
*             pointer is set to NULL.  Also, if it had a bit value associated
*             with it, the bit value is removed from the ExtendedStatus flags.
*/

void LIBENTRY removeexthdr(exthdrtype Which, char BitMask) {
  int X;

  checkstack();
  for (X = 0; X < NumExtHdrs; X++) {
    if (memcmp(ExtHdr[X]->Function,ExtHdrFunctions[Which],EXTFUNCLEN) == 0) {
      bfree(ExtHdr[X]);
      ExtHdr[X] = NULL;
      ExtendedStatus &= (bool) ~BitMask;
      goto found;
    }
  }
  return;

found:
  /* if it wasn't the last one in the table, then we have to move things around */
  if (X < NumExtHdrs-1)
    memmove(&ExtHdr[X],&ExtHdr[X+1],(NumExtHdrs - X - 1) * sizeof(ExtHdr[0]));

  NumExtHdrs--;
}


void LIBENTRY updateoldindex(pcbconftype *Conf, long Offset, long Num, long LowMsgNum) {
  int     Killed;
  int     NdxFile;
  bassngl BasNum;
  char    Name[66];

  checkstack();
  if (OldIndex || Conf->OldIndex) {
    strcpy(Name,Conf->MsgFile);
    strcat(Name,".NDX");
    Killed = (Offset < 0 ? -1 : 1);
    Offset *= Killed;        /* remove negative if killed */
    Offset /= 128;
    Offset++;
    Offset *= Killed;        /* make negative again if killed */
    longtobassngl(BasNum,Offset);
    if ((NdxFile = dosopencheck(Name,OPEN_RDWR|OPEN_DENYNONE)) != -1) {
      doslseek(NdxFile,(Num - LowMsgNum) * sizeof(bassngl),SEEK_SET);
      writecheck(NdxFile,BasNum,sizeof(bassngl));    /*lint !e534 */
      dosclose(NdxFile);
    }
  }
}


int LIBENTRY savetomsgbase(unsigned short ConfNum,
                         pcbconftype   *Conf,
                         msgbasetype   *MsgBase,
                         msgheadertype *Header,
                         char          *Body,
                         long           OldMsgNumber,
                         long           OldOffset,
                         void (LIBENTRY  *showsaving)(unsigned short ConfNum, long MsgNum, long MsgOffset)) {

  unsigned      NumBlocksToWrite;
  unsigned      NumBytesToWrite;
  int           ExtHdrSize;
  char          Date[9];
  char          Name[26];
  newindextype  Index;

  checkstack();
  NumBlocksToWrite = Header->NumBlocks;

  memcpy(Date,Header->Date,8); Date[8] = 0;
  Index.Date = datetojulian(Date);
  Index.Status = Header->Status;

  /* store the TO field in all uppercase letters */
  memcpy(Name,Header->ToField,25); Name[25] = 0;
  strupr(Name);
  memcpy(Index.To,Name,25);

  memcpy(Index.From,Header->FromField,25);
  memset(Index.Reserved,0,sizeof(Index.Reserved));

  if (OldMsgNumber == 0) {
    if (MsgBase->Stats.LowMsgNum == 0)
      MsgBase->Stats.LowMsgNum = 1;
    MsgBase->Stats.HighMsgNum++;
    MsgBase->Stats.NumActiveMsgs++;
    Index.Offset = doslseek(MsgBase->Msgs.handle,0,SEEK_END);
    Index.Num = MsgBase->Stats.HighMsgNum;
  } else {
    Index.Offset = doslseek(MsgBase->Msgs.handle,OldOffset,SEEK_SET);
    Index.Num = OldMsgNumber;
  }

  longtobassngl(Header->MsgNumber,Index.Num);

  if (showsaving != NULL)
    showsaving(ConfNum,Index.Num,Index.Offset);

  if (writecheck(MsgBase->Msgs.handle,Header,sizeof(msgheadertype)) == (unsigned) -1)
    return(-1);

  /* copy extended headers into the body of the message, possible return  */
  /* values are 0 (nothing written), -1 (an error occurred), or a positve */
  /* integer representing the number of bytes written to disk             */
  if ((ExtHdrSize = copyexthdrs(MsgBase->Msgs.handle)) == -1)
    return(-1);

  /* Take NumBlocksToWrite minus 1 (don't count the just written header, */
  /* multiply that value by 128 for the total storage requirements, then */
  /* subtract out any bytes that were written for extended headers       */
  NumBytesToWrite = ((NumBlocksToWrite-1) * 128) - ExtHdrSize;

  if (writecheck(MsgBase->Msgs.handle,Body,NumBytesToWrite) == (unsigned) -1)
    return(-1);

  doslseek(MsgBase->Index,(Index.Num-MsgBase->Stats.LowMsgNum)*sizeof(newindextype),SEEK_SET);
  if (writecheck(MsgBase->Index,&Index,sizeof(newindextype)) == (unsigned) -1)
    return(-1);

  updateoldindex(Conf,Index.Offset,Index.Num,MsgBase->Stats.LowMsgNum);
  return(0);
}


static void _NEAR_ LIBENTRY maxmemcpy(void *Dest, void *Srce, int Len, int MaxLen) {
  checkstack();
  if (Len > MaxLen)
    Len = MaxLen;
  memcpy(Dest,Srce,Len);
}


void LIBENTRY makemsgheader(msgheadertype *Header,
                          char           MsgStatus,
                          unsigned short NumBytes,
                          char          *Date,
                          char          *Time,
                          char          *To,
                          long           RefNum,
                          unsigned short ReadDate,
                          char          *ReadTime,
                          char           ReadStatus,
                          char          *From,
                          char          *Subject,
                          char          *Password,
                          bool           Echo) {

  char Temp1[9];
  char Temp2[9];
  long Temp3;

  checkstack();
  memset(Header,' ',sizeof(msgheadertype));
  memset(Header->Reserved,0,sizeof(Header->Reserved));

  Header->Status         = MsgStatus;
  Header->ReplyStatus    = ReadStatus;
  Header->ActiveFlag     = MSG_ACTIVE;
  Header->EchoFlag       = (Echo ? 'E' : ' ');
  Header->ExtendedStatus = ExtendedStatus;
  Header->NetTag         = 0;

  maxmemcpy(Header->Date     ,Date    ,strlen(Date)    ,sizeof(Header->Date));
  maxmemcpy(Header->Time     ,Time    ,strlen(Time)    ,sizeof(Header->Time));
  maxmemcpy(Header->ToField  ,To      ,strlen(To)      ,sizeof(Header->ToField));
  maxmemcpy(Header->FromField,From    ,strlen(From)    ,sizeof(Header->FromField));
  maxmemcpy(Header->SubjField,Subject ,strlen(Subject) ,sizeof(Header->SubjField));
  maxmemcpy(Header->Password ,Password,strlen(Password),sizeof(Header->Password));
  maxmemcpy(Header->Time     ,ReadTime,strlen(ReadTime),sizeof(Header->Time));

  longtobassngl(Header->RefNumber,RefNum);

  if (ReadDate == 0)
    Temp3 = 0;
  else {
    strcpy(Temp1,juliantodate(ReadDate));
    strtoyymmdd(Temp2,Temp1);
    Temp3 = atol(Temp2);
  }

  longtobassngl(Header->ReplyDate,Temp3);

  NumBytes += (unsigned short) (NumExtHdrs * sizeof(msgextendtype));
  Header->NumBlocks = (char) ((NumBytes / 128) + 1);   /* add one for the header */
  if ((NumBytes & 127) != 0)
    Header->NumBlocks++;
}


void LIBENTRY freehdrs(void) {
  checkstack();
  if (NumExtHdrs != 0) {
    for (NumExtHdrs--; NumExtHdrs >= 0; NumExtHdrs--)
      if (ExtHdr[NumExtHdrs] != NULL)
        bfree(ExtHdr[NumExtHdrs]);
    NumExtHdrs = 0;
    ExtendedStatus = 0;
  }
  CreateReceipt = FALSE;
}


#ifdef TEST
int LIBENTRY testmessage(void) {
  int           ConfNum;
  int           RetVal;
  long          ToRecNum;
  char          Body[256];
  msgbasetype   MsgBase;
  msgheadertype Header;
  pcbconftype   Conf;

  /* identify what conference number the message will be going into */
  ConfNum = 0;
  getconfrecord(ConfNum,&Conf);

  /* open the message base - NOTE:  IT IS LOCKED at this point, it would be     */
  /* best to get the message base closed as quickly as possible ... you can     */
  /* probably insert a couple of messages at once, but don't shoot for the      */
  /* moon, if you keep it open too long, other people, mail readers, importers, */
  /* etc, won't be able to save their messages                                  */
  if (openmessagebase(ConfNum,&Conf,&MsgBase,RDWRLOCK) == -1)
    return(-1);

  /* create a message body which is "packed" into 128 byte blocks, padded */
  /* with spaces on the end, and separated by "LineSeparator" characters. */

  memset(Body,' ',256);
  memcpy(Body,"This is a test",14);
  Body[14] = LineSeparator;

  /* here is an example of building a PCBoard-supported extended header, */
  /* in this case, the SUJECT extended header                            */
  buildextheader(EXTHDR_SUBJECT,"THE MESSAGE IS THE MESSAGE - DON'T SHOOT THE"
                                " MESSENGER",HDR_SUBJ);

  /* here is an example of building a generic header - PCBoard doesn't care, */
  /* it "hides" any extended headers it does not recognize internally.       */
  buildgenextheader("FIDOHDR","TEST OF FIDO HEADERS");

  /* create the message header block and store it in "Header" */
  makemsgheader(&Header,
                MSG_PBLC,         // public message
                15,               // 15 bytes in the message
                "12/30/94",       // date of the message
                "15:30",          // time of the message
                "SYSOP",          // TO: name
                0,                // REFER number
                0,                // READ date
                "",               // READ time
                ' ',              // READ status
                "THE MESSENGER",  // FROM name
                "THE MESSAGE IS THE MESSAGE",  // short subject
                "",               // Password
                FALSE);           // Echo Flag


  /* save the message */
  RetVal = savetomsgbase(ConfNum,&MsgBase,&Header,Body,0,0,NULL);

  /* close the message base */
  closemessagebase(&MsgBase);

  /* free up the extended headers we allocated */
  freehdrs();

  /* set the mail waiting flags for the users that messages have been */
  /* sent to                                                          */
  if (RetVal != -1) {
    /* get the TO: record number (0=disable), you'll probably use */
    /* finduser() to get the record number                        */
    ToRecNum = 1;
    putmessagewaiting(ConfNum,ToRecNum);
  }

  return(RetVal);
}
#endif
