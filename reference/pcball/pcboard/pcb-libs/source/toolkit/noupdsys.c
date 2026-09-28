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
  systype Sys;
  char    Str[60];
  DOSFILE File;
  int     BytesRead;
  #if ! (defined(PCB_DEMO) || defined(LIB))
  char    *p;
  #endif

  strcpy(Str,PcbDir);
  strcat(Str,"PCBOARD.SYS");
  if (fileexist(Str) == 255) {
    puts("WARNING Could not find:");
    puts(Str);
    goto readusersys;
  }
  if (dosfopen(Str,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    goto readusersys;

  BytesRead = dosfread(&Sys,sizeof(systype),&File);
  if (BytesRead != sizeof(systype) && BytesRead != 128) { /* bad format? */
    dosfclose(&File);
    goto readusersys;
  }

  Control.Screen   = (bool) ascii2bool(Sys.Screen);
  Status.PrintLog  = (bool) (ascii2bool(Sys.PrintLog) && (PcbData.ParallelPortNum != 0));
  Status.Alarm     = (bool) ascii2bool(Sys.Alarm);

  #ifndef LIB
    Status.PageBell  = ascii2bool(Sys.PageBell);
    Status.PackFlag  = ' ';             /* default to off */
  #endif

  Status.UserRecNo = 0;
  Status.SysopFlag = Sys.SysopFlag;
  if (Status.SysopFlag != 'X' && Status.SysopFlag != 'N')  /* make sure it */
    Status.SysopFlag = ' ';                                /* is valid     */

  if (BytesRead < sizeof(systype)) {
    Sys.UseAnsi = FALSE;
  }

  if (Control.Screen)
    Status.ForceScreenOff = FALSE;
  else {
    Status.ForceScreenOff = TRUE;
    turndisplayoff(FALSE);  /* FALSE so that it doesn't make another pcboard.sys */
  }

  if (Sys.GraphicsMode != ' ' && Sys.FirstName[0] != ' ' &&
      Sys.GraphicsMode != 0   && Sys.FirstName[0] != 0) {
    #ifndef LIB
      if (Sys.UserRecNo <= 0 || Sys.UserRecNo > numrecs(UsersFile,sizeof(UsersRead))) {
        displaypcbtext(TXT_USERRECNUMISBAD,NEWLINE|LFBEFORE|LOGIT);
        dosfclose(&File);
        goto readusersys;
      }
      #if defined(PCB_NETWORK) && ! defined(PCB_X)
        if (PcbData.Network && Sys.NodeNum != PcbData.NodeNum) {
          sprintf(Str,"Sys Node: %d",Sys.NodeNum);
          writelog(Str,SPACERIGHT);
        }
      #endif
      Status.BackFromDoor = TRUE;  /* getuserrecord() needs to know this */
    #endif

    Status.UserRecNo = Sys.UserRecNo;

    #ifndef LIB
      Status.PackFlag  = Sys.PackFlag;  /* but if there's a user get real status */
      Status.TerseMode = Sys.Reserve.TerseMode;

      #ifdef PCB_NETWORK
        Status.Available = (Sys.UserNetStatus == 'A');
      #endif
    #endif

    Asy.Telnet              = (bool) Sys.Reserve.Telnet;
    Asy.DataBits            = (char) (Sys.GraphicsMode == '7' ? 7 : 8);
    Control.GraphicsMode    = (bool) (Sys.GraphicsMode == 'Y');
    UseAnsi                 = (bool) (Sys.GraphicsMode == 'Y' ? TRUE : Sys.UseAnsi);
    Asy.ErrorCorrected      = (bool) ascii2bool(Sys.ErrorCorrected);
    Status.LogonMinute      = Sys.LogonMinute;

    #ifndef LIB
    Status.MaxKBytesAllowed = Sys.MaxKBytesAllowed;
    #endif

    Status.Conference       = (BytesRead < sizeof(systype) ? Sys.Conference : Sys.Conference2);
    Status.ConfAddTime      = Sys.ConfAddTime;
/*  Status.CreditMinutes    = Sys.CreditMinutes; */

    #ifndef LIB
      fmemcpy(&ConfFlags[JOINED ],Sys.ConfJoined ,sizeof(Sys.ConfJoined));
      fmemcpy(&ConfFlags[SCANNED],Sys.ConfScanned,sizeof(Sys.ConfScanned));

      #ifndef PCB_DEMO
        if (PcbData.NumAreas > 40 && BytesRead > 128) {
          if ((p = malloc(ExtConfLen*2)) != NULL) {
            dosfread(p,ExtConfLen*2,&File);
            fmemcpy(&ConfFlags[JOINED+5],p,ExtConfLen);
            fmemcpy(&ConfFlags[SCANNED+5],p+ExtConfLen,ExtConfLen);
            free(p);
          }
        }
      #endif
    #endif

    Str[5] = 0;
    memcpy(Str,Sys.ModemSpeed,5);
    Asy.ModemSpeed = atol(Str);
    if (Asy.ModemSpeed == 11520)
      Asy.ModemSpeed = 115200L;

    if (Sys.CarrierSpeed[0] == 'L' || Asy.ComPortNumber == 0) {
      Asy.CarrierSpeed = Asy.ConnectSpeed = Asy.ModemSpeed;
      Asy.Online = LOCAL;
      #ifndef LIB
        if (Sys.CarrierSpeed[1] == 'O') /* "LOCAL" = LocalOn, "Local" does not */
          LocalOn = TRUE;
      #endif
    } else {
      memcpy(Str,Sys.CarrierSpeed,5);
      Asy.CarrierSpeed = atol(Str);
      if (Asy.CarrierSpeed == 11520)
        Asy.CarrierSpeed = 115200L;
      Asy.ConnectSpeed = (Asy.ErrorCorrected && Asy.CarrierSpeed >= 9600 ? Asy.ModemSpeed : Asy.CarrierSpeed);
      Asy.Online = REMOTE;
    }

    #ifndef LIB
      getuserrecord(TRUE,FALSE);
      Status.BackFromDoor = FALSE;  /* no longer needed after calling getuserrecord() */
    #endif

    #ifdef LIB
      addtime(Sys.MinutesLeft * 60L,NEWTIME);
    #endif
    addtime(Sys.CreditMinutes * 60L,CREDTIME);

    memcpy(Status.LogonTime,Sys.LogonTime,5);
    memcpy(&Status.LogonTime[5],":00",4);
    Status.LogonMinute = (short) strtominutes(Status.LogonTime);
    datestr(Status.LogonDate);
    Status.JulianLogonDate = datetojulian(Status.LogonDate);
    if (currentminute() < Status.LogonMinute) {
      Status.JulianLogonDate--;
      strcpy(Status.LogonDate,juliantodate(Status.JulianLogonDate));
    }

    #ifndef LIB
      Status.CtodLogonDate = ctod(Status.LogonDate);
    #endif

    memcpy(Status.MultiLangExt,Sys.MultiLangExt,4);
    Status.MultiLangExt[4] = 0;
    stripright(Status.MultiLangExt,' ');
    if (Status.MultiLangExt[0] != 0)
      readpcbtextfile(Status.MultiLangExt,PROGRAM);

    #ifndef LIB
      Status.MemorizeNum = bassngltolong(Sys.MemorizeNum);
    #endif
    settimer(1,Control.KbdTimer);
    dosfclose(&File);
    return;
  }
  dosfclose(&File);

readusersys:;
  #ifndef LIB
    #ifndef PCB_DEMO
      if (fileexist("USERS.SYS") != 255) {
        Status.UserRecNo = 0;
        getuserrecord(TRUE,TRUE);
        if (Status.UserRecNo != 0)
          putuserrecord(IGNOREDIRTY);
        unlink("USERS.SYS");
        Status.UserRecNo = 0;
        memset(&UsersData,0,sizeof(UsersData));
        fmemset(MsgReadPtr,0,MsgPtrLen);
        fmemset(ConfFlags,0,ConfByteLen * 2);
        fmemset(ConfReg,0,ConfByteLen * 6);
        Status.Logoff = NLOGOFF;  /* cause $$LOGOFF.BAT to be run */
        recycle();
      }
    #endif
  #endif
}
