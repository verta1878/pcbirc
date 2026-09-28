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


#ifdef _MSC_VER
#include <memory.h>
#else
#include <mem.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcb.h>
#include <misc.h>
#include <screen.h>
#include "pcboard.h"
#include "pcboard.ext"
#include "pcbtext.h"
#include "users.h"
#ifdef LIB
#include "usersys.h"
#endif
#ifdef DEBUG
#include <memcheck.h>
#endif

extern char        PcbDir[30];

#define abool   int
#define aTRUE   0x312D       /* equals an ascii "-1" */
#define aFALSE  0x3020       /* equals an ascii " 0" */
#define aFALSE2 0x2030       /* equals an ascii "0 " */
#define aBLANK  0x2020       /* equals an ascii "  " */

#define bool2ascii(b)  ((b) ? aTRUE : aFALSE)
#define bool2ascii2(b) ((b) ? aTRUE : aFALSE2)
#define ascii2bool(a)  ((a) == aTRUE)

#ifdef _MSC_VER
typedef struct {
  char TerseMode:1;
  char RipMode:1;
  char UseAlias:1;
  char AliasInUse:1;
  char HstMode:1;
  char Telnet:1;
  char Reserved6:1;
  char Reserved7:1;
} resbyte;
#else
typedef struct {
  int TerseMode:1;
  int RipMode:1;
  int UseAlias:1;
  int AliasInUse:1;
  int HstMode:1;
  int Telnet:1;
  int Reserved6:1;
  int Reserved7:1;
} resbyte;
#endif

typedef struct {
  abool    Screen;
  abool    PrintLog;
  abool    PageBell;
  abool    Alarm;
  char     SysopFlag;
  abool    ErrorCorrected;
  char     GraphicsMode;
  char     UserNetStatus;
  char     ModemSpeed[5];         /* rate to open com port  */
  char     CarrierSpeed[5];       /* actual speed of caller */
  unsigned short UserRecNo;
  char     FirstName[15];
  char     Password[12];
  unsigned short LogonMinute;
  short    TimeUsed;
  char     LogonTime[5];
  short    PwrdTimeAllowed;
  short    MaxKBytesAllowed;
  char     Conference;
  char     ConfJoined[5];
  char     ConfScanned[5];
  short    ConfAddTime;
  short    CreditMinutes;
  char     MultiLangExt[4];
  char     Name[25];
  short    MinutesLeft;
  char     NodeNum;
  char     EventTime[5];
  abool    EventActive;
  abool    EventSlide;
  char     MemorizeNum[4];
  char     ComPortNumber;
  char     PackFlag;
  resbyte  Reserve;
  bool     UseAnsi;
  /* used to have 8 bytes for date of last event run and 2 bytes for the time */
  short    Country;
  short    CodePage;
  char     YesChar;
  char     NoChar;
  char     Language;
  char     Reserve2[3];   /* the rest of the date/time used to be used */
  bool     RemoteDOS;
  bool     RunningEvent;  /* was EventUpComing */
  bool     StopUploads;
  unsigned short Conference2;
} systype;


void LIBENTRY makepcboardsys(void) {
}


void LIBENTRY readpcboardsys(void) {
}

