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


#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <misc.h>
#include <dosfunc.h>
#include <country.h>
#include <pcb.h>
#include <pcboard.h>
#include <pcboard.ext>
#include <pcbtext.h>
#include <users.h>
#include <usersys.h>
#include <ansi.h>
#ifdef DEBUG
  #include <memcheck.h>
#endif

#ifdef _MSC_VER
  #include <borland.h>
#endif

#ifdef __OS2__
  #include <kbd.hpp>
  #include "sem.hpp"
#endif

void LIBENTRY checkpagelen(void);
void LIBENTRY initport(void);
void LIBENTRY openport(void);

typedef enum {WATCHKBD=1, WATCHSESSION=2, READUSERSYS=4, SHOWSTATUSLINE=8, LMRS=16, CONFFLAGS=32, TPA=64, NOCLS=128} openstatus;

int LIBENTRY readusersysfile(openstatus OpenStatus);
void LIBENTRY writeusersysfile(void);
void LIBENTRY pcbinit(void);

colortype static ScreenColors[2] = {{0x07,0x70,0x70,0x07,0x00,0x07,0x70,0x07,0x07,0x07,0x07}, {0x1F,0x70,0x70,0x4F,0x10,0x17,0x30,0x1E,0x4E,0x1E,0x1C}};
colortype *ScrnColors;


statustype _FARDATA_ Status;

asytype     Asy;
disptype    Display;
controltype Control;
syshdrtype  UserSysHdr;
userrectype UserSys;
char        *DoorParams;
char        DoorName[21];
bool        ForceUpdate;
char        ErrorLevel = 0;

bool NoLanguage = FALSE;
bool NoGraphics = FALSE;
bool NoSecurity = FALSE;

char YesChar;
char NoChar;
char SystemYesChar;
char SystemNoChar;


extern unsigned ConfByteLen;

static char ansicolors[] = "04261537";  /* build-enabling: ANSI.C owns the public copy */
char pcbcolors[8] = { 0,12,10,14,9,13,11,15 };

#ifndef __OS2__
bool CtrlBreak;
#endif

bool UseAnsi;

char mask_alphanum[]   = { 3,0,' ','~'};
char mask_yesno[]      = { 0 };
char mask_more[]       = { 3,'G','N','S'};
char mask_num[]        = { 3, 0,'0','9'};
char mask_atoz[]       = { 6,0,'A','Z',0,'a','z'};
char mask_systemyesno[]= { 2,'Y','N'};

countrytype SystemCountry;

#ifdef __OS2__
  CTimeoutSemaphore WaitKeySem;
  CSemaphore        KbdTimer;
#endif

void LIBENTRY setkbdtimer(int Minutes) {
/* calculate and store the keyboard timer value */
/* */
/* if the value is exactly 1 minute, then add 10 seconds to it - this */
/* will avoid the problem of having the kbd timer beep every time a */
/* key is pressed (i.e. the "60th second beep" is now 10 seconds away) */

  if (Minutes == 0) {
    Control.KbdTimer = 0;
    return;
  }

  #ifdef __OS2__
    Minutes--;  /* under OS/2, the timer runs up to the first bell which occurs */
/* at the 60-seconds-to-go mark, so subtract one minute. */

/* NOTE:  divide by 10 because dosgetlongtime() uses 1/100ths of a second */

    Control.KbdTimer = Minutes * ONEMINUTE;

    if (Minutes == 0)  /* if the timeout was only 1 minute */
      Control.KbdTimer += TENSECONDS;  /* then add 10 seconds */
  #else
    Control.KbdTimer = (unsigned) ((long) Minutes * 182 * 6);
    if (Minutes == 1)  /* if timeout is 1 minute, then */
      Control.KbdTimer += TENSECONDS;  /*   add 10 seconds to make it 70 seconds */
  #endif
}




int LIBENTRY initdoor(char *Name, int SessionLimit, int KbdTimeout, openstatus OpenStatus) {
  pcbinit();
  initbmalloc();

  #ifdef __OS2__
    kbdstartthread();  /*lint !e534 */
    WaitKeySem.createunique("WKEY");
    if (! WaitKeySem.opened())
      errorexittodos("Unable to create WKEY semaphore");
    kbdregistersemaphore(WaitKeySem);  /*lint !e534 */
    KbdTimer.createunique("HBEAT");
    if (! KbdTimer.opened())
      errorexittodos("Unable to create HBEAT semaphore");
    createheartbeatthread();
  #else
    CtrlBreak = getcbrk();
    setcbrk(0);  /*lint !e534 */
  #endif

  SystemCountry = Country;

  DoorParams = getenv("PCBDOOR");

  ScrnColors = &ScreenColors[Scrn_Mode];

  memset(&Status,0,sizeof(Status));

  Control.Screen       = TRUE;
  Display.ShowOnScreen = TRUE;
  Display.NumScrnLines = 25;
  Display.PageLen      = 23;
  setlimits(Display.NumScrnLines);

  YesChar = 'Y';
  NoChar  = 'N';

  /* getsystemlanguage will read in pcbtext IF it is set for multilingual    */
  /* if not set to multilingual, or if an error occurs getting the pcbml.dat */
  /* information, then it will have to read the pcbtext file separately.     */

  if (! PcbData.MultiLingual || getsystemlanguage() == -1) {
    PcbData.MultiLingual = FALSE;  /* in case pcbml.dat could not be read */
    readpcbtextfile("",PROGRAM);
  } else
    readpcbtextfile("",PROGRAM);

  mask_systemyesno[1] = SystemYesChar = YesChar;
  mask_systemyesno[2] = SystemNoChar  = NoChar;

  maxstrcpy(DoorName,Name,sizeof(DoorName));

  openprinter();

  if (Scrn_Rtrc && ! PcbData.EliminateSnow)  /* turn retrace checking back   */
    Scrn_Rtrc = FALSE;                       /* OFF if they don't want it on */

  Display.DefaultColor = (char) (ansicolors[PcbData.DefaultColor - 30] - '0');
  if (PcbData.DefaultIntensity == 1)
    Display.DefaultColor += (char) 8;

  if (KbdTimeout != 0)
    PcbData.KbdTimeout = (char) KbdTimeout;

  Control.KbdTimer          = (unsigned) ((long) PcbData.KbdTimeout * 182 * 6);
  Control.WatchKbdClock     = (bool) ((OpenStatus & WATCHKBD) && Control.KbdTimer != 0 ? TRUE : FALSE);
  Control.WatchSessionClock = (bool) (OpenStatus & WATCHSESSION ? TRUE : FALSE);
  Display.Break             = TRUE;
  Asy.Online                = OFFLINE;
  Asy.DataBits              = 8;
  settimer(1,Control.KbdTimer);

  ConfByteLen = (PcbData.NumAreas >> 3) + ((PcbData.NumAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;

  initport();
  readpcboardsys();
  openport();

  if (SessionLimit != 0 && minutesleft() > SessionLimit) {
    Status.SysLimit = minutesleft();  /* remember pcboard.sys time limit */
    addtime(SessionLimit * 60L,NEWTIME);
    Status.OverrideLimit = minutesleft();
  }

  openlog();

  if (Asy.Online == REMOTE) {
    if (cdstillup())
      checkforansi();
  } else
    UseAnsi = TRUE;

  if (OpenStatus & READUSERSYS)
    if (readusersysfile(OpenStatus) == -1)
      return(-1);

  if (OpenStatus & NOCLS) {
    agotoxy(0,wherey());
  } else {
    cls();
    agotoxy(0,2);
  }
  asetcolor(7);

  if (OpenStatus & SHOWSTATUSLINE) {
    Display.NumScrnLines  = 23;
    Status.StatusLine1    = 23;
    Status.StatusLine2    = 24;
    Display.PageLen       = (unsigned char) (Display.NumScrnLines-2);
    setlimits(Display.NumScrnLines);
    redisplaystatusline();
  }

  if (OpenStatus & READUSERSYS)
    checkpagelen();

  return(0);
}


void LIBENTRY closedoor(bool UpdateUserSys) {
  if (UpdateUserSys || ForceUpdate)
    writeusersysfile();

  waitforempty(30);
  recycle();
}
