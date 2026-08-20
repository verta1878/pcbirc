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
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <misc.h>
#include <pcb.h>
#include <help.h>
#include "pcbfiles.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: undo()
*
*  Desc    : Copies the USERS.BAK file to USERS and recreates the indexes.
*
*/

void pascal undo(void) {
  bool Okay;

  Okay = FALSE;

  boxcls(19,18,59,22,Colors[MENUBOX],SINGLE);
  inputnum(22,20,1,"Restore Users File from backup",&Okay,vBOOL,RESTORE);
  if (KeyFlags == ESC)
    return;

  if (fileexist(BackFileName) == 255) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Save    = TRUE;
    MsgData.Msg1    = "The BACKUP file does not exist and the operation cannot be undone.";
    MsgData.Line1   = 16;
    MsgData.Color1  = Colors[HEADING];
    MsgData.Msg2 = BackFileName;
    MsgData.Line2   = 18;
    MsgData.Color2  = Colors[QUESTION];
    showmessage();
    return;
  }

  if (Okay) {
    unlink(TempFileName);                    /* remove USERS.TMP           */
    unlink(TempInfFileName);                 /* remove USERS.NEW           */
    rename(BackFileName   ,TempFileName );   /* rename USERS.BAK USERS.TMP */  //lint !e534
    rename(PcbData.UsrFile,BackFileName );   /* rename USERS     USERS.BAK */  //lint !e534
    rename(TempFileName   ,PcbData.UsrFile); /* rename USERS.TMP USERS     */  //lint !e534

    if (fileexist(BackInfFileName) != 255) {
      rename(BackInfFileName,TempInfFileName); /* rename USERS.IBK USERS.NEW */  //lint !e534
      rename(PcbData.InfFile,BackInfFileName); /* rename USERS.INF USERS.IBK */  //lint !e534
      rename(TempInfFileName,PcbData.InfFile); /* rename USERS.NEW USERS.INF */  //lint !e534
    }
    createallindexes();
  }
}
