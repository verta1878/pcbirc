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


#include <alloc.h>
#include <misc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

       char ActionChar;
static bool ForwAction;
static int  Action;
static int  ActionLoc;
static int  ActionX;
static int  ActionY;
static int  ActionMax;
static int  ActionPivot;
static int  ActionStart;

void pascal showaction(void) {
  if ((Action & ActionMax) == 0) {
    if (Action == ActionPivot) {
      ForwAction = ! ForwAction;
      Action = 0;
    }
    if (ForwAction) {
      fastputc(ActionLoc,ActionChar);
      ActionLoc += 2;
      ActionX++;
    } else {
      fastputc(ActionLoc,' ');
      ActionLoc -= 2;
      ActionX--;
    }
    gotoxy(ActionX,ActionY);
  }
  Action++;
}

void pascal startaction(char ShowChar, bool ShowMem, int Max) {
  ActionChar  = ShowChar;
  ActionPivot = Max * 8;
  ActionMax   = Max - 1;           /* NOTE:  Max must be a POWER OF 2 */
  ForwAction  = TRUE;
  Action      = 0;
  ActionX     = wherex();
  ActionY     = wherey();
  ActionLoc   = xyvalue(ActionX,ActionY);
  ActionStart = ActionX;
}

void pascal resetaction(char ShowChar, bool ShowMem, int Max) {
  clsbox(ActionStart,ActionY,ActionStart+8,ActionY,Colors[DISPLAY]);
  gotoxy(ActionStart,ActionY);
  startaction(ShowChar,ShowMem,Max);
}
