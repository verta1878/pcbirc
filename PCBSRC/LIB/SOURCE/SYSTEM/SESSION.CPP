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


#ifdef __BORLANDC__
  #include <dir.h>
#else
  #include <direct.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <types.hpp>
#include <semafore.hpp>
#include <system.h>
#include <session.hpp>
#include <threads.h>
#include <dosfunc.h>

#define  INCL_DOSEXCEPTIONS
#define  INCL_DOSSEMAPHORES
#define  INCL_DOSPROCESS
#define  INCL_DOSQUEUES
#define  INCL_DOSSESMGR
#define  INCL_WINSWITCHLIST
#include <os2.h>

#ifdef DEBUG
  #include <memcheck.h>
#endif

#define PIB_FULLSCREEN 0
#define PIB_WINDOWED   2

typedef struct {
  int Time;   // number of milliseconds to wait
  int Pid;    // process id of spawned session
} waittype;


void THREADFUNC waitthread(void *Param) {
  waittype *WaitPacket = (waittype *) Param;

  // put this thread to sleep for the specified number of milliseconds
  DosSleep(WaitPacket->Time);

  // When it wakes up, try to shut down the spawned process.
  // NOTE:  We may not get this far because if the spawned process exits
  // first this thread will be killed while it sleeps.
  DosSendSignalException(WaitPacket->Pid,XCPT_SIGNAL_BREAK);

  // Put this thread back to sleep for 5 seconds (it may be killed during
  // this period of time).
  DosSleep(5000);

  // try a different method of breaking out of the program
  DosSendSignalException(WaitPacket->Pid,XCPT_SIGNAL_INTR);

  // Put this thread back to sleep for 25 seconds (it may be killed during
  // this period of time).
  DosSleep(25000);

  // If we're still alive by the time we get here, then chances are pretty
  // good that the BREAK signal didn't do the trick of forcing the spawned
  // process to exit.  So instead will try to forcibly "kill" the process.

  DosKillProcess(DKP_PROCESS,WaitPacket->Pid);

  // If that doesn't kill the process, nothing will, so we may as well
  // terminate this thread right now by simply returning.
}


static char * LIBENTRY name(char *Path) {
  char *p;
  if ((p = strrchr(Path,'\\')) != NULL || (p = strrchr(Path,':')) != NULL)
    return(p+1);  /* point to first character of filename */
  return(Path);
}


static int LIBENTRY addstring(char *Dest, char *Srce) {
  int Len = strlen(Srce) + 1;
  memcpy(Dest,Srce,Len);
  return(Len);
}


static HWND LIBENTRY gethwnd(void) {
  PTIB     ptib;
  PPIB     ppib;
  HWND     RetVal;
  HAB      hab        = NULLHANDLE;
  ULONG    cbItems;
  ULONG    ulBufSize;
  PSWBLOCK pswblk;
  PSWENTRY p;
  int      X;

  cbItems = WinQuerySwitchList(hab, NULL, 0);
  ulBufSize = (cbItems * sizeof(SWENTRY)) + sizeof(HSWITCH);

  if ((pswblk = (PSWBLOCK) malloc(ulBufSize)) == NULL)
    return(-1);

  cbItems = WinQuerySwitchList(hab, pswblk, ulBufSize);

  DosGetInfoBlocks(&ptib,&ppib);

  for (X = 0, RetVal = -1, p = pswblk->aswentry; X < cbItems; X++, p++) {
    if (p->swctl.idProcess == ppib->pib_ulpid || p->swctl.idProcess == ppib->pib_ulppid) {
      RetVal = p->swctl.hwnd;
      break;
    }
  }

  free(pswblk);
  return(RetVal);
}


void LIBENTRY setvisibility(bool Visible) {
  HWND hwnd;
  SWP  swp;

  hwnd = gethwnd();

  if (WinQueryWindowPos(hwnd,&swp)) {
    if (Visible)
      WinShowWindow(hwnd,TRUE);
    else
      WinShowWindow(hwnd,FALSE);
  }
}


int LIBENTRY isminimized(void) {
  HAB      hab        = NULLHANDLE;
  ULONG    cbItems;
  ULONG    ulBufSize;
  PSWBLOCK pswblk;
  PSWENTRY p;
  PTIB     ptib;
  PPIB     ppib;
  SWP      swp;
  int      X;
  int      Minimized;

  DosGetInfoBlocks(&ptib,&ppib);
  if (ppib->pib_ultype == PIB_FULLSCREEN)
    return(0);

  cbItems = WinQuerySwitchList(hab, NULL, 0);
  ulBufSize = (cbItems * sizeof(SWENTRY)) + sizeof(HSWITCH);

  if ((pswblk = (PSWBLOCK) malloc(ulBufSize)) == NULL)
    return(0);

  cbItems = WinQuerySwitchList(hab, pswblk, ulBufSize);

  for (X = 0, p = pswblk->aswentry; X < cbItems; X++, p++) {
    if (p->swctl.idProcess == ppib->pib_ulpid || p->swctl.idProcess == ppib->pib_ulppid) {
      if (WinQueryWindowPos(p->swctl.hwnd,&swp)) {
        Minimized = (swp.fl & SWP_MINIMIZE);
        break;
      } else {
        Minimized = 0;
        break;
      }
    }
  }

  free(pswblk);
  return(Minimized);
}


static char * LIBENTRY dossettings(char *FileName) {
  bool     FileOpen;
  int      Size;
  int      Len;
  char    *p;
  char    *Env;
  DOSFILE  In;
  char     Str[128];

  Size = 100;

  FileOpen = (bool) (FileName != NULL && FileName[0] != 0 && dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&In) != -1);

  if (FileOpen) {
    Size += dosfseek(&In,0,SEEK_END);
    dosrewind(&In);
  }

  if ((Env = (char *) malloc(Size)) != NULL) {
    p = Env;

    if (FileOpen) {
      for (; dosfgets(Str,sizeof(Str),&In) != -1; p += Len)
        Len = addstring(p,Str);
    }

    // force these settings in case the user doesn't set them
    Len = addstring(p,"SIO_Mode_DTR=No Change at OPEN or CLOSE"); p += Len;
    Len = addstring(p,"COM_HOLD=ON"); p += Len;

    // put in the second of the double-null-terminator bytes
    *p = 0;
  }

  if (FileOpen)
    dosfclose(&In);

  return(Env);
}


int LIBENTRY startsession(char *ProgName,
                          char *Params,
                          char *Title,
                          char *WorkDir,
                          int   Wait,           // 0=no wait, -1 wait forever, ##=1/1000 seconds to wait
                          char  StartType,
                          int  *SidReturn,
                          char *SettingsFile,
                          int   PriorityDelta,
                          bool  MinimizeSelf,
                          bool  MinimizeWindow) {

  STARTDATA   StartData;
  ULONG       SessionId;
  PID         pid;
  PID         pidOwner;
  APIRET      rc;
  HQUEUE      hQueue;
  char        QueueName[66];
  REQUESTDATA RequestData;
  STATUSDATA  StatusData;
  ULONG       ProgType;
  ULONG       DataLen;
  PVOID       pData;
  PTIB        ptib;
  PPIB        ppib;
  BYTE        Priority;
  SWP         swp;
  char        SemName[66];
  HEV         Semaphore;
  int         OldDrive;
  char        OldPath[128];
  char       *Env;
  char      **p;
  int         Len;
  bool        ChangedDir;
  bool        DosPgm;
  int         ParentMinimized;

  DosGetInfoBlocks(&ptib,&ppib);

  memset(&StartData,0,sizeof(StartData));
  StartData.Length      = 32;
  StartData.Related     = SSF_RELATED_CHILD;  // or SSF_RELATED_INDEPENDENT
  StartData.FgBg        = SSF_FGBG_FORE;
  StartData.TraceOpt    = SSF_TRACEOPT_NONE;
  StartData.PgmTitle    = Title;
  StartData.PgmName     = ProgName;
  StartData.PgmInputs   = (PBYTE) Params;
  StartData.TermQ       = NULL;
  StartData.InheritOpt  = SSF_INHERTOPT_PARENT;  // or SSF_INHERTOPT_SHELL
  DosPgm                = FALSE;

  if (DosQueryAppType(ProgName,&ProgType) != 0)
    StartData.SessionType = SSF_TYPE_DEFAULT;
  else {
    switch (ProgType) {
      case FAPPTYP_NOTWINDOWCOMPAT:
           StartData.SessionType = SSF_TYPE_FULLSCREEN;
           break;
      case FAPPTYP_DOS:
           DosPgm = TRUE;
           switch (StartType) {
             case START_FULLSCREEN: StartData.SessionType = SSF_TYPE_VDM; break;
             case START_DEFAULT:    switch (ppib->pib_ultype) {
                                      case PIB_FULLSCREEN: StartData.SessionType = SSF_TYPE_VDM; break;
                                      case PIB_WINDOWED:   StartData.SessionType = SSF_TYPE_WINDOWEDVDM; break;
                                      default:             StartData.SessionType = SSF_TYPE_DEFAULT; break;
                                    }
                                    break;
             case START_MINIMIZED:
             case START_WINDOWED:
             default:               StartData.SessionType = SSF_TYPE_WINDOWEDVDM; break;
           }
           break;
      case FAPPTYP_WINDOWAPI:
           StartData.SessionType = SSF_TYPE_PM;
           break;
      default:
           switch (StartType) {
             case START_FULLSCREEN: StartData.SessionType = SSF_TYPE_FULLSCREEN; break;
             case START_DEFAULT:    switch (ppib->pib_ultype) {
                                      case PIB_FULLSCREEN: StartData.SessionType = SSF_TYPE_FULLSCREEN; break;
                                      case PIB_WINDOWED:   StartData.SessionType = SSF_TYPE_WINDOWABLEVIO; break;
                                      default:             StartData.SessionType = SSF_TYPE_DEFAULT; break;
                                    }
                                    break;
             case START_MINIMIZED:
             case START_WINDOWED:
             default:               StartData.SessionType = SSF_TYPE_WINDOWABLEVIO; break;
           }
           break;
    }
  }

  if (DosPgm)
    Env = dossettings(SettingsFile);
  else {
    // scan the environment to find out how big it is, include 1 null byte for
    // each string plus 1 extra null byte for the very end of the list (to do
    // this, start with Len=1).
    for (p = environ, Len = 1; *p != 0; p++)
      Len += strlen(*p) + 1;

    // Allocate the memory we need for the environment variables
    if ((Env = (char *) malloc(Len)) != NULL) {
      int  Ofs;
      char **p;
      for (p = environ, Ofs = 0; *p != 0; p++) {
        Len = strlen(*p) + 1;
        memcpy(Env+Ofs,*p,Len);
        Ofs += Len;
      }
      Env[Ofs] = 0;  // put second NULL terminator to indicate end of environment
    }
  }

  StartData.Environment = (PBYTE) Env;

  if (Wait != 0) {
    sprintf(QueueName,"\\QUEUES\\SHELL\\%s\\%d",name(ProgName),getpid());
    // ignore error when creating, instead look for error when opening
    // reason:  it may have already been created (???)
    DosCreateQueue(&hQueue,QUE_FIFO,QueueName);
    pidOwner = 0;
    rc = DosOpenQueue(&pidOwner,&hQueue,QueueName);
    if (rc != 0) {
      Wait = 0;
    } else {
      sprintf(SemName,"\\SEM32\\SHELL\\%s\\%d",name(ProgName),getpid());
      if (DosCreateEventSem(SemName,(PHEV) &Semaphore,0,0) == 0)
        StartData.TermQ = (PBYTE) QueueName;
      else {
        // it might already be created, so don't take this as an error yet,
        // instead, try to just open it and if that fails, then abort
        if (DosCreateEventSem(SemName,(PHEV) &Semaphore,0,0) == 0)
          StartData.TermQ = (PBYTE) QueueName;
        else {
          DosCloseQueue(hQueue);
          Wait = 0;
        }
      }
    }
  }

  ChangedDir = FALSE;
  if (WorkDir != NULL && WorkDir[0] != 0) {
    OldDrive = dosgetcurdrive();
    dosgetcurpath(OldDrive,OldPath,sizeof(OldPath));
    strupr(WorkDir);
    if (WorkDir[1] == ':') {
      dossetcurdrive(WorkDir[0]-'A'+1);
      WorkDir += 2;
    }
    chdir(WorkDir);
    ChangedDir = TRUE;
  }

  // record the current window state for use below
  ParentMinimized = isminimized();

  // if the window is requested to be minimized, or if the parent is already
  // minimized, then force the new session to be minimized
  if (MinimizeWindow || ParentMinimized) {
    StartData.Length     = 50;
    StartData.PgmControl = SSF_CONTROL_MINIMIZE;
    StartData.FgBg       = SSF_FGBG_BACK;
  }

  // if we need to minimize ourself and we aren't already minimized
  // then minimize our window first before creating the new session
  if (MinimizeSelf && ! ParentMinimized) {
    // if we're not minimizing the shelled window, then let's put the
    // shelled window in exactly the same location where the parent was
    // before we minimized the parent
    if ((! MinimizeWindow) && WinQueryWindowPos(gethwnd(),&swp)) {
      StartData.Length      = 50;
      StartData.PgmControl |= SSF_CONTROL_SETPOS;
      StartData.InitXPos    = (short) swp.x;
      StartData.InitYPos    = (short) swp.y;
      StartData.InitXSize   = (short) swp.cx;
      StartData.InitYSize   = (short) swp.cy;
    }
    setvisibility(FALSE);
  }

  if (PriorityDelta != 0) {
    // adjust the current thread to the requested priority
    // we do this before starting the child process so that it will inherit
    // the new priority, then we'll "undo" the change down below
    DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,PriorityDelta,0);
  }

  rc = DosStartSession(&StartData,&SessionId,&pid);

  if (PriorityDelta != 0) {
    // undo the priority change by making negating it
    PriorityDelta = -PriorityDelta;
    DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,PriorityDelta,0);
  }

  if (ChangedDir) {
    dossetcurdrive(OldDrive-'A'+1);
    chdir(OldPath);
  }

  if (Wait != 0) {
    waittype WaitPacket;
    int      WaitThreadId;

    if (rc != 2 && rc != 3) {
      StatusData.Length    = sizeof(StatusData);
      StatusData.SelectInd = SET_SESSION_UNCHANGED;
      StatusData.BondInd   = SET_SESSION_BOND;
      DosSetSession(SessionId,&StatusData);

      // if Wait == -1 then wait forever
      //   otherwise, wait for the number of milliseconds specified
      if (Wait != -1) {
        // set up a packet to pass to the waitthread
        WaitPacket.Time = Wait;
        WaitPacket.Pid  = pid;
        // start the waitthread which will wait for the number of milliseconds
        // specified, at which point it will signal the spawned session with
        // a CTRL-BREAK and cause it to end
        WaitThreadId = startthread(waitthread,8192,&WaitPacket);
      }

      DosReadQueue(hQueue,&RequestData,&DataLen,&pData,0,DCWW_WAIT,&Priority,Semaphore);

      // if Wait != -1 then we've started waitthread and need to shut it down
      if (Wait != -1)
        killthread(WaitThreadId,TRUE);
    }
    DosCloseQueue(hQueue);
    DosCloseEventSem(Semaphore);

    // if we minimized ourself, then we need to make ourself visible again
    if (MinimizeSelf && ! ParentMinimized)
      setvisibility(TRUE);
  }

  if (Env != NULL)
    free(Env);

  if (SidReturn != NULL)
    *SidReturn = SessionId;

  return(rc);
}



#ifdef TEST
#include <stdio.h>
#include <screen.h>

void main(void) {
  printf("rc = %d\n",startsession("PE.EXE",NULL,"Testing",NULL,TRUE,START_WINDOWED,NULL));
}
#endif
