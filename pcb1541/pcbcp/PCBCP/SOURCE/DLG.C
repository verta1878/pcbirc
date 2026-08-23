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

#define  INCL_WINMENUS
#define  INCL_WINFRAMEMGR
#include <os2.h>
#include <string.h>
#include "main.h"
#include "dlg.h"
#include "xtrn.h"


MRESULT EXPENTRY ProductInfoDlgProc(HWND hwnd, USHORT msg,
                                          MPARAM mp1, MPARAM mp2);
static VOID LIBENTRY SetSysMenu( HWND hDlg );

/**************************************************************************
 *
 *  Name       : ProductInfoDlgProc(hwnd, msg, mp1, mp2)
 *
 *  Description: Processes all messages sent to the Product information
 *               dialog
 *
 *  Concepts:  The Product information dialog has only a button control,
 *             so this routine processes only WM_COMMAND messages.  Any
 *             WM_COMMAND posted must have come from the OK
 *             button, so we dismiss the dialog upon receiving it.
 *
 *  API's      :  WinDismissDlg
 *                WinDefDlgProc
 *
 *  Parameters :  hwnd     = window handle
 *                msg      = message i.d.
 *                mp1      = first message parameter
 *                mp2      = second message parameter
 *
 *  Return     :  dependent on message sent
 *
 *************************************************************************/
MRESULT EXPENTRY ProductInfoDlgProc(
                         HWND hwnd,      /* handle of window */
                         USHORT msg,     /* id of message */
                         MPARAM mp1,     /* first message parameter */
                         MPARAM mp2)     /* second message parameter */
{
   switch(msg)
   {
      case WM_INITDLG:
         SetSysMenu(hwnd);       /* system menu for this dialog  */
         return MRFROMSHORT(FALSE);

      case WM_COMMAND:
           /* no matter what the command, close the dialog */
         WinDismissDlg(hwnd, TRUE);
         break;

      default:
         return(WinDefDlgProc(hwnd, msg, mp1, mp2));

   }
   return (MRESULT)NULL;
}   /*  End of ProductInfoDlgProc   */


/**************************************************************************
 *
 *  Name       : SetSysMenu(hDlg)
 *
 *  Description: Sets only the Move and Close items of the system menu
 *
 *  Concepts:  Any dialog box is free to call this routine, to edit
 *             which menu items will appear on its System Menu pulldown.
 *
 *  API's      :  WinWindowFromID
 *                WinSendMsg
 *
 *  Parameters :  hDlg     = window handle of the dialog
 *
 *  Return     :  [none]
 *
 *************************************************************************/
static VOID LIBENTRY SetSysMenu(HWND hDlg)
{
    HWND     hSysMenu;
    MENUITEM Mi;
    ULONG    Pos;
    MRESULT  Id;
    SHORT    cItems;

    /******************************************************************/
    /*  We only want Move and Close in the system menu.               */
    /******************************************************************/

    hSysMenu = WinWindowFromID(hDlg, FID_SYSMENU);
    WinSendMsg( hSysMenu, MM_QUERYITEM
              , MPFROM2SHORT(SC_SYSMENU, FALSE), MPFROMP((PCH) & Mi));
    Pos = 0L;
    cItems = (SHORT)WinSendMsg( Mi.hwndSubMenu, MM_QUERYITEMCOUNT,
                                (MPARAM)NULL, (MPARAM)NULL);
    while (cItems--)
    {
        Id = WinSendMsg( Mi.hwndSubMenu, MM_ITEMIDFROMPOSITION
                          , MPFROMLONG(Pos), (MPARAM)NULL);
        switch (SHORT1FROMMR(Id))
        {
        case SC_MOVE:
        case SC_CLOSE:
            Pos++;  /* Don't delete that one. */
            break;
        default:
            WinSendMsg( Mi.hwndSubMenu, MM_DELETEITEM
                      , MPFROM2SHORT((USHORT)Id, TRUE), (MPARAM)NULL);
        }
    }
}   /*  End of SetSysMenu  */
/***************************  End of dlg.c  ****************************/
