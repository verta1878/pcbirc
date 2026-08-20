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

#define  INCL_PM
#define  INCL_WINHELP
#include <os2.h>
#include "main.h"
#include "help.h"
#include "xtrn.h"


#ifdef VALIDATOR
  extern "C" {
  #include <\toolkt21\c\os2h\valapi.h>
  }
#endif

/*
 *  Global variables
 */
HWND hwndMainFrame = NULLHANDLE;    /* handle to the main frame window */
HWND hwndMain;                      /* handle to the main client window */
HWND hwndVertScroll;                /* handle to the vertical scrollbar */
// HDC  hdcMain;                       /* handle to the DC of the client */
HAB  hab;                           /* anchor block for the process */
HMQ  hmq;                           /* handle to the process' message queue */
CHAR szAppName[MAXNAMEL];           /* buffer for application name string */
CHAR szLaunch[MAXNAMEL];            // buffer used when launching nodes
BOOL fHelpEnabled;                  /* flag to determine if help is enabled */

HPS  hpsMain;                       // PS of main client window
int  fontWidth;
int  fontHeight;
int  fontDescend;
int  fontProportional;
int  fontKerning;
int  numWinLines;
int  yTop;
int  xRight;
bool MainIsMinimized = FALSE;

static bool Button1Down;

/*
 *  Entry point declarations
 */
static VOID LIBENTRY MainCommand(MPARAM mp1, MPARAM mp2);


/**************************************************************************
 *
 *  Name       : main()
 *
 *  Description: Initializes the PM environment, calls the
 *               initialization routine, creates the main
 *               window,  and polls the message queue
 *
 *  Concepts:    - obtains anchor block handle and creates message
 *                    queue
 *               - calls the initialization routine
 *               - creates the main frame window which creates the
 *                    main client window
 *               - polls the message queue via Get/Dispatch Msg loop
 *               - upon exiting the loop, exits
 *
 *  API's      :  WinInitialize
 *                DosBeep
 *                WinCreateMsgQueue
 *                WinTerminate
 *                WinCreateStdWindow
 *                WinOpenWindowDC
 *                WinGetMsg
 *                WinDispatchMsg
 *
 *  Parameters :  [none]
 *
 *  Return     :  0 - if successful execution completed
 *                1 - if error
 *
 *************************************************************************/
int main(VOID) {
   QMSG qmsg;          /* message structure */
   ULONG flCtlData;    /* frame control data */

   #ifdef VALIDATOR
     ValInitialize("PCBCP.EXE",NULL,VL_VIEWPORT);
   #endif

   if ((hab = WinInitialize(0)) == NULLHANDLE) {
     DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
     return(RETURN_ERROR);
   }

   if ((hmq = WinCreateMsgQueue(hab,100)) == NULLHANDLE) {
     DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
     WinTerminate(hab);
     return(RETURN_ERROR);
   }

   if (! Init()) {
     MessageBox(HWND_DESKTOP,IDMSG_INITFAILED,0,MB_OK|MB_ERROR,TRUE);
     return(RETURN_ERROR);
   }

   /* NOTE:  clean up from here is handled by the DosExitList processing */


   /* create the main window */
   flCtlData = FCF_STANDARD | FCF_VERTSCROLL;

   hwndMainFrame = WinCreateStdWindow(HWND_DESKTOP,
                                      (isprofilesizeloaded() ? 0 : WS_VISIBLE),
                                      &flCtlData,
                                      (PSZ)szAppName,
                                      (PSZ)NULL,
                                      WS_VISIBLE,
                                      (HMODULE)NULL,
                                      IDR_MAIN,
                                      (PHWND)&hwndMain);

   if (NULLHANDLE == hwndMainFrame) {
     MessageBox(HWND_DESKTOP,IDMSG_MAINWINCREATEFAILED,0,MB_OK|MB_ERROR,TRUE);
     return(RETURN_ERROR);
   }


   // hdcMain = WinOpenWindowDC(hwndMain);

   hpsMain = WinGetPS(hwndMain);
   GpiSetColor(hpsMain,CLR_NEUTRAL);         // set text color
   GpiSetBackColor(hpsMain,CLR_BACKGROUND);  // set text background
   GpiSetBackMix(hpsMain,BM_LEAVEALONE);     // set text mix

   setprofilesize();
   setprofilefont();
   getfontparams();

   hwndVertScroll = WinWindowFromID(hwndMainFrame,FID_VERTSCROLL);
   setscrollbar();
   setmenuitems();

   InitHelp();

   if (! CreateBackgroundThread()) {
     MessageBox(HWND_DESKTOP,IDMSG_INITFAILED,0,MB_OK|MB_ERROR,TRUE);
     return(RETURN_ERROR);
   }

   WinSetFocus(HWND_DESKTOP,hwndVertScroll);
   WinStartTimer(hab,hwndMain,0,Settings.UpdateInterval * 1000);

   /* Get/Dispatch Message loop */
   while(WinGetMsg(hmq, (PQMSG)&qmsg, (HWND)NULL, (ULONG)NULL, (ULONG)NULL))
       WinDispatchMsg(hmq, (PQMSG)&qmsg);

   /* destroy the help instance */
   DestroyHelpInstance();
   DestroyBackgroundThread();

   WinReleasePS(hpsMain);


   return(RETURN_SUCCESS);
}   /* End of main   */


/**************************************************************************
 *
 *  Name       : MainWndProc(hwnd, msg, mp1, mp2)
 *
 *  Description:  Processes the messages sent to the main client
 *                window.  This routine processes the basic
 *                messages all client windows should process
 *                and passes all others onto UserWndProc where
 *                the developer can process any others.
 *
 *  Concepts:     Called for each message placed in the main
 *                window's message queue
 *
 *                A switch statement branches to the routines to be
 *                performed for each message processed.  Any messages
 *                not specifically process are passed to the user's
 *                message processing procedure UserWndProc().
 *
 *  API's      :  WinPostMsg
 *
 *  Parameters :  hwnd = window handle
 *                msg  = message i.d.
 *                mp1  = first message parameter
 *                mp2  = second message parameter
 *
 *  Return     :  value is determined by each message
 *
 *************************************************************************/

static bool LIBENTRY abortifnodesrunning(void) {
  if (areanynodesrunning()) {
    if (MBID_OK != MessageBox(HWND_DESKTOP,
                              IDMSG_NODESWILLCLOSE,
                              IDMSG_CONFIRMEXIT,
                              MB_OKCANCEL|MB_WARNING|MB_MOVEABLE,FALSE))
      return(TRUE);
  }
  return(FALSE);
}


typedef struct {
  PFNWP  pfnwp;
  RECTL  rc;
  bool   Initialized;
} HELPWINSUB;

HELPWINSUB hws;


MRESULT EXPENTRY HelpFrameSubProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  HELPWINSUB   *phws;
  ULONG        ul;
  PSWP         pswp;

  phws = (HELPWINSUB *) WinQueryWindowULong(hwnd,QWL_USER);

  if (! phws->Initialized) {
    // set up a default help size that occupies 75% of the left-right and
    // 75% of the top-bottom space on the desktop.
    WinQueryWindowRect(HWND_DESKTOP,&phws->rc);
    phws->rc.xRight = (phws->rc.xRight * 3) / 4;
    phws->rc.yTop   = (phws->rc.yTop   * 3) / 4;
    phws->Initialized = TRUE;
  }

  switch (msg) {
    case WM_MINMAXFRAME:
    case WM_TRACKFRAME: // To allow move/sizeing.
         phws->rc.xRight = 0;
         break;

    case WM_ADJUSTWINDOWPOS:
         ul = (ULONG) phws->pfnwp(hwnd, msg, mp1, mp2);
         pswp = (PSWP)mp1;
         if (phws->rc.xRight) {
           // set the window size to our "saved" size
           pswp->x = phws->rc.xLeft;
           pswp->y = phws->rc.yBottom;
           pswp->cx = phws->rc.xRight;
           pswp->cy = phws->rc.yTop;
         } else {
           // if they are NOT minimizing or maximizing, then save the new
           // size so that we can use it again later
           if (ul == AWP_MINIMIZED || ul == AWP_MAXIMIZED ||
               pswp->fl == SWP_MINIMIZE || pswp->fl == SWP_MAXIMIZE) {
           } else {
             phws->rc.xLeft   = pswp->x;
             phws->rc.yBottom = pswp->y;
             phws->rc.xRight  = pswp->cx;
             phws->rc.yTop    = pswp->cy;
           }
         }
         return((MRESULT)ul);
  }

  return(phws->pfnwp(hwnd, msg, mp1, mp2));
}


MRESULT EXPENTRY MainWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  PSWP   pswp;

  switch(msg) {
    case HM_NOTIFY:
         if (SHORT1FROMMP(mp1) == OPEN_COVERPAGE) {
           hws.pfnwp = WinSubclassWindow(HWNDFROMMP(mp2),(PFNWP)HelpFrameSubProc);
           WinSetWindowULong(HWNDFROMMP(mp2),QWL_USER,(ULONG)&hws);
           return(0);
         }
         break;

    case WM_CREATE:
         return(InitMainWindow(hwnd, mp1, mp2));

    case WM_TIMER:
    case WM_VSCROLL:
    case WM_BUTTON1DBLCLK:
    case WM_BUTTON2DOWN:
         PostBkThreadMsg(msg,mp1,mp2);
         break;

    case WM_BUTTON1UP:
         Button1Down = FALSE;
         PostBkThreadMsg(msg,mp1,mp2);
         break;

    case WM_BUTTON1DOWN:
         Button1Down = TRUE;
         PostBkThreadMsg(msg,mp1,mp2);
         break;

    case WM_MOUSEMOVE:
         if (Button1Down && SelectedNodeFirst != 0)
           PostBkThreadMsg(msg,mp1,mp2);
         return(UserWndProc(hwnd,msg,mp1,mp2));

    case WM_MINMAXFRAME:
         pswp = (PSWP) mp1;
         if ((pswp->fl & (SWP_DEACTIVATE|SWP_HIDE|SWP_MINIMIZE)) != 0)
           MainIsMinimized = TRUE;
         else {
           MainIsMinimized = FALSE;
           SkipScreenUpdate = FALSE;
         }
         break;

    case WM_PAINT:
         MainPaint(hwnd);
         break;

    case WM_SIZE:
         getfontparams();
         setscrollbar();
         break;

    case WM_CLOSE:
         if (abortifnodesrunning())
           break;
         WinPostMsg(hwndMain,WM_QUIT,0,0);
         break;

    case WM_COMMAND:
         MainCommand(mp1, mp2);
         break;

    case WM_INITMENU:
         InitMenu(mp1, mp2);
         break;

    case HM_QUERY_KEYS_HELP:
         return (MRESULT)PANEL_HELPKEYS;   /* return id of key help panel */

    case TM_THREADINITFAILED:
        /*  The message is received if the background thread
         *  initialization fails.
         *  A message box is displayed and the application is
         *  terminated.
         */
        MessageBox(HWND_DESKTOP,
                   IDMSG_INITFAILED,
                   0,
                   MB_OK|MB_ERROR,
                   TRUE);

        WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
        break;

    case TM_UPDATETITLE:
         UpdateTitleText(hwndMainFrame,(char *) mp1, (int) mp2);
         break;


    /*
     *      Any messages not processed are passed on
     *      to the user's window proc.  It is
     *      responsible for passing any messages it
     *      doesn't handle onto WinDefWindowProc()
     */

    default:
       return(UserWndProc(hwnd,msg,mp1,mp2));
  }

  return (MRESULT)NULL;  /* all window procedures return 0 as a default */
}   /* End of MainWndProc   */

/**************************************************************************
 *
 *  Name       : MessageBox(hwndOwner, nIdMsg, fsStyle, fBeep)
 *
 *  Description:  Displays the message box with the message
 *                given in idMsg retrieved from the message table
 *                and using the style flags in fsStyle.
 *
 *  Concepts:     Called whenever a message box is to be displayed
 *
 *                - Message string is loaded from the process'
 *                    message table
 *                - Alarm beep is sounded if desired
 *                - Message box with the message is displayed
 *                - WinMessageBox return value is returned
 *
 *  API's      :  WinLoadMessage
 *                WinAlarm
 *                WinMessageBox
 *
 *  Parameters :  hwndOwner = window handle of the owner
 *                nIdMsg    = message i.d.
 *                fsStyle   = style flags for the message box
 *                fBeep     = should an alarm be sounded?
 *
 *  Return     :  return value from WinMessageBox
 *
 *************************************************************************/
ULONG LIBENTRY MessageBox(HWND hwndOwner, ULONG idMsg, ULONG idTitle, ULONG fsStyle, BOOL fBeep) {
   CHAR szText[MESSAGELEN];
   CHAR szTitle[MESSAGELEN];

   if (! WinLoadMessage(hab,
                       (HMODULE)NULL,
                       idMsg,
                       MESSAGELEN,
                       (PSZ)szText)) {
     WinAlarm(HWND_DESKTOP, WA_ERROR);
     return MBID_ERROR;
   }

   if (! WinLoadMessage(hab,
                       (HMODULE)NULL,
                       idTitle,
                       MESSAGELEN,
                       (PSZ)szTitle)) {
     WinAlarm(HWND_DESKTOP, WA_ERROR);
     return MBID_ERROR;
   }

   if (fBeep)
     WinAlarm(HWND_DESKTOP, WA_ERROR);

   return(WinMessageBox(HWND_DESKTOP,
                        hwndOwner,
                        (PSZ)szText,
                        (PSZ) (idTitle == 0 ? NULL : szTitle),
                        MSGBOXID,
                        fsStyle));

}   /* End of MessageBox   */

/**************************************************************************
 *
 *  Name       : MainCommand(mp1, mp2)
 *
 *  Description: Calls the appropriate procedures that deal with
 *               the selected menu item.
 *
 *  Concepts:    Routine is called whenever a WM_COMMAND message
 *               is posted to the main window.
 *
 *               A switch statement branches on the id of the
 *               menu item that posted the message and the
 *               appropriate action for that item is taken.  Any
 *               menu ids that are not part of the standard menu
 *               set are passed onto the user defined WM_COMMAND
 *               processing procedure.
 *
 *  API's      : [none]
 *
 *  Parameters : mp1 = first message parameter
 *               mp2 = second message parameter
 *
 *  Return     : [none]
 *
 *************************************************************************/
static VOID LIBENTRY MainCommand(MPARAM mp1, MPARAM mp2) {
  switch(SHORT1FROMMP(mp1)) {
    case IDM_FILEOPEN:
         FileOpen(mp2);
         break;

    case IDM_FILEEXIT:
         if (abortifnodesrunning())
           break;
         WinPostMsg(hwndMain,WM_QUIT,0,0);
         break;

    case IDM_HELPINDEX:
         HelpIndex();
         break;

    case IDM_HELPGENERAL:
         HelpGeneral();
         break;

    case IDM_HELPUSINGHELP:
         HelpUsingHelp();
         break;

    case IDM_HELPKEYS:
         HelpKeys();
         break;

    case IDM_HELPTUTORIAL:
         HelpTutorial();
         break;

    case IDM_HELPPRODUCTINFO:
         HelpProductInfo();
         break;

    default:
         UserCommand(mp1, mp2);
         break;
  }
}
/***************************  End of main.c  ****************************/
