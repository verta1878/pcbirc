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

#include <math.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <borland.h>
  #include <malloc.h>
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#ifdef LIB
  #include "usersys.h"
#endif

extern countrytype SystemCountry;

#ifdef LIB
  extern char DoorName[20];
#endif

#define BLACK           0
#define LIGHTGRAY       7
#define STATUS_COLOR    BLACK + (LIGHTGRAY << 4)

#ifdef __OS2__
/* under OS/2, we don't have memory values on the status line so push the */
/* max file handle number to the right 4 spaces */
  const int Column = 64;
#else
  const int Column = 60;
#endif

#ifdef PCB153
/* this function is used to limit the size of the comma delimited string to */
/* just 12 characters, so even if the number is greater than 12 digits */
/* including the characters) it can still be represented on screen using either */
/* a 'K' or an 'M' (for i.e. 1K or 1M) as an abbreviation. */
/*          10        20 */
/* 1,345,789,123,567,901 */
static char * _NEAR_ LIBENTRY dlimcomma(char *Str, double Num) {
  int  Len;
  char Temp[20];

  Num = ceil(Num);
  dcomma(Temp,Num);
  Len = strlen(Temp);

  if (Len >= 17) {
    Num = ceil(Num/(1024.0*1024.0));
    dcomma(Str,Num);
    addchar(Str,'M');
  } else if (Len >= 13) {
    Num = ceil(Num/1024.0);
    dcomma(Str,Num);
    addchar(Str,'K');
  } else
    strcpy(Str,Temp);

  return(Str);
}
#else
#define dlimcomma(Str,Num) comma(Str,Num)
#endif

#ifdef COMM
static void _NEAR_ LIBENTRY modemstatus1(char *Str) {
  sprintf(Str,"PC-Modem:%6ld Overrun:%3u Parity  :%3u  InBuf:%5u  OutBuf:%5u  CTS:%s",
              Asy.ModemSpeed,OverrunErrors,ParityErrors,InBytes,OutBytes,(CTSokay ? "up" : "dn"));
}

static void _NEAR_ LIBENTRY modemstatus2(char *Str) {
  char *p;

  p = "NONE";

  #ifndef __OS2__
    if (PcbData.ModemPort[0] == 'C') {
      if (B16550A) {
        if (B16650)
          p = "16650 FIFO-32";
        else
          p = "16550A FIFO";
      } else if (B16550)
        p = "16550 FIFO";
      else if (B8250)
        p = "8250 (old)";
      else
        p = "8250A/16450";
    }
  #endif
  #ifdef MULTIPORT
    if (PcbData.ModemPort[0] == 'P') {
      #ifdef __OS2__
        p = "OS/2";
      #else
        if (strchr(PcbData.ModemPort,'F') != NULL)
          p = "FOSSIL";
        else if (PcbData.OS2Driver)
          p = "OS/2";
        else
          p = "COMM-DRV";
      #endif
    }
  #endif

  sprintf(Str,"Carrier :%6ld Framing:%3u ErrCorr :%3s  %s",
          Asy.CarrierSpeed,FramingErrors,(Asy.ErrorCorrected ? "YES" : "NO "),p);
}
#endif

void LIBENTRY showstatusline(void) {
  #ifdef COMM
    int  Loc;
    char Num[11];
    char Tmp1[20];
    char Tmp2[20];
    char Tmp3[20];
  #endif
  char Str[80];
  char Tmp4[20];
  #ifdef COMM
    countrytype Save;
    pcbtexttype Buf;
  #endif

  if (Status.StatLine == NONE)
    return;

  #ifndef LIB
    if (LocalOn) {
      if (Status.StatLine != Status.OldStatLine) {
        clsbox(0,Status.StatusLine2,79,Status.StatusLine2,STATUS_COLOR);
        fastprint(0,Status.StatusLine2,Status.Version,STATUS_COLOR);
        if (Status.CapFile.handle != 0)
          fastprint(Column,Status.StatusLine2,"F",STATUS_COLOR);
        if (Status.Printer)
          fastprint(Column+1,Status.StatusLine2,"P",STATUS_COLOR);
        #ifndef __OS2__
          sprintf(Str,"%3dK",(int) (coreleft()/1024));
          fastprint(65,Status.StatusLine2,Str,ScrnColors->MemColor);
        #endif
        settimer(2,0);
      }
      if (timerexpired(2)) {
        fastprint(74,Status.StatusLine2,timestr2(Tmp4),STATUS_COLOR);
        settimer(2,THIRTYSECONDS);
      }
      Status.OldStatLine = Status.StatLine;
      return;
    }
  #endif

  #ifdef COMM
    if (Status.StatLine == Status.OldStatLine) {
      if (Status.StatLine != NAME && timerexpired(2)) {
        Status.StatLine = Status.OldStatLine = NAME;
      } else goto update;
    } else
      Status.OldStatLine = Status.StatLine;

    clsbox(0,Status.StatusLine1,79,Status.StatusLine2,STATUS_COLOR);
    if (PcbData.Network) {
      ascii(Num,PcbData.NodeNum);
      fastprint(0,Status.StatusLine1,Num,ScrnColors->MemColor);
    }

    switch (Status.StatLine) {
      case NAME      : if (Asy.Online == LOCAL)
                         strcpy(Tmp1,"Local");
                       else {
                         lascii(Tmp1,Asy.CarrierSpeed);
                         if (Asy.ErrorCorrected)
                           addchar(Tmp1,'E');
                       }
                       Loc = (PcbData.Network ? strlen(Num) : 2);  /*lint !e645  Num is initialized of PcbData.Network is TRUE */
                       if (Status.UserRecNo == -1) {
                         sprintf(Str,"(%s)",Tmp1);
                         fastprint(Loc,Status.StatusLine1,Str,STATUS_COLOR);
                       } else {
                         sprintf(Str,"(%s) %s - %s",Tmp1,Status.DisplayName,usercity());
                         fastprint(Loc,Status.StatusLine1,Str,STATUS_COLOR);
                         #ifndef LIB
/*                         fastprint(54,Status.StatusLine1,"ALT-H=Help",STATUS_COLOR); */
                           fastprint(59,Status.StatusLine1,"ALT-H=Help",STATUS_COLOR);
                         #else
                           fastprint(64-strlen(DoorName),Status.StatusLine1,DoorName,STATUS_COLOR);
                         #endif
                         Save = Country;
                         Country = SystemCountry;
                         strcpy(Tmp1,Status.LastDateOnStr);
                         sprintf(Str,"%c (%s)  Sec(%u)=%d  Times On=%u  Up:Dn=%u:%u",
                                 (Control.GraphicsMode ? (Control.RipMode ? 'R' : 'G') : UseAnsi ? 'A' : 'N'),
                                 countrydate(Tmp1),
                                 Status.Conference,
                                 Status.CurSecLevel,
                                 UsersData.NumTimesOn,
                                 UsersData.NumUploads,
                                 UsersData.NumDownloads);
                         Country = Save;
                         #ifndef LIB
                         if (Status.PrintLog || Status.Alarm || Status.PageBell) {
                         #else
                         if (Status.PrintLog || Status.Alarm) {
                         #endif
                           #ifndef LIB
                             sprintf(Tmp1," (%s%s%s)",(Status.PrintLog ? "P" : ""),(Status.Alarm ? "A" : ""),(Status.PageBell ? "B" : ""));
                           #else
                             sprintf(Tmp1," (%s%s)",(Status.PrintLog ? "P" : ""),(Status.Alarm ? "A" : ""));
                           #endif
                           /* only show flags if they fit on screen before the File Handles */
                           if (strlen(Str)+strlen(Tmp1) < Column) {
                             strcat(Str,Tmp1);
                           } else {
                             /* didn't fit, try to make it fit */
                             substitute(Str,"  "," ",sizeof(Str));
                             if (strlen(Str)+strlen(Tmp1) < Column) {
                               strcat(Str,Tmp1);
                             } else {
                               /* still didn't fit, try harder! */
                               substitute(Str,"Times On","On",sizeof(Str));
                               if (strlen(Str)+strlen(Tmp1) < Column)
                                 strcat(Str,Tmp1);
                             }
                           }
                         }
                         fastprint(0,Status.StatusLine2,Str,STATUS_COLOR);
                         #ifndef LIB
                           if (Status.CapFile.handle != 0)
                             fastprint(Column,Status.StatusLine2,"F",STATUS_COLOR);
                           if (Status.Printer)
                             fastprint(Column+1,Status.StatusLine2,"P",STATUS_COLOR);
                         #endif
                       }
                       break;

      case CONFREG   : /*sprintf(Str,"Msgs Left: %s  Read: %s",comma(Tmp1,UsersData.MsgsLeft),comma(Tmp2,UsersData.MsgsRead));*/
                       /*sprintf(Str,"Bytes Uploaded: %s  Downloaded: %s  Today: %s",comma(Tmp1,UsersData.TotUpldBytes),comma(Tmp2,UsersData.TotDnldBytes),comma(Tmp3,UsersData.DailyDnldBytes));*/

                       Save = Country;
                       Country = SystemCountry;
                       sprintf(Str,"Msgs Left:%7s  Files U/L:%7s  Bytes U/L:%12s",
                               comma(Tmp1,UsersData.MsgsLeft),
                               comma(Tmp2,UsersData.NumUploads),
                               dlimcomma(Tmp3,UsersData.TotUpldBytes));
                       fastprint(2,Status.StatusLine1,Str,STATUS_COLOR);

                       sprintf(Str,"Msgs Read:%7s  Files D/L:%7s  Bytes D/L:%12s  Today:%8s",
                              comma(Tmp1,UsersData.MsgsRead),
                              comma(Tmp2,UsersData.NumDownloads),
                              dlimcomma(Tmp3,UsersData.TotDnldBytes),
                              comma(Tmp4,UsersData.DailyDnldBytes));
                       fastprint(2,Status.StatusLine2,Str,STATUS_COLOR);
                       Country = Save;
                       break;

      case COMMENT  : sprintf(Str,"%-13.13s / %-13.13s  E/D: %s  P/W: %s",
                               UsersData.BusDataPhone,
                               UsersData.HomeVoicePhone,
                               countrydate(juliantodate(UsersData.RegExpDate)),
                               (Status.UserRecNo == 1 ? "*PRIVATE*" : UsersData.Password));
                       fastprint(2,Status.StatusLine1,Str,STATUS_COLOR);
                       sprintf(Str,"C1: %-30.30s C2: %s",
                               UsersData.UserComment,
                               UsersData.SysopComment);
                       fastprint(2,Status.StatusLine2,Str,STATUS_COLOR);
                       break;

      case HELP      : getpcbtext(TXT_HELPLINE1,&Buf);
                       fastprint(2,Status.StatusLine1,Buf.Str,STATUS_COLOR);
                       getpcbtext(TXT_HELPLINE2,&Buf);
                       fastprint(2,Status.StatusLine2,Buf.Str,STATUS_COLOR);
                       break;

      case MODEMSTATS: modemstatus1(Str);
                       fastprint(2,Status.StatusLine1,Str,STATUS_COLOR);
                       modemstatus2(Str);
                       fastprint(2,Status.StatusLine2,Str,STATUS_COLOR);
                       break;
    }

update:
    if (Status.StatLine == NAME) {
      strcpy(Tmp1,Status.LogonTime);
      Tmp1[5] = 0;
      fastprint(74,Status.StatusLine1,Tmp1,STATUS_COLOR);
      #ifndef LIB
        ascii(Str,MaxHandle);
        if (MaxHandle < 10)
          fastprint(Column+3,Status.StatusLine2," ",STATUS_COLOR);
        fastprint(Column+2,Status.StatusLine2,Str,ScrnColors->HandleColor);
      #endif
      #if defined(LIB) && defined(SDATA)
        sprintf(Str,"%3dK",farcoreleft()/1024);
        fastprint(65,Status.StatusLine1,Str,ScrnColors->MemColor);
      #endif

      #ifndef __OS2__
        sprintf(Str,"%3dK",(int) (coreleft()/1024));
        fastprint(65,Status.StatusLine2,Str,ScrnColors->MemColor);
      #endif

      if (Status.UserRecNo != -1) {
        sprintf(Str,"%4d",minutesonline());
        fastprint(69,Status.StatusLine1,Str,STATUS_COLOR);
        sprintf(Str,"%4d",minutesleft());
        fastprint(69,Status.StatusLine2,Str,STATUS_COLOR);
      }
      fastprint(74,Status.StatusLine2,timestr2(Tmp1),STATUS_COLOR);
      if (Status.SysopFlag != ' ') {
        Tmp1[0] = Status.SysopFlag;
        Tmp1[1] = 0;
        fastprint(73,Status.StatusLine1,Tmp1,STATUS_COLOR+128);
      }
      #ifndef LIB
        if (Status.Paged)
          fastprint(73,Status.StatusLine2,"p",STATUS_COLOR+128);
        if (AutoGoodBye)
          fastprint(73,Status.StatusLine2,"g",STATUS_COLOR+128);
      #endif
    }

    settimer(2,THIRTYSECONDS);
  #endif
}


void LIBENTRY redisplaystatusline(void) {
  if (Asy.Online == OFFLINE)
    return;

  Status.StatLine    = NAME;
  Status.OldStatLine = NONE;
  showstatusline();
}


#ifndef LIB
#ifndef __OS2__
void LIBENTRY showusernetscan(bool Scanning) {
  if (Asy.Online != OFFLINE && Status.StatLine != NONE)
    fastputc(24*160 + 79*2,(Scanning ? '' : ' '));
}
#endif  /* ifndef __OS2__ */

void LIBENTRY showphoneringing(void) {
  clsbox(0,Status.StatusLine2,79,Status.StatusLine2,STATUS_COLOR);
  fastcenter(Status.StatusLine2,"(Incoming Call - Please Logoff)",STATUS_COLOR);
  settimer(2,THREESECONDS);
  Status.OldStatLine = NONE;  /* force it to be redrawn in 3 seconds */
}
#endif  /* ifndef LIB */
