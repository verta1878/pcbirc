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


#include <stdlib.h>
#include <stdio.h>
#include "scrnio.h"
#include "scrnio.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifndef min
  #define min(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
  #define max(x,y) ((x) > (y) ? (x) : (y))
#endif

/********************************************************************
 *
 *  Function:  processcursorkeys()
 *
 *  This function takes the return status of one of the input functions (return
 *  value is in the variable KeyFlags) and interrupts from it what it should
 *  do.  For example, whether the cursor should go left, right up or down.  Or
 *  if the page on the screen should be scroll up or down or even paged forward
 *  or backward one screen.
 *
 *  This function returns TRUE if the screen needs to be redrawn.
 */

bool LIBENTRY processcursorkeys(int *Index, long *TopLine, int *Column, long MaxLine, int MaxColumn, int LinesOnScreen) {
  int  Col;           /* Column number (base 0)                 */
  long Ind;           /* Index from top line on screen          */
  long Top;           /* offset for record shown on Top         */
  long MoveIndex;     /* Used when determine # of lines to move */
  long Less1;         /* Number of lines on screen less 1       */
  long LastTop;       /* The last possible TOP line             */
  bool RetVal;        /* TRUE if need to redraw the screen      */

  Col     = *Column;
  Ind     = *Index;
  Top     = *TopLine;
  Less1   = min(MaxLine-1,min(MaxLine-Top-1,LinesOnScreen-1));
  LastTop = max(0,MaxLine - Less1 - 1);

  if (KeyFlags == SHIFTTAB) {
    if (Col == 0)
      KeyFlags = UP;
    Col--;
  }

  if (KeyFlags == RET || KeyFlags == TAB) {
    if (Col == MaxColumn)
      KeyFlags = DN;
    Col++;
  }

  if (Col < 0)
    Col = MaxColumn;
  if (Col > MaxColumn)
    Col = 0;

  RetVal = FALSE;
  switch (KeyFlags) {

    case UP       : if (Ind > 0)
                      Ind--;
                    else
                      if (Top > 0) {
                        Top--;
                        RetVal = TRUE;
                      }
                    break;

    case DN       : if (Ind < min(MaxLine-1,Less1))
                      Ind++;
                    else
                      if (Top < MaxLine-Less1-1) {
                        Top++;
                        RetVal = TRUE;
                      }
                    break;

    case PGUP     : if (Top > 0) {
                      MoveIndex = min(Top,LinesOnScreen);
                      Top -= MoveIndex;
                      Ind = Less1;
                      RetVal = TRUE;
                    } else {
                      Ind = 0;
                    }
                    break;

    case PGDN     : MoveIndex = min(LastTop-Top,LinesOnScreen);
                    if (MoveIndex != 0)
                      if (MoveIndex == LinesOnScreen) {
                        Top += MoveIndex;
                        Ind = 0;
                        RetVal = TRUE;
                      } else {
                        Top = LastTop;
                        Ind = LinesOnScreen - MoveIndex;
                        RetVal = TRUE;
                      }
                    break;

    case CTRLPGUP : if (Ind != 0)
                      Ind = 0;
                    else {
                      Top = 0;
                      RetVal = TRUE;
                    }
                    break;

    case CTRLPGDN : if (Ind != Less1)
                      Ind = Less1;
                    else {
                      Top = LastTop;
                      RetVal = TRUE;
                    }
                    break;
  }

  *Column  = Col;
  *Index   = (int) Ind;
  *TopLine = Top;
  return(RetVal);
}
