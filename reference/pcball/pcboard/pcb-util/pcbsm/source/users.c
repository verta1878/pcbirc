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


#include <io.h>
#include <dir.h>
#include <dos.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

extern struct ffblk DTA;  /* declared in exist.c */

#ifndef DEMO
  bool    ShareStatus;
#endif


int      UsersFile;
int      TempFile;
unsigned ConfByteLen;
unsigned ExtConfLen;
long     InfHeaderSize;

UData    UsersData;
URead    UsersRead;
hdrtype  Header;
rectype  UsersRec;
apptype  AppHeader;

bool         AliasSupport = FALSE;
char         Alias[26];

bool         VerifySupport = FALSE;
char         Verify[26];

bool         AddressSupport = FALSE;
addresstypez Address;
addresstype  AddrRec;

bool          PasswordSupport = FALSE;
passwordtypez Password;
passwordtype  PwrdRec;

bool           StatsSupport;
callerstattype CallerStats;

bool           NotesSupport;
notestype      NotesRec;
notestypez     Notes;

bool           AccountSupport;
accounttype    Account;

bool           QwkSupport;
qwkconfigtype  QwkConfig;

#ifdef PCBSM
long        AliasOffset;         // needs to be non-static
long static VerifyOffset;
long static AddressOffset;
long static PasswordOffset;
long static StatsOffset;
long static NotesOffset;
long static AccountOffset;
long static QwkOffset;
long static TotalAppSize;
#endif

typedef enum {PSA_NONE=0,PSA_ALIAS,PSA_VERIFY,PSA_ADDRESS,PSA_PASSWORD,PSA_STATS,PSA_NOTES,PSA_ACCOUNT,PSA_QWKNET,PSA_END} psadeftype;

typedef struct {
  psadeftype Type;
  long       Offset;
  void      *Rec;
  int        Size;
} psaordertype;


#ifdef PCBSM
static psaordertype PSAorder[PSA_END];

char     BackFileName[40];
char     TempFileName[40];
char     BackInfFileName[40];
char     TempInfFileName[40];
unsigned static MsgPtrLen;
long far *MsgReadPtr;
char far *ConfReg;
char far *QwkConfFlags;
DOSFILE  DosUsersFile;
DOSFILE  DosUsersInfFile;
DOSFILE  DosTempFile;
char     CatchAll[220];
/*
long     UniqueIdForData;
long     UniqueIdForConf;
long     UniqueIdForMsgs;
long     UniqueIdForRec;
long     UniqueIdForAlias;
long     UniqueIdForVerify;
long     UniqueIdForAddress;
long     UniqueIdForPassword;
long     UniqueIdForStats;
long     UniqueIdForNotes;
long     UniqueIdForQwk;
long     UniqueIdForFlags;
*/
#endif


#if defined(PCBSM)
/********************************************************************
*
*  Function:  storepsaelement()
*
*  Desc    :  called by setpsaorder(), this function puts a single psa type
*             into the PSAorder list and keeps the list sorted.
*/

static void near pascal storepsaelement(psadeftype Type, long Offset, void *Rec, int Size) {
  psaordertype *p;

  for (p = PSAorder; p->Type != PSA_NONE && p->Offset < Offset; p++);

  if (p->Type != PSA_NONE)
    memmove(p+1,p,sizeof(psaordertype) * (int) (&PSAorder[PSA_END] - (p+1)));

  p->Type   = Type;
  p->Offset = Offset;
  p->Rec    = Rec;
  p->Size   = Size;
}


/********************************************************************
*
*  Function:  setpsaorder()
*
*  Desc    :  Creates a "sorted" list of the installed PSAs.  The list is
*             sorted by the order it is found on the hard disk (in users.inf).
*             Sorting the list allows us to optimize reads and writes of the
*             PSA information.
*/

void pascal setpsaorder(void) {
  memset(PSAorder,PSA_NONE,sizeof(PSAorder));

  if (AliasSupport)    storepsaelement(PSA_ALIAS,   AliasOffset,   Alias,        25);
  if (VerifySupport)   storepsaelement(PSA_VERIFY,  VerifyOffset,  Verify,       25);
  if (AddressSupport)  storepsaelement(PSA_ADDRESS, AddressOffset, &AddrRec,     sizeof(addresstype));
  if (PasswordSupport) storepsaelement(PSA_PASSWORD,PasswordOffset,&PwrdRec,     sizeof(passwordtype));
  if (StatsSupport)    storepsaelement(PSA_STATS,   StatsOffset,   &CallerStats, sizeof(callerstattype));
  if (NotesSupport)    storepsaelement(PSA_NOTES,   NotesOffset,   &NotesRec,    sizeof(notestype));
  if (AccountSupport)  storepsaelement(PSA_ACCOUNT, AccountOffset, &Account,     sizeof(accounttype));
  if (QwkSupport)      storepsaelement(PSA_QWKNET,  QwkOffset,     &QwkConfig,   sizeof(qwkconfigtype));
}


/********************************************************************
*
*  Function:  putpsas()
*
*  Desc    :  Using the RecordOffset, this function writes out all of the PSA
*             records at once.  It uses the sorted list (PSAorder) and attempts
*             to keep all of the high-level (stream I/O) PSA writes "together"
*             so that only a single low-level (direct I/O) write will be
*             necessary.  Of course, if the users.inf file has any TPAs that
*             are installed in between two PSAs this won't be possible because
*             buffered writes (using the DOS_x.LIB library) will not properly
*             skip over ranges in a file.  So a dosrewrind() is used to force
*             it to write to disk and re-position.
*/

static void near pascal putpsas(long RecordOffset) {
  psaordertype *p;
  long         Offset;
  long         NewOffset;

  Offset = dosfseek(&DosUsersInfFile,0,SEEK_CUR);

  for (p = PSAorder; p->Type != PSA_NONE; p++) {
    NewOffset = RecordOffset + p->Offset;
    if (NewOffset != Offset) {
      dosrewind(&DosUsersInfFile);
      dosfseek(&DosUsersInfFile,NewOffset,SEEK_SET);
      Offset = NewOffset;
    }

    switch (p->Type) {
      case PSA_ALIAS : padstr(Alias,' ',25);  break;
      case PSA_VERIFY: padstr(Verify,' ',25); break;
    }

    dosfwrite(p->Rec,p->Size,&DosUsersInfFile);  //lint !e534
    Offset += p->Size;

    if (p->Type == PSA_QWKNET) {
      dosfwrite(QwkConfFlags,PcbData.NumAreas * sizeof(char),&DosUsersInfFile);  //lint !e534
      Offset += PcbData.NumAreas * sizeof(char);
    }
  }
}


/********************************************************************
*
*  Function:  getpsas()
*
*  Desc    :  Using the RecordOffset, this function reads in all of the PSA
*             records at once.  It uses the sorted list (PSAorder) and attempts
*             to keep all of the high-level (stream I/O) PSA reads "together"
*             so that only a single low-level (direct I/O) read will be
*             necessary.  Of course, if the users.inf file has any TPAs that
*             are installed in between two PSAs this may not be possible
*             depending on how much was read into the buffer.
*/

static void near pascal getpsas(long RecordOffset) {
  psaordertype *p;
  long         Offset;
  long         NewOffset;

  Offset = dosfseek(&DosUsersInfFile,0,SEEK_CUR);

  for (p = PSAorder; p->Type != PSA_NONE; p++) {
    NewOffset = RecordOffset + p->Offset;
    if (NewOffset != Offset) {
      dosfseek(&DosUsersInfFile,NewOffset,SEEK_SET);
      Offset = NewOffset;
    }

    Offset += dosfread(p->Rec,p->Size,&DosUsersInfFile);

    switch (p->Type) {
      case PSA_ALIAS : Alias[25] = 0; stripright(Alias,' ');  break;
      case PSA_VERIFY: Verify[25] = 0; stripright(Verify,' '); break;
      case PSA_QWKNET: Offset += dosfread(QwkConfFlags,PcbData.NumAreas * sizeof(char),&DosUsersInfFile);
                       break;
    }
  }
}



void pascal usersalloc(void) {
  MsgPtrLen = (PcbData.NumAreas <= 40 ? 40*sizeof(long) : PcbData.NumAreas*sizeof(long));

  if ((MsgReadPtr = (long far *) farmalloc(MsgPtrLen)) == NULL)
    errorexittodos("insufficient memory for message pointers");

  ConfByteLen = (PcbData.NumAreas >> 3) + ((PcbData.NumAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;

  ExtConfLen = ConfByteLen - 5;

  /* only allocate memory for REG, EXP, USR, CON, MFL */
  if ((ConfReg = (char far *) farmalloc((long) ConfByteLen * 5)) == NULL)
    errorexittodos("insufficient memory for conference strings");

  if ((QwkConfFlags = (char far *) farmalloc(PcbData.NumAreas * sizeof(char))) == NULL)
    errorexittodos("insufficient memory for QWK/Net flags");

  memset(QwkConfFlags,0,PcbData.NumAreas * sizeof(char));
}

void pascal usersdealloc(void) {
  farfree(QwkConfFlags);
  farfree(ConfReg);
  farfree(MsgReadPtr);
}
#endif


#if defined(PCBSM) || defined(PCBDIAG)
/********************************************************************
*
*  Function: movestr()
*
*  Desc    : Copies Srce[] to Dest[] placing a NULL character at the end
*/

void pascal movestr(char *Dest, char *Srce, int Size) {
  memcpy(Dest,Srce,Size);
  Dest[Size] = 0;
  removespaces(Dest,Size);
}
#endif


#if defined(PCBSM)
/********************************************************************
*
*  Function: yymmddtostr()
*
*  Desc    : Converts a date of the form 'yymmdd' to the form 'mm-dd-yy'
*/

void pascal yymmddtostr(char Dest[], const char Srce[]) {
  _AH = Srce[2];
  _AL = Srce[3];
  _CH = Srce[4];
  _CL = Srce[5];
  _DH = Srce[0];
  _DL = Srce[1];
  Dest[0] = _AH;
  Dest[1] = _AL;
  Dest[3] = _CH;
  Dest[4] = _CL;
  Dest[6] = _DH;
  Dest[7] = _DL;
  Dest[5] = '-';
  Dest[2] = '-';
  Dest[8] = 0;
}


/********************************************************************
*
*  Function: strtoyymmdd()
*
*  Desc    : Converts a date of the form 'mm-dd-yy' to the form 'yymmdd'
*/

void pascal strtoyymmdd(char Dest[], const char Srce[]) {
  _AH = Srce[6];
  _AL = Srce[7];
  _CH = Srce[0];
  _CL = Srce[1];
  _DH = Srce[3];
  _DL = Srce[4];
  Dest[0] = _AH;
  Dest[1] = _AL;
  Dest[2] = _CH;
  Dest[3] = _CL;
  Dest[4] = _DH;
  Dest[5] = _DL;
}


/********************************************************************
*
*  Function: moveback()
*
*  Desc    : Copies the string from *Srce to *Dest, padding with spaces
*/

static void near pascal moveback(char *Dest, char *Srce, int Size) {
  for (; *Srce; Size--, Dest++, Srce++)
    if (Size)
      *Dest = *Srce;
    else
      return;

  for (; Size; Size--, Dest++)
    *Dest = ' ';
}


void pascal decryptusersrec(URead *Rec) {
  if (PcbData.Encrypt) {
    decrypt(Rec->City,62); /* 62 bytes = Rec->City thru Rec->HomeVoicePhone */
    decrypt(Rec->UserComment,60); /* 60 bytes = both comment fields */
  }
}


static void near pascal convertreadtoconf(char far *Conf, URead *Rec) {
  farmemcpy(&Conf[REG],Rec->RegConf   ,sizeof(UsersRead.RegConf));
  farmemcpy(&Conf[EXP],Rec->ExpRegConf,sizeof(UsersRead.ExpRegConf));
  farmemcpy(&Conf[USR],Rec->UsrRegConf,sizeof(UsersRead.UsrRegConf));
}


void pascal convertreadtoconfandmsgptrs(URead *Rec, char far *Conf) {
  int X;

  convertreadtoconf(Conf,Rec);
  for (X = 0; X < 40; X++)
    MsgReadPtr[X] = bassngltolong((char *) UsersRead.LastMsgRead[X]);
}


static void near pascal convertaddrtoaddrz(void) {
  movestr(Address.Street[0], AddrRec.Street[0], 50);
  movestr(Address.Street[1], AddrRec.Street[1], 50);
  movestr(Address.City     , AddrRec.City     , 25);
  movestr(Address.State    , AddrRec.State    , 10);
  movestr(Address.Zip      , AddrRec.Zip      , 10);
  movestr(Address.Country  , AddrRec.Country  , 15);
}


static void near pascal convertaddrztoaddr(void) {
  moveback(AddrRec.Street[0], Address.Street[0], 50);
  moveback(AddrRec.Street[1], Address.Street[1], 50);
  moveback(AddrRec.City     , Address.City     , 25);
  moveback(AddrRec.State    , Address.State    , 10);
  moveback(AddrRec.Zip      , Address.Zip      , 10);
  moveback(AddrRec.Country  , Address.Country  , 15);
}


static void near pascal convertpwrdtopwrdz(void) {
  movestr(Password.Previous[0], PwrdRec.Previous[0], 12);
  movestr(Password.Previous[1], PwrdRec.Previous[1], 12);
  movestr(Password.Previous[2], PwrdRec.Previous[2], 12);
  Password.LastChange   = PwrdRec.LastChange;
  Password.TimesChanged = PwrdRec.TimesChanged;
  Password.ExpireDate   = PwrdRec.ExpireDate;
}


static void near pascal convertpwrdztopwrd(void) {
  moveback(PwrdRec.Previous[0], Password.Previous[0], 12);
  moveback(PwrdRec.Previous[1], Password.Previous[1], 12);
  moveback(PwrdRec.Previous[2], Password.Previous[2], 12);
  PwrdRec.LastChange   = Password.LastChange;
  PwrdRec.TimesChanged = Password.TimesChanged;
  PwrdRec.ExpireDate   = Password.ExpireDate;
}


static void near pascal convertnotestonotesz(void) {
  int X;
  for (X = 0; X < 5; X++)
    movestr(Notes.Line[X],NotesRec.Line[X],60);
}


static void near pascal convertnotesztonotes(void) {
  int X;
  for (X = 0; X < 5; X++)
    moveback(NotesRec.Line[X],Notes.Line[X],60);
}


/********************************************************************
*
*  Function: convertreadtodata()
*
*  Desc    : Takes the data as read from the users file (in UsersRead) and
*            converts it to a format useable by PCBSM (in UsersData).
*/

void pascal convertreadtodata(void) {
  char   TempDate[9];

  #ifdef DEMO
  {
    int X;
    for (X = 0; X < sizeof(UsersRead.Reserved); X++) {
      if (UsersRead.Reserved[X] != 0 && UsersRead.Reserved[X] != ' ') {
        UsersFileModified = TRUE;
        memset(UsersRead.Reserved,0,sizeof(UsersRead.Reserved));
        break;
      }
    }
  }
  #endif

  movestr(UsersData.Name,           UsersRead.Name,              25);
  movestr(UsersData.City,           UsersRead.City,              24);
  movestr(UsersData.Password,       UsersRead.Password,          12);
  movestr(UsersData.BusDataPhone,   UsersRead.BusDataPhone,      13);
  movestr(UsersData.HomeVoicePhone, UsersRead.HomeVoicePhone,    13);
  movestr(UsersData.LastTimeOn,     UsersRead.LastTimeOn,         5);
  movestr(UsersData.UserComment,    UsersRead.UserComment,       30);
  movestr(UsersData.SysopComment,   UsersRead.SysopComment,      30);

  UsersData.Flags = UsersRead.Flags;
  memcpy(UsersData.Reserved,UsersRead.Reserved,sizeof(UsersRead.Reserved));

  yymmddtostr(TempDate,UsersRead.LastDateOn);
  UsersData.LastDateOn = datetojulian(TempDate);

  yymmddtostr(TempDate,UsersRead.DateLastDirRead);
  UsersData.DateLastDirRead = datetojulian(TempDate);

  if (memcmp(UsersRead.RegExpDate,"000000",6) != 0) {
    yymmddtostr(TempDate,UsersRead.RegExpDate);
    UsersData.RegExpDate = datetojulian(TempDate);
  } else {
    UsersData.RegExpDate = 0;
  }

  if (UsersRead.ExpertMode != 'Y')
    UsersRead.ExpertMode = 'N';
  if (UsersRead.DeleteFlag != 'Y')
    UsersRead.DeleteFlag = 'N';

  if (UsersRead.LastConference == 255)
    UsersData.LastConference = UsersRead.LastConf2;
  else
    UsersData.LastConference = UsersRead.LastConference;

  UsersData.ExpertMode     =(UsersRead.ExpertMode == 'Y' ? TRUE : FALSE);
  UsersData.Protocol       = UsersRead.Protocol;
  UsersData.SecurityLevel  = UsersRead.SecurityLevel;
  UsersData.NumTimesOn     = UsersRead.NumTimesOn;
  UsersData.PageLen        = UsersRead.PageLen;
  UsersData.PackedFlags    = UsersRead.PackedFlags;

  UsersData.NumUploads       = UsersRead.NumUploads;
  UsersData.NumDownloads     = UsersRead.NumDownloads;
  UsersData.ElapsedTimeOn    = UsersRead.ElapsedTimeOn;
  UsersData.DeleteFlag       =(UsersRead.DeleteFlag == 'Y' ? TRUE : FALSE);
  UsersData.ExpSecurityLevel = UsersRead.ExpSecurityLevel;

  UsersData.DailyDnldBytes = basdbletolong(UsersRead.DailyDnldBytes);
  UsersData.ulTotDnldBytes = basdbletolong(UsersRead.TotDnldBytes);
  UsersData.ulTotUpldBytes = basdbletolong(UsersRead.TotUpldBytes);
  UsersData.TotDnldBytes   = basdbletodouble(UsersRead.TotDnldBytes);
  UsersData.TotUpldBytes   = basdbletodouble(UsersRead.TotUpldBytes);

  // because the values used to be used only as unsigned long integers (but
  // stored as singled double precision floats), the new double might be a
  // negative value but it should be treated as positive, so convert it
  if (UsersData.TotDnldBytes < 0)
    UsersData.TotDnldBytes += 0xFFFFFFFFL + 1.0;
  if (UsersData.TotUpldBytes < 0)
    UsersData.TotUpldBytes += 0xFFFFFFFFL + 1.0;

  convertreadtoconfandmsgptrs(&UsersRead,ConfReg);

  stripright(Alias,' ');
  stripright(Verify,' ');
  convertaddrtoaddrz();
  convertpwrdtopwrdz();
  convertnotestonotesz();

/*
  UniqueIdForData     = evaluate(&UsersData,sizeof(UData));
  UniqueIdForRec      = evaluate(&UsersRec ,sizeof(rectype));
  UniqueIdForConf     = evaluate(ConfReg,ConfByteLen * 5);
  UniqueIdForMsgs     = evaluate(MsgReadPtr,MsgPtrLen);
  UniqueIdForAlias    = evaluate(Alias,strlen(Alias));
  UniqueIdForVerify   = evaluate(Verify,strlen(Verify));
  UniqueIdForAddress  = evaluate(&AddrRec,sizeof(addresstype));
  UniqueIdForPassword = evaluate(&PwrdRec,sizeof(passwordtype));
  UniqueIdForStats    = evaluate(&CallerStats,sizeof(callerstattype));
  UniqueIdForNotes    = evaluate(&NotesRec,sizeof(notestype));
  UniqueIdForQwk      = evaluate(&QwkConfig,sizeof(qwkconfigtype));
  UniqueIdForFlags    = evaluate(QwkConfFlags,PcbData.NumAreas * sizeof(char));

  if (UniqueIdForAddress == 0) {
    convertaddrztoaddr();
    UniqueIdForAddress = evaluate(&AddrRec,sizeof(addresstype));
  }

  if (UniqueIdForPassword == 0) {
    convertpwrdztopwrd();
    UniqueIdForPassword = evaluate(&PwrdRec,sizeof(passwordtype));
  }

  if (UniqueIdForNotes == 0) {
    convertnotesztonotes();
    UniqueIdForNotes = evaluate(&NotesRec,sizeof(notestype));
  }
*/
}


void pascal encryptusersrec(URead *Rec) {
  if (PcbData.Encrypt) {
    encrypt(Rec->City,62); /* 62 bytes = Rec->City thru Rec->HomeVoicePhone */
    encrypt(Rec->UserComment,60); /* 60 bytes = both comment fields */
  }
}


static void near pascal convertconftoread(URead *Rec, char far *Conf) {
  farmemcpy(Rec->RegConf   ,&Conf[REG],sizeof(UsersRead.RegConf));
  farmemcpy(Rec->ExpRegConf,&Conf[EXP],sizeof(UsersRead.ExpRegConf));
  farmemcpy(Rec->UsrRegConf,&Conf[USR],sizeof(UsersRead.UsrRegConf));
}


void pascal convertconfandmsgptrstoread(URead *Rec, char far *Conf) {
  int X;

  convertconftoread(Rec,Conf);
  for (X = 0; X < 40; X++)
    longtobassngl((char *) Rec->LastMsgRead[X],MsgReadPtr[X]);
}


/********************************************************************
*
*  Function: convertdatatoread()
*
*  Desc    : Converts the PCBSM data (in UsersData) back to a format to be
*            stored on disk (in UsersRead) and at the same time checks to see
*            if the data has changed since it was read in.
*
*  Returns : TRUE if the data has changed, otherwise FALSE
*/

int pascal convertdatatoread(char *First, char *Second) {
  char   Name[25];
  char   *TempDate;
//bool   Changed;

  stripright(Alias,' ');
  stripright(Verify,' ');
  convertaddrztoaddr();
  convertpwrdztopwrd();
  convertnotesztonotes();

/*
  Changed = FALSE;
  if (UniqueIdForData != evaluate(&UsersData,sizeof(UData)))
    Changed = TRUE;
  else if (UniqueIdForRec  != evaluate(&UsersRec,sizeof(rectype)))
    Changed = TRUE;
  else if (UniqueIdForConf != evaluate(ConfReg,ConfByteLen * 5))
    Changed = TRUE;
  else if (UniqueIdForMsgs != evaluate(MsgReadPtr,MsgPtrLen))
    Changed = TRUE;
  else if (UniqueIdForAlias != evaluate(Alias,strlen(Alias)))
    Changed = TRUE;
  else if (UniqueIdForVerify != evaluate(Verify,strlen(Verify)))
    Changed = TRUE;
  else if (UniqueIdForAddress != evaluate(&AddrRec,sizeof(addresstype)))
    Changed = TRUE;
  else if (UniqueIdForPassword != evaluate(&PwrdRec,sizeof(passwordtype)))
    Changed = TRUE;
  else if (UniqueIdForStats != evaluate(&CallerStats,sizeof(callerstattype)))
    Changed = TRUE;
  else if (UniqueIdForNotes != evaluate(&NotesRec,sizeof(notestype)))
    Changed = TRUE;
  else if (UniqueIdForQwk != evaluate(&QwkConfig,sizeof(qwkconfigtype)))
    Changed = TRUE;
  else if (QwkSupport && UniqueIdForFlags != evaluate(QwkConfFlags,PcbData.NumAreas*sizeof(char)))
    Changed = TRUE;
*/

  memcpy(Name,UsersRead.Name,25);

  moveback(UsersRead.Name,           UsersData.Name,              25);
  moveback(UsersRead.City,           UsersData.City,              24);
  moveback(UsersRead.Password,       UsersData.Password,          12);
  moveback(UsersRead.BusDataPhone,   UsersData.BusDataPhone,      13);
  moveback(UsersRead.HomeVoicePhone, UsersData.HomeVoicePhone,    13);
  moveback(UsersRead.LastTimeOn,     UsersData.LastTimeOn,         5);
  moveback(UsersRead.UserComment,    UsersData.UserComment,       30);
  moveback(UsersRead.SysopComment,   UsersData.SysopComment,      30);

  #ifdef DEMO
    memset(UsersRead.Reserved,0,sizeof(UsersRead.Reserved));
  #else
    memcpy(UsersRead.Reserved,UsersData.Reserved,sizeof(UsersRead.Reserved));
  #endif

  UsersRead.Flags = UsersData.Flags;

  TempDate = juliantodate(UsersData.LastDateOn);
  strtoyymmdd(UsersRead.LastDateOn,TempDate);

  TempDate = juliantodate(UsersData.DateLastDirRead);
  strtoyymmdd(UsersRead.DateLastDirRead,TempDate);

  if (UsersData.RegExpDate != 0) {
    TempDate = juliantodate(UsersData.RegExpDate);
    strtoyymmdd(UsersRead.RegExpDate,TempDate);
  } else {
    memcpy(UsersRead.RegExpDate,"000000",6);
  }

  UsersRead.LastConf2      = UsersData.LastConference;
  UsersRead.LastConference = (UsersData.LastConference > 255 ? 255 : UsersData.LastConference);

  UsersRead.ExpertMode     =(UsersData.ExpertMode ? 'Y' : 'N');
  UsersRead.Protocol       = UsersData.Protocol;
  UsersRead.SecurityLevel  = UsersData.SecurityLevel;
  UsersRead.NumTimesOn     = UsersData.NumTimesOn;
  UsersRead.PageLen        = UsersData.PageLen;
  UsersRead.PackedFlags    = UsersData.PackedFlags;

  UsersRead.NumUploads       = UsersData.NumUploads;
  UsersRead.NumDownloads     = UsersData.NumDownloads;
  UsersRead.ElapsedTimeOn    = UsersData.ElapsedTimeOn;
  UsersRead.DeleteFlag       =(UsersData.DeleteFlag ? 'Y' : 'N');
  UsersRead.ExpSecurityLevel = UsersData.ExpSecurityLevel;

  longtobasdble((char *) UsersRead.DailyDnldBytes,UsersData.DailyDnldBytes);
//longtobasdble((char *) UsersRead.TotDnldBytes  ,UsersData.TotDnldBytes);
//longtobasdble((char *) UsersRead.TotUpldBytes  ,UsersData.TotUpldBytes);
  doubletobasdble((char *) UsersRead.TotDnldBytes,UsersData.TotDnldBytes);
  doubletobasdble((char *) UsersRead.TotUpldBytes,UsersData.TotUpldBytes);

  convertconfandmsgptrstoread(&UsersRead,ConfReg);

  if (memcmp(Name,UsersRead.Name,25)) {
    *First  = Name[0];
    *Second = UsersRead.Name[0];
    return(TRUE);
  }

  *First  = 0;
  *Second = 0;

//return(Changed);
  return(FALSE);
}
#endif


/********************************************************************
*
*  Function: openusersfile()
*
*  Desc    : Opens the users file returns -1 if an error occurs
*
*/

int pascal openusersfile(void) {
  if (fileexist(PcbData.UsrFile) == 255) {
    if ((UsersFile = doscreatecheck(PcbData.UsrFile,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL)) != -1) {
      memset(&UsersRead,0,sizeof(URead));
      memcpy(UsersRead.Name,"SYSOP                    ",25);
      doswrite(UsersFile,&UsersRead,sizeof(URead));   //lint !e534
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.AutoBox = TRUE;
      MsgData.Msg1    = "ERROR!  Need to create User Info File";
      MsgData.Line1   = 18;
      MsgData.Color1  = Colors[HEADING];
      showmessage();
      close(UsersFile);
      return(-1);
    }
    return(UsersFile);
  } else
    return(UsersFile = dosopencheck(PcbData.UsrFile,OPEN_RDWR|OPEN_DENYNONE));
}


#ifdef PCBSM
/********************************************************************
*
*  Function: openusersinffile()
*
*  Desc    : Opens the users.inf file returns -1 if an error occurs
*
*/

int pascal openusersinffile(char *FileName) {
  bool NeedToUpdate;
  unsigned TpaStaticSize;
  unsigned TpaDynamicSize;
  unsigned TpaNumConf;
  unsigned X;
  long     Size;

  if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE,&DosUsersInfFile) != -1) {
    if (dosfread(&Header,sizeof(hdrtype),&DosUsersInfFile) != sizeof(hdrtype)) {
      Size = dosfseek(&DosUsersInfFile,0,SEEK_END);
      dosfclose(&DosUsersInfFile);
      /* maybe we just created a new file?  if so, delete it */
      if (Size == 0)
        unlink(FileName);
      return(-1);
    }
    NeedToUpdate = (Header.SizeOfRec != sizeof(rectype) || Header.SizeOfConf != calcconfsize(PcbData.NumAreas));
    if (! NeedToUpdate && Header.NumOfApps != 0) {
      TpaStaticSize = 0;
      TpaDynamicSize = 0;
      for (X = 0; X < Header.NumOfApps; X++) {
        if (dosfread(&AppHeader,sizeof(apptype),&DosUsersInfFile) != sizeof(apptype))
          break;
        TpaStaticSize  += AppHeader.SizeOfRec;
        TpaDynamicSize += AppHeader.SizeOfConfRec;
        if (AppHeader.Version == 10 && memcmp(AppHeader.Name,"PCB",3) == 0) {
          if (strcmp(&AppHeader.Name[3],"ALIAS") == 0 && AppHeader.SizeOfRec == 25) {
            AliasSupport = TRUE;
            AliasOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"VERIFY") == 0 && AppHeader.SizeOfRec == 25) {
            VerifySupport = TRUE;
            VerifyOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"ADDRESS") == 0 && AppHeader.SizeOfRec == sizeof(addresstype)) {
            AddressSupport = TRUE;
            AddressOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"PASSWORD") == 0 && AppHeader.SizeOfRec == sizeof(passwordtype)) {
            PasswordSupport = TRUE;
            PasswordOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"STATS") == 0 && AppHeader.SizeOfRec == sizeof(callerstattype)) {
            StatsSupport = TRUE;
            StatsOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"NOTES") == 0 && AppHeader.SizeOfRec == sizeof(notestype)) {
            NotesSupport = TRUE;
            NotesOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"ACCOUNT") == 0 && AppHeader.SizeOfRec == sizeof(accounttype)) {
            AccountSupport = TRUE;
            AccountOffset  = AppHeader.Offset;
          }
          if (strcmp(&AppHeader.Name[3],"QWKNET") == 0 && AppHeader.SizeOfRec == sizeof(qwkconfigtype)) {
            QwkSupport = TRUE;
            QwkOffset  = AppHeader.Offset;
          }
        }
      }
      if (TpaDynamicSize != 0) { /* only check TpaNumConf if we found some */
        TpaNumConf = (unsigned) ((Header.TotalRecSize - Header.SizeOfRec - Header.SizeOfConf - TpaStaticSize) / TpaDynamicSize);
        NeedToUpdate = (TpaNumConf != PcbData.NumAreas);
      }
    }
    setpsaorder();
    if (NeedToUpdate) {
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.AutoBox = TRUE;
      MsgData.Msg1    = "ERROR!  Need to change Conference Allocations in User Info File";
      MsgData.Line1   = 18;
      MsgData.Color1  = Colors[HEADING];
      showmessage();
      dosfclose(&DosUsersInfFile);
      return(-1);
    }
    InfHeaderSize = ((long) Header.NumOfApps * sizeof(apptype)) + sizeof(hdrtype);
    TotalAppSize = Header.TotalRecSize - (sizeof(rectype)+((long) 2*ConfByteLen)+((long)3*ExtConfLen)+(ExtConfLen > 0 ? (long) (PcbData.NumAreas-40)*sizeof(long) : 0));
  }
  return(0);
}


/********************************************************************
*
*  Function: convertreadtostr()
*
*  Desc    : Converts the entire UsersRead data into string format for use by
*            the Search for Text function.
*/

void pascal convertreadtostr(void) {
  char Temp[41];

  CatchAll[0] = 0;
  movestr(Temp, UsersRead.Name,              25);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.City,              24);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.Password,          12);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.BusDataPhone,      13);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.HomeVoicePhone,    13);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.LastDateOn,         8);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.LastTimeOn,         5);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.UserComment,       30);  strcat(CatchAll,Temp);
  movestr(Temp, UsersRead.SysopComment,      30);  strcat(CatchAll,Temp);
}

/********************************************************************
*
*  Function: closeusersfile()
*
*  Desc    : Closes the users file.  If operation was NOT aborted then the
*            tempfile is renamed to users and users changes to backup.
*/

void pascal closeusersfile(bool Aborted, int Buffered, bool UserInfFile) {
  switch (Buffered) {
    case BUFFERED   : dosfseek(&DosUsersFile,0,SEEK_END);  /* update slaves */
                      dosfclose(&DosUsersFile);
                      break;
    case NONBUFFERED: doslseek(UsersFile,0,SEEK_END);
                      dosclose(UsersFile);
                      break;
  }

  if (UserInfFile) {
    dosfseek(&DosUsersInfFile,0,SEEK_END);
    dosfclose(&DosUsersInfFile);
  }

  if (! Aborted) {
    unlink(BackFileName);
    rename(PcbData.UsrFile,BackFileName );       //lint !e534
    rename(TempFileName   ,PcbData.UsrFile);     //lint !e534
    if (UserInfFile) {
      unlink(BackInfFileName);
      rename(PcbData.InfFile,BackInfFileName);   //lint !e534
      rename(TempInfFileName,PcbData.InfFile);   //lint !e534
    }
  }
}


/********************************************************************
*
*  Function: lockusersfile()
*
*  Desc    : Attempts to "lock" the users file by opening it up in exclusive
*            read/write deny-all mode.
*
*  Returns : -1 if lock failed, otherwise number of records in users file
*/

long pascal lockusersfile(char *HeaderStr, int Buffered) {
  bool Error;

  if (HeaderStr[0] > 0) {
    clscolor(Colors[OUTBOX]);
    generalscreen(MainHead1,HeaderStr);
  }
  fastprint(3,4,"Attempting to LOCK the Users file...",Colors[DISPLAY]);

  Error = FALSE;

  switch(Buffered) {
    case BUFFERED   : if (dosfopen(PcbData.UsrFile,OPEN_READ|OPEN_DENYRDWR,&DosUsersFile) == -1)
                        Error = TRUE;
                      else
                        UsersFile = DosUsersFile.handle;
                      break;
    case NONBUFFERED: if ((UsersFile = dosopen(PcbData.UsrFile,OPEN_READ|OPEN_DENYRDWR)) == -1)
                        Error = TRUE;
                      break;
  }

  if (Error) {
    fastprintmove(3,6,"LOCK FAILED:  All nodes must be down before proceeding",Colors[HEADING]);
    mydelay(400);
    return(-1);
  }

  fastprintmove(3,6,"Successful LOCK:  All nodes will be forced to stay down during operation",Colors[DISPLAY]);
  fastprint(4,22," Press [Esc] to cancel the procedure and leave the users file as it was ",Colors[DESC]);
  return(numrecs(UsersFile,sizeof(URead)));
}


int pascal readusersinffile(void) {
  bool DeletedRecord;
  long Offset;
  char *Str;
  char *p;

  if ((p = (char *) mallochk(ConfByteLen)) == NULL)
    return(-1);

  DeletedRecord = FALSE;

  if (memcmp(UsersRead.Name,"DELETED RECORD - IGNORE  ",25) == 0 ||
      memcmp(UsersRead.Name,"                         ",25) == 0 ) {
    // this isn't really an error, but we don't want to attempt to read the
    // record either, so get out now, but don't show it as an error either
    DeletedRecord = TRUE;
    goto error;
  }

  if (UsersRead.RecNum < 1) {
    Str = "USERS.INF POINTER IS BAD";
    goto error;
  }

  Offset = ((UsersRead.RecNum-1) * Header.TotalRecSize) + InfHeaderSize;

  dosfseek(&DosUsersInfFile,Offset,SEEK_SET);
  if (dosfread(&UsersRec,sizeof(rectype),&DosUsersInfFile) != sizeof(rectype)) {
    Str = "CANNOT READ USERS.INF FILE";
    goto error;
  }

  if (memcmp(UsersRead.Name,UsersRec.Name,sizeof(UsersRead.Name)) != 0) {
    Str = "NAME MISMATCH IN USERS.INF FILE";
    goto error;
  }

  /* get the MAIL flags */
  if (dosfread(p,ConfByteLen,&DosUsersInfFile) != ConfByteLen) {
    Str = "CANNOT READ USERS.INF FILE";
    goto error;
  } else farmemcpy(&ConfReg[MFL],p,ConfByteLen);

  /* now get the CONFERENCE SYSOP flags */
  if (dosfread(p,ConfByteLen,&DosUsersInfFile) != ConfByteLen) {
    Str = "CANNOT READ USERS.INF FILE";
    goto error;
  } else farmemcpy(&ConfReg[CON],p,ConfByteLen);

  #ifndef DEMO
  {
    unsigned X;

    if (ExtConfLen > 0) {
      /* get the extended REGISTERED flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen) {
        Str = "CANNOT READ USERS.INF FILE";
        goto error;
      } else farmemcpy(&ConfReg[REG+5],p,ExtConfLen);
      /* get the extended EXPIRED flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen) {
        Str = "CANNOT READ USERS.INF FILE";
        goto error;
      } else farmemcpy(&ConfReg[EXP+5],p,ExtConfLen);
      /* get the extended USER SCAN flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen) {
        Str = "CANNOT READ USERS.INF FILE";
        goto error;
      } else farmemcpy(&ConfReg[USR+5],p,ExtConfLen);

      /* get extended LAST MSG READ pointers */
      for (X = 40; X < PcbData.NumAreas; X++) {
/*
        if (dosfread(p,sizeof(long),&DosUsersInfFile) != sizeof(long)) {
          Str = "CANNOT READ USERS.INF FILE";
          goto error;
        } else farmemcpy(&MsgReadPtr[X],p,sizeof(long));
*/
        if (dosfread(&MsgReadPtr[X],sizeof(long),&DosUsersInfFile) != sizeof(long)) {
          Str = "CANNOT READ USERS.INF FILE";
          goto error;
        }
      }
    }
  }
  #endif

  getpsas(Offset);
  free(p);
  return(0);

error:
  free(p);
  if (! DeletedRecord) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Save    = TRUE;
    MsgData.Msg1    = Str;
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    showmessage();
  }
  fmemset(&ConfReg[MFL],0,ConfByteLen);
  fmemset(&ConfReg[CON],0,ConfByteLen);
  fmemset(&ConfReg[REG+5],0,ExtConfLen);
  fmemset(&ConfReg[EXP+5],0,ExtConfLen);
  fmemset(&ConfReg[USR+5],0,ExtConfLen);

  #ifndef DEMO
    if (PcbData.NumAreas > 40)
      fmemset(&MsgReadPtr[40],0,(PcbData.NumAreas-40)*sizeof(long));
  #endif

  Alias[0] = 0;
  Verify[0] = 0;
  memset(&AddrRec,0,sizeof(addresstype));
  memset(&Address,0,sizeof(addresstypez));
  memset(&PwrdRec,0,sizeof(passwordtype));
  memset(&Password,0,sizeof(passwordtypez));
  memset(&CallerStats,0,sizeof(callerstattype));
  memset(&NotesRec,0,sizeof(notestype));
  memset(&Account,0,sizeof(accounttype));
  memset(&QwkConfig,0,sizeof(qwkconfigtype));
  memset(QwkConfFlags,0,PcbData.NumAreas * sizeof(char));
  // Don't return a -1 if it was simply a deleted record, we need to allow
  // the PROCESS.C module to continue to operate as it encounteres deleted
  // records but a -1 would cause it to abort the process.
  if (DeletedRecord)
    return(0);
  return(-1);
}


/********************************************************************
*
*  Function: readusersrecord()
*
*  Desc    : Reads the current record into the UsersRead buffer.
*
*/

static int near pascal readusersrecord(void) {
  if (readcheck(UsersFile,&UsersRead,sizeof(URead)) == (unsigned) -1)
    return(-1);
  #ifndef DEMO
    decryptusersrec(&UsersRead);
  #endif
  return(readusersinffile());
}


/********************************************************************
*
*  Function: readdosusersrecord()
*
*  Desc    : Reads the current record into the UsersRead buffer.
*
*/

int pascal readdosusersrecord(void) {
  if (dosfread((char *)&UsersRead,sizeof(URead),&DosUsersFile) != sizeof(URead))
    return(-1);
  #ifndef DEMO
    decryptusersrec(&UsersRead);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: readusersfile()
*
*  Desc    : Reads the record pointed to by RecNum into the UsersRead buffer.
*/

int pascal readusersfile(long RecNo) {
  RecNo--;
  doslseek(UsersFile,RecNo * sizeof(URead),SEEK_SET);
  return(readusersrecord());
}


/********************************************************************
*
*  Function: readpackinfo()
*
*  Desc    : Reads the current record in the users file into the UsersRead
*            buffer and the converts only the information necessary to the
*            Pack procedure into a format useable by PCBSM.
*
*  Returns : -1 if an error occured on the read otherwise 0
*/

int pascal readpackinfo(void) {
  char TempDate[9];

  if (readdosusersrecord() == -1)
    return(-1);

  movestr(UsersData.Name,UsersRead.Name,25);
  UsersData.DeleteFlag    = (UsersRead.DeleteFlag == 'Y' ? TRUE : FALSE);
  UsersData.SecurityLevel =  UsersRead.SecurityLevel;

  yymmddtostr(TempDate,UsersRead.LastDateOn);
  UsersData.LastDateOn = datetojulian(TempDate);

  if (memcmp(UsersRead.RegExpDate,"000000",6) != 0) {
    yymmddtostr(TempDate,UsersRead.RegExpDate);
    UsersData.RegExpDate = datetojulian(TempDate);
  } else {
    UsersData.RegExpDate = 0;
  }
  return(0);
}

/********************************************************************
*
*  Function:  lockuser()
*
*  Desc    :  Locks the space at the end of the file where the NEXT NEW
*             record will be written
*
*  Note    :  doslockcheck() is *not* used because we don't want to WAIT for
*             the record to become available if it isn't already - instead we
*             want to error out immediately and try for the NEXT record after
*             the end
*
*  Returns :  Upon successfully locking NEXT record in the USERS file it
*             returns the record number of the NEXT record.
*/

static long near pascal lockuser(void) {
  long Offset;
  long RecNo;

  /* calculate a RecNo first (to take care of round off) and then    */
  /* calculate the offset into the file from the record number below */
  Offset = doslseek(UsersFile,0,SEEK_END);
  RecNo  = (Offset / sizeof(URead)) + 1;

top:
  #ifndef DEMO
  {
    long Temp;

    if (ShareStatus) {
      Offset = (RecNo - 1) * sizeof(URead);
      while (lock(UsersFile,Offset,sizeof(URead)) == -1) {
        Offset += sizeof(UsersRead);
        RecNo++;
      }
      /* We've LOCKED what we think to be the NEXT NEW RECORD in the file */
      /* find out the offset to the end of the file and if it is now      */
      /* greater than what we thought was the end the start unlock what   */
      /* we had and go start all over again from the new end of file      */
      Temp = doslseek(UsersFile,0,SEEK_END);
      if (Temp > Offset) {
        unlock(UsersFile,Offset,sizeof(URead));
        RecNo++;
        goto top;
      }
    }
  }
  #endif
  return(RecNo);
}


/********************************************************************
*
*  Function:  unlockuser()
*
*  Desc    :  unlocks the user record
*/

#ifndef DEMO
static void near pascal unlockuser(long RecNo) {
  long Offset;

  if (ShareStatus) {
    Offset = (RecNo-1) * sizeof(URead);
    unlock(UsersFile,Offset,sizeof(URead));
  }
}
#endif


/********************************************************************
*
*  Function:  lockuserinf()
*
*  Desc    :  Locks the space at the end of the file where the NEXT NEW
*             record will be written
*
*  Note    :  doslockcheck() is *not* used because we don't want to WAIT for
*             the record to become available if it isn't already - instead we
*             want to error out immediately and try for the NEXT record after
*             the end
*
*  Returns :  Upon successfully locking NEXT record in the USERS.INF file it
*             returns the record number of the NEXT record.
*/

static long near pascal lockuserinf(void) {
  long Offset;
  long RecNo;

  /* calculate a RecNo first (to take care of round off) and then    */
  /* calculate the offset into the file from the record number below */
  Offset = dosfseek(&DosUsersInfFile,0,SEEK_END);
  RecNo  = ((Offset - InfHeaderSize) / Header.TotalRecSize) + 1;

top:
  #ifndef DEMO
  {
    long Temp;
    if (ShareStatus) {
      Offset = ((RecNo - 1) * Header.TotalRecSize) + InfHeaderSize;
      while (lock(DosUsersInfFile.handle,Offset,Header.TotalRecSize) == -1) {
        Offset += Header.TotalRecSize;
        RecNo++;
      }
      /* We've LOCKED what we think to be the NEXT NEW RECORD in the file */
      /* find out the offset to the end of the file and if it is now      */
      /* greater than what we thought was the end the start unlock what   */
      /* we had and go start all over again from the new end of file      */
      Temp = dosfseek(&DosUsersInfFile,0,SEEK_END);
      if (Temp > Offset) {
        unlock(DosUsersInfFile.handle,Offset,Header.TotalRecSize);
        RecNo++;
        goto top;
      }
    }
  }
  #endif
  return(RecNo);
}


/********************************************************************
*
*  Function:  unlockuserinf()
*
*  Desc    :  unlocks the user record of the users.inf file
*/

#ifndef DEMO
static void near pascal unlockuserinf(long RecNo) {
  long Offset;

  if (ShareStatus) {
    Offset = ((RecNo-1) * Header.TotalRecSize) + InfHeaderSize;
    unlock(DosUsersInfFile.handle,Offset,Header.TotalRecSize);
  }
}
#endif


int pascal writeusersinffile(bool ClearApp) {
  char *p;
  long Size;
  long Offset;
  bool FixUserInf;

  p          = NULL;
  FixUserInf = FALSE;
  dosflush(&DosUsersInfFile);

  /* check the LOWER bounds of the record numbers */
  if (UsersRead.RecNum < 1)
    FixUserInf = TRUE;

  if (! ClearApp && ! FixUserInf) {
    /* only check the UPPER bounds if we are NOT adding a new user record */
    if (UsersRead.RecNum > (dosfseek(&DosUsersInfFile,0,SEEK_END) - InfHeaderSize) / Header.TotalRecSize)
      FixUserInf = TRUE;
  }

  if (FixUserInf) {
    UsersRead.RecNum = lockuserinf();
    ClearApp = TRUE;  /* force it to write the TPA records too! */
  }

  dosflush(&DosUsersInfFile);
  memcpy(UsersRec.Name,UsersRead.Name,sizeof(UsersRead.Name));
  Offset = ((UsersRead.RecNum-1) * Header.TotalRecSize) + InfHeaderSize;

  dosfseek(&DosUsersInfFile,Offset,SEEK_SET);
  if (dosfwrite(&UsersRec,sizeof(rectype),&DosUsersInfFile) == -1)
    goto cantwrite;

  if ((p = (char *) mallochk(ConfByteLen)) == NULL)
    goto cantwrite;

  /* write the MAIL flags */
  farmemcpy(p,&ConfReg[MFL],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&DosUsersInfFile) == -1)
    goto cantwrite;

  /* write the CONFERENCE SYSOP flags */
  farmemcpy(p,&ConfReg[CON],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&DosUsersInfFile) == -1)
    goto cantwrite;

  #ifndef DEMO
  {
    unsigned X;

    if (ExtConfLen > 0) {
      /* write extended REGISTERED flags */
      farmemcpy(p,&ConfReg[REG+5],ExtConfLen);
      if (dosfwrite(p,ExtConfLen,&DosUsersInfFile) == -1)
        goto cantwrite;

      /* write extended EXPIRED flags */
      farmemcpy(p,&ConfReg[EXP+5],ExtConfLen);
      if (dosfwrite(p,ExtConfLen,&DosUsersInfFile) == -1)
        goto cantwrite;

      /* write extended USER SCAN flags */
      farmemcpy(p,&ConfReg[USR+5],ExtConfLen);
      if (dosfwrite(p,ExtConfLen,&DosUsersInfFile) == -1)
        goto cantwrite;

      /* write extended LAST MSG READ pointers */
      for (X = 40; X < PcbData.NumAreas; X++) {
        farmemcpy(p,&MsgReadPtr[X],sizeof(long));
        if (dosfwrite(p,sizeof(long),&DosUsersInfFile) == -1)
          goto cantwrite;
      }
    }
  }
  #endif

  if (ClearApp && TotalAppSize != 0) {
    free(p);
    if ((p = (char *) mallochk(4096)) == NULL)
      goto cantwrite;
    memset(p,0,4096);
    for (Size = TotalAppSize; Size > 4096; Size -= 4096) {
      if (dosfwrite(p,4096,&DosUsersInfFile) == -1)
        goto cantwrite;
    }
    if (dosfwrite(p,(unsigned) Size,&DosUsersInfFile) == -1)
      goto cantwrite;
  }

  putpsas(Offset);

  dosflush(&DosUsersInfFile);
  free(p);
  #ifndef DEMO
    if (FixUserInf)
      unlockuserinf(UsersRead.RecNum);
  #endif
  return(0);

cantwrite:
  dosflush(&DosUsersInfFile);
  if (p != NULL)
    free(p);
  #ifndef DEMO
    if (FixUserInf)
      unlockuserinf(UsersRead.RecNum);
  #endif
  return(-1);
}


/********************************************************************
*
*  Function: writeusersfile()
*
*  Desc    : Attempts to lock the users file and then save the contents of the
*            UsersRead buffer to disk.
*
*  Returns : TRUE if error in locking record, otherwise FALSE
*/

int pascal writeusersfile(long RecNo, bool LockRecord, bool ClearApp) {
  long     Offset;
  int      RetVal;

  RecNo--;
  Offset = RecNo * sizeof(URead);

  #ifndef DEMO
  /* set the Dirty Flag so that if a user was online  */
  /* he will update himself as he logs off the system */
  UsersRead.PackedFlags.Dirty = TRUE;

  if (LockRecord && ShareStatus) {
    if (doslockcheck(UsersFile,Offset,sizeof(URead)) == -1) {
      beep();
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.AutoBox = TRUE;
      MsgData.Save    = TRUE;
      MsgData.Msg1    = "Error writing user record .. record is currently in use!";
      MsgData.Line1   = 18;
      MsgData.Color1  = Colors[HEADING];
      return(-1);
    }
  }
  #endif

  if ((RetVal = writeusersinffile(ClearApp)) != -1) {
    doslseek(UsersFile,Offset,SEEK_SET);
    encryptusersrec(&UsersRead);
    RetVal = writecheck(UsersFile,&UsersRead,sizeof(URead));
  }

  #ifndef DEMO
  if (LockRecord && ShareStatus)
    unlock(UsersFile,Offset,sizeof(URead));
  #endif

  if (RetVal == -1) {
    beep();
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Save    = TRUE;
    MsgData.Msg1    = "Error writing user record";
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    return(-1);
  }
  return(0);
}



/********************************************************************
*
*  Function:  adduserrecord()
*
*  Desc    :  Adds a record into both the USERS file and the USERS.INF file
*
*             For handshaking on a network it starts by attempting to lock
*             the record just beyond the end of the USERS file and will retry
*             the lock it until it is finally given permission to do so.
*
*             At that point it knows that it has permission to add a record
*             on the end of the file.  If it has permission to write to the
*             USERS file it assumes it's okay to write to the USERS.INF file
*             as well.
*
*  Returns :  The record number of the new record in the USERS file.
*/

long pascal adduserrecord(void) {
  long UserRecNo;

  UserRecNo = lockuser();
  UsersRead.RecNum = 0; /* force it to lock and add a new record in USERS.INF */
  writeusersfile(UserRecNo,FALSE,TRUE);  //lint !e534
  #ifndef DEMO
    unlockuser(UserRecNo);
  #endif
  return(UserRecNo);
}
#endif


/********************************************************************
*
*  Function: numrecs()
*
*  Desc    : Determines the number of records in the users file
*
*  Returns : The number of records (starting with 1)
*/

long pascal numrecs(const int Handle, const int RecSize) {
  #ifdef DEMO
    return(min(10,doslseek(Handle,0,SEEK_END) / RecSize));
  #else
    return(doslseek(Handle,0,SEEK_END) / RecSize);
  #endif
}


#ifdef PCBSM
/********************************************************************
*
*  Function: copysysoprecord()
*
*  Desc    : Copies the first record from the UsersFile to the TempFile
*            straight across without acting on it.
*
*/

int pascal copysysoprecord(int UsersBuffered, int TempBuffered) {
  int RetVal = -1;

  switch (UsersBuffered) {
    case BUFFERED   : dosrewind(&DosUsersFile);
                      if (readdosusersrecord() == -1)
                        return(-1);
                      break;
    case NONBUFFERED: doslseek(UsersFile,0,SEEK_SET);
                      if (readcheck(UsersFile,&UsersRead,sizeof(URead)) == (unsigned) -1)
                        return(-1);
                      #ifndef DEMO
                        decryptusersrec(&UsersRead);
                      #endif
                      break;
  }

  encryptusersrec(&UsersRead);

  switch(TempBuffered) {
    case BUFFERED   : dosrewind(&DosTempFile);
                      RetVal = dosfwrite((char *)&UsersRead,sizeof(URead),&DosTempFile);
                      break;
    case NONBUFFERED: doslseek(TempFile,0,SEEK_SET);
                      RetVal = writecheck(TempFile,&UsersRead,sizeof(URead));
                      break;
  }
  return(RetVal);
}


static void near pascal popup(char *Str1, char *Str2) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Color2  = Colors[HEADING];
  MsgData.Msg1    = Str1;
  if (Str2 == NULL) {
    MsgData.Line1 = 18;
  } else {
    MsgData.Line1 = 17;
    MsgData.Line2 = 18;
    MsgData.Msg2  = Str2;
  }
  showmessage();
}


static void near pascal insufdiskspace(void) {
  popup("Insufficient disk space to store backup copy","of the USERS and USERS.INF files.");
}


/********************************************************************
*
*  Function: createtemp()
*
*  Desc    : Creates the temporary file
*
*/

int pascal createtemp(int PreAlloc, int Buffered, bool UserInfo) {
  unsigned long NumUserRecs;
  unsigned long PreAllocSize;
  unsigned long UsersInfSize;
  unsigned long SizeNeeded;

  NumUserRecs  = numrecs(UsersFile,sizeof(URead));
  PreAllocSize = (NumUserRecs * sizeof(URead)) - 1;

  if (fileexist(PcbData.InfFile) == 255) {
    popup("USERS.INF does not exist",NULL);
    return(-1);
  }

  UsersInfSize = DTA.ff_fsize;

  // define SKIPFREESPACECHECK to disable the free space checking

  #ifndef SKIPFREESPACECHECK
    // if both USERS and USERS.INF have a drive specified, and the drives
    // are the same, OR
    // if neither USERS nor USERS.INF have a drive specified, then
    // assume both files are stored on the same drive

    if ((PcbData.UsrFile[1] == ':' && PcbData.InfFile[1] == ':' && PcbData.UsrFile[0] == PcbData.InfFile[0]) ||
        (PcbData.UsrFile[1] != ':' && PcbData.InfFile[1] != ':')) {
      // both files on the same drive, add space allocations together
      SizeNeeded = PreAllocSize + UsersInfSize + 100000L;
      if (SizeNeeded > diskfreespace(PcbData.UsrFile)) {
        insufdiskspace();
        return(-1);
      }
    } else {
      // users and users.inf appear to be on different drives, calculate the
      // space requirements separately
      if ((PreAllocSize + 100000L) > diskfreespace(PcbData.UsrFile) ||
          (UsersInfSize + 100000L) > diskfreespace(PcbData.InfFile)) {
        insufdiskspace();
        return(-1);
      }
    }
  #endif

  if (UserInfo) {
    if (PreAlloc == PREALLOC) {
      if (copyfile(PcbData.InfFile,TempInfFileName,FALSE) != 0) {
        popup("Failure copying USERS.INF to USERS.NEW",NULL);
        return(-1);
      }
    }
    if (openusersinffile(TempInfFileName) == -1) {
      popup("Could not open",TempInfFileName);
      return(-1);
    }
  }

  switch(Buffered) {
    case BUFFERED:    if (dosfopen(TempFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&DosTempFile) == -1) {
                        popup("Could not open",TempFileName);
                        if (UserInfo)
                          dosfclose(&DosUsersInfFile);
                        return(-1);
                      }
                      TempFile = DosTempFile.handle;
                      if (PreAlloc == PREALLOC) {
                        dosfseek(&DosTempFile,PreAllocSize,SEEK_SET);
                        if (dosfwrite(" ",1,&DosTempFile) == -1) {   /* pre-allocate disk space */
                          popup("Could not preallocate space",NULL);
                          dosfclose(&DosTempFile);
                          if (UserInfo)
                            dosfclose(&DosUsersInfFile);
                          return(-1);
                        }
                        dosrewind(&DosTempFile);
                      }
                      break;
    case NONBUFFERED: if ((TempFile = doscreatecheck(TempFileName,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL)) == -1) {
                        popup("Could not create",TempFileName);
                        if (UserInfo)
                          dosfclose(&DosUsersInfFile);
                        return(-1);
                      }
                      if (PreAlloc == PREALLOC) {
                        doslseek(TempFile,PreAllocSize,SEEK_SET);
                        if (writecheck(TempFile," ",1) == (unsigned) -1) {   /* pre-allocate disk space */
                          popup("Could not preallocate space",NULL);
                          dosclose(TempFile);
                          if (UserInfo)
                            dosfclose(&DosUsersInfFile);
                          return(-1);
                        }
                        doslseek(TempFile,0,SEEK_SET);
                      }
                      break;
  }


  doslseek(UsersFile,0,SEEK_SET);
  return(0);
}
#endif
