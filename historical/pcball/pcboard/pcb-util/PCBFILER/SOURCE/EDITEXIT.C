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


#include <mem.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <pcb.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

char pascal editexit(char *Str) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox   = TRUE;
  MsgData.Save      = TRUE;
  MsgData.Msg1      = Str;
  MsgData.Line1     = Scrn_BottomRow - 6;
  MsgData.Color1    = Colors[QUESTION];
  MsgData.Quest     = "Save the changes made (Yes; No-resume edit; Abort-exit, no save)";
  MsgData.QuestLine = Scrn_BottomRow - 4;
  MsgData.Answer[0] = 'Y';
  MsgData.Answer[1] = 0;
  MsgData.Mask      = YNA;
  showmessage();
  if (KeyFlags == ESC)
    MsgData.Answer[0] = 'N';
  return(MsgData.Answer[0]);
}
