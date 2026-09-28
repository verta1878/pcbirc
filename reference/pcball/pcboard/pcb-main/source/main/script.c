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

#include <ctype.h>

#ifdef __WATCOMC__
  #include <io.h>
  #include <borland.h>
#endif

#include "newscr.h"

typedef struct {
  char QstFile[30];
  char AnsFile[30];
} scriptlisttype;

typedef enum {QUESTIONNAIRE,LOGONSCRIPT,LOGOFFSCRIPT,NEWREG} scripttype;

extern struct ffblk DTA;  /* declared in exist.c */

long static StartTime;
long static StopTime;


void LIBENTRY removecodes(char *Str) {
  char *p;

  while ((p = strchr(Str,'@')) != NULL) {
    if (p[1] == 'X' && isxdigit(p[2]) && isxdigit(p[3])) {
      memcpy(p,p+4,strlen(p+4)+1);
      Str = p;
    } else Str = p + 1;
  }
}


static void _NEAR_ LIBENTRY script(int ScriptNum, char *QFile, char *AFile, scripttype Type) {
  bool    Save;
  bool    BlankTextLine;
  bool    HasQuestions;
  int     Line;
  long    Size;
  char    Time[6];
  char    Date[9];
  DOSFILE Out;
  DOSFILE In;
  char    TempFile[15];
  char    AStr[61];
  char    Divider[65];
  char    QStr[2049];
  char    Temp[2049];

  TempFile[0] = 0;

  strupr(QFile);
  if (strstr(QFile,".PPE") != NULL) {
    if (*AFile == 0)
      AFile = NULL;
    else {
      StartTime = gettimer(6);
      Control.WatchSessionClock = FALSE;
    }
    writeusernetstatus(ANSWERSCRIPT,QFile);
    #ifdef COMM
      Asy.IgnoreCDLoss = FALSE;  // for PPE's, leave CD monitoring enabled
    #endif
    if (doScript(QFile,AFile,0) == 1) {
      switch (Type) {
        case LOGOFFSCRIPT : displaypcbtext(TXT_LOGOFFINFOSAVED,NEWLINE|LFBEFORE|LOGIT);
                            break;
        case QUESTIONNAIRE: ascii(Status.DisplayText,ScriptNum);
                            displaypcbtext(TXT_INFOSAVED,NEWLINE|LFBEFORE|LOGIT);
                            break;
      }
    }
    if (AFile != NULL) {
      StopTime = gettimer(6);
      #ifdef __OS2__
        addtime((-StopTime+StartTime)/ONESECOND,CREDTIME);
      #else
        addtime(((-StopTime+StartTime)*10)/182,CREDTIME);
      #endif
      Control.WatchSessionClock = TRUE;
    }
    usernetavailable();
    return;
  }

  if (dosfopen(QFile,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    return;

  HasQuestions = FALSE;
  Save         = FALSE;
  Size         = 0;
  AStr[0]      = NoChar;
  AStr[1]      = 0;

  if (AFile[0] != 0) {
    Save = TRUE;
    sprintf(TempFile,"TMP%d.%d",PcbData.NodeNum,ScriptNum);
    if (dosfopen(TempFile,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1) {
      dosfclose(&In);
      return;
    }
  }

  /* display header and ask if they wish to answer this questionnaire */
  if (Type == QUESTIONNAIRE) {
    newline();
    printcolor(PCB_WHITE);
    for (Line = 0; Line < 5; Line++) {
      if (dosfgets(QStr,sizeof(QStr),&In) == -1)
        break;
      if (QStr[0] == '%' && checkenvfileexist(&QStr[1],sizeof(QStr)) != 255)
        displayfile(&QStr[1],GRAPHICS|SECURITY|LANGUAGE);
      else if (QStr[0] == '!' && QStr[1] != '|')
        runscriptwithparams(&QStr[1]);      //lint !e534
      else {
        printxlated(QStr);
        newline();
      }
    }
    inputfield(AStr,TXT_COMPLETEQUESTION,1,YESNO|NEWLINE|FIELDLEN|UPCASE|LFBEFORE,NOHELP,mask_yesno);
  }

  if (AStr[0] == YesChar || Type != QUESTIONNAIRE) {
    StartTime = gettimer(6);
    Control.WatchSessionClock = FALSE;
    newline();
    Display.CountLines = FALSE;

    if (Save) {
      sprintf(QStr,"(%u) => #%d",Status.Conference,ScriptNum);
      writeusernetstatus(ANSWERSCRIPT,QStr);
      Status.EnteringMessage = TRUE;
      memset(Divider,'*',62);
      Divider[62] = 0;
      sprintf(Temp,"%s\r\n",Divider);
      dosfputs(Temp,&Out);   //lint !e534
      sprintf(Temp,"From: %s, %s (%s) Node %d Sec %d Exp %s\r\n",Status.DisplayName,countrydate(datestr(Date)),timestr2(Time),PcbData.NodeNum,UsersData.SecurityLevel,countrydate(juliantodate(UsersData.RegExpDate)));
      dosfputs(Temp,&Out);   //lint !e534
    }

    BlankTextLine = TRUE;
    while (dosfgets(QStr,sizeof(QStr),&In) != -1) {
      /* if we're saving to disk and the string doesn't start */
      /* with ";" then it's a question that needs to be asked */
      if (Save && QStr[0] != ';') {
        HasQuestions = TRUE;
        if (! BlankTextLine)
          newline();
        if (QStr[0] == '%' && checkenvfileexist(&QStr[1],sizeof(QStr)) != 255) {
          displayfile(&QStr[1],GRAPHICS|SECURITY|LANGUAGE);
          continue;  /* don't write out the Q/A section */
        } else if (QStr[0] == '!' && QStr[1] != '|') {
          runscriptwithparams(&QStr[1]);    //lint !e534
          continue;  /* don't write out the Q/A section */
        } else while (1) {
          printcolor(PCB_YELLOW);
          printxlated(QStr);
          newline();
          AStr[0] = 0;
          inputfieldstr(AStr,"",Display.DefaultColor,60,FIELDLEN|GUIDE|NEWLINE|LFAFTER|HIGHASCII,NOHELP,mask_alphanum);
          if (AStr[0] != 0 || Type != NEWREG)
            break;
          if (Status.KbdTimedOut)
            goto done;
          displaypcbtext(TXT_RESPONSEREQUIRED,NEWLINE|LFAFTER);
        }
        BlankTextLine = TRUE;  /* we did a LFAFTER */
        if (Save) {
          removecodes(QStr);
          sprintf(Temp,"Q: %s\r\n",QStr);
          dosfputs(Temp,&Out);              //lint !e534
          sprintf(Temp,"A: %s\r\n",AStr);
          dosfputs(Temp,&Out);              //lint !e534
        }
      } else {
        if (QStr[0] == ';') {      /* for compatibility set it to WHITE if */
          printcolor(PCB_WHITE);   /* the first character is a semicolon   */
          printxlated(&QStr[1]);
          BlankTextLine = (bool) (QStr[1] == 0);
          newline();
        } else {
          if (QStr[0] == '%') {
            displayfile(&QStr[1],GRAPHICS|SECURITY|LANGUAGE);
            #ifdef COMM
              Size += 1024;
            #endif
          } else if (QStr[0] == '!' && QStr[1] != '|') {
            runscriptwithparams(&QStr[1]);       //lint !e534
            #ifdef COMM
              Size += 1024;
            #endif
          } else {
            if (QStr[0] == '@' && memcmp(QStr,"@HANGUP@",8) == 0) {
              Status.AutoLogoff = TRUE;
              loguseroff(NLOGOFF);
            }

            printxlated(QStr);
            BlankTextLine = (bool) (QStr[0] == 0);
            newline();
          }
        }
        #ifdef COMM
          Size += strlen(QStr) + 2;
          /* if we're logging off then we've turned off automatic checking of */
          /* carrier loss and keyboard timeout so manually check them here    */
          #ifdef __OS2__
          if (Asy.LostCarrier || Status.KbdTimedOut) {
          #else
          if (Asy.LostCarrier || (Control.KbdTimer != 0 && timerexpired(1))) {
          #endif
            Display.AbortPrintout = TRUE;
            break;
          }
        #endif
      }
    }

done:
    switch (Type) {
      case LOGOFFSCRIPT :
                          if (HasQuestions)
                            displaypcbtext(TXT_LOGOFFINFOSAVED,NEWLINE|LFBEFORE|LOGIT);
                          else
                            Status.LogoffFile = Size;   /* this will slow down the logoff procedure for MODEM BUFFER effects */
                          break;
      case QUESTIONNAIRE: ascii(Status.DisplayText,ScriptNum);
                          displaypcbtext(TXT_INFOSAVED,NEWLINE|LFBEFORE|LOGIT);
                          break;
    }

    if (Save) {
      dosfclose(&Out);
      if (HasQuestions) {
        if (fileexist(AFile) == 255)
          pcbcopyfile(TempFile,AFile,TRUE,FALSE,NULL,TRUE);     //lint !e534
        else
          appendfile(TempFile,AFile,TRUE);                      //lint !e534
      }
    }

    StopTime = gettimer(6);
    #ifdef __OS2__
      addtime((-StopTime+StartTime)/ONESECOND,CREDTIME);
    #else
      addtime(((-StopTime+StartTime)*10)/182,CREDTIME);
    #endif
    Control.WatchSessionClock = TRUE;
  } else
    dosfclose(&Out);

  if (TempFile[0] != 0)
    unlink(TempFile);

  dosfclose(&In);

  if (Save) {
    Status.EnteringMessage = FALSE;
    usernetavailable();
  }

  if (Status.KbdTimedOut)
    loguseroff(ALOGOFF);
}


static int _NEAR_ LIBENTRY launchquestionnaire(int ScriptNum, char *ScriptList) {
  int  File;
  char QFile[31];
  char AFile[31];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if ((File = dosopen(ScriptList,OPEN_READ|OPEN_DENYNONE POS2ERROR)) == -1)
    return(-1);

  doslseek(File,((long) ScriptNum-1)*sizeof(scriptlisttype),SEEK_SET);
  if (readcheck(File,QFile,sizeof(QFile)-1) == (unsigned) -1 ||
      readcheck(File,AFile,sizeof(QFile)-1) == (unsigned) -1) {
    dosclose(File);
    return(-1);
  }
  dosclose(File);

  QFile[30] = AFile[30] = 0;
  stripright(AFile,' ');
  stripleft(QFile,'!');
  getalternatename(QFile,GRAPHICS|LANGUAGE,0);
  if (QFile[0] == 0)
    return(-1);

  script(ScriptNum,QFile,AFile,QUESTIONNAIRE);
  return(0);
}


/********************************************************************
*
*  Function:  doquestionnaire()
*
*  Desc    :  Runs a script questionnaire at the request of a script or menu.
*
*  Returns :   0 the questionnaire was run
*             -1 the questionnaire list could not be read
*             -2 the questionnaire does not exist or was invalid
*             -3 insufficient security to view bulletins
*/

int LIBENTRY doquestionnaire(int Qnum) {
  int  NumScripts;
  char ScriptList[66];

  if (seclevelokay("S",PcbData.UserLevels[SEC_S])) {
    if (checkforalternatelist(ScriptList,Status.CurConf.ScrNameLoc,sizeof(scriptlisttype)) == -1 ||
        fileexist(Status.CurConf.ScrMenu) == 255)
      return(-1);

    fileexist(ScriptList);   //lint !e534
    if ((NumScripts = (int) (DTA.ff_fsize / sizeof(scriptlisttype))) == 0 ||
        Qnum > NumScripts)
      return(-1);

    return(launchquestionnaire(Qnum,ScriptList));
  }
  return(-3);
}


#pragma warn -par
void LIBENTRY scriptmenu(int NumTokens) {
  int  X;
  int  NumScripts;
  char Str[5];
  char ScriptList[66];

  if (checkforalternatelist(ScriptList,Status.CurConf.ScrNameLoc,sizeof(scriptlisttype)) == -1 ||
      fileexist(Status.CurConf.ScrMenu) == 255) {
    displaypcbtext(TXT_NOQUESTIONNAIRES,NEWLINE|LFBEFORE|BELL);
    return;
  }

  fileexist(ScriptList);     //lint !e534
  if ((NumScripts = (int) (DTA.ff_fsize / sizeof(scriptlisttype))) == 0) {
    displaypcbtext(TXT_NOQUESTIONNAIRES,NEWLINE|LFBEFORE|BELL);
    return;
  }

  startdisplay(NOCHANGE);
  displayfile(Status.CurConf.ScrMenu,GRAPHICS|LANGUAGE|RUNMENU|RUNPPL);

retry:
  X = (NumScripts > 9 ? (NumScripts > 99 ? 3 : 2) : 1);
  Str[0] = 0;
  inputfield(Str,TXT_QNUMTOANSWER,X,NEWLINE|LFBEFORE,NOHELP,mask_numbers);
  if (Str[0] == 0)
    return;
  X = atoi(Str);

  if (X <= 0 || X > NumScripts) {
    displaypcbtext(TXT_INVALIDSELECTION,NEWLINE);
    goto retry;
  }

  launchquestionnaire(X,ScriptList);   //lint !e534
}
#pragma warn +par


void LIBENTRY logoffscript(void) {
  char Name[66];

  maxstrcpy(Name,PcbData.LogOffScr,sizeof(Name));
  getalternatename(Name,GRAPHICS|SECURITY|LANGUAGE,0);
  if (Name[0] == 0)
    return;

  #ifdef COMM
    Asy.IgnoreCDLoss = TRUE;
  #endif
  script(0,Name,PcbData.LogOffAns,LOGOFFSCRIPT);
}


void LIBENTRY logonscript(void) {
  char Name[66];

  maxstrcpy(Name,PcbData.LogOnScr,sizeof(Name));
  stripleft(Name,'!');
  getalternatename(Name,GRAPHICS|SECURITY|LANGUAGE,0);
  if (Name[0] == 0)
    return;

  script(0,Name,PcbData.LogOnAns,LOGONSCRIPT);
}


void LIBENTRY newregscript(void) {
  char Name[66];

  maxstrcpy(Name,PcbData.RegFile,sizeof(Name));
  stripleft(Name,'!');
  getalternatename(Name,GRAPHICS|LANGUAGE,0);
  if (PcbData.RegFile[0] == 0)
    return;

  script(0,Name,PcbData.AnsFile,NEWREG);
}
