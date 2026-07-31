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


#include "project.h"
#pragma hdrstop

#include "messages.h"

static void _NEAR_ LIBENTRY showstring(int PcbTextNum, char *Str) {
  displaypcbtext(PcbTextNum,DEFAULTS);
  printdefcolor();
  println(Str);
}


static void _NEAR_ LIBENTRY shownumber(int PcbTextNum, long Num) {
  pcbtexttype Buf;
  char Str[128];

  Str[0] = ' ';
  if (Num == -1) {
    getpcbtext(TXT_UNLIMITED,&Buf);
    strcpy(&Str[1],Buf.Str);
  } else {
    comma(&Str[1],Num);
  }
  showstring(PcbTextNum,Str);
}


void LIBENTRY showuserstatus(int NumTokens) {
  long            LastMsgRead;
  msgbasestattype MsgStats;
  char            Str[80];
  pcbtexttype     Buf;

  displaycmdfile("PRESTAT");

  if (displaycmdfile("STAT") == 0)
    return;

  newline();

  if (Display.PageLen != 0 && (Display.NumLinesPrinted + 18 > Display.PageLen))
    moreprompt(PRESSENTER);

  printdefcolor();

  if (NumTokens != -1) {
    if (Asy.Online == REMOTE || ! PcbData.ExcludeLocals)
      shownumber(TXT_CALLERNUMBER,Status.CallerNumber);

    if (Status.User == USER) {
      strcpy(Str,juliantodate(UsersData.LastDateOn));
      countrydate(Str);
      showstring(TXT_LASTDATEONE,Str);
      if (UsersData.RegExpDate == 0) {
        getpcbtext(TXT_NONE,&Buf);
        showstring(TXT_EXPIREDATE,Buf.Str);
      } else {
        strcpy(Str,juliantodate(UsersData.RegExpDate));
        countrydate(Str);
        showstring(TXT_EXPIREDATE,Str);
      }
      shownumber(TXT_NUMTIMESON ,UsersData.NumTimesOn);
    }

    shownumber(TXT_PAGELENGTH      ,UsersData.PageLen);

    if (UsersData.ExpertMode)
      displaypcbtext(TXT_EXPERTMODEON,NEWLINE);
    else
      displaypcbtext(TXT_EXPERTMODEOFF,NEWLINE);

    if (Status.User == USER) {
      shownumber(TXT_SECURITYLEVEL ,Status.CurSecLevel);
      shownumber(TXT_NUMDOWNLOADS  ,UsersData.NumDownloads);
      shownumber(TXT_NUMUPLOADS    ,UsersData.NumUploads);

      shownumber(TXT_BYTESAVAILABLE,totalbytesleft());
      if (Status.ByteRatio != 0) {
        byteratio(Str);
        showstring(TXT_SHOWBYTERATIO,Str);
      }
      if (Status.FileRatio != 0) {
        fileratio(Str);
        showstring(TXT_SHOWFILERATIO,Str);
      }
    }
  }

  msgbasestats(&MsgStats);   //lint !e534
  LastMsgRead = fixlmrpointer(Status.Conference,MsgStats.LowMsgNum,MsgStats.HighMsgNum);
  shownumber(TXT_LASTMSGREAD  ,LastMsgRead);
  shownumber(TXT_HIGHMSGNUMBER,MsgStats.HighMsgNum);

  if (Status.User == SYSOP && Status.CurConf.OldIndex) {
    shownumber(TXT_LOWMSGNUM,MsgStats.LowMsgNum);
    shownumber(TXT_NUMDEFINEDMSGS,(long) Status.CurConf.MsgBlocks << 10);
    shownumber(TXT_NUMFREEMSGS,freemsgs(&MsgStats));
  }

  shownumber(TXT_NUMACTIVEMSGS   ,MsgStats.NumActiveMsgs);

  if (NumTokens != -1) {
    if (UsersData.Protocol >= 'A' && UsersData.Protocol <= 'Z' && Protocols[UsersData.Protocol-'A'].Desc[0] != 0)
      showstring(TXT_TRANSFERPROTOCOL,Protocols[UsersData.Protocol-'A'].Desc);
    else {
      displaypcbtext(TXT_TRANSFERPROTOCOL,DEFAULTS);
      printdefcolor();
      displaypcbtext(TXT_NONE,NEWLINE);
    }
  }
}
