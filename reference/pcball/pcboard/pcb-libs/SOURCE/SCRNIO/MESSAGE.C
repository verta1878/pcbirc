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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

MsgType MsgData;

static int _NEAR_ LIBENTRY maxlen(char *Str, int Len) {
  int StrLen;

  if (Str == NULL)
    return(Len);

  StrLen = strlen(Str);
  if (StrLen > Len)
    return(StrLen);
  return(Len);
}

void LIBENTRY showmessage(void) {
  char         X,Y;
  int          Temp1;
  int          Temp2;
  int          CursorStatus;
  savescrntype ScrnBuf;


  if (MsgData.Save) {
    savescreen(&ScrnBuf);
    X = wherex();
    Y = wherey();
  }

  if (MsgData.AutoBox) {
    if (MsgData.Line4 == 0 && MsgData.QuestLine == 0) {
      MsgData.Line4  = Scrn_BottomRow - 4;
      MsgData.Msg4   = "press any key to continue";
      MsgData.Color4 = Colors[DISPLAY];
    }

    Temp1 = strlen(MsgData.Msg1);
    Temp1 = maxlen(MsgData.Msg2,Temp1);
    Temp1 = maxlen(MsgData.Msg3,Temp1);
    Temp1 = maxlen(MsgData.Msg4,Temp1);

    if (MsgData.Quest != NULL) {
      if (strlen(MsgData.Quest)+4 > Temp1)
        Temp1 = strlen(MsgData.Quest)+4;
    }

    Temp1 = (Temp1 >> 1) + 2;
    Temp2 = (MsgData.Line1 != 0 ? MsgData.Line1 : MsgData.QuestLine) - 2;
    boxcls((char) (39-Temp1),(char) Temp2,(char) (40+Temp1),(char) (Scrn_BottomRow-2),Colors[OUTBOX],SINGLE);
  } else
    boxcls((char) MsgData.X1,(char) MsgData.Y1,(char) MsgData.X2,(char) MsgData.Y2,Colors[OUTBOX],SINGLE);

  if (MsgData.Line1) {
    Temp1 = 39 - (strlen(MsgData.Msg1) >> 1);
    fastprintmove(Temp1,MsgData.Line1,MsgData.Msg1,(char) MsgData.Color1);
  }
  if (MsgData.Line2) {
    Temp1 = 39 - (strlen(MsgData.Msg2) >> 1);
    fastprintmove(Temp1,MsgData.Line2,MsgData.Msg2,(char) MsgData.Color2);
  }
  if (MsgData.Line3) {
    Temp1 = 39 - (strlen(MsgData.Msg3) >> 1);
    fastprintmove(Temp1,MsgData.Line3,MsgData.Msg3,(char) MsgData.Color3);
  }
  if (MsgData.Line4) {
    Temp1 = 39 - (strlen(MsgData.Msg4) >> 1);
    fastprintmove(Temp1,MsgData.Line4,MsgData.Msg4,(char) MsgData.Color4);
  }

  CursorStatus = getcursor();
  setcursor(CUR_NORMAL);
  if (MsgData.QuestLine) {
    Temp1 = 39 - ((strlen(MsgData.Quest)+4) >> 1);
    inputstr(Temp1,MsgData.QuestLine,1,MsgData.Quest,MsgData.Answer,MsgData.Answer,MsgData.Mask,INPUT_CAPS,0);
  } else {
    Temp2 = inkey(&X,ShowClock);
  }
  setcursor(CursorStatus);

  if (MsgData.Save) {
    restorescreen(&ScrnBuf);
    gotoxy(X,Y);
  }
}
