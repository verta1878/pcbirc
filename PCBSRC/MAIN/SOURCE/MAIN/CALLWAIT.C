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

#include "event.h"
#include "serial.h"

#ifdef __OS2__
  #include <threads.h>
  #include <semafore.hpp>
  static bool ExitThreadNow;
  static bool PauseUpdate;
  static bool WaitingForConnect;
#endif

#ifdef FIDO
//#include <messages.h>
  #include <defines.h>
  #include <structs.h>
  #include <prototyp.h>
  #include <tossmisc.h>
#endif

#define MAXINBYTES    256
#define MODEMBUFSIZE 2048
#define NUMCWOPTIONS   15
#define DESCLINE       15
#define WINDOWLEN      73-5

#ifdef PCB_DEMO
#define THIRTYSECONDS 546
#endif

typedef struct {
  char X;
  char Y;
  char *Text;
  char *Desc;
} fieldtype;

static fieldtype Fields[NUMCWOPTIONS] = {
 { 4, 4,"",""},
 { 4, 6,"",""},
 { 4, 8,"",""},
 { 4,10,"",""},
 { 4,12,"",""},
 {30, 4,"",""},
 {30, 6,"",""},
 {30, 8,"",""},
 {30,10,"",""},
 {30,12,"",""},
 {56, 4,"",""},
 {56, 6,"",""},
 {56, 8,"",""},
 {56,10,"",""},
 {56,12,"",""}
};

struct {
  char X;
  char Y;
} static Offsets[NUMCWOPTIONS] = { {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {2,0}, {2,1}, {2,2}, {2,3}, {2,4} };


showtype static OnScreen;
char static CommWindow[81];
char static WritePos;
char static Selection[3][5] = { {0,1,2,3,4}, {5,6,7,8,9}, {10,11,12,13,14} };

#ifdef PCBSTATS
  stattype static Stats;
#endif

typedef enum {SYSAVAIL,RINGDETECT,RESETMODEM,RESETERROR,GOINGOFFHOOK} modemstattype;
modemstattype static StatusNum;


static void _NEAR_ LIBENTRY updatestatus(void);

static void _NEAR_ LIBENTRY updatemodemstatus(char *Str) {
  fastprint(5,20,Str,ScrnColors->BarColor);
}

void LIBENTRY showmodemdata(char *Str) {
  char *p;
  int  NextChar;
  int  LastChar;

  if (OnScreen == HIDE)
    return;

  #ifdef DEBUG
    mc_register(CommWindow,sizeof(CommWindow));
  #endif

  for (p = Str, LastChar = 0; TRUE; p++) {       //lint !e506  TRUE is OK
    NextChar = *p;
    if (NextChar == 0)
      break;
    if (NextChar <= ' ' || NextChar > 'Z') {
      NextChar = ' ';
      if (NextChar == LastChar)
        continue;
    }
    LastChar = NextChar;
    if (strlen(CommWindow) < WINDOWLEN)
      addchar(CommWindow,NextChar);
    else {
      memcpy(&CommWindow[0],&CommWindow[1],WINDOWLEN);
      CommWindow[WINDOWLEN-1] = NextChar;
    }
    updatemodemstatus(CommWindow);
  }

  #ifdef DEBUG
    mc_unregister(CommWindow);
  #endif
}


void LIBENTRY initcommwindow(showtype Type) {
  CommWindow[0] = 0;
  OnScreen = Type;
  if (OnScreen == SHOW)
    clsbox(4,20,74,20,ScrnColors->BarColor);
}


/********************************************************************
*
*  Function:  showselection()
*
*  Desc    :  displays the highlight bar on the callwaiting menu
*             - unhighlights the previous bar
*/

static void _NEAR_ LIBENTRY showselection(int X, int Y, char *Str, int Len, int Color) {
  int  StrLen;
  char Field[80];
  char Shadow[80];

  memset(Field,' ',Len);  Field[Len] = 0;
  memset(Shadow,'ß',Len); Shadow[Len] = 0;

  StrLen = strlen(Str);
  if (StrLen > Len) {
    /* protect it from overflowing the buffer */
    Str += StrLen - Len;   /* cause it to shift the buffer right */
    StrLen = Len;          /* then limit it to the field length  */
  }

  memcpy(&Field[(Len - StrLen) >> 1],Str,StrLen);
  fastprint(X,Y,Field,Color);
  fastprint(X+Len,Y,"Ü",ScrnColors->Shadow);
  fastprint(X+1,Y+1,Shadow,ScrnColors->Shadow);

  if (Color == ScrnColors->HiLite)
    agotoxy(X+(Len >> 1),Y);
}


static void _NEAR_ LIBENTRY showhit(fieldtype *p) {
  int  X = p->X;
  int  Y = p->Y;
  char Str[80];

  Str[19] = 0;
  fastprint(X,Y," ",ScrnColors->BoxColor);
  memset(Str,'Ü',19);
  fastprint(X+1,Y,Str,ScrnColors->HitColor);
  memset(Str,'ß',19);
  fastprint(X+1,Y+1,Str,ScrnColors->HitColor);
  #ifdef __OS2__
    updatelinesnow();
  #endif
  mydelay(5);
}


/********************************************************************
*
*  Function:  updatetime()
*
*  Desc    :  updates the time and date on the callwaiting screen
*/

static void _NEAR_ LIBENTRY updatetime(void) {
  char Temp[9];

  datestr(Temp);
  countrydate(Temp);

  fastprint( 5,0,Temp,ScrnColors->BoxColor);
  fastprint(66,0,timestr1(Temp),ScrnColors->BoxColor);
}


static bool _NEAR_ LIBENTRY updateclockstatus(void) {
  char Temp[10];
  long Time;

  Time = gettimer(4);
  if (Time < 0) {
    tickdelay(ONESECOND);
    return(TRUE);
  }

  sprintf(Temp,"%2ld",(Time/ONESECOND)+1);
  fastprint(WritePos,18,Temp,ScrnColors->BarColor);

  #ifndef __OS2__
    if (timerexpired(2)) {
      updatetime();
      settimer(2,ONESECOND);
    }
  #endif

  return(FALSE);
}


#ifdef __OS2__
// this thread will monitor the time with 1-second intervals for the purpose of
//
// 1) updating the time at the top of the screen,
// 2) updating the modem countdown timer
// 3) turning the screen off after three minutes
//
// Note: The thread is destroyed when PCBoard leaves the call waiting screen
// Note: The thread avoids updating the screen if the screen is off, or if
//       the PauseUpdate variable is true

#pragma argsused
static void THREADFUNC timethread(void *Ignore) {
  CUpdateSemaphore TimerSem;

  if (TimerSem.createunique("CWTIME")) {
    TimerSem.enabletimer(1000);  // 1-second updates

    setdefaultpriority();

    while (1) {
      TimerSem.waitforevent();
      TimerSem.reset();

      // watch for a sign that we need to terminate this thread
      if (ExitThreadNow)
        break;

      #ifdef FIDO
        // check to see if it's time to perform any of the fido processing
        if (PcbData.EnableFido &&
            ((PcbData.FidoDialOut && Asy.ComPortNumber != 0 && PcbData.FidoMailFreq > 0 && timerexpired(9)) ||
             (PcbData.FidoProcessOut && PcbData.FidoExportFreq > 0 && timerexpired(10)) ||
             (PcbData.FidoProcessIn && PcbData.FidoPktFreq > 0 && timerexpired(11))))
          releasekbdblock(0);
      #endif

      if (Control.Screen) {
        if (! PauseUpdate) {
          updatetime();
          if (WaitingForConnect) {
            updateclockstatus();       //lint !e534
          } else {
            if ((! PcbData.DisableCls) && timerexpired(12))
              turndisplayoff(FALSE);
          }
        }
      }
    }
  }
}


static void LIBENTRY createtimethread(void) {
  ExitThreadNow     = FALSE;
  PauseUpdate       = TRUE;
  WaitingForConnect = FALSE;
  startthread(timethread,8*1024,NULL);
}

static void LIBENTRY destroytimethread(void) {
  // don't use DosKillThread because the thread owns a semaphore and it
  // needs to close it
  ExitThreadNow = TRUE;
}
#else
  #define destroytimethread()
#endif


static void _NEAR_ LIBENTRY setscrncleartimer(void) {
  settimer(12,THREEMINUTES);  // set 3 minute screen clear
}


static void _NEAR_ LIBENTRY updatescreen(void) {
  if (! Control.Screen) {
    setscrncleartimer();  // don't let the thread turn it right off again.
    #ifdef __OS2__
      turndisplayon(FALSE);
      PauseUpdate = FALSE;
    #else
      updatetime();
      turndisplayon(FALSE);
    #endif
    #ifdef PCBSTATS
      if (! Status.LocalStats && checkstats(&Stats,FALSE))
        updatestatus();
    #endif
    setcursor(CUR_BLANK);  /* turn it BACK OFF */
  }
}



/********************************************************************
*
*  Function:  updatestatus()
*
*  Desc    :  Shows the System, Printer, Alarm and Page Bell status
*/

static void _NEAR_ LIBENTRY updatestatus(void) {
  pcbtexttype Buf;
  char        Str[80];

  if (Asy.ComPortNumber == 0) {
    getpcbtext(TXT_NOMODEMSELECTED,&Buf);
    maxstrcpy(Str,Buf.Str,sizeof(Str));
  } else {
    switch (StatusNum) {
      case SYSAVAIL    :
                         #ifdef FIDO
                           if (PcbData.EnableFido && (FidoMailHour || ! Status.AllowHumanCallers)) {
                             strcpy(Buf.Str,"Fido Mail - No Human Callers");
                             break;
                           }
                         #endif
                         getpcbtext(TXT_SYSTEMAVAIL ,&Buf);
                         break;
      case RINGDETECT  : sprintf(Status.DisplayText,"%2d",PcbData.Seconds);
                         Status.AppendText = TRUE;
                         getpcbtext(TXT_RINGDETECTED,&Buf);
                         break;
      case RESETMODEM  : getpcbtext(TXT_RESETMODEM  ,&Buf);
                         break;
      case RESETERROR  : getpcbtext(TXT_MODEMERROR  ,&Buf);
                         break;
      case GOINGOFFHOOK: getpcbtext(TXT_GOINGOFFHOOK,&Buf);
                         break;
    }
    #ifdef MULTIPORT
      if (PcbData.ModemPort[0] == 'P' && PcbData.OS2Driver)
        sprintf(Str,"(OS/2) COM%d: %s",Asy.ComPortNumber,Buf.Str);
      else
        sprintf(Str,"%s %s",PcbData.ModemPort,Buf.Str);
    #else
      sprintf(Str,"%s %s",PcbData.ModemPort,Buf.Str);
    #endif
    if (StatusNum == RINGDETECT) {
      WritePos = strlen(Str);
      WritePos = ((71-WritePos) >> 1) + WritePos + 2;
    }
  }
  showselection(4,18,Str,71,ScrnColors->BarColor);

  #ifdef PCBSTATS
    if (StatusNum == SYSAVAIL) {
      getpcbtext(TXT_LASTCALLER,&Buf);
      if (Stats.LastCaller[0] == 0) {
        maxstrcpy(Str,Buf.Str,sizeof(Str));
        getpcbtext(TXT_NONE,&Buf);
        addchar(Str,' ');
        strcat(Str,Buf.Str);
      } else {
        Stats.Time[5] = 0;
        sprintf(Str,"%s %s (%s)",Buf.Str,Stats.LastCaller,Stats.Time);
      }
      Str[71] = 0;  /* make sure it fits on screen */
      showselection(4,20,Str,71,ScrnColors->BarColor);
    }

    getpcbtext(TXT_NUMCALLS,&Buf);
    sprintf(Str,"%s %ld",Buf.Str,Stats.NewCalls);
    showselection(4,22,Str,14,ScrnColors->BarColor);

    getpcbtext(TXT_NUMMESSAGES,&Buf);
    sprintf(Str,"%s %ld",Buf.Str,Stats.NewMsgs);
    showselection(23,22,Str,14,ScrnColors->BarColor);

    getpcbtext(TXT_NUMDOWN,&Buf);
    sprintf(Str,"%s %ld",Buf.Str,Stats.TotalDn);
    showselection(42,22,Str,14,ScrnColors->BarColor);

    getpcbtext(TXT_NUMUP,&Buf);
    sprintf(Str,"%s %ld",Buf.Str,Stats.TotalUp);
    showselection(61,22,Str,14,ScrnColors->BarColor);

    agotoxy(53,24);
  #endif
}


/********************************************************************
*
*  Function:  getlocalpassword()
*
*  Desc    :  Logs in the Sysop from the local console
*/

static calltype _NEAR_ LIBENTRY getlocalpassword(void) {
  int   Saved;
  char  LocalPassword[13];

  if (PcbData.Password[0] == 0)
    return(CALLWAIT_SYSOP);

  updatescreen();
  clsbox(1,15,78,15,0x07);
  agotoxy(2,15);
  setcursor(CUR_NORMAL);
  Control.GraphicsMode = PcbData.Graphics;
  LocalPassword[0] = 0;

  settimer(5,FIVEMINUTES);  /* give him some time so it doesn't time out here */
  setkbdtimer(1);                // set keyboard timer value to 1 minute
  settimer(2,THREEMINUTES);      // fix the status-line timer so it won't fire

  Control.WatchKbdClock     = TRUE;
  Control.WatchSessionClock = TRUE;

  // To prevent a keyboard timeout from sending us through the RECYCLE process
  // we'll set the EnteringMessage flag on.  That will cause the input to
  // simply abort and return back to this function instead of calling recycle()

  Status.EnteringMessage = TRUE;

  // In case the "keyboard timeout" message gets displayed, save the screen
  // first, then restore it down below if needed
  Saved = memsavescreen();

  inputfield(LocalPassword,TXT_LOCALPASSWORD,12,ECHODOTS|UPCASE,NOHELP,mask_alphanum);

  if (Status.KbdTimedOut)
    memrestorescreen(Saved,FREESCREEN);
  else
    memfreescreen(Saved);

  setkbdtimer(PcbData.KbdTimeout);  // reset normal keyboard timer value
  Control.WatchKbdClock     = FALSE;
  Control.WatchSessionClock = FALSE;
  Status.EnteringMessage    = FALSE;

  if (strcmp(LocalPassword,PcbData.Password) == 0)
    return(CALLWAIT_SYSOP);

  setcursor(CUR_BLANK);
  return(CALLWAIT_RECYCLE);
}


static void _NEAR_ LIBENTRY displaymenuselections(void) {
  fieldtype *p;
  int        X;

  for (X = 0, p = Fields; X < NUMCWOPTIONS; X++, p++)
    showselection(p->X,p->Y,p->Text,19,ScrnColors->LoLite);
}

// use CRYPTO.EXE to create the encrypted text
// static char *copyright="  (C) Copyright Clark Development Company, Inc., 1985-1996  ";

static char copyright[] = {0xA4,0x5B,0xF4,0xDD,0xDD,0xFD,0xB2,0xBE,0x25,0xB9,
  0x79,0x23,0xB7,0xF3,0xAA,0xFB,0xE9,0x97,0x71,0xCB,0x15,0xC6,0x70,0x82,0xA9,
  0x17,0x14,0xA6,0x12,0x71,0xD8,0xAD,0xD0,0xB0,0x38,0xDD,0xEF,0xDB,0x56,0x5E,
  0x2F,0x72,0x66,0x52,0xBD,0xCA,0x5F,0x08,0x59,0x2C,0x57,0x62,0x3E,0xCB,0x5C,
  0xB9,0xD2,0xF4,0xD3,0xF5,0xD3};

static void _NEAR_ LIBENTRY makemenu(void) {
  char Line[80];

  setcursor(CUR_BLANK);
  boxcls(0,0,79,24,ScrnColors->BoxColor,DOUBLE);
//fastprint(34,1,"S/N:",ScrnColors->BoxColor);
  sprintf(Line,"  %s  ",Status.Version);
  fastcenter(0,Line,ScrnColors->VersionColor);
  decrypt(copyright,61);
  fastcenter(24,copyright,ScrnColors->BoxColor);
  encrypt(copyright,61);

  memset(Line,'Í',78); Line[78] = 0;
  fastprint(1,DESCLINE+1,Line,ScrnColors->BoxColor);

  buildstr(Line,PcbData.BoardName,Status.NodeStr,NULL);
  showselection(4,2,Line,71,ScrnColors->TitleColor);

  displaymenuselections();
/*agotoxy(Fields[5].X,Fields[5].Y);*/
}


static fieldtype * _NEAR_ LIBENTRY moveselection(fieldtype *p, fieldtype *q) {
  showselection(p->X,p->Y,p->Text,19,ScrnColors->LoLite);
  showselection(q->X,q->Y,q->Text,19,ScrnColors->HiLite);
  clsbox(1,DESCLINE,78,DESCLINE,ScrnColors->BoxColor);
  fastcenter(DESCLINE,q->Desc,ScrnColors->BoxColor);
  return(q);
}


void LIBENTRY reseterror(void) {
  StatusNum = RESETERROR;
  updatestatus();
  mysound(500,100);
}


/********************************************************************
*
*  Function:  modemreset()
*
*  Desc    :  if the modem port is not set to NONE it attempts to reset
*             the modem.  It also displays the modem status information
*             in the callwaiting screen.
*/

static void _NEAR_ LIBENTRY modemreset(void) {
  if (Asy.ComPortNumber != 0) {
    closemodem(TRUE);          /* close it if it was open already */
    StatusNum = RESETMODEM;
    updatestatus();
    showselection(4,20,"",71,ScrnColors->BarColor);
    initcommwindow(SHOW);
    openmodem(SHOW);
  }
  StatusNum = SYSAVAIL;
  updatestatus();
}


/********************************************************************
*
*  Function:  errorcorrected()
*
*  Desc    :  Determines if the string passed to the function contains one of
*             the codes that would indicate an error corrected session has
*             been established.
*
*  Returns :  TRUE if an error corrected session has been made.
*/

bool LIBENTRY errorcorrected(char *Str) {
  int  X;
  int  Stop;
  char static *ErrCorrStrs[16] =
    { "X75","ISDN","MNP","HST","ALT","REL","ARQ","ERR","LAP","BUF","V42","V.42"," EC","/REC","192","960" };

  // found a modem that used "COMPRESSION NONE" to indicate no compression,
  // that means we can't use the word "COMPRESSION" to indicate an
  // error-corrected connection!

  Stop = (PcbData.FastComm ? 16 : 14);
  for (X = 0; X < Stop; X++) {
    if (X == 9 && PcbData.FastComm)
      continue;  /* FASTCOMM returns "BUF" on non-error-corrected sessions */
    if (strstr(Str,ErrCorrStrs[X]) != NULL)
      return(TRUE);
  }

  return(FALSE);
}


/********************************************************************
*
*  Function:  checkmodemdata()
*
*  Desc    :  gets input from the modem, displays it on screen, then copies
*             it to the end of the buffer used for holding the connect
*             message - if there is insufficient room to hold the new bytes
*             then room is made for exactly the number of bytes needed by
*             copying X number of bytes into the buffer down to offset 0.
*/

static void _NEAR_ LIBENTRY checkmodemdata(char *Buffer) {
  int  NumBytesInBuffer;
  int  NumBytesFound;
  int  ByteCount;
  char Temp[MAXINBYTES];

  #ifdef DEBUG
    mc_register(Temp,MAXINBYTES);
  #endif

  ByteCount = 0;

  while (1) {
    tickdelay(QUARTERSECOND);
    if ((NumBytesFound = cgetstr(Temp,MAXINBYTES-1)) == 0) {
      #ifdef DEBUG
        mc_unregister(Temp);
      #endif
      return;
    }

    showmodemdata(Temp);

    NumBytesInBuffer = strlen(Buffer);
    if (NumBytesInBuffer+NumBytesFound > MODEMBUFSIZE-1) {
      strcpy(Buffer,&Buffer[MAXINBYTES+1]);  /* make room in buffer */
      NumBytesInBuffer -= MAXINBYTES+1;
    }

    /* add the new data into the buffer */
    strcpy(&Buffer[NumBytesInBuffer],Temp);

    ByteCount += NumBytesFound;
    if (ByteCount > 2048) {
      /* we shouldn't receive 2K in one connect sequence - maybe the modem */
      /* is stuck sending data to us...  let's get out of here now.        */
      #ifdef DEBUG
        mc_unregister(Temp);
      #endif
      return;
    }
  }
}


static void _NEAR_ LIBENTRY controlcharsintospaces(char *Str) {
  for (; *Str; Str++)
    if (*Str < ' ' || *Str > 'Z')
      *Str = ' ';
}


static void _NEAR_ LIBENTRY removedoublespaces(char *Str) {
  char *p;

  stripleft(Str,' ');
  while ((p = strstr(Str,"  ")) != NULL)
    strcpy(p,p+1);
}


static void _NEAR_ LIBENTRY getconnectstr(char *Buffer) {
  strupr(Buffer);
  controlcharsintospaces(Buffer);

  if (PcbData.ModemAns[0] != 0)
    substitute(Buffer,PcbData.ModemAns,"",MODEMBUFSIZE);

  substitute(Buffer,"OK " ,"",MODEMBUFSIZE);
  removedoublespaces(Buffer);
  substitute(Buffer,"COMPRESSION","COMP",strlen(Buffer));
  substitute(Buffer,"PROTOCOL"   ,"PROT",strlen(Buffer));
  substitute(Buffer,"MODULATION" ,"MOD" ,strlen(Buffer));
  maxstrcpy(Asy.ConnectString,Buffer,sizeof(Asy.ConnectString));
  stripright(Asy.ConnectString,' ');
}


static void _NEAR_ LIBENTRY getcalleridstr(char *Buffer) {
  substitute(Buffer,"RING","",MODEMBUFSIZE);
  controlcharsintospaces(Buffer);
  removedoublespaces(Buffer);
  maxstrcpy(Asy.CallerIDString,Buffer,sizeof(Asy.CallerIDString));
  stripright(Asy.CallerIDString,' ');
}


void LIBENTRY offhook(void) {
  if (Asy.ComPortNumber != 0) {
    StatusNum = GOINGOFFHOOK;
    updatestatus();
    initcommwindow(SHOW);
    modemoffhook();
  }
}


static char * _NEAR_ LIBENTRY strnrchr(char *Str, char C, int Len) {
  for (Str += Len; Len >= 0; Str--, Len--)
    if (*Str == C)
      return(Str);
  return(NULL);
}


static char * _NEAR_ LIBENTRY strnrstr(char *Str, char *Search, int Len) {
  char  FirstChar;
  char *p;
  int   SearchLen;

  if ((SearchLen = strlen(Search)) == 0)
    return(NULL);

  FirstChar = *Search;
  if (SearchLen == 1)
    return(strrchr(Str,FirstChar));

  Search++;     // remove the first character from the search
  SearchLen--;  // reduce the search length by 1

  while (Len > 0) {
    if ((p = strnrchr(Str,FirstChar,Len)) == NULL)
      return(NULL);

    if (memcmp(p+1,Search,SearchLen) == 0)
      return(p);

    Len = (int) (p - Str) - 1; // treat Str as 1 byte shorter than last found
  }

  return(NULL);
}


void LIBENTRY logcalleridstring(void) {
  bool  Done;
  int   Len;
  int   X;
  char *p;
  char *q;
  char  Str[64];

  if (PcbData.LogConnectStr && Asy.CallerIDString[0] != 0) {
    for (p = Asy.CallerIDString, Len = strlen(p), Done = FALSE; ! Done; ) {
      // 62 is the length of the line in the log, 6=indent, 5="CID: "
      // and 62-11 equals 51 recordable bytes in the log
      // while 56 is the maximum length of the string (62-6 for indent)
      if (Len > 62-(6+5)) {
        sprintf(Str,"CID: %51.51s",p);
        // see if either NAME = or NMBR = is in the line, but NOT at the FRONT
        // of the line.  If either one is found somewhere in the line, then
        // we'll split the line at that point.
        if (((q = strnrstr(Str,"NAME =",56)) != NULL && q > Str+5) ||
            ((q = strnrstr(Str,"NMBR =",56)) != NULL && q > Str+5)) {
          *q = 0;                  // NULL terminate the string
          X = (int) (q - Str) - 5; // count characters before "NAME =" minus "CID: "
          p += X;
          Len -= X;
        } else {
          p += 51;
          Len -= 51;
        }
      } else {
        sprintf(Str,"CID: %s",p);
        Done = TRUE;
      }
      writelog(Str,SPACERIGHT);
    }
  }
}


static void _NEAR_ LIBENTRY updatelog(int TextNum) {
  pcbtexttype Buf;

  if (! PcbData.MonitorModem)
    return;

  logcalleridstring();

  getpcbtext(TextNum,&Buf);
  maxstrcpy(Status.DisplayText,Buf.Str,sizeof(Status.DisplayText));
  logsystext(TXT_NOCONNECT,SPACERIGHT);
  writelog(LogDivider,LEFTJUSTIFY);
}


static long _NEAR_ LIBENTRY getspeed(char *Str) {
  bool     Leading;
  unsigned Len;
  char     Buffer[15];

  for (Len = 0, Leading = TRUE; Len < sizeof(Buffer)-1; Str++) {
    // allow leading spaces, but spaces at the end terminate the string
    if (*Str == ' ') {
      if (Leading)
        continue;
      break;
    } else
      Leading = FALSE;

    if (*Str == '.')  // skip over periods in numbers
      continue;

    if (*Str == 0 || *Str < '0' || *Str > '9')
      break;

    Buffer[Len++] = *Str;
  }

  Buffer[Len] = 0;
  return(atol(Buffer));
}


static void _NEAR_ LIBENTRY setautoresettimer(void) {
  if (PcbData.AutoReset) {
    settimer(8,FIVEMINUTES);
    #ifdef __OS23__
      releasekbdblock(5 * 1000 * 60);
    #endif
  }
}


static void _NEAR_ LIBENTRY setcallwaitnettimer(void) {
  long CallWaitNetTimer = Control.NetTimer * 6;

  // under DOS, this is used for both usernet and pcbstats.dat scans
  // under OS/2, it is only used for pcbstats.dat scans
  settimer(7,CallWaitNetTimer);

  #ifdef __OS2__
    // Control.NetTimer was based on 1/100th of a second, but CallWaitNetTimer
    // is based on 1/1000th of a second, so multiply by 10
    releasekbdblock(CallWaitNetTimer*10);
  #endif
}


#ifdef FIDO
static void _NEAR_ LIBENTRY setfidotimer(int Timer, int Value) {
  long TimeToEvent = (long) Value * (long) ONEMINUTE;

  settimer(Timer,TimeToEvent);
  #ifdef __OS2__
    releasekbdblock(TimeToEvent);
  #endif
}
#endif


void LIBENTRY processconnectstring(char *ComBuffer) {
  char *p;

  if ((p = strstr(ComBuffer,"CONNECT")) == NULL)
    return;

  p += 7;

  Asy.ErrorCorrected = errorcorrected(ComBuffer);
  Asy.ModemSpeed     =
  Asy.CarrierSpeed   = PcbData.ModemSpeed;
  Asy.HstMode        = (strstr(ComBuffer,"HST") != NULL);

  if (strstr(p,"FAST") != NULL) {
    Asy.ErrorCorrected = TRUE;
  } else {
    if ((Asy.ModemSpeed = getspeed(p)) == 0)
      Asy.ModemSpeed = 300;
    Asy.CarrierSpeed = Asy.ModemSpeed;
  }

//checkmodemdata(ComBuffer);
  if (! Asy.ErrorCorrected)
    Asy.ErrorCorrected = errorcorrected(ComBuffer);

  if ((p = strstr(ComBuffer,"CARRIER")) != NULL) {
    Asy.CarrierSpeed = getspeed(p+7);
  } else if (PcbData.FastComm) {
    if ((p = strchr(ComBuffer,'-')) != NULL && getspeed(p+1) != 0)
      Asy.ConnectSpeed = Asy.CarrierSpeed = getspeed(p+1) * 100L;
    if (strstr(ComBuffer,"BUFF") != NULL)  /* BUFF means it's not error corrected */
      Asy.ErrorCorrected = FALSE;
  } else {
    /* check for modems that give CONNECT XXXX/ARQ/MNP/YYYY */
    /* messages, where YYYY is the carrier speed            */
    if ((p = strrchr(ComBuffer,'/')) != NULL && getspeed(p+1) >= 300)
      Asy.CarrierSpeed = getspeed(p+1);

    if (PcbData.LockSpeed || PcbData.ModemSpeed > 9600)
      Asy.ModemSpeed = PcbData.ModemSpeed;
  }

  /* deal with 14.4 connections (the "." was removed up above) */
  /* by making sure it is not a 300 bps connection, and then   */
  /* multiplying by 100 to get the full speed                  */
  if (Asy.CarrierSpeed < 1200 && Asy.CarrierSpeed > 0 && Asy.CarrierSpeed != 300)
    Asy.CarrierSpeed *= 100;

  if (Asy.ConnectSpeed < 1200 && Asy.ConnectSpeed > 0 && Asy.ConnectSpeed != 300)
    Asy.ConnectSpeed *= 100;

  getconnectstr(ComBuffer);

  // find out if we're using VMODEM to handle a TELNET call
  // VMODEM appends "/TEL" to the connect string and adds the
  // IP address (such as 199.67.41.2) to the end.  We record
  // the fact that Telnet is being used because we might receive
  // a CR/NULL pair and we don't want to get stuck waiting in
  // the commportinkey() function for a NULL/ScanCode pair.
  if (strstr(ComBuffer,"/TEL") != NULL && strchr(ComBuffer,'.') != NULL)
    Asy.Telnet = TRUE;
  #if defined(__OS2__) && defined(MULTIPORT)
    else if (PcbData.OS2Driver && istelnetviavmodem())
      Asy.Telnet = TRUE;
  #endif

  /* go set the port speed now */
  openlog();
  if (DebugLevel >= 1)
    reportcom(Asy.ModemSpeed);

  setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
  Asy.ConnectSpeed = (Asy.ErrorCorrected && Asy.CarrierSpeed >= 9600 ? Asy.ModemSpeed : Asy.CarrierSpeed);

  if (! Asy.ErrorCorrected)       // non-MNP modems should wait
    tickdelay(FOURSECONDS);       // 4 seconds before continuing
  else
    tickdelay(ONESECOND);         // MNP modems continue in 1 sec
}


/********************************************************************
*
*  Function:  callwait()
*
*  Desc    :  - Displays the callwaiting screen
*             - Allows the sysop to set Printer, Alarm and Page Bell status
*             - Waits for a ring detect
*             - Waits for an exit to dos or a local logon
*             - Dispatches to the appropriate routine for logon
*
*  Notes   :  Timer usage for the callwaiting screen is as follows:
*
*             timer 1 = three minute countdown to clear the screen
*             timer 2 = one second delays for showing the time on the screen
*             timer 4 = count down after ring detect before recycle
*             timer 5 = initial setting of session timer when caller logs in
*             timer 7 = network delay timer (for scanning usernet.dat)
*/

#define cleanup()                 \
           bfree(Text);           \
           bfree(ComBuffer);      \
           destroytimethread();   \
           destroyeventthread();

#pragma warn -aus
calltype LIBENTRY callwait(void) {
  #ifdef FIDO
  bool         SaveAllowHumanCallers;
  bool         SaveBlankerStatus;
  #endif
  bool         PerformChecks;  // TRUE most of the time, FALSE if handling keyboard or comm port input
  int          Key;
  int          X;
  int          Y;
  #ifndef PCB_DEMO
  int          XY;
  #endif
  int          Event;
  int          Offset;
  bool         LastAttemptFailed;
  char         NumRings;
  char        *p;
  fieldtype   *q;
  char        *ComBuffer;
  pcbtexttype *Text;
  char         AnswerStr[sizeof(PcbData.ModemAns)+1];

  if ((ComBuffer = (char *) bmalloc(MODEMBUFSIZE)) == NULL) {
    errorexittodos("Insufficient memory for CW ComBuffer");
    return(CALLWAIT_EXIT);  // this is just to keep PC-Lint happy
  }

  if ((Text = (pcbtexttype *) bmalloc(41 * sizeof(pcbtexttype))) == NULL) {
    errorexittodos("Insufficient memory for CW Text");
    return(CALLWAIT_EXIT);  // this is just to keep PC-Lint happy
  }

  for (Y = 0, X = TXT_SYSOPBUSY; X <= TXT_TURNPAGEON; X++, Y++)
    getpcbtext(X,&Text[Y]);
  for (X = TXT_SYSOPNOTBUSY; X <= TXT_NUMUP; X++, Y++)
    getpcbtext(X,&Text[Y]);

  getpcbtext(TXT_SETSTATSSYSTEM,&Text[Y]);

  #ifndef PCB_DEMO
//  setcollect(SERIAL);
  #endif

  Fields[ 0].Text = Text[ 1].Str;
  Fields[ 1].Text = Text[ 8].Str;
  Fields[ 3].Text = Text[ 9].Str;
  Fields[ 5].Text = Text[ 0].Str;
  Fields[ 6].Text = Text[ 7].Str;
  Fields[ 8].Text = Text[10].Str;
  Fields[ 9].Text = Text[13].Str;
  Fields[10].Text = Text[ 2].Str;
  Fields[11].Text = Text[ 3].Str;
  Fields[13].Text = Text[11].Str;
  Fields[14].Text = Text[14].Str;

  Fields[ 4].Text = (Status.LocalStats ? Text[40].Str : Text[12].Str);
  Fields[ 2].Text = (PcbData.ParallelPortNum > 0 ? (Status.PrintLog ? Text[16].Str : Text[4].Str) : Text[15].Str);
  Fields[ 7].Text = (Status.PageBell ? Text[18].Str : Text[6].Str);
  Fields[12].Text = (Status.Alarm ? Text[17].Str : Text[5].Str);

  for (Y = 0, X = 19; Y <= 14; Y++, X++)
    Fields[Y].Desc = Text[X].Str;

  Offset = 5;
  q = &Fields[Offset];
  X = Offsets[Offset].X;
  Y = Offsets[Offset].Y;

  #ifndef PCB_DEMO
    XY = xyvalue(39,1);
  #endif

  #ifdef PCBSTATS
    memset(&Stats,0,sizeof(stattype));
    checkstats(&Stats,TRUE);                //lint !e534
  #endif

  if (PcbData.Network)
    clearusernet();

  setcallwaitnettimer();

  LastAttemptFailed = FALSE;
  Status.UserRecNo  = 0;
  Asy.ModemSpeed = PcbData.ModemSpeed;
  setcursor(CUR_BLANK);
  agotoxy(0,0);
  makemenu();

  Control.WatchKbdClock     = FALSE;
  Control.WatchSessionClock = FALSE;

  #ifdef PCB_DEMO
  {
    int Saved;

    ScrType static Inform[13] = {
      {22, 2,"Welcome to the PCBoard v15.1 DEMO!"                                   ,0x70},
      { 5, 4,"The package you are now running is not a self running demonstration"   ,0x70},
      { 5, 5,"program.  Instead, it is an actual PCBoard system packaged in a manner",0x70},
      { 5, 6,"that will let you run it to get a feel for what PCBoard is like."      ,0x70},
      { 5, 8,"Many of the features in this DEMO version are limited such as the"     ,0x70},
      { 5, 9,"number of messages you can save or the number of users you can have."  ,0x70},
      { 5,11,"For PCBoard ordering information please call our order department at"  ,0x70},
      { 5,12,"800-356-1686 (voice) during regular business hours or you may call our",0x70},
      { 5,13,"own systems (with 10 phone lines to serve you) at either 801-261-8976" ,0x70},
      { 5,14,"(for HST modems) or 801-261-8979 (for Hayes modems), all are v.32bis." ,0x70},
      { 5,16,"For questions concerning this DEMO please call 801-261-1686."          ,0x70},
      { 5,18,"We hope you'll find this program useful in evaluating our software."   ,0x70},
      {38,20,"Clark Development Company, Inc."                                       ,0x70}
    };

    Saved = memsavescreen();
    boxcls(03,01,76,21,0x70,DOUBLE);
    showscreen(Inform,13);
    settimer(4,FOURSECONDS);
    while (! timerexpired(4))
      kbdinkey();
    fastcenter(21," Press A Key (or continue in 30 seconds) ",0x70);
    settimer(4,THIRTYSECONDS);
    while (! timerexpired(4) && kbdinkey() == 0);
    settimer(5,FIVEMINUTES);
    memrestorescreen(Saved,FREESCREEN);
  }
  #endif

  #ifndef PCB_DEMO
//  fastputc(XY,collect());  XY += 2;
  #endif

  clsbox(3,0,14,0,ScrnColors->BoxColor);

  #ifndef PCB_DEMO
//  fastputc(XY,collect());  XY += 2;
  #endif

  clsbox(64,0,75,0,ScrnColors->BoxColor);

  #ifndef PCB_DEMO
//  fastputc(XY,collect());  XY += 2;
  #endif

  updatetime();

  #ifndef PCB_DEMO
//  fastputc(XY,collect());  XY += 2;
  #endif

  if (DisableAnswer)     // Intel SatisFAXtion requires the "ATA\r" be
    AnswerStr[0] = 0;    // disabled in order for FAX and DATA to be supported
  else {
    strcpy(AnswerStr,PcbData.ModemAns);
    addchar(AnswerStr,'\r');
  }

  #ifndef PCB_DEMO
//  fastputc(XY,collect());  XY += 2;
  #endif


  #ifdef FIDO
    setfidotimer(9,PcbData.FidoMailFreq);
    setfidotimer(10,PcbData.FidoExportFreq);
    setfidotimer(11,PcbData.FidoPktFreq);
  #endif

  #ifndef PCB_DEMO
//  fastputc(XY,collect());  XY += 2;
  #endif

  setscrncleartimer();

  #ifdef __OS2__
    if (Control.Screen) {
      createtimethread();
      // to "fix" the messed up screens that OS/2 likes to cause, save and
      // restore the entire screen now
      int ScrnNum = memsavescreen();
      memrestorescreen(ScrnNum,FREESCREEN);
      updatelinesnow();
    }
    if (AnswerOnStartup)
      releasekbdblock(0);
  #endif


top:
  #ifdef __OS2__
    PauseUpdate  = FALSE;
  #endif

  PerformChecks = TRUE;

  writeusernetstatus(NOCALLER,NULL);

  setscrncleartimer();

  NumRings = 0;
  modemreset();
  initcommwindow(HIDE);
  showselection(q->X,q->Y,q->Text,19,ScrnColors->HiLite);
  clsbox(1,DESCLINE,78,DESCLINE,ScrnColors->BoxColor);
  fastcenter(DESCLINE,q->Desc,ScrnColors->BoxColor);
  memset(ComBuffer,0,MODEMBUFSIZE);

  setautoresettimer();
  updatestatus();

  Asy.ConnectString[0] = 0;
  Asy.CallerIDString[0] = 0;

  while (1) {
    #ifdef __OS2__
      if (Control.Screen)
        updatelinesnow();
      WaitingForConnect = FALSE;
    #endif

    if (timerexpired(4))  /* in case we're counting RINGS, reset it now */
      NumRings = 0;

    if (PcbData.AutoReset && NumRings == 0 && timerexpired(8))
      goto top;           /* go reset the modem */

    // don't perform this extra overhead unless we are not processing a
    // keystroke or a byte from the comm port
    if (PerformChecks) {
      if (PcbData.EventActive) {
        #ifdef FIDO
          SaveAllowHumanCallers = Status.AllowHumanCallers;
        #endif

        // NOTE:  The timeforevent() function, under OS/2, creates an event
        // thread which will post an event to the WaitKeySem semaphore at the
        // time when the event is supposed to run.  This thread is destroyed
        // upon leaving this function and recreated the next time timeforevent()
        // is called.  The cleanup() macro is used to kill the thread and
        // clean up a few other items before returning.

        if ((Event = timeforevent()) > 0) {
          destroytimethread();
          destroyeventthread();
          performevent();
        } else if (Event < 0)
          checkforevent();                  //lint !e534
        else {
          #ifdef FIDO
            if (checkfidowindow())
              goto top;
          #endif
        }
        #ifdef FIDO
          if (SaveAllowHumanCallers != Status.AllowHumanCallers)
            goto top;
        #endif
      }

      if (PcbData.Network) {
        #ifdef __OS2__
          // under OS/2 we have a usernet thread that monitors the usernet
          // "attention" flags and if the attention flag is ever set, then we need
          // to call scanusernet() to find out why
          if (needtoscanusernet())
            scanusernet();             //lint !e534

          #ifdef PCBSTATS
            if (! Status.LocalStats && timerexpired(7)) {
              if (Control.Screen && checkstats(&Stats,FALSE))
                updatestatus();
              setcallwaitnettimer();
            }
          #endif
        #else
          if (timerexpired(7)) {
            #ifdef PCBSTATS
              if (! Status.LocalStats && Control.Screen && checkstats(&Stats,FALSE))
                updatestatus();
            #endif
            scanusernet();                      //lint !e534
            setcallwaitnettimer();
          }
        #endif
      }

      #ifdef FIDO
        if (PcbData.EnableFido) {
          bool NeedToGoOnHook = FALSE;

          if (PcbData.FidoDialOut && Asy.ComPortNumber != 0 && PcbData.FidoMailFreq > 0 && timerexpired(9)) {
            if (okaytohandlemail()) {
              if (outboundRec()) {
                SaveBlankerStatus = Control.Screen;
                updatescreen();
                offhook();
                #ifdef __OS2__
                  PauseUpdate = TRUE;
                #endif
                send_outbound_mail(NULL,FROM_TIMER);
                setfidotimer(9,PcbData.FidoMailFreq);
                NeedToGoOnHook = TRUE;
                if (! SaveBlankerStatus)
                  turndisplayoff(FALSE);
              } else {
                // we need to reset the usernet file after okaytohandlemail()
                // normally NeedToGoOnHook would take care of that, but
                // in this case, we've called okaytohandlemail() without setting
                // NeedToGoOnHook to TRUE.
                writeusernetstatus(NOCALLER,NULL);
              }

              setfidotimer(9,PcbData.FidoMailFreq);
            }
          }

          if (PcbData.FidoProcessOut && PcbData.FidoExportFreq > 0 && timerexpired(10)) {
            if (okaytohandlemail()) {
              SaveBlankerStatus = Control.Screen;
              updatescreen();
              offhook();
              #ifdef __OS2__
                PauseUpdate = TRUE;
              #endif
              copy_pcb_messages();  //lint !e534
              setfidotimer(10,PcbData.FidoExportFreq);
              NeedToGoOnHook = TRUE;
              if (! SaveBlankerStatus)
                turndisplayoff(FALSE);
            }
            setfidotimer(10,PcbData.FidoExportFreq);
          }

          if (PcbData.FidoProcessIn && PcbData.FidoPktFreq > 0 && timerexpired(11)) {
            if (okaytohandlemail()) {
              SaveBlankerStatus = Control.Screen;
              updatescreen();
              offhook();
              #ifdef __OS2__
                PauseUpdate = TRUE;
              #endif
              toss_messages();      //lint !e534
              setfidotimer(11,PcbData.FidoPktFreq);
              NeedToGoOnHook = TRUE;
              if (! SaveBlankerStatus)
                turndisplayoff(FALSE);
            }
            setfidotimer(11,PcbData.FidoPktFreq);
          }

          if (NeedToGoOnHook)
            goto top;
        }
      #endif
    }

    if (Status.SysopFlag == 'N') {
      Key = 13;
      Status.SysopFlag = ' ';
      makepcboardsys();
      offhook();
      closemodem(FALSE);
      openlog();
      cleanup();
      return(getlocalpassword());
    }

    if (Status.SysopFlag == 'X' || Status.SysopFlag == 'R' || (PcbData.NoCarrierExit && LastAttemptFailed)) {
      updatescreen();

      Offset = Selection[2][0];
      q = moveselection(q,&Fields[Offset]);

      if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE) {
        Status.SysopFlag = ' ';
        Offset = Selection[1][0];
        q = moveselection(q,&Fields[Offset]);
        goto top;
      }

      Key = 1059;
      if (Status.SysopFlag == 'R')
        Status.ErrorLevel = EXIT_RECYCLE;
      else if (Status.SysopFlag == 'X')
        Status.ErrorLevel = EXIT_NORMAL;
      else
        Status.ErrorLevel = EXIT_NC;
      Status.SysopFlag = ' ';
      makepcboardsys();
      offhook();
      PcbData.OffHook    = FALSE;
      PcbData.ResetModem = FALSE;
      cleanup();
      return(CALLWAIT_EXIT);
    }

    switch (callwaitinkey(ComBuffer)) {
      case 0:
               #ifndef __OS2__
                 // If the screen is toggled off, we don't want to bother asking DOS what
                 // time it is unless it is within the first minute after midnight.  This
                 // is important because if nobody asks DOS (within 24 hours) what time it
                 // is after midnight is crossed, DOS will end up LOSING a day.  It only
                 // remembers ONE midnight crossing at a time.
                 if (Control.Screen || getticks() < ONEMINUTE)
                   updatetime();
                #endif
               // if packetswitch network, normally we wait for a keypress
               // if NoKeyWait is TRUE, the check for online status and
               // process it immediately
               if (PcbData.Packet && Asy.ComPortNumber != 0 && NoKeyWait && online())
                 break;    // go handle the incoming call
               PerformChecks = TRUE;
               continue;   // otherwise, loop back and wait some more
      case 1:  Key = kbdhit(CHECKBUFFER);
               PerformChecks = FALSE;
               goto kbd;
      default: PerformChecks = FALSE;  /* fall thru for comm port data */
    }

    if (Asy.ComPortNumber != 0) {
      if (PcbData.Packet) {
        if (online()) {
          CDokay = 128;
          if (! Status.ForceScreenOff)
            updatescreen();
          Asy.ErrorCorrected = TRUE;
          Asy.ConnectSpeed = (ConnectSpeedOverride != 0 ? ConnectSpeedOverride : PcbData.ModemSpeed);
          Asy.CarrierSpeed = Asy.ConnectSpeed;
          Asy.ModemSpeed   = PcbData.ModemSpeed;
          openlog();
          cleanup();
          return(CALLWAIT_CALLER);
        }
      } else {
        /* check to see if the word "RING" has been added yet */
        checkmodemdata(ComBuffer);
        if (strstr(ComBuffer,"RING") != NULL || (PcbData.AnswerRing && RingDetect)) {
          AnswerOnStartup = FALSE;  // only let it auto-answer once!
          #ifndef MULTIPORT
            RingDetect = FALSE;
          #endif
          /* reset timer so that screen doesn't clear during login process */
          setkbdtimer(3);
          renewkbdtimer();
          setscrncleartimer();
          settimer(4,TENSECONDS);    /* set ten second between-rings counter */
          NumRings++;
          if (NumRings >= 2)
            getcalleridstr(ComBuffer);

          if (NumRings >= PcbData.RingsRequired) {
            ComBuffer[0] = 0;

            if (! Status.ForceScreenOff) {
              updatescreen();
              setkbdtimer(3);
              renewkbdtimer();
            }

            StatusNum = RINGDETECT;
            updatestatus();
            initcommwindow(SHOW);

            tickdelay((PcbData.ModemDelay*HALFSECOND) + QUARTERSECOND);
            slowsendtomodem(AnswerStr);

            if (PcbData.Network)
              writeusernetstatus(LOGINTOSYSTEM,NULL);

            #ifdef __OS2__
              settimer(4,PcbData.Seconds * ONESECOND);
              WaitingForConnect = TRUE;
            #else
              settimer(4,(182L*PcbData.Seconds)/10);  /* number of seconds to wait for carrier */
              settimer(2,ONESECOND);  /* on-screen countdown interval */
            #endif

            tickdelay(ONESECOND);
            checkmodemdata(ComBuffer);

            while (1) {
              Key = kbdhit(CHECKBUFFER);
              if (Key == 27 || Key == 1059 || Key == 1066 || Key == 1068)
                goto kbd;

              if (updateclockstatus()) {
                updatelog(TXT_FULLCOUNTDOWN);
                LastAttemptFailed = TRUE;
                goto top;
              }

              checkmodemdata(ComBuffer);
              if (strstr(ComBuffer,"NO CARRIER") != NULL) {
                updatelog(TXT_NOCARRIER);
                LastAttemptFailed = TRUE;
                goto top;
              }

              if (strstr(ComBuffer,"FAX") != NULL || strstr(ComBuffer,"+FCO") != NULL) {
                getconnectstr(ComBuffer);
                maxstrcpy(Status.DisplayText,Asy.ConnectString,sizeof(Status.DisplayText));
                logsystext(TXT_MODEM,SPACERIGHTAT);
                writelog(LogDivider,LEFTJUSTIFY);
                Status.ErrorLevel = EXIT_FAX;
                Status.Logoff     = RUNFAX;
                cleanup();
                return(CALLWAIT_EXIT);
              }

              if (online()) {
                if (updateclockstatus()) {
                  updatelog(TXT_FULLCOUNTDOWN);
                  LastAttemptFailed = TRUE;
                  goto top;
                }

                CDokay = 128;

//              if (strstr(ComBuffer,"DATA\r") != NULL) {
//                slowsendtomodem("ATO\r");
//                continue;
//              }

                if ((p = strstr(ComBuffer,"CONNECT")) == NULL)
                  continue; /* no CONNECT message yet, loop around for more */

                p += 7;
                settimer(2,FOURSECONDS);

                do {
                  checkmodemdata(ComBuffer);
                } while (strchr(p,'\r') == NULL && ! timerexpired(2));

                tickdelay(QUARTERSECOND);
                checkmodemdata(ComBuffer);
                processconnectstring(ComBuffer);

//              Asy.ErrorCorrected = errorcorrected(ComBuffer);
//              Asy.ModemSpeed     =
//              Asy.CarrierSpeed   = PcbData.ModemSpeed;
//              Asy.HstMode        = (strstr(ComBuffer,"HST") != NULL);
//
//              if (strstr(p,"FAST") != NULL) {
//                Asy.ErrorCorrected = TRUE;
//              } else {
//                if ((Asy.ModemSpeed = getspeed(p)) == 0)
//                  Asy.ModemSpeed = 300;
//                Asy.CarrierSpeed = Asy.ModemSpeed;
//              }
//
//              checkmodemdata(ComBuffer);
//              if (! Asy.ErrorCorrected)
//                Asy.ErrorCorrected = errorcorrected(ComBuffer);
//
//              if ((p = strstr(ComBuffer,"CARRIER")) != NULL) {
//                Asy.CarrierSpeed = getspeed(p+7);
//              } else if (PcbData.FastComm) {
//                if ((p = strchr(ComBuffer,'-')) != NULL && getspeed(p+1) != 0)
//                  Asy.ConnectSpeed = Asy.CarrierSpeed = getspeed(p+1) * 100L;
//                if (strstr(ComBuffer,"BUFF") != NULL)  /* BUFF means it's not error corrected */
//                  Asy.ErrorCorrected = FALSE;
//              } else {
//                /* check for modems that give CONNECT XXXX/ARQ/MNP/YYYY */
//                /* messages, where YYYY is the carrier speed            */
//                if ((p = strrchr(ComBuffer,'/')) != NULL && getspeed(p+1) >= 300)
//                  Asy.CarrierSpeed = getspeed(p+1);
//
//                if (PcbData.LockSpeed || PcbData.ModemSpeed > 9600)
//                  Asy.ModemSpeed = PcbData.ModemSpeed;
//              }
//
//              /* deal with 14.4 connections (the "." was removed up above) */
//              /* by making sure it is not a 300 bps connection, and then   */
//              /* multiplying by 100 to get the full speed                  */
//              if (Asy.CarrierSpeed < 1200 && Asy.CarrierSpeed > 0 && Asy.CarrierSpeed != 300)
//                Asy.CarrierSpeed *= 100;
//
//              if (Asy.ConnectSpeed < 1200 && Asy.ConnectSpeed > 0 && Asy.ConnectSpeed != 300)
//                Asy.ConnectSpeed *= 100;
//
//              getconnectstr(ComBuffer);
//
//              // find out if we're using VMODEM to handle a TELNET call
//              // VMODEM appends "/TEL" to the connect string and adds the
//              // IP address (such as 199.67.41.2) to the end.  We record
//              // the fact that Telnet is being used because we might receive
//              // a CR/NULL pair and we don't want to get stuck waiting in
//              // the commportinkey() function for a NULL/ScanCode pair.
//              if (strstr(ComBuffer,"/TEL") != NULL && strchr(ComBuffer,'.') != NULL)
//                Asy.Telnet = TRUE;
//              #ifdef MULTIPORT
//                else if (PcbData.OS2Driver && istelnetviavmodem())
//                  Asy.Telnet = TRUE;
//              #endif
//
//              /* go set the port speed now */
//              openlog();
//              if (DebugLevel >= 1)
//                reportcom(Asy.ModemSpeed);
//
//              setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
//              Asy.ConnectSpeed = (Asy.ErrorCorrected && Asy.CarrierSpeed >= 9600 ? Asy.ModemSpeed : Asy.CarrierSpeed);
//
//              if (! Asy.ErrorCorrected)       // non-MNP modems should wait
//                tickdelay(FOURSECONDS);       // 4 seconds before continuing
//              else
//                tickdelay(ONESECOND);         // MNP modems continue in 1 sec

                cleanup();
                return(CALLWAIT_CALLER);
              }
            }
          }
        }
      }
    }
    continue;

kbd:
    setkbdtimer(3);
    renewkbdtimer();
    setscrncleartimer();

    if (! Control.Screen) {
      updatescreen();
      switch(Key) {
        case 1059: offhook();
                   if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   PcbData.LeaveDtrUp = TRUE;
                   PcbData.OffHook    = FALSE;
                   PcbData.ResetModem = FALSE;
                   Status.ErrorLevel  = EXIT_NORMAL;
                   cleanup();
                   return(CALLWAIT_EXIT);
        case 1066: tickdelay(ONESECOND);
                   goto top;
        case 1068: PcbData.OffHook    = FALSE;
                   if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   PcbData.ResetModem = FALSE;
                   Status.ErrorLevel  = EXIT_NORMAL;
                   cleanup();
                   return(CALLWAIT_EXIT);
//      default  : updatescreen();
//                 updatetime();
//                 turndisplayon(TRUE);
//                 setcursor(CUR_BLANK);  /* turn it BACK OFF */
//                 #ifdef PCBSTATS
//                   if (! Status.LocalStats && checkstats(&Stats,FALSE))
//                     updatestatus();
//                 #endif
//                 break;
      }
    } else {
      switch (Key) {
        case   13: showhit(q);
                   switch (Offset) {
                     case  0: offhook();
                              closemodem(FALSE);
                              openlog();
                              cleanup();
                              return(CALLWAIT_LOCALUSER);
                     case  1: Status.RingAlert = TRUE;
                              openlog();
                              cleanup();
                              return(CALLWAIT_LOCALUSER);
                     case  2: if (PcbData.ParallelPortNum > 0) {
                                Status.PrintLog = ! Status.PrintLog;
                                Fields[ 2].Text = (PcbData.ParallelPortNum > 0 ? (Status.PrintLog ? Text[16].Str : Text[4].Str) : Text[15].Str);
                                updatestatus();
                                makepcboardsys();
                              }
                              break;
                     case  3: offhook();
                              if (getlocalpassword() == CALLWAIT_RECYCLE)
                                goto top;
                              #ifdef __OS2__
                                PauseUpdate = TRUE;
                              #endif
                              runsysmgr();
                              goto top;
                     case  4: Status.LocalStats = ! Status.LocalStats;
                              Fields[4].Text = (Status.LocalStats ? Text[40].Str : Text[12].Str);
                              updatestats(0,0,0,SETTYPE);
                              checkstats(&Stats,TRUE);     //lint !e534
                              updatestatus();
                              break;
                     case  5: offhook();
                              closemodem(FALSE);
                              openlog();
                              cleanup();
                              return(getlocalpassword());
                     case  6: Status.RingAlert = TRUE;
                              openlog();
                              cleanup();
                              return(getlocalpassword());
                     case  7: Status.PageBell = ! Status.PageBell;
                              Fields[ 7].Text = (Status.PageBell ? Text[18].Str : Text[6].Str);
                              updatestatus();
                              makepcboardsys();
                              break;
                     case  8: offhook();
                              if (getlocalpassword() == CALLWAIT_RECYCLE)
                                goto top;
                              #ifdef __OS2__
                                PauseUpdate = TRUE;
                              #endif
                              runfiler();
                              goto top;
                     case  9: offhook();
                              #ifdef __OS2__
                                PauseUpdate = TRUE;
                              #endif
                              runmoni();
                              goto top;
                     case 10: offhook();
                              PcbData.LeaveDtrUp = TRUE;
                              /* fall thru */
                     case 11: PcbData.OffHook    = FALSE; /*lint !e616 */
                              if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE)
                                goto top;
                              PcbData.ResetModem = FALSE;
                              Status.ErrorLevel  = EXIT_NORMAL;
                              cleanup();
                              return(CALLWAIT_EXIT);
                     case 12: Status.Alarm = ! Status.Alarm;
                              Fields[12].Text = (Status.Alarm ? Text[17].Str : Text[5].Str);
                              updatestatus();
                              makepcboardsys();
                              break;
                     case 13: offhook();
                              if (getlocalpassword() == CALLWAIT_RECYCLE)
                                goto top;
                              destroytimethread();
                              runsetup();
                              updatescreen();
                              recycle();
                              goto top;
                     case 14: updatestats(0,0,0,RESETSTATS);
                              checkstats(&Stats,TRUE);
                              updatestatus();
                              break;
                   }
                   break;
        case   27: PcbData.OffHook    = FALSE;
                   if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   PcbData.ResetModem = FALSE;
                   Status.ErrorLevel  = EXIT_NORMAL;
                   cleanup();
                   return(CALLWAIT_EXIT);
        case 1031: offhook();
                   if (getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   #ifdef __OS2__
                     PauseUpdate = TRUE;
                   #endif
                   runsysmgr();
                   goto top;
        case 1032: offhook();
                   if (getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   #ifdef __OS2__
                     PauseUpdate = TRUE;
                   #endif
                   runfiler();
                   goto top;
        #ifdef FIDO
        case 1033: if (PcbData.EnableFido) {
                     offhook();
                     #ifdef __OS2__
                       PauseUpdate = TRUE;
                     #endif
                     do_fido_menu();
                     setfidotimer( 9,PcbData.FidoMailFreq);
                     setfidotimer(10,PcbData.FidoExportFreq);
                     setfidotimer(11,PcbData.FidoPktFreq);
                   }
                   goto top;
        #endif
        case 1050: offhook();
                   #ifdef __OS2__
                     PauseUpdate = TRUE;
                   #endif
                   runmoni();
                   goto top;
        case 1059: offhook();
                   if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   PcbData.LeaveDtrUp = TRUE;
                   PcbData.OffHook    = FALSE;
                   PcbData.ResetModem = FALSE;
                   Status.ErrorLevel  = EXIT_NORMAL;
                   cleanup();
                   return(CALLWAIT_EXIT);
        case 1060: offhook();
                   closemodem(FALSE);
                   openlog();
                   cleanup();
                   return(CALLWAIT_LOCALUSER);
        case 1061: Status.PrintLog = ! Status.PrintLog;
                   Fields[2].Text = (PcbData.ParallelPortNum > 0 ? (Status.PrintLog ? Text[16].Str : Text[4].Str) : Text[15].Str);
                   updatestatus();
                   displaymenuselections();
                   makepcboardsys();
                   break;
        case 1062: Status.PageBell = ! Status.PageBell;
                   Fields[7].Text = (Status.PageBell ? Text[18].Str : Text[6].Str);
                   updatestatus();
                   displaymenuselections();
                   makepcboardsys();
                   break;
        case 1065: Status.Alarm = ! Status.Alarm;
                   Fields[12].Text = (Status.Alarm ? Text[17].Str : Text[5].Str);
                   updatestatus();
                   displaymenuselections();
                   makepcboardsys();
                   break;
        case 1066: tickdelay(ONESECOND);
                   goto top;
        case 1067: turndisplayoff(TRUE);
                   break;
        case 1068: PcbData.OffHook    = FALSE;
                   if (PcbData.RequirePwrdToExit && getlocalpassword() == CALLWAIT_RECYCLE)
                     goto top;
                   PcbData.ResetModem = FALSE;
                   Status.ErrorLevel  = EXIT_NORMAL;
                   cleanup();
                   return(CALLWAIT_EXIT);
        case 1071: X--;
                   Y--;
                   break;
        case 1072: Y--;
                   break;
        case 1073: X++;
                   Y--;
                   break;
        case 1075: X--;
                   break;
        case 1077: X++;
                   break;
        case 1079: X--;
                   Y++;
                   break;
        case 1080: Y++;
                   break;
        case 1081: X++;
                   Y++;
                   break;
      }

      if (X < 0)
        X = 0;
      else if (X > 2)
        X = 2;

      if (Y < 0)
        Y = 0;
      else if (Y > 4)
        Y = 4;

      Offset = Selection[X][Y];
      q = moveselection(q,&Fields[Offset]);
    }
  }
}
#pragma warn +rvl



#else
#include "project.h"
#pragma hdrstop
#include "event.h"
#include <system.h>

#ifdef __OS2__
  #include <semafore.hpp>
  #include <kbd.hpp>
#endif


calltype LIBENTRY callwait(void) {
  setcursor(CUR_BLANK);
  boxcls(0,0,79,24,ScrnColors->BoxColor,DOUBLE);
  Control.GraphicsMode = TRUE;

  fastprint(13, 5,"ÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ                           ÛÛÛ",ScrnColors->VersionColor);
  fastprint(14, 6, "ÛÛ  ÛÛ ÛÛ   ÛÛ  ÛÛ  ÛÛ                            ÛÛ",ScrnColors->VersionColor);
  fastprint(14, 7, "ÛÛ  ÛÛ ÛÛ       ÛÛ  ÛÛ ÛÛÛÛÛÛ ÛÛÛÛÛÛ  ÛÛÛÛÛÛ      ÛÛ",ScrnColors->VersionColor);
  fastprint(14, 8, "ÛÛÛÛÛÛ ÛÛ       ÛÛÛÛÛ  ÛÛ  ÛÛ     ÛÛ   ÛÛ  ÛÛ ÛÛÛÛÛÛ",ScrnColors->VersionColor);
  fastprint(14, 9, "ÛÛ     ÛÛ       ÛÛ  ÛÛ ÛÛ  ÛÛ ÛÛÛÛÛÛ   ÛÛ  ÛÛ ÛÛ  ÛÛ",ScrnColors->VersionColor);
  fastprint(14,10, "ÛÛ     ÛÛ   ÛÛ  ÛÛ  ÛÛ ÛÛ  ÛÛ ÛÛ  ÛÛ   ÛÛ     ÛÛ  ÛÛ",ScrnColors->VersionColor);
  fastprint(13,11,"ÛÛÛÛ    ÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛÛ    ÛÛÛÛÛÛÛ",ScrnColors->VersionColor);

  fastprint(33,13,         "ÛÛ   ÛÛÛÛÛÛ    ÛÛÛÛÛÛ",ScrnColors->v14Color);
  fastprint(32,14,        "ÛÛÛ   ÛÛ        ÛÛ  ÛÛ",ScrnColors->v14Color);
  fastprint(33,15,         "ÛÛ   ÛÛ            ÛÛ",ScrnColors->v14Color);
  fastprint(24,16,"ÛÛ  ÛÛ   ÛÛ   ÛÛÛÛÛÛ    ÛÛÛÛÛÛ",ScrnColors->v14Color);
  fastprint(24,17,"ÛÛ  ÛÛ   ÛÛ       ÛÛ    ÛÛ"    ,ScrnColors->v14Color);
  fastprint(24,18,"ÛÛ  ÛÛ   ÛÛ   ÛÛ  ÛÛ    ÛÛ  ÛÛ",ScrnColors->v14Color);
  fastprint(24,19,"  ÛÛ   ÛÛÛÛÛÛ ÛÛÛÛÛÛ ÛÛ ÛÛÛÛÛÛ",ScrnColors->v14Color);

/*
  if (ScrnColors->v14Color != 0x07)
    ansi("[6;21H[0;1;30;44mÜ[7CÜ[7CÜ[29CÜ[7;17HÛß[2CÛ[2CÛßß[2CÛ[3CÛß[2CÛ[29CÛ[8;17HÛ[3CÛ[2CÛ[2Cßßß[3CÛ [2CÛ[6CÜ[6CÜ[7CÜ[7CÛ[9;21HÛ[2CÛ[11CÛ [2CÛß[2CÛ[1Cßßß[2CÛ[4CÛß[2CÛ[6CÛ[10;17HÛ"
         "ßßßß[2CÛ[8CÛß[2CÛ[2CÛ[3CÛ[6CÛ[4CÛ[3CÛ[2CÛß[2CÛ[11;17HÛ[6CÛ[8CÛ [2CÛ[2CÛ[3CÛ[2CÛß[2CÛ[4CÛ[1Cßßß[2CÛ [2CÛ[12;18HÜ[10CÛ[7CÛ[6CÛ[7CÜ[4CÜ[10CÜ[13;15Hßßßß[4Cßßßßßßß[1Cßßßßßßß[1Cßßßßßß[1Cßßßß"
         "ßßß[1Cßßßß[4Cßßßßßßß[14;32HÜ[3CÜ[3CÜ[10CÜ[15;32HÛ[3CÛ[3CÛ[6CÛßßßß[16;32HÛ[3CÛ[3CÛ[6CÛ[17;23HÜ[3CÜ[4CÛ[8CÛ[9CÜ[18;23HÛ[3CÛ[4CÛ[2Cßßß[2CÛß[4Cßßß[2CÛ[19;23HÛ[3CÛ[4CÛ[7CÛ[6CÜ[3CÛ"
         "[20;22Hß[2CÛßß[6CÜ[9CÜ[6CÛ[21;24Hßß[3Cßßßßßß[3Cßßßß[1Cßß[1Cßßßßßß");
*/

  fastcenter(24,"  (C) Copyright Clark Development Company, Inc., 1985-1996  ",ScrnColors->BoxColor);

  #ifdef __OS2__
    CTimeoutSemaphore Wait;
    Wait.createunique("CW");
    Wait.settimer(TWOSECONDS);
    kbdregistersemaphore(Wait);
  #else
    settimer(4,TWOSECONDS);
  #endif

  if (PcbData.EventActive) {
    checkforevent();
    if (timeforevent() > 0)
      performevent();
  }

  #ifdef __OS2__
    Wait.timerexpired(SEM_WAIT_NOTIMEOUT);
    kbdunregistersemaphore(Wait);
    Wait.close();
    kbdgetkeyfrombuffer();  // eat any keystrokes that came in
  #else
    while (kbdinkey() == 0 && ! timerexpired(4)) giveup();
  #endif

  return(CALLWAIT_LOCALUSER);
}

#endif
