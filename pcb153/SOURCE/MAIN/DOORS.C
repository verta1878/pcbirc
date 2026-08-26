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
  #include <direct.h>
  #include <io.h>
#else
//  #include <dir.h>
#endif

#ifdef __OS2__
  int  LIBENTRY OSDRIVER_pcbhandle(void);
  void LIBENTRY createmodemthread(void);
  void LIBENTRY destroymodemthread(void);
#else
  #include "swap.h"
#endif

static char _FARDATA_ PCBDOOR[80];     /* these variables are declared static because */
static char _FARDATA_ PCBDRIVE[12];    /* the putenv() function will only work if the */
static char _FARDATA_ PCBDIR[66];      /* parameters passed to it are global static   */
static char _FARDATA_ PCBDAT[66];      /* values.                                     */
static char _FARDATA_ PCBNODE[12];
#ifdef __OS2__
static char PCBDSZPORT[128]; /* under OS/2 we need the PCB= for DOS windows */
static char PCBDSZLOG[128];  /* under OS/2 we need the PCB= for DOS windows */
static char PCBVAR[128];     /* under OS/2 we need the PCB= for DOS windows */
static char PCBOS2[10];      /* indicate we're running PCB-OS2              */
static char PCBHANDLE[20];   /* under OS/2 pass the Comm Port hanlde        */
static char PCBFIDO[80];
#endif

void LIBENTRY remotedos(int NumTokens) {
  char Str[2];
  char *p;

  if (NumTokens == 0) {
    Str[0] = NoChar;
    Str[1] = 0;
    inputfield(Str,TXT_EXITTODOS,1,YESNO|NEWLINE|FIELDLEN|UPCASE,NOHELP,mask_yesno);
    if (Str[0] != YesChar)
      return;
    p = Str;
  } else
    p = getnexttoken();

  if (*p == YesChar && *(p+1) == 0) {
    if (fileexist("REMOTE.SYS") != 255) {
      #ifdef __OS2__
        rename("REMOTE.SYS","REMOTE.CMD");    //lint !e534
      #else
        rename("REMOTE.SYS","REMOTE.BAT");    //lint !e534
      #endif
      PcbData.ExitToDos = TRUE;
      Status.Logoff = REMOTEDOS;
      timestr2(Status.DisplayText);
      logsystext(TXT_EXITEDTODOSAT,SPACERIGHT);
      Status.ErrorLevel = EXIT_REMOTEDOS;
      recycle();
    } else {
      displaypcbtext(TXT_NOREMOTESYSFILE,NEWLINE);
    }
  }
}


static void _NEAR_ LIBENTRY setvariable(DOSFILE *Out, char *Str, bool PlaceInEnv) {
  char *p;
  char *q;

  if (PlaceInEnv) {
    if ((p = strchr(Str,'=')) != NULL) {
      *p = 0;
      if ((q = getenv(Str)) == NULL || strcmp(q,p+1) != 0) {
        *p = '=';
        putenv(Str);
      }
    }
  } else {
    dosfputs("SET ",Out);    //lint !e534
    dosfputs(Str,Out);       //lint !e534
    dosfputs("\r\n",Out);    //lint !e534
  }
}


void LIBENTRY setpcbenv(DOSFILE *Out, bool PlaceInEnv) {
  unsigned CurDisk;
  char     Str[128];

  if (NoEnv)
    return;

  getcwd(Str,sizeof(Str));

  // if the 2nd character is a colon, then the driver letter is contained
  // in the variable, so skip over drive letter and colon by moving the
  // rest down to the start of the string.
  if (Str[1] == ':')
    memmove(Str,Str+2,strlen(Str+2)+1);

  CurDisk = dosgetcurdrive() - 1;  // subtract 1 so that drive A equals 0

  sprintf(PCBDRIVE,"PCBDRIVE=%c:",CurDisk+'A');
  setvariable(Out,PCBDRIVE,PlaceInEnv);

  sprintf(PCBDIR,"PCBDIR=%s",Str);
  setvariable(Out,PCBDIR,PlaceInEnv);

  if (strcmp(DatFile,"PCBOARD.DAT") == 0) {
    addbackslash(Str,sizeof(Str));
    sprintf(PCBDAT,"PCBDAT=%c:%sPCBOARD.DAT",CurDisk+'A',Str);
  } else {
    fullyqualifiedname(Str,DatFile,sizeof(Str));
    sprintf(PCBDAT,"PCBDAT=%s",Str);
  }
  setvariable(Out,PCBDAT,PlaceInEnv);

  if (PcbData.Network) {
    sprintf(PCBNODE,"PCBNODE=%d",PcbData.NodeNum);
    setvariable(Out,PCBNODE,PlaceInEnv);
  }

  #ifdef __OS2__
    strcpy(PCBOS2,"PCBOS2=Y");
    setvariable(Out,PCBOS2,PlaceInEnv);

    sprintf(PCBHANDLE,"PCBHANDLE=%d",OSDRIVER_pcbhandle());
    setvariable(Out,PCBHANDLE,PlaceInEnv);

    if (! PlaceInEnv) {
      char *p;
      if ((p = getenv("PCB")) != NULL) {
        sprintf(PCBVAR,"PCB=%s",p);
        setvariable(Out,PCBVAR,PlaceInEnv);
      }
      sprintf(PCBDSZLOG,"DSZLOG=%s",DszLog);
      setvariable(Out,PCBDSZLOG,PlaceInEnv);
      if ((p = getenv("DSZPORT")) != NULL) {
        sprintf(PCBDSZPORT,"DSZPORT=%s",p);
        setvariable(Out,PCBDSZPORT,PlaceInEnv);
      }
      if ((p = getenv("PCBFIDO")) != NULL) {
        sprintf(PCBFIDO,"PCBFIDO=%s",p);
        setvariable(Out,PCBFIDO,PlaceInEnv);
      }
    }
  #endif
}


static void _NEAR_ LIBENTRY createdoorsys(void) {
  unsigned X;
  char     *p;
  DOSFILE  Out;
  char     Temp[80];
  char     Str[256];

  unlink("DOOR.SYS");
  if (dosfopen("DOOR.SYS",OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1)
    return;

  sprintf(Str,"COM%d:\r\n%ld\r\n%d\r\n%d\r\n%ld\r\n",
          (Asy.Online == REMOTE ? Asy.ComPortNumber : 0),
          Asy.CarrierSpeed,
          Asy.DataBits,
          (PcbData.Network ? PcbData.NodeNum : 1),
          Asy.ModemSpeed);
  dosfputs(Str,&Out);        //lint !e534

  sprintf(Str,"%s\r\n%s\r\n%s\r\n%s\r\n",
          (Status.ForceScreenOff ? "N" : "Y"),
          (Status.PrintLog       ? "Y" : "N"),
          (Status.PageBell       ? "Y" : "N"),
          (Status.Alarm          ? "Y" : "N"));
  dosfputs(Str,&Out);        //lint !e534

  sprintf(Str,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
          (Status.UserRecNo != 1 || PcbData.UseRealName ? UsersData.Name : "SYSOP"),
          usercity(),
          UsersData.HomeVoicePhone,
          UsersData.BusDataPhone,
          UsersData.Password);
  dosfputs(Str,&Out);        //lint !e534

  sprintf(Str,"%d\r\n%d\r\n%s\r\n%ld\r\n%d\r\n%s\r\n%d\r\n%s\r\n",
          Status.CurSecLevel,
          UsersData.NumTimesOn,
          juliantodate(UsersData.LastDateOn),
          minutesleft() * 60L,
          minutesleft(),
          (Control.GraphicsMode ? (Control.RipMode ? "RIP" : "GR") : (Asy.DataBits == 7 ? "7E" : "NG")),
          Display.PageLen+1,
          (UsersData.ExpertMode ? "Y" : "N"));
  dosfputs(Str,&Out);        //lint !e534

  if (! NoConfReg) {
    for (Str[0] = 0, X = 1; X < PcbData.NumAreas; X++) {
      if (isregistered(ConfReg,X)) {
        lascii(Temp,X);
        dosfputs(Temp,&Out);      //lint !e534
        if (X+1 < PcbData.NumAreas)
          dosfputs(",",&Out);     //lint !e534
      }
    }
  }
  dosfputs("\r\n",&Out);     //lint !e534

  sprintf(Str,"%d\r\n%s\r\n%ld\r\n%c\r\n%d\r\n%d\r\n%ld\r\n%d\r\n",
          Status.Conference,
          juliantodate(UsersData.RegExpDate),
          Status.UserRecNo,
          UsersData.Protocol,
          UsersData.NumUploads,
          UsersData.NumDownloads,
          (UsersData.DailyDnldBytes >> 10),
          Status.MaxKBytesAllowed);
  dosfputs(Str,&Out);        //lint !e534

  strcpy(Temp,PcbData.UsrFile);
  if ((p = strrchr(Temp,'\\')) == NULL)
    Temp[0] = 0;
  else {
    if (p == Temp && p == &Temp[2])
      p++;
    *p = 0;
  }

  sprintf(Str,"\r\n%s\r\n",Temp);
  dosfputs(Str,&Out);        //lint !e534

  strcpy(Temp,PcbData.TxtLoc);
  if ((p = strrchr(Temp,'\\')) == NULL)
    Temp[0] = 0;
  else {
    if (p == Temp && p == &Temp[2])
      p++;
    *p = 0;
  }

  sprintf(Str,"%s\r\n%s\r\n",Temp,Status.SysopName);
  dosfputs(Str,&Out);        //lint !e534

  sprintf(Str,"%s\r\n%s\r\n%c\r\n%c\r\n%c\r\n%d\r\n%d\r\n",
          UsersData.Alias,
          PcbData.EventTime,
          (Asy.ErrorCorrected ? 'Y' : 'N'),
          (UseAnsi && ! Control.GraphicsMode ? 'Y' : 'N'),
          (PcbData.Network ? 'Y' : 'N'),
          Display.DefaultColor,
          Status.CreditMinutes);
  dosfputs(Str,&Out);        //lint !e534

  dtoc(UsersData.DateLastDirRead,Temp);
  dosfputs(Temp,&Out);       //lint !e534
  strcpy(Temp,Status.LogonTime);
  Temp[5] = 0;

  sprintf(Str,"\r\n%s\r\n%s\r\n0\r\n0\r\n%f\r\n%f\r\n",
          Temp,
          UsersData.LastTimeOn,
          /* maximum daily download files */
          /* number of files so far today */
          (UsersData.TotUpldBytes / 1024),
          (UsersData.TotDnldBytes / 1024));
  dosfputs(Str,&Out);        //lint !e534

  sprintf(Str,"%s\r\n0\r\n%ld\r\n",
    UsersData.UserComment,
    /* total doors opened */
    UsersData.MsgsLeft);
  dosfputs(Str,&Out);        //lint !e534

  dosfclose(&Out);
}


/********************************************************************
*
*  Function:  executedoor()
*
*  Desc    :  copies the door batch file to DOOR.BAT and inserts any command
*             line parameters into the top of the file
*
*  Returns : -4 = Unable to open files
*            else it recycles and never returns
*/

static int _NEAR_ LIBENTRY executedoor(int NumTokens, char *Name, char *Path, bool MakeDoorSys, char ShellDoor) {
  int         Counter;
  char       *p;
  DOSFILE     In;
  DOSFILE     Out;
  char        Str[80];
  pcbtexttype Buf;
  char        Temp[256];

  #ifdef __OS2__
    Status.DoorPath[0] = 0;
  #endif

  strcat(Path,Name);
  if (dosfopen(Path,OPEN_READ|OPEN_DENYNONE,&In) == -1) {
    displaypcbtext(TXT_DOORNOTAVAILABLE,NEWLINE|LOGIT);
    return(-4);
  }

  #ifdef __OS2__
    maxstrcpy(Status.DoorPath,Path,sizeof(Status.DoorPath));
  #endif

  #ifdef __OS2__
  if (Status.Os2Door)
    p = "DOOR.CMD";
  else
  #endif
    p = "DOOR.BAT";

  if (dosfopen(p,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1) {
    dosfclose(&In);
    displaypcbtext(TXT_DOORNOTAVAILABLE,NEWLINE|LOGIT);
    return(-4);
  }

  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));

  if ((ShellDoor == 'Y' || ShellDoor == 'S' || ShellDoor == 'F') && ! Status.TerseMode) {
    /* it's a shelled door and we're not in terse mode, don't */
    /* display the "loading door xyz, please wait" message    */
  } else {
    if (Display.AbortPrintout)  /* catch any stray aborts */
      checkdisplaystatus();
    Display.NumLinesPrinted = 0;  /* don't let a "more?" prompt crop up here */
    displaypcbtext(TXT_OPENINGDOOR,LFBEFORE|NEWLINE);
  }

  usernetavailable();  // just in case we need to restore net status PRIOR to creating pcboard.sys

  strcpy(PCBDOOR,"PCBDOOR=");
  for (; NumTokens; NumTokens--) {
    strcat(PCBDOOR,getnexttoken());
    addchar(PCBDOOR,' ');
  }
  stripright(PCBDOOR,' ');
  stripall(PCBDOOR,'|');
  stripall(PCBDOOR,'<');
  stripall(PCBDOOR,'>');

  #if __OS2__
    // always set the environment variables for doors under OS/2
    setvariable(&Out,PCBDOOR,FALSE);
    setpcbenv(&Out,FALSE);
  #else
    setvariable(&Out,PCBDOOR,(bool) (ShellDoor != 'N'));
    if (ShellDoor == 'N')
      setpcbenv(&Out,FALSE);
  #endif

  while ((Counter = dosfread(Temp,sizeof(Temp),&In)) > 0)
    dosfwrite(Temp,Counter,&Out);   //lint !e534

  dosfclose(&Out);
  dosfclose(&In);

  displaycmdfile("PREOPEN");

  if (MakeDoorSys)
    createdoorsys();

  if (ShellDoor == 'N')
    PcbData.ExitToDos = TRUE;
  else {
    PcbData.ExitToDos = FALSE;
    Status.SwapDoor = (bool) (ShellDoor == 'S');
  }

  Status.Logoff = DOOR;
  getsystext(TXT_OPENEDDOOR,&Buf);
  timestr2(Str);
  strcat(Buf.Str,Str);
  writelog(Buf.Str,SPACERIGHT);
  maxstrcpy(Status.DoorName,Name,sizeof(Status.DoorName));
  Status.ErrorLevel = EXIT_DOOR;

  #ifdef PCB152
    if (Status.ActStatus != ACT_DISABLED)
      recordusage("DOOR USAGE",Name,Status.DoorChargePerUse,1,&UsersData.Account.DebitTPU);
  #endif

  if (ShellDoor == 'F') {
    updateuserrecord();
//  if (Status.MakeUserSys)
//    makeusersys();
//  writeusernetstatus(INADOOR,Status.DoorName);
    int SaveCursor = getcursor();
    int  Saved     = memsavescreen();
    char OldX      = awherex();
    char OldY      = awherey();
    cls();
    redisplaystatusline();
    setcursor(CUR_NORMAL);
    makepcboardsys();
    #ifdef __OS2__
      // get rid of the modem thread so that it doesn't interfere with the
      // shelled-to process which might try to use the comm handle
      if (ModemOpened)
        destroymodemthread();
    #endif
    runshelldoor();
    Status.CreditMinutes = 0;  // this will be re-initialized by readpcboardsys()
    Status.ConfAddTime = 0;    // this will be re-initialized by readpcboardsys()
    readpcboardsys();
    renewkbdtimer();
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);
    setcursor(SaveCursor);
    if (Control.GraphicsMode) /* force it to reset the color! */
      asetcolor(0);
    #ifdef __OS2__
      // the port is still open so we need to reestablish the modem threads
      if (ModemOpened)
        createmodemthread();
    #endif
    stuffkbdforcaller();
    usernetavailable();
    backfromdos();
    return(0);
  }

  recycle();
  return(0);  /* we'll never get this far */
}


/********************************************************************
*
*  Function:  searchdoorlist()
*
*  Desc    :  opens the DOORS.LST file and searches for Text whether it be a
*             door number or the name of a door.
*
*  Note    :  when everything is successful there is _NO_ returning from here!
*
*  Returns : -1 = unable to open DOORS.LST file
*            -2 = Door NUMBER or NAME was not found
*            -3 = FoundName is blank
*            -4 = Security or Password requirement was not met
*            -5 = Unable to copy door batch file
*            -6 = Caller cancelled door open request
*/

int LIBENTRY searchdoorlist(char *Text, int NumTokens) {
  bool        Found;
  bool        MakeDoorSys;
  char        ShellDoor;
  int         Counter;
  int         DoorNum;
  int         Security;
  unsigned    Len;
  double      Charge;
  DOSFILE     In;
  char        Shell[10];
  char        Password[13];
  char        Name[20];
  char        DoorList[66];
  char        Path[66];
  char        Temp[256];

  if ((Len = strlen(Text)) > sizeof(Name)-1)   /* don't check for names that */
    return(-2);                                /* are too long to process!   */

  /* check for a security specific version of the file */
  /* if not found but NovellSearch is specified then drop through anyway */
  if (checkforalternatelist(DoorList,Status.CurConf.DrsFile,0) == -1 && ! NovellSearch)
    return(-1);

  if (dosfopen(DoorList,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    return(-1);

  Name[0] = 0;
  maxstrcpy(Status.DisplayText,Text,sizeof(Status.DisplayText));

  if (alldigits(Text)) {
    #ifdef PCB_DEMO
      /* in the DEMO version only allow it to read the first entry */
      Counter = -1;
      if ((DoorNum = atoi(Text)) == 1) {
        if (dosfgets(Temp,sizeof(Temp)-1,&In) != -1)
          Counter = 1;
      }
    #else
      for (DoorNum = atoi(Text), Counter = 1; ; Counter++) {
        if (dosfgets(Temp,sizeof(Temp)-1,&In) == -1) {
          Counter = -1;
          break;
        }
        if (Counter == DoorNum)
          break;
      }
    #endif
    dosfclose(&In);
    if (Counter != DoorNum)
      return(-2);
    maxstrcpy(Name,strupr(parse(Temp)),sizeof(Name));
  } else {
    Found = FALSE;
    strupr(Text);
    #ifdef PCB_DEMO
      if (dosfgets(Temp,sizeof(Temp)-1,&In) != -1) {
        maxstrcpy(Name,parse(Temp),sizeof(Name));
        strupr(Name);
        if (memcmp(Text,Name,Len) == 0)
          Found = TRUE;
      }
    #else
      while (dosfgets(Temp,sizeof(Temp)-1,&In) != -1) {
        maxstrcpy(Name,parse(Temp),sizeof(Name));
        strupr(Name);
        if (strcmp(Text,Name) == 0) {
          Found = TRUE;
          break;
        }
      }
      if (! Found) {
        dosfseek(&In,0,SEEK_SET);
        while (dosfgets(Temp,sizeof(Temp)-1,&In) != -1) {
          maxstrcpy(Name,parse(Temp),sizeof(Name));
          strupr(Name);
          if (memcmp(Text,Name,Len) == 0) {
            Found = TRUE;
            break;
          }
        }
      }
    #endif
    dosfclose(&In);
    if (! Found)
      return(-2);
  }

  if (Name[0] == 0)
    return(-3);

  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  maxstrcpy(Password,strupr(parse(NULL)),sizeof(Password));
  Security = atoi(parse(NULL));

  switch (atoi(parse(NULL))) {
    case -1: Status.MakeUserSys = 1; break;
    case -2: Status.MakeUserSys = 2; break;
    case -3: Status.MakeUserSys = 3; break;
    case -4: Status.MakeUserSys = 4; break;
    default: Status.MakeUserSys = 0; break;
  }
  MakeDoorSys = (bool) (atoi(parse(NULL)) == -1);

  maxstrcpy(Path,parse(NULL),sizeof(Path));
  parse(NULL);  //lint !e534   throw away auto-login parameter
  maxstrcpy(Shell,parse(NULL),sizeof(Shell));

  ShellDoor = (Shell[0] != 'Y' && Shell[0] != 'S' && Shell[0] != 'F' ? 'N' : Shell[0]);
  #ifdef __OS2__
    if (ShellDoor == 'N' || ShellDoor == 'S')
      ShellDoor = 'Y';
  #endif

  #ifdef PCB152
    Status.DoorChargePerUse = atof(parse(NULL));
    Status.DoorChargePerMin = atof(parse(NULL));
    Charge = Status.DoorChargePerUse + Status.DoorChargePerMin;  /* assume 1 min */

    if (insufficientcredits(Charge,0))
      return(-4);
  #endif

  #ifdef __OS2__
    Status.Os2Door = (bool) (atoi(parse(NULL)) != 0);
  #endif

  if (Status.User != SYSOP) {
    if (Status.CurSecLevel < Security) {
      displaypcbtext(TXT_INSUFSECFORDOOR,NEWLINE|LOGIT);
      return(-4);
    }

    if (Password[0] != 0) {
      Temp[0] = 0;
      inputfield(Temp,TXT_PWRDFORDOOR,12,NEWLINE|LFBEFORE|FIELDLEN|UPCASE|ECHODOTS,NOHELP,mask_alphanum);
      if (Temp[0] == 0 || strcmp(Temp,Password) != 0) {
        displaypcbtext(TXT_BADPWRDFORDOOR,NEWLINE|LOGIT);
        return(-4);
      }
    }
  }


  if (filesflagged()) {
    displaypcbtext(TXT_FILESAREFLAGGED,NEWLINE|LFBEFORE|BELL);
    Temp[0] = NoChar;
    Temp[1] = 0;
    inputfield(Temp,TXT_CONTINUEDOOR,1,YESNO|NEWLINE|LFBEFORE|UPCASE|FIELDLEN,NOHELP,mask_yesno);
    if (Temp[0] != YesChar)
      return(-6);
  }

  return(executedoor(NumTokens,Name,Path,MakeDoorSys,ShellDoor));
}


void LIBENTRY opendoor(int NumTokens) {
  char Str[20];
  char *p;

  if (Status.UserRecNo == 0          ||
      Status.CurConf.DrsMenu[0] == 0 ||
      Status.CurConf.DrsFile[0] == 0) {
    displaypcbtext(TXT_NODOORSAVAILABLE,NEWLINE|LFBEFORE|BELL);
    return;
  }

  if (NumTokens == 0) {
    startdisplay(NOCHANGE);
    displayfile(Status.CurConf.DrsMenu,GRAPHICS|SECURITY|LANGUAGE|RUNMENU|RUNPPL);
    Str[0] = 0;
    inputfield(Str,TXT_DOORNUMBER,sizeof(Str)-1,NEWLINE|UPCASE|LFBEFORE,HLP_OPEN,mask_alphanum);
    if (Str[0] == 0)
      return;
    NumTokens = tokenize(Str);
  }

  p = getnexttoken();

  switch(searchdoorlist(p,NumTokens-1)) {
    case -1: displaypcbtext(TXT_NODOORSAVAILABLE,NEWLINE|LFBEFORE|LOGIT);
             break;
    case -2: displaypcbtext(TXT_INVALIDDOOR,NEWLINE|LOGIT);
             break;
    case -3: displaypcbtext(TXT_DOORNOTAVAILABLE,NEWLINE|LOGIT);
             break;
  }
}



/********************************************************************
*
*  Function:  logindoor()
*
*  Desc    :  This function is called during the login process.  It scans the
*             current doors.lst file for a door that has a matching security
*             level (matches the caller's current security level) and is
*             flagged as a "login door".
*
*             If such a door is found it is executed before arriving at the
*             Main Board Command prompt.  Otherwise nothing is executed.
*/

#ifndef PCB_DEMO              /* don't allow auto-login doors in the DEMO */
void LIBENTRY logindoor(void) {
  bool    MakeDoorSys;
  char    ShellDoor;
  bool    Found;
  int     Level;
  int     Num;
  char    Shell[10];
  char    Name[12];
  DOSFILE File;
  char    DoorList[66];
  char    Path[66];
  char    Buf[256];

  if (Status.UserRecNo == 0 || Status.CurConf.DrsFile[0] == 0)
    return;

  /* if not found but NovellSearch is specified then drop through anyway */
  if (checkforalternatelist(DoorList,Status.CurConf.DrsFile,0) == -1 && ! NovellSearch)
    return;

  MakeDoorSys = FALSE;
  ShellDoor = 'N';
  Found = FALSE;
  if (dosfopen(DoorList,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
    while (dosfgets(Buf,sizeof(Buf),&File) != -1) {
      maxstrcpy(Name,parse(Buf),sizeof(Name));
      parse(NULL);  //lint !e534    throw away the password field here
      Level = atoi(parse(NULL));  /* get the security level requirement */
      if (Level == 0 || Level == Status.CurSecLevel) { /* check for security level match */
        Num = atoi(parse(NULL));
        Status.MakeUserSys = (char) (Num == -1 ? 1 : Num == -2 ? 2 : Num == -3 ? 3 : 0);
        MakeDoorSys = (bool) (atoi(parse(NULL)) == -1);
        maxstrcpy(Path,parse(NULL),sizeof(Path));
        if (atoi(parse(NULL)) == -1) { /* check for auto login door */
          Found = TRUE;
          maxstrcpy(Shell,parse(NULL),sizeof(Shell));

          ShellDoor = (Shell[0] != 'Y' && Shell[0] != 'S' && Shell[0] != 'F' ? 'N' : Shell[0]);
          #ifdef __OS2__
            if (ShellDoor == 'N' || ShellDoor == 'S')
              ShellDoor = 'Y';
          #endif

          Status.DoorChargePerUse = atof(parse(NULL));
          Status.DoorChargePerMin = atof(parse(NULL));
          #ifdef __OS2__
            Status.Os2Door = (bool) (atoi(parse(NULL)) != 0);
          #endif
          break;
        }
      }
    }
    dosfclose(&File);
  }

  if (Found) {
    strcpy(Buf,"AUTOLOGIN DOOR");
    tokenize(Buf);      //lint !e534
    executedoor(1,Name,Path,MakeDoorSys,ShellDoor);   //lint !e534
  }
}
#endif


void LIBENTRY runshelldoor(void) {
  char *p;

  #ifdef __OS2__
  if (Status.Os2Door)
    p = "DOOR.CMD";
  else
  #endif
    p = "DOOR.BAT";

  savepath();
  #ifndef __OS2__
    if (Status.SwapDoor) {
      char  RetVal;
      if (swapdos(p,"",SHELLVIACOMMAND,&RetVal) != SWAP_OK)
        spawndos(p,"",SHELLVIACOMMAND,PcbData.PriorityShells,PcbData.MinimizeDoors & 1,PcbData.MinimizeDoors & 2,-1);  //lint !e534
      } else {
  #endif
      spawndos(p,"",SHELLVIACOMMAND,PcbData.PriorityShells,PcbData.MinimizeDoors & 1,PcbData.MinimizeDoors & 2,-1);    //lint !e534
  #ifndef __OS2__
    }
  #endif
  restorepath();
  unlink(PcbData.SwapPath);
  unlink(p);
}


#ifdef PCB152
void LIBENTRY scandoorsforcharges(char *DoorName) {
  char    Name[12];
  DOSFILE File;
  char    DoorList[66];
  char    Buf[256];

  Status.DoorChargePerUse = 0;
  Status.DoorChargePerMin = 0;

  /* if not found but NovellSearch is specified then drop through anyway */
  if (checkforalternatelist(DoorList,Status.CurConf.DrsFile,0) == -1 && ! NovellSearch)
    return;

  if (dosfopen(DoorList,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
    while (dosfgets(Buf,sizeof(Buf),&File) != -1) {
      maxstrcpy(Name,parse(Buf),sizeof(Name));
      if (strcmp(Name,DoorName) == 0) {
        parse(NULL);  /*lint !e534  throw away the password field here */
        parse(NULL);  /*lint !e534  throw away the security level */
        parse(NULL);  /*lint !e534  throw away make user sys */
        parse(NULL);  /*lint !e534  throw away make door sys */
        parse(NULL);  /*lint !e534  throw away path */
        parse(NULL);  /*lint !e534  throw away auto login */
        parse(NULL);  /*lint !e534  throw away shell */
        Status.DoorChargePerUse = atof(parse(NULL));
        Status.DoorChargePerMin = atof(parse(NULL));
        break;
      }
    }
    dosfclose(&File);
  }
}
#endif /* ifdef PCB152 */
