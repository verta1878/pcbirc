#include "pcbcp_compat.h"
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

#define  INCL_DOSSESMGR
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#define  INCL_PM
#include <os2.h>

#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <dosfunc.h>
#include <system.h>
#include <session.hpp>
#include <semafore.hpp>
#include <misc.h>
#include "main.h"
#include "xtrn.h"
#include "help.h"

extern CMutexSemaphore LaunchSemaphore;

#define THREADSTACKSIZE    16384L
#define SEM_TIMEOUT        20000L

enum {STARTNODE = 1, LASTNODE = 0};

#define LENGTHOFESSENTIALS  ((int)((char *) p->Message - (char *) p))

/*
 *  Global variables
 */

int SelectedNode;
int SelectedNodeFirst;
int SelectedNodeLast;

static HAB habBkThread;
static HMQ hmqBkThread;
static TID tidBkThread;
static HWND hwndObject;
static CHAR szObjectClass[MAXNAMEL];
static BOOL fThreadCreated = FALSE;
static HEV hevThreadInit;   /* semaphore for thread initialization */

static int  FirstNodeOnScreen;
static int  NumNodesRunning;
static bool BroughtToFore = FALSE;

VOID BackgroundThread (ULONG);
void LIBENTRY setnetstatusflag(int NodeNumMinusOne, bool On);

static void LIBENTRY writeusernet(int NodeNumMinusOne, nodetype *Buf);
static void LIBENTRY updatescreen(bool RefreshOnly);
static void LIBENTRY showselectednodes(void);
static void LIBENTRY selectnode(int Y,bool StartNode);
static void LIBENTRY pageup(void);
static void LIBENTRY pagedown(void);
static void LIBENTRY lineup(void);
static void LIBENTRY linedown(void);
static void LIBENTRY launchonenode(int NodeNum, bool AutoAnswer);
static int  LIBENTRY getnodenum(int Y);
static MRESULT EXPENTRY ObjectWndProc(HWND, ULONG, MPARAM, MPARAM);

/**************************************************************************
 *
 *  Name       : BackgroundThread(ulThreadParam)
 *
 *  Description: Routine is a background thread used for tasks to be
 *               completed in the background.
 *
 *  Concepts:    Routine is called at initialization time to create
 *               a background processing thread.
 *
 *               The routine initializes itself as a PM thread and creates
 *               a message queue.  It then creates an object window
 *               through which it will receive and send messages.
 *               It then polls through a message loop, processing any
 *               messages it receives in the object window's window
 *               procedure.  When the loop ends, it terminates.
 *
 *  API's      :  WinInitialize
 *                WinPostMsg
 *                Exit
 *                WinCreateMsgQueue
 *                WinRegisterClass
 *                WinCreateWindow
 *                DosPostEventSem
 *                WinGetMsg
 *                WinDispatchMsg
 *                WinIsWindow
 *                WinDestroyWindow
 *                WinDestroyMsgQueue
 *
 *  Parameters :  ulThreadParam = thread parameter
 *
 *  Return     :  [none]
 *
 *************************************************************************/
static VOID BackgroundThread(ULONG ulThreadParam) {
  QMSG qmsg;

  FirstNodeOnScreen = Settings.FirstShow - 1;

  /* create message queue for thread */
  habBkThread = WinInitialize(0);
  if (habBkThread == NULLHANDLE) {
    WinPostMsg(hwndMain, TM_THREADINITFAILED, (MPARAM)NULL, (MPARAM)NULL);
    exit(EXIT_THREAD);
  }

  hmqBkThread = WinCreateMsgQueue(habBkThread,100);
  if (hmqBkThread == NULLHANDLE) {
    WinPostMsg(hwndMain, TM_THREADINITFAILED, (MPARAM)NULL, (MPARAM)NULL);
    WinTerminate(habBkThread);
    exit(EXIT_THREAD);
  }

  // disable WM_QUIT messages for this thread
  WinCancelShutdown(hmqBkThread,TRUE);

  /* load the string for the object window class and register the class */
  loadstring(IDS_OBJECTCLASS,szObjectClass,MAXNAMEL);

  if (WinRegisterClass(habBkThread,
                       (PSZ)szObjectClass,
                       (PFNWP)ObjectWndProc,
                       0L,
                       0L)
      &&
      /* create the object window */
      (NULLHANDLE != (hwndObject = WinCreateWindow(HWND_OBJECT,
                                    (PSZ)szObjectClass,
                                    (PSZ)NULL,
                                    0L,
                                    0L,
                                    0L,
                                    0L,
                                    0L,
//                                  (HWND)NULLHANDLE,
                                    HWND_DESKTOP,
                                    HWND_TOP,
                                    OBJECTID,
                                    (PVOID)NULL,
                                    (PVOID)NULL)))) {
    /* set thread created flag so another thread of this type
     * cannot be created.
     */
    fThreadCreated = TRUE;

    /* clear initialization semaphore */
    DosPostEventSem(hevThreadInit);

    /* message loop */
    while(WinGetMsg(hmqBkThread, &qmsg, (HWND)NULLHANDLE, 0L, 0L))
       WinDispatchMsg(hmqBkThread, &qmsg);
  } else {
    WinPostMsg(hwndMain, TM_THREADINITFAILED, NULL, NULL);
  }

  /* destroy object window, clean up message queue and terminate */
  if (WinIsWindow(habBkThread, hwndObject))
    WinDestroyWindow(hwndObject);

  WinDestroyMsgQueue(hmqBkThread);
  WinTerminate(habBkThread);

  /* If termination is due to an error initializing the thread, then
      clear the initialization semaphore so that the main thread
      can continue. */
  if (! fThreadCreated)
    DosPostEventSem(hevThreadInit);

  exit(EXIT_THREAD);

  /* the thread parameter is not currently used */
#pragma warn -par
}   /* End of BackgroundThread */
#pragma warn +par


/**************************************************************************
 *
 *  Name       : CreateBackgroundThread()
 *
 *  Description: Creates the background thread
 *
 *  Concepts:    Routine is called at initialization time to create
 *               a background processing thread.
 *
 *               The routine calls DosCreateThread with the Background
 *               thread routine.
 *
 *  API's      :  DosCreateEventSem
 *                DosCreateThread
 *                WinWaitEventSem
 *
 *  Parameters :  [none]
 *
 *  Return     :  TRUE = thread is created successfully
 *                FALSE = not, or if the thread was already created
 *
 *************************************************************************/
BOOL LIBENTRY CreateBackgroundThread(VOID) {
  ULONG sRet;

  if (! fThreadCreated) {
    DosCreateEventSem((PSZ)NULL, &hevThreadInit, 0L, FALSE);
    sRet = (ULONG)DosCreateThread(&tidBkThread,
                                  (PFNTHREAD)BackgroundThread,
                                  0L,
                                  0L,
                                  THREADSTACKSIZE);
  } else
    return FALSE;

  /* wait until the thread has finished initialization */
  if (WinWaitEventSem(hevThreadInit,SEM_TIMEOUT))
    return FALSE;

  // start the nodes running now
  if (! (Settings.DisableStartRun && Settings.DisableConnectRun))
    PostBkThreadMsg(UM_STARTUP,0,0);


  return((BOOL)(sRet == 0L));
}  /* End of CreateBackgroundThread   */


/**************************************************************************
 *
 *  Name       : DestroyBackgroundThread()
 *
 *  Description: Destroys the background thread
 *
 *  Concepts:    Routine is called at exit time to destroy the
 *               background processing thread.
 *
 *               The routine posts a WM_CLOSE message to the object window
 *               to end its message loop.  It then waits to make sure that
 *               the thread has been terminated before it returns.
 *
 *  API's      :  DosWaitThread
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY DestroyBackgroundThread(VOID) {
  PostBkThreadMsg(WM_CLOSE, (MPARAM)NULL, (MPARAM)NULL);
  DosWaitThread(&tidBkThread, 0L);
} /* End of DestroyBackgroundThread */


/**************************************************************************
 *
 *  Name       : PostBkThreadMsg(msg, mp1, mp2)
 *
 *  Description: Routine posts a message to the object window of the
 *               background thread.
 *
 *  Concepts:    Routine is called whenever a message is to be posted to
 *               the background processing thread.
 *
 *               The routine posts the message to the object window of the
 *               thread.
 *
 *  API's      :  WinPostMsg
 *
 *  Parameters :  msg = the message i.d.
 *                mp1 = first message parameter
 *                mp2 = second message parameter
 *
 *  Return     :  the return value from WinPostMsg().
 *
 *************************************************************************/
BOOL LIBENTRY PostBkThreadMsg(ULONG msg, MPARAM mp1, MPARAM mp2) {
  return(WinPostMsg(hwndObject, msg, mp1, mp2));
} /* End of PostBkThreadMsg   */


/**************************************************************************
 *
 *  Name       : SendBkThreadMsg(msg, mp1, mp2)
 *
 *  Description: Routine sends a message to the object window of the
 *               background thread.
 *
 *  Concepts:    Routine is called whenever a message is to be sent to
 *               the background processing thread.
 *
 *               The routine sends the message to the object window of the
 *               thread.
 *
 *  API's      :  WinSendMsg
 *
 *  Parameters :  msg = the message i.d.
 *                mp1 = first message parameter
 *                mp2 = second message parameter
 *
 *  Return     :  the return value from WinSendMsg().
 *
 *************************************************************************/
// static MRESULT LIBENTRY SendBkThreadMsg(ULONG msg, MPARAM mp1, MPARAM mp2) {
//   return(WinSendMsg(hwndObject, msg, mp1, mp2));
// } /* End of SendBkThreadMsg */



/**************************************************************************
 *
 *  Name       : ObjectWndProc(hwnd, msg, mp1, mp2)
 *
 *  Description: Processes all messages sent to the Object window
 *
 *  Concepts:    Called for each message sent to the Object window.
 *
 *               The Object window processes the messages that tell
 *               the background thread what action to take.  Since
 *               the object window is not visible, it will not
 *               process any of the standard window messages.
 *
 *  API's      :  WinDefWindowProc
 *
 *  Parameters :  hwnd= window handle
 *                msg = the message i.d.
 *                mp1 = first message parameter
 *                mp2 = second message parameter
 *
 *  Return     :  dependent on message sent
 *
 *************************************************************************/

static void LIBENTRY confirmchange(HWND hwnd, char Status) {
  char Title[50];
  char Msg[80];
  char Text[128];

  if (Settings.VerifyChanges) {
    loadstring(IDS_STATUSCHANGETITLE,Title,sizeof(Title));
    loadstring(IDS_STATUSCHANGEMSG,Msg,sizeof(Msg));

    if (SelectedNodeFirst == SelectedNodeLast)
      sprintf(Text,"%s %d:\r",Msg,SelectedNodeFirst);
    else
      sprintf(Text,"%ss %d-%d:\r",Msg,SelectedNodeFirst,SelectedNodeLast);

    strcat(Text,getstatustext(Status));

    if (MBID_OK != WinMessageBox(HWND_DESKTOP,hwnd,Text,Title,0,MB_OKCANCEL|MB_ICONQUESTION|MB_MOVEABLE))
      return;
  }

  writeselectednodes(Status);
}

/*********************************************************************/

void LIBENTRY selectandconfirmchange(HWND hwnd, bool AllNodes, char Status) {
  deselectnodes();

  if (AllNodes) {
    SelectedNodeFirst = Settings.FirstNode;
    SelectedNodeLast  = Settings.LastNode;
  } else {
    SelectedNodeFirst = FirstNodeOnScreen + 1;
    SelectedNodeLast  = FirstNodeOnScreen + numWinLines;
    if (SelectedNodeLast > Settings.LastNode)
      SelectedNodeLast = Settings.LastNode;
  }

  confirmchange(hwnd,Status);
}

/*********************************************************************/

static HWND         hwndNoteBook;
static HWND         hwndNodeSettings;
static HWND         hwndRunSettings;
static HWND         hwndScale;
static ULONG       *aulPageIds;
static bool         ChangedNodeInfo = FALSE;
static bool         NeedToSave = FALSE;
static bool         InitNoteBook = FALSE;
static char         AutoStart;
static bool         WindowType;
static nodeinfotype SaveNodeInfo;
static int          CurNode;
static int          CopyFrom;
static int          CopyToFirst;
static int          CopyToLast;
static int          NoteBookFirstNode;


static MRESULT EXPENTRY CopyFromProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  switch(ulMsg) {
    case WM_INITDLG:
         CopyFrom = 0;
         centerwindow(hwnd);
         setspinbutton(hwnd,ID_COPYFROMNODE,MaxNodes,1,1);
         break;

   case WM_HELP:
        DisplayHelpPanel(HID_NOTEBOOK_COPYFROM_HELP_PANEL);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                CopyFrom = getspinbuttonvalue(hwnd,ID_COPYFROMNODE);
                break;
           case DID_CANCEL:
                CopyFrom = 0;
                break;
           default:
                return(0);
         }
         break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}

static bool LIBENTRY copyfromnode(void) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       CopyFromProc,
                       0,
                       IDD_COPYFROM,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);

  if (CopyFrom != 0 && (CopyFrom-1) != CurNode) {
    NodeInfo[CurNode] = NodeInfo[CopyFrom-1];
    AutoStart  = NodeInfo[CopyFrom-1].AutoStart;
    WindowType = NodeInfo[CopyFrom-1].WindowType;
    NeedToSave = TRUE;
    return(TRUE);
  }
  return(FALSE);
}

static MRESULT EXPENTRY CopyToProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  switch(ulMsg) {
    case WM_INITDLG:
         CopyToFirst = CopyToLast = 0;
         centerwindow(hwnd);
         setspinbutton(hwnd,ID_COPYTOFIRST,MaxNodes,1,1);
         setspinbutton(hwnd,ID_COPYTOLAST ,MaxNodes,1,1);
         break;

    case WM_CONTROL:
         relatespinbutton(hwnd,mp1,ID_COPYTOFIRST,ID_COPYTOLAST);
         break;

   case WM_HELP:
        DisplayHelpPanel(HID_NOTEBOOK_COPYTO_HELP_PANEL);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                CopyToFirst = getspinbuttonvalue(hwnd,ID_COPYTOFIRST);
                CopyToLast  = getspinbuttonvalue(hwnd,ID_COPYTOLAST);
                if (CopyToFirst > CopyToLast) {
                  MessageBox(HWND_DESKTOP,IDMSG_FIRSTNODELESS,IDMSG_COPYRANGEERROR,MB_ENTER|MB_ERROR|MB_MOVEABLE,FALSE);
                  CopyToFirst = CopyToLast = 0;
                  return(0);
                }
                break;
           case DID_CANCEL:
                CopyToFirst = CopyToLast = 0;
                break;
           default:
                return(0);
         }
         break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}

static void LIBENTRY copytonodes(void) {
  HWND  hwndDlg;
  int   X;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       CopyToProc,
                       0,
                       IDD_COPYTO,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);

  if (CopyToFirst != 0) {
    CopyToFirst--;
    CopyToLast--;
    for (X = CopyToFirst; X <= CopyToLast; X++) {
      NodeInfo[X] = SaveNodeInfo;
      NodeInfo[X].AutoStart  = AutoStart;
      NodeInfo[X].WindowType = WindowType;
    }
    NeedToSave = TRUE;
  }
}


static void LIBENTRY shownodeinfo(int NodeNum) {
  nodeinfotype *p;

  p          = &NodeInfo[NodeNum];
  CurNode    = NodeNum;
  AutoStart  = p->AutoStart;
  WindowType = p->WindowType;
  WinSetDlgItemText(hwndNodeSettings,ID_PATHNAME,  (PSZ)p->PathName);
  WinSetDlgItemText(hwndNodeSettings,ID_PARAMETERS,(PSZ)p->Parameters);
  WinSetDlgItemText(hwndNodeSettings,ID_WORKDIR,   (PSZ)p->WorkDir);
  setspinbuttonvalue(hwndNodeSettings,ID_PORT,p->PortNum);

  WinSendDlgItemMsg(hwndRunSettings,ID_WINDOWED+WindowType,BM_SETCHECK,
      MPFROMLONG(TRUE),MPVOID);
  WinSendDlgItemMsg(hwndRunSettings,ID_DISABLED+AutoStart,BM_SETCHECK,
      MPFROMLONG(TRUE),MPVOID);
}


static void LIBENTRY getnodeinfo(int NodeNum) {
  SaveNodeInfo = NodeInfo[NodeNum];
  shownodeinfo(NodeNum);
  ChangedNodeInfo = FALSE;
}


static void LIBENTRY putnodeinfo(int NodeNum) {
  NodeInfo[NodeNum] = SaveNodeInfo;
  NodeInfo[NodeNum].AutoStart  = AutoStart;
  NodeInfo[NodeNum].WindowType = WindowType;
  ChangedNodeInfo = FALSE;
}


static void LIBENTRY turntopage(int NodeNum, bool RunSettings) {
  WinSendMsg(hwndNoteBook,BKM_TURNTOPAGE,MPFROMLONG(aulPageIds[(NodeNum*2)+RunSettings]),NULL);
}


static MRESULT EXPENTRY ScaleProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
//HPS   hps;
//HWND  hwndBox;
//RECTL rcBox;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);
         WinShowWindow(hwnd,TRUE);
         return(0);
//  case UM_UPDATEBOX:
//       hwndBox = WinWindowFromID(hwnd,ID_PROGRESSBOX);
//       WinQueryWindowRect(hwndBox,&rcBox);
//       hps = WinBeginPaint(hwndBox,NULLHANDLE,&rcBox);
//       GpiSetDrawingMode(hps,DM_DRAW);
//       WinFillRect(hps,&rcBox,CLR_WHITE);
//       WinEndPaint(hps);
//       return(0);
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static MRESULT EXPENTRY RunDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  switch(ulMsg) {
    case WM_INITDLG:
         return(0);

    case WM_CONTROL:
         if (InitNoteBook)
           return(0);
         switch (SHORT2FROMMP(mp1)) {
           case BN_CLICKED:
                switch (SHORT1FROMMP(mp1)) {
                  case ID_DISABLED:
                       AutoStart = RUN_DISABLED;
                       WinSendDlgItemMsg(hwnd,ID_DISABLED+AutoStart,BM_SETCHECK,
                           MPFROMLONG(TRUE),MPVOID);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_AUTOSTART:
                       AutoStart = RUN_STARTUP;
                       WinSendDlgItemMsg(hwnd,ID_DISABLED+AutoStart,BM_SETCHECK,
                           MPFROMLONG(TRUE),MPVOID);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_RUNONCONNECT:
                       AutoStart = RUN_ONCONNECT;
                       WinSendDlgItemMsg(hwnd,ID_DISABLED+AutoStart,BM_SETCHECK,
                           MPFROMLONG(TRUE),MPVOID);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_WINDOWED:
                       WindowType = START_WINDOWED;
                       WinSendDlgItemMsg(hwnd,ID_WINDOWED+WindowType,BM_SETCHECK,
                           MPFROMLONG(TRUE),MPVOID);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_MINIMIZED:
                       WindowType = START_MINIMIZED;
                       WinSendDlgItemMsg(hwnd,ID_WINDOWED+WindowType,BM_SETCHECK,
                           MPFROMLONG(TRUE),MPVOID);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_FULLSCREEN:
                       WindowType = START_FULLSCREEN;
                       WinSendDlgItemMsg(hwnd,ID_WINDOWED+WindowType,BM_SETCHECK,
                           MPFROMLONG(TRUE),MPVOID);
                       ChangedNodeInfo = TRUE;
                       break;
                }
                break;
         }
         return(0);

    case WM_HELP:
         DisplayHelpPanel(HID_NOTEBOOK_RUN_DLG_HELP_PANEL);
         return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case ID_COPYFROM:
                if (copyfromnode())
                  getnodeinfo(CurNode);
                break;
           case ID_COPYTO:
                if (ChangedNodeInfo)
                  putnodeinfo(CurNode);
                copytonodes();
                break;
           case ID_UNDO:
                if (ChangedNodeInfo)
                  getnodeinfo(CurNode);
                break;
           case ID_NODESETTINGS:
                turntopage(CurNode+1,FALSE);
                break;
         }
         return(0);
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static MRESULT EXPENTRY NodeDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  switch(ulMsg) {
    case WM_INITDLG:
         WinSendDlgItemMsg(hwnd,ID_PATHNAME,  EM_SETTEXTLIMIT,(MPARAM)127,0);
         WinSendDlgItemMsg(hwnd,ID_PARAMETERS,EM_SETTEXTLIMIT,(MPARAM)127,0);
         WinSendDlgItemMsg(hwnd,ID_WORKDIR,   EM_SETTEXTLIMIT,(MPARAM)127,0);
         setspinbutton(hwnd,ID_PORT,100,0,0);
         return(0);

    case WM_CONTROL:
         if (InitNoteBook)
           return(0);
         switch (SHORT2FROMMP(mp1)) {
           case EN_CHANGE:
                switch(SHORT1FROMMP(mp1)) {
                  case ID_PATHNAME  :
                       WinQueryWindowText(WinWindowFromID(hwnd,ID_PATHNAME),sizeof(SaveNodeInfo.PathName),SaveNodeInfo.PathName);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_PARAMETERS:
                       WinQueryWindowText(WinWindowFromID(hwnd,ID_PARAMETERS),sizeof(SaveNodeInfo.Parameters),SaveNodeInfo.Parameters);
                       ChangedNodeInfo = TRUE;
                       break;
                  case ID_WORKDIR:
                       WinQueryWindowText(WinWindowFromID(hwnd,ID_WORKDIR),sizeof(SaveNodeInfo.WorkDir),SaveNodeInfo.WorkDir);
                       ChangedNodeInfo = TRUE;
                       break;
                }
                break;
           case SPBN_CHANGE:
                SaveNodeInfo.PortNum = (char) getspinbuttonvalue(hwnd,ID_PORT);
                ChangedNodeInfo = TRUE;
                break;
         }
         return(0);

    case WM_HELP:
         DisplayHelpPanel(HID_NOTEBOOK_NODE_DLG_HELP_PANEL);
         return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case ID_COPYFROM:
                if (copyfromnode())
                  getnodeinfo(CurNode);
                break;
           case ID_COPYTO:
                if (ChangedNodeInfo)
                  putnodeinfo(CurNode);
                copytonodes();
                break;
           case ID_UNDO:
                if (ChangedNodeInfo)
                  getnodeinfo(CurNode);
                break;
           case ID_RUNSETTINGS:
                turntopage(CurNode+1,TRUE);
                break;
         }
         return(0);
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static MRESULT EXPENTRY NoteBookProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  ULONG ulPageId;
  int   X;
  char  NodeStr[10];
  char  StatusStr[60];
  char  SettingsStr[60];

  switch(ulMsg) {
    case WM_INITDLG:
         InitNoteBook = TRUE;
         centerwindow(hwnd);
         hwndNoteBook = WinWindowFromID(hwnd,ID_NOTEBOOKCTRL);

         WinSendMsg(hwndNoteBook,BKM_SETNOTEBOOKCOLORS,
             MPFROMLONG(SYSCLR_MENU),MPFROMLONG(BKA_BACKGROUNDMAJORCOLOR));
         WinSendMsg(hwndNoteBook,BKM_SETNOTEBOOKCOLORS,
             MPFROMLONG(SYSCLR_MENU),MPFROMLONG(BKA_BACKGROUNDPAGECOLOR));
         WinSendMsg(hwndNoteBook,BKM_SETDIMENSIONS,MPFROM2SHORT(48,28),
             MPFROMLONG(BKA_MAJORTAB));
         WinSendMsg(hwndNoteBook,BKM_SETDIMENSIONS,MPFROM2SHORT(36,21),
             MPFROMLONG(BKA_PAGEBUTTON));

         hwndNodeSettings = WinLoadDlg(hwnd,
                                       hwnd,
                                       NodeDlgProc,
                                       0,
                                       IDD_NODESETTINGS,
                                       NULL);

         hwndRunSettings = WinLoadDlg(HWND_DESKTOP,
                                      hwndMain,
                                      RunDlgProc,
                                      0,
                                      IDD_RUNSETTINGS,
                                      NULL);

         loadstring(IDS_SETTINGSTEXT,SettingsStr,sizeof(SettingsStr));

         for (X = Settings.FirstNode; X <= Settings.LastNode; X++) {
           sprintf(NodeStr,"%d",X);

           ulPageId = (ULONG) WinSendMsg(hwndNoteBook,BKM_INSERTPAGE,(MPARAM)NULL,
                                  MPFROM2SHORT(BKA_MAJOR|BKA_STATUSTEXTON|BKA_AUTOPAGESIZE,
                                  BKA_LAST));

           if (ulPageId != 0) {
             sprintf(StatusStr,SettingsStr,X,1);
             WinSendMsg(hwndNoteBook,BKM_SETPAGEDATA,MPFROMLONG(ulPageId),
                            MPFROMLONG(X));
             WinSendMsg(hwndNoteBook,BKM_SETTABTEXT,MPFROMLONG(ulPageId),
                            MPFROMP(NodeStr));
             WinSendMsg(hwndNoteBook,BKM_SETSTATUSLINETEXT,(MPARAM)ulPageId,
                            MPFROMP(StatusStr));
             WinSendMsg(hwndNoteBook,BKM_SETPAGEWINDOWHWND,MPFROMLONG(ulPageId),
                            MPFROMHWND(hwndNodeSettings));
             aulPageIds[X*2+0] = ulPageId;
           }

           ulPageId = (ULONG) WinSendMsg(hwndNoteBook,BKM_INSERTPAGE,(MPARAM)NULL,
                                  MPFROM2SHORT(BKA_STATUSTEXTON|BKA_AUTOPAGESIZE,
                                  BKA_LAST));

           if (ulPageId != 0) {
             sprintf(StatusStr,SettingsStr,X,2);
             WinSendMsg(hwndNoteBook,BKM_SETPAGEDATA,MPFROMLONG(ulPageId),
                            MPFROMLONG(X));
             WinSendMsg(hwndNoteBook,BKM_SETSTATUSLINETEXT,(MPARAM)ulPageId,
                            MPFROMP(StatusStr));
             WinSendMsg(hwndNoteBook,BKM_SETPAGEWINDOWHWND,MPFROMLONG(ulPageId),
                            MPFROMHWND(hwndRunSettings));
             aulPageIds[X*2+1] = ulPageId;
           }
         }

         turntopage(NoteBookFirstNode,FALSE);
         WinDestroyWindow(hwndScale);
         NeedToSave = FALSE;
         InitNoteBook = FALSE;
         return(0);

    case WM_CONTROL:
         if (SHORT2FROMMP(mp1) == BKN_PAGESELECTED) {
           int               Node;
           PAGESELECTNOTIFY *page = (PAGESELECTNOTIFY *) mp2;

           if (ChangedNodeInfo) {
             Node = (ULONG) WinSendMsg(hwndNoteBook,BKM_QUERYPAGEDATA,MPFROMLONG(page->ulPageIdCur),0);
             if (Node > 0) {
               putnodeinfo(Node-1);
               NeedToSave = TRUE;
             }
             ChangedNodeInfo = FALSE;
           }

           Node = (ULONG) WinSendMsg(hwndNoteBook,BKM_QUERYPAGEDATA,MPFROMLONG(page->ulPageIdNew),0);
           if (Node > 0)
             shownodeinfo(Node-1);

           ChangedNodeInfo = FALSE;
         }
         break;

    case WM_CLOSE:
         if (ChangedNodeInfo) {
           putnodeinfo(CurNode);
           NeedToSave = TRUE;
         }
         break;

//  case WM_COMMAND:
//       switch(LOUSHORT(mp1)) {
//         case DID_OK:
//              break;
//         case DID_CANCEL:
//              break;
//       }
//       break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static void LIBENTRY EditNodes(int FirstNodeToEdit) {
  HWND          hwndDlg;
  nodeinfotype *Save;

  if ((aulPageIds = (PULONG) malloc((MaxNodes+1) * 2 * sizeof(ULONG))) == NULL)
    return;

  if ((Save = (nodeinfotype *) malloc((MaxNodes+1) * sizeof(nodeinfotype))) == NULL) {
    free(aulPageIds);
    return;
  }

  memset(aulPageIds,0,MaxNodes * sizeof(ULONG));
  memcpy(Save,NodeInfo,MaxNodes * sizeof(nodeinfotype));

  NoteBookFirstNode = FirstNodeToEdit;

  hwndScale = WinLoadDlg(HWND_DESKTOP,
                hwndMain,
                ScaleProc,
                0,
                IDD_PROGRESS,
                NULL);

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                hwndMain,
                NoteBookProc,
                0,
                IDD_NOTEBOOK,
                NULL);

  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);

  if (NeedToSave) {
    if (MBID_OK == MessageBox(HWND_DESKTOP,IDMSG_SAVECONFIGCHANGES,IDMSG_CHANGESMADE,MB_OKCANCEL|MB_ICONQUESTION|MB_MOVEABLE,FALSE))
      writenodeprofile();
    else
      memcpy(NodeInfo,Save,MaxNodes * sizeof(nodeinfotype));
    NeedToSave = FALSE;
  }

  free(Save);
  free(aulPageIds);
}

/*********************************************************************/

static void LIBENTRY showrunonconnect(int NodeNumMinusOne) {
  nodetype Buf;

  memset(&Buf,0,sizeof(Buf));
  Buf.Status = 'Q';  // run on connect
  writeusernet(NodeNumMinusOne,&Buf);
  setnetstatusflag(NodeNumMinusOne,TRUE);
}

static void LIBENTRY clearnodestatus(int NodeNumMinusOne) {
  nodetype Buf;

  memset(&Buf,0,sizeof(Buf));
  writeusernet(NodeNumMinusOne,&Buf);
  setnetstatusflag(NodeNumMinusOne,FALSE);
}

static bool LIBENTRY isnoderunning(int NodeNum) {
  STATUSDATA  StatusData;

  NodeNum--;
  if (SessionIds[NodeNum] == 0)
    return(FALSE);

  StatusData.Length    = sizeof(StatusData);
  StatusData.SelectInd = SET_SESSION_UNCHANGED;
  StatusData.BondInd   = SET_SESSION_UNCHANGED;
  if (DosSetSession(SessionIds[NodeNum],&StatusData) == 0)
    return(TRUE);

  LaunchSemaphore.acquire();
  {
    SessionIds[NodeNum] = 0;
    NumNodesRunning--;

    // if this node is set to run on connect, and the current status inside of
    // the usernet file does not indicate that it is set to run on connect,
    // but we do have a semaphore set up to handle it, then the node may have
    // just recently exited so update the usernet now.
    nodeinfotype *p = &NodeInfo[NodeNum];
    if (p->AutoStart == RUN_ONCONNECT &&
        p->PortNum != 0 &&
        (! Settings.DisableConnectRun) &&
        NodeArray[NodeNum].Status == 0 &&
        Semaphores[NodeNum] != 0) {
      showrunonconnect(NodeNum);
    }
  }
  LaunchSemaphore.release();
  return(FALSE);
}


bool LIBENTRY areanynodesrunning(void) {
  if (NumNodesRunning == 0)
    return(FALSE);

  for (int X = 1; X <= MaxNodes; X++)
    if (isnoderunning(X))
      return(TRUE);

  return(FALSE);
}


static void LIBENTRY detectendofsession(void) {
  if (NumNodesRunning == 0)
    return;

  // Scan through all of the nodes and just call the isnoderunning() function.
  // If a node is still running, fine.  If not, isnoderunning() will zero out
  // the sessionid so that it will no longer appear to be active.
  for (int X = 1; X <= MaxNodes; X++)
    isnoderunning(X);
}


static HWND LIBENTRY gethwnd(int SessionId) {
  HAB      hab        = NULLHANDLE;
  ULONG    cbItems;
  ULONG    ulBufSize;
  PSWBLOCK pswblk;
  PSWENTRY p;
  HWND     RetVal;
  int      X;

//if ((RetVal = WinQuerySwitchHandle(NULLHANDLE,SessionId)) != 0)
//  return(RetVal);

  cbItems = WinQuerySwitchList(hab, NULL, 0);
  ulBufSize = (cbItems * sizeof(SWENTRY)) + sizeof(HSWITCH);

  if ((pswblk = (PSWBLOCK) malloc(ulBufSize)) == NULL)
    return(0);

  cbItems = WinQuerySwitchList(hab, pswblk, ulBufSize);

  for (X = 0, RetVal = 0, p = pswblk->aswentry; X < cbItems; X++, p++) {
    if (p->swctl.idSession == SessionId || p->swctl.idProcess == SessionId) {
      RetVal = p->swctl.hwnd;
      break;
    }
  }

  free(pswblk);
  return(RetVal);
}


static void LIBENTRY runorswitchtonode(int NodeNum) {
  int  SessionId;
  HWND hwnd;

  if (isnoderunning(NodeNum)) {
    SessionId = SessionIds[NodeNum-1];
    // if the window type is not full screen (i.e. is either windowed or
    // minimized), then make sure the window is visible first
    if (NodeInfo[NodeNum-1].WindowType != START_FULLSCREEN) {
      if ((hwnd = gethwnd(SessionId)) != 0)
        WinShowWindow(hwnd,TRUE);
      else
        DosBeep(1000,250);
    }
    DosSelectSession(SessionId);
  } else if (NodeInfo[NodeNum-1].PathName[0] != 0)
    launchonenode(NodeNum,FALSE);
}


void LIBENTRY runorswitchtoselectednodes(void) {
  int           Node;
  nodeinfotype *p;

  Node = SelectedNodeFirst;
  p    = &NodeInfo[Node-1];

  for (; Node <= SelectedNodeLast; Node++, p++) {
    if (p->PathName[0] != 0)
      runorswitchtonode(Node);
  }
}

static void LIBENTRY logaction(char *Str, bool UpdateTitle) {
  DOSFILE File;
  char    Date[10];
  char    Time[10];
  char    Output[256];

  if (UpdateTitle)
    WinPostMsg(hwndMain,TM_UPDATETITLE, (MPARAM)Str, (MPARAM)0);

  if (dosfopen(".\\PCBCP.LOG",OPEN_RDWR|OPEN_DENYRDWR|OPEN_APPEND,&File) == -1)
    return;

  datestr(Date);
  timestr1(Time);

  sprintf(Output,"%s %s - %s\r\n",Date,Time,Str);
  dosfputs(Output,&File);
  dosfclose(&File);
}


static void LIBENTRY launchonenode(int NodeNum, bool AutoAnswer) {
  char static      PortStr[20];
  char static      NodeStr[20];
  APIRET           rc;
  nodeinfotype    *p;
  bool             Minimize;
  char             Str[80];
  char             PathAndParams[256];

  LaunchSemaphore.acquire();

    p = &NodeInfo[NodeNum-1];
    sprintf(Str,"Node %d",NodeNum);
    sprintf(PathAndParams,"/c %s %s",p->PathName,p->Parameters);
    if (p->PortNum == 0)
      putenv("PCBPORT=");
    else {
      sprintf(PortStr,"PCBPORT=%d",p->PortNum);
      putenv(PortStr);
    }

    sprintf(NodeStr,"PCBNODE=%d",NodeNum);
    putenv(NodeStr);
    putenv(AutoAnswer ? "PCBANSWER=Y" : "PCBANSWER=");

    clearnodestatus(NodeNum-1);

    if (fileexist(ComSpec) == 255) {
      sprintf(Str,"%s does not exist",ComSpec);
      logaction(Str,FALSE);
    }

    if (fileexist(p->PathName) == 255) {
      sprintf(Str,"%s does not exist",p->PathName);
      logaction(Str,FALSE);
    }

    Minimize = (bool) ((p->WindowType == START_MINIMIZED) || (bool) (p->WindowType == START_FULLSCREEN));
    rc = startsession(ComSpec,PathAndParams,Str,p->WorkDir,FALSE,p->WindowType,&SessionIds[NodeNum-1],NULL,0,FALSE,Minimize);

    if (SessionIds[NodeNum-1] == 0) {
      sprintf(Str,"Error %d launching %d",rc,NodeNum);
      logaction(Str,TRUE);
      logaction(PathAndParams,FALSE);
      logaction(p->WorkDir,FALSE);
    } else
      NumNodesRunning++;

  LaunchSemaphore.release();
}


static void vmodemthread(void *Param) {
  int           X;
  int           rc;
  ULONG         ThrowAway;
  nodeinfotype *p;
  char          Str[80];
  char          Msg[256];

  int NodeNum = (int) Param;

  p = &NodeInfo[NodeNum-1];

  sprintf(Str,"\\SEM32\\VMODEM%d",p->PortNum);

  // give it up to 10 tries, for 1/2 second each, to open the the semaphore
  // and if it can't do it then sound a beep and return

  for (X = 1; X <= 10; X++) {
    if ((rc = DosOpenEventSem(Str,(PHEV) &Semaphores[NodeNum-1])) == 0)
      break;
    if (X == 10) {
      sprintf(Msg,"Unable to open semaphore (error=%d)\r%s",rc,Str);
      PostBkThreadMsg(UM_SEMERROR,Msg,"Semaphore Error");
//    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,Msg,"Semaphore Error",0,MB_ENTER|MB_ERROR|MB_MOVEABLE);
      return;
    }
    DosSleep(500);
  }

  DosResetEventSem(Semaphores[NodeNum-1],&ThrowAway);
  showrunonconnect(NodeNum-1);

  while (1) {
    DosWaitEventSem(Semaphores[NodeNum-1],-1);
    DosResetEventSem(Semaphores[NodeNum-1],&ThrowAway);

    // if they disabled after the event was set up, then break out now
    if (Settings.DisableConnectRun || StopNodes[NodeNum-1]) {
      DosCloseEventSem(Semaphores[NodeNum-1]);
      Semaphores[NodeNum-1] = 0;
      StopNodes[NodeNum-1] = FALSE;
      break;
    }
    // if the node isn't already running, then launch it now
    if (! isnoderunning(NodeNum)) {
      launchonenode(NodeNum,TRUE);
      sprintf(Msg,"Launched Node %d (Run on Connect)",NodeNum);
      logaction(Msg,FALSE);
    }
  }
}

static void LIBENTRY runonconnect(int NodeNum) {
  nodeinfotype *p;

  p = &NodeInfo[NodeNum-1];
  if (p->AutoStart != RUN_ONCONNECT || p->PortNum == 0)
    return;

  #ifdef __BORLANDC__
    _beginthread(vmodemthread,8*1024,(void *)NodeNum);
  #else
    _beginthread(vmodemthread,malloc(12*1024),12*1024,(void *)NodeNum);
  #endif
}

static void LIBENTRY stopthread(int NodeNum) {
  nodetype *p;

  StopNodes[NodeNum-1] = TRUE;
  DosPostEventSem(Semaphores[NodeNum-1]);

  // if the node is already down then clear out the usernet status
  p = &NodeArray[NodeNum-1];
  if (p->Name[0] == 0) {
    p->Status = 0;
    writeusernet(NodeNum-1,p);
  }
}


static void LIBENTRY setvisibilityofnodes(bool Visible) {
  int  NodeNum;
  int  SessionId;
  HWND hwnd;

  for (NodeNum = Settings.FirstNode; NodeNum <= Settings.LastNode; NodeNum++) {
    // if the sessionid is 0 then the node isn't running
    if ((SessionId = SessionIds[NodeNum-1]) == 0)
      continue;
    // if the window type is full screen then we don't need to hide it
    if (NodeInfo[NodeNum-1].WindowType == START_FULLSCREEN)
      continue;
    // if we can't get the handle to the window then we can't hide it...
    if ((hwnd = gethwnd(SessionId)) == 0)
      continue;
    WinShowWindow(hwnd,Visible);
    if (Visible)
      DosSelectSession(SessionId);
    else
      WinSetWindowPos(hwnd,0,0,0,0,0,SWP_MINIMIZE);
  }
}


void LIBENTRY hidenodes(void) {
  setvisibilityofnodes(FALSE);
}


void LIBENTRY shownodes(void) {
  setvisibilityofnodes(TRUE);
}


void LIBENTRY arrangenodes(void) {
  int  X;
  int  Y;
  int  NodeNum;
  int  SessionId;
  int  WindowCount;
  HWND hwnd;
  SWP  swp;

  SkipScreenUpdate = TRUE;

  WindowCount = 0;
  for (NodeNum = Settings.FirstNode; NodeNum <= Settings.LastNode; NodeNum++) {
    // if the sessionid is 0 then the node isn't running
    if ((SessionId = SessionIds[NodeNum-1]) == 0)
      continue;
    // if the window type is full screen then we don't need to arrange it
    if (NodeInfo[NodeNum-1].WindowType == START_FULLSCREEN)
      continue;
    // if we can't get the handle to the window then we can't move it...
    if ((hwnd = gethwnd(SessionId)) == 0)
      continue;
    if (WinQueryWindowPos(hwnd,&swp)) {
      if ((swp.fl & SWP_MINIMIZE) == 0) {
        X = ((WindowCount * swp.cx) / 50) + 16;
        Y = (rclDesktop.yTop - ((WindowCount * swp.cy) / 15) - 16) - swp.cy;
//      X &= 0xFFFFFFF0;
//      Y &= 0xFFFFFFF0;
        WinSetWindowPos(hwnd,HWND_TOP,X,Y,0,0,SWP_MOVE);
        WinSetWindowPos(hwnd,0,0,0,0,0,SWP_MINIMIZE);
        DosSelectSession(SessionId);
        WindowCount++;
      }
    }
  }

  SkipScreenUpdate = FALSE;
}


static void LIBENTRY movewindow(int NodeNum, int WindowCount) {
  int  SessionId;
  int  X;
  int  Y;
  HWND hwnd;
  SWP  swp;

  if ((SessionId = SessionIds[NodeNum-1]) == 0) {
    DosBeep(1000,250);
    return;
  }

  if ((hwnd = gethwnd(SessionId)) == 0) {
    DosBeep(8000,250);
    return;
  }

  if (WinQueryWindowPos(hwnd,&swp)) {
    X = ((WindowCount * swp.cx) / 50) + 16;
    Y = (rclDesktop.yTop - ((WindowCount * swp.cy) / 15) - 16) - swp.cy;
//  X &= 0xFFFFFFF0;
//  Y &= 0xFFFFFFF0;
    WinSetWindowPos(hwnd,HWND_TOP,X,Y,0,0,SWP_MOVE);
  }
}


static void LIBENTRY startnodeorthread(int NodeNum) {
  switch (NodeInfo[NodeNum-1].AutoStart) {
    case RUN_STARTUP:
         launchonenode(NodeNum,FALSE);
         break;
    case RUN_ONCONNECT:
         runonconnect(NodeNum);
         break;
  }
}


#pragma argsused
static void launchprogramsthread(void *Ignore) {
  int           Node;
  int           Count;
  nodeinfotype *p;


  Node = Settings.FirstNode;
  p    = &NodeInfo[Node-1];
  for (Count = 0; Node <= Settings.LastNode; Node++, p++) {
    // we can't start it unless it's defined
    if (p->PathName[0] == 0)
      continue;
    // don't try to start it if it's already running
    if (isnoderunning(Node))
      continue;
    switch (p->AutoStart) {
      case RUN_STARTUP:
           if (! Settings.DisableStartRun) {
             WinPostMsg(hwndMain,TM_UPDATETITLE, (MPARAM)szLaunch, (MPARAM)Node);
             launchonenode(Node,FALSE);
             if (p->WindowType == START_WINDOWED)
               movewindow(Node,Count++);
             mydelay(200);
           }
           break;
      case RUN_ONCONNECT:
           if (! Settings.DisableConnectRun) {
             WinPostMsg(hwndMain,TM_UPDATETITLE, (MPARAM)szLaunch, (MPARAM)Node);
             runonconnect(Node);
           }
           break;
    }
  }

  WinPostMsg(hwndMain,TM_UPDATETITLE, (MPARAM)NULL, (MPARAM)NULL);
}


static void LIBENTRY launchprograms(void) {
  if (NodeInfo == NULL)
    return;

  #ifdef __BORLANDC__
    _beginthread(launchprogramsthread,12*1024,NULL);
  #else
    _beginthread(launchprogramsthread,malloc(12*1024),12*1024,NULL);
  #endif
}


void LIBENTRY restartnodes(void) {
  if (NodeInfo == NULL)
    return;

  launchprograms();
}

/*********************************************************************/

static MRESULT EXPENTRY ObjectWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  HWND hwndMenu;
  int  Node;

  switch(msg) {
    case UM_STARTUP:
         launchprograms();
         break;

    case UM_SEMERROR:
         WinMessageBox(HWND_DESKTOP,hwnd,(char *) mp1,(char *) mp2,0,MB_ENTER|MB_ERROR|MB_MOVEABLE);
         break;

    case WM_TIMER:
         updatescreen(TRUE);
         break;

    case WM_PAINT:
         setscrollbar();
         updatescreen(FALSE);
         break;

    case WM_BUTTON1DBLCLK:
         if (UserNetFile <= 0)
           break;
         deselectnodes();
         if (Settings.ClickToRun) {
           Node = getnodenum(SHORT2FROMMP(mp1));
           if (Node >= 1 && Node <= MaxNodes)
             runorswitchtonode(Node);
         } else {
           selectnode(SHORT2FROMMP(mp1),LASTNODE);
           if (SelectedNodeFirst != 0)
             UserCommand((MPARAM)IDM_EDITRECORD,0);
         }
         break;

    case WM_BUTTON2DOWN:
         if (UserNetFile <= 0)
           break;
         if (SelectedNodeFirst == 0)
           selectnode(SHORT2FROMMP(mp1),STARTNODE);
         if (SelectedNodeFirst != 0) {
           POINTL point;
           WinQueryPointerPos(HWND_DESKTOP,&point);
           WinMapWindowPoints(HWND_DESKTOP,hwnd,&point,1);
           hwndMenu = WinLoadMenu(hwnd,(HMODULE)NULL,IDR_POPUPMENU);
           WinPopupMenu(hwnd,hwnd,hwndMenu,
               (SHORT) point.x,(SHORT) point.y,
               904,PU_POSITIONONITEM|PU_HCONSTRAIN|PU_VCONSTRAIN|
               PU_MOUSEBUTTON1);
//             PU_MOUSEBUTTON2DOWN|PU_MOUSEBUTTON2);
         }
         break;

    case WM_BUTTON1DOWN:
         if (UserNetFile <= 0)
           break;
         WinSetActiveWindow(HWND_DESKTOP,hwndMainFrame);
         selectnode(SHORT2FROMMP(mp1),STARTNODE);
         break;

    case WM_MOUSEMOVE:
    case WM_BUTTON1UP:
         if (UserNetFile <= 0)
           break;
         if (SelectedNodeFirst != 0)
           selectnode(SHORT2FROMMP(mp1),LASTNODE);
         break;

    case WM_VSCROLL:
         deselectnodes();
         switch (SHORT2FROMMP(mp2)) {
           case SB_LINEUP:
                lineup();
                setscrollbar();
                updatescreen(FALSE);
                break;
           case SB_LINEDOWN:
                linedown();
                setscrollbar();
                updatescreen(FALSE);
                break;
           case SB_PAGEUP:
                pageup();
                setscrollbar();
                updatescreen(FALSE);
                break;
           case SB_PAGEDOWN:
                pagedown();
                setscrollbar();
                updatescreen(FALSE);
                break;
           case SB_SLIDERTRACK:
           case SB_SLIDERPOSITION:
                FirstNodeOnScreen = SHORT1FROMMP(mp2);
                updatescreen(FALSE);
                break;
         }
         showselectednodes();  // if there is a node selected, re-indicate it
         break;

    case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == IDM_FILENODE) {
           EditNodes(Settings.FirstNode);
           return(0);
         }
         if (SelectedNodeFirst != 0) {
           switch (SHORT1FROMMP(mp1)) {
             case IDM_DROPDOS:
                  confirmchange(hwnd,'W');
                  break;
             case IDM_DROPDOSNOW:
                  confirmchange(hwnd,'X');
                  break;
             case IDM_LOGOFF:
                  confirmchange(hwnd,'L');
                  break;
             case IDM_RECYCLE:
                  confirmchange(hwnd,'R');
                  break;
             case IDM_CLEAR:
                  confirmchange(hwnd,0);
                  break;
             case IDM_POPUPEDIT:
                  if (SelectedNode == 0 || (SelectedNodeFirst != SelectedNodeLast))
                    DosBeep(1000,250);
                  else
                    EditRecord();
                  break;
             case IDM_NOTEBOOK:
                  if (SelectedNode == 0 || (SelectedNodeFirst != SelectedNodeLast))
                    DosBeep(1000,250);
                  else
                    EditNodes(SelectedNode);
                  break;
             case IDM_RUNNODES:
                  if (SelectedNode == 0)
                    DosBeep(1000,250);
                  else
                    runorswitchtoselectednodes();
                  break;
           }
           deselectnodes();
           updatescreen(TRUE);
         }
         return(0);

    default:
         return (WinDefWindowProc(hwnd, msg, mp1, mp2));
  }
  // Turn off "unreacheable code" warning
  #pragma warn -rch
  return (MRESULT)NULL;
} /* End of ObjectWndProc   */


static void LIBENTRY setattentionflags(unsigned NodeNumMinusOne) {
  int  Offset;
  char BitMask;
  char Flag;

  BitMask = (char) (1 << (NodeNumMinusOne & 7));
  Offset  = sizeof(usernethdrtype) + (NodeNumMinusOne / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(char)) != -1) {
    if (readcheck(UserNetFile,&Flag,sizeof(char)) != (unsigned) -1) {
      Flag |= BitMask;
      doslseek(UserNetFile,Offset,SEEK_SET);
      writecheck(UserNetFile,&Flag,sizeof(char));
    }
    unlock(UserNetFile,Offset,sizeof(char));
  }
}


static void LIBENTRY setnetstatusflag(int NodeNumMinusOne, bool On) {
  int  Offset;
  char BitMask;
  char Flag;

  BitMask = (char) (1 << (NodeNumMinusOne & 7));
  Offset  = USERNETFLAGS + (NodeNumMinusOne / 8);

  doslseek(UserNetFile,Offset,SEEK_SET);
  if (doslockcheck(UserNetFile,Offset,sizeof(char)) != -1) {
    if (readcheck(UserNetFile,&Flag,sizeof(char)) != (unsigned) -1) {
      if (On)
        Flag |= BitMask;
      else
        Flag &= (char) ~BitMask;
      doslseek(UserNetFile,Offset,SEEK_SET);
      writecheck(UserNetFile,&Flag,sizeof(char));
    }
    unlock(UserNetFile,Offset,sizeof(char));
  }
}


static int LIBENTRY readusernet(int FirstNodeToRead, int NumToRead) {
  os2errtype Os2Error;

  doslseek(UserNetFile,FirstNodeToRead*sizeof(nodetype)+USERNETSTART,SEEK_SET);

  int BytesToRead = NumToRead * sizeof(nodetype);
  if (dosread(UserNetFile,&NodeArray[FirstNodeToRead],BytesToRead,&Os2Error) != BytesToRead)
    return(-1);

  return(0);
}


static void LIBENTRY writeusernet(int NodeNumMinusOne, nodetype *Buf) {
  stripright(Buf->Name,' ');
  stripright(Buf->City,' ');
  stripright(Buf->Operation,' ');
  stripright(Buf->Message,' ');

  // if the status says message but there is no message to send, then
  // blank out the message
  if (Buf->Status != 'M')
    memset(Buf->Message,0,sizeof(Buf->Message));

  Buf->LastUpdate.Split.Date = getjuliandate();
  Buf->LastUpdate.Split.Time = (uint) (exacttime() / 2);
  doslseek(UserNetFile,NodeNumMinusOne*sizeof(nodetype)+USERNETSTART,SEEK_SET);

  if (writecheck(UserNetFile,Buf,sizeof(nodetype)) != (unsigned) -1) {
    if (Buf->Status != SaveArray[NodeNumMinusOne].Status) {
      if (Buf->Status == 0)
        setnetstatusflag(NodeNumMinusOne,FALSE);
      if (SaveArray[NodeNumMinusOne].Status == 0)
        setnetstatusflag(NodeNumMinusOne,TRUE);
      switch (Buf->Status) {
        case 'M':
        case 'R':
        case 'W':
        case 'X': setattentionflags(NodeNumMinusOne);
                  break;
      }
    }
  }
}


void LIBENTRY writeselectednode(nodetype *Buf) {
  writeusernet(SelectedNode-1,Buf);
  updatescreen(TRUE);
}


static void LIBENTRY writeselectednodes(char Status) {
  int X;

  if (SelectedNodeFirst != 0) {
    // if desired status is 0, we're going to memset the whole thing anyway,
    // otherwise we'll be updating the existing settings so read them in
    if (Status != 0)
      readusernet(SelectedNodeFirst-1,SelectedNodeLast-SelectedNodeFirst+1);

    for (X = SelectedNodeFirst-1; X <= SelectedNodeLast-1; X++) {
      if (NodeArray[X].Status == 0 && (Status == 'X' || Status == 'W' || Status == 'L' || Status == 'R'))
        continue;

      if (Status == 0)
        memset(&NodeArray[X],0,sizeof(nodetype));
      else
        NodeArray[X].Status = Status;

      writeusernet(X,&NodeArray[X]);
    }
  }

  updatescreen(TRUE);
}


void LIBENTRY setscrollbar(void) {
  int Max;
  int TotalNodes;

  TotalNodes = Settings.LastNode - Settings.FirstNode + 1;

  if (TotalNodes > numWinLines) {
    Max = TotalNodes - numWinLines;
    WinEnableWindow(hwndVertScroll,TRUE);
    WinSendMsg(hwndVertScroll,SBM_SETSCROLLBAR,(MPARAM)FirstNodeOnScreen,MPFROM2SHORT(0,Max));
    WinSendMsg(hwndVertScroll,SBM_SETTHUMBSIZE,MPFROM2SHORT(numWinLines,TotalNodes),0);
  } else
    WinEnableWindow(hwndVertScroll,FALSE);
}


static void LIBENTRY lineup(void) {
  if (FirstNodeOnScreen > 0)
    FirstNodeOnScreen--;
}


static void LIBENTRY linedown(void) {
  if ((FirstNodeOnScreen + numWinLines) < Settings.LastNode)
    FirstNodeOnScreen++;
}


static void LIBENTRY pageup(void) {
  if ((FirstNodeOnScreen - numWinLines) > (Settings.FirstNode-1))
    FirstNodeOnScreen -= numWinLines;
  else
    FirstNodeOnScreen = Settings.FirstNode - 1;
}


static void LIBENTRY pagedown(void) {
  if ((FirstNodeOnScreen + numWinLines) < Settings.LastNode) {
    if ((FirstNodeOnScreen + numWinLines) < (Settings.LastNode - numWinLines)) {
      FirstNodeOnScreen += numWinLines;
    } else {
      if ((Settings.LastNode - numWinLines) <= 0)
        FirstNodeOnScreen = Settings.FirstNode;
      else
        FirstNodeOnScreen = Settings.LastNode - numWinLines;
    }
  }
}


static void LIBENTRY drawbox(int NodeStart, int NodeEnd, bool Select) {
  POINTL pt;
  RECTL  rc;

  // Tor some reason, if the user has moved the window on the screen, the
  // calls to GpiMove() and GpiBox() will draw the box incorrectly.  The
  // two lines below appear to work around this problem by resetting the
  // viewport back to the origin.
  WinQueryWindowRect(HWND_DESKTOP,&rc);
  GpiSetPageViewport(hpsMain,&rc);

  // move to the bottom left corner of the box
  pt.x = 3;
  pt.y = yTop - ((NodeEnd - FirstNodeOnScreen) * fontHeight) - 2;
  GpiMove(hpsMain,&pt);

  if (Select) {
    GpiSetLineType(hpsMain,LINETYPE_DOT);
  } else {
    GpiSetColor(hpsMain,CLR_BACKGROUND);      // set text color
    GpiSetLineType(hpsMain,LINETYPE_SOLID);
  }

  // move to the top right corner of the box and draw it
  pt.x  = xRight - 3;
  pt.y += (fontHeight * (NodeEnd-NodeStart+1)) - 2;
  GpiBox(hpsMain,DRO_OUTLINE,&pt,10,10);

  if (! Select)
    GpiSetColor(hpsMain,CLR_NEUTRAL);         // set text color
}


void LIBENTRY deselectnodes(void) {
  if (SelectedNodeFirst != 0) {
    drawbox(SelectedNodeFirst,SelectedNodeLast,FALSE);
    SelectedNode = SelectedNodeFirst = SelectedNodeLast = 0;
  }
}


static void LIBENTRY showselectednodes(void) {
  if (SelectedNodeFirst != 0) {
    if (SelectedNodeFirst < (FirstNodeOnScreen+1) || SelectedNodeLast > FirstNodeOnScreen+numWinLines)
      SelectedNode = SelectedNodeFirst = SelectedNodeLast = 0;
    else
      drawbox(SelectedNodeFirst,SelectedNodeLast,TRUE);
  }
}


static int LIBENTRY getnodenum(int Y) {
  if (fontHeight == 0)
    return(0);

  int NodeNum = ((yTop - Y) / fontHeight) + FirstNodeOnScreen + 1;

  if (NodeNum < Settings.FirstNode)
    NodeNum = Settings.FirstNode;

  if (NodeNum > Settings.LastNode)
    NodeNum = Settings.LastNode;

  if (NodeNum > FirstNodeOnScreen + numWinLines)
    NodeNum = FirstNodeOnScreen + numWinLines;

  return(NodeNum);
}


void LIBENTRY selectnode(int Y, bool StartNode) {
  int NewNode;

  if (SelectedNodeFirst != 0)
    drawbox(SelectedNodeFirst,SelectedNodeLast,FALSE);

  if ((NewNode = getnodenum(Y)) == 0) {
    SelectedNode = SelectedNodeFirst = SelectedNodeLast = 0;
    return;
  }

  if (StartNode) {
    if (SelectedNodeFirst == NewNode) {
      SelectedNode = SelectedNodeFirst = SelectedNodeLast = 0;
      return;
    } else
      SelectedNodeFirst = SelectedNodeLast = NewNode;
  } else {
    if (SelectedNodeFirst != NewNode) {
      if (SelectedNodeFirst == 0)
        SelectedNodeFirst = SelectedNodeLast = NewNode;
      else {
        if (SelectedNodeFirst < NewNode)
          SelectedNodeLast = NewNode;
        else {
          SelectedNodeLast  = SelectedNodeFirst;
          SelectedNodeFirst = NewNode;
        }
      }
    }
  }

  drawbox(SelectedNodeFirst,SelectedNodeLast,TRUE);
  SelectedNode = SelectedNodeFirst;
}


char *Options[NUMOPTIONS] = {
  "(Inactive Node)",
  "No Caller this Node",
  "Available for Chat",
  "Out to DOS",
  "Chatting with Sysop",
  "Inside a Door",
  "Entering a Message",
  "Viewing a File",
  "Chatting with Group",
  "Handling Mail",
  "Auto Logoff Pending",
  "Broadcast Msg",
  "Running Event",
  "Logging into System",
  "Paging the Sysop",
  "Run on Connect",
  "DOS Recycle Pending",
  "Answering Script",
  "Transferring a File",
  "Unavailable for Chat",
  "Drop to DOS at Logoff",
  "Drop to DOS - Urgent!"
};

int LIBENTRY getstatusnum(int Status) {
  switch (Status) {
    case  0 : return(0);
    case ' ': return(1);
    case 'A': return(2);
    case 'B': return(3);
    case 'C': return(4);
    case 'D': return(5);
    case 'E': return(6);
    case 'F': return(7);
    case 'G': return(8);
    case 'H': return(9);
    case 'L': return(10);
    case 'M': return(11);
    case 'N': return(12);
    case 'O': return(13);
    case 'P': return(14);
    case 'Q': return(15);
    case 'R': return(16);
    case 'S': return(17);
    case 'T': return(18);
    case 'U': return(19);
    case 'W': return(20);
    case 'X': return(21);
  }
  return(0);
}


char LIBENTRY getstatusletter(int Num) {
  switch (Num) {
    case  0: return( 0 );
    case  1: return(' ');
    case  2: return('A');
    case  3: return('B');
    case  4: return('C');
    case  5: return('D');
    case  6: return('E');
    case  7: return('F');
    case  8: return('G');
    case  9: return('H');
    case 10: return('L');
    case 11: return('M');
    case 12: return('N');
    case 13: return('O');
    case 14: return('P');
    case 15: return('Q');
    case 16: return('R');
    case 17: return('S');
    case 18: return('T');
    case 19: return('U');
    case 20: return('W');
    case 21: return('X');
  }
  return(0);
}


static char * LIBENTRY getstatustext(int Status) {
  return(Options[getstatusnum(Status)]);
}


static bool LIBENTRY isold(long DateTime, int CurrentDate, int CurrentTime, char Status, bool AddRestartDelay) {
  updttype LastUpdate;
  int      ElapsedTime;
  int      Limit;

  switch (Status) {
    case 'Q': return(FALSE);  // run on connect should not cause an alarm
    case 'N': Limit = Settings.WarnEventSeconds; break;
    case 'D': Limit = Settings.WarnDoorSeconds;  break;
    case 'T': Limit = Settings.WarnXferSeconds;  break;
    default : Limit = Settings.WarnSeconds;      break;
  }

  // if we are checking to restart a node, then add the extra time limit used
  // to avoid restarting a node before the sysop has a chance to look at it
  if (AddRestartDelay)
    Limit += Settings.WarnRestartSeconds;

  LastUpdate.DateTime = DateTime;
  if (LastUpdate.Split.Date <= 2) {
    ElapsedTime = CurrentTime - LastUpdate.DateTime;
    if (ElapsedTime < 0)
      ElapsedTime += (24 * 60 * 60);
    if (ElapsedTime > (23*60*60 + 30*60))  // ignore 30 minute variance in clock
      ElapsedTime = 0;
  } else {
    if (LastUpdate.Split.Date == CurrentDate)
      ElapsedTime = CurrentTime - ((int) LastUpdate.Split.Time * 2);
    else if (CurrentDate > LastUpdate.Split.Date) {
      ElapsedTime = CurrentTime - ((int) LastUpdate.Split.Time * 2)
                  + ((CurrentDate - LastUpdate.Split.Date) * (24 * 60 * 60));
    } else
      ElapsedTime = 0;
  }

  return((bool) (ElapsedTime > Limit));
}


static void LIBENTRY stopnode(int NodeNum) {
  DosStopSession(STOP_SESSION_SPECIFIED,SessionIds[NodeNum-1]);
}


#pragma argsused
static void restartinactivenodesthread(void *Ignore) {
  bool static InHere;
  int       X;
  int       Node;
  int       FirstNodeToCheck;
  int       NumToCheck;
  int       Count;
  int       CurrentDate;
  int       CurrentTime;
  nodetype *p;
  char      Action[128];
  char      NodeName[128];
  char      CurStatus[256];

  if (InHere)
    return;

  InHere = TRUE;

  // disable the screen update so that we do not accidently restart
  // this thread and go recursive
  SkipScreenUpdate = TRUE;

  if ((FirstNodeOnScreen < Settings.FirstNode-1) || (FirstNodeOnScreen > Settings.LastNode-1))
    FirstNodeOnScreen = Settings.FirstNode-1;

  if (Settings.MonitorAll) {
    FirstNodeToCheck = Settings.FirstNode-1;
    NumToCheck       = Settings.LastNode - Settings.FirstNode + 1;
  } else {
    if ((FirstNodeOnScreen < Settings.FirstNode-1) || (FirstNodeOnScreen > Settings.LastNode-1))
      FirstNodeOnScreen = Settings.FirstNode-1;

    FirstNodeToCheck = FirstNodeOnScreen;
    if ((Settings.LastNode - FirstNodeOnScreen) > numWinLines)
      NumToCheck = numWinLines;
    else
      NumToCheck = Settings.LastNode - FirstNodeOnScreen;
  }

  CurrentDate = getjuliandate();
  CurrentTime = exacttime();

  for (Count=0, Node = FirstNodeToCheck + 1, p=&NodeArray[FirstNodeOnScreen];
       Count < NumToCheck;
       Count++, Node++, p++) {

    // is this node defined?  if not, skip it
    if (NodeInfo[Node-1].PathName[0] == 0)
      continue;

    // don't try to restart a node whose status indicates it is offline
    if (p->Status == 0)
      continue;

    // is the node inactive for it's time plus 1 minute?
    if (isold(p->LastUpdate.DateTime,CurrentDate,CurrentTime,p->Status,TRUE)) {
      // yes, is the node running as a child of PCBCP?
      if (isnoderunning(Node)) {
        // yes, the node is running - begin shutdown procedure

        // record the current node status
        if (p->Operation[0] != 0 &&
            (p->Status == 'D' ||
             p->Status == 'N' ||
             p->Status == ' ')) {
          strcpy(NodeName,p->Operation);
        } else {
          if (p->Name[0] == 0)
            NodeName[0] = 0;
          else
            strcpy(NodeName,p->Name);
        }

        // log the fact that we are attempting to close the node
        sprintf(Action,"Attempting to close Node %d (Inactive)",Node);
        logaction(Action,TRUE);

        // record what the current status of the node was
        sprintf(CurStatus,"Node %d Status was: %s %s",Node,NodeName,getstatustext(p->Status));
        logaction(CurStatus,FALSE);

        // stop the session now
        stopnode(Node);

        // then wait for up to 30 seconds for the window to close
        for (X = 0; X < 15; X++) {
          DosSleep(2000);
          // if node has exited, then break out now
          if (! isnoderunning(Node))
            break;
        }

        // then check to make sure the node is no longer running and, if not,
        // try restarting the node
        if (isnoderunning(Node)) {
          sprintf(Action,"Node %d still active (did not exit).",Node);
          logaction(Action,TRUE);
        } else {
          // record that we're about to restart the node
          sprintf(Action,"Node %d stopped.  Attempting to restart now.",Node);
          logaction(Action,TRUE);
          // try starting the node now
          startnodeorthread(Node);
          // wait up to 30 more seconds to allow the node to be reloaded
          // so that we don't accidently attempt to *again* restart the node
          for (X = 0; X < 30; X++) {
            DosSleep(2000);
            // if node has been restarted then break out now
            if (isnoderunning(Node)) {
              sprintf(Action,"Node %d is now running.",Node);
              logaction(Action,TRUE);
              break;
            }
          }

          // if the node didn't get restarted then write an error message
          if (! isnoderunning(Node)) {
            sprintf(Action,"Node %d is still down (did not restart).",Node);
            logaction(Action,TRUE);
          }
        }
      }
    }
  }

  // we're done, re-enable the screen updates and exit this thread
  SkipScreenUpdate = FALSE;
  InHere = FALSE;

  // reset the title text
  WinPostMsg(hwndMain,TM_UPDATETITLE, (MPARAM)NULL, (MPARAM)NULL);
}


static void LIBENTRY restartinactivenodes(void) {
  #ifdef __BORLANDC__
    _beginthread(restartinactivenodesthread,16*1024,NULL);
  #else
    _beginthread(restartinactivenodesthread,malloc(16*1024),12*1024,NULL);
  #endif
}


static void LIBENTRY updatescreen(bool RefreshOnly) {
  bool      Paged;
  bool      WatchForOld;
  bool      Old;
  bool      FoundOld;
  bool      ChangedColor;
  int       i;
  int       Node;
  int       FirstNodeToRead;
  int       NumToRead;
  int       Count;
  int       Col1;
  int       Col2;
  int       Col3;
  int       Adjust;
  int       CurrentDate;
  int       CurrentTime;
  nodetype *p;
  nodetype *q;
  char     *Msg;
  POINTL    pt;
  RECTL     rc;
  char      Str[80];
  char      NodeName[80];

  // we may be updating something else, or the usernet file may not be opened,
  // in either case, avoid running this function and return

  if (SkipScreenUpdate || UserNetFile <= 0)
    return;

  // if PCBCP is minimized, we may want to return -- unless we are set to
  // monitor old nodes or sound the page alarm or bring a paging node to
  // the foreground, or restart an inactive node, in which case we still need
  // to scan the file
  if (MainIsMinimized && ! (Settings.WarnOld || Settings.PageAlarmEnabled || Settings.PageBringToFore || Settings.RestartInactive))
    return;

  if ((FirstNodeOnScreen < Settings.FirstNode-1) || (FirstNodeOnScreen > Settings.LastNode-1))
    FirstNodeOnScreen = Settings.FirstNode-1;

  // we'll read a reduced section of the usernet file *unless* we're set to
  // monitor all nodes -- and even then, only if we'll have something to "do"
  // after having read in all of the node settings
  if (Settings.MonitorAll && (Settings.WarnOld || Settings.PageAlarmEnabled || Settings.PageBringToFore)) {
    FirstNodeToRead = Settings.FirstNode-1;
    NumToRead       = Settings.LastNode - Settings.FirstNode + 1;
  } else {
    FirstNodeToRead = FirstNodeOnScreen;
    if ((Settings.LastNode - FirstNodeOnScreen) > numWinLines)
      NumToRead = numWinLines;
    else
      NumToRead = Settings.LastNode - FirstNodeOnScreen;
  }

  if (readusernet(FirstNodeToRead,NumToRead) == -1)
    return;

  detectendofsession();

  if (Settings.LastNode < 10)
    Adjust = 2;
  else if (Settings.LastNode < 100)
    Adjust = 3;
  else
    Adjust = 4;

  Col1 = fontWidth;
  Col2 = fontWidth * (Adjust + 1);
  Col3 = fontWidth * (Adjust + 23);

  if (fontProportional) {
    Col2 += fontWidth;
    Col3 += fontWidth*4;
  }

  if (fontKerning)
    Col3 += fontWidth;

  Paged        = FALSE;
  Old          = FALSE;
  ChangedColor = FALSE;
  Node         = FirstNodeOnScreen + 1;

  WatchForOld = Settings.ShowOld | Settings.WarnOld;
  if (WatchForOld) {
    FoundOld    = FALSE;
    CurrentDate = getjuliandate();
    CurrentTime = exacttime();
  }

  for (Count=0, i=yTop-fontHeight, p=&NodeArray[FirstNodeOnScreen], q=&SaveArray[FirstNodeOnScreen];
       Count < NumToRead;
       i -= fontHeight, Count++, Node++, p++, q++) {

    if (WatchForOld && p->Status != 0) {
      Old = isold(p->LastUpdate.DateTime,CurrentDate,CurrentTime,p->Status,FALSE);
      FoundOld |= Old;

      if (Old && Settings.ShowOld && ! MainIsMinimized) {
        GpiSetColor(hpsMain,CLR_RED);
        ChangedColor = TRUE;
      }
    }

    if (p->Status == 'P') {
      Paged = TRUE;
      if (! MainIsMinimized) {
        GpiSetColor(hpsMain,CLR_DARKGREEN);
        ChangedColor = TRUE;
      }
      if (Settings.PageBringToFore && ! BroughtToFore) {
        if (isnoderunning(Node)) {
          DosSelectSession(SessionIds[Node-1]);
          BroughtToFore = TRUE;
        }
      }
    }

//  if (RefreshOnly && (! Old) && memcmp(p,q,sizeof(nodetype)) == 0)
    if (RefreshOnly && (! Old) && memcmp(p,q,LENGTHOFESSENTIALS) == 0)
      continue;

    // check to see if a Run On Connect node is set to drop to dos, if so
    // stop the thread now and clear the drop status
    if (Semaphores[Node-1] != 0 && (p->Status == 'W' || p->Status == 'X'))
      stopthread(Node);

    if (! MainIsMinimized) {
      pt.y = i;
      pt.x = Col1;

      rc.yTop    = pt.y + fontHeight - fontDescend;
      rc.yBottom = pt.y - fontDescend;
      rc.xLeft   = pt.x;
      rc.xRight  = Col3;
      WinFillRect(hpsMain,&rc,SYSCLR_WINDOW);

      sprintf(Str,"%d",Node);
      GpiCharStringAt(hpsMain,&pt,strlen(Str),(PSZ)Str);

      Msg = getstatustext(p->Status);
      pt.x = Col2;
      GpiCharStringAt(hpsMain,&pt,strlen(Msg),(PSZ)Msg);

      if (p->Operation[0] != 0 &&
          (p->Status == 'D' ||
           p->Status == 'N' ||
           p->Status == ' ')) {
        strcpy(NodeName,p->Operation);
      } else {
        if (p->Name[0] == 0)
          NodeName[0] = 0;
        else {
//        if (RefreshOnly && ! Settings.ShowOld) {
//          if (strcmp(p->Name,q->Name) == 0 && strcmp(p->City,q->City) == 0)
//            continue;
//        }
          strcpy(NodeName,p->Name);
          if (p->City[0] != 0) {
            strcat(NodeName," (");
            strcat(NodeName,p->City);
            strcat(NodeName,")");
          }
        }
      }

      pt.x = Col3;

      rc.xLeft   = pt.x;
      rc.xRight  = xRight;
      WinFillRect(hpsMain,&rc,SYSCLR_WINDOW);
      GpiCharStringAt(hpsMain,&pt,strlen(NodeName),(PSZ)NodeName);

      if (ChangedColor) {
        GpiSetColor(hpsMain,CLR_NEUTRAL);
        ChangedColor = FALSE;
      }
    }
  }

  if (hwndEditDlg != 0)
    WinPostMsg(hwndEditDlg,WM_TIMER,(MPARAM)NULL,(MPARAM)NULL);

  memcpy(&SaveArray[FirstNodeToRead],&NodeArray[FirstNodeToRead],NumToRead*sizeof(nodetype));

  if (ChangedColor) {
    GpiSetColor(hpsMain,CLR_NEUTRAL);
    ChangedColor = FALSE;
  }

  if (RefreshOnly) {
    // If we're supposed to be monitoring ALL of the nodes and we didn't
    // find any nodes paging us, then we better scan any that we may have
    // missed just to be sure that all nodes are scanned.
    if (Settings.MonitorAll && (Settings.PageAlarmEnabled || Settings.PageBringToFore) && ! Paged) {
      for (Count=Settings.FirstNode-1, p=&NodeArray[Settings.FirstNode-1];
        Count < Settings.LastNode-1;
        Count++, p++) {
        if (Count < FirstNodeOnScreen || Count > (FirstNodeOnScreen+numWinLines)) {
          if (p->Status == 'P') {
            Paged = TRUE;
            if (Settings.PageBringToFore && ! BroughtToFore) {
              if (isnoderunning(Node)) {
                DosSelectSession(SessionIds[Node-1]);
                BroughtToFore = TRUE;
              }
            }
          }
        }
      }
    }

    if (Settings.WarnOld) {
      // If we're supposed to be monitoring ALL of the nodes and we didn't
      // find any old nodes, then we better scan any that we may have missed
      // just to be sure that all nodes are scanned.
      if (Settings.MonitorAll && ! FoundOld) {
        for (Count=Settings.FirstNode-1, p=&NodeArray[Settings.FirstNode-1];
          Count < Settings.LastNode-1;
          Count++, p++) {
          if (Count < FirstNodeOnScreen || Count > (FirstNodeOnScreen+numWinLines)) {
            if (p->Status != 0 && isold(p->LastUpdate.DateTime,CurrentDate,CurrentTime,p->Status,FALSE)) {
              FoundOld = TRUE;
              break;
            }
          }
        }
      }

      if (FoundOld) {
        DosBeep(800,150);
        DosBeep(600,150);
        DosBeep(800,150);
        DosBeep(600,150);
        DosBeep(800,150);
      }
    }


    // this test is outside of the above code because we need to allow the
    // restart option even if the "sound" is disabled
    if (FoundOld) {
      if (Settings.RestartInactive)
        restartinactivenodes();
    }

    if (Paged && Settings.PageAlarmEnabled) {
      if (FoundOld)
        DosSleep(500);
      DosBeep(400,150);
      DosBeep(400,150);
    }
  }

  if (! MainIsMinimized)
    showselectednodes();

  // if a node paged, then we brought the nodes to the foreground and we
  // don't want to do it again until after the node stops paging
  if (! Paged)
    BroughtToFore = FALSE;
}


/***************************  End of thrd.c  ****************************/


