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
  #include <malloc.h>
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#include <io.h>
#include <account.h>

#if defined(__LARGE__) || defined(__COMPACT__) || defined(__OS2__)
    #define farmalloc(x) malloc(x)
    #define farfree(x)   free(x)
#endif

#ifdef __OS2__
  #define farcoreleft() 0
#endif

static int  SaveUsersFile;
static int  SaveUsersInfFile;

UData    _FARDATA_ UsersData;
URead    _FARDATA_ UsersRead;
int      UsersFile;
int      UsersInfFile;
#ifndef LIB
unsigned ConfByteLen;
unsigned ExtConfLen;
#endif
hdrtype  Hdr;
rectype  UsersRec;
long     InfHeaderSize;

bool     AliasSupport = FALSE;
char     ReadAlias[26];
long     AliasOffset;

bool     VerifySupport = FALSE;
char     ReadVerify[26];
long     VerifyOffset;

bool         AddressSupport;
long         AddressOffset;
addresstype  _FARDATA_ AddrRec;

bool         PasswordSupport;
long         PasswordOffset;
passwordtype _FARDATA_ PwrdRec;

bool           StatsSupport;
long           StatsOffset;
callerstattype _FARDATA_ StatsRec;

bool           NotesSupport;
long           NotesOffset;
notestype      _FARDATA_ NotesRec;

#ifdef PCB152
bool           AccountSupport;
long           AccountOffset;
accounttype    _FARDATA_ Account;

bool           QwkSupport;
long           QwkOffset;
qwkconfigtype  _FARDATA_ QwkConfig;
#endif

typedef enum {PSA_NONE=0,PSA_ALIAS,PSA_VERIFY,PSA_ADDRESS,PSA_PASSWORD,PSA_STATS,PSA_NOTES,PSA_ACCOUNT,PSA_QWKNET,PSA_END} psatype;

typedef struct {
  psatype Type;
  long    Offset;
  void    *Rec;
  int     Size;
} psaordertype;

static psaordertype PSAorder[PSA_END];

URead    *TempRead;
UData    *TempData;
unsigned MsgPtrLen;
rectype  TempInfRec;

#ifndef LIB
long _FAR_ *MsgReadPtr       = NULL;
char _FAR_ *ConfFlags        = NULL;
char _FAR_ *ConfReg          = NULL;
long _FAR_ *MsgReadPtrBackup = NULL;
#endif
long _FAR_ *TempMsgPtr       = NULL;
char _FAR_ *TempReg          = NULL;

static long    TotalAppSize;
static long    UsersSize;
static long    UsersInfSize;

DOSFILE DosUsersInfFile;

#ifdef LIB
#define checkmalloc(num,str) malloc(num)
#endif


#ifndef LIB
void LIBENTRY movestr(char *dest, char *srce, unsigned size) {
  checkstack();
  memcpy(dest,srce,size);
  dest[size] = 0;

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


/********************************************************************
*
*  Function:  validateoffset()
*
*  Desc    :  This function checks the Offset passed to it and validates it
*             against the size of the USERS file.  If the Offset is less
*             than 0, or if it is greater than the size of the USERS file
*             then it writes an error message to the caller log and shows it
*             on screen.  Then it calls recycle() to hopefully avoid any
*             update to the file.
*
*             If the offset is valid, it seeks to the offset.
*/

#pragma argsused
static void _NEAR_ LIBENTRY validateoffset(long Offset, char *Operation) {
  #ifndef LIB
    char Str[80];
  #endif

  checkstack();
  if (Offset >= 0) {
    /* first assume that UsersSize is still valid for a 'quick' test */
    if (Offset <= UsersSize) {
      doslseek(UsersFile,Offset,SEEK_SET);
      return;
    }
    /* maybe another node has added to the file, ask DOS for the size of */
    /* the file and check it again against the offset we are seeking     */
    if (Offset <= (UsersSize = doslseek(UsersFile,Offset,SEEK_END))) {
      doslseek(UsersFile,Offset,SEEK_SET);
      return;
    }
  }

  #ifdef LIB
    exit(255);
  #else
    sprintf(Str,"USERS%s Offset Error (%ld/%ld) %s","",Offset,UsersInfSize,Operation);
    writedebugrecord(Str);
    fastprint(0,0,Str,0x70);
    mysound(400,100);
    tickdelay(THIRTYSECONDS);
    recycle();
  #endif
}


/********************************************************************
*
*  Function:  validateinfoffset()
*
*  Desc    :  This function checks the Offset passed to it and validates it
*             against the size of the USERS.INF file.  If the Offset is less
*             than 0, or if it is greater than the size of the USERS.INF file
*             then it writes an error message to the caller log and shows it
*             on screen.  Then it calls recycle() to hopefully avoid any
*             update to the file.
*
*             If the offset is valid, it seeks to the offset.
*/

#pragma argsused
void LIBENTRY validateinfoffset(long Offset, char *Operation) {
  #ifndef LIB
    char Str[80];
  #endif

  checkstack();
  if (Offset >= 0) {
    /* first assume that UsersInfSize is still valid for a 'quick' test */
    if (Offset <= UsersInfSize) {
      dosfseek(&DosUsersInfFile,Offset,SEEK_SET);
      return;
    }
    /* maybe another node has added to the file, ask DOS for the size of */
    /* the file and check it again against the offset we are seeking     */
    if (Offset <= (UsersInfSize = dosfseek(&DosUsersInfFile,Offset,SEEK_END))) {
      dosfseek(&DosUsersInfFile,Offset,SEEK_SET);
      return;
    }
  }

  #ifdef LIB
    exit(255);
  #else
    sprintf(Str,"USERS%s Offset Error (%ld/%ld) %s",".INF",Offset,UsersInfSize,Operation);
    writedebugrecord(Str);
    fastprint(0,0,Str,0x70);
    mysound(400,100);
    tickdelay(THIRTYSECONDS);
    recycle();
  #endif
}


/********************************************************************
*
*  Function:  storepsaelement()
*
*  Desc    :  called by setpsaorder(), this function puts a single psa type
*             into the PSAorder list and keeps the list sorted.
*/

static void _NEAR_ LIBENTRY storepsaelement(psatype Type, long Offset, void *Rec, int Size) {
  psaordertype *p;

  checkstack();
  for (p = PSAorder; p->Type != PSA_NONE && p->Offset < Offset; p++);

  if (p->Type != PSA_NONE)
    memmove(p+1,p,sizeof(psaordertype) * (int) (&PSAorder[PSA_END] - (p+1)));

  p->Type   = Type;
  p->Offset = Offset;
  p->Rec    = Rec;
  p->Size   = Size;

  #ifdef DEBUG
    mc_check_buffers();
  #endif
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

static void _NEAR_ LIBENTRY setpsaorder(void) {
  checkstack();
  memset(PSAorder,PSA_NONE,sizeof(PSAorder));

  if (AliasSupport)    storepsaelement(PSA_ALIAS,   AliasOffset,   ReadAlias, 25);
  if (VerifySupport)   storepsaelement(PSA_VERIFY,  VerifyOffset,  ReadVerify,25);
  if (AddressSupport)  storepsaelement(PSA_ADDRESS, AddressOffset, &AddrRec,   sizeof(addresstype));
  if (PasswordSupport) storepsaelement(PSA_PASSWORD,PasswordOffset,&PwrdRec,   sizeof(passwordtype));
  if (StatsSupport)    storepsaelement(PSA_STATS,   StatsOffset,   &StatsRec,  sizeof(callerstattype));
  if (NotesSupport)    storepsaelement(PSA_NOTES,   NotesOffset,   &NotesRec,  sizeof(notestype));
#ifdef PCB152
  if (AccountSupport)  storepsaelement(PSA_ACCOUNT, AccountOffset, &Account,   sizeof(accounttype));
  if (QwkSupport)      storepsaelement(PSA_QWKNET,  QwkOffset,     &QwkConfig, sizeof(qwkconfigtype));
#endif

  #ifdef DEBUG
    mc_check_buffers();
  #endif
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

static void _NEAR_ LIBENTRY putpsas(long RecordOffset, char _FAR_ *Conf) {
  char          Flag;
  unsigned      X;
  psaordertype *p;
  long          Offset;
  long          NewOffset;

  checkstack();
  Offset = dosfseek(&DosUsersInfFile,0,SEEK_CUR);

  for (p = PSAorder; p->Type != PSA_NONE; p++) {
    NewOffset = RecordOffset + p->Offset;
    if (NewOffset != Offset) {
      dosrewind(&DosUsersInfFile);
      validateinfoffset(NewOffset,"putpsas()");  /* validate & seek to NewOffset */
      Offset = NewOffset;
    }
    if (dosfwrite(p->Rec,p->Size,&DosUsersInfFile) == -1)
      break;
    Offset += p->Size;
    if (p->Type == PSA_QWKNET) {
      for (X = 0; X < PcbData.NumAreas; X++) {
        Flag = (char) (isset(&Conf[NET],X) ? 1 : 0);
        if (dosfwrite(&Flag,1,&DosUsersInfFile) == -1)
          break;
      }
      Offset += PcbData.NumAreas;
    }
  }

  #ifdef DEBUG
    mc_check_buffers();
  #endif
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

static void _NEAR_ LIBENTRY getpsas(long RecordOffset, char _FAR_ *Conf) {
  char          Flag;
  unsigned      X;
  psaordertype *p;
  long          Offset;
  long          NewOffset;

  checkstack();
  #ifdef DEBUG
    mc_check_buffers();
  #endif

  Offset = dosfseek(&DosUsersInfFile,0,SEEK_CUR);

  for (p = PSAorder; p->Type != PSA_NONE; p++) {
    NewOffset = RecordOffset + p->Offset;
    if (NewOffset != Offset) {
      validateinfoffset(NewOffset,"getpsas()");  /* validate & seek to NewOffset */
      Offset = NewOffset;
    }

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    if (dosfread(p->Rec,p->Size,&DosUsersInfFile) != p->Size)
      break;

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    Offset += p->Size;

    if (p->Type == PSA_QWKNET) {
      fmemset(&Conf[NET],0,ConfByteLen);
      for (X = 0; X < PcbData.NumAreas; X++) {
        if (dosfread(&Flag,1,&DosUsersInfFile) != 1)
          break;
        if (Flag)
          setbit(&Conf[NET],X);
      }
      Offset += PcbData.NumAreas;
    }
  }

  /* perform specialized initialization */

  if (! AliasSupport)   memset(ReadAlias,' ',25);
  if (! VerifySupport)  memset(ReadVerify,' ',25);
  if (! AddressSupport) memset(&AddrRec,' ',sizeof(addresstype));
  if (! StatsSupport)   memset(&StatsRec,0,sizeof(callerstattype));
  if (! NotesSupport)   memset(&NotesRec,' ',sizeof(notestype));
#ifdef PCB152
  if (! AccountSupport) memset(&Account,0,sizeof(accounttype));
  if (! QwkSupport) {
    memset(&QwkConfig,0,sizeof(qwkconfigtype));
    fmemset(&Conf[NET],0,ConfByteLen);
  }
#endif

  if (PasswordSupport) {
    #ifndef LIB
      if (PwrdRec.ExpireDate == 0 && PcbData.PwrdUpdate != 0)
        PwrdRec.ExpireDate = (short) (Status.JulianLogonDate + PcbData.PwrdUpdate);
    #endif
  } else {
    memset(&PwrdRec,' ',sizeof(passwordtype));
    PwrdRec.LastChange = 0;
    PwrdRec.TimesChanged = 0;
    #ifndef LIB
      PwrdRec.ExpireDate = (short) (PcbData.PwrdUpdate == 0 ? 0 : Status.JulianLogonDate + PcbData.PwrdUpdate);
    #else
      PwrdRec.ExpireDate = 0;
    #endif
  }

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}


static long _NEAR_ LIBENTRY calcconfsize(unsigned NumAreas) {
  checkstack();
  ConfByteLen = (NumAreas >> 3) + ((NumAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;
  ExtConfLen = ConfByteLen - 5;

  if (NumAreas > 40)
    NumAreas -= 40;
  else
    NumAreas = 0;

  return(((long) sizeof(long) * NumAreas) +  (ExtConfLen*3) + ConfByteLen + ConfByteLen);  /*lint !e647 */
}


#ifndef LIB
void LIBENTRY usersalloc(void) {
  char Str[80];

  checkstack();
  MsgPtrLen = (PcbData.NumAreas <= 40 ? 40*sizeof(long) : PcbData.NumAreas*sizeof(long));

  if ((MsgReadPtr = (long _FAR_ *) farmalloc(MsgPtrLen)) == NULL) {
    sprintf(Str,"insufficient memory for message pointers: %u : %ld",MsgPtrLen,farcoreleft());
    errorexittodos(Str);
  }
  if ((MsgReadPtrBackup = (long _FAR_ *) farmalloc(MsgPtrLen)) == NULL) {
    sprintf(Str,"insufficient memory for message pointers: %u / %ld",MsgPtrLen,farcoreleft());
    errorexittodos(Str);
  }

  ConfByteLen = (PcbData.NumAreas >> 3) + ((PcbData.NumAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;

  ExtConfLen = ConfByteLen - 5;

  if ((ConfReg = (char _FAR_ *) farmalloc((unsigned) (TOTCONFREGFLAGS))) == NULL) {
    sprintf(Str,"insufficient memory for conference strings: %u / %ld",TOTCONFREGFLAGS,farcoreleft());
    errorexittodos(Str);
  }
  if ((ConfFlags = (char _FAR_ *) farmalloc((unsigned) (TOTCONFFLAGS))) == NULL) {
    sprintf(Str,"insufficient memory for conference flags: %u / %ld",TOTCONFFLAGS,farcoreleft());
    errorexittodos(Str);
  }

  fmemset(MsgReadPtr,0,MsgPtrLen);
  fmemset(ConfFlags,0,TOTCONFFLAGS);
  fmemset(ConfReg,0,TOTCONFREGFLAGS);
}


void LIBENTRY usersdealloc(void) {
  checkstack();
  farfree(ConfFlags);
  farfree(ConfReg);
  farfree(MsgReadPtrBackup);
  farfree(MsgReadPtr);
}
#endif


int LIBENTRY tempuseralloc(bool AllocMsgPtrs) {
  checkstack();
  if ((TempRead = (URead *) bmalloc(sizeof(URead))) == NULL)
    return(-1);
  if ((TempData = (UData *) bmalloc(sizeof(UData))) == NULL) {
    bfree(TempRead);
    return(-1);
  }
  if ((TempReg = (char _FAR_ *) fbmalloc((unsigned) (TOTCONFREGFLAGS))) == NULL) {
    bfree(TempData);
    bfree(TempRead);
    return(-1);
  }
  if (AllocMsgPtrs) {
    if ((TempMsgPtr = (long _FAR_ *) fbmalloc(MsgPtrLen)) == NULL) {
      fbfree(TempReg);
      bfree(TempData);
      bfree(TempRead);
      return(-1);
    }
  } else TempMsgPtr = NULL;
  return(0);
}


void LIBENTRY tempuserdealloc(void) {
  checkstack();
  if (TempMsgPtr != NULL) {
    fbfree(TempMsgPtr);
    TempMsgPtr = NULL;
  }
  if (TempReg != NULL) {
    fbfree(TempReg);
    TempReg = NULL;
  }
  if (TempRead != NULL) {
    bfree(TempRead);
    TempRead = NULL;
  }
  if (TempData != NULL) {
    bfree(TempData);
    TempData = NULL;
  }
}


/********************************************************************
*
*  Function:  getfirstname()
*
*  Desc    :  Extracts the first name out of the full name passsed to it.
*             Capitalizes the first letter of the name while forcing the rest
*             of the name to lowercase letters.
*/

#ifndef LIB
static void _NEAR_ LIBENTRY getfirstname(char *FirstName, char *FullName) {
  int  Len;
  char *p;

  checkstack();
  if ((p = strchr(FullName,' ')) != NULL) {
    Len = (int) (p-FullName);
    movestr(FirstName,FullName,Len);
  } else
    strcpy(FirstName,FullName);

  proper(FirstName);
}
#endif


/********************************************************************
*
*  Function: yymmddtostr()
*
*  Desc    : Converts a date of the form 'yymmdd' to the form 'mm-dd-yy'
*/

#ifndef LIB
void LIBENTRY yymmddtostr(char Dest[], char Srce[]) {
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    char _AH,_AL,_CH,_CL,_DH,_DL;
  #endif
  checkstack();
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
#endif


/********************************************************************
*
*  Function: strtoyymmdd()
*
*  Desc    : Converts a date of the form 'mm-dd-yy' to the form 'yymmdd'
*/

void LIBENTRY strtoyymmdd(char Dest[], char Srce[]) {
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    char _AH,_AL,_CH,_CL,_DH,_DL;
  #endif
  checkstack();
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
*  Function: yymmddtommddyy()
*
*  Desc    :
*/

#ifndef LIB
void LIBENTRY yymmddtommddyy(char *Dest, char *Srce) {
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    char _AH,_AL,_CH,_CL,_DH,_DL;
  #endif
  checkstack();
  _AH = Srce[0];
  _AL = Srce[1];
  _CH = Srce[2];
  _CL = Srce[3];
  _DH = Srce[4];
  _DL = Srce[5];
  Dest[0] = _CH;
  Dest[1] = _CL;
  Dest[2] = _DH;
  Dest[3] = _DL;
  Dest[4] = _AH;
  Dest[5] = _AL;
}
#endif


/********************************************************************
*
*  Function: mmddyytoyymmdd()
*
*  Desc    :
*/

#ifndef LIB
void LIBENTRY mmddyytoyymmdd(char *Dest, char *Srce) {
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    char _AH,_AL,_CH,_CL,_DH,_DL;
  #endif
  checkstack();
  _AH = Srce[0];
  _AL = Srce[1];
  _CH = Srce[2];
  _CL = Srce[3];
  _DH = Srce[4];
  _DL = Srce[5];
  Dest[0] = _DH;
  Dest[1] = _DL;
  Dest[2] = _AH;
  Dest[3] = _AL;
  Dest[4] = _CH;
  Dest[5] = _CL;
}
#endif


void LIBENTRY decryptusersrec(URead *pur) {
  checkstack();
  if (PcbData.Encrypt) {
    decrypt(pur->City,62); /* 62 bytes = pur->City thru pur->HomeVoicePhone */
    decrypt(pur->UserComment,60); /* 60 bytes = both comment fields */
  }
}


#ifndef LIB
void LIBENTRY convertreadtoconf(char _FAR_ *Conf, URead *pur) {
  checkstack();
  fmemcpy(&Conf[REG],pur->RegConf   ,sizeof(UsersRead.RegConf));
  fmemcpy(&Conf[EXP],pur->ExpRegConf,sizeof(UsersRead.ExpRegConf));
  fmemcpy(&Conf[USR],pur->UsrRegConf,sizeof(UsersRead.UsrRegConf));
}
#endif


#ifndef LIB
void LIBENTRY convertreadtoconfandmsgs(URead *pur, char _FAR_ *Conf, long _FAR_ *MsgPtr) {
  int X;

  checkstack();
  convertreadtoconf(Conf,pur);

  if (MsgPtr != NULL)
    for (X = 0; X < 40; X++)
      MsgPtr[X] = bassngltolong((unsigned char *) pur->LastMsgRead[X]);

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


/********************************************************************
*
*  Function: moveback()
*
*  Desc    : Copies the string from *Srce to *Dest, padding with spaces
*/

#ifndef LIB
void LIBENTRY moveback(char *Dest, char *Srce, int Size) {
  checkstack();
  for (; *Srce; Size--, Dest++, Srce++)
    if (Size)
      *Dest = *Srce;
    else
      return;

  for (; Size; Size--, Dest++)
    *Dest = ' ';
}
#endif


#ifndef LIB
void LIBENTRY convertaddrtoaddrz(addresstype *pReadAddr, addresstypez *pDataAddr) {
  checkstack();
  movestr(pDataAddr->Street[0], pReadAddr->Street[0], 50);  stripright(pDataAddr->Street[0],' ');
  movestr(pDataAddr->Street[1], pReadAddr->Street[1], 50);  stripright(pDataAddr->Street[1],' ');
  movestr(pDataAddr->City     , pReadAddr->City     , 25);  stripright(pDataAddr->City     ,' ');
  movestr(pDataAddr->State    , pReadAddr->State    , 10);  stripright(pDataAddr->State    ,' ');
  movestr(pDataAddr->Zip      , pReadAddr->Zip      , 10);  stripright(pDataAddr->Zip      ,' ');
  movestr(pDataAddr->Country  , pReadAddr->Country  , 15);  stripright(pDataAddr->Country  ,' ');

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


#ifndef LIB
void LIBENTRY convertaddrztoaddr(addresstypez *pDataAddr, addresstype *pReadAddr) {
  checkstack();
  moveback(pReadAddr->Street[0], pDataAddr->Street[0], 50);
  moveback(pReadAddr->Street[1], pDataAddr->Street[1], 50);
  moveback(pReadAddr->City     , pDataAddr->City     , 25);
  moveback(pReadAddr->State    , pDataAddr->State    , 10);
  moveback(pReadAddr->Zip      , pDataAddr->Zip      , 10);
  moveback(pReadAddr->Country  , pDataAddr->Country  , 15);

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


#ifndef LIB
void LIBENTRY convertpwrdtopwrdz(passwordtype *pReadPwrd, passwordtypez *pDataPwrd) {
  checkstack();
  movestr(pDataPwrd->Previous[0], pReadPwrd->Previous[0], 12);
  movestr(pDataPwrd->Previous[1], pReadPwrd->Previous[1], 12);
  movestr(pDataPwrd->Previous[2], pReadPwrd->Previous[2], 12);
  pDataPwrd->LastChange   = pReadPwrd->LastChange;
  pDataPwrd->TimesChanged = pReadPwrd->TimesChanged;
  pDataPwrd->ExpireDate   = pReadPwrd->ExpireDate;

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


#ifndef LIB
void LIBENTRY convertpwrdztopwrd(passwordtypez *pDataPwrd, passwordtype *pReadPwrd) {
  checkstack();
  moveback(pReadPwrd->Previous[0], pDataPwrd->Previous[0], 12);
  moveback(pReadPwrd->Previous[1], pDataPwrd->Previous[1], 12);
  moveback(pReadPwrd->Previous[2], pDataPwrd->Previous[2], 12);
  pReadPwrd->LastChange   = pDataPwrd->LastChange;
  pReadPwrd->TimesChanged = pDataPwrd->TimesChanged;
  pReadPwrd->ExpireDate   = pDataPwrd->ExpireDate;

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


#ifndef LIB
static void _NEAR_ LIBENTRY convertnotestonotesz(notestype *pReadNotes, notestypez *pDataNotes) {
  int X;

  checkstack();
  for (X = 0; X < 5; X++)
    movestr(pDataNotes->Line[X],pReadNotes->Line[X],60);

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


#ifndef LIB
static void _NEAR_ LIBENTRY convertnotesztonotes(notestypez *pDataNotes, notestype *pReadNotes) {
  int X;

  checkstack();
  for (X = 0; X < 5; X++)
    moveback(pReadNotes->Line[X],pDataNotes->Line[X],60);

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


/******************************************************************************
*  Function: convertreadtodata()
*
*  Desc    : Takes the data as read from the users file (in URead format) and
*            converts it to a format useable in C (in UData format).
*/

#ifndef LIB
void LIBENTRY convertreadtodata(URead *pur, UData *pud) {
  char   TempDate[9];

  checkstack();
  movestr(pud->Name,          pur->Name,          25);
  movestr(pud->City,          pur->City,          24);
  movestr(pud->Password,      pur->Password,      12);
  movestr(pud->BusDataPhone,  pur->BusDataPhone,  13);
  movestr(pud->HomeVoicePhone,pur->HomeVoicePhone,13);
  movestr(pud->LastTimeOn,    pur->LastTimeOn,     5);
  movestr(pud->UserComment,   pur->UserComment,   30);
  movestr(pud->SysopComment,  pur->SysopComment,  30);

  pud->Flags = pur->Flags;
  memcpy(pud->Reserved,pur->Reserved,sizeof(pur->Reserved));

  yymmddtostr(TempDate,pur->LastDateOn);
  pud->LastDateOn = datetojulian(TempDate);

  yymmddtostr(TempDate,pur->DateLastDirRead);
  pud->DateLastDirRead = (short) ctod(TempDate);

  if (memcmp(pur->RegExpDate,"000000",6) != 0) {
    yymmddtostr(TempDate,pur->RegExpDate);
    pud->RegExpDate = datetojulian(TempDate);
  } else {
    pud->RegExpDate = 0;
  }

  if (pur->ExpertMode != 'Y')
    pur->ExpertMode = 'N';
  if (pur->DeleteFlag != 'Y')
    pur->DeleteFlag = 'N';

  if (pur->LastConference == 255)
    pud->LastConference = pur->LastConf2;
  else
    pud->LastConference = pur->LastConference;

  pud->ExpertMode     = (bool) (pur->ExpertMode == 'Y' ? TRUE : FALSE);
  pud->Protocol       = pur->Protocol;
  pud->SecurityLevel  = pur->SecurityLevel;
  pud->NumTimesOn     = pur->NumTimesOn;
  pud->PageLen        = pur->PageLen;
  pud->PackedFlags    = pur->PackedFlags;
  pud->RecNum         = pur->RecNum;

  pud->NumUploads       = pur->NumUploads;
  pud->NumDownloads     = pur->NumDownloads;
  pud->ElapsedTimeOn    = pur->ElapsedTimeOn;
  pud->DeleteFlag       = (bool) (pur->DeleteFlag == 'Y' ? TRUE : FALSE);
  pud->ExpSecurityLevel = pur->ExpSecurityLevel;

  pud->DailyDnldBytes = basdbletolong(pur->DailyDnldBytes);
  pud->ulTotDnldBytes = basdbletoulong(pur->TotDnldBytes);
  pud->ulTotUpldBytes = basdbletoulong(pur->TotUpldBytes);
  pud->TotDnldBytes   = basdbletodouble(pur->TotDnldBytes);
  pud->TotUpldBytes   = basdbletodouble(pur->TotUpldBytes);

  // because the values used to be used only as unsigned long integers (but
  // stored as singled double precision floats), the new double might be a
  // negative value but it should be treated as positive, so convert it
  if (pud->TotDnldBytes < 0)
    pud->TotDnldBytes += 0xFFFFFFFFL + 1.0;
  if (pud->TotUpldBytes < 0)
    pud->TotUpldBytes += 0xFFFFFFFFL + 1.0;

  if (pur == &UsersRead) {
    pud->MsgsRead = UsersRec.MsgsRead;
    pud->MsgsLeft = UsersRec.MsgsLeft;
  } else if (pur == TempRead) {
    pud->MsgsRead = TempInfRec.MsgsRead;
    pud->MsgsLeft = TempInfRec.MsgsLeft;
  }

  movestr(pud->Alias,ReadAlias,25);
  stripright(pud->Alias,' ');
  movestr(pud->Verify,ReadVerify,25);
  stripright(pud->Verify,' ');
  convertaddrtoaddrz(&AddrRec,&pud->Address);
  convertpwrdtopwrdz(&PwrdRec,&pud->PwrdHistory);
  pud->Stats = StatsRec;
  convertnotestonotesz(&NotesRec,&pud->Notes);
#ifdef PCB152
  pud->Account = Account;
  pud->QwkConfig = QwkConfig;
#endif

  stripright(pud->Name,' ');
  stripright(pud->City,' ');
  stripright(pud->Password,' ');

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


void LIBENTRY encryptusersrec(URead *pur) {
  checkstack();
  if (PcbData.Encrypt) {
    encrypt(pur->City,62); /* 62 bytes = pur->City thru pur->HomeVoicePhone */
    encrypt(pur->UserComment,60); /* 60 bytes = both comment fields */
  }
}


#ifndef LIB
void LIBENTRY convertconftoread(URead *pur, char _FAR_ *Conf) {
  checkstack();
  fmemcpy(pur->RegConf   ,&Conf[REG],sizeof(UsersRead.RegConf));
  fmemcpy(pur->ExpRegConf,&Conf[EXP],sizeof(UsersRead.ExpRegConf));
  fmemcpy(pur->UsrRegConf,&Conf[USR],sizeof(UsersRead.UsrRegConf));

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


#ifndef LIB
void LIBENTRY convertconfandmsgstoread(URead *pur, char _FAR_ *Conf, long _FAR_ *MsgPtr) {
  int X;

  checkstack();
  convertconftoread(pur,Conf);

  if (MsgPtr != NULL)
    for (X = 0; X < 40; X++)
      longtobassngl((unsigned char *) pur->LastMsgRead[X],MsgPtr[X]);

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


/********************************************************************
*
*  Function: convertdatatoread()
*
*  Desc    : Converts the users data (in UData format) back to a format to be
*            stored on disk (in URead format).
*/

#ifndef LIB
void LIBENTRY convertdatatoread(UData *pud, URead *pur) {
  char   *TempDate;
  char    Temp[10];

  checkstack();
  moveback(pur->Name,          pud->Name,          25);
  moveback(pur->City,          pud->City,          24);
  moveback(pur->Password,      pud->Password,      12);
  moveback(pur->BusDataPhone,  pud->BusDataPhone,  13);
  moveback(pur->HomeVoicePhone,pud->HomeVoicePhone,13);
  moveback(pur->LastTimeOn,    pud->LastTimeOn,     5);
  moveback(pur->UserComment,   pud->UserComment,   30);
  moveback(pur->SysopComment,  pud->SysopComment,  30);

  pur->Flags = pud->Flags;
  memcpy(pur->Reserved,pud->Reserved,sizeof(pur->Reserved));

  #ifndef LIB
  if (pur == &UsersRead)
    convertconfandmsgstoread(&UsersRead,ConfReg,MsgReadPtr);
  else
  #endif
    convertconfandmsgstoread(TempRead,TempReg,TempMsgPtr);

  TempDate = juliantodate(pud->LastDateOn);
  strtoyymmdd(pur->LastDateOn,TempDate);

  dtoc(pud->DateLastDirRead,Temp);
  strtoyymmdd(pur->DateLastDirRead,Temp);

  if (pud->RegExpDate != 0) {
    TempDate = juliantodate(pud->RegExpDate);
    strtoyymmdd(pur->RegExpDate,TempDate);
  } else {
    memcpy(pur->RegExpDate,"000000",6);
  }

  pur->LastConf2      = pud->LastConference;
  pur->LastConference = (char) (pud->LastConference > 255 ? 255 : pud->LastConference);

  pur->ExpertMode     =(pud->ExpertMode ? 'Y' : 'N');
  pur->Protocol       = pud->Protocol;
  pur->SecurityLevel  = pud->SecurityLevel;
  pur->NumTimesOn     = pud->NumTimesOn;
  pur->PageLen        = pud->PageLen;
  pur->PackedFlags    = pud->PackedFlags;
  pur->RecNum         = pud->RecNum;

  pur->NumUploads       = pud->NumUploads;
  pur->NumDownloads     = pud->NumDownloads;
  pur->ElapsedTimeOn    = pud->ElapsedTimeOn;
  pur->DeleteFlag       =(pud->DeleteFlag ? 'Y' : 'N');
  pur->ExpSecurityLevel = pud->ExpSecurityLevel;

  longtobasdble((unsigned char *) pur->DailyDnldBytes,pud->DailyDnldBytes);
  doubletobasdble((unsigned char *) pur->TotDnldBytes,pud->TotDnldBytes);
  doubletobasdble((unsigned char *) pur->TotUpldBytes,pud->TotUpldBytes);

  if (pur == &UsersRead) {
    memcpy(UsersRec.Name,pur->Name,sizeof(pur->Name));
    UsersRec.MsgsRead = pud->MsgsRead;
    UsersRec.MsgsLeft = pud->MsgsLeft;
  } else if (pur == TempRead) {
    memcpy(TempInfRec.Name,pur->Name,sizeof(pur->Name));
    TempInfRec.MsgsRead = pud->MsgsRead;
    TempInfRec.MsgsLeft = pud->MsgsLeft;
  }

  moveback(ReadAlias,pud->Alias,25);
  moveback(ReadVerify,pud->Verify,25);
  convertaddrztoaddr(&pud->Address,&AddrRec);
  convertpwrdztopwrd(&pud->PwrdHistory,&PwrdRec);
  StatsRec = pud->Stats;
  convertnotesztonotes(&pud->Notes,&NotesRec);
#ifdef PCB152
  Account = pud->Account;
  QwkConfig = pud->QwkConfig;
#endif

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


static int _NEAR_ LIBENTRY writeusersfile(long Offset, URead *pur) {
  int  RetVal;

  checkstack();
  if (UsersFile != SaveUsersFile) {
    fastprint(0,0,"Users Handle Corrupted",0x70);
    mysound(400,100);
    tickdelay(THIRTYSECONDS);
    return(-1);
  }

  #ifndef BIGNDX
  {
    char Str[80];
    /* if using Big Indexes, then this check is invalid */
    if (Offset > sizeof(URead) * 65535L) {
      sprintf(Str,"User number out of range %sing users - offset %ld","writ",Offset);
      fastprint(0,0,Str,0x70);
      writedebugrecord(Str);
      mysound(400,100);
      tickdelay(THIRTYSECONDS);
      return(-1);
    }
  }
  #endif

  encryptusersrec(pur);

  validateoffset(Offset,"writeusersfile()");  /* validate & seek to Offset */

  #ifndef __OS2__
    if (PcbData.Slaves) {
      int Handle;
      if ((Handle = dosdup(UsersFile)) == -1)
        return(-1);
      RetVal = writecheck(Handle,pur,sizeof(URead));
      doscommit(Handle);
      /* if we added a record and another node did TOO then let the slave card */
      /* detect the real length of the file before it closes and truncates it! */
      doslseek(Handle,0,SEEK_END);
      dosclose(Handle);
      return(RetVal);
    }
  #endif

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  RetVal = writecheck(UsersFile,pur,sizeof(URead));
  doscommit(UsersFile);
  /* if we added a record and another node did TOO then let the slave card */
  /* detect the real length of the file before it closes and truncates it! */
  doslseek(UsersFile,0,SEEK_END);
  return(RetVal);
}


static int _NEAR_ LIBENTRY readusersfile(long Offset, URead *pur) {
  checkstack();
  if (UsersFile != SaveUsersFile) {
    fastprint(0,0,"Users Handle Corrupted",0x70);
    mysound(400,100);
    tickdelay(THIRTYSECONDS);
    return(-1);
  }

  validateoffset(Offset,"readusersfile()");  /* Validate & seek to Offset */

  if (readcheck(UsersFile,pur,sizeof(URead)) == (unsigned) -1)
    return(-1);

  decryptusersrec(pur);
  #ifdef DEBUG
    mc_check_buffers();
  #endif
  return(0);
}


void LIBENTRY closeusersfile(void) {
  checkstack();
  if (UsersFile > 0) {
    doslseek(UsersFile,0,SEEK_END);  /* force slaves to update end ptr */
    dosclose(UsersFile);
    SaveUsersFile = UsersFile = 0;
  }
}

void LIBENTRY closeusersinffile(void) {
  checkstack();
  if (UsersInfFile > 0) {
    doslseek(UsersInfFile,0,SEEK_END);  /* force slaves to update end ptr */
    dosfclose(&DosUsersInfFile);
    SaveUsersInfFile = UsersInfFile = 0;
  }
}


/********************************************************************
*
*  Function: openusersfile()
*
*  Desc    : Opens the users file, exits if there is an error
*
*/

int LIBENTRY openusersfile(void) {
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  checkstack();
  UsersFile = dosopen(PcbData.UsrFile,OPEN_RDWR|OPEN_DENYNONE POS2ERROR);
  if (UsersFile == -1)
    errorexittodos("UNABLE TO OPEN USERS FILE - RUN PCBSM TO CREATE IT");
  SaveUsersFile = UsersFile;
  UsersSize = doslseek(UsersFile,0,SEEK_END);
  return(UsersFile);
}


/********************************************************************
*
*  Function: openusersinffile()
*
*  Desc    : Opens the users.inf file returns -1 if an error occurs
*
*/

int LIBENTRY openusersinffile(void) {
  unsigned TpaStaticSize;
  unsigned TpaDynamicSize;
  unsigned TpaNumConf;
  unsigned X;
  apptype  *Buf;
  apptype  *p;

  checkstack();
  if (dosfopen(PcbData.InfFile,OPEN_RDWR|OPEN_DENYNONE,&DosUsersInfFile) == -1)
    errorexittodos("UNABLE TO OPEN USERS.INF - RUN PCBSM TO CREATE IT");

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(&Hdr,sizeof(hdrtype),&DosUsersInfFile) != sizeof(hdrtype))
    errorexittodos("HEADER ERROR IN USERS.INF - OR UNABLE TO READ USERS.INF FILE");

  if (Hdr.SizeOfRec != sizeof(rectype) || Hdr.SizeOfConf != calcconfsize(PcbData.NumAreas))
    errorexittodos("ERROR!  Need to update User Info File Conference Allocations");

  if (Hdr.NumOfApps != 0) {
    X = Hdr.NumOfApps * sizeof(apptype);
    if ((Buf = (apptype *) malloc(X)) != NULL) {
      #ifdef DEBUG
        mc_check_buffers();
      #endif
      if (dosfread(Buf,X,&DosUsersInfFile) != X)
        errorexittodos("HEADER ERROR IN USERS.INF - OR UNABLE TO READ USERS.INF FILE");
      TpaStaticSize = 0;
      TpaDynamicSize = 0;
      for (p = Buf; p < &Buf[Hdr.NumOfApps]; p++) {
        TpaStaticSize  += p->SizeOfRec;
        TpaDynamicSize += p->SizeOfConfRec;
        if (p->Version == 10 && memcmp(p->Name,"PCB",3) == 0) {
          if (strcmp(&p->Name[3],"ALIAS") == 0 && p->SizeOfRec == 25) {
            AliasSupport = TRUE;
            AliasOffset  = p->Offset;
          }
          if (strcmp(&p->Name[3],"VERIFY") == 0 && p->SizeOfRec == 25) {
            VerifySupport = TRUE;
            VerifyOffset  = p->Offset;
          }
          if (strcmp(&p->Name[3],"ADDRESS") == 0 && p->SizeOfRec == sizeof(addresstype)) {
            AddressSupport = TRUE;
            AddressOffset  = p->Offset;
          }
          if (strcmp(&p->Name[3],"PASSWORD") == 0 && p->SizeOfRec == sizeof(passwordtype)) {
            PasswordSupport = TRUE;
            PasswordOffset  = p->Offset;
          }
          if (strcmp(&p->Name[3],"STATS") == 0 && p->SizeOfRec == sizeof(callerstattype)) {
            StatsSupport = TRUE;
            StatsOffset  = p->Offset;
          }
          if (strcmp(&p->Name[3],"NOTES") == 0 && p->SizeOfRec == sizeof(notestype)) {
            NotesSupport = TRUE;
            NotesOffset  = p->Offset;
          }
#ifdef PCB152
          if (strcmp(&p->Name[3],"ACCOUNT") == 0 && p->SizeOfRec == sizeof(accounttype)) {
            AccountSupport = TRUE;
            AccountOffset  = p->Offset;
          }
          if (strcmp(&p->Name[3],"QWKNET") == 0 && p->SizeOfRec == sizeof(qwkconfigtype)) {
            QwkSupport = TRUE;
            QwkOffset  = p->Offset;
          }
#endif
        }
      }
      setpsaorder();
      free(Buf);
      if (TpaDynamicSize != 0) {  /* only check TpaNumConf if we found some */
        TpaNumConf = (unsigned) ((Hdr.TotalRecSize - Hdr.SizeOfRec - Hdr.SizeOfConf - TpaStaticSize) / TpaDynamicSize);
        if (TpaNumConf != PcbData.NumAreas)
          errorexittodos("ERROR!  Need to update User Info File Conference Allocations");
      }
    }
  }

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  InfHeaderSize = ((long) Hdr.NumOfApps * sizeof(apptype)) + sizeof(hdrtype);
  TotalAppSize = Hdr.TotalRecSize - (sizeof(rectype)+(2L*ConfByteLen)+(3L*ExtConfLen)+(ExtConfLen > 0 ? ((long)PcbData.NumAreas-40)*sizeof(long) : 0));
  SaveUsersInfFile = UsersInfFile = DosUsersInfFile.handle;
  UsersInfSize = dosfseek(&DosUsersInfFile,0,SEEK_END);
  return(UsersInfFile);
}


#ifndef LIB
static bool _NEAR_ LIBENTRY checkdirtyrec(URead *pur, long Offset) {
  char  Str[80];
  URead OnDisk;

  checkstack();
  if (PcbData.Network) {
    if (doslockcheck(UsersFile,Offset,sizeof(URead)) == -1)
      return(FALSE);

    /* check to see if the record is "dirty" meaning has  */
    /* someone else updated it while the user was online? */

    if (readusersfile(Offset,&OnDisk) == -1)
      return(FALSE);

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    if (OnDisk.PackedFlags.Dirty) {
      memcpy(pur->City          ,OnDisk.City          ,sizeof(OnDisk.City          ));
      memcpy(pur->Password      ,OnDisk.Password      ,sizeof(OnDisk.Password      ));
      memcpy(pur->BusDataPhone  ,OnDisk.BusDataPhone  ,sizeof(OnDisk.BusDataPhone  ));
      memcpy(pur->HomeVoicePhone,OnDisk.HomeVoicePhone,sizeof(OnDisk.HomeVoicePhone));
      memcpy(pur->UserComment   ,OnDisk.UserComment   ,sizeof(OnDisk.UserComment   ));
      memcpy(pur->SysopComment  ,OnDisk.SysopComment  ,sizeof(OnDisk.SysopComment  ));
      memcpy(pur->RegExpDate    ,OnDisk.RegExpDate    ,sizeof(OnDisk.RegExpDate    ));
      memcpy(pur->RegConf       ,OnDisk.RegConf       ,sizeof(OnDisk.RegConf       ));
      memcpy(pur->ExpRegConf    ,OnDisk.ExpRegConf    ,sizeof(OnDisk.ExpRegConf    ));
      memcpy(pur->UsrRegConf    ,OnDisk.UsrRegConf    ,sizeof(OnDisk.UsrRegConf    ));
      pur->SecurityLevel       = OnDisk.SecurityLevel;
      pur->ExpSecurityLevel    = OnDisk.ExpSecurityLevel;
      pur->DeleteFlag          = OnDisk.DeleteFlag;
      pur->PackedFlags.HasMail = OnDisk.PackedFlags.HasMail;

      #ifdef DEBUG
        mc_check_buffers();
      #endif
      return(TRUE);
    }

    if (OnDisk.RecNum != pur->RecNum) {
      sprintf(Str,"ERROR! USERS.INF PTR CHANGED: %ld / %ld",pur->RecNum,OnDisk.RecNum);
      writedebugrecord(Str);
      pur->RecNum = OnDisk.RecNum;
    }
  }
  return(FALSE);
}
#endif


#ifndef LIB
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

static long _NEAR_ LIBENTRY lockuser(void) {
  long Offset;
  long RecNum;
  long Temp;

  checkstack();
  /* calculate a RecNum first (to take care of round off) and then   */
  /* calculate the offset into the file from the record number below */
  Offset = doslseek(UsersFile,0,SEEK_END);
  RecNum = (Offset / sizeof(URead)) + 1;

top:
  if (PcbData.Network) {
    Offset = (RecNum - 1) * sizeof(URead);
    while (lock(UsersFile,Offset,sizeof(URead)) == -1) {
      Offset += sizeof(UsersRead);
      RecNum++;
    }

    /* We've LOCKED what we think to be the NEXT NEW RECORD in the file */
    /* Find the offset to the CURRENT end of the file and check to make */
    /* sure that it is not now extended BEYOND what we thought was the  */
    /* end.  If it is then go to the top and try again.                 */
    /*                                                                  */
    /* Also, check to see that we have not moved outrageously out of    */
    /* range from the end of the file.  By that we mean - check to make */
    /* sure that our "expected" end of file is not more than the size   */
    /* of about 10 user records from the real end of file.  If it is    */
    /* then calculate a new record number and go start the process over */

    Temp = doslseek(UsersFile,0,SEEK_END);
    if (Temp > Offset) {
      unlock(UsersFile,Offset,sizeof(URead));
      RecNum++;
      goto top;
    }
    if (Offset > Temp + 10*sizeof(URead)) {
      unlock(UsersFile,Offset,sizeof(URead));
      RecNum = (Offset / sizeof(URead)) + 1;
      goto top;
    }
  }

  UsersSize = Offset + sizeof(URead);
  return(RecNum);
}


/********************************************************************
*
*  Function:  unlockuser()
*
*  Desc    :  unlocks the user record
*/

static void _NEAR_ LIBENTRY unlockuser(long RecNum) {
  long Offset;

  checkstack();
  if (PcbData.Network) {
    Offset = (RecNum-1) * sizeof(URead);
    unlock(UsersFile,Offset,sizeof(URead));
  }
}


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

static long _NEAR_ LIBENTRY lockuserinf(void) {
  long Offset;
  long RecNum;
  long Temp;

  checkstack();
  /* calculate a RecNum first (to take care of round off) and then   */
  /* calculate the offset into the file from the record number below */
  Offset = doslseek(UsersInfFile,0,SEEK_END);
  RecNum = ((Offset - InfHeaderSize) / Hdr.TotalRecSize) + 1;

top:
  if (PcbData.Network) {
    Offset = ((RecNum - 1) * Hdr.TotalRecSize) + InfHeaderSize;
    while (lock(UsersInfFile,Offset,Hdr.TotalRecSize) == -1) {
      Offset += Hdr.TotalRecSize;
      RecNum++;
    }
    /* We've LOCKED what we think to be the NEXT NEW RECORD in the file */
    /* find out the offset to the end of the file and if it is now      */
    /* greater than what we thought was the end the start unlock what   */
    /* we had and go start all over again from the new end of file      */
    Temp = doslseek(UsersInfFile,0,SEEK_END);
    if (Temp > Offset) {
      unlock(UsersInfFile,Offset,Hdr.TotalRecSize);
      RecNum++;
      goto top;
    }
  }

  UsersInfSize = Offset + Hdr.TotalRecSize;
  return(RecNum);
}


/********************************************************************
*
*  Function:  unlockuserinf()
*
*  Desc    :  unlocks the user record of the users.inf file
*/

static void _NEAR_ LIBENTRY unlockuserinf(long RecNum) {
  long Offset;

  checkstack();
  if (PcbData.Network) {
    Offset = ((RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize;
    unlock(UsersInfFile,Offset,Hdr.TotalRecSize);
  }
}


static long _NEAR_ LIBENTRY baduserinfpointer(long RecNum) {
  long NewNum;
  char Str[80];

  checkstack();
  NewNum = lockuserinf();
  sprintf(Str,"BAD USERS.INF POINTER %ld, CHANGED TO %ld",RecNum,NewNum);
  writedebugrecord(Str);
  return(NewNum);
}
#endif


static int _NEAR_ LIBENTRY putuserinfrecord(long *RecNum, rectype *Rec, char _FAR_ *Conf, long _FAR_ *MsgPtr, bool ClearApp, bool Dirty, bool WritePSAs) {
  bool     FixUserInf;
  char     *p;
  long     Size;
  long     Offset;
  #ifndef PCB_DEMO
  unsigned X;
  #endif
  #ifndef LIB
  char     Str[80];
  #endif

  checkstack();
  p = NULL;
  FixUserInf = FALSE;

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  /* check the LOWER bounds of the record numbers */
  if (*RecNum < 1)
    FixUserInf = TRUE;

  if (! ClearApp && ! FixUserInf) {
    /* only check the UPPER bounds if we are NOT adding a new user record */
    if (*RecNum > (doslseek(UsersInfFile,0,SEEK_END) - InfHeaderSize) / Hdr.TotalRecSize)
      FixUserInf = TRUE;
  }

  #ifndef LIB
    if (FixUserInf) {
      if (ClearApp)                            /* are we ADDING a new record? */
        *RecNum = lockuserinf();               /* yes, just get a record num  */
      else
        *RecNum = baduserinfpointer(*RecNum);  /* no, show error, get rec num */

      if (*RecNum < 1) {
        unlockuserinf(*RecNum);
        sprintf(Str,"NEW USERS.INF RECNUM BAD: %ld (%ld)",*RecNum,doslseek(UsersInfFile,0,SEEK_END));
        writedebugrecord(Str);
        fastprint(0,0,Str,0x70);
        mysound(400,100);
        tickdelay(THIRTYSECONDS);
        if (doslseek(UsersInfFile,0,SEEK_END) < 0)
          recycle();
        return(-1);
      }

      ClearApp = TRUE;  /* force it to write the TPA records too! */
    }

    if (UsersInfFile != SaveUsersInfFile) {
      fastprint(0,0,"Users Info Handle Corrupted",0x70);
      mysound(400,100);
      tickdelay(THIRTYSECONDS);
      return(-1);
    }
  #endif

  Offset = (((*RecNum)-1) * Hdr.TotalRecSize) + InfHeaderSize;
  dosflush(&DosUsersInfFile);

  validateinfoffset(Offset,"put inf #1");  /* validate & seek to Offset */

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfwrite(Rec,sizeof(rectype),&DosUsersInfFile) == -1)
    goto error;

  if ((p = (char *) checkmalloc(ConfByteLen,"USER INFO OUT")) == NULL)
    goto error;

  if (Dirty) {
    dosflush(&DosUsersInfFile);

    validateinfoffset(Offset+sizeof(rectype)+ConfByteLen,"put inf #2");  /* validate & seek to offset */

    /* now get the CONFERENCE SYSOP flags */
    #ifdef DEBUG
      mc_check_buffers();
    #endif
    if (dosfread(p,ConfByteLen,&DosUsersInfFile) != ConfByteLen)
      goto error;
    else
      fmemcpy(&Conf[CON],p,ConfByteLen);

    if (ExtConfLen > 0) {
      #ifdef DEBUG
        mc_check_buffers();
      #endif
      /* get the extended REGISTERED flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen)
        goto error;
      else
        fmemcpy(&Conf[REG+5],p,ExtConfLen);

      #ifdef DEBUG
        mc_check_buffers();
      #endif
      /* get the extended EXPIRED flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen)
        goto error;
      else
        fmemcpy(&Conf[EXP+5],p,ExtConfLen);

      #ifdef DEBUG
        mc_check_buffers();
      #endif
      /* get the extended USER SCAN flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen)
        goto error;
      else
        fmemcpy(&Conf[USR+5],p,ExtConfLen);
    }

    dosflush(&DosUsersInfFile);

    validateinfoffset(Offset+sizeof(rectype),"put inf #3");  /* validate & seek to offset */

    #ifdef DEBUG
      mc_check_buffers();
    #endif
  }

  /* write MAIL flags */
  fmemcpy(p,&Conf[MFL],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&DosUsersInfFile) == -1)
    goto cantwrite;

  /* write the CONFERENCE SYSOP flags */
  fmemcpy(p,&Conf[CON],ConfByteLen);
  if (dosfwrite(p,ConfByteLen,&DosUsersInfFile) == -1)
    goto cantwrite;

  #ifndef PCB_DEMO
    if (ExtConfLen > 0) {
      /* write extended REGISTERED flags */
      fmemcpy(p,&Conf[REG+5],ExtConfLen);
      if (dosfwrite(p,ExtConfLen,&DosUsersInfFile) == -1)
        goto cantwrite;

      /* write extended EXPIRED flags */
      fmemcpy(p,&Conf[EXP+5],ExtConfLen);
      if (dosfwrite(p,ExtConfLen,&DosUsersInfFile) == -1)
        goto cantwrite;

      /* write extended USER SCAN flags */
      fmemcpy(p,&Conf[USR+5],ExtConfLen);
      if (dosfwrite(p,ExtConfLen,&DosUsersInfFile) == -1)
        goto cantwrite;

      /* write extended LAST MSG READ pointers */
      if (MsgPtr != NULL) {
        for (X = 40; X < PcbData.NumAreas; X++) {
          fmemcpy(p,&MsgPtr[X],sizeof(long));
          if (dosfwrite(p,sizeof(long),&DosUsersInfFile) == -1)
            goto cantwrite;
        }
      }
    }
  #endif

  if (ClearApp && TotalAppSize != 0) {
    bfree(p);
    if ((p = (char *) checkmalloc(2048,"APP INFO OUT")) == NULL)
      goto error;
    memset(p,0,2048);
    for (Size = TotalAppSize; Size > 2048; Size -= 2048) {
      if (dosfwrite(p,2048,&DosUsersInfFile) == -1)
        goto cantwrite;
    }
    if (dosfwrite(p,(unsigned) Size,&DosUsersInfFile) == -1)
      goto cantwrite;
  }

  if (WritePSAs)
    putpsas(Offset,Conf);

  /* if we added a record and another node did TOO then let the slave card */
  /* detect the real length of the file before it closes and truncates it! */
  dosfseek(&DosUsersInfFile,0,SEEK_END);

  #ifndef LIB
    if (FixUserInf)
      unlockuserinf(*RecNum);
  #endif
  bfree(p);
  dosflush(&DosUsersInfFile);
  return(0);

cantwrite:
  writedebugrecord("CAN'T WRITE TO USERS.INF");

error:
  #ifndef LIB
    if (FixUserInf)
      unlockuserinf(*RecNum);
  #endif
  if (p != NULL)
    bfree(p);
  dosflush(&DosUsersInfFile);
  return(-1);
}


#ifndef LIB
#pragma warn -par
void LIBENTRY puttemprecord(long RecNo, dirtytype Flag) {
  bool  Dirty;
  long  Offset;

  checkstack();
  Offset = (RecNo-1) * sizeof(URead);

  if (Flag == CHECKDIRTY)
    Dirty = checkdirtyrec(TempRead,Offset);
  else
    Dirty = FALSE;

  if (putuserinfrecord(&TempRead->RecNum,&TempInfRec,TempReg,TempMsgPtr,FALSE,Dirty,TRUE) != -1)
    writeusersfile(Offset,TempRead);  /*lint !e534 */

  if (PcbData.Network)
    unlock(UsersFile,Offset,sizeof(URead));
}
#pragma warn +par
#endif


/********************************************************************
*
*  Function: putuserrecord()
*
*  Desc    : Attempts to lock the users file and then save the contents of the
*            UsersRead buffer to disk.
*/

#ifndef LIB
#pragma warn -par
void LIBENTRY putuserrecord(dirtytype Flag) {
  bool  Dirty;
  long  Offset;
  char  Str[80];

  checkstack();
  /* Check for an invalid user record number */
  if (Status.UserRecNo <= 0
      #ifndef BIGNDX
      || Status.UserRecNo > 65535U  /* only check if NOT using Big Indexes */
      #endif
      ) {
    /* if the number is 0 and we're still logging in, then return without */
    /* updating the user record but ALSO without writing an error message */
    #ifndef LIB
      if (Status.UserRecNo == 0 && Status.LoggingIn)
        return;
    #endif
    sprintf(Str,"ERROR! BAD RECNO: %ld",Status.UserRecNo);
    writedebugrecord(Str);
    return;
  }

  Offset = (Status.UserRecNo-1) * sizeof(URead);

  if (Flag == CHECKDIRTY)
    Dirty = checkdirtyrec(&UsersRead,Offset);
  else
    Dirty = FALSE;

  if (putuserinfrecord(&UsersRead.RecNum,&UsersRec,ConfReg,MsgReadPtr,FALSE,Dirty,TRUE) != -1)
    writeusersfile(Offset,&UsersRead);  /*lint !e534 */

  if (PcbData.Network)
    unlock(UsersFile,Offset,sizeof(URead));
}
#pragma warn +par
#endif


/********************************************************************
*
*  Function: numrecs()
*
*  Desc    : Determines the number of records in the users file
*
*  Returns : The number of records (starting with 1)
*/

long LIBENTRY numrecs(const int Handle, const int RecSize) {
  checkstack();
  return(doslseek(Handle,0,SEEK_END) / RecSize);
}


#ifndef LIB
void LIBENTRY getsysopname(void) {
  int   RetVal;
  URead SysopRec;

  checkstack();
  RetVal = readusersfile(0,&SysopRec);
  Status.SysopLevel = (char) (RetVal == -1 ? 255 : SysopRec.SecurityLevel);

  if (PcbData.UseRealName && RetVal != -1) {
    SysopRec.City[0] = 0;
    stripright(SysopRec.Name,' ');
    getfirstname(Status.SysopName,SysopRec.Name);
  } else
    getfirstname(Status.SysopName,PcbData.Sysop);
}
#endif


#ifndef LIB
static void _NEAR_ LIBENTRY getlastdatetime(void) {
  char Ch;

  Ch = UsersData.LastTimeOn[2];
  UsersData.LastTimeOn[2] = 0;
  Status.LastDateTime = ((long) (UsersData.LastDateOn) * 2048) +
                         atol(UsersData.LastTimeOn)*60 +
                         atol(&UsersData.LastTimeOn[3]);
  UsersData.LastTimeOn[2] = Ch;
}
#endif


#ifndef LIB
void LIBENTRY getdisplaynames(void) {
  if (Status.CurConf.AllowAliases && UsersData.Alias[0] != 0 && Status.AllowAlias && Status.UseAlias) {
    Status.User = (Status.UserRecNo == 1 ? SYSOP : USER);
    getfirstname(Status.FirstName,UsersData.Alias);
    strcpy(Status.DisplayName,UsersData.Alias);
  } else {
    if (Status.UserRecNo == 1) {
      Status.User = SYSOP;
      if (PcbData.UseRealName) {
        getfirstname(Status.FirstName,UsersData.Name);
        strcpy(Status.DisplayName,UsersData.Name);
      } else {
        if (PcbData.Sysop[0] == 0)
          strcpy(PcbData.Sysop,"SYSOP");
        getfirstname(Status.FirstName,PcbData.Sysop);
        strcpy(Status.DisplayName,"SYSOP");
      }
    } else {
      Status.User = USER;
      getfirstname(Status.FirstName,UsersData.Name);
      strcpy(Status.DisplayName,UsersData.Name);
    }
  }
}
#endif


#ifndef LIB
static void _NEAR_ LIBENTRY setcurseclevel(int BaseLevel) {
  checkstack();
  Status.CurSecLevel = (char) (BaseLevel + Status.ConfAddSec + Status.TempAddSec);
}
#endif


#ifndef LIB
void LIBENTRY processuserrecord(void) {
  unsigned    Today;
  #ifndef LIB
  unsigned    OldConf;
  int         OldLevel;
  long        X;      /* use long integer for calculations */
  pcbconftype Conf;
  #endif

  checkstack();
  #ifdef DEBUG
    mc_check_buffers();
  #endif

  convertreadtodata(&UsersRead,&UsersData);
  convertreadtoconfandmsgs(&UsersRead,ConfReg,MsgReadPtr);

  #ifndef LIB
    strcpy(Status.LastDateOnStr,juliantodate(UsersData.LastDateOn));
    getdisplaynames();
    Status.UseSingleLines = UsersData.Flags.SingleLines;
  #endif

  Today = getjuliandate();
  if (PcbData.SubscriptMode && UsersData.RegExpDate != 0 && Today > UsersData.RegExpDate) { /* expired? */
    #ifndef LIB
      setcurseclevel(UsersData.ExpSecurityLevel);
    #endif
    fmemcpy(&ConfReg[CUR],&ConfReg[EXP],ConfByteLen);
  } else {
    #ifndef LIB
      setcurseclevel(UsersData.SecurityLevel);
    #endif
    fmemcpy(&ConfReg[CUR],&ConfReg[REG],ConfByteLen);
  }

  if (Status.UserRecNo == 1)
    UsersData.ElapsedTimeOn = 0;

  #if defined(PCB152) && ! defined(LIB)
    calculatebalance();
  #endif

  #ifndef LIB
    if (UsersData.LastDateOn == Status.JulianLogonDate || Status.BackFromDoor) {
      Status.PrevTimeOn = UsersData.ElapsedTimeOn;
      readpwrdfile(UsersData.ElapsedTimeOn);
      if (Status.BytesRemaining != -1) {
        Status.BytesRemaining -= UsersData.DailyDnldBytes;
        if (Status.BytesRemaining < 0)
          Status.BytesRemaining = 0;
      }
    } else {
      Status.PrevTimeOn = 0;
      UsersData.ElapsedTimeOn = 0;
      UsersData.DailyDnldBytes = 0;
      readpwrdfile(0);
    }
  #endif


  #ifdef PCB152
    #ifndef LIB
      if (Status.UseDropSecLevel)
        setcurseclevel(UsersData.Account.DropSecLevel);
    #endif
  #endif

  /* don't let previous callers on a multiple-login account run the next guy */
  /* out on the daily download limit - effectively, make it a session limit. */
  /* also, zero out the conference selections and last message read pointers */
  /* so that previous callers don't set the pointers and limits for future   */
  /* callers.                                                                */

  #ifndef LIB
    if (Status.MultipleLogins && ! Status.BackFromDoor) {
      UsersData.ElapsedTimeOn = 0;
      UsersData.DailyDnldBytes = 0;
      memset(&ConfReg[USR],0,ConfByteLen);
      memset(MsgReadPtr,0,MsgPtrLen);
      memset(MsgReadPtrBackup,0,MsgPtrLen);
    }

    if (Status.CreditMinutes != 0) { /* add back in any existing CREDIT minutes */
      X = Status.CreditMinutes;
      Status.CreditMinutes = 0;
      addtime(X * 60,CREDTIME);
    }
    if (Status.ConfAddTime != 0) {  /* add back in any existing CONF minutes */
      X = Status.ConfAddTime;
      Status.ConfAddTime = 0;
      addtime(X * 60,CONFTIME);
    }

    getlastdatetime();

    if (Status.BackFromDoor) /* Status.Conference is set by readpcboardsys() */
      UsersData.LastConference = Status.Conference;

    OldConf = Status.Conference;
    Status.Conference = 0;
    getconfrecord(0,&Status.CurConf);

    /* auto rejoin? */
    if (UsersData.LastConference != 0 && UsersData.LastConference <= PcbData.NumConf) {
      getconfrecord(UsersData.LastConference,&Conf);
      /* if the conference is marked as Auto Rejoin _OR_ if we are returning  */
      /* from DOS - signalled by having already joined this conference - _OR_ */
      /* if we were just in that conference signalled by OldConf being equal  */
      /* then join the conference now by setting up the appropriate variables */
      if ((Conf.AutoRejoin || UsersData.LastConference == OldConf || isset(&ConfFlags[JOINED],UsersData.LastConference)) && Conf.Name[0] != 0) {
        if (isregistered(ConfReg,UsersData.LastConference)) {
          Status.Conference = UsersData.LastConference;
          Status.CurConf = Conf;
        } else UsersData.LastConference = 0;
      } else UsersData.LastConference = 0;
    } else UsersData.LastConference = 0;

    if (Asy.Online != OFFLINE) {
      OldLevel = Status.CurSecLevel;
      confdetails(NOUPDATE);
      /* added check for ConfPwrdAdjust on 8/28/90 */
      if (PcbData.ConfPwrdAdjust && OldLevel != Status.CurSecLevel)
        readpwrdfile(UsersData.ElapsedTimeOn);
    }

    checkpagelen();
  #endif

  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


static int _NEAR_ LIBENTRY getuserinfrecord(URead *pur, rectype *Rec, char _FAR_ *Conf, long _FAR_ *MsgPtr, bool ReadPSAs) {
  char     *p;
  long     Offset;

  checkstack();
  p = NULL;

  if (UsersInfFile != SaveUsersInfFile) {
    fastprint(0,0,"Users Info Handle Corrupted",0x70);
    mysound(400,100);
    tickdelay(THIRTYSECONDS);
    goto cantread;
  }

  /* allocate this buffer now, before modifying the read buffer size, so */
  /* that if we are low on memory it is the read buffer that will suffer */
  /* and simply be smaller than desired ... rather than aborting because */
  /* we could not allocate memory to hold the conference flags           */
  if ((p = (char *) checkmalloc(ConfByteLen,"USER INFO IN")) == NULL)
    goto zeroit;

  dosrewind(&DosUsersInfFile);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  Offset = ((pur->RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize;

  validateinfoffset(Offset,"get inf");  /* validate & seek to Offset */

  #ifdef DEBUG
    mc_check_buffers();
  #endif
  if (dosfread(Rec,sizeof(rectype),&DosUsersInfFile) != sizeof(rectype))
    goto cantread;

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (memcmp(pur->Name,Rec->Name,sizeof(UsersRead.Name)) != 0) {
    pur->RecNum = 0;  /* change it to ZERO so it can't hurt anything when they log off! */
    writedebugrecord("NAME MISMATCH ON USERS.INF RECORD");
    goto zeroit;
  }

  #ifdef DEBUG
    mc_check_buffers();
  #endif
  /* read MAIL flags */
  if (dosfread(p,ConfByteLen,&DosUsersInfFile) != ConfByteLen)
    goto cantread;
  else
    fmemcpy(&Conf[MFL],p,ConfByteLen);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  #ifdef DEBUG
    mc_check_buffers();
  #endif
  /* now get the CONFERENCE SYSOP flags */
  if (dosfread(p,ConfByteLen,&DosUsersInfFile) != ConfByteLen)
    goto cantread;
  else
    fmemcpy(&Conf[CON],p,ConfByteLen);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  #ifndef PCB_DEMO
  {
    unsigned X;

    if (ExtConfLen > 0) {
      #ifdef DEBUG
        mc_check_buffers();
      #endif

      /* get the extended REGISTERED flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen)
        goto cantread;
      else
        fmemcpy(&Conf[REG+5],p,ExtConfLen);

      #ifdef DEBUG
        mc_check_buffers();
      #endif
      /* get the extended EXPIRED flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen)
        goto cantread;
      else
        fmemcpy(&Conf[EXP+5],p,ExtConfLen);

      #ifdef DEBUG
        mc_check_buffers();
      #endif

      /* get the extended USER SCAN flags */
      if (dosfread(p,ExtConfLen,&DosUsersInfFile) != ExtConfLen)
        goto cantread;
      else
        fmemcpy(&Conf[USR+5],p,ExtConfLen);

      #ifdef DEBUG
        mc_check_buffers();
      #endif

      /* get extended LAST MSG READ pointers */
      if (MsgPtr != NULL) {
        for (X = 40; X < PcbData.NumAreas; X++) {
          if (dosfread(p,sizeof(long),&DosUsersInfFile) != sizeof(long))
            goto cantread;
          else
            fmemcpy(&MsgPtr[X],p,sizeof(long));
        }
      }

      #ifdef DEBUG
        mc_check_buffers();
      #endif
    }
  }
  #endif

  if (ReadPSAs)
    getpsas(Offset,Conf);

  bfree(p);
  #ifdef DEBUG
    mc_check_buffers();
  #endif
  return(0);

cantread:
  writedebugrecord("UNABLE TO READ USERS.INF RECORD");

zeroit:
  if (p != NULL)
    bfree(p);
  memset(Rec,0,sizeof(rectype));
  memcpy(Rec->Name,pur->Name,sizeof(pur->Name));
  fmemset(&Conf[MFL],0,ConfByteLen);
  fmemset(&Conf[CON],0,ConfByteLen);
  fmemset(&Conf[REG+5],0,ExtConfLen);
  fmemset(&Conf[EXP+5],0,ExtConfLen);
  fmemset(&Conf[USR+5],0,ExtConfLen);
  UsersData.Alias[0]  = ReadAlias[0]  = 0;
  UsersData.Verify[0] = ReadVerify[0] = 0;

  #ifndef PCB_DEMO
    if (MsgPtr != NULL && PcbData.NumAreas > 40)
      fmemset(&MsgPtr[40],0,MsgPtrLen-(40*sizeof(long)));
  #endif
  #ifdef DEBUG
    mc_check_buffers();
  #endif
  return(-1);
}


#ifndef LIB
int LIBENTRY gettemprecord(long RecNo) {
  checkstack();
  if (readusersfile((RecNo-1) * sizeof(URead),TempRead) == -1)
    return(-1);
  if (getuserinfrecord(TempRead,&TempInfRec,TempReg,TempMsgPtr,TRUE) == -1)
    return(-1);
  convertreadtodata(TempRead,TempData);
  convertreadtoconfandmsgs(TempRead,TempReg,TempMsgPtr);
  #ifdef DEBUG
    mc_check_buffers();
  #endif
  return(0);
}
#endif


#ifndef LIB
void LIBENTRY getuserrecord(bool CheckForUserSys, bool LoggedOff) {
  long Offset;

  checkstack();
  Offset = -1;  /* this is a flag only - we should never use -1! */

  if (Status.UserRecNo != 0) {
    Offset = ((long) Status.UserRecNo-1) * sizeof(URead);
    if (readusersfile(Offset,&UsersRead) == -1) {
      /* couldn't read it!!! drop to DOS and recycle back up */
      exit(99);
    }

    getuserinfrecord(&UsersRead,&UsersRec,ConfReg,MsgReadPtr,TRUE); /*lint !e534 */
  }

  /* Status.UserRecNo might be ZERO but a USERS.SYS file may exist */
  /* if so, read it and get a new Offset for the user record       */
  #ifndef LIB
    if (CheckForUserSys && fileexist("USERS.SYS") != 255) {
      readusersys(LoggedOff);
      Offset = ((long) Status.UserRecNo-1) * sizeof(URead);
    }
  #endif

  /* if Offset == -1, then we don't have a record to get! */
  if (Offset == -1)
    return;

  processuserrecord();

  #ifndef LIB
    /* reset the protocol to "N" if the current one is not valid */
    if (strchr(ValidProtocols,UsersData.Protocol) == NULL || Protocols[protoffset(UsersData.Protocol)].Desc[0] == 0)
      UsersData.Protocol = 'N';

    if (PcbData.Network && UsersRead.PackedFlags.Dirty) {
      /* we've just read the record so let's reset the Dirty Flag so that    */
      /* we don't get confused later thinking that it has been updated again */
      if (doslockcheck(UsersFile,Offset,sizeof(URead)) == -1)
        return;  /* return without resetting flag if we can't lock it */

      UsersRead.PackedFlags.Dirty = UsersData.PackedFlags.Dirty = FALSE;
      writeusersfile(Offset,&UsersRead);       /*lint !e534 */
      unlock(UsersFile,Offset,sizeof(URead));

      /* we encrypted the data in order to write the record out */
      /* so now decrypt it again so that we can continue online */
      decryptusersrec(&UsersRead);
    }
  #endif
  #ifdef DEBUG
    mc_check_buffers();
  #endif
}
#endif


/********************************************************************
*
*  Function:  newuserdefaults()
*
*  Desc    :
*
*  Returns :
*/

#ifndef LIB
static bool _NEAR_ LIBENTRY accesspublic(pcbconftype *ConfRec, unsigned Num, char _FAR_ *Conf) {
  checkstack();

  /* subtract any existing "conference additional security" and then    */
  /* check to see if the current level is sufficient to access the conf */

  if (ConfRec->PublicConf && (Status.CurSecLevel-Status.ConfAddSec) >= ConfRec->ReqSecLevel) {
    if (ConfRec->RegFlags[0] != 0) {
      if (strchr(ConfRec->RegFlags,'R') != NULL) {
        setbit(&Conf[REG],Num);
        setbit(&Conf[CUR],Num);
      }
      if (strchr(ConfRec->RegFlags,'X') != NULL)
        setbit(&Conf[EXP],Num);
      if (strchr(ConfRec->RegFlags,'S') != NULL)
        setbit(&Conf[USR],Num);
    }
    return(TRUE);  /* yes, caller has access */
  }
  return(FALSE);   /* no, caller does not have access */
}


void LIBENTRY newuserdefaults(void) {
  unsigned X;
  pcbconftype Conf;

  checkstack();
  Status.UserRecNo            = 0;
  UsersData.RecNum            = 0;
  UsersData.SecurityLevel     = PcbData.UserLevels[SEC_REG];
  UsersData.ExpSecurityLevel  = PcbData.DefExpiredLevel;
  UsersData.ExpertMode        = FALSE;
  UsersData.Protocol          = 'N';
  UsersData.NumTimesOn        = 1;
  UsersData.PageLen           = 23;
  UsersData.LastDateOn        = Status.JulianLogonDate;
  UsersData.DateLastDirRead   = (short) ctod("01/01/80");
  UsersData.MsgsRead          = 0;
  UsersData.MsgsLeft          = 0;

  getfirstname(Status.FirstName,UsersData.Name);

  strcpy(Status.LastDateOnStr,Status.LogonDate);

  Status.Conference  = 0;                        /* remove */
  Status.CurSecLevel = UsersData.SecurityLevel;  /* remove */

  UsersData.LastTimeOn[5] = 0;
  memcpy(UsersData.LastTimeOn,Status.LogonTime,5);
  getlastdatetime();

  fmemset(MsgReadPtr,0,MsgPtrLen);
  fmemset(ConfReg,0,TOTCONFREGFLAGS);

  /* auto-register the caller in all PUBLIC conferences that have NO */
  /* SECURITY requirement unless the sysop makes AutoRegConf FALSE.  */

  if (PcbData.AutoRegConf) {
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&Conf);
      accesspublic(&Conf,X,ConfReg);  /*lint !e534 */
    }
  }

  if (PcbData.SubscriptMode && PcbData.SubscriptionDays != 0)
    UsersData.RegExpDate = (unsigned short) (Status.JulianLogonDate+PcbData.SubscriptionDays);
  else
    UsersData.RegExpDate = 0;

  UsersData.PwrdHistory.LastChange = Status.JulianLogonDate;
  if (PcbData.PwrdUpdate > 0)
    UsersData.PwrdHistory.ExpireDate = (unsigned short) (Status.JulianLogonDate + PcbData.PwrdUpdate);

  #ifdef PCB152
    if (AccountSupport) {
      UsersData.Account.StartingBalance = AccountRates.NewUserBalance;
      UsersData.Account.DropSecLevel    = PcbData.DefExpiredLevel;
    }

    if (QwkSupport)
      memset(&QwkConfig,0,sizeof(qwkconfigtype));
  #endif

  convertdatatoread(&UsersData,&UsersRead);
  readpwrdfile(0);
}
#endif


/********************************************************************
*
*  Function:  adduserrecord()
*
*  Desc    :  Attempts to lock a range just beyond the end of the users file
*             and if unsuccessful it loops around and tries again (assuming
*             that another node is adding a name at the time).
*
*             It then attempts to write the new user record and index files.
*
*  Returns :  0 if an error occured, otherwise a valid User Record Number
*/

#ifndef LIB
long LIBENTRY adduserrecord(URead *pur, rectype *ExtRec, char _FAR_ *Conf, long _FAR_ *MsgPtr) {
  int      RetVal;
  long     UserRecNo;

  checkstack();
  fmemset(MsgPtr,0,MsgPtrLen);
  memcpy(ExtRec->Name,pur->Name,sizeof(pur->Name));

  UserRecNo   = lockuser();
  pur->RecNum = 0; /* force it to lock and add a new record in USERS.INF */
  RetVal = putuserinfrecord(&pur->RecNum,ExtRec,Conf,MsgPtr,TRUE,FALSE,TRUE);
  if (RetVal != -1) {
    RetVal = writeusersfile(((long) UserRecNo-1)*sizeof(URead),pur);
    decryptusersrec(pur);
  }

  unlockuser(UserRecNo);

  if (RetVal == -1) {
    displaypcbtext(TXT_ERRORINUSERSFILE,NEWLINE|LFBEFORE|LOGIT);
    return(0);
  }

  addusertoindex(pur->Name,UserRecNo);  /*lint !e534 */
  return(UserRecNo);
}
#endif


#ifndef LIB
void LIBENTRY updateuserinformation(void) {
  checkstack();
  convertdatatoread(&UsersData,&UsersRead);
  Status.UseDropSecLevel = FALSE; /* force it to calculate from scratch     */
  Status.BackFromDoor = TRUE;     /* this will cause processuserrecord() to */
  processuserrecord();            /* remember the conference and also to    */
  Status.BackFromDoor = FALSE;    /* remember any bytes downloaded so _FAR_   */
  getdisplaynames();
  redisplaystatusline();
  Status.CmdLst[0] = 0;  // force it to re-load CMD.LST
  loadcmds();
}
#endif


#ifndef LIB
void LIBENTRY togglesysopprivileges(void) {
  int Msg;

  if (Status.TempAddSec != 0) {
    if (Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL]) {
      Status.CurSecLevel -= (char) Status.TempAddSec;
      Status.TempAddSec   = 0;
      Msg = TXT_SYSOPLEVELREMOVED;
    } else {
      Status.CurSecLevel -= (char) Status.TempAddSec;
      Status.TempAddSec   = (char) (PcbData.SysopSec[SEC_SYSOPLEVEL] - Status.CurSecLevel);
      Status.CurSecLevel += (char) Status.TempAddSec;
      Msg = TXT_SYSOPLEVELGRANTED;
    }
  } else {
    Status.TempAddSec = (char) (PcbData.SysopSec[SEC_SYSOPLEVEL] - Status.CurSecLevel);
    Status.CurSecLevel += (char) Status.TempAddSec;
    Msg = TXT_SYSOPLEVELGRANTED;
  }

  showmessage(Msg);
  logsystext(Msg,SPACERIGHT);
  updateuserinformation();
  if (! Status.FullScreen)
    stuffbuffer(13);  /* stuff a carriage return into the keyboard buffer */
}
#endif


#ifndef LIB
void LIBENTRY increasetime(void) {
  checkstack();
  if (minutesleft() < 1435) {
    addtime(5 * 60L,CREDTIME);
    redisplaystatusline();
  }
}
#endif


#ifndef LIB
void LIBENTRY decreasetime(void) {
  checkstack();
  if (minutesleft() < 10)
    subtime(1 * 60L,CREDTIME);
  else
    subtime(5 * 60L,CREDTIME);
  redisplaystatusline();
}
#endif


#ifndef LIB
void LIBENTRY increasesecurity(void) {
  int OldSec = Status.CurSecLevel;

  checkstack();
  readpwrdfileadjustsecurity(INCREASE);
  if (OldSec != Status.CurSecLevel)
    updateuserinformation();
}
#endif


#ifndef LIB
void LIBENTRY decreasesecurity(void) {
  int OldSec = Status.CurSecLevel;

  checkstack();
  readpwrdfileadjustsecurity(DECREASE);
  if (OldSec != Status.CurSecLevel)
    updateuserinformation();
}
#endif


#ifndef LIB
bool LIBENTRY lockedout(char _FAR_ *Conf, unsigned ConfNum) {
  checkstack();
  // LOCKED-OUT is indicated by EXP being ON, while REG is OFF
  if (isset(&Conf[EXP],ConfNum) && ! isset(&Conf[REG],ConfNum))
    return(TRUE);
  return(FALSE);
}
#endif


/********************************************************************
*
*  Function: isregistered()
*
*  Desc    : Rather than maintain code throughout the program to determine
*            whether or not a caller has access to a given conference - all
*            of the rules are coded right here according to the following:
*
*            If the caller is ALREADY in the conference then he must have
*              access for reading the mail, etc, so return TRUE.
*
*            If the conference number is 0 (Main Board) and his security level
*              is LOWER than that necessary to abandon a conference (and after
*              passing the above rule we know he is NOT in the Main Board)
*              then the sysop must not want him to access the main board so
*              we'll return FALSE regardless of his registration flags!
*
*            If a caller is LOCKED-OUT then return FALSE.  The LOCKED-OUT
*              condition is signalled by having the REGISTERED flag turned off
*              while leaving the EXPIRED flag turned on.
*
*            If the caller is registered in the conference return TRUE.
*
*            If the conference is PUBLIC but the security level requirement is
*              less then zero then we will use ONLY the above rules and thus
*              return FALSE if the caller is not specifically registered in
*              such a conference.
*
*            If the conference is PUBLIC and has a security requirement = 0
*              then register him in the conference now and return TRUE.
*
*            If the conference is PUBLIC and he has enough security to meet the
*              security requirement then return TRUE.
*
*            Otherwise return FALSE.
*/

#ifndef LIB
bool LIBENTRY isregistered(char _FAR_ *Conf, unsigned ConfNum) {
  pcbconftype ConfRec;

  checkstack();
  if (Status.Conference == ConfNum || Status.User == SYSOP)
    return(TRUE);

  if (ConfNum == 0 && Status.CurSecLevel < PcbData.UserLevels[SEC_A])
    return(FALSE);

  if (ConfNum > PcbData.NumConf)
    return(FALSE);

  if (lockedout(Conf,ConfNum))
    return(FALSE);

  if (isset(&Conf[CUR],ConfNum))
    return(TRUE);

  getconfrecord(ConfNum,&ConfRec);
  if (accesspublic(&ConfRec,ConfNum,Conf))
    return(TRUE);

  return(FALSE);
}
#endif


/********************************************************************
*
*  Function:  checkuserreginconf()
*
*  Desc    :  Checks to see if a given user, specified by the user record
*             passed to the function, has access to the specified conference.
*
*             It first determines the security level of the user based on
*             whether his subscription has expired or not.
*
*             It then checks to see if the conference is PUBLIC and if so it
*             just returns TRUE to indicate that he does have access.
*
*             If the conference is within the first 40 it then does a quick
*             check to see if the user is registered.  If it is outside of the
*             first 40 then it must go read the USERS.INF file to get the
*             registration information.
*
*  Returns :  TRUE if the user is registered in ConfNum, FALSE otherwise.
*
*  Anomoly :  This function NEVER checks for LOCKED OUT users and therefore
*             if the user is locked out of a public conference it may indicate
*             that he has access to the while he really cannot join it.
*/

#ifndef LIB
bool LIBENTRY checkuserreginconf(unsigned ConfNum, URead *pur, char *Today) {
  bool Expired;
  int  Level;
  pcbconftype Conf;

  checkstack();
  if (ConfNum > PcbData.NumConf)
    return(FALSE);

  if (PcbData.SubscriptMode && memcmp(pur->RegExpDate,"000000",6) != 0 && memcmp(pur->RegExpDate,Today,6) <= 0) {
    Expired = TRUE;
    Level   = pur->ExpSecurityLevel;
  } else {
    Expired = FALSE;
    Level   = pur->SecurityLevel;
  }

  if (Status.Conference == ConfNum)
    Conf = Status.CurConf;
  else
    getconfrecord(ConfNum,&Conf);

  convertreadtoconf(TempReg,pur);
  if (getuserinfrecord(pur,&TempInfRec,TempReg,NULL,FALSE) == -1)
    return(FALSE);

  // LOCKED-OUT is indicated by EXP being ON, while REG is OFF
  if (isset(&TempReg[EXP],ConfNum) && ! isset(&TempReg[REG],ConfNum))
    return(FALSE);

  if (Conf.PublicConf && Conf.ReqSecLevel >= 0)
    if (Level >= Conf.ReqSecLevel)
      return(TRUE);

  return((bool) (isset(&TempReg[(Expired ? EXP : REG)],ConfNum)));
}
#endif


#ifndef LIB
bool LIBENTRY userreginconf(unsigned ConfNum, long RecNo) {
  bool  RetVal;
  char  Today[6];

  checkstack();
  if (tempuseralloc(FALSE) == -1)
    return(FALSE);

  if (readusersfile((RecNo-1) * sizeof(URead),TempRead) == -1)
    RetVal = FALSE;
  else if (TempRead->SecurityLevel != 0) {
    strtoyymmdd(Today,Status.LogonDate);
    RetVal = checkuserreginconf(ConfNum,TempRead,Today);
  } else
    RetVal = FALSE;

  tempuserdealloc();
  return(RetVal);
}
#endif


void LIBENTRY putmessagewaiting(unsigned ConfNum, long RecNo) {
  long     Offset;
  char     Name[26];

  checkstack();
  if (tempuseralloc(FALSE) == -1)
    return;

  Offset = (RecNo-1) * sizeof(URead);
  if (readusersfile(Offset,TempRead) == -1) {
    tempuserdealloc();
    return;
  }

  if (getuserinfrecord(TempRead,&TempInfRec,TempReg,NULL,FALSE) == -1) {
    tempuserdealloc();
    return;
  }

  TempRead->PackedFlags.HasMail = TRUE;  /*lint !e542 */

  #ifdef LIB
    /* only in the library should the dirty flag be set, because in PCBoard, */
    /* setting the dirty flag would cause current user updates to be lost    */
    TempRead->PackedFlags.Dirty   = TRUE;  /*lint !e542 */
  #endif

  setbit(&TempReg[MFL],ConfNum);

  if (putuserinfrecord(&TempRead->RecNum,&TempInfRec,TempReg,NULL,FALSE,FALSE,FALSE) != -1) {
    writeusersfile(Offset,TempRead);  /*lint !e534 */

    if (PcbData.Network) {
      if (RecNo == 1 && ! PcbData.UseRealName)
        strcpy(Name,"SYSOP");
      else {
        memcpy(Name,TempRead->Name,25); Name[25] = 0;
        stripright(Name,' ');
      }

      /* free up the memory now so that we have memory to update usernet */
      tempuserdealloc();

      #ifndef LIB
        setmsgflaginusernet(Name);
      #endif
    }
  }

  /* it's okay to call tempuserdealloc() more than once because it properly */
  /* guards against RE-freeing the memory.  Even though the if() statement  */
  /* above *may* have freed it, go ahead and do it again here to be sure.   */
  tempuserdealloc();
}


#ifndef LIB
static void _NEAR_ LIBENTRY showpercent(long Count, long Total) {
  static int  LastPercent;
  int         Percent;
  char        Str[10];

  checkstack();
  if (Total == 0) {
    print("  0%");
    LastPercent = 0;
    return;
  }

  Percent = (int) ((Count*100) / Total);
  if (Percent > LastPercent) {
    if (Percent > 100)
      Percent = 100;
    backupcleareol(4);
    sprintf(Str,"%3d%",Percent);  /*lint !e557 */
    print(Str);
    LastPercent = Percent;
    checkstatus();
  }
}
#endif


#ifndef LIB
void LIBENTRY scanusersputmessagewaiting(unsigned ConfNum) {
  char         Flag;
  int          ConfBit;
  int          BytesRead;
  char        *Rec;
  long         FileSize;
  long         Offset;
  long         NumRecs;
  long         Counter;
  DOSFILE      OutFile;

  checkstack();
  FileSize    = doslseek(UsersInfFile,0,SEEK_END);
  Offset      = InfHeaderSize + sizeof(rectype) + (ConfNum/8);
  ConfBit     = (1 << (ConfNum & 7));
  NumRecs     = (FileSize - InfHeaderSize) / Hdr.TotalRecSize;

  showpercent(0,0);

  if (Hdr.TotalRecSize > 16384 || (Rec = (char *) bmalloc((unsigned) Hdr.TotalRecSize)) == NULL)
    goto oldway;

  if (dosfopen(PcbData.InfFile,OPEN_RDWR|OPEN_DENYNONE,&OutFile) == -1) {
    bfree(Rec);
    goto oldway;
  }

  dossetbuf(&OutFile,16384);
  dossetbuf(&DosUsersInfFile,16384);

  dosrewind(&DosUsersInfFile);
  dosfseek(&DosUsersInfFile,Offset,SEEK_SET);
  dosfseek(&OutFile,Offset,SEEK_SET);

  for (Counter = 1; Counter <= NumRecs; Counter++) {
    if ((BytesRead = dosfread(Rec,(int) Hdr.TotalRecSize,&DosUsersInfFile)) <= 0)
      break;

    Rec[0] |= (char) ConfBit;
    if (dosfwrite(Rec,BytesRead,&OutFile) == -1)
      break;

    if (BytesRead < Hdr.TotalRecSize)
      break;

    showpercent(Counter,NumRecs);
    if (Display.AbortPrintout)
      break;
  }

  showpercent(Counter,NumRecs);
  dosfclose(&OutFile);
  dossetbuf(&DosUsersInfFile,2048);
  bfree(Rec);
  return;

oldway:
  for (Counter = 1; Offset < FileSize; Offset += Hdr.TotalRecSize, Counter++) {
    doslseek(UsersInfFile,Offset,SEEK_SET);
    if (readcheck(UsersInfFile,&Flag,sizeof(Flag)) == (unsigned) -1)  /* break if can't read */
      break;
    Flag |= (char) ConfBit;
    doslseek(UsersInfFile,Offset,SEEK_SET);
    writecheck(UsersInfFile,&Flag,sizeof(Flag));  /* no loss if can't update */  /*lint !e534 */
    showpercent(Counter,NumRecs);
    if (Display.AbortPrintout)
      break;
  }
  showpercent(Counter,NumRecs);
}
#endif


#ifndef LIB
bool LIBENTRY scanmailflags(void) {
  unsigned X;

  for (X = 0; X < ConfByteLen; X++)
    if (ConfReg[MFL+X] != 0) {
      UsersData.PackedFlags.HasMail = TRUE;  /*lint !e542 */
      return(TRUE);
    }

  UsersData.PackedFlags.HasMail = FALSE;
  return(FALSE);
}
#endif


#ifndef LIB
bool LIBENTRY checkmailflags(void) {
/*unsigned X;*/
  char     *p;

  checkstack();
  if (UsersData.RecNum < 1) {
    writedebugrecord("USERS.INF POINTER ERROR");
    return(FALSE);
  }

  if ((p = (char *) checkmalloc(ConfByteLen,"USER INFO CHECK MAIL FLAGS")) == NULL)
    return(FALSE);

  /* position it right on the mail flags for an immediate read */
  doslseek(UsersInfFile,((UsersData.RecNum-1) * Hdr.TotalRecSize) + InfHeaderSize + sizeof(rectype),SEEK_SET);

  /* read MAIL flags */
  if (readcheck(UsersInfFile,p,ConfByteLen) == (unsigned) -1) {
    bfree(p);
    return(FALSE);
  } else
    fmemcpy(&ConfReg[MFL],p,ConfByteLen);

  bfree(p);
  return(scanmailflags());
}
#endif


#ifndef LIB
bool LIBENTRY isrealname(long RecNo, char *Name) {
  URead Temp;

  checkstack();
  if (readusersfile((RecNo-1) * sizeof(URead),&Temp) == -1)
    return(FALSE);

  padstr(Name,' ',sizeof(Temp.Name));
  if (memcmp(Temp.Name,Name,sizeof(Temp.Name)) == 0)
    return(TRUE);

  return(FALSE);
}
#endif


#ifndef LIB
#ifdef FIDO
void LIBENTRY resetuser(void) {
  checkstack();
  memset(&UsersData,0,sizeof(UsersData));
  memset(&UsersRead,0,sizeof(UsersRead));
  Status.UserRecNo=0;
  Asy.Online=OFFLINE;
  Status.FirstName[0] = 0;
  Status.DisplayName[0] = 0;
  Status.FoundFido = FALSE;
  makepcboardsys();
  clearusernet();
}
#endif
#endif

// this function is used to return either the City field from the main
// user record, or if that field is blank, it returns a combination of the
// city, state and country information from the Address TPA
char * LIBENTRY usercity(void) {
  static char City[80];

  if (UsersData.City[0] != 0)
    return(UsersData.City);

  City[0] = 0;
  if (! AddressSupport)
    return(City);

  if (UsersData.Address.City[0] != 0) {
    strcpy(City,UsersData.Address.City);
    if (UsersData.Address.State[0] != 0 || UsersData.Address.Country[0] != 0)
      strcat(City,", ");
  }

  if (UsersData.Address.State[0] != 0) {
    strcat(City,UsersData.Address.State);
    if (UsersData.Address.Country[0] != 0)
      addchar(City,' ');
  }

  if (UsersData.Address.Country[0] != 0)
    strcat(City,UsersData.Address.Country);

  City[25] = 0;  // truncate it to no more than 25 characters
  stripright(City,' ');
  return(City);
}
