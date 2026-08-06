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


#if defined(COMM) && defined(MULTIPORT) && defined(OSDRIVER)

#include "project.h"
#pragma hdrstop

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <borland.h>
  #include <malloc.h>
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#include <devioctl.h>

#ifdef __OS2__
  #define  INCL_DOSPROCESS
  #include <os2.h>
  #include <threads.h>
  #include <semafore.hpp>
#else
//  #pragma inline
#endif

#ifdef LIB
  #define fbmalloc  farmalloc
  #define fbfree    farfree
  #define bmalloc   malloc
  #define bfree     free
#endif

/******************************************************************************
  OSDRIVER SUPPORT FUNCTIONS
******************************************************************************/

#ifdef __OS2__
  #define OSINBUFSIZE   16384         /* MUST be a power of TWO! */
#else
  #define OSINBUFSIZE    2048         /* MUST be a power of TWO! */
#endif

extern int OutBufSize;

#ifdef __OS2__
  extern int ComHandle;
#endif

static char *OSBuffer = NULL;
static int   OSPort;
static int   OSInBytes;
static int   OSHeadPtr;
static int   OSTailPtr;

#ifdef __OS2__
  enum { TH_MUSTEXIT, TH_INTHREAD, TH_EXITED };
  static bool ModemThreadState;   // thread state used for modemreadthread
  static bool MonitorThreadState; // thread state used for modemreadthread
  static unsigned long ModemThreadId;
  static unsigned long MonitorThreadId;
  static CMutexSemaphore OSInBytesSemaphore;
#endif


static int LIBENTRY OSDRIVER_ringdetect(void) {
  #ifndef LIB
    if (AnswerOnStartup)
      return(TRUE);
  #endif
  return(FALSE);
}

static int LIBENTRY OSDRIVER_ctsokay(void) {
  return(TRUE);
}

static int LIBENTRY OSDRIVER_online(void) {
  char ModemStatus;

  #ifdef __OS2__
  if (getioctrl(0x01,0x67,&ModemStatus,sizeof(ModemStatus)) != 0) {
  #else
  if (getioctrl(0x0167,&ModemStatus) != 0) {
  #endif
    CDokay = FALSE;
    return(FALSE);
  }

  ModemStatus &= 0x80;
  CDokay = ModemStatus;
  return(ModemStatus);
}


static int LIBENTRY OSDRIVER_cdstillup(void) {
  int X;
  #ifndef __OS2__
  long Ticks;
  long static LastTicks;
  #endif

  if (VerifyCDLoss) {
    /* To reduce the overhead in a DOS application, we'll avoid calling */
    /* the online() function any more than once per second by checking  */
    /* to see if 1 second has elapsed since our last check and, if not  */
    /* we'll simply return TRUE if the CDokay variable shows the CD     */
    /* signal was last up within the past second.                       */
    #ifndef __OS2__
      if (CDokay) {
        Ticks = getticks() - LastTicks;
        if (Ticks >= 0 && Ticks < 18)
          return(TRUE);
      }
    #endif
    for (X = 0; X < 15; X++) {
      if (online()) {
        #ifndef __OS2__
          LastTicks = getticks();
        #endif
        return(TRUE);  /* cd is up */
      }
      #ifdef __OS2__
        DosSleep(250);
      #else
        settimer(4,QUARTERSECOND);
        while (! timerexpired(4))
          giveup();
      #endif
    }
    goto cdoff;
  }

  if (online())
    return(TRUE);

  cdoff:
  clearoutbuf();
  return(FALSE);
}

#ifndef LIB
#pragma warn -par
static void LIBENTRY OSDRIVER_reportcom(long Speed) {
}
#pragma warn +par
#endif

static int LIBENTRY OSDRIVER_bauddivisor(long PortSpeed) {
  return((int) (PortSpeed/10));
}

typedef struct {
  long BaudRate;
  char Fraction;
} extendedrate;

typedef struct {
  char DataBits;
  char Parity;
  char StopBits;
} lstype;

static void LIBENTRY OSDRIVER_setport(int BaudRate, int DataBits) {
  extendedrate Extended;
  lstype       LS;
  long         Word;

  if (BaudRate != 11520) {
    Word = BaudRate * 10L;
    #ifdef __OS2__
      setioctrl(0x01,0x41,&Word,sizeof(Word));
    #else
      setioctrl(0x0141,&Word);
    #endif
  } else {
    Extended.BaudRate = 115200L;
    Extended.Fraction = 0;
    #ifdef __OS2__
      setioctrl(0x01,0x43,&Extended,sizeof(Extended));
    #else
      setioctrl(0x0143,&Extended);
    #endif
  }

  LS.StopBits = 0;
  if (DataBits == 8) {
    LS.DataBits = 8;
    LS.Parity = 0;
  } else {
    LS.DataBits = 7;
    LS.Parity = 2;
  }

  #ifdef __OS2__
    setioctrl(0x01,0x42,&LS,sizeof(LS));
  #else
    setioctrl(0x0142,&LS);
  #endif
}

typedef struct {
  unsigned short NumBytes;
  unsigned short Size;
} qtype;


static void _NEAR_ LIBENTRY updateinbytes(int AddBytes) {
  #ifdef __OS2__
    OSInBytesSemaphore.acquire();
  #endif

  // AddBytes will be positive if bytes have been received, or
  // negative if we have just removed bytes
  OSInBytes += AddBytes;

  #ifdef __OS2__
    OSInBytesSemaphore.release();
  #endif
}


static void _NEAR_ LIBENTRY OSDRIVER_readport(void) {
  int    FreeBytes;
  int    BytesRead;
  int    BytesToBufEnd;
  char * pTempBuf;
  #ifdef __OS2__
  char   static TempBuf[OSINBUFSIZE];   // static to reduce stack size
  #else
  char          TempBuf[OSINBUFSIZE];   // non-static to reduce DGROUP
  #endif
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  #ifdef DEBUG
    if (OSBuffer == NULL) {
      writedebugrecord("call to OSDRIVER_readport() while closed");
      return;
    }
  #endif

  #ifdef __OS2__
  // set to TIMECRITICAL so that OS/2 doesn't adjust the priority
  DosSetPriority(PRTYS_THREAD,PRTYC_TIMECRITICAL,1,0);

  while (ModemThreadState != TH_MUSTEXIT) {
  #endif

    /* find out if there is any room in our local buffer */
    if ((FreeBytes = OSINBUFSIZE - OSInBytes) > 0) {

      /* point to temporary buffer for moving data between dosread and OSBuffer */
      pTempBuf = TempBuf;

      /* there is room, now see if OS/2 has anything for us */
      if ((BytesRead = dosread(OSPort,pTempBuf,FreeBytes POS2ERROR)) > 0) {

        /* if there are no bytes in the buffer, reset the head pointer to 0 */
        /* because doing so can avoid the extra code used for wrapping      */
        if (OSInBytes == 0)
          OSHeadPtr = OSTailPtr = 0;

        /* there is, now find out if copying it will wrap around the buffer */
        BytesToBufEnd = OSINBUFSIZE - OSHeadPtr;
        if (BytesRead > BytesToBufEnd) {
          /* it needs to wrap, so copy only to the end of the buffer first */
          memcpy(OSBuffer+OSHeadPtr,pTempBuf,BytesToBufEnd);
          /* then reset the OSHeadPtr back to the beginning */
          OSHeadPtr = 0;
          /* and reduce the number of bytes to copy */
          BytesRead -= BytesToBufEnd;
          /* move pTempBuf forward so that it can be used down below */
          pTempBuf += BytesToBufEnd;
          /* Count the number of bytes copied so far */
          updateinbytes(BytesToBufEnd);
//        OSInBytes += BytesToBufEnd;
        }

        /* copy bytes out of temporary buffer into OSBuffer */
        memcpy(OSBuffer+OSHeadPtr,pTempBuf,BytesRead);

        OSHeadPtr += BytesRead;        /* Move the head pointer forward    */
        OSHeadPtr &= (OSINBUFSIZE-1);  /* wrap the pointer if necessary    */
        updateinbytes(BytesRead);
//      OSInBytes += BytesRead;        /* count the number of bytes copied */
        #ifdef __OS2__
          releasekbdblock(0);
        #endif
      }
    #ifdef __OS2__
    } else {
      // This will only occur in an OS/2 application:
      //
      // There was not enough room for us to read anything, the application
      // must be slow in picking up data, but if we just loop back up and
      // check again we're gonna hog the cpu!  To avoid that, we'll give up
      // some time now and let the application catch up.
      mydelay(50);
    #endif
    }

  #ifdef __OS2__
  }
  ModemThreadState = TH_EXITED;
  #endif
}


#ifdef __OS2__
static CSemaphore ModemMonitorSemaphore;

#pragma argsused
static void THREADFUNC modemreadthread(void * Ignore) {
  ModemThreadState = TH_INTHREAD;
  OSDRIVER_readport();
}

#pragma argsused
static void THREADFUNC modemmonitorthread(void * Ignore) {
  int  Save;
  int  New;

  MonitorThreadState = TH_INTHREAD;
  Save = online();

  while (MonitorThreadState != TH_MUSTEXIT) {
    ModemMonitorSemaphore.waitforevent(ONESECOND);
    if (Asy.Online == REMOTE) {
      if ((New = cdstillup()) != Save) {
        if (! New) {
          Asy.LostCarrier = TRUE;
          if (PcbData.Packet)
            turnoffdtr();
        }
        releasekbdblock(0);
        Save = New;
      }
    } else {
      if ((New = online()) != Save) {
        releasekbdblock(0);
        Save = New;
      }
    }
  }

  MonitorThreadState = TH_EXITED;
}

void LIBENTRY createmodemthread(void) {
  if (ModemMonitorSemaphore.createunique("MODEM")) {
    if (OSInBytesSemaphore.createunique("INBYTE")) {
      ModemThreadId   = startthread(modemreadthread,16*1024,NULL);
      MonitorThreadId = startthread(modemmonitorthread,8*1024,NULL);
    }
  }
}

void LIBENTRY destroymodemthread(void) {
  MonitorThreadState = TH_MUSTEXIT;
  ModemThreadState   = TH_MUSTEXIT;
  ModemMonitorSemaphore.postevent();
  killthread(MonitorThreadId,TRUE);
  killthread(ModemThreadId,TRUE);
//waitthread(MonitorThreadId);
//waitthread(ModemThreadId);
//while (MonitorThreadState != TH_EXITED || ModemThreadState != TH_EXITED)
//  DosSleep(250);
  ModemMonitorSemaphore.close();
  OSInBytesSemaphore.close();
}
#endif


static int LIBENTRY OSDRIVER_inbytes(void) {
  #ifdef __OS2__
    return(OSInBytes);
  #else
    if (OSInBytes != 0)
      return(OSInBytes);

    OSDRIVER_readport();
    return(OSInBytes);
  #endif
}


static int LIBENTRY OSDRIVER_outbytes(void) {
  qtype Queue;

  #ifdef __OS2__
  if (getioctrl(0x01,0x69,&Queue,sizeof(Queue)) != 0)
  #else
  if (getioctrl(0x0169,&Queue) != 0)
  #endif
    return(0);

  return(Queue.NumBytes);
}


static int LIBENTRY OSDRIVER_framingerrors(void) {
  return(0);
}

static int LIBENTRY OSDRIVER_overrunerrors(void) {
  return(0);
}

static int LIBENTRY OSDRIVER_parityerrors(void) {
  return(0);
}

typedef struct {
  char OnBits;
  char OffBits;
} onoffmask;

static void LIBENTRY OSDRIVER_turnoffdtr(void) {
  onoffmask Mask;
  long      RetVal;

  RetVal = 0;
  Mask.OnBits  = 0x00;
  Mask.OffBits = 0xFE;

  #ifdef __OS2__
    DevIOCtl(OSPort,0x01,0x46,&RetVal,sizeof(RetVal),&Mask,sizeof(Mask));
  #else
    DevIOCtl(OSPort,0x146,&RetVal,&Mask);
  #endif
}

static void LIBENTRY OSDRIVER_turnondtr(void) {
  onoffmask Mask;
  long      RetVal;

  RetVal = 0;
  Mask.OnBits  = 0x01;
  Mask.OffBits = 0xFF;
  #ifdef __OS2__
    DevIOCtl(OSPort,0x01,0x46,&RetVal,sizeof(RetVal),&Mask,sizeof(Mask));
  #else
    DevIOCtl(OSPort,0x146,&RetVal,&Mask);
  #endif
}

static void LIBENTRY OSDRIVER_turnoffrts(void) {
  onoffmask Mask;
  long      RetVal;

  RetVal = 0;
  Mask.OnBits  = 0x00;
  Mask.OffBits = 0xFD;
  #ifdef __OS2__
    DevIOCtl(OSPort,0x01,0x46,&RetVal,sizeof(RetVal),&Mask,sizeof(Mask));
  #else
    DevIOCtl(OSPort,0x146,&RetVal,&Mask);
  #endif
}

static void LIBENTRY OSDRIVER_turnonrts(void) {
  onoffmask Mask;
  long      RetVal;

  RetVal = 0;
  Mask.OnBits  = 0x02;
  Mask.OffBits = 0xFF;
  #ifdef __OS2__
    DevIOCtl(OSPort,0x01,0x46,&RetVal,sizeof(RetVal),&Mask,sizeof(Mask));
  #else
    DevIOCtl(OSPort,0x146,&RetVal,&Mask);
  #endif
}

#ifndef __OS2__
static void LIBENTRY OSDRIVER_turnonxmit(void) {
}
#endif

static void LIBENTRY OSDRIVER_clearoutbuf(void) {
}

static void LIBENTRY OSDRIVER_clearinbuf(void) {
  OSInBytes = 0;
  OSHeadPtr = 0;
  OSTailPtr = 0;
}

static void LIBENTRY OSDRIVER_commgo(void) {
  #ifdef __OS2__
    callioctrl(0x01,0x48);
  #else
    callioctrl(0x0148);
  #endif
}

static void LIBENTRY OSDRIVER_commstop(void) {
  #ifdef __OS2__
    callioctrl(0x01,0x47);
  #else
    callioctrl(0x0147);
  #endif
}

static void LIBENTRY OSDRIVER_commpause(void) {
}

static int LIBENTRY OSDRIVER_checkcomm(void) {
  int   NumBytes;
  char *p;
  #ifndef __OS2__
  long  CurTicks;
  long  static LastTicks;
  long  Ticks;
  #endif

  #ifdef DEBUG
    if (OSBuffer == NULL) {
      writedebugrecord("call to OSDRIVER_checkcomm() while closed");
      return(0);
    }
  #endif

  #ifndef __OS2__
    CurTicks = getticks();
    Ticks = CurTicks - LastTicks;
    if (Ticks > 18 || Ticks < 0) {
      OSDRIVER_readport();
      LastTicks = CurTicks;
    }
  #endif

  if (OSInBytes == 0)
    return(0);

  NumBytes = OSInBytes;
  if (NumBytes > OSINBUFSIZE-OSTailPtr)
    NumBytes = OSINBUFSIZE-OSTailPtr;

  p = OSBuffer + OSTailPtr;
  if (memchr(p,19,(unsigned) NumBytes) != NULL)
    return(19);
  if (memchr(p,24,(unsigned) NumBytes) != NULL)
    return(24);
  if (memchr(p,11,(unsigned) NumBytes) != NULL)
    return(11);
  return(0);
}

static int LIBENTRY OSDRIVER_comminkey(void) {
  char Byte;

  #ifdef DEBUG
    if (OSBuffer == NULL) {
      writedebugrecord("call to OSDRIVER_comminkey() while closed");
      return(-1);
    }
  #endif

  if (InBytes == 0)
    return(-1);

  Byte = (char) OSBuffer[OSTailPtr++];
  OSTailPtr &= (OSINBUFSIZE-1);         /* wrap the pointer if necessary */
  updateinbytes(-1);
//OSInBytes--;
  return(Byte);
}

static int LIBENTRY OSDRIVER_cgetstr(char *pStr, int StrLen) {
  int  NumBytesFound;
  int  NumBytesToCopy;
  int  NumBytesToBufEnd;

  #ifdef DEBUG
    if (OSBuffer == NULL) {
      writedebugrecord("call to OSDRIVER_cgetstr() while closed");
      return(0);
    }
  #endif

  StrLen--;  /* make room for the NULL terminator */
  NumBytesFound = InBytes;

  /* don't let the number of bytes found overrun the size of the buffer */
  if (NumBytesFound > StrLen)
    NumBytesFound = StrLen;

  if (NumBytesFound != 0) {
    NumBytesToCopy = NumBytesFound;

    /* found out how many bytes there are from the tail pointer to the end */
    /* of the buffer so that, if the buffer wraps around the end we can    */
    /* memcpy() the tail portion first, then add the wrapped around part   */

    NumBytesToBufEnd = OSINBUFSIZE - OSTailPtr;
    if (NumBytesToCopy > NumBytesToBufEnd) {
      /* copy from tail pointer to the end of the buffer */
      memcpy(pStr,OSBuffer+OSTailPtr,NumBytesToBufEnd);

      /* move the buffer pointer forward so that we can use it below */
      pStr += NumBytesToBufEnd;

      /* now reset the tail pointer, and reduce the number of bytes to copy */
      OSTailPtr = 0;
      NumBytesToCopy -= NumBytesToBufEnd;
    }

    /* copy the bytes from the OS buffer into the user buffer */
    memcpy(pStr,OSBuffer+OSTailPtr,NumBytesToCopy);

    /* NULL-terminate the string */
    pStr[NumBytesToCopy] = 0;

    /* advance the tail pointer by the number of bytes copied */
    OSTailPtr += NumBytesToCopy;

    /* wrap the tail pointer around if necessary */
    OSTailPtr &= (OSINBUFSIZE-1);

    /* reduce the number of bytes recorded in the buffer */
    updateinbytes(-NumBytesFound);
//  OSInBytes -= NumBytesFound;
  }

  return(NumBytesFound);
}


static int LIBENTRY OSDRIVER_cgetbuf(char *Buf, int BufLen) {
  int  NumBytesFound;
  int  NumBytesToCopy;
  int  NumBytesToBufEnd;

  #ifdef DEBUG
    if (OSBuffer == NULL) {
      writedebugrecord("call to OSDRIVER_cgetbuf() while closed");
      return(0);
    }
  #endif

  NumBytesFound = InBytes;

  /* don't let the number of bytes found overrun the size of the buffer */
  if (NumBytesFound > BufLen)
    NumBytesFound = BufLen;

  if (NumBytesFound != 0) {
    NumBytesToCopy = NumBytesFound;

    /* find out how many bytes there are from the tail pointer to the end  */
    /* of the buffer so that, if the buffer wraps around the end we can    */
    /* memcpy() the tail portion first, then add the wrapped around part   */

    NumBytesToBufEnd = OSINBUFSIZE - OSTailPtr;
    if (NumBytesToCopy > NumBytesToBufEnd) {
      /* copy from tail pointer to the end of the buffer */
      memcpy(Buf,OSBuffer+OSTailPtr,NumBytesToBufEnd);

      /* move the buffer pointer forward so that we can use it below */
      Buf += NumBytesToBufEnd;

      /* now reset the tail pointer, and reduce the number of bytes to copy */
      OSTailPtr = 0;
      NumBytesToCopy -= NumBytesToBufEnd;
    }

    /* copy the bytes from the OS buffer into the user buffer */
    memcpy(Buf,OSBuffer+OSTailPtr,NumBytesToCopy);

    /* advance the tail pointer by the number of bytes copied */
    OSTailPtr += NumBytesToCopy;

    /* wrap the tail pointer around if necessary */
    OSTailPtr &= (OSINBUFSIZE-1);

    /* reduce the number of bytes recorded in the buffer */
    updateinbytes(-NumBytesFound);
//  OSInBytes -= NumBytesFound;
  }

  return(NumBytesFound);
}


static void LIBENTRY OSDRIVER_csendbyte(unsigned char ByteToSend) {
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  doswrite(OSPort,&ByteToSend,1 POS2ERROR);      /*lint !e534 */
}

static void LIBENTRY OSDRIVER_csendstr(char *pStr, int StrLen) {
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  doswrite(OSPort,pStr,StrLen POS2ERROR);        /*lint !e534 */
}

#ifdef _MSC_VER
typedef struct {
  char DTR:2;
  char Zero1:1;
  char IgnoreCTS:1;
  char HandshakeDSR:1;
  char DCDrequired:1;
  char DSRrequired:1;
  char Zero2:1;
} f1type;
typedef struct {
  char ReceiveXonXoff:1;
  char SendXonXoff:1;
  char EnableErrChar:1;
  char EnableNullStrip:1;
  char EnableBreak:1;
  char EnableFullDuplex:1;
  char RTScontrol:2;
} f2type;
typedef struct {
  char InfiniteWriteTimeout:1;
  char ReadTimeout:2;
  char FIFOcontrol:2;
  char FIFOtrigger:2;
  char TransmitLoad:1;
} f3type;
#else
typedef struct {
  int  DTR:2;
  int  Zero1:1;
  int  IgnoreCTS:1;
  int  HandshakeDSR:1;
  int  DCDrequired:1;
  int  DSRrequired:1;
  int  Zero2:1;
} f1type;
typedef struct {
  int  ReceiveXonXoff:1;
  int  SendXonXoff:1;
  int  EnableErrChar:1;
  int  EnableNullStrip:1;
  int  EnableBreak:1;
  int  EnableFullDuplex:1;
  int  RTScontrol:2;
} f2type;
typedef struct {
  int  InfiniteWriteTimeout:1;
  int  ReadTimeout:2;
  int  FIFOcontrol:2;
  int  FIFOtrigger:2;
  int  TransmitLoad:1;
} f3type;
#endif

typedef struct {
  unsigned short WriteTimeout;
  unsigned short ReadTimeout;
  f1type         Flags1;
  f2type         Flags2;
  f3type         Flags3;
  unsigned char  ErrReplaceByte;
  unsigned char  BrkReplaceByte;
  unsigned char  XonChar;
  unsigned char  XoffChar;
} dcbtype;


typedef struct {
  int  Len;       /* structure length */
  char ID[3];     /* SIO signature "SIO" */
  char Ver[8];    /* version number (ascii+NULL terminated) */
  char Bld[6];    /* build number (ascii+NULL terminated) */
  char SN[12];    /* serial number (ascii+NULL terminated) */
  int  NumPorts;  /* number of ports supported */
  int  Mouse;     /* 1 relative mouse port, 0 if not comm */
  int  FlagBits;  /* 0 - On if PMLM enabled, 1-15 reserved */
  char IsReg;     /* TRUE if registered or beta SIO */
} aboutsio;

static bool UsingSIO = FALSE;

typedef struct {
  int  Len;       /* structure length */
  char Type;      /* 0=Vmodem not in use, 1=No connection, 2=Vmodem, 3=Telnet */
} aboutvmodem;


bool LIBENTRY istelnetviavmodem(void) {
  aboutvmodem Vmodem;

  if (! UsingSIO)
    return(FALSE);

  Vmodem.Len  = 3;
  Vmodem.Type = 0;
  #ifdef __OS2__
  if (getioctrl(0x01,0xE8,&Vmodem,sizeof(Vmodem)) != -1 && Vmodem.Type == 3)
  #else
  if (getioctrl(0x01E8,&Vmodem) != -1 && Vmodem.Type == 3)
  #endif
    return(TRUE);
  return(FALSE);
}


bool LIBENTRY isvmodem(void) {
  aboutvmodem Vmodem;

  if (! UsingSIO)
    return(FALSE);

  Vmodem.Len  = 3;
  Vmodem.Type = 0;
  #ifdef __OS2__
  if (getioctrl(0x01,0xE8,&Vmodem,sizeof(Vmodem)) != -1 && Vmodem.Type == 2)
  #else
  if (getioctrl(0x01E8,&Vmodem) != -1 && Vmodem.Type == 2)
  #endif
    return(TRUE);
  return(FALSE);
}


static int _NEAR_ LIBENTRY OSDRIVER_getdriverinfo(void) {
  qtype    Queue;
  dcbtype  DCB;
  aboutsio SIO;

  #ifdef __OS2__
  if (getioctrl(0x01,0x73,&DCB,sizeof(DCB)) == -1 /* || DCB.XonChar != 17 || DCB.XoffChar != 19 */)
  #else
  if (getioctrl(0x0173,&DCB) == -1 /* || DCB.XonChar != 17 || DCB.XoffChar != 19 */)
  #endif
    errorexittodos("Could not get OS/2 driver info");

  #ifdef __OS2__
  DCB.ReadTimeout = 1000;  // just experimenting, maybe this won't work - or maybe it can be used in DOS too!
  #else
  DCB.ReadTimeout = 1;
  #endif
  DCB.WriteTimeout = 6000;
  DCB.Flags3.ReadTimeout = 2;
  DCB.Flags1.HandshakeDSR = 0;
  DCB.Flags1.DTR = 0;             /* Digiboard says this will disable DTR drop on port close - 06/03/94 */

/*DevIOCtl(OSPort,0x153,NULL,&DCB);*/

  #ifdef __OS2__
    setioctrl(0x01,0x53,&DCB,sizeof(DCB));
  #else
    setioctrl(0x153,&DCB);
  #endif

  OutBufSize = 0;
  #ifdef __OS2__
  if (getioctrl(0x01,0x69,&Queue,sizeof(Queue)) == 0)
  #else
  if (getioctrl(0x0169,&Queue) == 0)
  #endif
    OutBufSize = Queue.Size;

  memset(&SIO,0,sizeof(SIO));
  SIO.Len = sizeof(SIO);

  #ifdef __OS2__
  if (getioctrl(0x01,0xE0,&SIO,sizeof(SIO)) != -1 && memcmp(SIO.ID,"SIO",3) == 0)
  #else
  if (getioctrl(0x01E0,&SIO) != -1 && memcmp(SIO.ID,"SIO",3) == 0)
  #endif
    UsingSIO = TRUE;
  else
    UsingSIO = FALSE;

  return(0);
}

static void LIBENTRY OSDRIVER_disconnectmodem(void) {
  #ifdef __OS2__
    destroymodemthread();
  #endif

  dosclose(OSPort);
  OSPort = 0;

  if (OSBuffer != NULL) {
    bfree(OSBuffer);
    OSBuffer = NULL;
  }

  ModemOpened = FALSE;
  OSInBytes = OSHeadPtr = OSTailPtr = 0;
}


int LIBENTRY OSDRIVER_pcbhandle(void) {
  return(ModemOpened ? OSPort : 0);
}


void LIBENTRY OSDRIVER_openmodem(showtype Show) {
  int  Count;
  char Str[80];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  #ifdef __OS2__
    if (ComHandle != 0)
      Asy.ComPortNumber = (char) ComHandle;
  #endif

  if (Asy.ComPortNumber == 0 || ! ModemFixupsDone)
    return;

  if ((OSBuffer = (char *) bmalloc(OSINBUFSIZE)) == NULL) {
    #ifdef __OS2__
      sprintf(Str,"insufficient memory for comm buffers: %u",OSINBUFSIZE);
    #else
      sprintf(Str,"insufficient memory for comm buffers: %u : %ld",OSINBUFSIZE,coreleft());
    #endif
    errorexittodos(Str);
    return;
  }

  OSInBytes = OSHeadPtr = OSTailPtr = 0;
  #ifdef DEBUG
    memset(OSBuffer,0,OSINBUFSIZE);
  #endif


  sprintf(Str,"COM%d",Asy.ComPortNumber);

  for (Count = 0; TRUE; Count++) {     /*lint !e506 */
    #ifndef LIB
      if (Count == 2) {
        closemodem(TRUE);
        Status.ErrorLevel = EXIT_RECYCLE;
        PcbData.OffHook    = FALSE;
        PcbData.ResetModem = FALSE;
        PcbData.ExitToDos  = TRUE;
        recycle();
      }
    #endif

    #ifdef __OS2__
      if (ComHandle != 0)
        OSPort = dosdup(ComHandle POS2ERROR);
      else
        OSPort = dosopen(Str,OPEN_RDWR|OPEN_DENYNONE POS2ERROR);
    #else
      OSPort = dosopen(Str,OPEN_RDWR|OPEN_DENYNONE);
    #endif

    if (OSPort == -1) {
      errorexittodos("Unable to open Operating System COMM Port Driver");
      return;
    }

    #ifdef __OS2__
      createmodemthread();
    #endif

    setioctrlhandle(OSPort);
    OSDRIVER_getdriverinfo();  /*lint !e534 */
    setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);

    if (initializemodem(Show))
      break;

    #ifdef __OS2__
      destroymodemthread();
    #endif

    dosclose(OSPort);
    OSPort = 0;
  }

  #ifndef LIB
    setpcbenv(NULL,TRUE);  /* set up the PCB environment variables */
  #endif
  tickdelay((PcbData.ModemDelay*HALFSECOND)+QUARTERSECOND);
}


static void LIBENTRY OSDRIVER_reopenport(void) {
  OSDRIVER_openmodem(HIDE);
}


#ifdef __OS2__
char volatile CDokay = 128;

int  (LIBENTRY *ringdetect)(void)                      = OSDRIVER_ringdetect;
int  (LIBENTRY *ctsokay)(void)                         = OSDRIVER_ctsokay;
int  (LIBENTRY *online)(void)                          = OSDRIVER_online;
int  (LIBENTRY *cdstillup)(void)                       = OSDRIVER_cdstillup;
int  (LIBENTRY *bauddivisor)(long PortSpeed)           = OSDRIVER_bauddivisor;
void (LIBENTRY *setport)(int BaudDivisor,int DataBits) = OSDRIVER_setport;
int  (LIBENTRY *inbytes)(void)                         = OSDRIVER_inbytes;
int  (LIBENTRY *outbytes)(void)                        = OSDRIVER_outbytes;
int  (LIBENTRY *framingerrors)(void)                   = OSDRIVER_framingerrors;
int  (LIBENTRY *overrunerrors)(void)                   = OSDRIVER_overrunerrors;
int  (LIBENTRY *parityerrors)(void)                    = OSDRIVER_parityerrors;
void (LIBENTRY *turnoffdtr)(void)                      = OSDRIVER_turnoffdtr;
void (LIBENTRY *turnondtr)(void)                       = OSDRIVER_turnondtr;
void (LIBENTRY *turnoffrts)(void)                      = OSDRIVER_turnoffrts;
void (LIBENTRY *turnonrts)(void)                       = OSDRIVER_turnonrts;
void (LIBENTRY *clearoutbuf)(void)                     = OSDRIVER_clearoutbuf;
void (LIBENTRY *clearinbuf)(void)                      = OSDRIVER_clearinbuf;
void (LIBENTRY *commgo)(void)                          = OSDRIVER_commgo;
void (LIBENTRY *commstop)(void)                        = OSDRIVER_commstop;
void (LIBENTRY *commpause)(void)                       = OSDRIVER_commpause;
int  (LIBENTRY *checkcomm)(void)                       = OSDRIVER_checkcomm;
int  (LIBENTRY *comminkey)(void)                       = OSDRIVER_comminkey;
int  (LIBENTRY *cgetstr)(char *pStr, int StrLen)       = OSDRIVER_cgetstr;
int  (LIBENTRY *cgetbuf)(char *Buf, int BufLen)        = OSDRIVER_cgetbuf;
void (LIBENTRY *csendbyte)(unsigned char ByteToSend)   = OSDRIVER_csendbyte;
void (LIBENTRY *disconnectmodem)(void)                 = OSDRIVER_disconnectmodem;
void (LIBENTRY *csendstr)(char *pStr, int StrLen)      = OSDRIVER_csendstr;
void (LIBENTRY *reopenport)(void)                      = OSDRIVER_reopenport;

#ifndef LIB
void LIBENTRY (*reportcom)(long Speed);
#endif  // ifndef LIB
#endif  // ifdef __OS2__

void LIBENTRY OSDRIVER_dofixups(void) {
  ringdetect      = OSDRIVER_ringdetect;
  ctsokay         = OSDRIVER_ctsokay;
  online          = OSDRIVER_online;
  cdstillup       = OSDRIVER_cdstillup;
  bauddivisor     = OSDRIVER_bauddivisor;
  setport         = OSDRIVER_setport;
  inbytes         = OSDRIVER_inbytes;
  outbytes        = OSDRIVER_outbytes;
  framingerrors   = OSDRIVER_framingerrors;
  overrunerrors   = OSDRIVER_overrunerrors;
  parityerrors    = OSDRIVER_parityerrors;
  turnoffdtr      = OSDRIVER_turnoffdtr;
  turnondtr       = OSDRIVER_turnondtr;
  turnoffrts      = OSDRIVER_turnoffrts;
  turnonrts       = OSDRIVER_turnonrts;
  #ifndef __OS2__
  turnonxmit      = OSDRIVER_turnonxmit;
  #endif
  clearoutbuf     = OSDRIVER_clearoutbuf;
  clearinbuf      = OSDRIVER_clearinbuf;
  commgo          = OSDRIVER_commgo;
  commstop        = OSDRIVER_commstop;
  commpause       = OSDRIVER_commpause;
  checkcomm       = OSDRIVER_checkcomm;
  comminkey       = OSDRIVER_comminkey;
  cgetstr         = OSDRIVER_cgetstr;
  cgetbuf         = OSDRIVER_cgetbuf;
  csendbyte       = OSDRIVER_csendbyte;
  csendstr        = OSDRIVER_csendstr;
  disconnectmodem = OSDRIVER_disconnectmodem;
  reopenport      = OSDRIVER_reopenport;

#ifndef LIB
  reportcom       = OSDRIVER_reportcom;
#endif
}

#endif  /* COMM && MULTIPORT && OSDRIVER */
