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

#define  INCL_WINHELP

#include <os2.h>
#include <dos.h>
#include <string.h>
#include <misc.h>
#include "main.h"
#include "dlg.h"
#include "help.h"
#include "xtrn.h"

#define HELPLIBRARYNAMELEN  80
#define HELPTITLELEN        40

/* If DEBUG is defined, then the help panels will display their
 *  id values on their title bar.  This is useful for determining
 *  which help panels are being shown for each dialog item.  When
 *  the DEBUG directive is not defined, then the panel ids are not
 *  displayed.
 */

/* #define  DEBUG */

/*
 *  Global variables
 */
static HWND hwndHelpInstance;
static CHAR szWindowTitle[HELPTITLELEN];


/*
 *  Entry point declarations
 */
MRESULT EXPENTRY ProductInfoDlgProc(HWND hwnd,
                                    USHORT msg,
                                    MPARAM mp1,
                                    MPARAM mp2);


/**************************************************************************
 *
 *  Name       : InitHelp()
 *
 *  Description:  Initializes the IPF help facility
 *
 *  Concepts:     Called once during initialization of the program
 *
 *                Initializes the HELPINIT structure and creates the
 *                help instance.  If successful, the help instance
 *                is associated with the main window.
 *
 *  API's      :  WinCreateHelpInstance
 *                WinAssociateHelpInstance
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY InitHelp(VOID) {
   char     *p;
   HELPINIT  hini;
   CHAR      szLibName[HELPLIBRARYNAMELEN];
   CHAR      szTemp[HELPLIBRARYNAMELEN];

   /* if we return because of an error, Help will be disabled */
   fHelpEnabled = FALSE;

   /* initialize help init structure */
   hini.cb = sizeof(HELPINIT);
   hini.ulReturnCode = 0;

   hini.pszTutorialName = (PSZ)NULL;   /* if tutorial added, add name here */

   hini.phtHelpTable = (PHELPTABLE)MAKELONG(TEMPLATE_HELP_TABLE, 0xFFFF);
   hini.hmodHelpTableModule = 0;
   hini.hmodAccelActionBarModule = 0;
   hini.idAccelTable = 0;
   hini.idActionBar = 0;

   if (loadstring(IDS_HELPWINDOWTITLE,szWindowTitle,HELPTITLELEN) == NULL)
     return;

   hini.pszHelpWindowTitle = (PSZ)szWindowTitle;

   /* if debugging, show panel ids, else don't */
   #ifdef DEBUG
     hini.fShowPanelId = CMIC_SHOW_PANEL_ID;
   #else
     hini.fShowPanelId = CMIC_HIDE_PANEL_ID;
   #endif

   if (loadstring(IDS_HELPLIBRARYNAME,szLibName,HELPLIBRARYNAMELEN) == NULL)
     return;

   hini.pszHelpLibraryName = (PSZ)szLibName;

   if (fileexist(szLibName) == 255) {
     strcpy(szTemp,__argv[0]);
     if ((p = strrchr(szTemp,'\\')) != NULL || (p = strrchr(szTemp,':')) != NULL) {
       strcpy(p+1,szLibName);
       strcpy(szLibName,szTemp);
     }
   }


   /* creating help instance */
   hwndHelpInstance = WinCreateHelpInstance(hab, &hini);

   if(hwndHelpInstance == NULLHANDLE || hini.ulReturnCode)
   {
      MessageBox(hwndMainFrame, IDMSG_HELPLOADERROR, 0, MB_OK | MB_ERROR, TRUE);
      return;
   }

   /* associate help instance with main frame */
   if(!WinAssociateHelpInstance(hwndHelpInstance, hwndMainFrame))
   {
      MessageBox(hwndMainFrame, IDMSG_HELPLOADERROR, 0, MB_OK | MB_ERROR, TRUE);
      return;
   }

   /* help manager is successfully initialized so set flag to TRUE */
   fHelpEnabled = TRUE;
}   /* End of InitHelp   */


/**************************************************************************
 *
 *  Name       : HelpGeneral()
 *
 *  Description: Processes the WM_COMMAND message posted by the
 *               General Help item of the Help menu.
 *
 *  Concepts:    Called from MainCommand when the General Help
 *               menu item is selected.
 *
 *               Sends an HM_EXT_HELP message to the help
 *               instance so that the default Extended Help is
 *               displayed.
 *
 *  API's      : WinSendMsg
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY HelpGeneral(VOID) {
    /* this just displays the system extended help panel */
   if(fHelpEnabled)
      if(NULL != WinSendMsg(hwndHelpInstance, HM_EXT_HELP,
                            (MPARAM)NULL, (MPARAM)NULL))
         MessageBox(hwndMain,
                    IDMSG_HELPDISPLAYERROR,
                    0,
                    MB_OK | MB_ERROR,
                    FALSE);
}   /* End of HelpGeneral  */

/**************************************************************************
 *
 *  Name       : HelpUsingHelp()
 *
 *  Description: Processes the WM_COMMAND message posted by the
 *               Using Help item of the Help menu.
 *
 *  Concepts:    Called from MainCommand when the Using Help
 *               menu item is selected.
 *
 *               Sends an HM_DISPLAY_HELP message to the help
 *               instance so that the default Using Help is
 *               displayed.
 *
 *  API's      : WinSendMsg
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY HelpUsingHelp(VOID) {
   /* this just displays the system help for help panel */
   if(fHelpEnabled)
      if(NULL != WinSendMsg(hwndHelpInstance, HM_DISPLAY_HELP,
                            (MPARAM)NULL, (MPARAM)NULL))
         MessageBox(hwndMain,
                    IDMSG_HELPDISPLAYERROR,
                    0,
                    MB_OK | MB_ERROR,
                    FALSE);
}   /* End of HelpUsingHelp   */


/**************************************************************************
 *
 *  Name       : HelpKeys()
 *
 *  Description: Processes the WM_COMMAND message posted by the
 *               Keys Help item of the Help menu.
 *
 *  Concepts:    Called from MainCommand when the Keys Help
 *               menu item is selected.
 *
 *               Sends an HM_KEYS_HELP message to the help
 *               instance so that the default Keys Help is
 *               displayed.
 *
 *  API's      : WinSendMsg
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY HelpKeys(VOID) {
   /* this just displays the system keys help panel */
   if(fHelpEnabled)
      if(NULL != WinSendMsg(hwndHelpInstance, HM_KEYS_HELP,
                            (MPARAM)NULL, (MPARAM)NULL))
         MessageBox(hwndMain,
                    IDMSG_HELPDISPLAYERROR,
                    0,
                    MB_OK | MB_ERROR,
                    FALSE);
}   /* End of HelpKeys   */


/**************************************************************************
 *
 *  Name       : HelpIndex()
 *
 *  Description: Processes the WM_COMMAND message posted by the
 *               Help Index item of the Help menu.
 *
 *  Concepts:    Called from MainCommand when the Help Index
 *               menu item is selected.
 *
 *               Sends an HM_INDEX_HELP message to the help
 *               instance so that the default Help Index is
 *               displayed.
 *
 *  API's      : WinSendMsg
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY HelpIndex(VOID) {
   /* this just displays the system help index panel */
   if(fHelpEnabled)
      if(NULL != WinSendMsg(hwndHelpInstance, HM_HELP_INDEX,
                             (MPARAM)NULL, (MPARAM)NULL))
         MessageBox(hwndMain,
                    IDMSG_HELPDISPLAYERROR,
                    0,
                    MB_OK | MB_ERROR,
                    FALSE);
}   /* End of HelpIndex() */


/**************************************************************************
 *
 *  Name       : HelpTutorial()
 *
 *  Description: Processes the WM_COMMAND message posted by the
 *                Tutorial Help item of the Help menu.  While the
 *                standard template application does not include a
 *                Tutorial menu item, you can add one if your
 *                application has a tutorial.
 *
 *  Concepts:    Called from MainCommand when the Tutorial Help
 *               menu item is selected.
 *
 *  API's      :  WinLoadMessage
 *                WinMessageBox
 *                WinAlarm
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY HelpTutorial(VOID) {
  DisplayHelpPanel(PANEL_TUTORIAL);
}


/**************************************************************************
 *
 *  Name       : HelpProductInfo()
 *
 *  Description: Processes the WM_COMMAND message posted by the
 *               Product information item of the Help Menu.
 *
 *  Concepts:    Called from MainCommand when the Product information
 *               menu item is selected
 *
 *               Calls WinDlgBox to display the Product information dialog.
 *
 *  API's      : WinDlgBox
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY HelpProductInfo(VOID) {
   /* display the Product Information dialog. */
   WinDlgBox(HWND_DESKTOP,
             hwndMain,
             (PFNWP)ProductInfoDlgProc,
             0,
             IDD_PRODUCTINFO,
             (PVOID)NULL);
}   /* End of HelpProductInfo() */


/**************************************************************************
 *
 *  Name       : DisplayHelpPanel(idPanel)
 *
 *  Description: Displays the help panel whose id is given
 *
 *  Concepts:    Called whenever a help panel is desired to be
 *               displayed, usually from the WM_HELP processing
 *               of the dialog boxes.
 *
 *               Sends HM_DISPLAY_HELP message to the help instance.
 *
 *  API's      : WinSendMsg
 *
 *  Parameters :  idPanel = panel i.d.
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY DisplayHelpPanel(ULONG idPanel) {
   if(fHelpEnabled)
      if(NULL != WinSendMsg(hwndHelpInstance,
                            HM_DISPLAY_HELP,
                            MPFROMLONG(idPanel),
                            MPFROMSHORT(HM_RESOURCEID)))
         MessageBox(hwndMainFrame,
                    IDMSG_HELPDISPLAYERROR,
                    0,
                    MB_OK | MB_ERROR,
                    TRUE);
}   /* End of DisplayHelpPanel   */


/**************************************************************************
 *
 *  Name       : DestroyHelpInstance()
 *
 *  Description: Destroys the help instance for the application
 *
 *  Concepts:    Called after exit from message loop.
 *
 *               Calls WinDestroyHelpInstance() to destroy the
 *               help instance.
 *
 *  API's      : WinDestroyHelpInstance
 *
 *  Parameters :  [none]
 *
 *  Return     :  [none]
 *
 *************************************************************************/
VOID LIBENTRY DestroyHelpInstance(VOID) {
   if(hwndHelpInstance != NULLHANDLE)
      WinDestroyHelpInstance(hwndHelpInstance);
}   /* End of DestroyHelpInstance   */
/***************************  End of help.c  ****************************/
