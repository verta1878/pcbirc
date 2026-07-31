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


#include "screen.h"

#ifdef OLDWINDOWFUNCTIONS
extern struct WindType Windows[];


/********************************************************************
 *
 *  Function:  defwindow()
 *
 *  Defines a window's coordinates, line-type, color and pointer so that
 *  subsequent calls to openwindow() only require the "number" of the window
 *  to open
 */

void pascal defwindow( int  Num,
                            char X1, char Y1, char X2, char Y2,
                            char Type,
                            char Color,
                            int  far *Sp ) {

  struct WindType *W;

  W = &Windows[Num];

  (*W).WindX1 = X1;    /* store coordinates */
  (*W).WindY1 = Y1;
  (*W).WindX2 = X2;
  (*W).WindY2 = Y2;

  (*W).BorderType  = Type;
  (*W).BorderColor = Color;

  (*W).P = Sp;
}


/********************************************************************
 *
 *  Function:  openwindow()
 *
 *  Opens a specified window which was defined previously with the defwindow()
 *  function
 */

void pascal openwindow(int Num) {

  struct WindType *W;

  W = &Windows[Num];

  (*W).CursorX      = wherex();
  (*W).CursorY      = wherey();
  (*W).CursorStatus = getcursor();

  savescreen((*W).P);

  boxcls( (*W).WindX1,      (*W).WindY1,
          (*W).WindX2,      (*W).WindY2,
          (*W).BorderColor, (*W).BorderType);
}


/********************************************************************
 *
 *  Function:  closewindow()
 *
 *  Closes a specified window as predefined by the defwindow() function
 */

void pascal closewindow(int Num) {

  struct WindType *W;

  W = &Windows[Num];

  restorescreen((*W).P);

  gotoxy((*W).CursorX,(*W).CursorY);
  setcursor((*W).CursorStatus);
}
#endif
