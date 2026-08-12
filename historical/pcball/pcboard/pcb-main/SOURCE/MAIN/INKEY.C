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
#include "serial.h"
#include <system.h>
#include <ctype.h>

#if defined(CPU386) && defined(MULTIPORT) && ! defined(LIB)
  #include "md5.h"
#endif

#ifdef __WATCOMC__
  #include <io.h>
#endif

#ifdef __OS2__
  #include <threads.h>
  #include <kbd.hpp>
  #include "sem.hpp"
#endif

#ifdef FIDO
//#include <messages.h>
  #include <defines.h>
  #include <structs.h>
  #include <prototyp.h>
#endif

/*    For PCBOARD use only...

      REMOTE:    ESC[0tESC[12346;t

                 Added around 5/4/93

      LOCAL :    Log in locally, hold down ALT and spell out PIRATE, then
                 end by pressing ALT-P once again

                 Added on 9/01/94
*/

#ifdef LIB
  #include "usersys.h"
#endif

#ifdef __OS2__
  static long LastKbdTime;     // last time a key was pressed (in 100ths of second)
  static long KbdTimeOutTime;  // time when kbd timeout was noticed
//static bool ForceLogoff;     // checktime() sets this, waitforkey() resets it
#endif

#ifndef __OS2__
  #define bgetkey(c) bgetkey2(c)
#endif

#define SECOND10          1*182
#define SECOND20          2*182
#define SECOND30          3*182
#define SECOND40          4*182
#define SECOND50          5*182
#define SECOND60          6*182

#define KBDBUFSIZE  256 /* leave this at 256! otherwise if() will be         */
                        /* necessary to rollover the read and write pointers */
                        /* when it reaches the end of the buffer             */

enum { F1=1,F2,F3,F4,F5,F6,F7,F8,F9,F10,SF1,SF2,SF3,SF4,SF5,SF6,SF7,SF8,SF9,SF10,
       HOMEKEY,UPKEY,PGUPKEY,LEFTKEY,RIGHTKEY,ENDKEY,DOWNKEY,PGDNKEY,INSKEY,DELKEY,
       ALTA, ALTB, ALTD, ALTE, ALTF, ALTH, ALTI, ALTM, ALTN, ALTO, ALTP, ALTR, ALTS, ALTT, ALTU, ALTX, ALTY,
       AF1, AF2, AF9, AF10 };

#ifndef LIB
#define LOWNUM       1018
#define HIGHNUM      1113
#define TABLESIZE      96

static char translatetable1[TABLESIZE] = {    /* normal translate table */
  /* 1018 */       ALTE,
  /* 1019 */       ALTR,
  /* 1020 */       ALTT,
  /* 1021 */       ALTY,
  /* 1022 */       ALTU,
  /* 1023 */       ALTI,
  /* 1024 */       ALTO,
  /* 1025 */       ALTP,
  /* 1026-1029 */  0,0,0,0,
  /* 1030 */       ALTA,
  /* 1031 */       ALTS,
  /* 1032 */       ALTD,
  /* 1033 */       ALTF,
  /* 1034 */       0,
  /* 1035 */       ALTH,
  /* 1036-1044 */  0,0,0,0,0,0,0,0,0,
  /* 1045 */       ALTX,
  /* 1046-1047 */  0,0,
  /* 1048 */       ALTB,
  /* 1049 */       ALTN,
  /* 1050 */       ALTM,
  /* 1051-1058 */  0,0,0,0,0,0,0,0,
  /* 1059 */       F1,
  /* 1060 */       F2,
  /* 1061 */       F3,
  /* 1062 */       F4,
  /* 1063 */       F5,
  /* 1064 */       F6,
  /* 1065 */       F7,
  /* 1066 */       F8,
  /* 1067 */       F9,
  /* 1068 */       F10,
  /* 1069-1070 */  0,0,
  /* 1071 */       HOMEKEY,
  /* 1072 */       UPKEY,
  /* 1073 */       PGUPKEY,
  /* 1074 */       0,
  /* 1075 */       LEFTKEY,
  /* 1076 */       0,
  /* 1077 */       RIGHTKEY,
  /* 1078 */       0,
  /* 1079 */       ENDKEY,
  /* 1080 */       DOWNKEY,
  /* 1081 */       PGDNKEY,
  /* 1082 */       INSKEY,
  /* 1083 */       DELKEY,
  /* 1084 */       SF1,
  /* 1085 */       SF2,
  /* 1086 */       SF3,
  /* 1087 */       SF4,
  /* 1088 */       SF5,
  /* 1089 */       SF6,
  /* 1090 */       SF7,
  /* 1091 */       SF8,
  /* 1092 */       SF9,
  /* 1093 */       SF10,
  /* 1094 */       AF1,           /* it's really CTRL-F1  */
  /* 1095 */       AF2,           /* it's really CTRL-F2  */
  /* 1096-1101 */  0,0,0,0,0,0,
  /* 1102 */       AF9,           /* it's really CTRL-F9  */
  /* 1103 */       AF10,          /* it's really CTRL-F10 */
  /* 1104 */       AF1,
  /* 1105 */       AF2,
  /* 1106-1111 */  0,0,0,0,0,0,
  /* 1112 */       AF9,
  /* 1113 */       AF10,
};

static char translatetable2[TABLESIZE] = {  /* translate table during file transfer */
  /* 1018 */       0,
  /* 1019 */       ALTR,
  /* 1020-1035 */  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 1036 */       ALTH,
  /* 1037-1044 */  0,0,0,0,0,0,0,0,0,
  /* 1045 */       ALTX,
  /* 1046-1048 */  0,0,0,
  /* 1049 */       ALTN,
  /* 1050-1059 */  0,0,0,0,0,0,0,0,0,0,
  /* 1060 */       F2,
  /* 1061 */       F3,
  /* 1062 */       F4,
  /* 1063-1064 */  0,0,
  /* 1065 */       F7,
  /* 1066 */       F8,
  /* 1067 */       F9,
};

static char translatetable3[TABLESIZE] = {  /* translate table for /LOCALON */
  /* 1018 */       0,
  /* 1019 */       ALTR,
  /* 1020 */       ALTT,
  /* 1021-1022 */  0,0,
  /* 1023 */       ALTI,
  /* 1024 */       0,
  /* 1025 */       ALTP,
  /* 1026-1032 */  0,0,0,0,0,0,0,
  /* 1033 */       ALTF,
  /* 1034-1049 */  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 1050 */       ALTM,
  /* 1051-1062 */  0,0,0,0,0,0,0,0,0,0,0,0,
  /* 1063 */       F5,
  /* 1064-1070 */  0,0,0,0,0,0,0,
  /* 1071 */       HOMEKEY,
  /* 1072 */       UPKEY,
  /* 1073 */       PGUPKEY,
  /* 1074 */       0,
  /* 1075 */       LEFTKEY,
  /* 1076 */       0,
  /* 1077 */       RIGHTKEY,
  /* 1078 */       0,
  /* 1079 */       ENDKEY,
  /* 1080 */       DOWNKEY,
  /* 1081 */       PGDNKEY,
  /* 1082 */       INSKEY,
  /* 1083 */       DELKEY,
  /* 1084 */       SF1,
  /* 1085 */       SF2,
  /* 1086 */       SF3,
  /* 1087 */       SF4,
  /* 1088 */       SF5,
  /* 1089 */       SF6,
  /* 1090 */       SF7,
  /* 1091 */       SF8,
  /* 1092 */       SF9,
  /* 1093 */       SF10, /* the rest are zeroes */
};
#else

/* library constants */

#define LOWNUM   1035
#define HIGHNUM  1083

static char translatetable[49] = {    /* normal translate table */
  /* 1035 */       ALTH,
  /* 1036-1044 */  0,0,0,0,0,0,0,0,0,
  /* 1045 */       ALTX,
  /* 1046-1047 */  0,0,
  /* 1048 */       ALTB,
  /* 1049 */       ALTN,
  /* 1050 */       0,
  /* 1051-1058 */  0,0,0,0,0,0,0,0,
  /* 1059 */       0,        /* F1 */
  /* 1060 */       F2,
  /* 1061-1062 */  0,0,      /* F3-F4 */
  /* 1063 */       F5,
  /* 1064-1065 */  0,0,      /* F6-F7 */
  /* 1066 */       F8,
  /* 1067 */       F9,
  /* 1068 */       F10,
  /* 1069-1070 */  0,0,
  /* 1071 */       HOMEKEY,
  /* 1072 */       UPKEY,
  /* 1073 */       PGUPKEY,
  /* 1074 */       0,
  /* 1075 */       LEFTKEY,
  /* 1076 */       0,
  /* 1077 */       RIGHTKEY,
  /* 1078 */       0,
  /* 1079 */       ENDKEY,
  /* 1080 */       DOWNKEY,
  /* 1081 */       PGDNKEY,
  /* 1082 */       INSKEY,
  /* 1083 */       DELKEY
};
#endif

short   static KbdBuffer[KBDBUFSIZE];
char    static KbdHeadPtr = 0;
char    static KbdTailPtr = 0;
char    static KbdBufCount;

short   static PPLBuffer[KBDBUFSIZE];
char    static PPLHeadPtr = 0;
char    static PPLTailPtr = 0;
char    static PPLBufCount;

#ifndef LIB
bool    static AutoStuff = FALSE;
bool    static RemoveStuffer = FALSE;
DOSFILE static StuffFile;

#endif


#ifdef COMM
#define MAXSAVEDBYTES 40

static int  NumSavedBytes;
static int  SavedBytes[MAXSAVEDBYTES];

#if defined(CPU386) && defined(MULTIPORT) && ! defined(LIB)
enum {MD5INIT, MD5ABORT, MD5DONE};
static char Md5Stage;
#endif


static void _NEAR_ LIBENTRY addsavedbyte(int C) {
  /* don't add -1 values */
  if (C == -1)
    return;
  /* record valid bytes if there is enough room */
  if (NumSavedBytes < MAXSAVEDBYTES)
    SavedBytes[NumSavedBytes++] = C;
}


static int _NEAR_ LIBENTRY removesavedbyte(void) {
  int Top;
  int X;

  Top = SavedBytes[0];                    /* remember the first byte */
  for (X = 0; X < NumSavedBytes-1; X++)   /* rotate the rest down a slot */
    SavedBytes[X] = SavedBytes[X+1];

  SavedBytes[NumSavedBytes-1] = 0;        /* zero out the last one */
  NumSavedBytes--;                        /* reduce the count by one */
  return(Top);                            /* return the first byte */
}


static void _NEAR_ LIBENTRY clearsavedbytes(void) {
  memset(SavedBytes,0,MAXSAVEDBYTES);
  NumSavedBytes = 0;
  #if defined(CPU386) && defined(MULTIPORT) && ! defined(LIB)
    Md5Stage = MD5INIT;
  #endif
}
#endif


void LIBENTRY initkbdbuffers(void) {
  KbdHeadPtr  = 0;
  KbdTailPtr  = 0;
  KbdBufCount = 0;
  PPLHeadPtr  = 0;
  PPLTailPtr  = 0;
  PPLBufCount = 0;
  memset(KbdBuffer,0,KBDBUFSIZE * sizeof(short));
  memset(PPLBuffer,0,KBDBUFSIZE * sizeof(short));
  #ifdef COMM
    clearsavedbytes();
  #endif
}


#if ! (defined(PCB_DEMO) || defined(LIB))
static void _NEAR_ LIBENTRY showserial(void  (LIBENTRY *show)(char *Str)) {
  unsigned    X;
  char       *p;
  char        Str[10];
  serialtype  Serial;

  setcollect(SERIAL);
  for (X = 0, p = (char *) &Serial; X < sizeof(serialtype); X++, p++)
    *p = collect();

  show("\r\n");
  show("\r\n");
  show(juliantodate(Serial.Date));
  show("\r\n");
  Serial.Date = 0;
  show(Serial.Num);
  show("\r\n");
  ascii(Str,Serial.Version);
  show(Str);
  show("\r\n");
  ascii(Str,Serial.NumNodes);
  show(Str);
  show("\r\n");
  show(Serial.CustName);
  show("\r\n");
  memset(&Serial,0,sizeof(Serial));

  setcollect(SERIALENCRYPTED);
  for (X = 0, p = (char *) &Serial; X < sizeof(serialtype); X++, p++)
    *p = collect();

  decrypt2((char *) &Serial,sizeof(serialtype));
  show("\r\n");
  show(juliantodate(Serial.Date));
  show("\r\n");
  Serial.Date = 0;
  show(Serial.Num);
  show("\r\n");
  ascii(Str,Serial.Version);
  show(Str);
  show("\r\n");
  ascii(Str,Serial.NumNodes);
  show(Str);
  show("\r\n");
  show(Serial.CustName);
  show("\r\n");
  memset(&Serial,0,sizeof(Serial));
}
#endif


/********************************************************************
*
*  Function:  warntime()
*
*  Desc    :  Displays a warning on the screen when session time is about
*             to expire.  It is called only if checktime() has set the
*             Control.WarnMinute to something other than 0.
*/

void LIBENTRY warntime(void) {
  bool static Warning;

  if (Warning) {  /* make sure we don't go recursive */
    Control.WarnMinute = 0;
    return;
  }

  /* in case the time avail has jumped back up (due to */
  /* a time credit) don't show the warning message     */

  if (gettimer(5) > (((long) Control.WarnMinute - '0') * ONEMINUTE)) {
    Control.WarnMinute = 0;  /* keep it from trying again! */
    return;
  }

  Warning = TRUE;
  bell();
  Status.DisplayText[0] = Control.WarnMinute;
  Status.DisplayText[1] = 0;
  Control.WarnMinute = 0;
  showmessage(TXT_AUTODISCONNECT);
  Warning = FALSE;
}


/********************************************************************
*
*  Function:  wastetime()
*
*  Desc    :  Makes sure that the current clock tick passes before continuing.
*             This avoids having the bell sound off twice for the same clock
*             tick.
*/

#ifndef __OS2__
static void _NEAR_ LIBENTRY wastetime(int Timer, int Tick) {
  /* make sure that the current time is way out of range for the clock tick */
  /* we're supposed to be letting expire - don't want to get stuck in here  */
  /* for a long time waiting for an invalid timer to expire                 */

  if (gettimer(Timer) > Tick + HALFSECOND)
    return;

  Tick--;
  while(gettimer(Timer) >= Tick)
    giveup();
}
#endif


/********************************************************************
*
*  Function:  warningbells()
*
*  Desc    :  sounds the alarm right on a 10-second interval
*/

#pragma argsused
static void _NEAR_ LIBENTRY warningbells(int Timer, int Tick) {
  switch (Tick) {
    case SECOND10:
    case SECOND20:
    case SECOND30:
    case SECOND40:
    case SECOND50:
    case SECOND60: bell();
                   #ifndef __OS2__
                     wastetime(Timer,Tick);
                   #endif
  }
}


/********************************************************************
*
*  Function:  checktime()
*
*  Desc    :  this function only gets called if we have less than three
*             minutes left on the clock and only if we are NOT in the middle
*             of a file transfer.
*
*  Returns :
*/

#ifdef __OS2__
  #define FUDGEFACTOR TENSECONDS
#else
  #define FUDGEFACTOR HALFSECOND
#endif

void LIBENTRY checktime(void) {
  int ShortTimer;

  ShortTimer = (int) gettimer(5);
  if (ShortTimer >= THREEMINUTES-FUDGEFACTOR)  {
    Control.WarnMinute = '3';
    #ifndef __OS2__
      wastetime(5,THREEMINUTES-FUDGEFACTOR);
    #endif
    return;
  }
  if (ShortTimer <= TWOMINUTES) {
    if (ShortTimer >= TWOMINUTES-FUDGEFACTOR)  {
      Control.WarnMinute = '2';
      #ifndef __OS2__
        wastetime(5,TWOMINUTES-FUDGEFACTOR);
      #endif
      return;
    }
    if (ShortTimer <= ONEMINUTE) {
      if (ShortTimer >= ONEMINUTE-FUDGEFACTOR)  {
        Control.WarnMinute = '1';
        #ifndef __OS2__
          wastetime(5,ONEMINUTE-FUDGEFACTOR);
        #endif
        return;
      }
      warningbells(5,ShortTimer);
      if (ShortTimer < 0) {
        Control.WatchKbdClock     = FALSE; /* don't check the time again since    */
        Control.WatchSessionClock = FALSE; /* we're in the process of logging off */
        #ifdef COMM
          clearandwaitformodemempty();
        #endif
        Status.SessionTimedOut = TRUE;
        #ifdef __OS2__
//        ForceLogoff = TRUE;
        #else
          Display.ShowOnScreen  = TRUE;
          Display.AbortPrintout = FALSE;
          freshline();
          displaypcbtext(TXT_TIMELIMITEXCEEDED,NEWLINE|LFBEFORE|LOGIT|BELL);
          loguseroff(ALOGOFF);
        #endif
        return;
      }
    }
  }
}


/********************************************************************
*
*  Function:  checkkbdtime()
*
*  Desc    :  this function only gets called if we have less than one
*             minute left on the keyboard timer
*
*  Returns :  -1 if it attempted to recycle but couldn't otherwise 0
*/

static int _NEAR_ LIBENTRY checkkbdtime(void) {
  #ifndef __OS2__
    Status.KbdTimedOut = timerexpired(1);
  #endif

  if (Status.KbdTimedOut) {
    Control.WatchKbdClock     = FALSE; /* don't check the time again since    */
    Control.WatchSessionClock = FALSE; /* we're in the process of logging off */
    #ifdef COMM
      clearandwaitformodemempty();
    #endif
    Display.ShowOnScreen  = TRUE;
    Display.AbortPrintout = FALSE;
    freshline();
    displaypcbtext(TXT_KBDTIMEEXPIRED,NEWLINE|LFBEFORE|BELL | (Status.UserRecNo != 0 ? LOGIT : DEFAULTS));
    #ifndef LIB
      if (Status.EnteringMessage) {
        /* entering a message or a file description when kbd timer ran out */
        /* drop DTR to hangup, then return -1 to signify carrier was lost  */
        #ifdef COMM
          if (Asy.Online == REMOTE) {
            waitforempty(SECOND10);
            Asy.IgnoreCDLoss = TRUE;
            turnoffdtr();
            initcommwindow(HIDE);
            modemoffhook();
          }
        #endif
        return(-1);
      } else {
        loguseroff(ALOGOFF);
        return(-1);         /* doesn't return -1 unless logoff fails (unlikely) */
      }
    #else
      loguseroff(ALOGOFF);
      return(-1);         /* doesn't return -1 unless logoff fails (unlikely) */
    #endif
  }

  #ifndef __OS2__
    warningbells(1,(int) gettimer(1));
  #endif
  return(0);
}


void LIBENTRY renewkbdtimer(void) {
  if (Control.WatchKbdClock) {
    #ifdef __OS2__
      LastKbdTime = dosgetlongtime();
    #else
      settimer(1,Control.KbdTimer);
    #endif
  }
}


/********************************************************************
*
*  Function:  turnkbdtimeron()
*
*  Desc    :  This function will ONLY turn the keyboard timer on if the time
*             value is NON-zero.  Otherwise it purposely turns the timer off
*             so that the routine that checks the time doesn't have to ALSO
*             check to see if Control.KbdTimer is zero.
*/

void LIBENTRY turnkbdtimeron(void) {
  if (Control.KbdTimer != 0) {
    Control.WatchKbdClock = TRUE;
    renewkbdtimer();
  } else {
    Control.WatchKbdClock = FALSE;
  }
}


/********************************************************************
*
*  Function:  clearkbdbuffer(void);
*
*  Desc    :  clears the keyboard buffer by reseting the head and tail ptr's
*/

void LIBENTRY clearkbdbuffer(void) {
  KbdTailPtr  = 0;
  KbdHeadPtr  = 0;
  KbdBufCount = 0;
}


#ifndef LIB
bool LIBENTRY filestuffinuse(void) {
  return(AutoStuff);
}

void LIBENTRY closeautostuff(void) {
  if (AutoStuff) {
    dosfclose(&StuffFile);
    AutoStuff = FALSE;
    if (RemoveStuffer)
      unlink("PCBSTUFF.KBD");
  }
}


/********************************************************************
*
*  Function:  stuffit()
*
*  Desc    :  If AutoStuff is turned on then kbdinkey calls this routine to
*             get the next batch of keystrokes.
*/

static void _NEAR_ LIBENTRY stuffit(void) {
  char Str[KBDBUFSIZE];
  char *p;

  if (dosfgets(Str,KBDBUFSIZE,&StuffFile) != -1) {
    if (Str[0] == 0) {
      Str[0] = ' ';
      Str[1] = 0;
    }
    for (p = Str; *p != 0; p++) {
      KbdBuffer[KbdHeadPtr] = *p;
      KbdHeadPtr++;   /* it's a char - automatically rolls over to 0 after 255 */
      KbdBufCount++;
    }
    KbdBuffer[KbdHeadPtr] = 13;
    KbdHeadPtr++;   /* it's a char - automatically rolls over to 0 after 255 */
    KbdBufCount++;
  } else closeautostuff();
}


/********************************************************************
*
*  Function:  stuffkbdwithfile()
*
*  Desc    :  Sets up the keyboard buffer to "auto-stuff itself" with the
*             contents of a file.
*
*  Returns :  0 if it successfully opens the file, -1 if there was an error
*/

void LIBENTRY stuffkbdwithfile(char *FileName) {
  if (AutoStuff)  /* don't allow redundant calls */
    return;

  if (fileexist(FileName) == 255 || (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&StuffFile) == -1)) {
    AutoStuff = FALSE;
    return;
  }

  RemoveStuffer = (bool) (strcmp(FileName,"PCBSTUFF.KBD") == 0);
  AutoStuff = TRUE;
  #ifdef __OS2__
    releasekbdblock(0);
  #endif
}


/********************************************************************
*
*  Function:  getlocalinput()
*
*  Desc    :  Puts a box up at the top of the screen to ask the local user for
*             input.   When enter is pressed the screen is restored.
*
*  Returns :  The variable Str is filled with the user's input.
*/

void LIBENTRY getlocalinput(char *Str, int TextNum, int FieldLen, char *Mask) {
  bool static  InHere;
  #ifdef PCBCOMM
  bool         SaveTerse;
  #endif
  char         X,Y;
  char         OldColor;
  bool         OldUseAnsi;
  bool         OldGraphicsMode;
  int          Saved;
  char         Color;
  onlinetype   SaveOnline;
  DISPLAYTYPE  DispType;
  pcbtexttype  Buf;

  if (InHere)
    return;

  InHere = TRUE;
  SaveOnline = Asy.Online;
  OldUseAnsi = UseAnsi;
  OldGraphicsMode = Control.GraphicsMode;
  Asy.Online = LOCAL;
  UseAnsi = TRUE;

  #ifdef PCBCOMM
    SaveTerse  = Status.TerseMode;
    Status.TerseMode = FALSE;
  #endif

  Control.GraphicsMode = TRUE;
  Saved = memsavescreen();
  X = awherex();
  Y = awherey();
  Color = (char) ((Display.DefaultColor & 0xF0) + 0x07);
  boxcls(0,0,79,2,Color,SINGLE);
  agotoxy(2,1);
  if (FieldLen == 0) {
    getpcbtext(TextNum,&Buf);
    FieldLen = 80-8-strlen(Buf.Str);
  }
  DispType = UPCASE|FIELDLEN|AUTO;
  if (TextNum == TXT_LOCALPASSWORD)
    DispType |= ECHODOTS;
  OldColor = curcolor();
  inputfield(Str,TextNum,FieldLen,DispType,NOHELP,Mask);
  printcolor(OldColor);
  memrestorescreen(Saved,FREESCREEN);
  agotoxy(X,Y);
  Asy.Online = SaveOnline;
  Control.GraphicsMode = OldGraphicsMode;
  UseAnsi = OldUseAnsi;
  InHere = FALSE;

  #ifdef PCBCOMM
    Status.TerseMode = SaveTerse;
  #endif
}
#endif


/********************************************************************
*
*  Function:  importfile()
*
*  Desc    :  gets a filename from the LOCAL keyboard/screen and sets up
*             for automatic keyboard stuffing
*/

#ifndef LIB
static void _NEAR_ LIBENTRY importfile(void) {
  char Str[80];

  Str[0] = 0;
  getlocalinput(Str,TXT_INPUTFILENAME,0,mask_filename);
  if (Str[0] != 0)
    stuffkbdwithfile(Str);
}


/********************************************************************
*
*  Function:  exportfile()
*
*  Desc    :  gets a filename from the LOCAL keyboard/screen and sets up
*             screen capture mode
*/

static void _NEAR_ LIBENTRY exportfile(void) {
  char Str[80];

  if (Status.CapFile.handle != 0) { /* check the handle cuz Status.Capture   */
    closecapturefile();             /* might be off temporarily              */
    Status.CapFileName[0] = 0;      /* now make sure you FORGET it!          */
    return;
  }

  Str[0] = 0;
  getlocalinput(Str,TXT_OUTPUTFILENAME,0,mask_filename);
  if (Str[0] != 0)
    if (opencapturefile(Str) != -1)
      Status.CapFileExport = TRUE;
}
#endif


#ifndef LIB
static void _NEAR_ LIBENTRY disconnectcaller(void) {
  char Str[2];

  Str[0] = SystemNoChar;
  Str[1] = 0;
  getlocalinput(Str,TXT_DISCONNECTNOW,1,mask_systemyesno);
  if (Str[0] == SystemYesChar) {
    Display.ShowOnScreen  = TRUE;
    Display.AbortPrintout = FALSE;
    displaypcbtext(TXT_AUTODISCONNECTNOW,NEWLINE|LFBEFORE);
    Status.Logoff = ALOGOFF;
    loguseroff(ALOGOFF);
  }
}
#endif


#ifdef COMM
#ifndef LIB
static void _NEAR_ LIBENTRY lockcallerout(void) {
  char Str[2];

  if (Asy.Online == REMOTE) {
    Str[0] = SystemNoChar;
    Str[1] = 0;
    getlocalinput(Str,TXT_LOCKCALLEROUT,1,mask_systemyesno);
    if (Str[0] == SystemYesChar) {
      Display.ShowOnScreen  = TRUE;
      Display.AbortPrintout = FALSE;
      displaypcbtext(TXT_AUTOMATICLOCKOUT,NEWLINE|LFBEFORE|LOGIT);
      UsersData.SecurityLevel = 0;
      displaypcbtext(TXT_AUTODISCONNECTNOW,NEWLINE|LFBEFORE);
      Status.Logoff = ALOGOFF;
      loguseroff(ALOGOFF);
    }
  }
}
#endif


#endif


#ifndef LIB
static void _NEAR_ LIBENTRY sysopprivileges(void) {
  char Str[2];

  if (UsersData.Name[0] == 0)  /* don't do it until he has signed on */
    return;

  /* don't bother him if he already has sysop privileges */
  if (Status.TempAddSec == 0 && Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL])
    return;

  if (Status.CurSecLevel < PcbData.SysopSec[SEC_SYSOPLEVEL]) {
    Str[0] = SystemNoChar;
    Str[1] = 0;
    getlocalinput(Str,TXT_GIVESYSOPPRIV,1,mask_systemyesno);
    if (Str[0] != SystemYesChar)
      return;
  }

  togglesysopprivileges();
}


/********************************************************************
*
*  Function: callwaitinkey()
*
*  - Checks the number of bytes in the comm buffer
*  - Calls bgetkey() to read the keyboard
*  - Waits for a key stroke, a character to come in from the comm port or
*    for one second to pass
*  - returns:
*      -1 = exited due to data in the comm buffer
*       0 = exited to update the clock
*       1 = exited due to data in the keyboard buffer
*/

#ifdef COMM
int LIBENTRY callwaitinkey(char *ComBuffer) {
  int CommKey;

  if (AnswerOnStartup)
    return(-1);

  #ifdef __OS2__
    // we have to check here, before we get stuck waiting for an event, to
    // see if there is anything already in the buffer to be processed
    if ((CommKey = comminkey()) != -1) {
      addchar(ComBuffer,CommKey);
      Status.Kbd = FALSE;
      return(-1);
    }

    if (KbdBufCount != 0 || kbdcount() != 0)
      return(1);

    // there was nothing already waiting for us, so NOW we can plant ourselves
    // in the waitforevent() function and sit here for awhile
    WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
    WaitKeySem.reset();

    // now that the event has triggered, we need to check to see if it was
    // because of keyboard or comm port data
    if ((CommKey = comminkey()) != -1) {
      addchar(ComBuffer,CommKey);
      Status.Kbd = FALSE;
      return(-1);
    }

    if (KbdBufCount != 0 || kbdcount() != 0)
      return(1);

    // there was nothing really waiting, so maybe an event was posted to
    // force us to cycle through the call waiting processing
    return(0);
  #else
    settimer(2,ONESECOND);
    while(1) {
      CommKey = comminkey();
      if (CommKey != -1) {
        addchar(ComBuffer,CommKey);
        Status.Kbd = FALSE;
        return(-1);
      }

      if (KbdBufCount != 0)   /* check our keyboard buffer */
        return(1);

      if (bgetkey(1))
        return(1);

      /* if timer2 has expired then return back to the */
      /* calling routine to update the clock display   */
      if (timerexpired(2))
        return(0);

      /* if timer1 has expired and it's okay to clear */
      /* the screen then turn the display off         */
      if (Control.Screen && (! PcbData.DisableCls) && timerexpired(12))
        turndisplayoff(FALSE);

      giveup();
      giveup();
    }
  #endif  /* ifdef __OS2__ */
}
#endif  /* ifdef COMM */
#endif


/********************************************************************
*
*  Function:  stuffbuffer()
*
*  Desc    :  stuffs the keyboard buffer a Key or with keystrokes from
*             PCBOARD.DAT for shifted function keys
*/

void LIBENTRY stuffbuffer(int Key) {
#ifndef LIB
  int  Special;
  char *p;
  char Temp[sizeof(PcbData.FuncKeys[0])];

  if (Key >= LOWNUM && Key <= HIGHNUM) {
    Special = translatetable1[Key-LOWNUM];
    if (Special >= SF1 && Special <= SF10) {
      maxstrcpy(Temp,PcbData.FuncKeys[Special-SF1],sizeof(Temp));
      p = Temp;

      if (*(p+1) != 0) {
        switch (*p) {
          case '%': stuffkbdwithfile(p+1);
                    return;
          case '!': runscriptwithparams(p+1);    /*lint !e534 */
                    return;
        }
      }

      for ( ; *p != 0; p++) {
        if (*p == '^' && *(p+1) >= 'A' && *(p+1) <= '[') {
          p++;
          KbdBuffer[KbdHeadPtr] = *p - '@';
        } else
          KbdBuffer[KbdHeadPtr] = *p;

        KbdHeadPtr++;   /* it's a char - automatically rolls over to 0 after 255 */
        KbdBufCount++;
      }
      #ifdef __OS2__
        releasekbdblock(0);
      #endif
      return;
    }
  }
#endif

  KbdBuffer[KbdHeadPtr] = (short) Key;
  KbdHeadPtr++;   /* it's a char - automatically rolls over to 0 after 255 */
  KbdBufCount++;
  #ifdef __OS2__
    releasekbdblock(0);
  #endif
}


#ifndef LIB
char LIBENTRY bytesleftinkbdbuffer(void) {
  return(KbdBufCount);
}


char LIBENTRY bytesleftinpplbuffer(void) {
  return(PPLBufCount);
}


void LIBENTRY clearpplbuffer(void) {
  PPLTailPtr  = 0;
  PPLHeadPtr  = 0;
  PPLBufCount = 0;
}


void LIBENTRY stuffbufferstr(char *Str) {
  for (; *Str; Str++) {
    if (*Str == '^' && *(Str+1) >= 'A' && *(Str+1) <= '[') {
      Str++;
      stuffbuffer(*Str - '@');
    } else
      stuffbuffer(*Str);
  }
}


void LIBENTRY stuffpplbuffer(char *Str) {
  int C;

  for (; *Str; Str++) {
    if (*Str == '^' && *(Str+1) >= 'A' && *(Str+1) <= '[') {
      Str++;
      C = *Str - '@';
    } else
      C = *Str;

    PPLBuffer[PPLHeadPtr] = (short) C;
    PPLHeadPtr++;   /* it's a char - automatically rolls over to 0 after 255 */
    PPLBufCount++;
  }
}
#endif


#ifndef LIB
static bool _NEAR_ LIBENTRY localpasswordfailed(void) {
  char Str[13];

  if (PcbData.Password[0] == 0)
    return(FALSE);

  Str[0] = 0;
  getlocalinput(Str,TXT_LOCALPASSWORD,12,mask_alphanum);
  return((bool) (strcmp(Str,PcbData.Password) != 0));
}
#endif


/********************************************************************
*
*  Function:  processkeys()
*
*  Desc    :  takes a partial-processed keys and processes them
*/

static int _NEAR_ LIBENTRY processkeys(int KeyNum) {
  #if ! (defined(LIB) || defined(PCB_DEMO))
    static bool SpellP;
    static bool SpellI;
    static bool SpellR;
    static bool SpellA;
    static bool SpellT;
    static bool SpellE;
  #endif
  int Special;

  #ifndef LIB
    if (Status.FileXfer)
      Special = translatetable2[KeyNum-LOWNUM];
    else if (LocalOn)
      Special = translatetable3[KeyNum-LOWNUM];
    else
      Special = translatetable1[KeyNum-LOWNUM];
  #else
    Special = translatetable[KeyNum-LOWNUM];
  #endif

  switch (Special) {
    case 0      : /* ignore keys that the translate table zeroed out */
                  break;
    #ifndef PCB_DEMO
    #ifndef LIB
    case ALTA   : if (SpellR) {
                    SpellR = FALSE;
                    SpellA = TRUE;
                  }
                  return(KeyNum);
    #endif
    #endif
    case ALTB   : bell();
                  break;
    #ifndef LIB
    case ALTD   : if (localpasswordfailed())
                    break;
                  runfiler();
                  #ifdef COMM
                    if (Asy.Online == REMOTE && ! Status.InChat && Status.FullScreen)
                       return(12);   /* force CTRL-L to redisplay the screen */
                  #endif
                  break;
    case ALTE   :
                  #ifndef PCB_DEMO
                    if (SpellT) {
                      SpellT = FALSE;
                      SpellE = TRUE;
                    }
                  #endif
                  return(KeyNum);
    case ALTF   : exportfile();
                  redisplaystatusline();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    #endif
    #ifdef COMM
    case ALTH   : switch (Status.StatLine) {
                    case NONE      : return(0);
                    case NAME      : Status.StatLine = HELP;       break;
                    case HELP      : Status.StatLine = COMMENT;    break;
                    case COMMENT   : Status.StatLine = CONFREG;    break;
                    case CONFREG   : Status.StatLine = (Asy.ComPortNumber == 0 ? NAME : MODEMSTATS); break;
                    case MODEMSTATS: Status.StatLine = NAME;       break;
                  }
                  showstatusline();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    #endif
    #ifndef LIB
    case ALTI   : importfile();
                  #ifndef PCB_DEMO
                    if (SpellP) {
                      SpellP = FALSE;
                      SpellI = TRUE;
                    }
                  #endif
                  break;
    case ALTM   : runmoni();
                  #ifdef COMM
                    if (Asy.Online == REMOTE && ! Status.InChat && Status.FullScreen)
                       return(12);   /* force CTRL-L to redisplay the screen */
                  #endif
                  break;
    #endif
    case ALTN   : Status.SysopFlag = (Status.SysopFlag == 'N' ? ' ' : 'N');
                  redisplaystatusline();
                  makepcboardsys();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    #ifndef LIB
    case ALTP   : toggleprinter();
                  #ifndef PCB_DEMO
                    if (Asy.Online == LOCAL) {
                      SpellP = (bool) (! SpellP);
                      if (SpellE) {
                        SpellP = FALSE;
                        SpellE = FALSE;
                        showserial(print);
                      }
                    }
                  #endif
                  break;
    case ALTR   :
                  #ifndef PCB_DEMO
                    if (SpellI) {
                      SpellR = TRUE;
                      SpellI = FALSE;
                      break;
                    }
                  #endif
                  Status.SysopFlag = (Status.SysopFlag == 'R' ? ' ' : 'R');
                  redisplaystatusline();
                  makepcboardsys();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    case ALTS   : if (localpasswordfailed())
                    break;
                  runsysmgr();
                  #ifdef COMM
                    if (Asy.Online == REMOTE && ! Status.InChat && Status.FullScreen)
                       return(12);   /* force CTRL-L to redisplay the screen */
                  #endif
                  break;
    case ALTT   :
                  #ifndef PCB_DEMO
                    if (SpellA) {
                      SpellA = FALSE;
                      SpellT = TRUE;
                      break;
                    }
                  #endif
                  if (prn > 0) {
                    /* under OS/2, closing the printer is necessary in order */
                    /* for anything to print .. and since closing the port   */
                    /* ALSO issues a form feed, we'll just skip the form     */
                    /* feed and just close the port.                         */
                    if (Tasker == OS2) {
                      dosclose(prn);
                      openprinter();
                    } else {
                      sendtoprinter("\r\n",3);   /*lint !e534 */
                    }
                  }
                  break;
    #endif
    case ALTX   : Status.SysopFlag = (Status.SysopFlag == 'X' ? ' ' : 'X');
                  redisplaystatusline();
                  makepcboardsys();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    #ifndef LIB
    case F1     : sysopprivileges();
                  break;
    case F2     :
                  #ifdef COMM
                    lockcallerout();
                  #endif
                  return(0);
    #endif
    #ifndef LIB
    case F3     : if (PcbData.ParallelPortNum > 0) {
                    Status.PrintLog = (bool) (! Status.PrintLog);
                    redisplaystatusline();
                    makepcboardsys();
                    #ifdef __OS2__
                      updatelinesnow();
                    #endif
                  }
                  break;
    case F4     : Status.PageBell = (bool) (! Status.PageBell);
                  redisplaystatusline();
                  makepcboardsys();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    #endif
    case F5     : if (PcbData.AllowShell && Asy.Online != OFFLINE) {
                    shelltodos();
                    #if defined(COMM) && ! defined(LIB)
                      if (Asy.Online == REMOTE && ! Status.InChat && Status.FullScreen)
                         return(12);   /* force CTRL-L to redisplay the screen */
                    #endif
                  }
                  return(0);
    #ifndef LIB
    case F6     :
                  #ifdef COMM
                    if (! Status.OnlineUpdate && Status.LoggingIn == NOTNOW && Asy.Online == REMOTE) {
                      if (Status.FullScreen) {
                        if (! Status.InChat)
                          printcls();
                        else
                          savescrnbuffer();      /*lint !e534 */
                      } else
                        savescrnbuffer();        /*lint !e534 */

                      onlineupdate();

                      if (Status.FullScreen) {
                        if (! Status.InChat)
                          return(12);   /* force CTRL-L to redisplay the screen */
                        else
                          restscrnbuffer();
                      } else
                        restscrnbuffer();
                    }
                  #endif
                  break;
    case F7     : Status.Alarm = (bool) (! Status.Alarm);
                  redisplaystatusline();
                  makepcboardsys();
                  #ifdef __OS2__
                    updatelinesnow();
                  #endif
                  break;
    case F8     : disconnectcaller();
                  return(0);
    #endif
    case F9     : if (Control.Screen)
                    turndisplayoff(TRUE);
                  else
                    turndisplayon(TRUE);
                  break;
    #ifdef COMM
    case F10    :
                  #ifndef LIB
                  /* NOTE:  Checking for REMOTE also has the effect of */
                  /* ensuring that you don't hit F10 at the "local password" */
                  /* prompt at the call waiting screen.  Things look really */
                  /* screwy if you allow the F10 at the password prompt */
                  if (Status.InChat || Asy.Online != REMOTE)
                    return(1068);

                  if (Status.FullScreen) {
                    if (! Status.OnlineUpdate)
                      printcls();
                    else
                      savescrnbuffer();          /*lint !e534 */
                  } else
                    savescrnbuffer();            /*lint !e534 */

                  /* in case the activity thread is running, suspend now */
                  showactivity(ACTSUSPEND);

                  sysopchat(1);

                  /* in case the activity thread was running, resume it now */
                  showactivity(ACTRESUME);

                  if (Status.FullScreen) {
                    if (! Status.OnlineUpdate)
                      return(12);   /* force CTRL-L to redisplay the screen */
                    else
                      restscrnbuffer();
                  } else
                    restscrnbuffer();
                  #else
                    if (Status.InChat)
                      return(1068);
                    savescrnbuffer();
                    sysopchat();
                    restscrnbuffer();
                  #endif
                  return(0);
    #endif
    #ifndef LIB
    case SF1    :
    case SF2    :
    case SF3    :
    case SF4    :
    case SF5    :
    case SF6    :
    case SF7    :
    case SF8    :
    case SF9    :
    case SF10   : stuffbuffer(KeyNum);
                  break;
    case AF1    : decreasetime();
                  break;
    case AF2    : increasetime();
                  break;
    case AF9    : decreasesecurity();
                  break;
    case AF10   : increasesecurity();
                  break;
    #endif  /* ifndef LIB */
    default     : return(KeyNum);
  }
  return(0);
}


/********************************************************************
*
*  Function:  kbdhit()
*
*  Desc    :  This function does nothing except check the keyboard buffer and
*             keyboard itself (it ignores the keyboard buffer if "NOBUFFER"
*             is passed).
*
*  Returns :  The number of the key pressed - or 0 if no key has been pressed.
*/

int LIBENTRY kbdhit(kbdhittype Action) {
  bkeytype Key;

  if (Action == CHECKBUFFER) {
    if (KbdBufCount != 0) {     /* get chars from buffer if any there */
      Key.W = KbdBuffer[KbdTailPtr];
      KbdTailPtr++;   /* it's a char - automatically rolls over to 0 after 255 */
      KbdBufCount--;
      return(Key.W);
    }
  }

  #ifdef __OS2__
    if (kbdcount()) {
      Key.W = (short) kbdgetkeyfrombuffer();
      if (Key.B.LoB == 0)
        Key.W = (short) (Key.B.HiB + 1000);
      else
        Key.B.HiB = 0;
      return(Key.W);
    }
  #else
    if (bgetkey(1)) {
      Key.W=bgetkey(0);
      if (Key.B.LoB == 0)
        Key.W = Key.B.HiB + 1000;
      else
        Key.B.HiB = 0;
      return(Key.W);
    }
    giveup();
  #endif
  return(0);
}


#ifndef LIB
bool LIBENTRY typeahead(void) {
  return((bool) (KbdBufCount));
}

bool LIBENTRY ppltypeahead(void) {
  return((bool) (PPLBufCount));
}


void LIBENTRY gettypeaheadstr(char *Str, int MaxLen) {
  while (PPLBufCount != 0) {     /* get chars from buffer if any there */
    PPLBufCount--;
    *Str = (char) PPLBuffer[PPLTailPtr++];
    if (*Str == 13)
      break;
    Str++;
    MaxLen--;
    if (MaxLen == 0)
      break;
  }
  *Str = 0;
}
#endif

#ifdef COMM
#ifndef LIB
static char RingDetectBuffer[64];

static void _NEAR_ LIBENTRY checkringdetect(void) {
  unsigned  X;
  int       Rcvd;
  char      Temp[32];

  Rcvd = cgetstr(Temp,sizeof(Temp));
  X = strlen(RingDetectBuffer);
  if (X >= sizeof(Temp)) {
    memmove(RingDetectBuffer,&RingDetectBuffer[X-10],10);
    memcpy(&RingDetectBuffer[10],Temp,Rcvd+1);
  } else strcat(RingDetectBuffer,Temp);

  if (strstr(RingDetectBuffer,"RING") != NULL || (PcbData.AnswerRing && RingDetect)) {
    showphoneringing();
    for (X = 0; X < 3; X++) {
      mysound(1200,5);
      mysound(1000,5);
    }
    memset(RingDetectBuffer,0,sizeof(RingDetectBuffer));
    #ifndef MULTIPORT
      RingDetect = FALSE;
    #endif
    Status.Ringing = TRUE;
  }
}
#endif  /* ifndef LIB */


#pragma warn -rvl
static bool _NEAR_ LIBENTRY gotabyte(int Ticks) {
  #ifdef __OS2__
    /* convert Ticks to hundredths of a second (i.e. 1/18.2 to 1/100) */
    /* then cause the system to wait until either Ticks time has past */
    /* or until either a kbd or comm byte has come in and posted an   */
    /* event to the WaitKeySem semaphore.                             */
    if (InBytes == 0) {
      WaitKeySem.reset();
      WaitKeySem.waitforevent((Ticks * 10000) / 182);
    }
    return((bool) (InBytes != 0));
  #else
    settimer(4,Ticks);
    while (1) {
      if (InBytes != 0)
        return(TRUE);
      if (timerexpired(4))
        return(FALSE);
      giveup();
    }
  #endif
}
#pragma warn +rvl


static int _NEAR_ LIBENTRY timedgetbyte(int Ticks) {
  if (gotabyte(Ticks))
    return(comminkey());
  return(-1);
}


static int _NEAR_ LIBENTRY watchescapecodes(void) {
  bkeytype     Key;
  #if defined(PCBCOMM) || defined(LIB)
    char      LastChar;
  #endif
  #ifdef LIB
    bool static QueryEnable;
  #else
    char        Temp[40];
    #ifdef PCBCOMM
      unsigned  Accumulator;
    #endif
  #endif

  /* wait 2/3rds of a second for the bracket */
  if ((Key.W = (short) timedgetbyte(18)) != '[') {  /* was it a BRACKET?   */
    addsavedbyte(Key.W);  /* it wasn't so remember what it was and */
    return(27);           /* return the ESC character back now     */
  }

  if (! gotabyte(36)) /* wait 2 sec for the rest if */
    return(0);        /* nothing comes then get out */

  #if defined(PCBCOMM) || defined(LIB)
    LastChar = 0;
    #ifndef LIB
      Accumulator = 0;
    #endif
  #endif

  /* process any cursor key escape codes but EAT the byte if it's a    */
  /* number or a semicolon since we don't really need that information */
  /* otherwise since we got this far eat the invalid character!        */
retry:
  Key.W = (short) comminkey();
  switch (Key.W) {
    case 'A': /* return(CTRL_E);  */  return(K_UP);
    case 'B': /* return(CTRL_X);  */  return(K_DOWN);
    case 'C': /* return(CTRL_D);  */  return(K_RIGHT);
    case 'D': /* return(CTRL_S);  */  return(K_LEFT);
    case 'H': /* return(CTRL_W);  */  return(K_HOME);
    case 'K': /* return(CTRL_P);  */  return(K_END);
    case 'R': UseAnsi = TRUE;
              #ifndef LIB
                redisplaystatusline();
              #endif
              return(0);
    /* These are commands for PCBComm's use */
    #if defined(PCBCOMM) || defined(LIB)
    #ifndef LIB
    case 't': switch (LastChar) {
                case '0': Status.TerseMode = TRUE;
                          printcom("PCB152");
                          Redisplay = FALSE;
                          break;
                case '1': if (Status.TerseMode) {
                            #ifdef __OS2__
                              sprintf(Temp,"SEC%ld",gettimer(5)/ONESECOND);
                            #else
                              sprintf(Temp,"SEC%ld",(gettimer(5)*10L)/182L);
                            #endif
                            printcom(Temp);
                          }
                          break;
                case '2': if (Status.TerseMode)
                            Redisplay = TRUE;
                          break;
                #ifdef PCBCOMM2
                case '3': if (Status.TerseMode && Status.LoggingIn == NOTNOW)
                            tersemodeuserinfo();
                          break;
                #endif
                case '4': if (Status.TerseMode) {
                            sprintf(Temp,"%u",PcbData.NumConf);
                            printcom(Temp);
                          }
                          break;
                case '5': if (Status.TerseMode)
                            tersemodescanmail();
                          break;
                case '[': if (Status.TerseMode)
                            tersemodeconfinfo(Accumulator);
                          break;
                #ifndef PCB_DEMO
                case ';': if (Status.TerseMode) {
                            if (Accumulator == 12346) {
                              showserial(printcom);
                            } else {
                              Status.TerseMode = FALSE;
                              displaypcbtext(Accumulator,NEWLINE);
                              Status.TerseMode = TRUE;
                              Redisplay = TRUE;
                            }
                          }
                          break;
              #endif  /* ifndef PCB_DEMO */
              }
              return(0);

    case 'T': if (LastChar == '0')
                Status.TerseMode = FALSE;
              return(0);
    #else
    case 255: switch (LastChar) {
                case '[': QueryEnable = TRUE;    /* ESC[[<255> */
                          break;
                case '3': if (QueryEnable)       /* ESC[3<255> */
                            printcom("þ");
                          break;
              }
              return(0);
    #endif    /* ifndef LIB */
    case '[':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case ';': LastChar = Key.W;
              #ifndef LIB
                if (Key.W <= '9') {
                  Accumulator *= 10;
                  Accumulator += Key.W - '0';
                }
              #endif
              if (gotabyte(36)) /* wait another two seconds */
                goto retry;     /* if something came in then process it */
    #endif    /* ifdef PCBCOMM */
    default : return(0);        /* otherwise return a 0 */
  }
}


#if defined(CPU386) && defined(MULTIPORT) && ! defined(LIB)
static bool _NEAR_ LIBENTRY ismd5string(void) {
  int      Key;
  int      X;
  char     Buffer[40];

  /* we're watching for <################> which is the MD5 string */
  /* we already have the "<" so watch for the rest                 */

  clsbox(0,24,79,24,0x70);
  fastprint(0,24,"Watching for MD5 String",0x70);

  Buffer[0] = '<';

  for (X = 1; X <= 32; X++) {
    if ((Key = timedgetbyte(4)) == -1) {
      Md5Stage = MD5ABORT;
      clsbox(0,24,79,24,0x70);
      return(FALSE);
    }

    addsavedbyte(Key);  /* save the byte in case it's not valid */

    if (islower(Key))
      Key = toupper(Key);

    /* only hex digits (0-9 and A-Z are valid */
    if (! isxdigit(Key)) {
      Md5Stage = MD5ABORT;  /* if not a match, then get out now after    */
      clsbox(0,24,79,24,0x70);
      return(FALSE);        /* aborting any further MD5 searching        */
    }

    Buffer[X] = Key;   /* store the byte */
  }

  /* if we got this far then we have 32 valid bytes */
  /* now look for the final '>' character           */

  if ((Key = timedgetbyte(4)) != '>') {
    addsavedbyte(Key);  /* save the byte in case it's not a '>'      */
    Md5Stage = MD5ABORT;  /* if not a match, then get out now after    */
    clsbox(0,24,79,24,0x70);
    return(FALSE);        /* aborting any further MD5 searching        */
  }

  Buffer[33] = '>';
  Buffer[34] = 0;

  maxstrcpy(ReceivedMd5String,Buffer,sizeof(ReceivedMd5String));
  Md5Stage = MD5DONE;
  clearsavedbytes();
  clsbox(0,24,79,24,0x70);
  fastprint(0,24,"MD5 String Received!",0x70);
  return(TRUE);
}
#endif


#ifdef FIDO
static int _NEAR_ LIBENTRY checkforEMSI(void) {
  bkeytype  Key;
  char     *p;

  /* we're watching for **EMSI_INQC816 */

  if (! gotabyte(12)){  /* wait half second for the next '*'       */
    #ifdef DEBUG
      if (DebugLevel >= 2)
        writedebugrecord("EMSI: Got only 1 * (timed out)");
    #endif
    clsbox(0,24,79,24,0x70);
    return('*');         /* if nothing came in then return '*'  */
  }

  Key.W = comminkey();
  addsavedbyte(Key.W);  /* save the byte in case it's not an '*' */

  if (Key.W != '*') {   /* was it a '*'        */
    #ifdef DEBUG
      if (DebugLevel >= 2)
        writedebugrecord("EMSI: Got only 1 * (next char not *)");
    #endif
    clsbox(0,24,79,24,0x70);
    return('*');          /* return the original '*' character back now */
  }


  if (! gotabyte(45)) {  /* wait 2 sec for the rest if */
    #ifdef DEBUG
      if (DebugLevel >= 2)
        writedebugrecord("EMSI: Got only 2 * (timed out)");
    #endif
    clsbox(0,24,79,24,0x70);
    return('*');          /* return the original '*' character back now */
  }

  for (p = "EMSI_INQC816"; TRUE; p++) {     //lint !e506
    Key.W = comminkey();
    addsavedbyte(Key.W);  /* save the byte in case it's not valid */
    if (Key.W != *p)
      break;

    if (Key.W == '6') {   /* is it the last character in the EMSI sequence? */
      #ifdef DEBUG
        if (DebugLevel >= 2)
          writedebugrecord("EMSI Detected!");
      #endif
      Status.FoundFido = TRUE;
      clsbox(0,24,79,24,0x70);
      fastprint(0,24,"EMSI Detected!",0x70);
      clearsavedbytes();  /* get rid of the bytes we saved */
      return('\r');
    }

    if (! gotabyte(45)) { /* are there any more bytes waiting? */
      #ifdef DEBUG
        if (DebugLevel >= 2)
          writedebugrecord("EMSI: Timed out");
      #endif
      clsbox(0,24,79,24,0x70);
      return('*');          /* return the original '*' character back now  */
    }
  }

  #ifdef DEBUG
    if (DebugLevel >= 2) {
      char Str[60];
      writedebugrecord("EMSI: Mismatch on EMSI_INQC816");
      sprintf(Str,"EMSI: Looking for %c, found %c",*p,Key.W);
      writedebugrecord(Str);
    }
  #endif
  clsbox(0,24,79,24,0x70);
  return('*');          /* return the original '*' character back now  */
}
#endif  /* ifdef FIDO */


/********************************************************************
*
*  Function:  commportinkey()
*
*  Desc    :  Prioritized COMM inkey routine (checks ONLY the comm port) for
*             input and thus is optimized towards modem input.
*
*             At the same time it watches for ANSI escape codes and translates
*             them into PC Keyboard Scan codes similar to that returned by
*             the kbdinkey() function.
*
*  NOTE    :  It is assumed that kbdinkey() will later be called for keyboard
*             input - and since kbdinkey() will perform the 'housekeeping' that
*             is normally found in cinkey() we won't bother with it here.
*
*  Returns :  - a 0 if nothing was available for input
*             - the ASCII code of a letter or ASCII+1000 if an extended key
*/

int LIBENTRY commportinkey(void) {
  #ifndef LIB
    static int LastKey;
  #endif
  bkeytype Key;

  Status.Kbd = FALSE;
  if (NumSavedBytes != 0) {
    return(removesavedbyte());  /* return a saved byte */
  }

  /* check for Online != REMOTE here instead of Online == LOCAL because */
  /* we don't want to drop down to the part that checks for carrier if  */
  /* we're logging in locally and haven't yet established that we are   */
  /* in "local" mode- the Online setting doesn't take place until after */
  /* the sysop gives his local password at the call waiting screen.     */

  if (Asy.Online != REMOTE) {
    #ifndef LIB
      if (Status.RingAlert && InBytes != 0)
        checkringdetect();
    #endif
    /* if we're online locally, then clear the incoming buffer so that it    */
    /* never fills up -- in PCB-OS2, having something in the buffer actually */
    /* causes the waitforkey() function to run non-stop causing 100% cpu     */
    /* utilization */
    clearinbuf();
    return(0);
  }

  /* check for carrier */
  if (Asy.LostCarrier || cdstillup() == 0) {
    Asy.LostCarrier = TRUE;
    if (PcbData.Packet)
      turnoffdtr();
    if (Asy.IgnoreCDLoss)  /* we've got more work to do so don't recycle yet */
      return(-1);
    loguseroff(ALOGOFF);
    return(-1);
  }

  if (InBytes == 0)
    return(0);

  #ifndef LIB
    if (Status.FileXfer)    /* we're transferring a file so don't do any */
      return(comminkey());  /* special processing on the incoming data   */
  #endif

  renewkbdtimer();

  Key.W = (short) comminkey();
  switch(Key.W) {
    case 0:
               #ifndef LIB
                 if (Asy.Telnet && LastKey == 13) { /* is TELNET in use?  if so was the last key a Carriage Return?    */
                   LastKey = 0;                     /* if so, chances are this is a CR/NULL pair so do NOT hang around */
                   return(0);                       /* waiting for a NULL/Scan Code pair, just return the 0.           */
                 }
               #endif
               // fall thru

    case 0xE0: while (1) {
                 if (! gotabyte(36))
                   return(0);
                 if ((Key.W = (short) comminkey()) != 0xE0) /* ignore enhanced kbd scan codes */
                   break;
               }
               return(Key.W + 1000); /* return extended character */

    case 27:   return(watchescapecodes());
  }

  #ifndef LIB
    LastKey = Key.W;
  #endif

  #if defined(CPU386) && defined(MULTIPORT) && ! defined(LIB)
    if ((Status.LoggingIn & WATCHFORMD5) && Md5Stage == MD5INIT) {
      if (Key.W == '<') {
        if (ismd5string())
          return(0);
        else
          return(Key.W);
      } else if (Key.W == '\r')  // if we get a CR then abort the MD5 watch
        Md5Stage = MD5ABORT;
    }
  #endif

  #ifdef FIDO
    if ((Status.LoggingIn & WATCHFORFIDO) && PcbData.EnableFido && ! Status.FoundFido) {
      switch(Key.W) {
        case  '*': if (! NoEmsi) {
                     clsbox(0,24,79,24,0x70);
                     fastprint(0,24,"Checking for EMSI",0x70);
                     return(checkforEMSI());
                   }
                   break;

        case 0xF1: if (! NoFTS1) {
                     Status.LoggingIn &= ~WATCHFORFIDO;
                     clsbox(0,24,79,24,0x70);
                     fastprint(0,24,"Checking for WAZOO",0x70);
                     if (Recv_Hello()) {
                       clsbox(0,24,79,24,0x70);
                       fastprint(0,24,"Performing WAZOO Initialization",0x70);
                       Receiver_WaZoo_Init();      /*lint !e534 */
                       recycle();
                     } else {
                       clsbox(0,24,79,24,0x70);
                       fastprint(0,24,"No WAZOO Detected - Resuming Normal Operation",0x70);
                     }
                     return(0);
                   }
                   break;

        case 0xAE: if (! NoWazoo) {
                     Status.LoggingIn &= ~WATCHFORFIDO;
                     clsbox(0,24,79,24,0x70);
                     fastprint(0,24,"Performing Xmodem Transfer",0x70);
                     receive_inbound_xmodem();
                     recycle();
                   }
                   break;
      }
    }
  #endif

  return(Key.W);
}
#endif   /* ifdef COMM */


/********************************************************************
*
*  Function:  systemoverhead()
*
*  Desc    :  This function takes, as input, an input word.  If the input is
*             non-Zero then the input is processed.  If it is zero, then the
*             this function performs some system overhead.  Basically, it
*             keeps data moving out the comm port, it monitors keyboard and
*             session timeouts, etc.
*
*  Returns :  The processed input
*             - a 0 if nothing was available for input
*             - the ASCII code of a letter or ASCII+1000 if an extended key
*/

static int _NEAR_ LIBENTRY systemoverhead(int Key, bool Stuffed) {
  #ifdef __OS2__
    if (Status.SessionTimedOut) {
      releasekbdblock(0);
      Display.ShowOnScreen  = TRUE;
      Display.AbortPrintout = FALSE;
      freshline();
      displaypcbtext(TXT_TIMELIMITEXCEEDED,NEWLINE|LFBEFORE|LOGIT|BELL);
      loguseroff(ALOGOFF);
      return(0);
    }
  #else
    /* this function is implimented in the heartbeatthread under OS/2 */
    if (Control.WatchSessionClock && gettimer(5) <= THREEMINUTES)
      checktime();
  #endif

  if (Key != 0) {
    if (! Stuffed) {
      /* only renew the keyboard timer if the character came from the user */
      renewkbdtimer();

      /* if the screen is off and the keystroke came from the KEYBOARD (i.e. it */
      /* was not STUFFED into the keyboard buffer then turn the screen on first */
      /* by processing an F9 key (value = 1067) but only if the keystroke was   */
      /* not an F9 key to begin with (to avoid turning the screen on only to    */
      /* have the user's keystroke turn it back off again.                      */

      if (Status.ForceScreenOff && Key != 1067)
        processkeys(1067);     /*lint !e534 */
    }

    if (Key >= LOWNUM && Key <= HIGHNUM)
      return(processkeys(Key));
    return(Key);
  }

  #ifndef __OS2__
    #ifndef LIB
      if (timerexpired(2)) {
        showstatusline();
        return(0);
      }
    #endif
  #endif

  #ifdef __OS2__
    if (Control.WatchKbdClock && checkkbdtime() == -1)
      return(-1);
  #else
    if (Control.WatchKbdClock && gettimer(1) <= ONEMINUTE && checkkbdtime() == -1)
      return(-1);
  #endif

  #ifndef LIB
    if (! Status.FileXfer) {
      if (getkbdstatus() & SCROLL) {
        viewscrollback();
//      setkbdstatus(getkbdstatus() & ~SCROLL);
        return(0);
      }
    }
  #endif

  #ifdef COMM
    turnonxmit();
  #endif

  /* normally we wouldn't need to call giveup() under the OS/2 version, but */
  /* the #ifdef is commented out to test to see how well this solves the    */
  /* problem that occurs when PPEs poll the keyboard.                       */
//#ifndef __OS2__
    giveup();
//#endif
  return(0);   /* indicate that we have nothing to do */
}


/********************************************************************
*
*  Function:  watchsystemfunctions()
*
*  Desc    :  This function is called from the modem routines.  If the modem
*             is stuck in a loop waiting for room in the outgoing buffer, we
*             STILL must maintain system functions such as:  keeping the comm
*             port output moving, checking for local keyboard requests,
*             watching for keyboard and session time limits, etc.
*
*             In other words, the systemoverhead() function needs to be called.
*
*             However, the systemoverhead() function will only process keyboard
*             input if you first GET the input and pass it to systemoverhead().
*
*             So this function checks to see if there is any keyboard input by
*             calling kbdhit() and then calls systemoverhead() with the resuts.
*             If a key WAS hit, but it was not PROCESSED by systemoverhead(),
*             in other words, it was not a "special key" (such as F8 to log a
*             caller off) then it must stuff the key into the keyboard buffer
*             so that the next time an input prompt needs to read keyboard
*             input the kbdinkey() function will find it.
*/

void LIBENTRY watchsystemfunctions(void) {
  int Key;

  if ((Key = systemoverhead(kbdhit(NOBUFFER),FALSE)) != 0)
    stuffbuffer(Key);
}


/********************************************************************
*
*  Function:  kbdinkey()
*
*  Desc    :  KEYBOARD inkey routine
*
*  NOTE    :  It is assumed that commportinkey() was previously called to
*             check for comm port input.  This routine will now check for
*             keyboard input and also perform housecleaning chores.
*
*  Returns :  - a 0 if nothing was available for input
*             - the ASCII code of a letter or ASCII+1000 if an extended key
*/

int LIBENTRY kbdinkey(void) {
  bkeytype Key;

  Status.Kbd = TRUE;

  if (PPLBufCount != 0) {
    PPLBufCount--;
    return(systemoverhead(PPLBuffer[PPLTailPtr++],TRUE));
  }

checkbuf:
  if (KbdBufCount != 0) {
    KbdBufCount--;
    return(systemoverhead(KbdBuffer[KbdTailPtr++],TRUE));
  }

  #ifndef LIB
    if (AutoStuff) {   /* are we supposed to be stuffing the buffer? */
      stuffit();       /* yes, go do it */
      goto checkbuf;   /* now go process any keys we may have stuffed */
    }
  #endif

  #ifdef __OS2__
    if (kbdcount()) {
      Key.W = (uint) kbdgetkeyfrombuffer();
      if (Key.B.LoB == 0)
        Key.W = (short) (Key.B.HiB + 1000);
      else
        Key.B.HiB = 0;
      return(systemoverhead(Key.W,FALSE));
    }
  #else
    if (bgetkey(1)) {
      Key.W=bgetkey(0);
      if (Key.B.LoB == 0)
        Key.W = Key.B.HiB + 1000;
      else
        Key.B.HiB = 0;
      return(systemoverhead(Key.W,FALSE));
    }
  #endif

  return(systemoverhead(0,FALSE));
}


int LIBENTRY cinkey(void) {
  #ifdef COMM
    int Key;

    /* Don't check for comm port input unless */
    /* the PPL and KBD buffers are empty      */

    #ifdef LIB
      if (KbdBufCount == 0 && (Key = commportinkey()) != 0)
        return(Key);
    #else
      if (PPLBufCount == 0 && KbdBufCount == 0 && ! AutoStuff &&
          (Key = commportinkey()) != 0)
        return(Key);
    #endif
  #endif
  return(kbdinkey());
}


#ifdef __OS2__
enum { TH_MUSTEXIT, TH_INTHREAD, TH_EXITED };
static bool ThreadState;
static unsigned long HeartBeatThreadId;

#pragma argsused
static void THREADFUNC heartbeatthread(void * Ignore) {
  long ElapsedTime;

  setdefaultpriority();

  // make sure LastKbdTime gets initialized
  LastKbdTime = dosgetlongtime();

  while (ThreadState != TH_MUSTEXIT) {
    KbdTimer.waitforevent(TENSECONDS);
    KbdTimer.reset();

    if (Control.WatchKbdClock) {
      ElapsedTime = doselapsedtime(LastKbdTime);

      // check to see if we've gone past the keyboard timer warning point
      if ((unsigned) ElapsedTime >= Control.KbdTimer) {
        // yes, we're either in the last 60 seconds, or it has expired
        // check to see if the keyboard timer is completely expired
        if (ElapsedTime >= Control.KbdTimer + (ONEMINUTE/10)) {
          // if we haven't recorded a keyboard timeout yet, do so now and
          // also record WHEN it occurred so that we can act on it again
          // if it appears that nothing is happening
          if (! Status.KbdTimedOut) {
            KbdTimeOutTime = dosgetlongtime();
            Status.KbdTimedOut = TRUE;
          }
          // the keyboard has timed out and we need to cause a recycle
          releasekbdblock(0);
        } else {
          // otherwise, just ring the bell to indicate a 10 second mark
          bell();
        }
      }
    }

    if (Control.WatchSessionClock) {
      if (gettimer(5) <= THREEMINUTES) {
        checktime();
        releasekbdblock(0);
      }
    }

    // if the keyboard timed out, then try once again unblocking the thread
    if (Status.KbdTimedOut) {
      releasekbdblock(0);
      // if the system appears to be "hung" with the keyboard timeout, as
      // determined by the keyboard timeout having occured ten minutes ago,
      // then force a recycle to occur by exiting to DOS.
      if (doselapsedtime(KbdTimeOutTime) >= (10 * 60 * 100)) {
        writelog("ERROR: Kbd Timed Out - Forced Recycle",SPACERIGHT);
        exit(99);
      }
    }

    #ifndef LIB
      // if we're not currently viewing the scrollback buffer and timer #2 has
      // expired then refresh the status line now
      if ((! Status.InScrollBack) && timerexpired(2))
        showstatusline();
    #else
      // if timer #2 has expired then refresh the status line now
      if (timerexpired(2))
        showstatusline();
    #endif

    #ifdef KBD3
      serialcheck();
    #endif
  }

  ThreadState = TH_EXITED;
}


void LIBENTRY createheartbeatthread(void) {
  ThreadState = TH_INTHREAD;
  HeartBeatThreadId = startthread(heartbeatthread,8192,NULL);
}


void LIBENTRY destroyheartbeatthread(void) {
  if (ThreadState != TH_EXITED) {
    ThreadState = TH_MUSTEXIT;
    KbdTimer.postevent();
    waitthread(HeartBeatThreadId);
    KbdTimer.close();
  }
}


int LIBENTRY waitforkey(void) {
  if (KbdBufCount == 0 &&
      PPLBufCount == 0 &&
      #ifndef LIB
        (! AutoStuff)  &&
      #endif
      #ifdef COMM
        InBytes == 0   &&
      #endif
      kbdcount() == 0) {
    WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
    WaitKeySem.reset();
  }

/*
  // the heartbeat thread watches for the time to run out and, when it
  // does, it sets the ForceLogoff variable.  We should watch for this
  // and when it occurs, reset the variable first, then log the user off
  if (ForceLogoff) {
    ForceLogoff = FALSE;
    loguseroff(ALOGOFF);
  }
*/
  if (Status.SessionTimedOut)
    loguseroff(ALOGOFF);

  #ifdef COMM
    int Key;

    /* Don't check for comm port input unless */
    /* the PPL and KBD buffers are empty      */

    #ifdef LIB
      if (KbdBufCount == 0 && (Key = commportinkey()) != 0)
        return(Key);
    #else
      if (PPLBufCount == 0 && KbdBufCount == 0 && ! AutoStuff &&
          (Key = commportinkey()) != 0)
        return(Key);
    #endif
  #endif

  return(kbdinkey());
}


void LIBENTRY releasekbdblock(int TimeToWait) {
  if (TimeToWait == 0)
    WaitKeySem.postevent();
  else {
//  WaitKeySem.stoptimer();
    WaitKeySem.settimer(TimeToWait);
  }
}


int LIBENTRY waitforkey(int TimeToWait) {
  if (TimeToWait != 0)
    releasekbdblock(TimeToWait);
  return(waitforkey());
}
#endif


/********************************************************************
*
*  Function:  checkstatus()
*
*  Desc    :  Checks for the following items:
*               - Loss of carrier
*               - Time expired
*               - Ctrl-K or Ctrl-S
*               - F9
*               - Any other characters - which get copied to the input buffer
*
*  NOTE:  This routine is in here because it can stuff the keyboard buffer.
*/

void LIBENTRY checkstatus(void) {
  bool     Kbd;
  bkeytype bKey;
  int      Key;

  #ifndef __OS2__
    /* this function is implemented in the heartbeatthread under OS/2 */
    if (Control.WatchSessionClock && gettimer(5) <= THREEMINUTES)
      checktime();
  #endif

  /* NOTE!  we duplicate the effort of getting KBD or COMM input here   */
  /* because we are going to stuff the buffer if we get characters that */
  /* we don't need .. this will allow for keyboard buffering to occur   */

  #ifdef COMM
    if (Asy.Online == REMOTE) {
      /* check for carrier */
      if (Asy.LostCarrier || cdstillup() == 0) {
        Asy.LostCarrier = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
        if (Asy.IgnoreCDLoss)
          return;
        loguseroff(ALOGOFF);
        return;
      }

      #ifdef FIDO
        /* if Fido is enabled, and we're still logging in, then disable    */
        /* the CTRL-K processing because FrontDoor sends CTRL-K as part of */
        /* the handshaking process.                                        */
        if ((Status.LoggingIn & WATCHFORFIDO) && PcbData.EnableFido)
          return;
      #endif

      /* check for CTRL-K, CTRL-S or CTRL-X in input queue */
      if ((Key = checkcomm()) != 0) { /* if we found one go process it */
        Kbd = FALSE;
        goto process;
      }
    }
  #endif

top:
  #ifdef __OS2__
    if (kbdcount()) {
      Kbd = TRUE;
      bKey.W = (short) kbdgetkeyfrombuffer();
      if (! bKey.B.LoB)
        Key = bKey.B.HiB + 1000;
      else
        Key = bKey.B.LoB;
    } else {
      return;
    }
  #else
    if (bgetkey(1)) {
      Kbd = TRUE;
      bKey.W = bgetkey(0);
      if (! bKey.B.LoB)
        Key = bKey.B.HiB + 1000;
      else
        Key = bKey.B.LoB;
    } else {
      return;
    }
  #endif


process:
  renewkbdtimer();
  switch (Key) {
    case    0: break;  /* do nothing */
    case   11:                                                    /* CTRL-K */
    case   24: if (Display.Break) {                                /* CTRL-X */
                 #ifdef COMM
                   if (Asy.Online == REMOTE) {
                     clearoutbuf();  /* purge the input and output buffers */
                     clearinbuf();
                     /* sendbreak();  still doesn't work */
                   }
                 #endif
                 clearkbdbuffer();
                 if (Control.GraphicsMode) { /* force it to reset the color! */
                   asetcolor(0);
                   printdefcolor();
                   printdefcolor();
                   cleareol();
                 }
                 Display.AbortPrintout = TRUE;
               }
               break;
    case   19:
               #ifndef LIB
                 fastputc(3838,0x53);
               #endif
               #ifdef COMM
                 if (Asy.Online == REMOTE) {
                   commstop();
                   while (1) {
                     Key = comminkey();  /* eat all chars up to the CTRL-S while saving the rest */
                     if (Key == 19)      /* found it, now get out */
                       break;
                     if (Key == -1)      /* no more characters in buffer */
                       break;
                   }
                 }
               #endif
               while (1) {
                 switch (Key = cinkey()) {
                   case 19: renewkbdtimer();
                            Key = 0;
                            break;
                   case 11:
                   case 24: goto process;
                 }
                 #ifdef COMM
                   /* check for carrier */
                   if (Asy.Online == REMOTE && cdstillup() == 0) {
                     Asy.LostCarrier = TRUE;
                     if (PcbData.Packet)
                       turnoffdtr();
                     if (Asy.IgnoreCDLoss)
                       return;
                     loguseroff(ALOGOFF);
                     return;
                   }
                 #endif
                 if (Key != 0) {
                   #ifndef LIB
                     fastputc(3838,0x20);
                   #endif
                   break;
                 }
                 /* we don't want the user to sit FOREVER with CTRL-S so  */
                 /* we'll check the keyboard timer here regardless of the */
                 /* Control.WatchKbdClock settings!                       */
                 #ifndef __OS2__
                   if (PcbData.KbdTimeout != 0 && timerexpired(1)) {
                     #ifdef COMM
                       commgo();
                       clearoutbuf();
                     #endif
                     checkkbdtime();     /*lint !e534 */
                   }
                 #endif
               }
               #ifdef COMM
                 commgo();
               #endif
               break;
    default  : if (Kbd) {
                 if (Key >= LOWNUM && Key <= HIGHNUM) {
                   if ((Key = processkeys(Key)) == 0)
                     break;
                 }
                 KbdBuffer[KbdHeadPtr] = (short) Key;
                 KbdHeadPtr++; /* it's a char - automatically rolls over to 0 after 255 */
                 KbdBufCount++;
                 goto top;     /* go back and see if any more keyboard chars are there  */
               }
               break;
  }
}


#ifdef COMM
#ifdef COMMENTOUT
void LIBENTRY watchstatuswhilesending(void) {
  if (Asy.Online == REMOTE && OutBytes > 1) {
    Asy.Online = LOCAL; showactivity(ACTBEGIN); Asy.Online = REMOTE;
    while (OutBytes > 1 && cdstillup()) {
      checkstatus();   /* process any CTRL-K/X/S requests while continuing output */
      turnonxmit();
      giveup();
      giveup();
      Asy.Online = LOCAL; showactivity(ACTSHOW); Asy.Online = REMOTE;
    }
    Asy.Online = LOCAL; showactivity(ACTEND); Asy.Online = REMOTE;
  }
  Display.AbortPrintout = FALSE;
}
#endif
#endif
