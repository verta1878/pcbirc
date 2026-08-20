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

#define  INCL_GPI
#define  INCL_WINSHELLDATA
#define  INCL_WINWINDOWMGR
#define  INCL_DOSPROCESS
#include <os2.h>

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <dosfunc.h>
#include <misc.h>
#include <semafore.hpp>
#include "main.h"
#include "xtrn.h"

/*
 *  Global variables
 */

int              UserNetFile;
usernethdrtype   UserNetHeader;
int              MaxNodes;
nodetype        *NodeArray;
nodetype        *SaveArray;
int              SkipScreenUpdate;
settingstype     Settings;
nodeinfotype    *NodeInfo;
int             *SessionIds;
int             *Semaphores;
bool            *StopNodes;
char             ComSpec[128];
CMutexSemaphore  LaunchSemaphore;
RECTL            rclDesktop;

char  Scrn_24Hour = 0;             /* True: 24 hour clock - can be changed */
char  Scrn_DateSeparator = '-';    /* i.e. 12-12-88  - can be changed      */

/*
 *  Entry point declarations
 */

/**************************************************************************
 *
 *  Name       : Init()
 *
 *  Description:  Performs initialization functions required
 *                before the main window can be created.
 *
 *  Concepts:     Called once before the main window is created.
 *
 *                - Installs the routine ExitProc into the
 *                  DosExitList chain
 *                - registers all window classes
 *                - performs any command line processing
 *
 *  API's      :  DosExitList
 *                Exit
 *                WinLoadString
 *                WinRegisterClass
 *
 *  Parameters :  [none]
 *
 *  Return     :  TRUE = initialization is successful
 *                FALSE = initialization failed
 *
 *************************************************************************/

static char  *ProgName = "PCBCP";

static FATTRS fAttrs;
static bool   LoadedFont;
static SWP    swp;
static bool   LoadedPos;


char * LIBENTRY loadstring(int ID, char *Buf, int MaxLen) {
  if (WinLoadString(hab, (HMODULE)0, ID, MaxLen, (PSZ)Buf) == 0) {
    MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, 0, MB_OK|MB_ERROR, TRUE);
    return(NULL);
  }
  return(Buf);
}

void LIBENTRY setprofilefont(void) {
  HPS hps;

  if (! LoadedFont)
    return;

  hps = hpsMain;

  // Discard LCID (logical character identifier) 1 if it exists
  if (GpiSetCharSet(hps,1)) {
    GpiSetCharSet(hps,0);        // select font 0
    GpiDeleteSetId(hps,1);       // delete existing font 1
  }

  if (GpiCreateLogFont(hps,NULL,1,&fAttrs) != GPI_ERROR)
    GpiSetCharSet(hps,1);
}


bool LIBENTRY isprofilesizeloaded(void) {
  return(LoadedPos);
}


void LIBENTRY setprofilesize(void) {
  if (! LoadedPos)
    return;

  WinSetWindowPos(hwndMainFrame,
                  HWND_TOP,
                  swp.x,
                  swp.y,
                  swp.cx,
                  swp.cy,
                  SWP_SIZE|SWP_MOVE|SWP_SHOW);
}


static bool LIBENTRY readvariable(HINI hini, char *Name, void *Variable, int Size) {
  ULONG  cb;

  PrfQueryProfileSize(hini,ProgName,Name,&cb);
  if (cb == Size) {
    PrfQueryProfileData(hini,ProgName,Name,Variable,&cb);
    return(TRUE);
  }
  return(FALSE);
}


static void LIBENTRY getininame(char *IniName) {
  char *p;
  int  X;

  for (X = 1; X < _argc; X++) {
    if (memicmp(_argv[X],"/INI=",5) == 0) {
      strcpy(IniName,_argv[X]+5);
      return;
    }
  }

  strcpy(IniName,_argv[0]);
  if ((p = strrchr(IniName,'\\')) != NULL || (p = strrchr(IniName,':')) != NULL)
    strcpy(p+1,"PCBCP.INI");
  else
    strcpy(IniName,"PCBCP.INI");
}


static void LIBENTRY readprofile(void) {
  HINI   hini;
  char   IniName[128];

  // set default values
  Settings.FixedFont          = TRUE;
  Settings.ShowOld            = FALSE;
  Settings.WarnOld            = FALSE;
  Settings.MonitorAll         = FALSE;
  Settings.PageAlarmEnabled   = FALSE;
  Settings.PageBringToFore    = FALSE;
  Settings.RestartInactive    = FALSE;
  Settings.VerifyChanges      = TRUE;
  Settings.ClickToRun         = FALSE;
  Settings.DisableStartRun    = FALSE;
  Settings.DisableConnectRun  = FALSE;
  Settings.WarnSeconds        = 60 * 60;
  Settings.WarnEventSeconds   = 60 * 60;
  Settings.WarnDoorSeconds    = 60 * 60;
  Settings.WarnXferSeconds    = 60 * 60;
  Settings.WarnRestartSeconds =  3 * 60;
  Settings.NumLines           = 10;
  Settings.UpdateInterval     = 3;
  Settings.FirstShow          = 1;
  Settings.FirstNode          = 1;
  Settings.LastNode           = 0;  // openusernet() will adjust this if necessary

  // At the time that readprofile() is called, the window size and font
  // cannot be updated.  So we'll use these variables to record the status
  // of whether or not these items were read into memory and then later in
  // the initialization process we'll set the window size and font.
  LoadedFont = FALSE;
  LoadedPos  = FALSE;

  getininame(IniName);
  if ((hini = PrfOpenProfile(hab,IniName)) != 0) {
    readvariable(hini,"FixedFont",  &Settings.FixedFont,         sizeof(Settings.FixedFont));
    readvariable(hini,"NumLines",   &Settings.NumLines,          sizeof(Settings.NumLines));
    readvariable(hini,"UpdateInt",  &Settings.UpdateInterval,    sizeof(Settings.UpdateInterval));
    readvariable(hini,"FirstShow",  &Settings.FirstShow,         sizeof(Settings.FirstShow));
    readvariable(hini,"ShowOld",    &Settings.ShowOld,           sizeof(Settings.ShowOld));
    readvariable(hini,"WarnOld",    &Settings.WarnOld,           sizeof(Settings.WarnOld));
    readvariable(hini,"MonitorAll", &Settings.MonitorAll,        sizeof(Settings.MonitorAll));
    readvariable(hini,"PageAlarm",  &Settings.PageAlarmEnabled,  sizeof(Settings.PageAlarmEnabled));
    readvariable(hini,"PageFore",   &Settings.PageBringToFore,   sizeof(Settings.PageBringToFore));
    readvariable(hini,"RestartOld", &Settings.RestartInactive,   sizeof(Settings.RestartInactive));
    readvariable(hini,"OldSeconds", &Settings.WarnSeconds,       sizeof(Settings.WarnSeconds));
    readvariable(hini,"FirstNode",  &Settings.FirstNode,         sizeof(Settings.FirstNode));
    readvariable(hini,"LastNode",   &Settings.LastNode,          sizeof(Settings.LastNode));
    readvariable(hini,"ClickToRun", &Settings.ClickToRun,        sizeof(Settings.ClickToRun));
    readvariable(hini,"DisableRunS",&Settings.DisableStartRun,   sizeof(Settings.DisableStartRun));
    readvariable(hini,"DisableRunC",&Settings.DisableConnectRun, sizeof(Settings.DisableConnectRun));

    if (! readvariable(hini,"WarnEvents", &Settings.WarnEventSeconds, sizeof(Settings.WarnEventSeconds)))
      Settings.WarnEventSeconds = Settings.WarnSeconds;

    if (! readvariable(hini,"WarnDoors",  &Settings.WarnDoorSeconds,  sizeof(Settings.WarnDoorSeconds)))
      Settings.WarnDoorSeconds = Settings.WarnSeconds;

    if (! readvariable(hini,"WarnXfers",  &Settings.WarnXferSeconds,  sizeof(Settings.WarnXferSeconds)))
      Settings.WarnDoorSeconds = Settings.WarnSeconds;

    if (! readvariable(hini,"RestartMin", &Settings.WarnRestartSeconds,sizeof(Settings.WarnRestartSeconds)))
      Settings.WarnRestartSeconds = 3 * 60;

    if (! readvariable(hini,"VerifyChg", &Settings.VerifyChanges,sizeof(Settings.VerifyChanges)))
      Settings.VerifyChanges = TRUE;

    LoadedFont = readvariable(hini,"Font",&fAttrs,sizeof(FATTRS));
    LoadedPos  = readvariable(hini,"WindowPos",&swp,sizeof(SWP));

    PrfCloseProfile(hini);
  }
}


void LIBENTRY readnodeprofile(void) {
  HINI hini;
  int  X;
  char Str[20];
  char IniName[128];

  if (NodeInfo != NULL) {
    free(NodeInfo);
    NodeInfo = NULL;
  }

  if ((NodeInfo = (nodeinfotype *) malloc(MaxNodes * sizeof(nodeinfotype))) == NULL)
    return;

  memset(NodeInfo,0,MaxNodes * sizeof(nodeinfotype));

  getininame(IniName);
  if ((hini = PrfOpenProfile(hab,IniName)) != 0) {
    for (X = Settings.FirstNode; X <= Settings.LastNode; X++) {
      sprintf(Str,"Node_%d",X);
      readvariable(hini,Str,&NodeInfo[X-1],sizeof(nodeinfotype));
    }
    PrfCloseProfile(hini);
  }
}


void LIBENTRY writeprofile(void) {
  HINI   hini;
  BOOL   fSuccess;
  STR8   Name;
  FATTRS fAttrs;
  SWP    swp;
  char   IniName[128];

  getininame(IniName);
  if ((hini = PrfOpenProfile(hab,IniName)) != 0) {
    PrfWriteProfileData(hini,ProgName,"FixedFont",&Settings.FixedFont,sizeof(Settings.FixedFont));
    PrfWriteProfileData(hini,ProgName,"NumLines",&Settings.NumLines,sizeof(Settings.NumLines));
    PrfWriteProfileData(hini,ProgName,"UpdateInt",&Settings.UpdateInterval,sizeof(Settings.UpdateInterval));
    PrfWriteProfileData(hini,ProgName,"FirstShow",&Settings.FirstShow,sizeof(Settings.FirstShow));
    PrfWriteProfileData(hini,ProgName,"ShowOld",&Settings.ShowOld,sizeof(Settings.ShowOld));
    PrfWriteProfileData(hini,ProgName,"WarnOld",&Settings.WarnOld,sizeof(Settings.WarnOld));
    PrfWriteProfileData(hini,ProgName,"MonitorAll",&Settings.MonitorAll,sizeof(Settings.MonitorAll));
    PrfWriteProfileData(hini,ProgName,"PageAlarm",&Settings.PageAlarmEnabled,sizeof(Settings.PageAlarmEnabled));
    PrfWriteProfileData(hini,ProgName,"PageFore",&Settings.PageBringToFore,sizeof(Settings.PageBringToFore));
    PrfWriteProfileData(hini,ProgName,"RestartOld",&Settings.RestartInactive,sizeof(Settings.RestartInactive));
    PrfWriteProfileData(hini,ProgName,"OldSeconds",&Settings.WarnSeconds,sizeof(Settings.WarnSeconds));
    PrfWriteProfileData(hini,ProgName,"FirstNode",&Settings.FirstNode,sizeof(Settings.FirstNode));
    PrfWriteProfileData(hini,ProgName,"LastNode",&Settings.LastNode,sizeof(Settings.LastNode));
    PrfWriteProfileData(hini,ProgName,"ClickToRun",&Settings.ClickToRun,sizeof(Settings.ClickToRun));
    PrfWriteProfileData(hini,ProgName,"DisableRunS",&Settings.DisableStartRun,sizeof(Settings.DisableStartRun));
    PrfWriteProfileData(hini,ProgName,"DisableRunC",&Settings.DisableConnectRun,sizeof(Settings.DisableConnectRun));
    PrfWriteProfileData(hini,ProgName,"WarnEvents",&Settings.WarnEventSeconds,sizeof(Settings.WarnEventSeconds));
    PrfWriteProfileData(hini,ProgName,"WarnDoors",&Settings.WarnDoorSeconds,sizeof(Settings.WarnDoorSeconds));
    PrfWriteProfileData(hini,ProgName,"WarnXfers",&Settings.WarnXferSeconds,sizeof(Settings.WarnXferSeconds));
    PrfWriteProfileData(hini,ProgName,"RestartMin",&Settings.WarnRestartSeconds,sizeof(Settings.WarnRestartSeconds));
    PrfWriteProfileData(hini,ProgName,"VerifyChg",&Settings.VerifyChanges,sizeof(Settings.VerifyChanges));

    fSuccess = GpiQueryLogicalFont(hpsMain,1,&Name,&fAttrs,sizeof(FATTRS));
    if (fSuccess == TRUE)
      PrfWriteProfileData(hini,ProgName,"Font",&fAttrs,sizeof(FATTRS));

    fSuccess = WinQueryWindowPos(hwndMainFrame,&swp);
    if (fSuccess == TRUE)
      PrfWriteProfileData(hini,ProgName,"WindowPos",&swp,sizeof(SWP));

    PrfCloseProfile(hini);
  }
}


void LIBENTRY writenodeprofile(void) {
  HINI hini;
  int  X;
  char Str[20];
  char IniName[128];

  if (NodeInfo == NULL)
    return;

  getininame(IniName);
  if ((hini = PrfOpenProfile(hab,IniName)) != 0) {
    for (X = Settings.FirstNode; X <= Settings.LastNode; X++) {
      sprintf(Str,"Node_%d",X);
      PrfWriteProfileData(hini,ProgName,Str,&NodeInfo[X-1],sizeof(nodeinfotype));
    }
    PrfCloseProfile(hini);
  }
}


BOOL LIBENTRY Init(VOID) {
  char *p;
  int   X;

  /* Add ExitProc to the exit list to handle the exit processing.  If
   * there is an error, then terminate the process since there have
   * not been any resources allocated yet.
   */
  if (DosExitList(EXLST_ADD, (PFNEXITLIST)ExitProc)) {
    MessageBox(HWND_DESKTOP,
               IDMSG_CANNOTLOADEXITLIST,
               0,
               MB_OK | MB_ERROR,
               TRUE);
    exit(EXIT_PROCESS);
  }

  /* load application name from resource file */
  if (loadstring(IDS_APPNAME,szAppName,MAXNAMEL) == NULL)
    return FALSE;

  if (loadstring(IDS_LAUNCHING,szLaunch,MAXNAMEL) == NULL)
    return(FALSE);

  /* register the main client window class */
  if (! WinRegisterClass(hab,
                        (PSZ)szAppName,
                        (PFNWP)MainWndProc,
                        CS_SIZEREDRAW | CS_CLIPCHILDREN,
                        0L))
     return FALSE;

  WinQueryWindowRect(HWND_DESKTOP,&rclDesktop);

  readprofile();
  Scrn_24Hour = TRUE;

  if ((p = getenv("COMSPEC")) != NULL)
    strcpy(ComSpec,p);
  else if ((p = getenv("OS2_SHELL")) != NULL)
    strcpy(ComSpec,p);
  else
    strcpy(ComSpec,"CMD.EXE");

  for (X = 1; X < _argc; X++) {
    if (_argv[X] != NULL && _argv[X][0] != 0 && _argv[X][0] != '/') {
      openusernetfile(_argv[X]);
      break;
    }
  }

//DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,+1,0);
  LaunchSemaphore.create(NULL);

  return TRUE;
} /* End of Init   */


/**************************************************************************
 *
 *  Name       : InitMainWindow(hwnd, mp1, mp2)
 *
 *  Description: Performs initialization functions required
 *               when the main window is created.
 *
 *  Concepts:    Called once during the WM_CREATE processing when
 *               the main window is created.
 *
 *  API's      : [none]
 *
 *  Parameters :  hwnd = window handle
 *                mp1  = first message parameter
 *                mp2  = second message parameter
 *
 *  Return     :   value to be returned from the WM_CREATE message:
 *                TRUE =  window creation should stop
 *                FALSE = window creation should continue
 *
 *************************************************************************/
#pragma warn -par
MRESULT LIBENTRY InitMainWindow(HWND hwnd, MPARAM mp1, MPARAM mp2) {
  UpdateTitleText(((PCREATESTRUCT)PVOIDFROMMP(mp2))->hwndParent,NULL,0);

  /* return FALSE to continue window creation, TRUE to abort it */
  return (MRESULT)FALSE;


  /* This routine currently doesn't use the hwnd, mp1, and mp2 parameters
   *  so they are referenced here to prevent an 'Unreferenced Parameter'
   *  warning at compile time.
   */

}  /* End of InitMainWindow   */
#pragma warn .par


/**************************************************************************
 *
 *  Name       : ExitProc(usTermCode)
 *
 *  Description: Cleans up certain resources when the application
 *               terminates.
 *
 *  Concepts:    Routine is called by DosExitList when the
 *               application exits.
 *
 *               Global resources, such as the main window and
 *               message queue, are destroyed and any system
 *               resources used are freed.
 *
 *  API's      : WinIsWindow
 *               WinDestroyWindow
 *               WinDestroyMsgQueue
 *               WinTerminate
 *               DosExitList
 *
 *  Parameters :  usTermCode = termination code number
 *
 *  Return    :   Returns EXLST_EXIT to the DosExitList handler
 *
 *************************************************************************/
static VOID LIBENTRY ExitProc(USHORT usTermCode) {
  /* destroy the main window if it exists */
  if (WinIsWindow(hab,hwndMainFrame))
     WinDestroyWindow(hwndMainFrame);

  /*
   *      Any other system resources used
   *      (e.g. memory or files) should be freed here.
   */

  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);

  if (UserNetFile > 0) {
    dosclose(UserNetFile);
    UserNetFile = 0;
  }

  if (NodeArray != NULL) {
    free(NodeArray);
    NodeArray = NULL;
  }

  if (SaveArray != NULL) {
    free(SaveArray);
    SaveArray = NULL;
  }

  DosExitList(EXLST_EXIT, (PFNEXITLIST)NULL);    /* termination complete */

  /* This routine currently doesn't use the usTermCode parameter so
   *  it is referenced here to prevent an 'Unreferenced Parameter'
   *  warning at compile time
   */

#pragma warn -par
}   /* End of ExitProc    */
#pragma warn +par
/***************************  End of init.c  ***************************/
