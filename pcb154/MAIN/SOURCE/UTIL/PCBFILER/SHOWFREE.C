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


#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

static struct dfree Rec;
static long   BytesPerCluster;

unsigned long  diskfreespace(char *Path) {
  char   Exist;
  int    DriveToCheck;
  char  *p;
  char   PathToCheck[60];
  char   SavePath[60];

  strupr(Path);
  if (Path[1] == ':') {
    DriveToCheck = Path[0] - 'A' + 1;
    strcpy(PathToCheck,Path);
    if (strlen(PathToCheck) > 3)     /* strip trailing backslash *unless* */
      stripright(PathToCheck,'\\');  /* it is for the ROOT directory      */
  } else {
    strcpy(&PathToCheck[2],Path);
    DriveToCheck   = getdisk() + 1;
    PathToCheck[0] = DriveToCheck + 'A' - 1;
    PathToCheck[1] = ':';
  }

  /* is it a FILE instead of a directory? */
  if ((Exist = fileexist(PathToCheck)) != 255 && (Exist & 16) != 16) {
    /* yes, it's a file, so strip it down to just the path */
    if ((p = strrchr(PathToCheck,'\\')) != NULL || (p = strrchr(PathToCheck,':')) != NULL)
      *p = 0;
  }

  SavePath[0] = DriveToCheck + 'A' - 1;
  SavePath[1] = ':';
  SavePath[2] = '\\';
  getcurdir(DriveToCheck,&SavePath[3]);

  chdir(PathToCheck);
  getdfree(DriveToCheck,&Rec);
  chdir(SavePath);

  if (Rec.df_sclus == 0xffff)
    return(0xFFFFFFFFL);

  BytesPerCluster = (unsigned long) Rec.df_bsec * (unsigned long) Rec.df_sclus;
  return((unsigned long) (Rec.df_avail * BytesPerCluster));
}


void  showfreespace(void) {
  unsigned long Temp;
  char          Answer[35];
  char          Str1[40];
  char          Str2[40];
  char          Str3[40];
  savescrntype  Screen;

  savescreen(&Screen);
  boxcls(18,16,62,22,Colors[OUTBOX],SINGLE);
  fastcenter(18,"Check Free Space on Drive",Colors[HEADING]);

  Answer[0] = getdisk() + 'A';
  Answer[1] = ':';
  Answer[2] = '\\';
  Answer[3] = 0;
  inputstr(20,20,34,"Path",Answer,Answer,ALLFILE,INPUT_CAPS,0);

  if (Answer[0] < 'A' || Answer[0] > 'Z' || KeyFlags == ESC) {
    KeyFlags = NOTHING;
    restorescreen(&Screen);
    return;
  }

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;

  restorescreen(&Screen);
  Temp = diskfreespace(Answer);
  if (Temp == 0xFFFFFFFFL) {
    MsgData.Msg1    = "Invalid Drive!";
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    beep();
    showmessage();
    KeyFlags = NOTHING;
    restorescreen(&Screen);
    return;
  }

  sprintf(Str1,"Drive Space Statistics for Drive %c:",Answer[0]);
  sprintf(Str2,"Total Disk Space: %10lu",(unsigned long) (Rec.df_total * BytesPerCluster));
  sprintf(Str3,"Avail Disk Space: %10lu",Temp);
  MsgData.QuestLine = 0;
  MsgData.Msg1   = Str1;
  MsgData.Line1  = 14;
  MsgData.Color1 = Colors[DISPLAY];
  MsgData.Msg2   = Str2;
  MsgData.Line2  = 16;
  MsgData.Color2 = Colors[DISPLAY];
  MsgData.Msg3   = Str3;
  MsgData.Line3  = 18;
  MsgData.Color3 = Colors[DISPLAY];
  showmessage();
  KeyFlags = NOTHING;
  restorescreen(&Screen);
}
