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


#ifdef COMM

#include "project.h"
#pragma hdrstop

#ifndef LIB
static void _NEAR_ LIBENTRY showpaging(void) {
  pcbtexttype Buf;

  getpcbtext(TXT_SPACETOACKNOWLEDGE,&Buf);
  boxcls(0,0,79,2,ScrnColors->BoxColor,SINGLE);
  fastcenter(1,Buf.Str,ScrnColors->BoxColor);
}


static char * _NEAR_ LIBENTRY chatmessage(int TextNum) {
  pcbtexttype static Buf;
  char Str[80];

  newline();
  timestr2(Str);
  strcpy(Status.DisplayText,Str);
  Status.AppendText = TRUE;

  #if defined(COMM) && ! defined(LIB)
    #ifdef PCBCOMM2
    if (Status.TerseMode) {
      displaypcbtext(TextNum,NEWLINE|LOGIT);
      Buf.Str[0] = 0;
    } else {
    #endif
      logsystext(TextNum,SPACERIGHT);
      strcpy(Status.DisplayText,Str);
      Status.AppendText = TRUE;
      getpcbtext(TextNum,&Buf);
    #ifdef PCBCOMM2
    }
    #endif
  #else
    logsystext(TextNum,SPACERIGHT);
    strcpy(Status.DisplayText,Str);
    Status.AppendText = TRUE;
    getpcbtext(TextNum,&Buf);
  #endif
  return(Buf.Str);
}
#endif


static void _NEAR_ LIBENTRY chat(void) {
  char *Space;
  int  Key;
  int  Len;
  char Str[2];
  char Buf[100];
  char Xlat[256];

  #ifdef PCBCOMM2
    onlinetype SaveOnline;
    onlinetype LocalInTerse;
    SaveOnline = Asy.Online;
    LocalInTerse = (Status.TerseMode ? LOCAL : Asy.Online);
  #endif

  memset(Xlat,0,K_ESC);                 /* default first bytes to 0 */
  for (Len = K_ESC; Len <= 255; Len++)  /* set up translate table to allow ALL chars */
    Xlat[Len] = (char) Len;

  Xlat[CTRL_H] = CTRL_H;   /* don't forget backspaces */
  Xlat[CTRL_M] = CTRL_M;   /* don't forget carriage returns */
  Xlat[127] = CTRL_H;      /* change ascii 127 to an ascii 8 */

  if (! PcbData.DisableFilter) {
    memset(&Xlat[128],0,168-128+1);  /* then filter out the garbage */
    memset(&Xlat[224],0,246-224+1);
    memset(&Xlat[251],0,253-251+1);
  }

  Len = awherex();
  memset(Buf,' ',Len);
  Buf[Len] = 0;

  /* allow carrier loss detection to occur inside of chat so that, upon exit */
  /* the time credit can still be posted */
  #ifdef COMM
    Asy.IgnoreCDLoss = TRUE;
  #endif

  while (1) {
    Len = awherex();

    /* NOTE:  because the sysop COULD press F10 as the caller was logging */
    /* off - and during the display of the LOGOFF file - there's a chance */
    /* that we'll be IGNORING CD LOSS at this point so we need to check   */
    /* for CD loss manually in this routine                               */

    #ifdef PCBCOMM2
      Asy.Online = SaveOnline;
    #endif

    #ifdef __OS2__
      updatelinesnow();
      Key = waitforkey();
    #else
      Key = cinkey();
    #endif

    if (Key == -1) {
      Display.AbortPrintout = TRUE;
      return;
    }

    #ifdef PCBCOMM2
      Asy.Online = (Status.Kbd ? SaveOnline : LocalInTerse);
    #endif

    if (Key > 255)   /* filter out local extended keys (like Insert) */
      continue;

    Key = Xlat[Key];
    switch (Key) {
      case      0: continue;
      case CTRL_H: if (Len > 0) {
                     backupdestructive(1);
                     Buf[Len-1] = 0;
                   }
                   continue;
      case CTRL_M: print("\r\n");
                   Buf[0] = 0;
                   continue;
      case K_ESC : if (Status.Kbd) {
                     #ifdef PCBCOMM2
                       Asy.Online = SaveOnline;
                     #endif
                     return;
                   } else
                     continue;
      default    : Str[0] = (char) Key;
                   Str[1] = 0;
                   break;
    }

    if (Status.Kbd)
      printcolor(PCB_GREEN);
    else
      printdefcolor();

    if (Len >= 78) {
      if (Key == ' ') {
        print("\r\n");
        Buf[0] = 0;
        continue;
      }

      // find the last space on the line
      if ((Space = strrchr(Buf,' ')) != NULL) {
        int BufLen = strlen(Buf);
        int LenToSpace = (int) (Space - Buf);
        // back up the number of spaces from the right side to the last space
        backupcleareol(BufLen - LenToSpace);
        print("\r\n");
        strcpy(Buf,Space+1);
        print(Buf);
      } else {
        print("\r\n");
        Buf[0] = 0;
      }
    }

    print(Str);
    addchar(Buf,(char) Key);
  }
}

#ifdef LIB
void LIBENTRY sysopchat(void) {
#else
void LIBENTRY sysopchat(int NumTokens) {  /* ignore NumTokens */
  char Answer[2];
  int  X;
  bool CaptureOn;
#endif
  long StartTime;
  long StopTime;
  bool ScreenOff;

  /* NOTE: the NumTokens variable will NOT be used in the normal manner here */
  /* instead a value of 0 means that the user typed "O" for an operator chat */
  /* while a value of 1 means that the system initiated the chat.  The need  */
  /* for the differentiation - is that the user causes the sysop to be paged */

  if (Status.InChat || Asy.Online == OFFLINE)
    return;

  if (! Display.ShowOnScreen) {
    ScreenOff = TRUE;
    Display.ShowOnScreen = TRUE;
  } else
    ScreenOff = FALSE;

  Status.InChat = TRUE;

#ifndef LIB
  if (Status.Capture) {
    CaptureOn = TRUE;
    setcapture(FALSE);
  } else
    CaptureOn = FALSE;

  if (NumTokens == 0) {
    UsersData.Stats.NumSysopPages++;
    if (! Status.PageBell || ! timeinrange(PcbData.SysopStart,PcbData.SysopStop)) {
      displaypcbtext(TXT_SYSOPUNAVAILABLE,NEWLINE|LFBEFORE|LOGIT);
ask:
      Status.InChat = FALSE;
      #ifndef LIB
        usernetavailable();
      #endif
      Status.Paged  = TRUE;
      redisplaystatusline();
      if (Status.CurSecLevel >= PcbData.UserLevels[SEC_C]) {
        Answer[0] = NoChar;
        Answer[1] = 0;
        inputfield(Answer,TXT_COMMENTINSTEAD,1,YESNO|UPCASE|NEWLINE|LFBEFORE|LFAFTER|FIELDLEN,NOHELP,mask_yesno);
        if (Answer[0] == YesChar)
          entercomment(-1);    /* -1 will avoid asking to leave a comment */
      }
      goto exit;
    }

    displaypcbtext(TXT_PAGING,LFBEFORE);
    #ifndef LIB
      writeusernetstatus(PAGINGSYSOP,NULL);
    #endif
    showpaging();
    for (X = 15; X; X--) {
      print(".");
      settimer(4,TWOSECONDS);
      mysound(1000,30);
      while (! timerexpired(4)) {
        switch (cinkey()) {
          case  0  : giveup();
                     break;
          case 11  : chatmessage(TXT_OPERATORPAGED);  /*lint !e534 */
                     #ifndef LIB
                       usernetavailable();
                     #endif
                     Status.Paged  = TRUE;
                     redisplaystatusline();
                     Status.InChat = FALSE;
                     goto exit;
          case 32  :
          case 1068: if (Status.Kbd) goto chat;
        }
      }
    }
    newline();
    displaypcbtext(TXT_SYSOPUNAVAILABLE,LFBEFORE);
    chatmessage(TXT_OPERATORPAGED);         /*lint !e534 */
    goto ask;
  }
#endif

chat:
  #ifndef LIB
    writeusernetstatus(CHATWITHSYSOP,NULL);
  #endif
  Control.WatchKbdClock     = FALSE;
  Control.WatchSessionClock = FALSE;
  #ifndef LIB
    if (Status.FullScreen)
      printcls();
    else
      newline();

    Status.Paged  = FALSE;
  #else
    newline();
  #endif

  printcolor(PCB_WHITE);

  #ifndef LIB
    printxlated(chatmessage(TXT_SYSOPCHATACTIVE));
    strcpy(Status.DisplayText,Status.SysopName);
  #else
    strcpy(Status.DisplayText,"Sysop");
  #endif

  newline();
  displaypcbtext(TXT_HELLOTHISIS,LFBEFORE);
  redisplaystatusline();

  StartTime = gettimer(6);
  chat();
  StopTime = gettimer(6);
  #ifdef __OS2__
    addtime((-StopTime+StartTime)/ONESECOND,CREDTIME);
  #else
    addtime(((-StopTime+StartTime)*10)/182,CREDTIME);
  #endif

  #ifndef LIB
    Display.NumLinesPrinted = 0;
    printxlated(chatmessage(TXT_SYSOPCHATENDED));
    newline();
  #endif

  Status.InChat = FALSE;
  turnkbdtimeron();
  Control.WatchSessionClock = TRUE;

exit:
  if (ScreenOff)
    Display.ShowOnScreen = FALSE;

#ifndef LIB
  usernetavailable();
  if (CaptureOn)
    setcapture(TRUE);
#endif

  /* restore the CD loss detection */
  #ifdef COMM
    Asy.IgnoreCDLoss = FALSE;
  #endif
}
#endif
