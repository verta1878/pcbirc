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

#ifdef __WATCOMC__
  #include <io.h>
#endif

#include <cnameidx.h>
#include "messages.h"

typedef enum {CONFNUMOKAY,CONFNUMBAD,CONFSEARCH,CONFRELIST} confnumtype;

#define MAXCONFLINES   50
#define NUMAREAS       PcbData.NumAreas
#define NUMCONFS       PcbData.NumConf

typedef struct {
  unsigned Num;
  char     Name[61];
  long     HighNum;
} conflinetype;

#if defined(__cplusplus) && ! defined(max)
  #define max(a,b)    (((a) > (b)) ? (a) : (b))
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

static void _NEAR_ LIBENTRY makeconfstrsub(char *Str, unsigned ConfNum, bool Sys) {
  bool (LIBENTRY *gettext)(int PcbTextNum, pcbtexttype *Buffer);
  pcbtexttype Buf;
  pcbconftype Conf;

  gettext = (Sys ? getsystext : getpcbtext);

  if (ConfNum == 0) {
    gettext(TXT_MAINBOARD,&Buf);
    buildstr(Str,Buf.Str," ",NULL);
  } else {
    gettext(TXT_CONFERENCE,&Buf);
    if (ConfNum == Status.Conference) {
      maxstrcpy(Str,Status.CurConf.Name,Sys ? 14 : 61);
    } else {
      getconfrecord(ConfNum,&Conf);
      maxstrcpy(Str,Conf.Name,Sys ? 14 : 61);
    }
    sprintf(Str+strlen(Str)," (%u)%s ",ConfNum,Buf.Str);
  }
}


void LIBENTRY makeconfstr(char *Str, unsigned ConfNum) {
  makeconfstrsub(Str,ConfNum,FALSE);
}


void LIBENTRY makeconfstrsys(char *Str, unsigned ConfNum) {
  makeconfstrsub(Str,ConfNum,TRUE);
}


void LIBENTRY makeconfreadstr(char *Str, unsigned ConfNum) {
  pcbtexttype Buf;
  pcbconftype Conf;

  if (ConfNum == 0) {
    getpcbtext(TXT_MAINBOARD,&Buf);
    sprintf(Str,"%s (0)",Buf.Str);
  } else {
    if (ConfNum == Status.Conference) {
      sprintf(Str,"%s (%d)",Status.CurConf.Name,ConfNum);
    } else {
      getconfrecord(ConfNum,&Conf);
      sprintf(Str,"%s (%d)",Conf.Name,ConfNum);
    }
  }
//strupr(Str);
//padstr(Str,' ',25);
}


void LIBENTRY confdetails(addsectype Action) {
  int Temp;
  int OldLevel;

  OldLevel = Status.CurSecLevel;

  /* remove any added security from a previous conference */
  if (Status.ConfAddSec != 0) {
    Status.CurSecLevel -= (char) Status.ConfAddSec;
    Status.ConfAddSec = 0;
  }

  /* if he's a conference sysop then change to sysop level */
  if (isset(&ConfReg[CON],Status.Conference)) {
    if (Status.CurSecLevel < PcbData.SysopSec[SEC_SYSOPLEVEL]) {
      Temp = PcbData.SysopSec[SEC_SYSOPLEVEL] - Status.CurSecLevel;
      Status.ConfAddSec  += (char) Temp;
      Status.CurSecLevel += (char) Temp;
    }
  }

  /* Add any additional security that may be available in this conference. */
  /* But be careful not to go over 255 because the BYTE will only store up */
  /* to 255.  If adding would go over 255, it'll avoid adding and also it  */
  /* will avoid recording (in Status.ConfAddSec) that anything was added.  */
  if (Status.CurConf.AddSec != 0 && Status.CurSecLevel + Status.CurConf.AddSec <= 255) {
    Status.ConfAddSec  += Status.CurConf.AddSec;
    Status.CurSecLevel += (char) Status.CurConf.AddSec;
  }

  /* add new time that comes with this conference, if any (but not during   */
  /* a message scan - so use UPDATEUSER to determine when it's okay to add) */
  if (Status.CurConf.AddTime != 0 && Action == UPDATEUSER) {
    addtime(Status.CurConf.AddTime * 60L,CONFTIME);
  }

  if (Action == UPDATEUSER && PcbData.ConfPwrdAdjust && Status.CurSecLevel != OldLevel)
    updateuserinformation();

  redisplaystatusline();
}


void LIBENTRY processjoin(bool ShowNews, bool QuickLogon, bool FullStats) {
  bool Scanned;
  int  File;
  int  ConfNum;
  unsigned X;
  int  Retry;
  long FileDateTime;
  char Name[66];
  char Str[80];
  char Block[4096];

  ConfNum = Status.Conference;
  strcpy(Name,Status.CurConf.MsgFile);

  Retry = 0;
try1:
  if (fileexist(Name) == 255) {
    #ifndef PCB_DEMO
      if (! PcbData.AutoMakeMsgs)
        goto try2;
      if (Retry < 6) {
        Retry++;
        tickdelay(QUARTERSECOND);
        goto try1;
      }
    #endif
    displaypcbtext(TXT_CREATINGNEWMSGBASE,DEFAULTS);
    if ((File = doscreatecheck(Name,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL)) == -1)
      return;
    memset(Block,0,12);
    memset(&Block[12],' ',116);
    writecheck(File,Block,128);   /*lint !e534 */
    dosclose(File);
    if (OldIndex || Status.CurConf.OldIndex) {
      strcat(Name,".NDX");
      unlink(Name);
    }
  } else
    if (OldIndex || Status.CurConf.OldIndex)
      strcat(Name,".NDX");

  Retry = 0;
try2:
  if (OldIndex || Status.CurConf.OldIndex) {
    if (fileexist(Name) == 255) {
      #ifndef PCB_DEMO
        if (! PcbData.AutoMakeMsgs)
          goto try3;
        if (Retry < 6) {
          Retry++;
          tickdelay(QUARTERSECOND);
          goto try2;
        }
      #endif
      if ((File = doscreatecheck(Name,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL)) == -1)
        return;
      memset(Block,0,4096);
      for (X = 0; X < Status.CurConf.MsgBlocks; X++) {
        print(".");
        writecheck(File,Block,4096);   /*lint !e534 */
      }
      newline();
      dosclose(File);
    }
  }

try3:
  updatemsgindex();  // update MSGS.IDX file if necessary

  if (PcbData.DisplayNews == 'A')  /* override the ShowNews variable */
    ShowNews = TRUE;
  else {
    /* don't check for new NEWS if we've already checked it */
    if (previousconfmatchfound(Status.CurConf.NewsFile))
      ShowNews = FALSE;
  }

  if (ShowNews) {
    strcpy(Name,Status.CurConf.NewsFile);
    getalternatename(Name,GRAPHICS|SECURITY|LANGUAGE,0);
    /* if the file is not specified, or is missing, don't display the NEWS */
    if (Name[0] == 0 || (FileDateTime = getfiledatetime(Name)) == 0)
      ShowNews = FALSE;
    /* If set to ALWAYS display the news, then go show it now */
    else if (PcbData.DisplayNews == 'A')
      goto showit;
    /* if it's the sysop AND he is in expert mode, don't show the NEWS */
    else if (Status.User == SYSOP && UsersData.ExpertMode)
      ShowNews = FALSE;
    /* if the news has already been seen, by having joined, don't show it again */
    else if (isset(&ConfFlags[JOINED],ConfNum))
      ShowNews = FALSE;
    /* if this is a NEW USER or a DEMO ACCOUNT then show the news now */
    else if (Status.NewUser || Status.MultipleLogins)
      goto showit;
    /* if set to display news only if changed, check the date of the news */
    /* and if not more recent disable news display                        */
    else if (PcbData.DisplayNews == 'Y') {
      if (FileDateTime <= Status.LastDateTime)
        ShowNews = FALSE;
    } else {
      /* Display News Only If Changed is set to NO - that means show it on */
      /* every login *EXCEPT* if the caller has already called today so    */
      /* check the last date on and disable the news display if he has     */
      /* already been online today                                         */
      if (UsersData.LastDateOn == Status.JulianLogonDate)
        ShowNews = FALSE;
    }

showit:
    if (ShowNews) {
      newline();
      /* use NOALTERNATE because we already checked alternates */
      displayfile(Name,NOALTERNATE);
    }
  }

  confdetails(UPDATEUSER);

  if (! isset(&ConfFlags[JOINED],ConfNum)) {
    if (Status.CurConf.ViewMembers)
      if (! QuickLogon && (Status.User != SYSOP || ! UsersData.ExpertMode)) {
        freshline();
        Str[0] = NoChar;
        Str[1] = 0;
        inputfield(Str,TXT_VIEWCONFMEMBERS,1,YESNO|FIELDLEN|UPCASE|NEWLINE,NOHELP,mask_yesno);
        if (Str[0] == YesChar)
          listconfmembers();
      }

    if (PcbData.ScanBlts)
      checkbulletins();

    if (FullStats && UsersData.PackedFlags.HasMail) {
      // checkformail() sets SCANNED flag -- we want to know if it was
      // ALREADY set before coming into this routine, so save it below
      Scanned = (bool) isset(&ConfFlags[SCANNED],ConfNum);
      checkformail();   //lint !e534
      if (! Scanned)
        unsetbit(&ConfFlags[SCANNED],ConfNum);
    }

    if (Status.TerseMode ||
        (Status.CurSecLevel >= PcbData.UserLevels[SEC_Y] &&
          ! PcbData.DisableScan &&
          ! isset(&ConfFlags[SCANNED],ConfNum))) {
      Str[0] = YesChar;
      Str[1] = 0;
      inputfield(Str,TXT_SCANMSGBASE,8,YESNO|UPCASE|STACKED|NEWLINE|LFBEFORE,HLP_Y,mask_scan);
      scanmail(Str,FullStats);
    }
  }

  if (PcbData.ShowSettings && Status.CurSecLevel >= PcbData.UserLevels[SEC_V] && ! isset(&ConfFlags[JOINED],ConfNum))
    showuserstatus(FullStats ? 0 : -1);

  setbit(&ConfFlags[JOINED],ConfNum);
  loadcmds();
}


void LIBENTRY aliasnames(void) {
  bool UseAlias;
  char OldName[26];

  UseAlias = (bool) (Status.CurConf.AllowAliases && Status.UseAlias);

  if (Status.AllowAlias && UsersData.Alias[0] != 0) {
    strcpy(OldName,Status.DisplayName);
    getdisplaynames();
    if (strcmp(OldName,Status.DisplayName) != 0) {
      if (PcbData.Network) {
        if (PcbData.ShowAlias)
          hidealiasinusernet();
        displaypcbtext(TXT_HIDINGIDENTITY,LFBEFORE|NEWLINE);
      }
      redisplaystatusline();
      strcpy(Status.DisplayText,Status.DisplayName);
      displaypcbtext(TXT_CHANGEDNAMETO,NEWLINE|LFBEFORE|LOGIT);
      if (UseAlias)
        displaypcbtext(TXT_IDENTITYPROTECTED,NEWLINE|LFAFTER);
    }
  }
}


void LIBENTRY showconfjoined(unsigned short Num, pcbconftype *Conf, bool ShowJoined, bool ShowIntro) {
  bool        Save;
  unsigned    OldNum;
  pcbconftype OldConf;

  Save = (bool) (Status.CurConf.AllowAliases && Status.UseAlias);

  OldNum = Status.Conference;

  #ifdef PCB152
    if (Num != OldNum)
      chargetimeinconf();
  #endif

  Status.Conference = Num;

  if (Num != OldNum) {
    getconfrecord(OldNum,&OldConf);
    Save = (bool) (OldConf.AllowAliases && Status.UseAlias);
    if (Num == 0) {
      if (OldNum == 0xFFFF) {
        // 0xFFFF is used in the MSGREAD module to start an ALL-conf scan
        // because of that, we don't really want to show the caller, nor record
        // in the callers log, that 0xFFFF has been abandoned, we just need to
        // assign the Status.CurConf variable
        Status.CurConf = *Conf;
      } else {
        if (ShowJoined) {
          makeconfstr(Status.DisplayText,OldNum);
          stripright(Status.DisplayText,' ');
          displaypcbtext(TXT_CONFABANDONED,NEWLINE|LFBEFORE);
        }

        makeconfstrsys(Status.DisplayText,OldNum);
        stripright(Status.DisplayText,' ');
        logsystext(TXT_CONFABANDONED,SPACERIGHT);
        Status.CurConf = *Conf;   /* get conf info AFTER showing it abandoned */
      }
    } else {
      Status.CurConf = *Conf;   /* get conf info BEFORE showing it joined */

      if (ShowJoined) {
        makeconfstr(Status.DisplayText,Num);
        stripright(Status.DisplayText,' ');
        displaypcbtext(TXT_CONFJOINED,NEWLINE|LFBEFORE);
      }

      makeconfstrsys(Status.DisplayText,Num);
      stripright(Status.DisplayText,' ');
      logsystext(TXT_CONFJOINED,SPACERIGHT);
    }
  }

  if (Save != (Status.CurConf.AllowAliases && Status.UseAlias))
    aliasnames();

  redisplaystatusline();

  if (ShowIntro || PcbData.ForceIntro) {
    if (Status.CurConf.Intro[0] != 0 && fileexist(Status.CurConf.Intro) != 255) {
      displayfile(Status.CurConf.Intro,GRAPHICS|SECURITY|LANGUAGE);
      Display.WasAborted = FALSE;  // don't let an abort on the INTRO file prevent reading mail
    }
  }
}


static confnumtype _NEAR_ LIBENTRY confnumokay(int NumTokens, bool *ShowNews) {
  bool           NonDigit;
  unsigned short Num;
  char          *p;
  cnamesidxtype *q;
  bool           Search;
  char           Password[13];
  char           Name[80];
  char           Temp[80];
  pcbconftype    Conf;

  *ShowNews = TRUE;
  Search    = FALSE;
  Num       = 0xFFFF;
  Name[0]   = 0;
  NonDigit  = FALSE;

  for (; NumTokens; NumTokens--) {
    p = getnexttoken();
    if (p[0] == 0)
      continue;
    if (alldigits(p) && ! NonDigit) {
      Num = (unsigned short) atoi(p);
    } else {
      if (p[1] == 0) {
        switch(*p) {
          case 'Q': *ShowNews = FALSE; NonDigit = TRUE;  break;
          case 'S': Search = TRUE;     NonDigit = TRUE;  break;
          case 'J': /* just eat it */                    break;
          case 'R': return(CONFRELIST);
          default : /* rebuild the name in case of multi-word names - but watch size */
                    maxstrcpy(&Name[strlen(Name)],p,sizeof(Name)-strlen(Name)-1);
                    addchar(Name,' ');
                    NonDigit = TRUE;
                    break;
        }
      } else {
        /* rebuild the name in case of multi-word names - but watch size */
        maxstrcpy(&Name[strlen(Name)],p,sizeof(Name)-strlen(Name)-1);
        addchar(Name,' ');
      }
    }
  }

  stripright(Name,' ');
  if (Num == 0xFFFF && ! Search && Name[0] != 0) {
    if (strcmp(Name,"MAIN") == 0 || strcmp(Name,"MAIN BOARD") == 0)
      Num = 0;
    else {
      opencnamesidx(FALSE);
      Num = (unsigned short) getconfbyname(Name);
      closecnamesidx();
    }
  }

  if (Num == 0xFFFF && Search) {
    if (Name[0] == 0) {
      inputfield(Name,TXT_TEXTTOSCANFOR,sizeof(Conf.Name)-1,UPCASE|NEWLINE|LFBEFORE|HIGHASCII,HLP_J,mask_join);
      if (Name[0] == 0)
        return(CONFNUMOKAY);
    }
    fopencnamesidx();
    while ((q = findconfbystr(Name)) != NULL) {
      if (isregistered(ConfReg,q->Num)) {
        sprintf(Temp,"%5u) %s",q->Num,q->Name);
        println(Temp);
        if (Display.AbortPrintout)
          break;
      }
    }
    fclosecnamesidx();
    return(CONFSEARCH);
  }

  if (Status.Conference == Num)
    return(CONFNUMOKAY);

  if (Num > NUMCONFS) {
    stripright(Name,' ');
    maxstrcpy(Status.DisplayText,(Name[0] != 0 ? Name : p),sizeof(Status.DisplayText));
    displaypcbtext(TXT_INVALIDCONFNUM,NEWLINE|LFBEFORE|LOGIT);
    return(CONFNUMBAD);
  }

  getconfrecord(Num,&Conf);
  if (Conf.Name[0] == 0 || Conf.MsgFile[0] == 0) {
    ascii(Status.DisplayText,Num);
    displaypcbtext(TXT_INVALIDCONFNUM,NEWLINE|LFBEFORE|LOGIT);
    return(CONFNUMBAD);
  }

  if (isregistered(ConfReg,Num)) {
    showconfjoined(Num,&Conf,TRUE,*ShowNews);
    return(CONFNUMOKAY);
  } else {
    // if there is a conference password, AND the caller is not locked out
    // of the conference, then ask the caller for a password and if the
    // caller answers correctly, let the caller join the conference.
    stripright(Conf.Password,' ');
    if (Conf.Password[0] != 0 && ! lockedout(ConfReg,Num)) {
      Password[0] = 0;
      if (passwordokay(Conf.Password,Password,TXT_PWRDTOJOIN,TXT_WRONGPWRDENTERED,2)) {
        showconfjoined(Num,&Conf,TRUE,*ShowNews);
        setbit(&ConfReg[REG],Num);
        setbit(&ConfReg[CUR],Num);
        return(CONFNUMOKAY);
      }

      Status.PwrdFailureCount++;
      if (Status.PwrdFailureCount >= 4) {
        displaypcbtext(TXT_DENIEDWRONGPWRD,NEWLINE|LFBEFORE|LOGIT);
        loguseroff(ALOGOFF);
      }

      return(CONFNUMBAD);
    } else {
      ascii(Status.DisplayText,Num);
      displaypcbtext(TXT_NOTREGINCONF,NEWLINE|LFBEFORE|LOGIT);
      UsersData.Stats.NumNotReg++;
      return(CONFNUMBAD);
    }
  }
}


void LIBENTRY joinconfcommand(int NumTokens) {
  char Str[61];
  bool ShowNews;
  confnumtype RetVal;

  if (NUMCONFS == 0) {
    displaypcbtext(TXT_NOCONFAVAILABLE,NEWLINE|LFBEFORE);
    return;
  }

top:
  if (NumTokens == 0) {
    startdisplay(NOCHANGE);
    displayfile(PcbData.CnfMenu,GRAPHICS|SECURITY|LANGUAGE|RUNMENU|RUNPPL);
input:
    Str[0] = 0;
    inputfield(Str,TXT_JOINCONFNUM,sizeof(Str)-1,UPCASE|STACKED|NEWLINE|LFBEFORE|HIGHASCII,HLP_J,mask_join);
    if (Str[0] == 0)
      return;
    NumTokens = tokenize(Str);
  }

  if ((RetVal = confnumokay(NumTokens,&ShowNews)) == CONFNUMBAD || RetVal == CONFSEARCH)
    goto input;
  if (RetVal == CONFRELIST) {
    NumTokens = 0;
    goto top;
  }

  processjoin(ShowNews,(bool) !ShowNews,FALSE);
}


#pragma warn -par
void LIBENTRY abandon(int NumTokens) {
  char Str[5];

  Str[0] = '0';
  Str[1] = 0;
  joinconfcommand(tokenizestr(Str));
}
#pragma warn +par


static void _NEAR_ LIBENTRY printheader(char *Divider) {
  printcls();
  displaypcbtext(TXT_CONFHEADER1,NEWLINE);
  /* indent one space becase we didn't have run in the   */
  /* pcbtext file to edit the complete CONFHEADER2 entry */
  displaypcbtext(TXT_CONFHEADER2,NEWLINE);
  printcolor(PCB_WHITE);
  println(Divider);
  printdefcolor();
}


static void _NEAR_ LIBENTRY getflags(char *Str, char _FAR_ *Conf, unsigned Num, choosetype Flag) {
  switch (Flag) {
    case SCANONLY: Str[0] = (isset(&Conf[USR],Num) ? 'X' : ' ');
                   Str[1] = 0;
                   break;
    case ALLCONF : Str[0] = 0;
                   if (isset(&Conf[REG],Num)) {
                     addchar(Str,'R');
                     if (isset(&Conf[EXP],Num))
                       addchar(Str,'X');
                   } else if (isset(&Conf[EXP],Num))
                     addchar(Str,'L');
                   if (isset(&Conf[USR],Num))
                     addchar(Str,'S');
                   if (isset(&Conf[CON],Num))
                     addchar(Str,'C');
                   if (QwkSupport && isset(&Conf[NET],Num))
                     addchar(Str,'N');
                   break;
  }
}


static void _NEAR_ LIBENTRY printline(char _FAR_ *Conf, long _FAR_ *MsgPtr, conflinetype *Line, choosetype Flag) {
  char      Net;
  int       X;
  unsigned  Num;
  char     *p;
  char      Temp[5];
  char      Str[80];

  if (Line->Name[0] != 0) {
    printdefcolor();
    Num = Line->Num;
    Net = (QwkSupport && isset(&Conf[NET],Num) ? '*' : ' ');

    sprintf(Str,"%5u%c",Num,Net);
    print(Str);

    p = Line->Name;
    if ((X = strlen(p)) >= 52) {
      p += (X - 52);
      print(p);
    } else {
      print(p);
      X = 52 - X;
      printcolor(0x08);
      for (; X > 0; X--)
        print(X & 1 ? "." : " ");
    }

    printcolor(0x07);
    getflags(Temp,Conf,Num,Flag);
    sprintf(Str,"%7ld%7ld %-5s",MsgPtr[Num],Line->HighNum,Temp);
    stripright(Str,' ');
    print(Str);
  }
  newline();
}


void LIBENTRY chooseconferences(int NumTokens, char _FAR_ *Conf, long _FAR_ *MsgPtr, choosetype Flag) {
  bool            Done;
  bool            FoundOne;
  bool            SelectAll;
  bool            DeSelectAll;
  bool            DeSelect;
  bool            SkipPrint;
  unsigned        Begin;
  unsigned        End;
  unsigned        Start;
  unsigned        Stop;
  unsigned        X;
  unsigned        LineNum;
  unsigned        NumLines;
  char           *p;
  char           *q;
  msgbasestattype MsgStats;
  char            Input[10];
  char            Str[40];
  char            Divider[80];
  conflinetype   *LinePtr;
  conflinetype    Line[MAXCONFLINES];

  memset(Divider,'-',79); Divider[79] = 0;

  if (Display.PageLen < 4 || Display.PageLen > 50)
    NumLines = 19;
  else
    NumLines = Display.PageLen - 4;

  Str[0] = 0;
  Input[0] = 0;
  Begin = End = 0;
  Done = FALSE;
  FoundOne = FALSE;
  SkipPrint = FALSE;

  if (NumTokens != 0) {
    SkipPrint = TRUE;
    goto process;
  }

scan:
  // make sure that the activity thread (if there is one) is for sure stopped
  showactivity(ACTEND);

  while (! Done) {
    printheader(Divider);
    memset(Line,0,sizeof(Line));
    for (X = Begin, LineNum = 0, LinePtr = Line; X < NUMAREAS; X++) {
      if (Flag == SCANONLY && ! isregistered(Conf,X))
        continue;
      getconfrecord(X,&Status.CurConf);
      if (Status.CurConf.Name[0] == 0)
        continue;
      if (msgbasestats(&MsgStats) != -1) {
        FoundOne = TRUE;
        LinePtr->Num = X;
        maxstrcpy(LinePtr->Name,Status.CurConf.Name,sizeof(LinePtr->Name));
        LinePtr->HighNum = MsgStats.HighMsgNum;
        MsgPtr[X] = max(MsgPtr[X],0);
        MsgPtr[X] = min(MsgPtr[X],MsgStats.HighMsgNum);
        printline(Conf,MsgPtr,LinePtr,Flag);
        LineNum++;
        LinePtr++;
        if (LineNum >= NumLines)
          break;
      }
    }

    End = X;
    if (End > NUMAREAS)
      Done = TRUE;

    if (! FoundOne)
      break;

    for (; LineNum < NumLines; LineNum++)
      newline();

top:
    printcolor(PCB_WHITE);
    println(Divider);

    if (NumTokens == 0) {
      Str[0] = 0;
      if (Flag == SCANONLY)
        inputfield(Str,TXT_CONFNUMBERS,sizeof(Str)-1,ERASELINE|STACKED|UPCASE,HLP_SEL,mask_cnfnumbers);
      else
        inputfield(Str,TXT_CONFNUMBERS2,sizeof(Str)-1,ERASELINE|STACKED|UPCASE,HLP_REG,mask_cnfnumbers);
      NumTokens = tokenize(Str);
    }

process:
    if (NumTokens != 0) {
      if (Str[0] == 'Q')
        goto exit;

      if (Flag == ALLCONF) {
        Input[0] = 0;
        Status.DisplayText[0] = 0;
        inputfield(Input,TXT_SELECTCONFFLAGS,5,NEWLINE|FIELDLEN|GUIDE|UPCASE,NOHELP,mask_crsxn);
      }

      for (; NumTokens; NumTokens--) {
        SelectAll = FALSE;
        DeSelectAll = FALSE;
        DeSelect = FALSE;
        p = getnexttoken();
        if (*(p+1) == 0) {
          switch (*p) {
            case 'S': displaypcbtext(TXT_SELECTINGALL,DEFAULTS);
                      showactivity(ACTBEGIN);
                      SelectAll = TRUE;
                      Start = 0;
                      Stop = NUMAREAS;
                      break;
            case 'D': DeSelectAll = TRUE;
                      Start = 0;
                      Stop = NUMAREAS;
                      break;
            default : Start = Stop = atoi(p);
                      break;
          }
        } else {
          if (strchr(p,'D') != NULL)
            DeSelect = TRUE;
          if ((q = strchr(p,'-')) != NULL) {
            *q = 0;
            Start = atoi(p);
            Stop  = atoi(q+1);
          } else {
            Start = Stop = atoi(p);
          }
        }

        if (Stop > NUMAREAS)
          Stop = NUMAREAS;

        for (X = Start; X <= Stop; X++) {
          switch (Flag) {
            case SCANONLY: if (isregistered(Conf,X)) {
                             if (DeSelectAll || DeSelect || (! (SelectAll || SkipPrint) && isset(&Conf[USR],X)))
                               unsetbit(&Conf[USR],X);
                             else {
                               getconfrecord(X,&Status.CurConf);
                               if (msgbasestats(&MsgStats) != -1) {
                                 setbit(&Conf[USR],X);
                                 if (! SelectAll) {
                                   sprintf(Status.DisplayText,"%s %ld-%ld",Status.CurConf.Name,MsgStats.LowMsgNum,MsgStats.HighMsgNum);
                                   MsgPtr[X] = max(MsgPtr[X],0);
                                   MsgPtr[X] = min(MsgPtr[X],MsgStats.HighMsgNum);
                                   lascii(Input,MsgPtr[X]);
                                   if (! SkipPrint)
                                     inputfield(Input,TXT_SETLASTMSGREADPTR,sizeof(Input)-1,NEWLINE|FIELDLEN|GUIDE,NOHELP,mask_numbers);
                                   if (Input[0] != 0) {
                                     long Temp = atol(Input);
                                     MsgPtr[X] = min(Temp,MsgStats.HighMsgNum);
                                   }
                                 } else showactivity(ACTSHOW);
                               }
                             }
                           }
                           break;
            case ALLCONF : getconfrecord(X,&Status.CurConf);
                           if (strchr(Input,'L') != NULL) { /* LOCKED-OUT   */
                             unsetbit(&Conf[REG],X);        /* turn off REG */
                             setbit(&Conf[EXP],X);          /* turn on EXP  */
                           } else {
                             if (strchr(Input,'R') != NULL) setbit(&Conf[REG],X); else unsetbit(&Conf[REG],X);
                             if (strchr(Input,'X') != NULL) setbit(&Conf[EXP],X); else unsetbit(&Conf[EXP],X);
                             if (strchr(Input,'S') != NULL) setbit(&Conf[USR],X); else unsetbit(&Conf[USR],X);
                             /* don't check the CurSecLevel which might be */
                             /* updated for conference sysop access - but  */
                             /* instead check his own security level       */
                             if (UsersData.SecurityLevel >= PcbData.Sysop[SEC_SYSOPLEVEL]) {
                               if (strchr(Input,'C') != NULL) setbit(&Conf[CON],X); else unsetbit(&Conf[CON],X);
                             }
                             if (QwkSupport && strchr(Input,'N') != NULL) setbit(&Conf[NET],X); else unsetbit(&Conf[NET],X);
                           }
                           break;
          }

          checkstatus();
          if (Display.AbortPrintout) {
            checkdisplaystatus();
            Begin = 0;
            NumTokens = 0;
            goto scan;
          }
        }

      }

      // Let's see if we can reposition the 'window' of displayable
      // conferences to include those that have just been affected

      if (! SkipPrint) {
        if (SelectAll || DeSelectAll) {
          Begin = 0;
          NumTokens = 0;
          goto scan;
        }
        if (Start < Begin || Start > End) {
          if (isregistered(Conf,Start)) {
            Begin = Start;
            NumTokens = 0;
            goto scan;
          }
          for (Begin = Stop, X = 0; Begin > 0; ) {
            if (isregistered(Conf,Begin))
              if (++X >= NumLines)
                break;
            Begin--;
          }
          NumTokens = 0;
          goto scan;
        }
        if (Stop > End) {
          for (Begin = Stop, X = 0; Begin > 0; ) {
            if (isregistered(Conf,Begin))
              if (++X >= NumLines)
                break;
            Begin--;
          }
          NumTokens = 0;
          goto scan;
        }
      }

      if (SelectAll)
        showactivity(ACTEND);
      if (SkipPrint)
        goto exit;
      printheader(Divider);
      for (X = 0, LinePtr = Line; X < NumLines; X++, LinePtr++)
        printline(Conf,MsgPtr,LinePtr,Flag);
      goto top;
    }

    Begin = End + 1;
    if (Begin > NUMAREAS)
      Done = TRUE;
  }

exit:
  getconfrecord(Status.Conference,&Status.CurConf);
}


void LIBENTRY selectconf(int NumTokens) {
  chooseconferences(NumTokens,ConfReg,MsgReadPtr,SCANONLY);
}


#if defined(PCBCOMM) && defined(COMM)
void LIBENTRY tersemodeconfinfo(unsigned ConfNum) {
  long            LastMsgRead;
  msgbasestattype MsgStats;
  char            Str[80];

  if (ConfNum > NUMCONFS || ! isregistered(ConfReg,ConfNum)) {
    sprintf(Str,"[%5u%38s]",ConfNum,"");
  } else {
    getconfrecord(ConfNum,&Status.CurConf);
    if (Status.CurConf.Name[0] == 0 || msgbasestats(&MsgStats) == -1) {
      sprintf(Str,"[%5u%38s]",ConfNum,"");
    } else {
      LastMsgRead = fixlmrpointer(ConfNum,MsgStats.LowMsgNum,MsgStats.HighMsgNum);
      sprintf(Str,"[%5u%-13s%8ld%8ld%8ld%c]",
              ConfNum,
              Status.CurConf.Name,
              MsgStats.LowMsgNum,
              MsgStats.HighMsgNum,
              LastMsgRead,
              isset(&ConfReg[USR],ConfNum) ? 'Y' : 'N');
    }
  }

  tersemodechecksum(Str,43);
  getconfrecord(Status.Conference,&Status.CurConf);
}
#endif
