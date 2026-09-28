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

#ifdef _MSC_VER
#include <borland.h>
#include <malloc.h>
#else
#include <alloc.h>
#endif

#ifdef LIB
  #define fbmalloc  farmalloc
  #define fbfree    farfree
  #define bmalloc   malloc
  #define bfree     free
#endif

/******************************************************************************
  REGULAR ASYNCH SUPPORT FUNCTIONS
******************************************************************************/

#define INBUFSIZE      4096         /* MUST be a power of TWO! */
#define OUTBUFSIZE     2048         /* MUST be a power of TWO! */
#define TRIGGER8        128         /* trigger 1 = 0, trigger 4 = 64 */

extern int OutBufSize;

int  LIBENTRY ASYNC_ringdetect(void);
int  LIBENTRY ASYNC_ctsokay(void);
int  LIBENTRY ASYNC_framingerrors(void);
int  LIBENTRY ASYNC_overrunerrors(void);
int  LIBENTRY ASYNC_parityerrors(void);
int  LIBENTRY ASYNC_outbytes(void);
int  LIBENTRY ASYNC_inbytes(void);


#ifdef MULTIPORT
int  (LIBENTRY *ringdetect)(void)                      = ASYNC_ringdetect;
int  (LIBENTRY *ctsokay)(void)                         = ASYNC_ctsokay;
int  (LIBENTRY *online)(void)                          = ASYNC_online;
int  (LIBENTRY *cdstillup)(void)                       = ASYNC_cdstillup;
int  (LIBENTRY *bauddivisor)(long PortSpeed)           = ASYNC_bauddivisor;
void (LIBENTRY *setport)(int BaudDivisor,int DataBits) = ASYNC_setport;
int  (LIBENTRY *inbytes)(void)                         = ASYNC_inbytes;
int  (LIBENTRY *outbytes)(void)                        = ASYNC_outbytes;
int  (LIBENTRY *framingerrors)(void)                   = ASYNC_framingerrors;
int  (LIBENTRY *overrunerrors)(void)                   = ASYNC_overrunerrors;
int  (LIBENTRY *parityerrors)(void)                    = ASYNC_parityerrors;
void (LIBENTRY *turnoffdtr)(void)                      = ASYNC_turnoffdtr;
void (LIBENTRY *turnondtr)(void)                       = ASYNC_turnondtr;
void (LIBENTRY *turnoffrts)(void)                      = ASYNC_turnoffrts;
void (LIBENTRY *turnonrts)(void)                       = ASYNC_turnonrts;
void (LIBENTRY *turnonxmit)(void)                      = ASYNC_turnonxmit;
void (LIBENTRY *clearoutbuf)(void)                     = ASYNC_clearoutbuf;
void (LIBENTRY *clearinbuf)(void)                      = ASYNC_clearinbuf;
void (LIBENTRY *commgo)(void)                          = ASYNC_commgo;
void (LIBENTRY *commstop)(void)                        = ASYNC_commstop;
void (LIBENTRY *commpause)(void)                       = ASYNC_commpause;
int  (LIBENTRY *checkcomm)(void)                       = ASYNC_checkcomm;
int  (LIBENTRY *comminkey)(void)                       = ASYNC_comminkey;
int  (LIBENTRY *cgetstr)(char *pStr, int StrLen)       = ASYNC_cgetstr;
int  (LIBENTRY *cgetbuf)(char *Buf, int BufLen)        = ASYNC_cgetbuf;
void (LIBENTRY *csendbyte)(unsigned char ByteToSend)   = ASYNC_csendbyte;
void (LIBENTRY *disconnectmodem)(void)                 = ASYNC_disconnectmodem;
void (LIBENTRY *csendstr)(char *pStr, int StrLen)      = ASYNC_csendstr;
void (LIBENTRY *reopenport)(void)                      = ASYNC_reopenport;

#ifndef LIB
void LIBENTRY (*reportcom)(long Speed);
#endif
#endif


char static far *ComPortBuffer = NULL;

int LIBENTRY ASYNC_ringdetect(void) { return(_RingDetect); }
int LIBENTRY ASYNC_ctsokay(void) { return(_CTSokay); }
int LIBENTRY ASYNC_framingerrors(void) { return(_FramingErrors); }
int LIBENTRY ASYNC_overrunerrors(void) { return(_OverrunErrors); }
int LIBENTRY ASYNC_parityerrors(void) { return(_ParityErrors); }

int LIBENTRY ASYNC_outbytes(void) {
  int NumBytes;
  disable();
  NumBytes = _OutBytes;
  enable();
  return(NumBytes);
}

int LIBENTRY ASYNC_inbytes(void) {
  int NumBytes;
  disable();
  NumBytes = _InBytes;
  enable();
  return(NumBytes);
}

int LIBENTRY ASYNC_bauddivisor(long PortSpeed) {
  int CPS;

  CPS = (int) (PortSpeed/10);
  switch (CPS) {
    case    30:  return(0x0180);
    case   120:  return(0x0060);
    case   240:  return(0x0030);
    case   480:  return(0x0018);
    case   960:  return(0x000C);
    case  1920:  return(0x0006);
    case  3840:  return(0x0003);
    case  5760:  return(0x0002);
    case 11520:  return(0x0001);
    default   :  /* Invalid Port Speed - set it to 1200 */
                 return(0x0060);
  }
}

void LIBENTRY ASYNC_disconnectmodem(void) {
  ASYNC_closecom();

  if (ComPortBuffer != NULL) {
    fbfree(ComPortBuffer);
    ComPortBuffer = NULL;
  }
  ModemOpened = FALSE;
}

#ifndef LIB
void LIBENTRY ASYNC_reportcom(long Speed) {
  char Str[80];
  char Temp[20];

  if (B16550A)
    strcpy(Temp,"16550A FIFO");
  else if (B16550)
    strcpy(Temp,"16550 FIFO");
  else if (B8250)
    strcpy(Temp,"8250 OLD");
  else
    strcpy(Temp,"8250A/16450");

  sprintf(Str,"COM%d (%s at IRQ:%d ADDRESS:%X)  %ld:%d,%c,1",
          Asy.ComPortNumber,
          Temp,
          PcbData.IrqNum,
          PcbData.BaseAddress,
          Speed,
          Asy.DataBits,
          (Asy.DataBits == 8 ? 'N' : 'E'));
  writedebugrecord(Str);
}
#endif


void LIBENTRY ASYNC_openmodem(showtype Show) {
  int  Count;
  int  RetVal;
  char Str[128];

  if (Asy.ComPortNumber == 0 || ! ModemFixupsDone)
    return;

  if (Asy.ComPortNumber > 2 && (PcbData.IrqNum == 0 || PcbData.BaseAddress == 0))
    return;

  OutBufSize = OUTBUFSIZE;

  if ((ComPortBuffer = (char far *) fbmalloc(INBUFSIZE+OUTBUFSIZE)) == NULL) {
    sprintf(Str,"insufficient memory for comm buffers: %u : %ld",INBUFSIZE+OUTBUFSIZE,farcoreleft());
    errorexittodos(Str);
    return;
  }

  ASYNC_init(PcbData.IrqNum,PcbData.BaseAddress,ComPortBuffer,&ComPortBuffer[INBUFSIZE],INBUFSIZE,OUTBUFSIZE,!PcbData.DisableCTS,PcbData.ShareIRQs);

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

    ASYNC_closecom();
    RetVal = ASYNC_opencom(ASYNC_bauddivisor(Asy.ModemSpeed),Asy.DataBits);
    if (RetVal <= -1) {
      errorexittodos("Invalid comm port or UART not functioning");
      return;
    }

    ASYNC_turnonfifo(TRIGGER8);
    if (initializemodem(Show))
      break;
  }
}

void LIBENTRY ASYNC_reopenport(void) {
  ASYNC_openmodem(HIDE);
}

#ifdef MULTIPORT
void LIBENTRY ASYNC_dofixups(void) {
  ringdetect      = ASYNC_ringdetect;
  ctsokay         = ASYNC_ctsokay;
  online          = ASYNC_online;
  cdstillup       = ASYNC_cdstillup;
  bauddivisor     = ASYNC_bauddivisor;
  setport         = ASYNC_setport;
  inbytes         = ASYNC_inbytes;
  outbytes        = ASYNC_outbytes;
  framingerrors   = ASYNC_framingerrors;
  overrunerrors   = ASYNC_overrunerrors;
  parityerrors    = ASYNC_parityerrors;
  turnoffdtr      = ASYNC_turnoffdtr;
  turnondtr       = ASYNC_turnondtr;
  turnoffrts      = ASYNC_turnoffrts;
  turnonrts       = ASYNC_turnonrts;
  turnonxmit      = ASYNC_turnonxmit;
  clearoutbuf     = ASYNC_clearoutbuf;
  clearinbuf      = ASYNC_clearinbuf;
  commgo          = ASYNC_commgo;
  commstop        = ASYNC_commstop;
  commpause       = ASYNC_commpause;
  checkcomm       = ASYNC_checkcomm;
  comminkey       = ASYNC_comminkey;
  cgetstr         = ASYNC_cgetstr;
  cgetbuf         = ASYNC_cgetbuf;
  csendbyte       = ASYNC_csendbyte;
  csendstr        = ASYNC_csendstr;
  disconnectmodem = ASYNC_disconnectmodem;
  reopenport      = ASYNC_reopenport;

#ifndef LIB
  reportcom       = ASYNC_reportcom;
#endif
}
#endif  /* MULTIPORT */
#endif  /* COMM */
