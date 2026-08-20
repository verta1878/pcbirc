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
#include "newscr.h"
#include "menu.h"

/***********************/
#define NUMOPTIONS 30

wordtype static Options[NUMOPTIONS] = {
 {"ALIAS"}, {"BD"}, {"BROADCAST"}, {"BU"}, {"BYE"}, {"CHAT"}, {"DB"}, {"DOOR"},
 {"DOWNLOAD"}, {"FLAG"}, {"HELP"}, {"JOIN"}, {"LANG"}, {"MENU"}, {"NEWS"},
 {"NODE"}, {"OPEN"}, {"PPE"}, {"QWK"}, {"REPLY"}, {"RM"}, {"RM+"}, {"RM-"},
 {"SELECT"}, {"TEST"}, {"TS"}, {"UB"}, {"UPLOAD"}, {"USERS"}, {"WHO"}
};

enum {
  O_ALIAS,O_BD,O_CAST,O_BU,O_BYE,O_CHAT,O_DB,O_DOOR,O_DOWN,O_FLAG,O_HELP,
  O_JOIN,O_LANG,O_MENU,O_NEWS,O_NODE,O_OPEN,O_PPE,O_QWK,O_REPLY,O_RM,O_RMF,
  O_RMB,O_SEL,O_TEST,O_TS,O_UB,O_UPLD,O_USERS,O_WHO
};
/***********************/


#pragma warn -par
static void LIBENTRY initialscreen(int NumTokens) {
  println(PcbData.BoardName);
  print(Status.Version);
  print(Status.NodeStr);

  newline();
  displayfile(PcbData.WlcFile,WELCOME|GRAPHICS|LANGUAGE);
}
#pragma warn +par


#ifdef PCB152
static void LIBENTRY runppe(int NumTokens) {
  char *p;
  char  FileName[66];

  if (NumTokens > 0) {
    maxstrcpy(FileName,getnexttoken(),sizeof(FileName));

    if ((p = strchr(FileName,'.')) == NULL)
      strcat(FileName,".PPE");
    else if (strcmp(p,".PPE") != 0)
      return;

    writeusernetstatus(UNAVAILABLE,NULL);
    doScript(FileName,NULL,NumTokens-1);    //lint !e534
    usernetavailable();
  }
}
#endif


static void _NEAR_ LIBENTRY displaynews(void) {
  newline();
  startdisplay(NOCHANGE);
  if (fileexist(Status.CurConf.NewsFile) == 255) {
    displaypcbtext(TXT_NONEWS,NEWLINE);
    return;
  }
  if (displayfile(Status.CurConf.NewsFile,GRAPHICS|SECURITY|LANGUAGE) == -2)
    displaypcbtext(TXT_NONEWS,DEFAULTS);
  newline();
}


static void LIBENTRY displayfilecommand(int NumTokens) {
  char *p;

  if (NumTokens) {
    p = getnexttoken();
    maxstrcpy(Status.DisplayText,p,sizeof(Status.DisplayText));
  } else {
    Status.DisplayText[0] = 0;
    inputfield(Status.DisplayText,TXT_TEXTVIEWFILENAME,30,UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_filename);
    if (Status.DisplayText[0] == 0)
      return;
    p = Status.DisplayText;
    newline();
  }

  startdisplay(NOCHANGE);
  if (displayfile(p,NOALTERNATE) != -2)
    logsystext(TXT_TEXTFILEVIEWED,SPACERIGHT);
}


static void LIBENTRY dircommand(int NumTokens) {
  char   Exist;
  char  *p;
  int    Count;
  int    Len;
  char   DateStr[10];
  char   TimeStr[10];
  char   Size[20];
  char   Input[31];
  char   Path[80];
  char   Str[80];
  char   mask_dircmd[] = {15,'!',0,'#','*',')','-','.',0,'0',':',0,'?','z','á','~'};
  struct ffblk FileInfo;
  #ifdef __OS2__
  int    DirHandle = MAKEDIRHANDLE;
  #endif

  if (NumTokens) {
    p = getnexttoken();
    maxstrcpy(Input,p,sizeof(Input));
  } else {
    Input[0] = 0;
    inputfield(Input,TXT_ENTERDIRCMD,30,UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_dircmd);
    if (Input[0] == 0)
      return;
    p = Input;
  }

  // if it's just a drive letter (e.g. C:) or drive letter plus backslash (C:\)
  // then add *.* to the string
  if ((Input[0] == '\\' &&  Input[1] == 0) ||
      (Input[1] == ':'  && (Input[2] == 0  ||
                           (Input[2] == '\\' && Input[3] == 0)))
     )
    strcat(Input,"*.*");
  else if (strchr(Input,'?') == NULL &&
           strchr(Input,'*') == NULL &&
           ((Exist = fileexist(Input)) & 16) != 0 && Exist != 255) {
    // it's a directory, so add "\*.*" to the directory name
    strcat(Input,"\\*.*");
  } else if ((Len = strlen(Input)) > 0 && Input[Len-1] == '.' && (Len == 1 || Input[Len-2] == '.' || Input[Len-2] == ':')) {
    // if it's a path such as "." or "F:." or "F:.." add "\*.*" to the name
    strcat(Input,"\\*.*");
  }

  if (Input[0] == '\\' && Input[1] == '\\') {
    // skip calling fullyqualifiedname() if the input path begins with two
    // backslashes because the function will prepend a drive letter on it
    // which will be *wrong* for a UNC pathname.
    strcpy(Path,Input);
  } else
    fullyqualifiedname(Path,Input,sizeof(Path));

  Count = 1;
  startdisplay(NOCHANGE);
  strcpy(Status.DisplayText,Path);
  displaypcbtext(TXT_DIRECTORYOF,NEWLINE|LFAFTER|LFBEFORE);

  if (dosfindfirst(Path,&FileInfo,FA_ARCH|FA_DIREC,&Count PDIRHANDLE) == -1) {
    displaypcbtext(TXT_NOFILESFOUND,NEWLINE);
    return;
  }

  printcolor(0x07);

  do {
    // avoid showing device names
    if (FileInfo.ff_attrib != 64) {
      #ifdef __OS2__
        sprintf(Str,"%s  %s",
                dtoc(FileInfo.ff_fdate,DateStr),
                ttoc(FileInfo.ff_ftime,TimeStr));
        print(Str);
        if ((FileInfo.ff_attrib & 16) != 0)
          strcpy(Str,"    <DIR>        ");
        else
          sprintf(Str,"%15s  ",ucomma(Size,FileInfo.ff_fsize));
        print(Str);
        println(FileInfo.ff_name);
      #else
        sprintf(Str,"%-12.12s",FileInfo.ff_name);
        print(Str);
        if ((FileInfo.ff_attrib & 16) != 0)
          strcpy(Str,"    <DIR>        ");
        else
          sprintf(Str,"%15s  ",ucomma(Size,FileInfo.ff_fsize));
        print(Str);
        sprintf(Str,"%s  %s",
                dtoc(FileInfo.ff_fdate,DateStr),
                ttoc(FileInfo.ff_ftime,TimeStr));
        println(Str);
      #endif
    }

    Count = 1;
  } while (dosfindnext(&FileInfo,&Count PDIRHANDLE2) != -1);

  #ifdef __OS2__
    dosclosedirhandle(DirHandle);
  #endif
}


static void _NEAR_ LIBENTRY displaymenu(void) {
  char *p;

  newline();
  if (Status.CurSecLevel < PcbData.SysopSec[SEC_SYSOPLEVEL])
    p = Status.CurConf.UserMenu;
  else
    p = Status.CurConf.SysopMenu;

  displayMNU = FALSE;

  startdisplay(NOCHANGE);
  displayfile(p,GRAPHICS|SECURITY|LANGUAGE|RUNMENU|RUNPPL);

  if (displayMNU)
    displaymenu();
}


/********************************************************************
*
*  Function:  doCommand()
*
*  Desc    :  This is the main subroutine of command().
*             It will process passed in command information and then
*             return to the caller, allowing commands to be processed
*             from multiple locations without requiring keyboard stuffing.
*
*  NOTES   :  Externalized from command() by Scott Dale Robison on 18 Jan 1995.
*             Now also used by MNU processor and PPL.
*/

//  char *p;
//  int  NumTries;
//  int  NumTokens;
//  int  Num;
//  bool PressEnter;
//  bool TypedAhead;
//  char Str[250];
//  char Command[256];
//  char Save[256];

int LIBENTRY doCommand ( bool TypedAhead, char * p, int NumTokens,
    int & NumTries, char * Command, int CommandSize, char *Str)
{
    int  Num;
    int  RetVal;

    // NOTES!!!  These changes to code are necessary for command()
    //
    //    formerly return   statements = return -2 (command() should terminate)
    //    formerly continue statements = return -1 (command() should redo loop)
    //    normal function exit         = return  0 (command() should do nothing)

    // The PPL may have stuffed the keyboard ... if so, don't let it feed back
    // on itself, instead, drop the keystrokes through to be processed.

    if (! TypedAhead) {
      if (runcmds(p,NumTokens-1)) {
        NumTries = 0;
        if (ppltypeahead()) {
          gettypeaheadstr(Command,CommandSize-1);
          NumTokens = tokenize(Command);
          p = getnexttoken();
        } else
          return -1; // continue;
      }
    }

    if (alldigits(p)) {            /* number requests */
      Num = atoi(p);
      switch (Num) {
        case  1: dispatch(p,PcbData.SysopSec[SEC_1],NumTokens,viewcallerslog);
                 NumTries = 0;
                 break;
        case  2: dispatch(p,PcbData.SysopSec[SEC_2],NumTokens,viewprintusers);
                 NumTries = 0;
                 break;
        case  3: dispatch(p,PcbData.SysopSec[SEC_3],NumTokens,packmessages);
                 NumTries = 0;
                 break;
        case  4: dispatch(p,PcbData.SysopSec[SEC_4],NumTokens,unkill);
                 NumTries = 0;
                 break;
        case  5: Status.HeaderScan = TRUE;
                 Status.QuickScan  = TRUE;
                 dispatch(p,PcbData.SysopSec[SEC_5],NumTokens,messageread);
                 Status.HeaderScan = FALSE;
                 Status.QuickScan  = FALSE;
                 NumTries = 0;
                 break;
        case  6: dispatch(p,PcbData.SysopSec[SEC_6],NumTokens,displayfilecommand);
                 NumTries = 0;
                 break;
        case  7: dispatch(p,PcbData.SysopSec[SEC_7],NumTokens,usermaint);
                 NumTries = 0;
                 break;
        case  8: dispatch(p,PcbData.SysopSec[SEC_8],NumTokens,packusers);
                 NumTries = 0;
                 break;
        case  9: dispatch(p,PcbData.SysopSec[SEC_9],NumTokens,remotedos);
                 NumTries = 0;
                 break;
        case 10: dispatch(p,PcbData.SysopSec[SEC_10],NumTokens,doscommand);
                 NumTries = 0;
                 break;
        case 11: if (PcbData.Network) {
                   strcpy(Command,"X");
                   tokenize(Command);   //lint !e534
                   dispatch("11",PcbData.SysopSec[SEC_11],2,displayusernet);
                 } else
                   displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                 NumTries = 0;
                 break;
        case 12: if (PcbData.Network)
                   dispatch(p,PcbData.SysopSec[SEC_12],NumTokens,logoffcommand);
                 else
                   displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                 NumTries = 0;
                 break;
        case 13: if (PcbData.Network)
                   dispatch(p,PcbData.SysopSec[SEC_13],NumTokens,nodeviewcallerslog);
                 else
                   displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                 NumTries = 0;
                 break;
        case 14: if (PcbData.Network)
                   dispatch(p,PcbData.SysopSec[SEC_14],NumTokens,dropdoscommand);
                 else
                   displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                 NumTries = 0;
                 break;
        case 15: if (PcbData.Network)
                   dispatch(p,PcbData.SysopSec[SEC_15],NumTokens,recyclecommand);
                 else
                   displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                 NumTries = 0;
                 break;
        case 16: dispatch(p,PcbData.SysopSec[SEC_6],NumTokens,dircommand);
                 NumTries = 0;
                 break;
      }
    } else if (p[1] == 0) {                      /* single letter requests */
      switch (p[0]) {
        case 'A': dispatch(p,PcbData.UserLevels[SEC_A],NumTokens,abandon);
                  NumTries = 0;
                  break;
        case 'B': dispatch(p,PcbData.UserLevels[SEC_B],NumTokens,bltmenu);
                  NumTries = 0;
                  break;
        case 'C': dispatch(p,PcbData.UserLevels[SEC_C],1,entercomment);
                  NumTries = 0;
                  break;
        case 'D': dispatch(p,PcbData.UserLevels[SEC_D],NumTokens,send);
                  NumTries = 0;
                  break;
        case 'E': dispatch(p,PcbData.UserLevels[SEC_E],NumTokens,entermessage);
                  NumTries = 0;
                  break;
        case 'F': dispatch(p,PcbData.UserLevels[SEC_F],NumTokens,dirmenu);
                  NumTries = 0;
                  break;
        case 'G': byecommand();
                  break;
        case '?':
        case 'H': dispatch(p,PcbData.UserLevels[SEC_H],NumTokens,helpcommand);
                  NumTries = 0;
                  break;
        case 'I': dispatch(p,0,NumTokens,initialscreen);
                  if (UsersData.ExpertMode)
                    NumTries = 1; /* we don't want it to give an extra line feed afterwards */
                  else {
                    NumTries = 0; /* force it to redisplay menu in novice mode */
                    Display.NumLinesPrinted = 1;
                  }
                  break;
        case 'J': dispatch(p,PcbData.UserLevels[SEC_J],NumTokens,joinconfcommand);
                  NumTries = 0;
                  break;
        case 'K': dispatch(p,PcbData.UserLevels[SEC_K],NumTokens,killcommand);
                  NumTries = 0;
                  break;
        case 'L': dispatch(p,PcbData.UserLevels[SEC_L],NumTokens,filescan);
                  NumTries = 0;
                  break;
        case 'M': if (! seclevelokay("M",PcbData.UserLevels[SEC_M])) {
                    NumTries = -2;  /* indicate error */
                    break;
                  }
                  displaycmdfile("M");
                  if (PcbData.NonGraphics)
                    displaypcbtext(TXT_GRAPHICSUNAVAIL,NEWLINE|LFBEFORE);
                  else {
                    if (NumTokens > 1) {
                      p = getnexttoken();
                      if (memcmp(p,"CT",2) == 0) {
                        UseAnsi = FALSE;
                        HasRip  = FALSE;
                        Control.RipMode = FALSE;
                        Control.GraphicsMode = FALSE;
                        displaypcbtext(TXT_CTTYON,NEWLINE|LFBEFORE|LOGIT);
                      } else if (memcmp(p,"AN",2) == 0) {
                        UseAnsi = TRUE;
                        Control.RipMode = FALSE;
                        Control.GraphicsMode = FALSE;
                        displaypcbtext(TXT_ANSION,NEWLINE|LFBEFORE|LOGIT);
                      } else if (memcmp(p,"GR",2) == 0) {
                        asetcolor(0);  /* this will force a full color reset */
                        Control.RipMode = FALSE;
                        Control.GraphicsMode = TRUE;
                        UseAnsi = TRUE;
                        displaypcbtext(TXT_GRAPHICSON,NEWLINE|LFBEFORE|LOGIT);
                      } else if (memcmp(p,"RI",2) == 0) {
                        asetcolor(0);  /* this will force a full color reset */
                        Control.GraphicsMode = TRUE;
                        Control.RipMode = TRUE;
                        UseAnsi = TRUE;
                        HasRip  = TRUE;
                        displaypcbtext(TXT_RIPMODEON,NEWLINE|LFBEFORE|LOGIT);
                        printdefcolor();
                      }
                    } else if (Control.GraphicsMode) {
                      asetcolor(0);  /* this will force a full color reset */
                      printcolor(7); /* set color to grey on black */
                      asetcolor(Display.DefaultColor);
                      UseAnsi = FALSE;
                      HasRip  = FALSE;
                      Control.GraphicsMode = FALSE;
                      Control.RipMode = FALSE;
                      checkforrip();
                      displaypcbtext(TXT_GRAPHICSOFF,NEWLINE|LFBEFORE|LOGIT);
                    } else {
                      asetcolor(0);  /* this will force a full color reset */
                      Control.GraphicsMode = TRUE;
                      UseAnsi = TRUE;
                      displaypcbtext(TXT_GRAPHICSON,NEWLINE|LFBEFORE|LOGIT);
                    }
                  }
                  redisplaystatusline();
                  NumTries = 0;
                  break;
        case 'N': dispatch(p,PcbData.UserLevels[SEC_N],NumTokens,newfilescan);
                  NumTries = 0;
                  break;
        #ifdef COMM
        case 'O': NumTokens = 1;  /* force it to 0 (1 minus 1 = 0) to indicate user-requested chat */
                  dispatch(p,PcbData.UserLevels[SEC_O],NumTokens,sysopchat);
                  NumTries = 0;
                  break;
        #endif
        case 'P': dispatch(p,PcbData.UserLevels[SEC_P],NumTokens,setpagelength);
                  NumTries = 0;
                  break;
        case 'Q': Status.QuickScan = TRUE;
                  dispatch(p,PcbData.UserLevels[SEC_Q],NumTokens,messageread);
                  if (Status.XferCapture)
                    send(0);
                  Status.QuickScan = FALSE;
                  NumTries = 0;
                  break;
        case 'R': Status.ReplyCommand = FALSE;
                  dispatch(p,PcbData.UserLevels[SEC_R],NumTokens,messageread);
                  if (Status.XferCapture)
                    send(0);
                  NumTries = 0;
                  break;
        case 'S': dispatch(p,PcbData.UserLevels[SEC_S],NumTokens,scriptmenu);
                  NumTries = 0;
                  break;
        case 'T': dispatch(p,PcbData.UserLevels[SEC_T],NumTokens,setprotocol);
                  NumTries = 0;
                  break;
        case 'U': dispatch(p,PcbData.UserLevels[SEC_U],NumTokens,receive);
                  NumTries = 0;
                  break;
        case 'V': dispatch(p,PcbData.UserLevels[SEC_V],NumTokens,showuserstatus);
                  NumTries = 0;
                  break;
        case 'W': dispatch(p,(Status.MultipleLogins ? 256 : PcbData.UserLevels[SEC_W]),NumTokens,modifyself);
                  NumTries = 0;
                  break;
        case 'X': if (seclevelokay(p,PcbData.UserLevels[SEC_X])) {
                    displaycmdfile("X");
                    if (NumTokens > 1) {
                      p = getnexttoken();
                      if (strcmp(p,"ON") == 0) {
                        UsersData.ExpertMode = TRUE;
                        displaypcbtext(TXT_EXPERTON,NEWLINE|LFBEFORE|LOGIT);
                      } else if (strcmp(p,"OFF") == 0) {
                        UsersData.ExpertMode = FALSE;
                        displaypcbtext(TXT_EXPERTOFF,NEWLINE|LFBEFORE|LOGIT);
                      }
                    } else {
                      if (UsersData.ExpertMode) {
                        UsersData.ExpertMode = FALSE;
                        displaypcbtext(TXT_EXPERTOFF,NEWLINE|LFBEFORE|LOGIT);
                      } else {
                        UsersData.ExpertMode = TRUE;
                        displaypcbtext(TXT_EXPERTON,NEWLINE|LFBEFORE|LOGIT);
                      }
                    }
                  }
                  NumTries = 0;
                  break;
        case 'Y': dispatch(p,PcbData.UserLevels[SEC_Y],NumTokens,messagescan);
                  NumTries = 0;
                  break;
        case 'Z': dispatch(p,PcbData.UserLevels[SEC_Z],NumTokens,zippyscan);
                  NumTries = 0;
                  break;
        default:  if (Status.UserRecNo > 0 && Status.CurSecLevel >= PcbData.UserLevels[SEC_OPEN]) {
                    displaycmdfile(p);
                    RetVal = searchdoorlist(p,NumTokens-1);
                    if (RetVal == 0) {
                      NumTries = 0;
                      break;
                    }
                    if (RetVal <= -4)
                      return -1; // continue;
                  }
                  NumTries++;
                  if (NumTries > 15) {
                    loguseroff(ALOGOFF);
                    return -2; // return;
                  }
      }
    } else {                              /* multi-letter (WORD) requests */
      Num = option(Options,p,NUMOPTIONS);
      switch (Num) {
        case O_ALIAS: if (AliasSupport) {
                        displaycmdfile("ALIAS");
                        if (NumTokens > 1) {
                          p = getnexttoken();
                          if (strcmp(p,"ON") == 0)
                            Status.UseAlias = TRUE;
                          else if (strcmp(p,"OFF") == 0)
                            Status.UseAlias = FALSE;
                          else
                            Status.UseAlias = (bool) (! Status.UseAlias);
                        } else
                          Status.UseAlias = (bool) (! Status.UseAlias);
                        aliasnames();
                      }
                      NumTries = 0;
                      break;
        case O_BYE  : displaycmdfile("BYE");
                      loguseroff(NLOGOFF);
                      return -2; // return;
        case O_NODE :
        case O_CHAT : if (PcbData.Network)
                        dispatch("CHAT",PcbData.UserLevels[SEC_CHAT],NumTokens,chatcommand);
                      else
                        displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                      NumTries = 0;
                      break;
        case O_WHO:   if (PcbData.Network)
                        dispatch("WHO",PcbData.UserLevels[SEC_WHO],NumTokens,displayusernet);
                      else
                        displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                      NumTries = 0;
                      break;
        case O_CAST : if (PcbData.Network)
                        dispatch("BRDCST",PcbData.SysopSec[SEC_BROADCAST],NumTokens,broadcast);
                      else
                        displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                      NumTries = 0;
                      break;
        case O_BD   :
        case O_DB   : Status.Batch = (bool) (Status.CurSecLevel >= PcbData.UserLevels[SEC_BATCH]);
                      dispatch("DB",PcbData.UserLevels[SEC_D],NumTokens,send);
                      NumTries = 0;
                      break;
        case O_HELP : dispatch("HELP",PcbData.UserLevels[SEC_H],NumTokens,helpcommand);
                      NumTries = 0;
                      break;
        case O_DOOR :
        case O_OPEN : dispatch("OPEN",PcbData.UserLevels[SEC_OPEN],NumTokens,opendoor);
                      NumTries = 0;
                      break;
        case O_DOWN : dispatch("DOWN",PcbData.UserLevels[SEC_D],NumTokens,send);
                      NumTries = 0;
                      break;
        case O_FLAG : dispatch("FLAG",PcbData.UserLevels[SEC_D],NumTokens,flagfiles);
                      NumTries = 0;
                      break;
        case O_REPLY: Status.ReplyCommand = TRUE;
                      dispatch("REPLY",PcbData.UserLevels[SEC_E],NumTokens,messageread);
                      NumTries = 0;
                      break;
        case O_JOIN : dispatch("JOIN",PcbData.UserLevels[SEC_J],NumTokens,joinconfcommand);
                      NumTries = 0;
                      break;
        case O_LANG : if (PcbData.MultiLingual) {
                        displaycmdfile("LANG");
                        newline();
                        getlanguage(NumTokens-1);
                      } else
                        displaypcbtext(TXT_LANGALTNOTAVAIL,NEWLINE|LFBEFORE);
                      NumTries = 0;
                      break;
        case O_MENU : displaycmdfile("MENU");
                      displaymenu();
                      NumTries = 1;  /* AVOID! redisplaying the menu */
                      break;
        case O_NEWS : displaycmdfile("NEWS");
                      displaynews();
                      NumTries = 0;
                      break;
#ifdef PCB152
        case O_PPE  : dispatch("PPE",PcbData.SysopSec[SEC_10],NumTokens,runppe);
                      NumTries = 0;
                      break;
#endif
        case O_QWK  : dispatch("QWK",PcbData.UserLevels[SEC_R],NumTokens,qwkcommand);
                      NumTries = 0;
                      break;
        case O_RM   :
        case O_RMF  :
        case O_RMB  : strcpy(Command,Str); /* reset token pointer, add 1 to NumTokens for assumed 'R' command */
                      NumTokens = tokenize(Command);
                      dispatch(Str,PcbData.UserLevels[SEC_R],NumTokens+1,messageread);
                      if (Status.XferCapture)
                        send(0);
                      NumTries = 0;
                      break;
        case O_SEL  : dispatch("SELECT",PcbData.UserLevels[SEC_J],NumTokens,selectconf);
                      NumTries = 0;
                      break;
        case O_TEST : dispatch("TEST",PcbData.UserLevels[SEC_TEST],NumTokens,verifyfilecommand);
                      NumTries = 0;
                      break;
        case O_TS   : strcpy(Command,Str); /* reset token pointer, add 1 to NumTokens for assumed 'R' command */
                      NumTokens = tokenize(Command);
                      dispatch("TS",PcbData.UserLevels[SEC_R],NumTokens+1,messageread);
                      if (Status.XferCapture)
                        send(0);
                      NumTries = 0;
                      break;
        case O_BU   :
        case O_UB   : Status.Batch = (bool) (Status.CurSecLevel >= PcbData.UserLevels[SEC_BATCH]);
                      dispatch("UB",PcbData.UserLevels[SEC_U],NumTokens,receive);
                      NumTries = 0;
                      break;
        case O_UPLD : dispatch("U",PcbData.UserLevels[SEC_U],NumTokens,receive);
                      NumTries = 0;
                      break;
        case O_USERS: dispatch("USERS",(ProtectAlias && AliasSupport && Status.CurConf.AllowAliases ? PcbData.SysopSec[SEC_7] : PcbData.UserLevels[SEC_USER]),NumTokens,scanforusers);
                      NumTries = 0;
                      break;
        default     : if (Status.UserRecNo > 0 && Status.CurSecLevel >= PcbData.UserLevels[SEC_OPEN]) {
                        displaycmdfile(p);
                        RetVal = searchdoorlist(p,NumTokens-1);
                        if (RetVal == 0) {
                          NumTries = 0;
                          break;
                        }
                        if (RetVal <= -4)
                          return -1; // continue;
                      }
                      maxstrcpy(Status.DisplayText,p,sizeof(Status.DisplayText));
                      displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE|LFAFTER|BELL);
                      NumTries++;
                      if (NumTries > 15) {
                        loguseroff(ALOGOFF);
                        return -2; // return;
                      }
      }
    }

    return 0;
}


/********************************************************************
*
*  Function:  command()
*
*  Desc    :  This is the main command center.  The only way out of this
*             function is to logoff or recycle the code.
*
*  NOTES   :  The Status.InFileListing is left turned on after exiting the
*             file listing so that the MORE prompt can allow the caller to
*             flag files for download.  Then it is turned off right after the
*             MORE prompt.
*/

void LIBENTRY command(void) {
  char *p;
  int  NumTries;
  int  NumTokens;
  bool PressEnter;
  bool TypedAhead;
  char Str[250];
  char Command[256];
  char Save[256];

  Save[0]            = 0;
  NumTries           = 0;
  PressEnter         = (bool) (Display.NumLinesPrinted > 3);  /* did we scan mail before getting here? */
  Display.CountLines = FALSE;

  usernetavailable();
  loadcmds();

  while (1) {
    if (Display.AbortPrintout)   /* catch any stray aborts */
      checkdisplaystatus();

    // Check to see if the caller has moved around where the alias name has
    // changed and the WHO display needs to be updated.
    if (PcbData.ShowAlias && Status.HideAliasStartTime != 0 && doselapsedtime(Status.HideAliasStartTime) > 60*100 )
      usernetavailable();

    if ((! Display.WasNonStop) && (! Display.WasAborted) && Status.InFileListing && checkscreenforfilenames(1,NULL)) {
      newline();
      moreprompt(MOREPROMPT);
    }

    Status.InFileListing = FALSE;

    if (NumTries == 0 && ! (UsersData.ExpertMode || mnuListExist || mnuExit || ppltypeahead())) {
      if (PressEnter && Display.NumLinesPrinted > 0) {
        newline();
        moreprompt(PRESSENTER);
        checkdisplaystatus();
      }
      displaymenu();
      NumTries = 1;  /* set to 1 so that we don't print another blank line */
    }

    /* in case the keyboard timer was turned off due to a NON-STOP display */
    /* we'll turn it back on now plus we'll reset the keyboard timer       */
    turnkbdtimeron();

    Display.NumLinesPrinted = 0;  /* avoid a more prompt at this point */
    freshline();
    if (NumTries == 0)
      newline();

    if (Status.CheckForMail || UsersData.PackedFlags.HasMail)  // has someone posted mail to us online?
      if (checkmailflags())                                    // if so, go check mail waiting flags to be sure
        if (checkformail()) {  // if the user read mail then loop back up
          NumTries = 0;        // so that it will display the menu in
          continue;            // novice mode
        }

    if (ppltypeahead()) {
      gettypeaheadstr(Str,sizeof(Str)-1);
      strupr(Str);
      TypedAhead = TRUE;
    } else {

      if (mnuListExist && ! typeahead()) {
        doMenuList();   //lint !e534
        continue;
      }

      TypedAhead = FALSE;
      Str[0] = 0;
      Status.CmdPrompt = TRUE;
      inputfield(Str,TXT_COMMANDPROMPT,sizeof(Str)-1,UPCASE|HIGHASCII|STACKED|NEWLINE,NOHELP,mask_command);
      Status.CmdPrompt = FALSE;
      if (Str[0] == '!' && Str[1] == 0) {
        stuffbufferstr(Save);
        continue;
      }
    }

    strcpy(Command,Str);
    if (strlen(Str) >= 5)
      strcpy(Save,Str);

    NumTokens = tokenize(Command);
    if (NumTokens == 0) {
      PressEnter = FALSE;
      NumTries++;
      if (NumTries > 15) {
        displaypcbtext(TXT_EXCESSIVEERRORS,NEWLINE|LFBEFORE|BELL|LOGIT);
        loguseroff(ALOGOFF);
        return;
      }
      continue;
    }

    PressEnter = TRUE;
    p = getnexttoken();

    switch (doCommand(TypedAhead,p,NumTokens,NumTries,Command,sizeof(Command),Str))
    {
        case  0: break;
        case -1: continue;
        case -2: return;
        default: /* perhaps an error handler here? */ break;
    }

    if (NumTries == 0)
      mnuExit = 0;
  }
}

