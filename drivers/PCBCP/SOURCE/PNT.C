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

#include <string.h>

#define  INCL_WINSYS
#define  INCL_GPILCIDS
#define  INCL_GPIPRIMITIVES
#include <os2.h>
#include "main.h"
#include "xtrn.h"

/*
 *  Global variables
 */

/*
 *  Entry point declarations
 */


/**************************************************************************
 *
 *  Name       : MainPaint(hwnd)
 *
 *  Description: Paints the main client window.
 *
 *  Concepts:  Routine is called whenver the client window
 *             procedure receives a WM_PAINT message
 *
 *             - begins painting by calling WinBeginPaint
 *                 and retrieving the HPS for the window
 *             - performs any painting desired
 *             - ends painting by calling WinEndPaint
 *
 *  API's      :  WinBeginPaint
 *                WinFillRect
 *                WinEndPaint
 *
 *  Parameters :  hwnd     = window handle
 *
 *  Return     :  [none]
 *
 *************************************************************************/

// static char *Str = "Hello PM World!";

#define NUMLINES 10


VOID LIBENTRY MainPaint(HWND hwnd) {
   RECTL  rc;
   HPS    hps;

   if (SkipScreenUpdate)
     return;

   SkipScreenUpdate = TRUE;
   hps = WinBeginPaint(hwnd, NULLHANDLE, &rc);

   WinQueryWindowRect(hwndMain,&rc);
// numWinLines = rc.yTop/fontHeight;
// yTop = rc.yTop;

   /* fill update rectangle with window color */
   WinFillRect(hpsMain,&rc,SYSCLR_WINDOW);
   WinEndPaint(hps);

   SkipScreenUpdate = FALSE;
   PostBkThreadMsg(WM_PAINT,0,0);

}   /* End of MainPaint   */
/***************************  End of pnt.c  ****************************/
