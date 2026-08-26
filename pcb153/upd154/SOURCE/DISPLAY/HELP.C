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

/***********************/
#define NUMHELPOPTIONS 23
wordtype static Options[NUMHELPOPTIONS] = {
 {"ALIAS"}, {"BD"}, {"BROADCAST"}, {"BU"}, {"BYE"}, {"CHAT"}, {"DB"}, {"DOOR"},
 {"FLAG"}, {"JOIN"}, {"LANG"}, {"NEWS"}, {"NODE"}, {"OPEN"}, {"QWK"},
 {"REPLY"}, {"RM"}, {"SELECT"}, {"TEST"}, {"TS"}, {"UB"},
 {"USERS"}, {"WHO"} };

enum {O_ALIAS,O_BD,O_BROADCAST,O_BU,O_BYE,O_CHAT,O_DB,O_DOOR,O_FLAG,O_JOIN,O_LANG,O_NEWS,O_NODE,O_OPEN,O_QWK,O_REPLY,O_RM,O_SEL,O_TEST,O_TS,O_UB,O_USERS,O_WHO};
/***********************/

char static *HelpStr;

int LIBENTRY displayhelpfile(int HelpNum) {
  bool Temp;
  bool ClearScrnWhenFull;
  int  RetVal;
  char *p;
  char *q;
  char Name[66];

  if (HelpNum == NOHELP)
    return(0);

  ClearScrnWhenFull = Display.ClearScrnWhenFull;
  if (ClearScrnWhenFull) {
    Display.ClearScrnWhenFull = FALSE;
    savescrnbuffer();
  }

  RetVal = 0;
  Temp = Display.CountLines;
  startdisplay(NOCHANGE);

  if (HelpNum == HLP_MORE || HelpNum == HLP_MOREXFER) {
    displaypcbtext(TXT_MOREHELP_ENTER   ,NEWLINE|LFBEFORE);
    displaypcbtext(TXT_MOREHELP_YES     ,NEWLINE);
    displaypcbtext(TXT_MOREHELP_NO      ,NEWLINE);
    displaypcbtext(TXT_MOREHELP_NONSTOP ,NEWLINE);
    if (HelpNum == HLP_MOREXFER) {
      displaypcbtext(TXT_MOREHELP_VIEW,NEWLINE);
      displaypcbtext(TXT_MOREHELP_FLAG,NEWLINE);
    }

    newline();

    if (ClearScrnWhenFull) {
      moreprompt(PRESSENTER);
      restscrnbuffer();
      Display.NumLinesPrinted = 0;
      Display.ClearScrnWhenFull = TRUE;
    }
  } else {
    buildstr(Name,PcbData.HlpLoc,"HLP",NULL);
    p = &Name[strlen(Name)];

    if (HelpNum >= HLP_A && HelpNum <= HLP_Z) {
      *p = (char) ('A' + (HelpNum-HLP_A));
      p++;
      *p = 0;
    } else if (Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL] && HelpNum >= HLP_1 && HelpNum <= HLP_15) {
      ascii(p,HelpNum - HLP_1 + 1);
    } else {
      switch (HelpNum) {
        case HLP_BROADCAST: q = "BRD";     break;
        case HLP_OPEN    : q = "OPEN";     break;
        case HLP_CHAT    : q = "CHAT";     break;
        case HLP_CMENU   : q = "CMENU";    break;
        case HLP_ENDR    : q = "ENDR";     break;
        case HLP_FLAG    : q = "FLAG";     break;
        case HLP_FULLSCRN: q = "FSCRN";    break;
        case HLP_LANG    : q = "LANG";     break;
        case HLP_MSG     : q = "R";        break;
        case HLP_NEWS    : q = "NEWS";     break;
        case HLP_QWK     : q = "QWK";      break;
        case HLP_REG     : q = "REG";      break;
        case HLP_REP     : q = "REP";      break;
        case HLP_RM      : q = "RM";       break;
        case HLP_SEC     : q = "SEC";      break;
        case HLP_SEL     : q = "SEL";      break;
        case HLP_SRCH    : q = "SRCH";     break;
        case HLP_TEST    : q = "TEST";     break;
        case HLP_TS      : q = "TS";       break;
        case HLP_USER    : q = "USERS";    break;
        case HLP_WHO     : q = "WHO";      break;
        case HLP_REPEAT  : q = "!";        break;
        case HLP_ALIAS   : q = "ALIAS";    break;
        case HLP_TEXT    : p -= 3;         /* remove "HLP" from the string */
                           q = HelpStr;
                           if (invalidfilename(q,INFORM))
                             return(0);
                           break;
        default          : return(0);
      }
      strcpy(p,q);
    }


    startdisplay(FORCECOUNTLINES);
    RetVal = displayfile(Name,LANGUAGE|SECURITY|GRAPHICS|RUNMENU|RUNPPL);

    if (ClearScrnWhenFull) {
      moreprompt(PRESSENTER);
      restscrnbuffer();
      Display.NumLinesPrinted = 0;
      Display.ClearScrnWhenFull = TRUE;
    }
  }
  Display.CountLines = Temp;
  return(RetVal);
}


void LIBENTRY helpcommand(int NumTokens) {
  char Str[10];
  char *p;
  int  HelpNum;

  if (NumTokens) {
    p = getnexttoken();
  } else {
    if (UsersData.ExpertMode) {
      if (Status.CurSecLevel < PcbData.SysopSec[SEC_SYSOPLEVEL])
        p = Status.CurConf.UserMenu;
      else
        p = Status.CurConf.SysopMenu;
      startdisplay(NOCHANGE);
      displayfile(p,GRAPHICS|SECURITY|LANGUAGE);
    }
    freshline();
    Str[0] = 0;
    inputfield(Str,TXT_HELPPROMPT,8,UPCASE|NEWLINE|HIGHASCII,NOHELP,mask_command);
    if (Str[0] == 0)
      return;
    p = Str;
  }

  HelpNum = atoi(p);
  if (HelpNum >= 1 && HelpNum <= 15) {
    displayhelpfile(HelpNum - 1 + HLP_1);
    return;
  }

  if (*(p+1) == 0) {
    if (*p == '!')
      displayhelpfile(HLP_REPEAT);
    else if (*p >= 'A' && *p <= 'Z')
      displayhelpfile(*p-'A' + HLP_A);
    return;
  }

  switch (option(Options,p,NUMHELPOPTIONS)) {
    case O_ALIAS: HelpNum = HLP_ALIAS; break;
    case O_BROADCAST: HelpNum = HLP_BROADCAST; break;
    case O_NODE :
    case O_CHAT : HelpNum = HLP_CHAT;
                  break;
    case O_OPEN :
    case O_DOOR : HelpNum = HLP_OPEN;
                  break;
    case O_BYE  : HelpNum = HLP_G;     break;
    case O_BD   :
    case O_DB   : HelpNum = HLP_D;     break;
    case O_FLAG : HelpNum = HLP_FLAG;  break;
    case O_JOIN : HelpNum = HLP_J;     break;
    case O_LANG : HelpNum = HLP_LANG;  break;
    case O_NEWS : HelpNum = HLP_NEWS;  break;
    case O_QWK  : HelpNum = HLP_QWK;   break;
    case O_REPLY: HelpNum = HLP_REP;   break;
    case O_RM   : HelpNum = HLP_RM;    break;
    case O_SEL  : HelpNum = HLP_SEL;   break;
    case O_TS   : HelpNum = HLP_TS;    break;
    case O_UB   :
    case O_BU   : HelpNum = HLP_U;     break;
    case O_TEST : HelpNum = HLP_TEST;  break;
    case O_USERS: HelpNum = HLP_USER;  break;
    case O_WHO  : HelpNum = HLP_WHO;   break;
    default     : if (strlen(p) > 8)
                    p[8] = 0;
                  HelpStr = p;
                  HelpNum = HLP_TEXT;
                  break;
  }

  displayhelpfile(HelpNum);
}
