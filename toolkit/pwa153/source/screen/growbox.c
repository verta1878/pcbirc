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


#include <dos.h>
#include "screen.h"

/********************************************************************
 *
 *  Function:  growbox()
 *
 *  Determine the "center" of a given box area and then draw/erase repeated
 *  boxes growing out from the center until the actual box specified is drawn
 */

void LIBENTRY growbox(char X1,char Y1,char X2,char Y2,char Color,boxlinetype LineType,int DelayTime) {
  int SX1,SY1,SX2,SY2,XDiff,YDiff,MinDiff,XFact,YFact,Counter;

  SX1   = (X2 + X1 - 1) >> 1;
  SX2   = SX1 + 2;
  XDiff = SX1 - X1;
  SY1   = (Y2 + Y1 - 1) >> 1;
  SY2   = SY1 + 2;
  YDiff = SY1 - Y1;

  if (XDiff > YDiff) {
    XFact   = (XDiff / YDiff);
    MinDiff = YDiff;
    YFact   = 1;
  } else {
    YFact   = (YDiff / XDiff);
    MinDiff = XDiff;
    XFact   = 1;
  }
  for (Counter = 0; Counter <= MinDiff; Counter++) {
    boxcls((char) SX1,(char) SY1,(char) SX2,(char) SY2,Color,LineType);
    mydelay(DelayTime);
    SX1 -= XFact;
    SX2 += XFact;
    SY1 -= YFact;
    SY2 += YFact;
  }
  boxcls(X1,Y1,X2,Y2,Color,LineType);
}
