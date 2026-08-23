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

// Turn off "argument not used" warnings in this file
#pragma warn -par

#define INCL_WINFRAMEMGR
#define INCL_WINSWITCHLIST
#define INCL_WINSTDFILE
#define INCL_WINMENUS

#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dosfunc.h>
#include <misc.h>
#include "main.h"
#include "xtrn.h"
#include "help.h"

/*
 *  Global variables
 */

CHAR szFullPath[CCHMAXPATH] = "";

/*
 *  Entry point declarations
 */

void LIBENTRY setmenuitems(void) {
  if (hwndMainFrame > 0) {
    bool Enabled  = (bool) (UserNetFile > 0);
    HWND hwndMenu = WinWindowFromID(hwndMainFrame,FID_MENU);
    EnableMenuItem(hwndMenu,IDM_FILELIMIT,Enabled);
    EnableMenuItem(hwndMenu,IDM_FILENODE,Enabled);
    EnableMenuItem(hwndMenu,IDM_EDITRECORD,Enabled);
    EnableMenuItem(hwndMenu,IDM_ACTION,Enabled);
    EnableMenuItem(hwndMenu,IDM_UPDATE,Enabled);
  }
}


int LIBENTRY openusernetfile(char *FileName) {
  int  NumBytes;

  if ((UserNetFile = dosopencheck(FileName,OPEN_RDWR|OPEN_DENYNONE)) == -1) {
    MessageBox(hwndMain,IDMSG_CANNOTOPENUSERNETFILE,0,MB_OK|MB_ERROR,FALSE);
    UserNetFile = 0;
    return(-1);
  }

  readcheck(UserNetFile,&UserNetHeader,sizeof(usernethdrtype));

  if (UserNetHeader.Version != 150) {
    MessageBox(hwndMain,IDMSG_WRONGVERSION,0,MB_OK|MB_ERROR,FALSE);
    dosclose(UserNetFile);
    UserNetFile = 0;
    return(-1);
  }

  MaxNodes = UserNetHeader.NumOfNodes;
  NumBytes = sizeof(nodetype) * MaxNodes;

  if ((NodeArray = (nodetype *) malloc(NumBytes)) == NULL ||
      (SaveArray = (nodetype *) malloc(NumBytes)) == NULL ||
      (SessionIds = (int *) malloc(MaxNodes * sizeof(int))) == NULL ||
      (Semaphores = (int *) malloc(MaxNodes * sizeof(int))) == NULL ||
      (StopNodes = (bool *) malloc(MaxNodes)) == NULL) {
    MessageBox(hwndMain,IDMSG_CANNOTALLOCATEMEMORY,0,MB_OK|MB_ERROR,FALSE);
    if (NodeArray != NULL)
      free(NodeArray);
    dosclose(UserNetFile);
    UserNetFile = 0;
    return(-1);
  }

  memset(NodeArray,0,NumBytes);
  memset(SaveArray,0,NumBytes);
  memset(SessionIds,0,MaxNodes * sizeof(int));
  memset(Semaphores,0,MaxNodes * sizeof(int));
  memset(StopNodes,0,MaxNodes);

  if (Settings.FirstNode > MaxNodes || Settings.FirstNode < 1)
    Settings.FirstNode = 1;

  if (Settings.LastNode > MaxNodes || Settings.LastNode < 1)
    Settings.LastNode = MaxNodes;

  if (Settings.FirstNode > Settings.LastNode) {
    Settings.FirstNode = 1;
    Settings.LastNode = MaxNodes;
  }

  setmenuitems();
  readnodeprofile();
  return(0);
}


static void LIBENTRY closeusernetfile(void) {
  SkipScreenUpdate = TRUE;
  if (UserNetFile != 0) {
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
  if (SessionIds != NULL) {
    free(SessionIds);
    SessionIds = NULL;
  }
  if (Semaphores != NULL) {
    free(Semaphores);
    Semaphores = NULL;
  }
  setmenuitems();
}


/**************************************************************************
 *
 *  Name       : FileOpen(mp2)
 *
 *  Description: Processes the File menu's Open item
 *
 *  Concepts:  Called whenever OPEN from the FILE menu is selected
 *
 *             Calls the standard file open dialog to get the
 *             file name.  The file name is passed onto DosOpen
 *             which returns the handle to the file.  The file
 *             input procedure is called and then the file handle
 *             is closed.
 *
 *  API's      : WinFileDlg
 *               DosOpen
 *               DosClose
 *
 *  Parameters :  mp2      = second message parameter
 *
 *  Return     :  [none]
 *
 *************************************************************************/
#pragma option -w-par
VOID LIBENTRY FileOpen(MPARAM mp2) {
   FILEDLG fdg;
   CHAR szTitle[MESSAGELEN], szButton[MESSAGELEN];

   SkipScreenUpdate = TRUE;

   fdg.cbSize = sizeof(FILEDLG);

   if (loadstring(IDS_OPEN,szTitle,MESSAGELEN) == NULL)
     return;

   if (loadstring(IDS_OPEN,szButton,MESSAGELEN) == NULL)
     return;

   fdg.pszTitle = (PSZ)szTitle;
   fdg.pszOKButton = (PSZ)szButton;
   fdg.ulUser = 0L;
   fdg.fl = FDS_HELPBUTTON | FDS_CENTER | FDS_OPEN_DIALOG;
   fdg.pfnDlgProc = (PFNWP)TemplateOpenFilterProc;
   fdg.lReturn = 0L;
   fdg.lSRC = 0L;
   fdg.hMod = 0;
   fdg.usDlgId = FILEOPEN;
   fdg.x = 0;
   fdg.y = 0;

   if (loadstring(IDS_FILEOPENEXT,fdg.szFullFile,CCHMAXPATH) == NULL)
     return;

   fdg.pszIType       = (PSZ)NULL;
   fdg.papszITypeList = (PAPSZ)NULL;
   fdg.pszIDrive      = (PSZ)NULL;
   fdg.papszIDriveList= (PAPSZ)NULL;
   fdg.sEAType        = (SHORT)0;
   fdg.papszFQFilename= (PAPSZ)NULL;
   fdg.ulFQFCount     = 0L;

retry:
   /* get the file */
   if(!WinFileDlg(HWND_DESKTOP, hwndMain, &fdg))
      return;

   /*
    *  Upon sucessful return of a file, open it for reading
    */

   switch (fdg.lReturn) {
     case ID_OK    :  closeusernetfile();
                      if (openusernetfile(fdg.szFullFile) == -1)
                        goto retry;
                      SkipScreenUpdate = FALSE;
                      WinPostMsg(hwndMain,WM_PAINT,0,0);
                      break;
     case ID_CANCEL:  SkipScreenUpdate = FALSE;
                      WinPostMsg(hwndMain,WM_PAINT,0,0);
                      return;
   }

}   /* End of FileOpen   */
#pragma option -wpar


/**************************************************************************
 *
 *  Name       : UpdateTitleText
 *
 *  Description: Updates the text in the main window's title bar to
 *               display the app name, followed by the separator,
 *               followed by the file name.
 *
 *  Concepts :   Called at init time and when the text file is changed.
 *               Gets the program name, appends the separator, and
 *               appends the file name.
 *
 *  API's      : WinSetWindowText
 *               WinWindowFromID
 *
 *  Parameters : hwnd = window handle
 *
 *  Return     :  [none]
 *
 *************************************************************************/

VOID LIBENTRY UpdateTitleText(HWND hwnd, char *Text, int NodeNum) {
  if (Text != NULL) {
    char Str[128];
    sprintf(Str,Text,NodeNum);
    WinSetWindowText(hwnd,(PSZ)Str);
  } else {
    WinSetWindowText(hwnd,(PSZ)szAppName);
  }
}   /* End of UpdateTitleText   */


/**************************************************************************
 *
 *  Name       : TemplateOpenFilterProc(hwnd, msg, mp1, mp2)
 *
 *  Description: Filters the help messages to the open dialog
 *
 *  Concepts :   If help is requested during the standard OPEN
 *               dialog, this routine will display an appropriate
 *               panel.
 *
 *  API's      : WinDefFileDlgProc
 *
 *  Parameters : hwnd = window handle
 *               message = message i.d.
 *               mp1     = first message parameter
 *               mp2     = second message parameter
 *
 *  Return     :  the result of the message
 *
 *************************************************************************/
static MRESULT EXPENTRY TemplateOpenFilterProc(HWND hwnd, ULONG message,
                                        MPARAM mp1, MPARAM mp2) {
  if (message == WM_HELP) {
    DisplayHelpPanel(HID_FS_OPEN_DLG_HELP_PANEL);
    return (MRESULT)FALSE ;
  }

  return WinDefFileDlgProc(hwnd, message, mp1, mp2);
}

/***************************  End of file.c  ***************************/
