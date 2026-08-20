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

#define INCL_PM
#define INCL_WINTIMER
#define INCL_WINBUTTONS
#define INCL_WINDIALOGS
#define INCL_WINWINDOWMGR
#define INCL_WINMENUS
#define INCL_WINSTDFONT
#define INCL_WINSTDSPIN
#define INCL_GPILCIDS
#define INCL_GPIPRIMITIVES
#define INCL_WINMLE

#include <os2.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <misc.h>
#include "main.h"
#include "dlg.h"
#include "help.h"
#include "xtrn.h"

/*
 *  Global variables
 */

HWND hwndEditDlg;


/*
 *  Entry point declarations
 */

VOID LIBENTRY EditRecord(VOID);

static VOID LIBENTRY SetWarnings(VOID);
static VOID LIBENTRY SetStartup(VOID);
static VOID LIBENTRY SetNodeLimit(VOID);
static VOID LIBENTRY SetMouse(VOID);
static VOID LIBENTRY SetInterval(VOID);
static VOID LIBENTRY SetFont(VOID);
static VOID LIBENTRY SetDisplay(VOID);
static VOID LIBENTRY ConvertVectorFontSize(FIXED fxPointSize, PFATTRS pfattrs);



/**************************************************************************
 *
 *  Name       : UserWndProc(hwnd, msg, mp1, mp2)
 *
 *  Description: Process any messages sent to hwndMain that
 *               are not processed by the standard window procedure.
 *
 *  Concepts:    Routine is called for each message MainWndProc
 *               does not process.
 *
 *               A switch statement branches control based upon
 *               the message passed.  Any messages not processed
 *               here must be passed onto WinDefWindowProc().
 *
 *  API's      : WinDefWindowProc
 *
 *  Parameters :  hwnd = window handle
 *                msg  = message i.d.
 *                mp1  = first message parameter
 *                mp2  = second message parameter
 *
 *  Return     :  Return value dependent upon the message processed
 *
 *************************************************************************/
MRESULT UserWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
/*
  switch(msg) {
  }
*/
  // default must call WinDefWindowProc()
  return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}


/**************************************************************************
 *
 *  Name       : UserCommand(mp1, mp2)
 *
 *  Description: Process any WM_COMMAND messages sent to hwndMain
 *               that are not processed by MainCommand.
 *
 *  Concepts:    Routine is called for each WM_COMMAND that is
 *               not posted by a standard menu item.
 *
 *               A switch statement branches control based upon
 *               the id of the control which posted the message.
 *
 *  API's      :  [none]
 *
 *  Parameters :  mp1  = first message parameter
 *                mp2  = second message parameter
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY UserCommand(MPARAM mp1, MPARAM mp2) {
  switch(SHORT1FROMMP(mp1)) {
    case IDM_FILELIMIT:
         SetNodeLimit();
         break;

    case IDM_FILENODE:
         PostBkThreadMsg(WM_COMMAND,mp1,mp2);
         break;

    case IDM_UPDATENOW:
         PostBkThreadMsg(WM_PAINT,0,0);
         break;

    case IDM_EDITRECORD:
         if (SelectedNode == 0 || (SelectedNodeFirst != SelectedNodeLast))
           DosBeep(1000,250);
         else {
           EditRecord();
           deselectnodes();
         }
         break;

    case IDM_UPDATEINTERVAL:
         SetInterval();
         break;

    case IDM_ACTSCRNDROPDOSNOW:
         selectandconfirmchange(hwndMain,FALSE,'X');
         break;

    case IDM_ACTSCRNDROPDOS:
         selectandconfirmchange(hwndMain,FALSE,'W');
         break;

    case IDM_ACTSCRNLOGOFF:
         selectandconfirmchange(hwndMain,FALSE,'L');
         break;

    case IDM_ACTSCRNRECYCLE:
         selectandconfirmchange(hwndMain,FALSE,'R');
         break;

    case IDM_ACTSCRNCLEAR:
         selectandconfirmchange(hwndMain,FALSE,0);
         break;

    case IDM_ACTALLDROPDOSNOW:
         selectandconfirmchange(hwndMain,TRUE,'X');
         break;

    case IDM_ACTALLDROPDOS:
         selectandconfirmchange(hwndMain,TRUE,'W');
         break;

    case IDM_ACTALLLOGOFF:
         selectandconfirmchange(hwndMain,TRUE,'L');
         break;

    case IDM_ACTALLRECYCLE:
         selectandconfirmchange(hwndMain,TRUE,'R');
         break;

    case IDM_ACTALLCLEAR:
         selectandconfirmchange(hwndMain,TRUE,0);
         break;

    case IDM_ACTRUNNODES:
         runorswitchtoselectednodes();
         break;

    case IDM_ACTRESTARTNODES:
         restartnodes();
         break;

    case IDM_ACTHIDENODES:
         hidenodes();
         break;

    case IDM_ACTSHOWNODES:
         shownodes();
         break;

    case IDM_ACTARRANGENODES:
         arrangenodes();
         break;

    case IDM_OPTIONSFONT:
         SetFont();
         break;

    case IDM_OPTIONSDISPLAY:
         SetDisplay();
         break;

    case IDM_OPTIONSWARNINGS:
         SetWarnings();
         break;

    case IDM_OPTIONSMOUSE:
         SetMouse();
         break;

    case IDM_OPTIONSSTARTUP:
         SetStartup();
         break;

    case IDM_OPTIONSSAVE:
         writeprofile();
         break;

    default:
         break;
  }

#pragma warn -par
}
#pragma warn +par


/**************************************************************************

 *
 *  Name       : InitMenu()
 *
 *  Description: Processes the WM_INITMENU message for the main window,
 *               disabling any menus that are not active.
 *
 *  Concepts:    Routine is called each time a menu is dropped.
 *
 *               A switch statement branches control based upon
 *               the id of the menu that is being displayed.
 *
 *  API's      :  [none]
 *
 *  Parameters :  mp1  = first message parameter
 *                mp2  = second message parameter
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY InitMenu(MPARAM mp1, MPARAM mp2) {
  HWND hwndMenu;

  switch(SHORT1FROMMP(mp1)) {
    case IDM_HELP:
         hwndMenu = HWNDFROMMP(mp2);
         // Enable or disable the Help menu depending upon whether the
         // help manager has been enabled
         EnableMenuItem(hwndMenu, IDM_HELPUSINGHELP, fHelpEnabled);
         EnableMenuItem(hwndMenu, IDM_HELPGENERAL, fHelpEnabled);
         EnableMenuItem(hwndMenu, IDM_HELPKEYS, fHelpEnabled);
         EnableMenuItem(hwndMenu, IDM_HELPINDEX, fHelpEnabled);

         // REMEMBER: add a case for IDM_HELPTUTORIAL if you include
         // the Tutorial menu item.
         break;

    default:
         break;
  }
}


/**************************************************************************
 *
 *  Name       : EnableMenuItem(hwndMenu, idItem, fEnable)
 *
 *  Description: Enables or disables the menu item
 *
 *  Concepts:    Called whenever a menu item is to be enabled or
 *               disabled
 *
 *               Sends a MM_SETITEMATTR to the menu with the
 *               given item id.  Sets the MIA_DISABLED attribute
 *               flag if the item is to be disabled, clears the flag
 *               if enabling.
 *
 *  API's      : WinSendMsg
 *
 *  Parameters :  hwndmenu = menu window handle
 *                idItem   = menu item i.d.
 *                fEnable  = enable (yes) or disable (no)
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY EnableMenuItem(HWND hwndMenu, USHORT idItem, BOOL fEnable) {
   SHORT fsFlag;

   if(fEnable)
      fsFlag = 0;
   else
      fsFlag = MIA_DISABLED;

   WinSendMsg(hwndMenu,
              MM_SETITEMATTR,
              MPFROM2SHORT(idItem, TRUE),
              MPFROM2SHORT(MIA_DISABLED, fsFlag));

}   /* End of EnableMenuItem() */


/*********************************************************************/

void LIBENTRY centerwindow(HWND hwnd) {
  RECTL  rclWindow;
  LONG   sWidth;
  LONG   sHeight;
  LONG   sBLCx;
  LONG   sBLCy;

  WinQueryWindowRect(hwnd,&rclWindow);
  sWidth  =  (LONG) (rclWindow.xRight - rclWindow.xLeft);
  sHeight =  (LONG) (rclWindow.yTop - rclWindow.yBottom);
  sBLCx   = ((LONG) rclDesktop.xRight - sWidth) / 2;
  sBLCy   = ((LONG) rclDesktop.yTop - sHeight) / 2;
  WinSetWindowPos(hwnd,HWND_TOP,sBLCx,sBLCy,0,0,SWP_MOVE);
}

/**************************************************************************/

void LIBENTRY setspinbuttonvalue(HWND hwnd, int ID, int Value) {
  WinSendDlgItemMsg(hwnd,ID,SPBM_SETCURRENTVALUE,MPFROMLONG(Value),MPVOID);
}

void LIBENTRY setspinbutton(HWND hwnd, int ID, int High, int Low, int Current) {
  WinSendDlgItemMsg(hwnd,ID,SPBM_SETLIMITS,MPFROMLONG(High),MPFROMLONG(Low));
  setspinbuttonvalue(hwnd,ID,Current);
}

ULONG LIBENTRY getspinbuttonvalue(HWND hwnd, int ID) {
  ULONG Value;

   WinSendDlgItemMsg(hwnd,ID,SPBM_QUERYVALUE,&Value,MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
   return(Value);
}

void LIBENTRY relatespinbutton(HWND hwnd, MPARAM mp1, int LowID, int HighID) {
  ULONG Low;
  ULONG High;

  // we only want to perform this processing if the control word indicates it
  // was a spin button change.

  if (HIUSHORT(mp1) == SPBN_CHANGE) {
    Low  = getspinbuttonvalue(hwnd,LowID);
    High = getspinbuttonvalue(hwnd,HighID);

    if (LOUSHORT(mp1) == LowID) {
      // they changed the LOW button value, did this cause the LOW button
      // value to become greater than the HIGH button value?
      if (Low > High) {
        // yes, increase the HIGH button value to compensate
        High = Low;
        setspinbuttonvalue(hwnd,HighID,High);
      }
    } else if (LOUSHORT(mp1) == HighID) {
      // they changed the HIGH button value, did this cause the HIGH button
      // value to become less than the LOW button value?
      if (High < Low) {
        // yes, decrease the LOW button value to compensate
        Low = High;
        setspinbuttonvalue(hwnd,LowID,Low);
      }
    }
  }
}

/**************************************************************************/

static MRESULT EXPENTRY DisplayDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  USHORT usButtonID;
  static bool FixedFont;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);

         WinSendDlgItemMsg(hwnd,ID_FONTSCALE+Settings.FixedFont,BM_SETCHECK,
             MPFROMLONG(TRUE),MPVOID);

         setspinbutton(hwnd,ID_NUMLINES,Settings.LastNode - Settings.FirstNode + 1,1,Settings.NumLines);
         setspinbutton(hwnd,ID_FIRSTSHOW,Settings.LastNode,Settings.FirstNode,Settings.FirstShow);

         FixedFont = Settings.FixedFont;
         return(0);

   case WM_HELP:
        DisplayHelpPanel(PANEL_OPTIONDISPLAY);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                Settings.FirstShow = getspinbuttonvalue(hwnd,ID_FIRSTSHOW);
                Settings.NumLines  = getspinbuttonvalue(hwnd,ID_NUMLINES);
                getfontparams();
                WinPostMsg(hwndMain,WM_PAINT,0,0);
                break;
           case DID_CANCEL:
                if (Settings.FixedFont != FixedFont) {
                  Settings.FixedFont = FixedFont;
                  getfontparams();
                  WinPostMsg(hwndMain,WM_PAINT,0,0);
                }
                break;
         }
         break;

    case WM_CONTROL:
         switch(SHORT2FROMMP(mp1)) {
           case BN_CLICKED:
                usButtonID = SHORT1FROMMP(mp1);
                Settings.FixedFont = (bool) (usButtonID - ID_FONTSCALE);
                getfontparams();
                WinPostMsg(hwndMain,WM_PAINT,0,0);
                return(0);
         }

         /* fall through to the default control processing */
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static VOID LIBENTRY SetDisplay(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       DisplayDlgProc,
                       0,
                       IDD_FONTSETTINGS,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);
}

/**************************************************************************/

static MRESULT EXPENTRY MouseDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  USHORT usButtonID;
  static bool ClickToRun;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);

         WinSendDlgItemMsg(hwnd,ID_MOUSEEDIT+Settings.ClickToRun,BM_SETCHECK,
             MPFROMLONG(TRUE),MPVOID);
         ClickToRun = Settings.ClickToRun;
         return(0);

   case WM_HELP:
        DisplayHelpPanel(PANEL_OPTIONMOUSE);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                break;
           case DID_CANCEL:
                if (Settings.ClickToRun != ClickToRun) {
                  Settings.ClickToRun = ClickToRun;
                }
                break;
         }
         break;

    case WM_CONTROL:
         switch(SHORT2FROMMP(mp1)) {
           case BN_CLICKED:
                usButtonID = SHORT1FROMMP(mp1);
                Settings.ClickToRun = (bool) (usButtonID - ID_MOUSEEDIT);
                return(0);
         }
         break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static VOID LIBENTRY SetMouse(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       MouseDlgProc,
                       0,
                       IDD_MOUSE,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);
}

/**************************************************************************/

static MRESULT EXPENTRY StartupDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  USHORT usButtonID;
  static bool DisableStartRun;
  static bool DisableConnectRun;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);

         WinSendDlgItemMsg(hwnd,ID_ENABLESTARTRUN+Settings.DisableStartRun,
             BM_SETCHECK,MPFROMLONG(TRUE),MPVOID);
         DisableStartRun = Settings.DisableStartRun;
         WinSendDlgItemMsg(hwnd,ID_ENABLECONNECTRUN+Settings.DisableConnectRun,
             BM_SETCHECK,MPFROMLONG(TRUE),MPVOID);
         DisableConnectRun = Settings.DisableConnectRun;
         return(0);

   case WM_HELP:
        DisplayHelpPanel(PANEL_OPTIONSTARTUP);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                break;
           case DID_CANCEL:
                if (Settings.DisableStartRun != DisableStartRun) {
                  Settings.DisableStartRun = DisableStartRun;
                }
                if (Settings.DisableConnectRun != DisableConnectRun) {
                  Settings.DisableConnectRun = DisableConnectRun;
                }
                break;
         }
         break;

    case WM_CONTROL:
         switch(SHORT2FROMMP(mp1)) {
           case BN_CLICKED:
                usButtonID = SHORT1FROMMP(mp1);
                switch(usButtonID) {
                  case ID_ENABLESTARTRUN:    Settings.DisableStartRun   = FALSE; break;
                  case ID_DISABLESTARTRUN:   Settings.DisableStartRun   = TRUE;  break;
                  case ID_ENABLECONNECTRUN:  Settings.DisableConnectRun = FALSE; break;
                  case ID_DISABLECONNECTRUN: Settings.DisableConnectRun = TRUE;  break;
                }
                return(0);
         }
         break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static VOID LIBENTRY SetStartup(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       StartupDlgProc,
                       0,
                       IDD_STARTUP,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);
}

/*********************************************************************/

static MRESULT EXPENTRY WarnDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  static bool ShowOld;
  static bool WarnOld;
  static bool AllOld;
  static bool RestartOld;
  static bool VerifyOld;
  static bool PageAlarmOld;
  static bool PageForeOld;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);

         ShowOld      = Settings.ShowOld;
         WarnOld      = Settings.WarnOld;
         AllOld       = Settings.MonitorAll;
         RestartOld   = Settings.RestartInactive;
         VerifyOld    = Settings.VerifyChanges;
         PageAlarmOld = Settings.PageAlarmEnabled;
         PageForeOld  = Settings.PageBringToFore;
         WinSendDlgItemMsg(hwnd,ID_WARNHIGHLIGHT,BM_SETCHECK,
             MPFROMLONG(ShowOld),MPVOID);
         WinSendDlgItemMsg(hwnd,ID_WARNALARM,BM_SETCHECK,
             MPFROMLONG(WarnOld),MPVOID);
         WinSendDlgItemMsg(hwnd,ID_WARNALL,BM_SETCHECK,
             MPFROMLONG(AllOld),MPVOID);
         WinSendDlgItemMsg(hwnd,ID_WARNRESTART,BM_SETCHECK,
             MPFROMLONG(RestartOld),MPVOID);
         WinSendDlgItemMsg(hwnd,ID_VERIFYCHANGE,BM_SETCHECK,
             MPFROMLONG(VerifyOld),MPVOID);
         WinSendDlgItemMsg(hwnd,ID_PAGEALARM,BM_SETCHECK,
             MPFROMLONG(PageAlarmOld),MPVOID);
         WinSendDlgItemMsg(hwnd,ID_PAGEFOREGROUND,BM_SETCHECK,
             MPFROMLONG(PageForeOld),MPVOID);

         setspinbutton(hwnd,ID_WARNMINUTES   ,1440,1,Settings.WarnSeconds/60);
         setspinbutton(hwnd,ID_WARNEVENTS    ,1440,1,Settings.WarnEventSeconds/60);
         setspinbutton(hwnd,ID_WARNDOORS     ,1440,1,Settings.WarnDoorSeconds/60);
         setspinbutton(hwnd,ID_WARNXFERS     ,1440,1,Settings.WarnXferSeconds/60);
         setspinbutton(hwnd,ID_WARNRESTARTMIN,1440,1,Settings.WarnRestartSeconds/60);
         return(0);

   case WM_HELP:
        DisplayHelpPanel(PANEL_OPTIONWARNING);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                Settings.ShowOld            = ShowOld;
                Settings.WarnOld            = WarnOld;
                Settings.MonitorAll         = AllOld;
                Settings.RestartInactive    = RestartOld;
                Settings.VerifyChanges      = VerifyOld;
                Settings.PageAlarmEnabled   = PageAlarmOld;
                Settings.PageBringToFore    = PageForeOld;
                Settings.WarnSeconds        = getspinbuttonvalue(hwnd,ID_WARNMINUTES) * 60;
                Settings.WarnEventSeconds   = getspinbuttonvalue(hwnd,ID_WARNEVENTS) * 60;
                Settings.WarnDoorSeconds    = getspinbuttonvalue(hwnd,ID_WARNDOORS) * 60;
                Settings.WarnXferSeconds    = getspinbuttonvalue(hwnd,ID_WARNXFERS) * 60;
                Settings.WarnRestartSeconds = getspinbuttonvalue(hwnd,ID_WARNRESTARTMIN) * 60;
                WinPostMsg(hwndMain,WM_PAINT,0,0);
                break;
         }
         break;

    case WM_CONTROL:
         switch(SHORT2FROMMP(mp1)) {
           case BN_CLICKED:
                switch (SHORT1FROMMP(mp1)) {
                  case ID_WARNHIGHLIGHT:
                       ShowOld = (bool) (! ShowOld);
                       WinSendDlgItemMsg(hwnd,ID_WARNHIGHLIGHT,
                           BM_SETCHECK,MPFROMLONG(ShowOld),MPVOID);
                       break;
                  case ID_WARNALARM:
                       WarnOld = (bool) (! WarnOld);
                       WinSendDlgItemMsg(hwnd,ID_WARNALARM,
                           BM_SETCHECK,MPFROMLONG(WarnOld),MPVOID);
                       break;
                  case ID_WARNALL:
                       AllOld = (bool) (! AllOld);
                       WinSendDlgItemMsg(hwnd,ID_WARNALL,
                           BM_SETCHECK,MPFROMLONG(AllOld),MPVOID);
                       break;
                  case ID_WARNRESTART:
                       RestartOld = (bool) (! RestartOld);
                       WinSendDlgItemMsg(hwnd,ID_WARNRESTART,
                           BM_SETCHECK,MPFROMLONG(RestartOld),MPVOID);
                       break;
                  case ID_VERIFYCHANGE:
                       VerifyOld = (bool) (! VerifyOld);
                       WinSendDlgItemMsg(hwnd,ID_VERIFYCHANGE,
                           BM_SETCHECK,MPFROMLONG(VerifyOld),MPVOID);
                       break;
                  case ID_PAGEALARM:
                       PageAlarmOld = (bool) (! PageAlarmOld);
                       WinSendDlgItemMsg(hwnd,ID_PAGEALARM,
                           BM_SETCHECK,MPFROMLONG(PageAlarmOld),MPVOID);
                       break;
                  case ID_PAGEFOREGROUND:
                       PageForeOld = (bool) (! PageForeOld);
                       WinSendDlgItemMsg(hwnd,ID_PAGEFOREGROUND,
                           BM_SETCHECK,MPFROMLONG(PageForeOld),MPVOID);
                       break;
                }
                return(0);
         }
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}


static VOID LIBENTRY SetWarnings(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       WarnDlgProc,
                       0,
                       IDD_WARNINGS,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);
}

/*********************************************************************/

static MRESULT EXPENTRY UpdateDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  ULONG  ulValue;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);
         setspinbutton(hwnd,ID_INTERVAL,1000,1,Settings.UpdateInterval);
         return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                ulValue = getspinbuttonvalue(hwnd,ID_INTERVAL);
                if (Settings.UpdateInterval != (int) ulValue) {
                  WinStopTimer(hab,hwndMain,0);
                  Settings.UpdateInterval = (int) ulValue;
                  WinStartTimer(hab,hwndMain,0,Settings.UpdateInterval * 1000);
                }
                break;
         }
         break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}

static VOID LIBENTRY SetInterval(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       UpdateDlgProc,
                       0,
                       IDD_UPDATE,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);
}

/*********************************************************************/

static MRESULT EXPENTRY NodeLimitDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  ULONG  ulFirstNode;
  ULONG  ulLastNode;

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);
         setspinbutton(hwnd,ID_FIRSTNODE,MaxNodes,1,Settings.FirstNode);
         setspinbutton(hwnd,ID_LASTNODE ,MaxNodes,1,Settings.LastNode);
         return(0);

    case WM_CONTROL:
         relatespinbutton(hwnd,mp1,ID_FIRSTNODE,ID_LASTNODE);
         break;

   case WM_HELP:
        DisplayHelpPanel(HID_NODELIMIT_DLG_HELP_PANEL);
        return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {
           case DID_OK:
                ulFirstNode = getspinbuttonvalue(hwnd,ID_FIRSTNODE);
                ulLastNode  = getspinbuttonvalue(hwnd,ID_LASTNODE);
                if (ulFirstNode > ulLastNode) {
                  MessageBox(HWND_DESKTOP,IDMSG_FIRSTNODELESS,IDMSG_LIMITSERROR,MB_ENTER|MB_ERROR|MB_MOVEABLE,FALSE);
                  return(0);
                }
                if ((Settings.FirstNode != ulFirstNode) || (Settings.LastNode != ulLastNode)) {
                  Settings.FirstNode = (int) ulFirstNode;
                  Settings.LastNode = (int) ulLastNode;
                  WinPostMsg(hwndMain,WM_PAINT,0,0);
                }
                break;
         }
         break;
  }
  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}

static VOID LIBENTRY SetNodeLimit(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       NodeLimitDlgProc,
                       0,
                       IDD_NODELIMIT,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);
}

/*********************************************************************/

static void LIBENTRY updatetime(HWND hwnd, long DateTime) {
  bool            PM;
  unsigned int    Hour;
  unsigned int    Min;
  unsigned int    Sec;
  unsigned int    Time;
  unsigned int    Today;
  updttype        LastUpdate;
  long            ElapsedTime;
  char            Str[40];

  PM = FALSE;
  LastUpdate.DateTime = DateTime;
  if (LastUpdate.Split.Date <= 1)
    Time = LastUpdate.DateTime;
  else
    Time  = (unsigned int) LastUpdate.Split.Time * 2;

  Hour  = Time / 3600;
  Time -= (Hour * 3600L);
  if (Hour >= 12) {
    PM = TRUE;
    if (Hour > 12)
      Hour -= 12;
  }
  Min = Time / 60;
  Sec = Time - (Min * 60);
  sprintf(Str,"%02d:%02d:%02d %s",Hour,Min,Sec,(PM ? "pm" : "am"));
  WinSetDlgItemText(hwnd,ID_UPDTTIME,(PSZ)Str);

  if (LastUpdate.Split.Date <= 2) {
    Time = LastUpdate.DateTime;
    ElapsedTime = exacttime() - Time;
    if (ElapsedTime < 0)
      ElapsedTime += (24 * 60 * 60);
  } else {
    Time  = (unsigned int) LastUpdate.Split.Time * 2;
    Today = getjuliandate();
    if (LastUpdate.Split.Date == Today)
      ElapsedTime = exacttime() - Time;
    else if (Today > LastUpdate.Split.Date) {
      ElapsedTime = exacttime() - Time + ((Today - LastUpdate.Split.Date) * (24 * 60 * 60));
    } else
      ElapsedTime = 0;
  }

  Hour = ElapsedTime / 3600;
  ElapsedTime -= (Hour * 3600L);
  Min = ElapsedTime / 60;
  Sec = ElapsedTime - (Min * 60);

  if (Hour != 0)
    sprintf(Str,"%2dh %2dm %2ds",Hour,Min,Sec);
  else if (Min != 0)
    sprintf(Str,"%2d min %2d sec",Min,Sec);
  else
    sprintf(Str,"%d seconds",Sec);

  WinSetDlgItemText(hwnd,ID_UPDTMIN,(PSZ)Str);
}

static void LIBENTRY settext(HWND hwnd, int ID, int Limit, char *Text) {
  WinSendDlgItemMsg(hwnd,ID,EM_SETTEXTLIMIT,(MPARAM)Limit,0);
  WinSetDlgItemText(hwnd,ID,(PSZ)Text);
}

static MRESULT EXPENTRY EditDlgProc(HWND hwnd, ULONG ulMsg, MPARAM mp1, MPARAM mp2) {
  static bool     ChangedUserName;
  static bool     ChangedUserCity;
  static bool     ChangedOperation;
  static bool     ChangedMessage;
  static int      SaveStatus;
  static nodetype Save;
  ULONG           NewStatus;
  int             X;
  nodetype       *p;
  char            Tmp[80];
  char            Str[80];

  switch(ulMsg) {
    case WM_INITDLG:
         centerwindow(hwnd);

         loadstring(IDS_EDITNODESTATUS,Tmp,sizeof(Tmp));
         sprintf(Str,Tmp,SelectedNode,SessionIds[SelectedNode-1]);
         if (SessionIds[SelectedNode-1] == 0) {
           // if there isn't a PID, then remove PID from the string
           char *dash = strrchr(Str,'-');
           if (dash != NULL)
             *dash = 0;
         }
         WinSetWindowText(WinWindowFromID(hwnd,FID_TITLEBAR),Str);

         Save = NodeArray[SelectedNode-1];
         settext(hwnd,ID_USERNAME,25,Save.Name);
         settext(hwnd,ID_USERCITY,24,Save.City);
         settext(hwnd,ID_OPERATION,48,Save.Operation);
         settext(hwnd,ID_MESSAGE,79,Save.Message);

         updatetime(hwnd,Save.LastUpdate.DateTime);

         for (X = 0; X < NUMOPTIONS; X++) {
           WinSendDlgItemMsg(hwnd,ID_STATUS,LM_INSERTITEM,
               MPFROMLONG(X),MPFROMP(Options[X]));
         }

         SaveStatus = getstatusnum(Save.Status);
         WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
              MPFROMLONG(SaveStatus),MPFROMLONG(TRUE));

         ChangedUserName  =
         ChangedUserCity  =
         ChangedOperation =
         ChangedMessage   = FALSE;

         hwndEditDlg = hwnd;
         return(0);

    case WM_CONTROL:
         if ((USHORT) SHORT2FROMMP(mp1) == EN_CHANGE) {
           switch(SHORT1FROMMP(mp1)) {
             case ID_USERNAME  : ChangedUserName  = TRUE;  break;
             case ID_USERCITY  : ChangedUserCity  = TRUE;  break;
             case ID_OPERATION : ChangedOperation = TRUE;  break;
             case ID_MESSAGE   : ChangedMessage   = TRUE;
                                 if (WinQueryWindowTextLength(WinWindowFromID(hwnd,ID_MESSAGE)) == 0) {
                                   p = &NodeArray[SelectedNode-1];
                                   WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                                        MPFROMSHORT(getstatusnum(Save.Status)),MPFROMLONG(TRUE));
                                 } else {
                                   WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                                        MPFROMSHORT(getstatusnum('M')),MPFROMLONG(TRUE));
                                 }
                                 break;
           }
         }
         return(0);

   case WM_HELP:
        DisplayHelpPanel(PANEL_EDITNODE);
        return(0);

    case WM_TIMER:
         p = &NodeArray[SelectedNode-1];
         updatetime(hwnd,p->LastUpdate.DateTime);

         if (memcmp(p,&Save,sizeof(nodetype)) == 0)
           return(0);

         if (! ChangedUserName) {
           if (memcmp(p->Name,Save.Name,sizeof(Save.Name)) != 0)
             WinSetDlgItemText(hwnd,ID_USERNAME,(PSZ)p->Name);
         }
         if (! ChangedUserCity) {
           if (memcmp(p->City,Save.City,sizeof(Save.City)) != 0)
             WinSetDlgItemText(hwnd,ID_USERCITY,(PSZ)p->City);
         }
         if (! ChangedOperation) {
           if (memcmp(p->Operation,Save.Operation,sizeof(Save.Operation)) != 0)
             WinSetDlgItemText(hwnd,ID_OPERATION,(PSZ)p->Operation);
         }
         if (! ChangedMessage) {
           if (memcmp(p->Message,Save.Message,sizeof(Save.Message)) != 0)
             WinSetDlgItemText(hwnd,ID_MESSAGE,(PSZ)p->Message);
         }

         NewStatus = SHORT1FROMMR(WinSendDlgItemMsg(hwnd,ID_STATUS,LM_QUERYSELECTION,0,0));
         if (NewStatus == SaveStatus) {
           SaveStatus = getstatusnum(Save.Status);
           WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                MPFROMLONG(SaveStatus),MPFROMLONG(TRUE));
         }


         return(0);

    case WM_COMMAND:
         switch(LOUSHORT(mp1)) {

           case DID_LOGOFF:
                WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                    MPFROMSHORT(getstatusnum('L')),MPFROMLONG(TRUE));
                return(0);

           case DID_DROPDOS:
                WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                    MPFROMSHORT(getstatusnum('W')),MPFROMLONG(TRUE));
                return(0);

           case DID_DROPDOSNOW:
                WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                    MPFROMSHORT(getstatusnum('X')),MPFROMLONG(TRUE));
                return(0);

           case DID_RECYCLE:
                WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                    MPFROMSHORT(getstatusnum('R')),MPFROMLONG(TRUE));
                return(0);

           case DID_CLEARNODE:
                WinSetDlgItemText(hwnd,ID_USERNAME, (PSZ)"");
                WinSetDlgItemText(hwnd,ID_USERCITY, (PSZ)"");
                WinSetDlgItemText(hwnd,ID_OPERATION,(PSZ)"");
                WinSetDlgItemText(hwnd,ID_MESSAGE,  (PSZ)"");
                WinSendDlgItemMsg(hwnd,ID_STATUS,LM_SELECTITEM,
                    MPFROMSHORT(0),MPFROMLONG(TRUE));
                return(0);

           case DID_OK:
                NewStatus = SHORT1FROMMR(WinSendDlgItemMsg(hwnd,ID_STATUS,LM_QUERYSELECTION,0,0));

                if (ChangedUserName || ChangedUserCity || ChangedOperation ||
                    ChangedMessage || (SaveStatus != NewStatus)) {

                  p = &NodeArray[SelectedNode-1];

                  if (SaveStatus != NewStatus)
                    p->Status = getstatusletter(NewStatus);

                  if (ChangedUserName) {
                    WinQueryWindowText(WinWindowFromID(hwnd,ID_USERNAME),
                                       sizeof(p->Name),p->Name);
                  }
                  if (ChangedUserCity) {
                    WinQueryWindowText(WinWindowFromID(hwnd,ID_USERCITY),
                                       sizeof(p->City),p->City);
                  }
                  if (ChangedOperation) {
                    WinQueryWindowText(WinWindowFromID(hwnd,ID_OPERATION),
                                       sizeof(p->Operation),p->Operation);
                  }
                  if (ChangedMessage) {
                    WinQueryWindowText(WinWindowFromID(hwnd,ID_MESSAGE),
                                       sizeof(p->Message),p->Message);
                  }

                  writeselectednode(p);
                }
                break;
         }
         break;
  }

  return(WinDefDlgProc(hwnd,ulMsg,mp1,mp2));
}

VOID LIBENTRY EditRecord(VOID) {
  HWND  hwndDlg;

  hwndDlg = WinLoadDlg(HWND_DESKTOP,
                       hwndMain,
                       EditDlgProc,
                       0,
                       IDD_EDITRECORD,
                       NULL);
  WinProcessDlg(hwndDlg);
  hwndEditDlg = 0;
  WinDestroyWindow(hwndDlg);
}

/*********************************************************************/

static MRESULT EXPENTRY SetFontProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2) {
  if (message == WM_HELP) {
    DisplayHelpPanel(HID_FONT_DLG_HELP_PANEL);
    return (MRESULT)FALSE ;
  }

  return WinDefFontDlgProc(hwnd, message, mp1, mp2);
}


void LIBENTRY getfontparams(void) {
  FONTMETRICS fontMetrics;
  RECTL  rc;

  if (SkipScreenUpdate || MainIsMinimized)
    return;

  WinQueryWindowRect(hwndMain,&rc);
  yTop   = rc.yTop;
  xRight = rc.xRight;

top:
  if (Settings.FixedFont) {
    GpiSetCharMode(hpsMain,CM_MODE1);
    GpiQueryFontMetrics(hpsMain, sizeof(FONTMETRICS), &fontMetrics);
    fontWidth        =   fontMetrics.lAveCharWidth;
    fontHeight       =   fontMetrics.lMaxBaselineExt;
    fontDescend      =   fontMetrics.lMaxDescender;
    fontProportional = ((fontMetrics.fsType & FM_TYPE_FIXED) == 0);
    fontKerning      =  (fontMetrics.fsType & FM_TYPE_KERNING);
    if (fontHeight == 0)
      numWinLines = 1;
    else
      numWinLines = rc.yTop/fontHeight;
  } else {
    const int StrLen = 79;
    char   Str[StrLen+1];
    int    UsableWidth  = rc.xRight - 20;
    int    UsableHeight = rc.yTop;
    POINTL botleft      = {0,0};
    POINTL aptl[StrLen+1];
    SIZEF  sizef;

    // first, let's find out if the current font is scalable
    GpiQueryFontMetrics(hpsMain,sizeof(FONTMETRICS),&fontMetrics);
    if ((fontMetrics.fsDefn & FM_DEFN_OUTLINE) == 0) {
      const int    MaxFonts = 100;
      bool         FoundVector;
      long         FontsFound;
      int          X;
      PFONTMETRICS pFontMetrics;

      // we've got a problem, we're set up for a scalable font, but we don't
      // currently have a outline/vector font loaded!

      // allocate storage for up to MaxFonts fonts
      if ((pFontMetrics = (PFONTMETRICS) malloc(MaxFonts * sizeof(FONTMETRICS))) == NULL) {
        // if the allocation fails, then give up, go back to a fixed font
        Settings.FixedFont = TRUE;
        goto top;
      }

      FontsFound = MaxFonts;
      if (GpiQueryFonts(hpsMain,QF_PUBLIC|QF_NO_DEVICE,NULL,&FontsFound,
          sizeof(FONTMETRICS),pFontMetrics) == GPI_ALTERROR) {
        // if the call fails, then give up, go back to a fixed font
        free(pFontMetrics);
        Settings.FixedFont = TRUE;
        goto top;
      }

      for (X = 0, FoundVector = FALSE; X < FontsFound; X++) {
        if ((pFontMetrics[X].fsDefn & FM_DEFN_OUTLINE) != 0) {
          FATTRS fAttrs;
          memset(&fAttrs,0,sizeof(fAttrs));

          // describe the font by name, and specify that we want the font to
          // be "transformable" (i.e. scalable).
          fAttrs.usRecordLength = sizeof(fAttrs);
          fAttrs.fsFontUse = FATTR_FONTUSE_TRANSFORMABLE;
          strcpy(fAttrs.szFacename,pFontMetrics[X].szFamilyname);

          // try to select the font we just described
          if (GpiCreateLogFont(hpsMain,NULL,1,&fAttrs) == FONT_MATCH) {
            GpiSetCharSet(hpsMain,1);
            FoundVector = TRUE;
            break;
          }
        }
      }

      free(pFontMetrics);

      if (! FoundVector) {
        // we failed to find any vector fonts, so go back to a fixed font
        Settings.FixedFont = TRUE;
        goto top;
      }
    }


    loadstring(IDS_SAMPLETEXT,Str,sizeof(Str));

    GpiSetCharMode(hpsMain,CM_MODE3);

    // We don't have to guess at how tall the font is because the font is
    // only "proportional" along the X-axis.  So we just divide the height of
    // the screen by the number of lines we will be displaying to determine
    // the font height.

    fontHeight = UsableHeight / Settings.NumLines;
    sizef.cy = MAKEFIXED(fontHeight,0);

    // To get the size of the font along the X-axis we first have to make a
    // "guess" as to what the right CharBox size is by dividing the "usable
    // screen width" by the number of characters in the string (i.e.  to spread
    // 40 characters across the entire usable screen we would use the usable
    // screen width divided by 40).

    fontWidth = UsableWidth / StrLen;
    sizef.cx  = MAKEFIXED(fontWidth,0);
    GpiSetCharBox(hpsMain,&sizef);

    // Now we find out how large the string really was by filling the aptl
    // array with coordinates for each character in the string.

    GpiQueryCharStringPosAt(hpsMain,&botleft,0,StrLen,Str,NULL,aptl);

    // avoid a division by zero down below

    if (aptl[StrLen].x == 0)
      aptl[StrLen].x = 1;

    // Now we find out how good our "guess" really was.  We probably did not
    // fill up the usable width, so divide the usable width by the point we
    // reached (i.e. if the string only filled half of the screen and the
    // screen was 100 wide, then we'd be dividing 100 by 50 to determine that
    // we need to expand our CharBox by a factor of 2).  To avoid loss of
    // precision using integers, we'll multiply by 10 before the division.

    int Ratio = (UsableWidth * 10) / aptl[StrLen].x;

    // Now adjust the width by re-using our original equation while throwing
    // in the scaling ratio ... don't forget to divide the 10 back out.

    fontWidth = (UsableWidth * Ratio) / (StrLen * 10);
    sizef.cx  = MAKEFIXED(fontWidth,0);
    GpiSetCharBox(hpsMain,&sizef);

    GpiQueryFontMetrics(hpsMain, sizeof(FONTMETRICS), &fontMetrics);
    fontWidth        =   fontMetrics.lAveCharWidth;
    fontProportional = ((fontMetrics.fsType & FM_TYPE_FIXED) == 0);
    fontKerning      =  (fontMetrics.fsType & FM_TYPE_KERNING);

    // don't allow a divide-by-zero to occur...
    if (fontMetrics.lMaxBaselineExt == 0) {
      Settings.FixedFont = TRUE;
      getfontparams();
    } else {
      sizef.cy = MAKEFIXED((fontHeight * fontHeight) / fontMetrics.lMaxBaselineExt,0);
      GpiSetCharBox(hpsMain,&sizef);
      numWinLines = Settings.NumLines;
    }
  }
}



/*********************************************************************
 *  Name: SetFont
 *
 *  Description : Allows the user to select a font for the text
 *                displayed in the MLE.
 *
 *  Concepts : Routine is called each time the user selects the
 *             Font menu item from the Options menu.  The
 *             standard font dialog is called with the current
 *             available fonts.  If the user selects one, then
 *             the MLM_SETFONT message is sent to the MLE to
 *             display its text to the font chosen.
 *
 *  API's : WinSendMsg
 *          GpiCreateLogFont
 *          GpiSetCharSet
 *          GpiQueryFontMetrics
 *          GpiDeleteSetId
 *          WinGetPS
 *          WinReleasePS
 *          WinFontDlg
 *
 *  Parameters :  None
 *
 *  Returns: None
 *
 ****************************************************************/
static VOID LIBENTRY SetFont(void) {
   FONTDLG Dlg;
   HPS hps;
   FONTMETRICS fm;
   CHAR szTitle[MESSAGELEN];
   CHAR szPreview[MESSAGELEN];
   CHAR szFamily[CCHMAXPATH];
   static int fxPointSize = 0;       // keep track of this for vector fonts

   SkipScreenUpdate = TRUE;

   // initialize all fields to 0
   memset(&Dlg,0,sizeof(Dlg));
   hps = hpsMain;

   // Get the current font attributes
   GpiQueryFontMetrics(hps,sizeof(fm),&fm);

   // Initialize the Dlg structure with the current font
   Dlg.cbSize     = sizeof(Dlg);                  // sizeof(Dlg)
   Dlg.hpsScreen  = hps;                          // Screen presentation space

   if (loadstring(IDS_FONTDLGTITLE,szTitle,MESSAGELEN) == NULL) {
     SkipScreenUpdate = FALSE;
     return;
   }

   if (loadstring(IDS_PREVIEWTEXT,szPreview,MESSAGELEN) == NULL) {
     SkipScreenUpdate = FALSE;
     return;
   }

   // first time thru, default to 10-point (static data defaults to 0, but
   // will remember its value the next time it's used)
   if (fxPointSize == 0)
     fxPointSize = MAKEFIXED(10,0);

   // save current font family name
   strcpy(szFamily,fm.szFamilyname);

   Dlg.pszTitle       = (PSZ)szTitle;           // Application supplied title
   Dlg.pszPreview     = (PSZ)szPreview;         // String to print in preview window
   Dlg.pszFamilyname  = (PSZ)szFamily;          // point to Family name of font
   Dlg.fxPointSize    = fxPointSize;            // Point size the user selected
   Dlg.clrFore        = CLR_BLACK;              // Selected foreground color
   Dlg.clrBack        = CLR_WHITE;              // Selected background color
   Dlg.usFamilyBufLen = sizeof(szFamily);       // Length of family name buffer
   Dlg.pfnDlgProc     = (PFNWP)SetFontProc;     // used for providing help


   // the following lines of code look like they could be useful but don't
   // appear to have ANY affect on the operation whatsoever!
// Dlg.sNominalPointSize = fm.sNominalPointSize;
// Dlg.usWeight = fm.usWeightClass; /* The boldness of the font */
// Dlg.usWidth = fm.usWidthClass;  /* The width of the font     */
// Dlg.usDlgId = IDD_FONT;       /* ID of a custom dialog template  */


   // I tried setting all of the Dlg.fAttrs variables in the structure and
   // the only one that appeared to have any influence on the font dialog
   // was the szFacename.  Without it, the dialog defaulted to BOLD all the
   // time.  With it, it defaults to normal.  But I can't seem to get the
   // dialog to default to bold *if* bold was previously selected!
   Dlg.fAttrs.usRecordLength = sizeof(Dlg.fAttrs);
   strcpy(Dlg.fAttrs.szFacename,szFamily);

   // FNTS_* flags - dialog styles
   Dlg.fl = FNTS_CENTER |
            FNTS_INITFROMFATTRS |
//          FNTS_NOSYNTHESIZEDFONTS |
            FNTS_RESETBUTTON |
            FNTS_HELPBUTTON;

   if (! Settings.FixedFont) {
     // when doing scalable fonts, we can only use vector fonts
     Dlg.fl |= FNTS_VECTORONLY;
   }

   // Bring up the standard Font Dialog
   if (WinFontDlg(HWND_DESKTOP,hwndMain,&Dlg) == NULLHANDLE || Dlg.lReturn != DID_OK) {
     SkipScreenUpdate = FALSE;
     return;
   }

   // save the point size for the next dialog
   fxPointSize = Dlg.fxPointSize;

   /*
    *   If outline font, calculate the maxbaselineext and
    *   avecharwidth for the point size selected
    */

   if (Dlg.fAttrs.fsFontUse == FATTR_FONTUSE_OUTLINE)
     ConvertVectorFontSize(Dlg.fxPointSize,&Dlg.fAttrs);

/*
   // Discard LCID (logical character identifier) 1 if it exists.
   if (GpiSetCharSet(hps,1)) {
     GpiSetCharSet(hps,0);        // select font 0
     GpiDeleteSetId(hps,1);       // delete existing font 1
   }
*/
   // try just deleting it for now
     GpiSetCharSet(hps,0);        // select font 0
     GpiDeleteSetId(hps,1);       // delete existing font 1

   GpiCreateLogFont(hps,NULL,1,&Dlg.fAttrs);
   GpiSetCharSet(hps,1);

   SkipScreenUpdate = FALSE;
   getfontparams();
   WinPostMsg(hwndMain,WM_PAINT,0,0);
}


/*
 *   Convert vector font size using point size and fAttrs structure and
 *   return it in that structure.
 */

static VOID LIBENTRY ConvertVectorFontSize(FIXED fxPointSize, PFATTRS pfattrs) {
  HPS   hps;
  HDC   hDC;
  LONG  lxFontResolution;
  LONG  lyFontResolution;
  SIZEF sizef;

  hps = WinGetScreenPS(HWND_DESKTOP);        /* Screen presentation space */

  /*
   *   Query device context for the screen and then query
   *   the resolution of the device for the device context.
   */

  hDC = GpiQueryDevice(hps);
  DevQueryCaps( hDC, CAPS_HORIZONTAL_FONT_RES, (LONG)1, &lxFontResolution);
  DevQueryCaps( hDC, CAPS_VERTICAL_FONT_RES, (LONG)1, &lyFontResolution);

  /*
   *   Calculate the size of the character box, based on the
   *   point size selected and the resolution of the device.
   *   The size parameters are of type FIXED, NOT int.
   *   NOTE: 1 point == 1/72 of an inch.
   */

  sizef.cx = (FIXED)(((fxPointSize) / 72 ) * lxFontResolution );
  sizef.cy = (FIXED)(((fxPointSize) / 72 ) * lyFontResolution );

  pfattrs->lMaxBaselineExt = MAKELONG( HIUSHORT( sizef.cy ), 0 );
  pfattrs->lAveCharWidth   = MAKELONG( HIUSHORT( sizef.cx ), 0 );
  WinReleasePS(hps);
}
/***************************  End of user.c  ****************************/
