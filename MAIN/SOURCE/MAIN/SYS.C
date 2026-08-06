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

#ifdef __WATCOMC__
  #include <io.h>
#endif

#ifndef LIB
  #include "event.h"
#else
  #include "usersys.h"
#endif

#ifdef LIB
  extern char PcbDir[30];
#endif

#define abool   short
#define aTRUE   0x312D       /* equals an ascii "-1" */
#define aFALSE  0x3020       /* equals an ascii " 0" */
#define aFALSE2 0x2030       /* equals an ascii "0 " */

#define bool2ascii(b)  ((short) ((b) ? aTRUE : aFALSE))
#define bool2ascii2(b) ((short) ((b) ? aTRUE : aFALSE2))
#define ascii2bool(a)  ((bool)  ((a) == aTRUE))

#ifdef __cplusplus
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#pragma pack(1)
#if defined(_MSC_VER) || defined(__WATCOMC__)
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
  #ifndef LIB
  bassngl  MemorizeNum;
  #else
  char     MemorizeNum[4];
  #endif
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
#pragma pack()


void LIBENTRY makepcboardsys(void) {
  #if ! (defined(PCB_DEMO) || defined(LIB))
  char    *p;
  #endif
  DOSFILE File;
  char    Str[80];
  systype Sys;

  #ifndef LIB
    if (dosfopen(".\\PCBOARD.SYS",OPEN_RDWR|OPEN_DENYRDWR,&File) == -1)
      return;

    memset(&Sys,' ',sizeof(Sys));  /* pre-fill with spaces */
  #else
    strcpy(Str,PcbDir);
    strcat(Str,"PCBOARD.SYS");
    if (dosfopen(Str,OPEN_RDWR|OPEN_DENYRDWR,&File) == -1)
      return;

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    dosfread(&Sys,sizeof(Sys),&File);  /* pre-fill with pcboard.sys values */
    dosrewind(&File);
  #endif


  Sys.Screen        = bool2ascii(! Status.ForceScreenOff);
  Sys.PrintLog      = bool2ascii(Status.PrintLog);
  Sys.Alarm         = bool2ascii(Status.Alarm);
  Sys.SysopFlag     = Status.SysopFlag;

  #ifndef LIB
    Sys.RunningEvent  = (bool) (Status.Logoff == RUNEVENT);
    Sys.PageBell      = bool2ascii(Status.PageBell);
    Sys.PackFlag      = Status.PackFlag;
    Sys.RemoteDOS     = (bool) (Status.Logoff == REMOTEDOS || Status.Logoff == DOOR);
    Sys.Reserve.TerseMode = Status.TerseMode;

    if (PcbData.Network)
      Sys.NodeNum = (char) (PcbData.NodeNum > 255 ? 255 : PcbData.NodeNum);
    else
      Sys.NodeNum = 0;
  #endif

  if (Asy.Online != OFFLINE) {
    #ifndef LIB
    if (PcbData.EnforceTime && UsersData.LastDateOn == Status.JulianLogonDate) {
      Sys.TimeUsed = (short) -UsersData.ElapsedTimeOn;
    } else
    #endif
      Sys.TimeUsed = 0;

    #ifndef LIB
      Sys.StopUploads = FALSE;
      if (Status.TimeAdjustedForEvent && PcbData.EventStopUplds)
        if (PcbData.MinPriorToEvent == 0 || minutesleft() <= PcbData.MinPriorToEvent)
          Sys.StopUploads = TRUE;

      Sys.UserNetStatus = (char) (Status.Available ? AVAILABLE : UNAVAILABLE);
      Sys.MaxKBytesAllowed = Status.MaxKBytesAllowed;
    #endif

    Sys.ErrorCorrected   = bool2ascii(Asy.ErrorCorrected);
    Sys.GraphicsMode     = (Control.GraphicsMode ? 'Y' : (Asy.DataBits == 8 ? 'N':'7'));
    Sys.UseAnsi          = UseAnsi;
    Sys.Reserve.RipMode  = Control.RipMode;
    Sys.Reserve.UseAlias = Status.UseAlias;
    Sys.Reserve.HstMode  = Asy.HstMode;
    Sys.Reserve.Telnet   = Asy.Telnet;
    Sys.LogonMinute      = Status.LogonMinute;
    Sys.Conference       = (char) (Status.Conference < 255 ? Status.Conference : 255);
    Sys.Conference2      = Status.Conference;
    Sys.ConfAddTime      = Status.ConfAddTime;
    Sys.CreditMinutes    = Status.CreditMinutes;
    Sys.EventActive      = bool2ascii2(Status.SuspendMinute > 0 && Status.SuspendMinute <= 1440);
    Sys.EventSlide       = aFALSE;  /* if we're sliding we simply won't put the event time in the pcboard.sys file! */
    Sys.ComPortNumber    = (char) (Asy.ComPortNumber+'0');
    Sys.Country          = (short) CurrentCountry;
    Sys.CodePage         = (short) CodePage;
    Sys.YesChar          = YesChar;
    Sys.NoChar           = NoChar;

    #ifdef BIGNDX
      Sys.UserRecNo  = (unsigned short) (Status.UserRecNo < 65535U ? Status.UserRecNo : 65535U);
    #else
      Sys.UserRecNo  = (unsigned short) Status.UserRecNo;
    #endif

    #ifndef LIB
      Sys.PwrdTimeAllowed = Status.PwrdTimeAllowed;
      Sys.MinutesLeft     = minutesleft();
    #else
      if (Status.SysLimit == 0)
        Sys.MinutesLeft = minutesleft();
      else
        Sys.MinutesLeft = (short) (Status.SysLimit - (Status.OverrideLimit - minutesleft()));
    #endif

    sprintf(Str,"%-5ld",Asy.ModemSpeed);
    memcpy(Sys.ModemSpeed,Str,5);

    if (Asy.Online == LOCAL)
      #ifdef LIB
        memcpy(Sys.CarrierSpeed,"Local",5);
      #else
        memcpy(Sys.CarrierSpeed,(LocalOn ? "LOCAL" : "Local"),5);
      #endif
    else {
      sprintf(Str,"%-5ld",Asy.CarrierSpeed);
      memcpy(Sys.CarrierSpeed,Str,5);
    }

    #ifdef LIB
        memcpy(Sys.FirstName,Status.FirstName,min(15,strlen(Status.FirstName)));
    #else
      if (Status.CurConf.AllowAliases && UsersData.Alias[0] != 0 && Status.AllowAlias && Status.UseAlias) {
        Sys.Reserve.AliasInUse = TRUE;
        // don't put the alias name into pcboard.sys
        Status.UseAlias = FALSE;
        getdisplaynames();
        memcpy(Sys.FirstName,Status.FirstName,min(15,strlen(Status.FirstName)));
        // now put things back the way we found them!
        Status.UseAlias = TRUE;
        getdisplaynames();
      } else {
        memcpy(Sys.FirstName,Status.FirstName,min(15,strlen(Status.FirstName)));
        Sys.Reserve.AliasInUse = FALSE;
      }
    #endif

    memcpy(Sys.LogonTime,Status.LogonTime,5);

    if (Asy.Online == OFFLINE)
      Sys.Language = 0;
    else {
      Sys.Language = Status.Language;
      memcpy(Sys.MultiLangExt,Status.MultiLangExt,strlen(Status.MultiLangExt));
    }

    #ifndef LIB
      if (Status.SuspendMinute > 0 && Status.SuspendMinute <= 1440) {
        nextevent(Str);
        memcpy(Sys.EventTime,Str,5);
      } else
        memcpy(Sys.EventTime,"00:00",5);
    #endif

    strcpy(Str,UsersData.Password);
    padstr(Str,' ',12);
    memcpy(Sys.Password,Str,12);

    if (Status.UserRecNo != 1 || PcbData.UseRealName) {
      strcpy(Str,UsersData.Name);
      padstr(Str,' ',25);
      memcpy(Sys.Name,Str,25);
    } else {
      memset(Sys.Name,' ',25);
      memcpy(Sys.Name,"SYSOP",5);
    }

    #ifndef LIB
      longtobassngl(Sys.MemorizeNum,Status.MemorizeNum);
      fmemcpy(Sys.ConfJoined ,&ConfFlags[JOINED ],sizeof(Sys.ConfJoined));
      fmemcpy(Sys.ConfScanned,&ConfFlags[SCANNED],sizeof(Sys.ConfScanned));
    #endif

    dosfwrite(&Sys,sizeof(systype),&File);  /*lint !e534 */

    #ifndef LIB
      #ifndef PCB_DEMO
        if (PcbData.NumAreas > 40) {
          if ((p = (char *) malloc(ExtConfLen*2)) != NULL) {
            fmemcpy(p,&ConfFlags[JOINED+5],ExtConfLen);
            fmemcpy(p+ExtConfLen,&ConfFlags[SCANNED+5],ExtConfLen);
            dosfwrite(p,ExtConfLen*2,&File);     /*lint !e534 */
            free(p);
          }
        }
      #endif
    #endif
  } else
    dosfwrite(&Sys,sizeof(systype),&File);            /*lint !e534 */

  dosfwrite(&PcbData.NodeNum,sizeof(short),&File);      /*lint !e534 */

  #ifdef BIGNDX
    dosfwrite(&Status.UserRecNo,sizeof(long),&File);  /*lint !e534 */
  #endif

  dosftrunc(&File,-1);   /*lint !e534 truncate the file at this point */
  dosfclose(&File);
}


void LIBENTRY readpcboardsys(void) {
  unsigned BytesRead;
  #if ! (defined(PCB_DEMO) || defined(LIB))
  char    *p;
  #endif
  long    RecNo;
  DOSFILE File;
  char    Str[60];
  systype Sys;

  #ifdef LIB
    strcpy(Str,PcbDir);
    strcat(Str,"PCBOARD.SYS");
    if (fileexist(Str) == 255) {
      puts("WARNING Could not find:");
      puts(Str);
      goto readusersys;
    }
    if (dosfopen(Str,OPEN_READ|OPEN_DENYNONE,&File) == -1)
      goto readusersys;
  #else
    if (fileexist(".\\PCBOARD.SYS") == 255)
      goto readusersys;
    if (dosfopen(".\\PCBOARD.SYS",OPEN_READ|OPEN_DENYNONE,&File) == -1)
      goto readusersys;
  #endif

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  BytesRead = dosfread(&Sys,sizeof(systype),&File);
  if (BytesRead != sizeof(systype) && BytesRead != 128) { /* bad format? */
    dosfclose(&File);
    goto readusersys;
  }

  RecNo = Sys.UserRecNo;
  #ifdef BIGNDX
    if (Sys.UserRecNo == 65535U) {
      // seek past systype, past bit flags, and past node number
      dosfseek(&File,sizeof(systype)+(ExtConfLen*2L)+sizeof(short),SEEK_SET);
      // then read the user record number in long integer format

      #ifdef DEBUG
        mc_check_buffers();
      #endif

      if (dosfread(&RecNo,sizeof(long),&File) != sizeof(long))
        RecNo = Sys.UserRecNo;  // an error occured, set it back to the original record number
    }
  #endif

  Control.Screen   = ascii2bool(Sys.Screen);
  Status.PrintLog  = (bool) (ascii2bool(Sys.PrintLog) && (PcbData.ParallelPortNum != 0));
  Status.Alarm     = ascii2bool(Sys.Alarm);

  #ifndef LIB
    Status.PageBell  = ascii2bool(Sys.PageBell);
    Status.PackFlag  = ' ';             /* default to off */
  #endif

  Status.UserRecNo = 0;
  Status.SysopFlag = Sys.SysopFlag;
  /* make sure the sysop flag is valid */
  if (Status.SysopFlag != 'X' && Status.SysopFlag != 'N' && Status.SysopFlag != 'R')
    Status.SysopFlag = ' ';

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
      if (RecNo <= 0 || RecNo > numrecs(UsersFile,sizeof(UsersRead))) {
        if (Status.UserRecNo == 0 && RecNo == 0 && UsersData.Name[0] != 0) {
          // this must be an UNREGISTERED user, he's online without a user
          // record, don't worry about the record number being 0, just skip
          // over this part check and read the rest of the pcboard.sys file
        } else {
          displaypcbtext(TXT_USERRECNUMISBAD,NEWLINE|LFBEFORE|LOGIT);
          dosfclose(&File);
          goto readusersys;
        }
      }
      if (PcbData.Network && Sys.NodeNum != PcbData.NodeNum) {
        sprintf(Str,"Sys Node: %d",Sys.NodeNum);
        writelog(Str,SPACERIGHT);
      }
      Status.BackFromDoor = TRUE;  /* getuserrecord() needs to know this */
    #else
      PcbData.NodeNum = Sys.NodeNum;
    #endif

    Status.UserRecNo = RecNo;

    #ifndef LIB
      Status.PackFlag  = Sys.PackFlag;  /* but if there's a user get real status */
      Status.TerseMode = (bool) Sys.Reserve.TerseMode;
      Status.Available = (bool) (Sys.UserNetStatus == 'A');
      Status.UseAlias  = (bool) Sys.Reserve.UseAlias;
    #else
      Status.UseAlias  = (bool) (Sys.Reserve.UseAlias && Sys.Reserve.AliasInUse);
    #endif

    Asy.DataBits            = (char) (Sys.GraphicsMode == '7' ? 7 : 8);
    Control.GraphicsMode    = (bool) (Sys.GraphicsMode == 'Y');
    Control.RipMode         = (bool) Sys.Reserve.RipMode;
    Asy.HstMode            |= (bool) Sys.Reserve.HstMode;
    Asy.Telnet             |= (bool) Sys.Reserve.Telnet;
    UseAnsi                 = (bool) (Sys.GraphicsMode == 'Y' ? TRUE : Sys.UseAnsi);
    Asy.ErrorCorrected      = ascii2bool(Sys.ErrorCorrected);
    Status.LogonMinute      = Sys.LogonMinute;

    #ifndef LIB
      Status.MaxKBytesAllowed = Sys.MaxKBytesAllowed;
    #endif

    Status.Conference = (BytesRead < sizeof(systype) ? Sys.Conference : Sys.Conference2);

    #ifndef LIB
      getconfrecord(Status.Conference,&Status.CurConf);
    #endif

    Status.ConfAddTime      = Sys.ConfAddTime;
/*  Status.CreditMinutes    = Sys.CreditMinutes; */

    #ifndef LIB
      fmemcpy(&ConfFlags[JOINED ],Sys.ConfJoined ,sizeof(Sys.ConfJoined));
      fmemcpy(&ConfFlags[SCANNED],Sys.ConfScanned,sizeof(Sys.ConfScanned));

      #ifndef PCB_DEMO
        if (PcbData.NumAreas > 40 && BytesRead > 128) {
          if ((p = (char *) malloc(ExtConfLen*2)) != NULL) {

            #ifdef DEBUG
              mc_check_buffers();
            #endif

            if (dosfread(p,ExtConfLen*2,&File) == ExtConfLen*2) {
              fmemcpy(&ConfFlags[JOINED+5],p,ExtConfLen);
              fmemcpy(&ConfFlags[SCANNED+5],p+ExtConfLen,ExtConfLen);
            }
            free(p);
          }
        }
      #endif
      if (PcbData.Network) {
        uint NodeNum;
        if ((NodeNum = Sys.NodeNum) == 255)
          if (dosfread(&NodeNum,sizeof(uint),&File) != sizeof(uint))
            NodeNum = PcbData.NodeNum;
        if (NodeNum != PcbData.NodeNum) {
          sprintf(Str,"Sys Node: %d",NodeNum);
          writelog(Str,SPACERIGHT);
        }
      }
    #else
      if (Sys.NodeNum == 255) {
        uint NodeNum;

        dosfseek(&File,sizeof(systype)+(ExtConfLen*2),SEEK_SET);
        if (dosfread(&NodeNum,sizeof(uint),&File) == sizeof(uint))
          PcbData.NodeNum = NodeNum;
      }
    #endif

    Str[5] = 0;
    memcpy(Str,Sys.ModemSpeed,5);
    Asy.ModemSpeed = atol(Str);

    if (Asy.ModemSpeed == 11520)
      Asy.ModemSpeed = 115200L;
    if (Asy.CarrierSpeed == 11520)
      Asy.CarrierSpeed = 115200L;

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
      Asy.ConnectSpeed = (Asy.ErrorCorrected && Asy.CarrierSpeed >= 9600 ? Asy.ModemSpeed : Asy.CarrierSpeed);
      Asy.Online = REMOTE;
    }

    #ifndef LIB
      getuserrecord(TRUE,FALSE);
      Status.BackFromDoor = FALSE;  /* no longer needed after calling getuserrecord() */
    #endif

    #ifdef LIB
      addtime((Sys.MinutesLeft - Sys.CreditMinutes) * 60L,NEWTIME);
    #endif
    addtime(Sys.CreditMinutes * 60L,CREDTIME);

    memcpy(Status.LogonTime,Sys.LogonTime,5);
    memcpy(&Status.LogonTime[5],":00",4);
    Status.LogonMinute = (short) strtominutes(Status.LogonTime);
    datestr(Status.LogonDate);
    Status.JulianLogonDate = datetojulian(Status.LogonDate);
    if (currentminute() < (unsigned) Status.LogonMinute) {
      Status.JulianLogonDate--;
      strcpy(Status.LogonDate,juliantodate(Status.JulianLogonDate));
    }

    #ifndef LIB
      Status.CtodLogonDate = (short) ctod(Status.LogonDate);
    #endif

    if (Asy.Online != OFFLINE && (Status.Language = Sys.Language) != 0)
      getlanguagespecifics();

    memcpy(Status.MultiLangExt,Sys.MultiLangExt,4);
    Status.MultiLangExt[4] = 0;
    stripright(Status.MultiLangExt,' ');

    if (Asy.Online != OFFLINE && Status.MultiLangExt[0] != 0)
      readpcbtextfile(Status.MultiLangExt,PROGRAM);   /*lint !e534 */

    #ifndef LIB
      Status.MemorizeNum = bassngltolong(Sys.MemorizeNum);
    #endif

    renewkbdtimer();
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


#ifndef LIB
short LIBENTRY readpcbsysfornodenum(void) {
  short   NodeNum;
  DOSFILE File;
  systype Sys;

  if (fileexist(".\\PCBOARD.SYS") == 255)
    return(0);

  if (dosfopen(".\\PCBOARD.SYS",OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(0);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(&Sys,sizeof(systype),&File) != sizeof(systype)) {
    dosfclose(&File);
    return(0);
  }

  if (Sys.NodeNum == 255) {
    dosfseek(&File,sizeof(systype)+(ExtConfLen*2L),SEEK_SET);

    #ifdef DEBUG
      mc_check_buffers();
    #endif

    if (dosfread(&NodeNum,sizeof(NodeNum),&File) != sizeof(short))
      NodeNum = PcbData.NodeNum;
  } else
    NodeNum = Sys.NodeNum;

  dosfclose(&File);

  if (Sys.GraphicsMode == ' ' || Sys.FirstName[0] == ' ' || Sys.UserRecNo == 0) {
    /* if no user is online, then check the RunningEvent flag, if it is */
    /* not FALSE, nor is it a SPACE, then return the Node Number found  */
    if (Sys.RunningEvent && Sys.RunningEvent != ' ')
      return(NodeNum);
    return(0);
  }

  if (checkforusernetmatch(NodeNum,Sys.Name))  /* does nodenum match name in usernet.dat? */
    return(NodeNum);

  makepcboardsys();   /* no match found so clear out pcboard.sys */
  return(0);
}
#endif
