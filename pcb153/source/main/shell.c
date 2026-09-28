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

/* if compiling for the toolkit and INCL_SWAP is defined, then define */
/* "SWAP", otherwise define "NOSWAP".  If not compiling for the toolkit */
#ifdef __OS2__
  #define NOSWAP
#else
  #ifdef LIB
    #ifdef INCL_SWAP
      #define SWAP
    #else
      #define NOSWAP
    #endif
  #else
    #define SWAP
  #endif
#endif


#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <io.h>
  #include <dos.h>
  #include <direct.h>
  #include <borland.h>
  #include <malloc.h>
/*  #define setdisk _chdrive */
/*  #define chdir   _chdir */
#else
/*#include <dir.h> */
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#ifdef __OS2__
  #define  INCL_DOSSESMGR
  #include <os2.h>
#endif

#include "messages.h"

#include "system.h"
#include "session.hpp"
#include <errno.h>
#include <float.h>

#ifndef __OS2__
  #include <process.h>

  #ifdef _MSC_VER
    typedef void (__cdecl __interrupt __far * __cdecl intfunctype)();
  #else
    #ifdef __cplusplus
      typedef void interrupt (far * _CType intfunctype)(...);
    #else
      #if __TURBOC__ >= 0x300
        typedef void interrupt (far * _CType intfunctype)(void);
      #else
        typedef void interrupt (*intfunctype)(void);
      #endif
    #endif
  #endif
#endif

#ifdef SWAP
  #include "swap.h"
#endif

extern struct ffblk DTA;  /* declared in exist.c */

#define MSGBASE       254
#define USRFILE       255

typedef enum { INFORMCALLER, NOINFORM } informtype;
typedef enum { NOCLOSE, CLOSEUSERSFILE } closetype;

int  static OldDrive;
char static OldPath[66];

#ifdef SWAP
  int  static SwapError;
  char static SwapErrClass;
  char static SwapErrAction;
  char static SwapErrLocus;
#endif


void LIBENTRY savepath(void) {
  OldDrive = dosgetcurdrive();
  dosgetcurpath(OldDrive,OldPath,sizeof(OldPath));
}


void LIBENTRY restorepath(void) {
  dossetcurdrive(OldDrive);
  chdir(OldPath);
}

#ifdef LIB
/* When a door is executed, the CURRENT DIRECTORY may be changed to a location */
/* other than where PCBoard was running.  Because of this, the PCBDOS.BAT */
/* file may no longer be found simply by looking in the current directory! */
 /* */
/* So for Toolkit applications, the PCBDOS.BAT file must be executed by */
/* pre-pending the proper drive and path to get to PCBDOS.BAT.  The PcbDir */
/* variable is created by the PCBINIT.C module which is executed at startup. */
extern char PcbDir[];
static void LIBENTRY makecommand(char *RealCommand, char *Command) {
  buildstr(RealCommand,PcbDir,"PCBDOS ",Command,NULL);
}
#endif


#pragma argsused
char LIBENTRY spawndos(char *Command, char *Params, shelltype ShellType, int PriorityDelta, bool MinimizeSelf, bool MinimizeWindow, int WaitTime) {
  char RetVal;
  #ifdef __OS2__
    char        Title[20];
    char        Show[256];
    bool        WatchKbdClock;
    bool        WatchSessionClock;
  #else
    int         X;
    intfunctype array[0x10];
    intfunctype int3x[3];
  #endif
  char  Str[256];

  #ifdef __OS2__
    char  RealCommand[256];
    char  ProgName[256];
    char *p;
    char *SettingsFile = NULL;

/* default ProgType to an invalid value */
    ULONG ProgType = 0xFFFFFFFF;
/* default CmdProc to poing to CMD.EXE */
    char *CmdProc = ComSpec;

/* RealCommand will be what we "really" run (which may change down below */
/* if this is a DOS program).  But on the screen we will only show the */
/* contents of the "unchanged" Command variable so that sysops won't see */
/* the "PCBDOS.BAT" appear on the screen. */
    strcpy(RealCommand,Command);

/* The contents of Command *may* be more than just the name of the */
/* program to run (separated by spaces of course).  So truncate the */
/* command at the first space and assume that everything from the */
/* start is the real name of the program.  Do this so that the */
/* strstr() function can properly determine if this is a .BAT file and/or */
/* the DosQueryAppType() function can determine the application type */
    strcpy(ProgName,Command);
    if ((p = strchr(ProgName,' ')) != NULL)
      *p = 0;

/* if the command has .BAT in it, then assume a DOS batch file */
    if (strstr(ProgName,".BAT") != NULL) {
      ProgType = FAPPTYP_DOS;
    }

/* if we've already assumed a DOS batch file, or if the query to determine */
/* the application type is successful, then check for a DOS app and */
/* override how it is run to use command.com instead of cmd.exe */
    if (ProgType == FAPPTYP_DOS || DosQueryAppType(ProgName,&ProgType) == 0) {
/* check to see if it's a DOS application */
      if (ProgType == FAPPTYP_DOS) {
/* if so, override it and cause it to run via COMMAND.COM */
        ShellType = SHELLVIACOMMAND;
        CmdProc = DosComSpec;

/* Then have it run PCBDOS.BAT and pass the command and its parameters */
/* to PCBDOS.BAT and let PCBDOS.BAT run the real command.  This is done */
/* so that PCBDOS.BAT can set the PCBoard environment variables. */
/* NOTE:  Using "PCBDOS" instead of "PCBDOS.BAT" to shave 4 characters */
/* off the length of the string due to DOS's 128 character limit */
        #ifdef LIB
          makecommand(RealCommand,Command);
        #else
          buildstr(RealCommand,"PCBDOS ",Command,NULL);
        #endif

        #ifndef LIB
/* is it a door that we're shelling out to? */
          if (Status.DoorName[0] != 0 && strcmp(ProgName,"DOOR.BAT") == 0) {
/* yes, so let's use the configuration file associated with the */
/* door *not* the one associated with door.bat */
            strcpy(ProgName,Status.DoorPath);
          }
        #endif

/* check to see if a customized settings file exists for this program */
        if ((p = strrchr(ProgName,'.')) != NULL) {
/* the name of the file is the program name with the extension */
/* changed to .OS2 (i.e. MYPROG.EXE is changed to MYPROG.OS2) */
          strcpy(p,".OS2");
        } else
          strcat(ProgName,".OS2");

        if (fileexist(ProgName) != 255)
          SettingsFile = ProgName;

/* if there isn't a customized settings file, then use the default */
/* settings file (if there is one) */
        if (SettingsFile == NULL && DosSettingsFile[0] != 0 && fileexist(DosSettingsFile) != 255)
          SettingsFile = DosSettingsFile;
      }
    }
  #endif

  #ifndef __OS2__
    if (farcoreleft() < 64000L) {
      sprintf(Str,"Insufficient memory for SHELL operation! (need 64000, have %ld)",farcoreleft());
      #ifdef LIB
        println(Str);
      #else
        fastprintmove(0,0,Str,0x0C);
      #endif
      mysound(800,20);
      mydelay(80);
      return(8);  /* not enough free memory to shell */
    }
  #endif

  #if defined(KBD3) && ! defined(__OS2__)
    restoreint3();
  #endif

  #ifndef __OS2__
    for (X = 0; X < 0x10; X++)
      array[X] = getvect(X);

    int3x[0] = getvect(0x3D);
    int3x[1] = getvect(0x3E);
    int3x[2] = getvect(0x3F);
  #endif

  #ifdef __OS2__
    if (PcbData.PriorityNormal != 0)
      PriorityDelta -= PcbData.PriorityNormal;

    WatchKbdClock = Control.WatchKbdClock;
    WatchSessionClock = Control.WatchSessionClock;
    Control.WatchKbdClock = FALSE;
    Control.WatchSessionClock = FALSE;
  #endif

  switch(ShellType) {
    case SHELLVIACOMMAND:
                          #ifdef __OS2__
                            sprintf(Str,"/c %s %s",RealCommand,Params);

                            #ifndef LIB
                            if (Status.DoorName[0] != 0)
                              sprintf(Show,"Shelling out to: %s (%s)",Command,Status.DoorName);
                            else
                            #endif
                              sprintf(Show,"Shelling out to: %s %s",Command,Params);

                            sprintf(Title,"PCBShell (Node %d)",PcbData.NodeNum);
                            fastprintmove(0,3,Show,0x0F);

                            if (DebugLevel > 0) {
                              char Output[80];
                              sprintf(Output,"COMSPEC: %s",CmdProc);
                              writedebugrecord(Output);
                              sprintf(Output,"Shell: %s",Str);
                              writedebugrecord(Output);
                              if (SettingsFile != NULL) {
                                sprintf(Output,"DOS Settings File: %s",SettingsFile);
                                writedebugrecord(Output);
                              }
                            }

                            RetVal = (char) startsession(CmdProc,Str,Title,NULL,WaitTime,START_DEFAULT,NULL,SettingsFile,PriorityDelta,MinimizeSelf,MinimizeWindow);

                          #else

                            if (PcbData.EnvSize != 0)
                              sprintf(Str,"/e:%d /c",PcbData.EnvSize);
                            else
                              strcpy(Str,"/c");

                            if (DebugLevel > 0) {
                              char Output[80];
                              sprintf(Output,"COMSPEC: %s",ComSpec);
                              writedebugrecord(Output);
                              sprintf(Output,"Shell: %s %s",Str,Params);
                              writedebugrecord(Output);
                            }
                            RetVal = (char) spawnlp(P_WAIT,ComSpec,ComSpec,Str,Command,Params,NULL);

                          #endif
                          break;

    case SHELLDIRECT    :
                          #ifdef __OS2__
                            sprintf(Show,"Shelling out to: %s %s",Command,Params);
                            sprintf(Title,"PCBShell (Node %d)",PcbData.NodeNum);
                            fastprintmove(0,3,Show,0x0F);

                            if (DebugLevel > 0) {
                              sprintf(Show,"Shell: %s %s",Command,Params);
                              writedebugrecord(Show);
                              if (SettingsFile != NULL) {
                                sprintf(Show,"DOS Settings File: %s",SettingsFile);
                                writedebugrecord(Show);
                              }
                            }

                            RetVal = (char) startsession(Command,Params,Title,NULL,WaitTime,START_DEFAULT,NULL,SettingsFile,PriorityDelta,MinimizeSelf,MinimizeWindow);
                          #else
                            if (DebugLevel > 0) {
                              char Output[80];
                              sprintf(Output,"Shell: %s %s",Command,Params);
                              writedebugrecord(Output);
                            }
                            RetVal = (char) spawnlp(P_WAIT,Command,Command,Params,NULL);
                          #endif
                          break;

    default             : RetVal = 0;
                          break;
  }

  #ifdef __OS2__
    Control.WatchSessionClock = WatchSessionClock;
    if (WatchKbdClock)
      turnkbdtimeron();
  #endif

  #ifndef __OS2__
    for (X = 0; X < 0x10; X++)
      setvect(X,array[X]);

    setvect(0x3D,int3x[0]);
    setvect(0x3E,int3x[1]);
    setvect(0x3F,int3x[2]);
  #endif

  #if defined(KBD3) && ! defined(__OS2__)
    setint3();
  #endif

  #ifndef LIB
    _fpreset();
  #endif

  return(RetVal);
}


#ifdef SWAP
char LIBENTRY swapdos(char *Command, char *Params, shelltype ShellType, char *RetVal) {
  char SwapRetVal;
  int X;
  intfunctype array[0x10];
  intfunctype int3x[3];
  char Str[10];
  char CmdStr[128];

  #if defined(KBD3)
    restoreint3();
  #endif

  for (X = 0; X < 0x10; X++)
    array[X] = getvect(X);

  int3x[0] = getvect(0x3D);
  int3x[1] = getvect(0x3E);
  int3x[2] = getvect(0x3F);

  switch(ShellType) {
    case SHELLVIACOMMAND: if (PcbData.EnvSize != 0)
                            sprintf(Str,"/e:%d ",PcbData.EnvSize);
                          else
                            Str[0] = 0;
                          sprintf(CmdStr,"%s/c %s %s",Str,Command,Params);
                          SwapRetVal = swapenv(ComSpec,CmdStr,RetVal,PcbData.SwapPath);
                          /* save the swap error status */
                          SwapError     = ExtendedError;
                          SwapErrClass  = ExtendedClass;
                          SwapErrAction = ExtendedAction;
                          SwapErrLocus  = ExtendedLocus;
/*                        doswrite(0,"\r\nBACK FROM SWAP...",19); */
                          break;
    case SHELLDIRECT    : SwapRetVal = swapenv(Command,Params,RetVal,PcbData.SwapPath);
                          /* save the swap error status */
                          SwapError     = ExtendedError;
                          SwapErrClass  = ExtendedClass;
                          SwapErrAction = ExtendedAction;
                          SwapErrLocus  = ExtendedLocus;
/*                        doswrite(0,"\r\nBACK FROM SWAP...",19); */
                          break;

    default             : SwapRetVal = 0;
  }

  for (X = 0; X < 0x10; X++)
    setvect(X,array[X]);

  setvect(0x3D,int3x[0]);
  setvect(0x3E,int3x[1]);
  setvect(0x3F,int3x[2]);

  #if defined(KBD3)
    setint3();
  #endif

  _fpreset();
  return(SwapRetVal);
}
#endif


static char * _NEAR_ LIBENTRY geterrorstr(int Error) {
  if (Error != -1)
    return("");

  switch (errno) {
    case E2BIG  : return("Argument list too long");
    case EINVAL : return("Invalid argument");
    case ENOENT : return("Path or filename not found!");
    case ENOEXEC: return("Exec format error");
    case ENOMEM : return("Insufficient memory");
  }
  return("");
}


int LIBENTRY performshell(char *Command, char *Params, shelltype ShellType, int PriorityDelta, bool MinimizeSelf, bool MinimizeWindow, int WaitTime) {
  char RetVal;
  #ifdef SWAP
    char SwapRetVal;
    char Str[128];
  #endif

  savepath();
  doscommit(CallersLog);  /* write it all out to disk before shelling */

  #ifndef __OS2__
    uninstallhandlers();
  #endif

  #ifdef LIB
    {
      bool SaveModem;

      SaveModem = ModemOpened;
      closemodem(FALSE);

      #ifdef NOSWAP
        RetVal = spawndos(Command,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,WaitTime);
      #else
        SwapRetVal = SWAP_OK;
        if (PcbData.Swap) {
          if ((SwapRetVal = swapdos(Command,Params,ShellType,&RetVal)) != SWAP_OK) {
            writelog("SWAP FAILED - SHELLING INSTEAD",SPACERIGHT);
            /* try a normal shell instead */
            RetVal = spawndos(Command,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,WaitTime);
          }
        } else {
          RetVal = spawndos(Command,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,WaitTime);
        }
      #endif

      if (SaveModem) {
        reopenport();
        if (Asy.Online == REMOTE) {
          online();
          if (cdstillup() == 0) {
            Asy.LostCarrier = TRUE;
            if (PcbData.Packet)
              turnoffdtr();
            if (! Asy.IgnoreCDLoss) /* don't recycle if we're to ignore cd loss */
              loguseroff(ALOGOFF);
          }
        }
      }
    }
  #else  /* ifdef LIB */
    #ifdef __OS2__
      RetVal = spawndos(Command,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,WaitTime);
    #else
      SwapRetVal = SWAP_OK;
      if (PcbData.Swap) {
        if ((SwapRetVal = swapdos(Command,Params,ShellType,&RetVal)) != SWAP_OK) {
          writelog("SWAP FAILED - SHELLING INSTEAD",SPACERIGHT);
          /* try a normal shell instead */
          RetVal = spawndos(Command,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,WaitTime);
        }
      } else {
        RetVal = spawndos(Command,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,WaitTime);
      }
    #endif  /* ifdef __OS2__ */
  #endif  /* ifdef LIB */

  #ifndef __OS2__
    reinstallhandlers();
  #endif

  repositionlogpointer();
  restorepath();

  #ifdef SWAP
    unlink(PcbData.SwapPath);

    if (SwapRetVal != SWAP_OK) {
      sprintf(Str,"Swap Failure Code: %d (%d:%d:%d:%d) Pgm=(%s %s)",SwapRetVal,SwapError,SwapErrClass,SwapErrLocus,SwapErrAction,Command,Params);
      writelog(Str,SPACERIGHT);
      if (SwapRetVal != SWAP_NO_EXEC)
        PcbData.Swap = FALSE;  /* we had an error so don't let it try to swap any more! */
    }

    #ifndef LIB
      if (DebugLevel > 0) {
        sprintf(Str,"Shell=%d %s Pgm=(%s %s)",RetVal,geterrorstr(RetVal),Command,Params);
        writedebugrecord(Str);
      }
    #endif
  #endif

  return(RetVal);
}


#pragma warn -par
static int _NEAR_ LIBENTRY shelltocommand(char *Command, char *Params, shelltype ShellType, informtype Action, closetype Close, int PriorityDelta, bool MinimizeSelf, bool MinimizeWindow, bool CloseModem) {
  #ifdef COMM
  bool         ModemWasOpened;
  #endif
  int          RetVal;
  char         OldX;
  char         OldY;
  int          SaveCursor;
  int          Saved;
  char         CmdPath[128];
  #ifdef COMM
  pcbtexttype  Buf;
  #endif

  SaveCursor = getcursor();
  Saved = memsavescreen();
  OldX = awherex();
  OldY = awherey();

  #ifdef COMM
    if (Action == INFORMCALLER && Asy.Online == REMOTE) {
      #ifndef LIB
      if (Status.FullScreen)
        printcls();
      else
      #endif
        savescrnbuffer();
      newline();
      #ifndef LIB
        print(Status.SysopName);
      #endif
      displaypcbtext(TXT_SYSOPEXITEDTODOS,DEFAULTS);
      getpcbtext(TXT_SYSOPEXITEDTODOS,&Buf);
    }
  #endif

  strcpy(CmdPath,Command);
  if (ShellType == SHELLDIRECT)
    srchpath(CmdPath);  /*lint !e534 */

  #ifndef LIB
    if (Close == CLOSEUSERSFILE) {
      closeusersfile();
      closeusersinffile();
    }
  #endif

  cls();
  fastprint(31,0,"(PCBoard Shell)",0x0F);
  agotoxy(0,1);
  setcursor(CUR_NORMAL);

  #ifdef __OS2__
    redisplaystatusline();
  #endif

  #ifdef COMM
/* record whether or not the modem was opened */
    ModemWasOpened = ModemOpened;

    if (ModemWasOpened) {
/* if the modem was opened, and we're using the "A"sync mode, then we */
/* have a live interrupt handler installed so we need to close the modem */
/* no matter what because we may be swapping out. */
      if (ModemFixupsDone == 'A')
        CloseModem = TRUE;

/* if the modem is to be closed, then close it now but leave DTR up */
      if (CloseModem)
        closemodem(FALSE);
    }
  #endif

  RetVal =  performshell(CmdPath,Params,ShellType,PriorityDelta,MinimizeSelf,MinimizeWindow,-1);

  if (Status.ForceScreenOff) {
    turndisplayon(FALSE);
    cls();
    turndisplayoff(FALSE);
    Status.ForceScreenOff = TRUE;  /* turndisplayon() set it to FALSE */
  }

/*setkbdtimer(Asy.Online != OFFLINE ? Control.KbdTimer : THREEMINUTES); */
  renewkbdtimer();
  memrestorescreen(Saved,FREESCREEN);
  agotoxy(OldX,OldY);
  setcursor(SaveCursor);
  if (Control.GraphicsMode) { /* force it to reset the color! */
    asetcolor(0);
/*  printdefcolor();    WE CAN'T DO THIS YET, THE MODEM ISN'T OPEN! */
  }

  #ifndef LIB
    /* do this BEFORE opening the modem or  */
    /* risk carrier loss detection with the */
    /* users file closed - causes an error  */
    /* during the recycle with it closed    */
    if (Close == CLOSEUSERSFILE) {
      openusersfile();       /*lint !e534 */
      openusersinffile();    /*lint !e534 */
    }
  #endif

  #ifdef COMM
/* was the modem previously open? */
    if (ModemWasOpened) {
/* yes, it was open, so did we close it? */
      if (CloseModem) {
/* yes, it was closed so now reopen it */
        reopenport();
      }
/* now check to see if we are still online */
      if (Asy.Online == REMOTE) {
        online();                   /*lint !e534 this just sets the state */
        if (cdstillup() == 0) {
          Asy.LostCarrier = TRUE;
          if (PcbData.Packet)
            turnoffdtr();
          if (! Asy.IgnoreCDLoss) { /* don't recycle if we're to ignore cd loss */
            #ifndef LIB
              /* if a USERS.SYS file was created BEFORE the shell, then it */
              /* might have the update flag turned on .. better read it back */
              /* in BEFORE we recycle because the $$LOGOFF.BAT file might */
              /* be dependent on the USERS.SYS file *already* having been */
              /* processed.  Andy Keeve's CMS is a case in point.  The LOGON */
              /* batch file updates users.sys.  If carrier is lost, and the */
              /* logoff.bat file is processed *before* users.sys is read in */
              /* from the next cycle, then we're out of sync.  Read it now */
              if (fileexist("USERS.SYS") != 255) {
                getuserrecord(TRUE,FALSE);
                unlink("USERS.SYS");
              }
            #endif
            loguseroff(ALOGOFF);
          }
        }
        printdefcolor();
        if (Action == INFORMCALLER) {
          displaypcbtext(TXT_THANKSFORWAITING,LFBEFORE);
          print(Status.FirstName);
          newline();
          #ifndef LIB
            if (! Status.FullScreen)
              restscrnbuffer();
          #else
            restscrnbuffer();
          #endif
        }
      } else printdefcolor();
    } else printdefcolor();
  #else
    printdefcolor();
  #endif
  return(RetVal);
}
#pragma warn +par


static void _NEAR_ LIBENTRY showshellerror(int Error) {
  char *p;

  if (Error != -1)
    return;

  p = geterrorstr(Error);

  #ifdef LIB
    println(p);
  #else
  {
    char X,Y;
    int  Saved;

    Saved = memsavescreen();
    X = awherex();
    Y = awherey();
    boxcls(20,11,61,17,0x07,SINGLE);
    fastcenter(13,p,0x0A);
    fastcenter(15,"press any key to continue",0x0A);
    agotoxy(52,15);
    beep();
    bgetkey(0);         /*lint !e534 read the key and throw it away */
    agotoxy(2,1);
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(X,Y);
  }
  #endif
}


void LIBENTRY shelltodos(void) {
  long     StartTime;
  long     StopTime;
  char     Str[10];

  StartTime = gettimer(6);

  #ifndef LIB
  writeusernetstatus(OUTINDOS,NULL);
  #endif

  #ifndef __OS2__  /* don't use environment size for OS/2 shells */
  if (PcbData.EnvSize != 0)
    sprintf(Str,"/e:%d",PcbData.EnvSize);
  else
  #endif
    Str[0] = 0;

  #if defined(__OS2__) && ! defined(LIB)
    Status.DisableUserNetUpdate = TRUE;
  #endif

  showshellerror(shelltocommand(ComSpec,Str,SHELLDIRECT,INFORMCALLER,NOCLOSE,PcbData.PriorityShells,TRUE,FALSE,TRUE));

  #if defined(__OS2__) && ! defined(LIB)
    Status.DisableUserNetUpdate = FALSE;
  #endif

  #ifndef LIB
    usernetavailable();
  #endif

  StopTime = gettimer(6);
  #ifdef __OS2__
    addtime((-StopTime+StartTime)/ONESECOND,CREDTIME);
  #else
    addtime(((-StopTime+StartTime)*10)/182,CREDTIME);
  #endif
}


#ifndef LIB
void LIBENTRY runexportbat(char *FileName) {
  char BatchFile[66];

  if (findbat(BatchFile,"EXPORT") != -1) {
    #ifdef __OS2__
      Status.DisableUserNetUpdate = TRUE;
    #endif

    shelltocommand(BatchFile,FileName,SHELLVIACOMMAND,NOINFORM,CLOSEUSERSFILE,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,FALSE);  /*lint !e534 */

    #ifdef __OS2__
      Status.DisableUserNetUpdate = FALSE;
    #endif
  }
}


void LIBENTRY runcdlostbat(void) {
  char BatchFile[66];

  Asy.IgnoreCDLoss = TRUE;
  if (findbat(BatchFile,"CDLOST") != -1) {
    #ifdef __OS2__
      Status.DisableUserNetUpdate = TRUE;
    #endif

    shelltocommand(BatchFile,"",SHELLVIACOMMAND,NOINFORM,CLOSEUSERSFILE,PcbData.PriorityShells,PcbData.MinimizeLogOnOff & 1,PcbData.MinimizeLogOnOff & 2,FALSE);  /*lint !e534 */

    #ifdef __OS2__
      Status.DisableUserNetUpdate = FALSE;
    #endif
  }
}


static void _NEAR_ LIBENTRY runbatchfile(char *File, bool Logon) {
  bool Save;
  char BatchFile[66];

  if (findbat(BatchFile,File) != -1) {
    if (Logon) {
      if (Status.UserRecNo != 0) {
        convertdatatoread(&UsersData,&UsersRead);
        putuserrecord(CHECKDIRTY);
      }
      if (PcbData.UserSysDuringBat)
        makeusersys();
    }

    #ifdef __OS2__
      Status.DisableUserNetUpdate = TRUE;
    #endif

    Save = PcbData.Swap;
    PcbData.Swap = PcbData.SwapDuringBat;
    showshellerror(shelltocommand(BatchFile,"",SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityShells,PcbData.MinimizeLogOnOff & 1,PcbData.MinimizeLogOnOff & 2,TRUE));
    PcbData.Swap = Save;

    #ifdef __OS2__
      Status.DisableUserNetUpdate = FALSE;
    #endif

    if (Logon) {                  /* log is closed during $$LOGOFF.BAT    */
      repositionlogpointer();     /* so reposition only after $$LOGON.BAT */
      if (PcbData.UserSysDuringBat) {
        if (fileexist("USERS.SYS") != 255) {
          getuserrecord(TRUE,FALSE);
          unlink("USERS.SYS");
        }
      }
      readpcboardsys();  /* in case time left was changed, read in pcboard.sys */
    }
  }
}


void LIBENTRY checkforlogoffbat(void) {
  runbatchfile("$$LOGOFF",FALSE);
}


void LIBENTRY checkforlogonbat(void) {
  runbatchfile("$$LOGON",TRUE);
}


void LIBENTRY runslowdrivebat(char *FileName) {
  char BatchFile[66];
  char NewName[66];
  char *p;

  strcpy(NewName,PcbData.SlowDriveBat);
  if ((p = strstr(NewName,".BAT")) != NULL) {
    *p = 0;
    if (findbat(BatchFile,NewName) == -1)
      return;
  } else {
    strcpy(BatchFile,PcbData.SlowDriveBat);
    if (srchpath(BatchFile) == -1 || DTA.ff_fsize == 0)
      return;
  }

  #ifdef __OS2__
    Status.DisableUserNetUpdate = TRUE;
  #endif

  showshellerror(shelltocommand(BatchFile,FileName,SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,FALSE));

  #ifdef __OS2__
    Status.DisableUserNetUpdate = FALSE;
  #endif
}


void LIBENTRY runfiler(void) {
  char PcbDataFile[66];

  #ifndef __OS2__
    if (Asy.Online == OFFLINE) {
      Status.Logoff = RUNFILER;
      recycle();
    }
  #endif

  sprintf(PcbDataFile,"/FILE:%s",DatFile);
  showshellerror(shelltocommand("PCBFILER.EXE",PcbDataFile,SHELLDIRECT,INFORMCALLER,CLOSEUSERSFILE,0,TRUE,FALSE,FALSE));
}

void LIBENTRY runmoni(void) {
  #ifdef __OS2__
    showshellerror(shelltocommand("PCBMONI2.EXE",PcbData.NetFile,SHELLDIRECT,INFORMCALLER,NOCLOSE,0,TRUE,FALSE,FALSE));
  #else
    showshellerror(shelltocommand("PCBMONI.EXE",PcbData.NetFile,SHELLDIRECT,INFORMCALLER,NOCLOSE,0,TRUE,FALSE,FALSE));
  #endif
}

void LIBENTRY runsysmgr(void) {
  char PcbDataFile[66];

  #ifndef __OS2__
    if (Asy.Online == OFFLINE) {
      Status.Logoff = RUNSM;
      recycle();
    }
  #endif

  sprintf(PcbDataFile,"/FILE:%s",DatFile);
  showshellerror(shelltocommand("PCBSM.EXE",PcbDataFile,SHELLDIRECT,INFORMCALLER,CLOSEUSERSFILE,0,TRUE,FALSE,FALSE));
}

void LIBENTRY runsetup(void) {
  #ifndef __OS2__
    Status.Logoff = RUNSETUP;
    recycle();
  #else
    char PcbDataFile[66];
    sprintf(PcbDataFile,"/FILE:%s",DatFile);
    showshellerror(shelltocommand("PCBSETUP.EXE",PcbDataFile,SHELLDIRECT,INFORMCALLER,CLOSEUSERSFILE,0,TRUE,FALSE,FALSE));
  #endif
}


#pragma warn -par
void LIBENTRY doscommand(int NumTokens) {
  int  RetVal;
  char Str[100];

  displaypcbtext(TXT_DOSFUNCTION,NEWLINE);
  Str[0] = 0;
  inputfieldstr(Str,"",PCB_YELLOW,sizeof(Str)-1,NEWLINE|UPCASE,NOHELP,mask_message);
  if (Str[0] == 0)
    return;

  #ifdef __OS2__
    Status.DisableUserNetUpdate = TRUE;
  #endif

  RetVal = shelltocommand(Str,"",SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,TRUE);

  #ifdef __OS2__
    Status.DisableUserNetUpdate = FALSE;
  #endif

  if (Asy.Online == LOCAL)
    showshellerror(RetVal);

  maxstrcpy(Status.DisplayText,Str,sizeof(Status.DisplayText));
  displaypcbtext(TXT_SHELLCOMPLETED,NEWLINE|LFBEFORE|LOGIT);
}
#pragma warn +par


/********************************************************************
*
*  Function:  findfileindlpath()
*
*  Desc    :  Searches the download paths for an exact match on the filename
*             passed to it (no wildcards used - and no extension to the name
*             is added for search purposes).
*
*  Returns :   0, 1, 2 or 3 if the file was found AND is cleared for security
*             -1 if a security/password failure occured
*             -2 if the file was not found
*/

static int _NEAR_ LIBENTRY findfileindlpath(char *FullPath, char *FileName) {
  if (scandlpaths(CHECKDNLDS|REPORTONLY,FileName) > 0) {
    buildstr(FullPath,LastPathFound,FileName,NULL);
    return(fsecokay(FullPath,FileName,CHECKVIEW,NULL));
  }
  return(-2);
}


int LIBENTRY performfileview(char *FullPath, char *Name) {
  char BatPath[66];
  char Command[80];
  char Str[80];

  sprintf(Command,"%s %d",FullPath,Asy.ComPortNumber);
/*maxstrcpy(Command,FullPath,sizeof(Command)); */
/*addchar(Command,' '); */
/*addchar(Command,(char) (Asy.ComPortNumber+'0')); */

  strcpy(BatPath,PcbData.ViewBatch);
  if (srchpath(BatPath) == -1) {
    maxstrcpy(Status.DisplayText,PcbData.ViewBatch,sizeof(Status.DisplayText));
    displaypcbtext(TXT_NOTFOUNDONDISK,NEWLINE|LFBEFORE|LOGIT|BELL);
    return(0);  /* force it to exit */
  }

  sprintf(Str,"F V %s",Name);
  writeusernetstatus(FILEVIEW,Str);
  displaycmdfile("PREVIEW");

/* set FILEVIEWSHELL so that updateuserrecord() will work correctly */
  Status.Logoff = FILEVIEWSHELL;
  updateuserrecord();  /* save the user information (so that on return from the shell it won't lose any updates) */
  makepcboardsys();  /* make the pcboard.sys file */
  makeusersys();  /* make the users.sys file */
  Status.Logoff = LOGON;

  if (shelltocommand(BatPath,Command,SHELLVIACOMMAND,NOINFORM,CLOSEUSERSFILE,PcbData.PriorityShells,PcbData.MinimizeFileView & 1,PcbData.MinimizeFileView & 2,TRUE) == -1 || fileexist("PCBVIEW.TXT") == 255) {
    Status.CreditMinutes = 0;  /* reset to 0 since pcboard.sys will have current credits */
    readpcboardsys();  /* read the pcboard.sys and users.sys files */
    usernetavailable();
    maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
    displaypcbtext(TXT_ERRORVIEWINGFILE,NEWLINE|LFAFTER|LOGIT);
    return(-1);  /* force it to retry the operation */
  }

  Status.CreditMinutes = 0;  /* reset to 0 since pcboard.sys will have current credits */
  readpcboardsys();  /* read the pcboard.sys and users.sys files */
  startdisplay(NOCHANGE);
  displayfile("PCBVIEW.TXT",NOALTERNATE|DISABLESUBFILES);
  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  displaypcbtext(TXT_VIEWEXECUTEDONFILE,NEWLINE|LFBEFORE|LOGIT);
  maxstrcpy(Status.LastView,Name,sizeof(Status.LastView));
  unlink("PCBVIEW.TXT");
  usernetavailable();
  return(0);
}


void LIBENTRY viewfile(int NumTokens) {
  char *p;
  char Name[13];
  char FullPath[66];
  char Str[128];

  if (NumTokens == 0) {
retry:
    Name[0] = 0;
    inputfield(Name,TXT_ARCVIEWFILENAME,sizeof(Name)-1,NEWLINE|UPCASE|FIELDLEN,NOHELP,mask_filename);
    if (Name[0] == 0)
      return;
  } else {
    maxstrcpy(Name,getnexttoken(),sizeof(Name));
  }

  if (strchr(Name,'.') == NULL && PcbData.ViewExt[0] != 0) {
    Name[8] = 0;  /* guarantee that it's 8 characters or less */
    addchar(Name,'.');
    strcat(Name,PcbData.ViewExt);
  }

  if (invalidfilename(Name,INFORM))
    goto retry;

  displaypcbtext(TXT_CHECKINGARCVIEW,NEWLINE);
  strcpy(Str,Name);
  Status.LastView[0] = 0;
  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));

  switch (findfileindlpath(FullPath,Str)) {
    case  3:
    case  2:
    case  1:
    case  0: p = &Name[strlen(Name)];       /* avoid problems with     */
             if (p != Str && *(p-1) == '.') /* filenames such as       */
               *(p-1) = 0;                  /* "BLT1." by removing "." */
             if ((p = strstr(Str,Name)) == NULL) {
               displaypcbtext(TXT_ERRORVIEWINGFILE,NEWLINE|LFAFTER|LOGIT);
               goto retry;
             }
             maxstrcpy(Name,p,sizeof(Name));
             if (performfileview(FullPath,Name) == -1)
               goto retry;

             stuffkbdwithfile("PCBSTUFF.KBD");
             break;
    case -2: /* displaypcbtext(TXT_NOTFOUNDONDISK,NEWLINE|LFBEFORE|LOGIT|BELL); */
    case -1: goto retry;
  }
}


int LIBENTRY verifyfile(char *FullName, char *Name, char *UpDescName) {
  char Params[128];

  if (TestBat[0] == 0) {
    #ifdef __OS2__
      writelog("PCBTEST.CMD/PCBTEST.BAT not found",SPACERIGHT);
    #else
      writelog("PCBTEST.BAT not found",SPACERIGHT);
    #endif
    return(0);
  }

  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  displaypcbtext(TXT_VERIFYINGFILE,DEFAULTS);

  maxstrcpy(Params,FullName,sizeof(Params));
  addchar(Params,' ');
  if (UpDescName != NULL) {
    strcat(Params,(Status.FileAttach ? "ATTACH " : "UPLOAD "));
    strcat(Params,UpDescName);
    addchar(Params,' ');
    strcat(Params,Name);
  } else {
    strcat(Params,"TEST TEST ");
    strcat(Params,Name);
  }

  unlink("PCBFAIL.TXT");
  unlink("PCBPASS.TXT");
  #ifdef COMM
    waitforempty(TWOSECONDS);
  #endif

  makepcboardsys();

  #ifdef __OS2__
    Status.DisableUserNetUpdate = TRUE;
  #endif

  if (shelltocommand(TestBat,Params,SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityShells,PcbData.MinimizeFileTest & 1,PcbData.MinimizeFileTest & 2,TRUE) == -1 || fileexist("PCBFAIL.TXT") == 255) {

    #ifdef __OS2__
      Status.DisableUserNetUpdate = FALSE;
    #endif

    renewkbdtimer();
    Display.AutoMore = TRUE;
    Display.NumLinesPrinted = 0;
    startdisplay(FORCENONSTOP);
    displaypcbtext(TXT_PASSED,NEWLINE);
    if (fileexist("PCBPASS.TXT") != 255) {
      printdefcolor();
      displayfile("PCBPASS.TXT",NOALTERNATE|DISABLESUBFILES);
      unlink("PCBPASS.TXT");
    }
    Display.AutoMore = FALSE;
    return(0);
  }

  #ifdef __OS2__
    Status.DisableUserNetUpdate = FALSE;
  #endif

  renewkbdtimer();
  Display.AutoMore = TRUE;
  Display.NumLinesPrinted = 0;
  startdisplay(FORCENONSTOP);
  displaypcbtext(TXT_FAILED,NEWLINE);
  printdefcolor();
  displayfile("PCBFAIL.TXT",NOALTERNATE|DISABLESUBFILES);
  Display.AutoMore = FALSE;
  unlink("PCBFAIL.TXT");
  logsystext(TXT_FILEVERIFYFAILED,SPACERIGHT);
  return(-1);
}


void LIBENTRY verifyfilecommand(int NumTokens) {
  char *p;
  char Name[13];
  char PathList[66];
  char FullPath[66];
  char Str[128];

  if (checkforalternatelist(PathList,Status.CurConf.PthNameLoc,0) == -1) {
    displaypcbtext(TXT_NODIRSAVAILABLE,NEWLINE|LFBEFORE);
    return;
  }

  if (NumTokens == 0) {
retry:
    Name[0] = 0;
    inputfield(Name,TXT_TESTFILENAME,sizeof(Name)-1,NEWLINE|UPCASE|FIELDLEN,NOHELP,mask_filename);
    if (Name[0] == 0)
      return;
  } else {
    maxstrcpy(Name,getnexttoken(),sizeof(Name));
  }

  if (strchr(Name,'.') == NULL && PcbData.ViewExt[0] != 0) {
    Name[8] = 0;  /* guarantee that it's 8 characters or less */
    addchar(Name,'.');
    strcat(Name,PcbData.ViewExt);
  }

  if (invalidfilename(Name,INFORM))
    goto retry;

  maxstrcpy(Str,Name,sizeof(Str));
  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));

  switch (findfileindlpath(FullPath,Str)) {
    case  3:
    case  2:
    case  1:
    case  0: p = &Name[strlen(Name)];       /* avoid problems with     */
             if (p != Str && *(p-1) == '.') /* filenames such as       */
               *(p-1) = 0;                  /* "BLT1." by removing "." */
             if ((p = strstr(Str,Name)) == NULL) {
               displaypcbtext(TXT_ERRORVIEWINGFILE,NEWLINE|LFAFTER|LOGIT);
               goto retry;
             }
             maxstrcpy(Name,p,sizeof(Name));
             newline();
             verifyfile(FullPath,Name,NULL);   /*lint !e534 */
             break;
    case -2: /* displaypcbtext(TXT_NOTFOUNDONDISK,NEWLINE|LFBEFORE|LOGIT|BELL); */
    case -1: goto retry;
  }
}


void LIBENTRY zipcapfile(void) {
  int  RetVal;
  char Ext[5];
  char Name[66];
  char Command[128];

  if (PcbData.ViewExt[0] != 0) {
    displaypcbtext(TXT_COMPRESSING,NEWLINE);
    Ext[0] = '.';
    strcpy(&Ext[1],PcbData.ViewExt);
    strcpy(Name,Status.CapFileName);
    substitute(Name,".CAP",Ext,sizeof(Name));
    unlink(Name);
    sprintf(Command,"%s %s",Name,Status.CapFileName);

    #ifdef __OS2__
      Status.DisableUserNetUpdate = TRUE;
    #endif

    RetVal = shelltocommand(CompressBat,Command,SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityCompress,PcbData.MinimizeCompress & 1,PcbData.MinimizeCompress & 2,FALSE);

    #ifdef __OS2__
      Status.DisableUserNetUpdate = FALSE;
    #endif

    if (RetVal != -1 && fileexist(Name) != 255) {
      unlink(Status.CapFileName);
      strcpy(Status.CapFileName,Name);
    } else {
      displaypcbtext(TXT_ERRORCOMPRESSING,NEWLINE|LOGIT);
      unlink(Name);  /* in case of error remove any stray ZIP files */
    }
  }
}


int LIBENTRY zipqwkfile(void) {
  char Command[256];

  displaypcbtext(TXT_CREATINGQWK,NEWLINE);
  buildstr(Status.CapFileName,PcbData.TmpLoc,getqwkroot(),".QWK",NULL);

  if (Status.QwkCapCount > 0)
    Status.CapFileName[strlen(Status.CapFileName)-1] = (char) ('0' + Status.QwkCapCount - 1);

  #ifdef __OS2__
    Status.DisableUserNetUpdate = TRUE;
  #endif

  sprintf(Command,"%s %s %s %sPCBQWK.LST","COMPRESS",Status.CapFileName,PcbData.TmpLoc,PcbData.TmpLoc);
  shelltocommand(QwkBat,Command,SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityCompress,PcbData.MinimizeCompress & 1,PcbData.MinimizeCompress & 2,FALSE);  /*lint !e534 */

  #ifdef __OS2__
    Status.DisableUserNetUpdate = FALSE;
  #endif

  if (fileexist(Status.CapFileName) == 255) {
    displaypcbtext(TXT_ERRORCOMPRESSING,NEWLINE|LOGIT);
    return(-1);
  }
  return(0);
}


int LIBENTRY extractmessages(char *RepPacket, char *Msgs) {
  char Command[256];

  displaypcbtext(TXT_EXTRACTINGMSGS,NEWLINE|LFBEFORE);
  buildstr(Status.CapFileName,PcbData.TmpLoc,PcbData.CapFile,".QWK",NULL);

  #ifdef __OS2__
    Status.DisableUserNetUpdate = TRUE;
  #endif

  sprintf(Command,"%s %s %s","EXTRACT",RepPacket,PcbData.TmpLoc);
  shelltocommand(QwkBat,Command,SHELLVIACOMMAND,NOINFORM,NOCLOSE,PcbData.PriorityCompress,PcbData.MinimizeCompress & 1,PcbData.MinimizeCompress & 2,FALSE);  /*lint !e534 */

  #ifdef __OS2__
    Status.DisableUserNetUpdate = FALSE;
  #endif

  unlink(RepPacket);
  if (fileexist(Msgs) == 255) {
    displaypcbtext(TXT_ERROREXTRACTING,NEWLINE|LOGIT);
    return(-1);
  }
  return(0);
}


char PackPgm[66];
char PackCmd[80];


static bool _NEAR_ LIBENTRY getyn(int TextNum, bool DefaultTrue) {
  char Str[2];

  Str[0] = (DefaultTrue ? YesChar : NoChar);
  Str[1] = 0;
  inputfield(Str,TextNum,1,YESNO|FIELDLEN|UPCASE|NEWLINE,NOHELP,mask_yesno);
  return((bool) (Str[0] == YesChar || (Str[0] == 0 && DefaultTrue)));
}


static void _NEAR_ LIBENTRY pack(int NumTokens, int TextNum, char PackType) {
  char Str[10];
  char *p;

  #ifdef __OS2__
    strcpy(PackPgm,PackType == MSGBASE ? "PCBPACK2.EXE" : "PCBSM.EXE");
  #else
    strcpy(PackPgm,PackType == MSGBASE ? "PCBPACK.EXE" : "PCBSM.EXE");
  #endif

  if (srchpath(PackPgm) == -1) {
    displaypcbtext(TXT_PACKMODULEMISSING,NEWLINE);
    return;
  }

  if (NumTokens == 0) {
    newline();
    if (! getyn(TextNum,FALSE))
      return;
  } else {
    p = getnexttoken();
    if (*p != YesChar)
      return;
  }

  switch (PackType) {
    case MSGBASE: sprintf(PackCmd,"/FI:%s /AR:%u /FA",DatFile,Status.Conference);

                  if (getyn(TXT_GENERATENEWINDEX,FALSE)) {
                    strcat(PackCmd," /IN");
                    if (OldIndex || Status.CurConf.OldIndex)
                      strcat(PackCmd," /OL");
                    break;
                  }

                  strcpy(Str,"010180");
                  countrydate2(Str);
                  inputfield(Str,TXT_PURGEOLDERTHAN,6,FIELDLEN|NEWLINE,NOHELP,mask_numbers);
                  if (strlen(Str) == 6) {
                    uncountrydate2(Str);
                    if (strcmp(Str,"010180") != 0) {
                      strcat(PackCmd," /DA:");
                      strcat(PackCmd,Str);
                    }
                  }

                  if (getyn(TXT_PURGEPRIVRECEIVED,FALSE))
                    strcat(PackCmd," /PU");

                  if (getyn(TXT_RENUMBERDURINGPACK,FALSE)) {
                    strcpy(Str,"1");
                    inputfield(Str,TXT_NEWLOWMSGNUM,8,FIELDLEN|NEWLINE,NOHELP,mask_numbers);
                    if (atol(Str) > 0) {
                      strcat(PackCmd," /RE:");
                      strcat(PackCmd,Str);
                    }
                  }
                  break;

    case USRFILE: sprintf(PackCmd,"/FILE:%s /PURGE",DatFile);

                  if (getyn(TXT_KEEPLOCKEDOUT,TRUE))
                    strcat(PackCmd,";LOCKOUT");
                  else
                    strcat(PackCmd,";REMOVE");

                  strcpy(Str,"010180");
                  countrydate2(Str);
                  inputfield(Str,TXT_PURGEOLDERTHAN,6,FIELDLEN|NEWLINE,NOHELP,mask_numbers);
                  if (strlen(Str) == 6) {
                    uncountrydate2(Str);
                    if (strcmp(Str,"010180") != 0) {
                      strcat(PackCmd,";DAYS:");
                      strcat(PackCmd,Str);
                    }
                  }

                  strcpy(Str,"100");
                  inputfield(Str,TXT_KEEPSECURITY,3,FIELDLEN|NEWLINE,NOHELP,mask_numbers);
                  if (atoi(Str) > 0) {
                    strcat(PackCmd,";KEEP:");
                    strcat(PackCmd,Str);
                  }
                  break;
  }

  newline();
  printdefcolor();
  print(PackPgm);
  print(" ");
  println(PackCmd);

  displaypcbtext(TXT_LOADINGPACKMODULE,LFBEFORE);
  Status.PackFlag   = PackType;
  Status.Logoff     = REMOTEDOS;
  Status.ErrorLevel = EXIT_PACK;
  recycle();
}


void LIBENTRY packmessages(int NumTokens) {
  pack(NumTokens,TXT_PACKTHEMSGBASE,MSGBASE);
}


void LIBENTRY packusers(int NumTokens) {
  if (Status.UserRecNo != 1) {
    displaypcbtext(TXT_NOTRECNUMONE,NEWLINE);
    return;
  }
  pack(NumTokens,TXT_PACKTHEUSERSFILE,USRFILE);
}
#endif
