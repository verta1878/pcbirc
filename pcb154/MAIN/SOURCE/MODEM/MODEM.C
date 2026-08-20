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

/******************************************************************************
  COMMON FUNCTIONS FOR ALL DRIVERS
******************************************************************************/


bool ModemOpened = FALSE;
bool ModemOffHook;
bool ModemFixupsDone = FALSE;    /* possible values:  FALSE, 'A'=async, 'F'=fossil, 'C'=commdrv, 'O'=os/2 */

#ifndef __OS2__
void LIBENTRY ASYNC_dofixups(void);
void LIBENTRY ASYNC_openmodem(showtype Show);
void LIBENTRY FOSSIL_dofixups(void);
void LIBENTRY FOSSIL_openmodem(showtype Show);
void LIBENTRY COMMDRV_dofixups(void);
void LIBENTRY COMMDRV_openmodem(showtype Show);
#endif

void LIBENTRY OSDRIVER_dofixups(void);
void LIBENTRY OSDRIVER_openmodem(showtype Show);

int OutBufSize;

#ifndef LIB
static char InBuf[64];   /* comm input buffer for modem initialization */


static void _NEAR_ LIBENTRY watchkbddropout(void) {
  int Key;

  if (! PcbData.RequirePwrdToExit) {
    if ((Key = kbdhit(NOBUFFER)) == 27 || Key == 1068 || Key == 1059) {
      PcbData.ResetModem   = FALSE;
      PcbData.OffHook      = FALSE;
      PcbData.ExitToDos = TRUE;
      Status.ErrorLevel = 0;
      recycle();
    }
    if (Key != 0)
      stuffbuffer(Key);
  }
}


static void _NEAR_ LIBENTRY showandtell(void) {
  unsigned Len;
  char     Temp[sizeof(InBuf)];

  Len = strlen(InBuf);
  if (Len > sizeof(InBuf) - 8) {
    Len -= (sizeof(InBuf)/2);
    memset(Temp,0,sizeof(Temp));
    memcpy(Temp,&InBuf[sizeof(InBuf)/2],Len);
    memcpy(InBuf,Temp,sizeof(InBuf));
  }

  memset(Temp,0,sizeof(Temp));
  if (cgetstr(Temp,sizeof(Temp) - Len) > 0) {
    showmodemdata(Temp);
    strcat(InBuf,Temp);
  }
}


static void _NEAR_ LIBENTRY writecommtolog(char *Buf) {
  char Temp[128];

  maxstrcpy(Temp,Buf,sizeof(Temp));
  stripall(Temp,13);                /* remove carriage return */
  writedebugrecord(Temp);
}


bool LIBENTRY sendmodemwaitokay(char *OutBuf, bool WatchKbd) {
  clearoutbuf();
  tickdelay(QUARTERSECOND);  /* wait 1/4 second and then */
  clearinbuf();              /* clear input to make sure nothing is waiting */

  if (DebugLevel >= 1)
    writecommtolog(OutBuf);

  memset(InBuf,0,sizeof(InBuf));
  slowsendtomodem(OutBuf);
  settimer(3,THREESECONDS+(PcbData.ModemDelay * ONESECOND));

  while (! timerexpired(3)) {
    turnonxmit();
    if (WatchKbd)
      watchkbddropout();
    showandtell();
    if (strstr(InBuf,"OK") != NULL)
      return(TRUE);
    if (strstr(InBuf,"ERROR") != NULL)
      return(FALSE);
  }
  if (DebugLevel >= 1)
    writecommtolog(InBuf);

  return(FALSE);
}


void LIBENTRY modemoffhook(void) {
  char OffHookStr[80];

  if (ModemOpened) {
    if (PcbData.Packet)
      turnoffdtr();
    else if (PcbData.ModemOff[0] != 0 && ! ModemOffHook) {
      maxstrcpy(OffHookStr,PcbData.ModemOff,sizeof(OffHookStr)-1);
      addchar(OffHookStr,'\r');
      settimer(4,THREESECONDS);
      while (! timerexpired(4)) {
        if (sendmodemwaitokay(OffHookStr,TRUE)) {
          tickdelay((PcbData.ModemDelay*HALFSECOND)+HALFSECOND);
          ModemOffHook = TRUE;
          break;
        }
      }
    }
  }
}


void LIBENTRY slowsendtomodem(char *Str) {
  char C;

  while ((C = *Str) != 0) {
    switch (C) {
      case '~': tickdelay(HALFSECOND);
                break;
      case '^': Str++;
                if (*Str != '^')
                  C = *Str - '@';
                /* fall thru */
      default : sendbyte(C);
                mydelay(2+PcbData.ModemDelay);
                break;
    }
    Str++;
    showandtell();
  }
}


int LIBENTRY resetmodem(bool FullReset, bool WatchKbd) {
  char *p;
  char  InitStr[80];

  /* if the modem isn't opened then exit OR if we're in a      */
  /* PAD environtment then there is no modem to reset so exit. */

  if (! ModemOpened || PcbData.Packet)
    return(0);

  p = (FullReset ? "ATZ\r" : "AT\r");

  settimer(4,TENSECONDS);
  while (1) {
    if (sendmodemwaitokay(p,WatchKbd))
      break;
    if (timerexpired(4)) {
      sendmodemwaitokay("ATZ\r",WatchKbd);     /*lint !e534 */
      break;
    }
  }

  if (PcbData.ModemInit[0] == 0 && PcbData.ModemInit2[0] == 0)
    return(0);

  maxstrcpy(InitStr,PcbData.ModemInit,sizeof(InitStr)-1);
  addchar(InitStr,'\r');

  settimer(4,TENSECONDS);
  while (1) {
    if (sendmodemwaitokay(InitStr,WatchKbd))
      break;
    if (timerexpired(4))
      return(-1);
  }

  if (PcbData.ModemInit2[0] != 0) {
    maxstrcpy(InitStr,PcbData.ModemInit2,sizeof(InitStr)-1);
    addchar(InitStr,'\r');

    settimer(4,TENSECONDS);
    while (1) {
      if (sendmodemwaitokay(InitStr,WatchKbd))
        break;
      if (timerexpired(4))
        return(-1);
    }
  }

  tickdelay(QUARTERSECOND);
  return(0);
}
#endif    /*  LIB  */


/********************************************************************
*
*  Function:  initializemodem()
*
*  Desc    :  Attempts to initialize the modem
*
*  Returns :  TRUE if successful, FALSE otherwise
*/

#pragma warn -par
bool LIBENTRY initializemodem(showtype Show) {
  #ifndef LIB
    char Str[128];
  #endif

  ModemOpened = TRUE;
  ModemOffHook = FALSE;

  #ifndef LIB
    if (Show == SHOW) {
      sprintf(Str,"OPEN %ld: ",Asy.ModemSpeed);
      showmodemdata(Str);
    }

    if (DebugLevel >= 1)
      reportcom(Asy.ModemSpeed);

    if (Show == SHOW) {
      turnoffdtr(); /* in case DTR is already up turn it off for 1/2 second */
      tickdelay((PcbData.ModemDelay*HALFSECOND)+HALFSECOND);
    } else if (Asy.HstMode)
      tickdelay((PcbData.ModemDelay*HALFSECOND)+ONESECOND);
  #else
    tickdelay((PcbData.ModemDelay*HALFSECOND)+QUARTERSECOND);
  #endif

  turnondtr();
  turnonrts();

  #ifndef LIB
    if (Show == SHOW && ! AnswerOnStartup)
      if (resetmodem(FALSE,TRUE) == -1) {

        if (timeforevent() > 0)
          performevent();

        turnoffrts();
        turnoffdtr();
        reseterror();
        settimer(3,(PcbData.ModemDelay * HALFSECOND) + ONESECOND);
        while (! timerexpired(3)) {
          watchkbddropout();
          giveup();
        }
        return(FALSE);
      }
  #endif
  return(TRUE);
}
#pragma warn +par


static void _NEAR_ LIBENTRY waitforroominbuffer(int Len) {
  int Bytes;

  settimer(0,SIXTYSECONDS);   /* give it up to 60 seconds to make room in    */
                              /* the buffer and the just go ahead and return */

  while (1) {
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
      if (timerexpired(0)) {      /* ran out of time?                 */
        clearoutbuf();            /* yes, clear the out buffer        */
        writelog("FLOW TIMEOUT",SPACERIGHT);
        return;                   /* and return to what we were doing */
      }
    }

    #if defined(MULTIPORT) && defined(OSDRIVER)
      if (PcbData.OS2Driver)
        return;
    #endif

    Bytes = OutBytes;
    if (Bytes+Len < OutBufSize)
      return;

    turnonxmit();
    giveup();

    /* watch for local keyboard input, system timeouts, etc, while we wait */

    watchsystemfunctions();
  }
}


void LIBENTRY waitforempty(int Seconds) {
  int Bytes;

  if (Asy.Online == REMOTE) {
    settimer(4,Seconds);
    do {
      turnonxmit();
      giveup();
      Bytes = OutBytes;

      /* Some FOSSIL drivers will return a value that looks like there is */
      /* ONE byte still in the buffer - so check for the number of bytes  */
      /* left to be less-than-or-equal-to ONE instead of zero.            */

      if (Bytes <= 1)
        break;

      if (! cdstillup()) {
        if (PcbData.Packet)
          turnoffdtr();
        break;
      }

    } while (! timerexpired(4));
  }
}


void LIBENTRY clearandwaitformodemempty(void) {
  int  CPS;
  int  Bytes;
  int  DelayBytes;
  long Delay;

  if (Asy.Online != REMOTE)
    return;

  Bytes = OutBytes;
  clearoutbuf();

  if (Bytes != 0 && (Asy.ModemSpeed > Asy.CarrierSpeed || Asy.ModemSpeed > 2400))
    DelayBytes = (Bytes > 128 ? (Bytes > 1500 ? 3076 : 2048) : 1024);
  else
    DelayBytes = 128;

  CPS = (int) (Asy.CarrierSpeed / 10);
  if (CPS <= 0)
    Delay = HALFSECOND;
  else
    Delay = ((long) DelayBytes * TICKSPERSECOND) / CPS;

  settimer(3,Delay);
  while (! timerexpired(3) && cdstillup()) {
    giveup();
    giveup();
  }
}


/* this function now gets around the old required-2K-output-buffer problem  */
/* by checking to see if the string to be sent is greater than the output   */
/* buffer size and, if so, it will split the string into 1/2-of-buffer size */
/* chunks and send them individually.                                       */

void LIBENTRY sendstr(char *Str, int StrLen) {
  int HalfBuf = OutBufSize/2;

  while (StrLen > HalfBuf) {
    waitforroominbuffer(HalfBuf);
    csendstr(Str,HalfBuf);
    Str += HalfBuf;
    StrLen -= HalfBuf;
  }

  if (StrLen > 0) {
    waitforroominbuffer(StrLen);
    csendstr(Str,StrLen);
  }
}


void LIBENTRY sendbyte(char Byte) {
  waitforroominbuffer(1);
  csendbyte(Byte);
}


/********************************************************************
*
*  Function:  closemodem()
*
*  Desc    :  Closes the comm port by disengaging the IRQ and freeing up the
*             allocated send and receive buffers.
*
*             First we'll wait for any bytes in OUR output buffer to be sent
*             to the modem.
*
*             Then, if TurnOffDTR is specified - then we'll assume the modem
*             has a buffer on it and wait for it to empty as well.  Then we'll
*             turn DTR off and wait for CD to drop if it was on.
*
*             And finally, we'll free up the send and receive buffers.
*/


void LIBENTRY closemodem(bool TurnOffDTR) {
  int  CPS;
  long BytesBuffered;
  long Delay;

  if (! ModemOpened)
    return;

  BytesBuffered = OutBytes;

  if (online()) {
    waitforempty(THIRTYSECONDS);

    if (TurnOffDTR) {
      Delay = HALFSECOND; /* default to half a second */
      if (Asy.CarrierSpeed > 2400 || Asy.CarrierSpeed != Asy.ModemSpeed) {
        /* assume the modem has a buffer and wait for it to clear */
        CPS = (int) (Asy.CarrierSpeed / 10);
        if (CPS > 0) {
          #ifndef LIB
            if (Status.LogoffFile > BytesBuffered)
              BytesBuffered = Status.LogoffFile;
          #endif
          if (BytesBuffered > 1000)
            Delay = ((BytesBuffered * TICKSPERSECOND) / CPS) + HALFSECOND;
        }
      }

      settimer(3,Delay);
      while (! timerexpired(3) && cdstillup()) {
        turnonxmit();
        giveup();
      }
    } else
      tickdelay(HALFSECOND);  /* give it another half second to get the data out */
  }

  if (PcbData.ModemPort[0] == 'C')
    turnoffrts();

  if (TurnOffDTR) {
    if (online()) {
      tickdelay(PcbData.ModemDelay*(ONESECOND+HALFSECOND));  /* multiply by 1.5 seconds */
      turnoffdtr();
      settimer(3,(PcbData.ModemDelay * HALFSECOND) + ONESECOND);
      while (! timerexpired(3) && cdstillup()) {
        giveup();
        giveup();
      }
    } else turnoffdtr();
  }

  disconnectmodem();
  ModemOpened = FALSE;
}


void LIBENTRY openmodem(showtype Show) {
  if (Asy.ComPortNumber == 0)
    return;

  /* First, test for ASYNC access - all DOS versions of ASYNC capability */
  /* but the OS/2 version does not.                                      */

  #ifndef __OS2__
    if (PcbData.ModemPort[0] == 'C') {
      #ifdef MULTIPORT
        ASYNC_dofixups();
      #endif
      ModemFixupsDone = 'A';
      ASYNC_openmodem(Show);
      return;
    }
  #endif

  /* PCBoard has all comm handlers built-in, but the Toolkit can be:         */
  /*                                                                         */
  /*    1) Async-only                                                        */
  /*    2) Async-only + FOSSIL + OS/2                                        */
  /*    2) Async-only + FOSSIL + OS/2 + COMMDRV                              */
  /*                                                                         */
  /* To accomodate all of the above (PCBoard and variations of the Toolkit)  */
  /* the code below tests for each possibility separate.  The ORDER of the   */
  /* tests is IMPORTANT!  If OS/2 capability is requested, and it is         */
  /* available, then it should be used.  Next, if COMMDRV capability is      */
  /* available, and if neither FOSSIL nor OS/2 is requested, then it should  */
  /* be used.  Finally, as a last resort, FOSSIL is utilized since it should */
  /* be available on all platforms.                                          */

  #ifdef MULTIPORT
    #ifdef OSDRIVER
      if (PcbData.OS2Driver) {
        OSDRIVER_dofixups();
        ModemFixupsDone = 'O';
        OSDRIVER_openmodem(Show);
        return;
      }
      #ifdef __OS2__
        Asy.ComPortNumber = 0;
        return;
      #endif
    #endif

    #ifdef COMMDRV
      if (strchr(PcbData.ModemPort,'F') == NULL) {
        COMMDRV_dofixups();
        ModemFixupsDone = 'C';
        COMMDRV_openmodem(Show);
        return;
      }
    #endif

    #if defined(FOSSIL)
      FOSSIL_dofixups();
      ModemFixupsDone = 'F';
      FOSSIL_openmodem(Show);
      return;
    #endif
  #endif
}
#endif  /* ifdef COMM */
