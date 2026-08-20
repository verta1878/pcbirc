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


#if defined(COMM) && defined(MULTIPORT)

#include "project.h"
#pragma hdrstop

#ifdef _MSC_VER
#include <malloc.h>
#include <borland.h>
#else
#include <alloc.h>
#endif

#include "model.h"




#ifdef LIB
  #define fbmalloc  farmalloc
  #define fbfree    farfree
  #define bmalloc   malloc
  #define bfree     free
#endif

/******************************************************************************
  FOSSIL SUPPORT FUNCTIONS
******************************************************************************/

extern int OutBufSize;

#define FOSINBUFSIZE    1024         /* MUST be a power of TWO! */

extern int OutBufSize;

static char *FosBuffer = NULL;
static int   FosInBytes;
static int   FosHeadPtr;
static int   FosTailPtr;

/* #define FOSSILTEST */
/* #define FOSSILTEST2 */

#ifdef FOSSILTEST
  enum {FOSSIL_IN = 0, FOSSIL_OUT, FOSSIL_DTRON, FOSSIL_DTROFF, FOSSIL_CDON, FOSSIL_CDOFF, FOSSIL_CLRIN};
  static DOSFILE FossilDump;

  void near LIBENTRY dump(char *Buf, int NumBytes, char InOut);
#endif

#ifdef FOSSILTEST2
  extern bool Sound;
#endif


typedef struct {
  int  StructSize;
  char FossilVersion;
  char DriverLevel;
  char far *ID;
  int  InBufSize;
  int  BufInBytes;
  int  OutBufSize;
  int  BufOutBytes;
  char ScreenW;
  char ScreenH;
  char BaudRateMask;
} fossilstruct;

static int Port;
static fossilstruct Fossil;

int LIBENTRY FOSSIL_ringdetect(void) {
  return(FALSE);
}

int LIBENTRY FOSSIL_ctsokay(void) {
  return(TRUE);
}

int LIBENTRY FOSSIL_online(void) {
  #ifdef FOSSILTEST
    char SaveCD = CDokay;
  #endif

  _AH = 3;                              //  asm mov ah,03
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h
  _AX &= 0x80;                          //  asm and ax,80h
  CDokay = _AL;                         //  asm mov CDokay,al

  #ifdef FOSSILTEST
    if (CDokay != SaveCD)
      dump(NULL,0,(CDokay ? FOSSIL_CDON : FOSSIL_CDOFF));

    asm xor ax,ax
    asm mov al,CDokay
  #endif

  #ifdef _MSC_VER
    return;        /* ignore error, the value is in AX */
  #else
    return(_AX);
  #endif
}

int LIBENTRY FOSSIL_cdstillup(void) {
  int X;

  if (VerifyCDLoss) {
    for (X = 0; X < 15; X++) {
      if (online())
        return(TRUE);  /* cd is up */
      settimer(4,3);
      while (! timerexpired(4))
        giveup();
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
void LIBENTRY FOSSIL_reportcom(long Speed) {
}
#pragma warn +par
#endif

int LIBENTRY FOSSIL_bauddivisor(long PortSpeed) {
  int CPS;

  CPS = (int) (PortSpeed/10);
  switch (CPS) {
    case    30:  return(0x02);
    case   120:  return(0x04);
    case   240:  return(0x05);
    case   480:  return(0x06);
    case   960:  return(0x07);
    case  1920:  return(0x00);
    case  3840:  return(0x01);
    default   :  /* Invalid Port Speed - set it to 38400 */
                 return(0x01);
  }
}

void LIBENTRY FOSSIL_setport(int BaudDivisor,int DataBits) {
  bool N81;

  N81 = (DataBits == 8);

  BaudDivisor <<= 5;                           /* rotate divisor into place */
  BaudDivisor += (N81 ? 3 : 2);                /* add data bits             */
  BaudDivisor += 0;                            /* hardcode 1 stop bit       */
  BaudDivisor += (N81 ? 0 : 0x18);             /* set parity bits           */
  _AX = BaudDivisor;                    //  asm mov Ax,BaudDivisor
  _AH = 0;                              //  asm mov Ah,00
  _DX = Port;                           //  asm mov dx, Port
  geninterrupt (0x14);                  //  asm int 14h
}   /*lint !e550 */


static int near LIBENTRY FOSSIL_bytesinbuffer(void) {
  #ifdef _MSC_VER
    int CXreg;
    CXreg = sizeof(Fossil);
    asm mov cx,CXreg
  #else
    _CX = sizeof(Fossil);
  #endif

  _ES = FP_SEG ((void far *) &Fossil);  //  asm mov ax, seg Fossil
                                        //  asm mov es, ax
  _DI = FP_OFF ((void far *) &Fossil);  //  asm mov di, offset Fossil
  _DX = Port;                           //  asm mov dx, Port
  _AH = 0x1b;                           //  asm mov ah, 1Bh
  geninterrupt (0x14);                  //  asm int 14h

//#ifdef FOSSILTEST
//{
//  char Str[80];
//  sprintf(Str,"%4d/%4d",Fossil.InBufSize - Fossil.BufInBytes,Fossil.InBufSize);
//  fastprint(0,24,Str,0x1F);
//}
//#endif

  return(Fossil.InBufSize - Fossil.BufInBytes);
}


#ifdef FOSSILTEST
static void near LIBENTRY dump(char *Buf, int NumBytes, char InOut) {
  long Ticks;

  Ticks = (getticks() & 0xFFFFFFL) + ((long) InOut << 24);

  if (Buf == NULL)
    dosfwrite(&Ticks,4,&FossilDump);
  else {
    for (; NumBytes > 0; NumBytes--, Buf++) {
      dosfwrite(&Ticks,4,&FossilDump);
      dosfwrite(Buf,1,&FossilDump);
    }
  }
}
#endif


static int near LIBENTRY FOSSIL_readin(char *Buf, int BufLen) {
  int  NumBytes;

  NumBytes = FOSSIL_bytesinbuffer();
  if (NumBytes > BufLen)
    NumBytes = BufLen;

  if (NumBytes != 0) {
    if (NumBytes == 1) {
      _AH = 2;                          //  asm mov ah,02
      _DX = Port;                       //  asm mov dx,Port
      geninterrupt (0x14);              //  asm int 14h
      #ifdef _MSC_VER
        {
          char ALreg;
          asm mov ALreg,al
          Buf[0] = ALreg;
        }
      #else
        Buf[0] = _AL;
      #endif
    } else {
      _ES = FP_SEG ((void far *) Buf);
      _DI = FP_OFF ((void far *) Buf);
      _AH = 0x18;                       //  asm mov ah,18h
      _CX = NumBytes;                   //  asm mov cx,NumBytes
      _DX = Port;                       //  asm mov dx,Port
      geninterrupt (0x14);              //  asm int 14h
    }

    #ifdef FOSSILTEST
      dump(Buf,NumBytes,FOSSIL_IN);
    #endif

    #ifdef FOSSILTEST2
      if (Sound != 0)
        mysound(2000,Sound);
    #endif
  }

  return(NumBytes);
}


static void near LIBENTRY FOSSIL_readport(void) {
  int    FreeBytes;
  int    BytesRead;
  int    BytesToBufEnd;
  char * pTempBuf;
  char   TempBuf[FOSINBUFSIZE];

  #ifdef DEBUG
    if (FosBuffer == NULL) {
      writedebugrecord("call to FOSSIL_readport() while closed");
      return;
    }
  #endif

  /* found out if there is any room in our local buffer */
  if ((FreeBytes = FOSINBUFSIZE - FosInBytes) > 0) {

    /* point to temporary buffer for moving data between read and OSBuffer */
    pTempBuf = TempBuf;

    /* there is room, now see if FOSSIL has anything for us */
    if ((BytesRead = FOSSIL_readin(pTempBuf,FreeBytes)) > 0) {

      /* if there are no bytes in the buffer, reset the head pointer to 0 */
      /* because doing so can avoid the extra code used for wrapping      */
      if (FosInBytes == 0)
        FosHeadPtr = FosTailPtr = 0;

      /* there is, now find out if copying it will wrap around the buffer */
      BytesToBufEnd = FOSINBUFSIZE - FosHeadPtr;
      if (BytesRead > BytesToBufEnd) {
        /* it needs to wrap, so copy only to the end of the buffer first */
        memcpy(FosBuffer+FosHeadPtr,pTempBuf,BytesToBufEnd);
        /* then reset the FosHeadPtr back to the beginning */
        FosHeadPtr = 0;
        /* and reduce the number of bytes to copy */
        BytesRead -= BytesToBufEnd;
        /* move pTempBuf forward so that it can be used down below */
        pTempBuf += BytesToBufEnd;
        /* Count the number of bytes copied so far */
        FosInBytes += BytesToBufEnd;
      }

      /* copy bytes out of temporary buffer into FosBuffer */
      memcpy(FosBuffer+FosHeadPtr,pTempBuf,BytesRead);

      FosHeadPtr += BytesRead;         /* Move the head pointer forward    */
      FosHeadPtr &= (FOSINBUFSIZE-1);  /* wrap the pointer if necessary    */
      FosInBytes += BytesRead;         /* count the number of bytes copied */
    }
  }
}


int LIBENTRY FOSSIL_inbytes(void) {
  if (FosInBytes != 0)
    return(FosInBytes);

  FOSSIL_readport();
  return(FosInBytes);
}


int LIBENTRY FOSSIL_outbytes(void) {
  #ifdef _MSC_VER
    int CXreg;
    CXreg = sizeof(Fossil);
    asm mov cx,CXreg
  #else
    _CX = sizeof(Fossil);
  #endif

  _ES = FP_SEG ((void far *) &Fossil);  //  asm mov ax, seg Fossil
                                        //  asm mov es, ax
  _DI = FP_OFF ((void far *) &Fossil);  //  asm mov di, offset Fossil
  _DX = Port;                           //  asm mov dx, Port
  _AH = 0x1b;                           //  asm mov ah, 1Bh
  geninterrupt (0x14);                  //  asm int 14h
  return(Fossil.OutBufSize - Fossil.BufOutBytes);
}

int LIBENTRY FOSSIL_framingerrors(void) {
  return(0);
}

int LIBENTRY FOSSIL_overrunerrors(void) {
  return(0);
}

int LIBENTRY FOSSIL_parityerrors(void) {
  return(0);
}

void LIBENTRY FOSSIL_turnoffdtr(void) {
  _AX = 0x600;                          //  asm mov ax,0600h
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h

  #ifdef FOSSILTEST
    dump(NULL,0,FOSSIL_DTROFF);
  #endif
}

void LIBENTRY FOSSIL_turnondtr(void) {
  _AX = 0x601;                          //  asm mov ax,0601h
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h

  #ifdef FOSSILTEST
    dump(NULL,0,FOSSIL_DTRON);
  #endif
}

void LIBENTRY FOSSIL_turnoffrts(void) {
}

void LIBENTRY FOSSIL_turnonrts(void) {
}

void LIBENTRY FOSSIL_turnonxmit(void) {
}

void LIBENTRY FOSSIL_clearoutbuf(void) {
  _AH = 9;                              //  asm mov ah,09h
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h
}

void LIBENTRY FOSSIL_clearinbuf(void) {
//#ifdef FOSSILTEST
//  fastprint(0,23,"in buffer cleared",0x2F);
//#endif
  _AH = 0x0a;                           //  asm mov ah,0Ah
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h

  FosInBytes = 0;
  FosHeadPtr = 0;
  FosTailPtr = 0;

  #ifdef FOSSILTEST
    dump(NULL,0,FOSSIL_CLRIN);
  #endif
}

void LIBENTRY FOSSIL_commgo(void) {
  _AX = 0x1000;                         //  asm mov Ax,1000h
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h
}

void LIBENTRY FOSSIL_commstop(void) {
//#ifdef FOSSILTEST
//  fastprint(0,23,"comm stopped",0x2F);
//#endif
  _AX = 0x1002;                         //  asm mov Ax,1002h
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h
}

void LIBENTRY FOSSIL_commpause(void) {
}

int LIBENTRY FOSSIL_checkcomm(void) {
  #ifdef _MSC_VER
    int AXreg;
  #endif
  int NumBytes;

  NumBytes = InBytes;
  if (NumBytes == 0)
    return(0);

  _AH = 0x0c;                           //  asm mov ah,0Ch
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h

  #ifdef _MSC_VER
    asm mov AXreg,ax
    switch(AXreg) {
      case 11:
      case 19:
      case 24: return(AXreg);
    }
  #else
    switch(_AL) {
      case 11:
      case 19:
      case 24: return(_AX);
    }
  #endif
  return(0);
}



int LIBENTRY FOSSIL_comminkey(void) {
  char Byte;

  #ifdef DEBUG
    if (FosBuffer == NULL) {
      writedebugrecord("call to FOSSIL_comminkey() while closed");
      return(-1);
    }
  #endif

  if (InBytes == 0)
    return(-1);

  FosInBytes--;
  Byte = (char) FosBuffer[FosTailPtr++];
  FosTailPtr &= (FOSINBUFSIZE-1);         /* wrap the pointer if necessary */
  return(Byte);
}


int LIBENTRY FOSSIL_cgetstr(char *pStr, int StrLen) {
  int  NumBytesFound;
  int  NumBytesToCopy;
  int  NumBytesToBufEnd;

  #ifdef DEBUG
    if (FosBuffer == NULL) {
      writedebugrecord("call to FOSSIL_cgetstr() while closed");
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

    NumBytesToBufEnd = FOSINBUFSIZE - FosTailPtr;
    if (NumBytesToCopy > NumBytesToBufEnd) {
      /* copy from tail pointer to the end of the buffer */
      memcpy(pStr,FosBuffer+FosTailPtr,NumBytesToBufEnd);

      /* move the buffer pointer forward so that we can use it below */
      pStr += NumBytesToBufEnd;

      /* now reset the tail pointer, and reduce the number of bytes to copy */
      FosTailPtr = 0;
      NumBytesToCopy -= NumBytesToBufEnd;
    }

    /* copy the bytes from the OS buffer into the user buffer */
    memcpy(pStr,FosBuffer+FosTailPtr,NumBytesToCopy);

    /* NULL-terminate the string */
    pStr[NumBytesToCopy] = 0;

    /* advance the tail pointer by the number of bytes copied */
    FosTailPtr += NumBytesToCopy;

    /* wrap the tail pointer around if necessary */
    FosTailPtr &= (FOSINBUFSIZE-1);

    /* reduce the number of bytes recorded in the buffer */
    FosInBytes -= NumBytesFound;
  }

  return(NumBytesFound);
}


int LIBENTRY FOSSIL_cgetbuf(char *Buf, int BufLen) {
  int  NumBytesFound;
  int  NumBytesToCopy;
  int  NumBytesToBufEnd;

  #ifdef DEBUG
    if (FosBuffer == NULL) {
      writedebugrecord("call to FOSSIL_cgetbuf() while closed");
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

    NumBytesToBufEnd = FOSINBUFSIZE - FosTailPtr;
    if (NumBytesToCopy > NumBytesToBufEnd) {
      /* copy from tail pointer to the end of the buffer */
      memcpy(Buf,FosBuffer+FosTailPtr,NumBytesToBufEnd);

      /* move the buffer pointer forward so that we can use it below */
      Buf += NumBytesToBufEnd;

      /* now reset the tail pointer, and reduce the number of bytes to copy */
      FosTailPtr = 0;
      NumBytesToCopy -= NumBytesToBufEnd;
    }

    /* copy the bytes from the OS buffer into the user buffer */
    memcpy(Buf,FosBuffer+FosTailPtr,NumBytesToCopy);

    /* advance the tail pointer by the number of bytes copied */
    FosTailPtr += NumBytesToCopy;

    /* wrap the tail pointer around if necessary */
    FosTailPtr &= (FOSINBUFSIZE-1);

    /* reduce the number of bytes recorded in the buffer */
    FosInBytes -= NumBytesFound;
  }

  return(NumBytesFound);
}



void LIBENTRY FOSSIL_csendbyte(unsigned char ByteToSend) {
  _AH = 1;                              //  asm mov ah,01h
  _AL = ByteToSend;                     //  asm mov al,ByteToSend
  _DX = Port;                           //  asm mov dx,Port
  geninterrupt (0x14);                  //  asm int 14h

  #ifdef FOSSILTEST
    dump(&ByteToSend,1,FOSSIL_OUT);
  #endif

  #ifdef FOSSILTEST2
    if (Sound != 0)
      mysound(200,Sound);
  #endif
}


void LIBENTRY FOSSIL_csendstr(char *pStr, int StrLen) {
  _CX = StrLen;                         //  asm mov cx,StrLen
  _ES = FP_SEG ((void far *) pStr);
  _DI = FP_OFF ((void far *) pStr);
  _DX = Port;                           //  asm mov dx,Port
  _AH = 0x19;                           //  asm mov ah,19h
  geninterrupt (0x14);                  //  asm int 14h

  #ifdef FOSSILTEST
    dump(pStr,StrLen,FOSSIL_OUT);
  #endif

  #ifdef FOSSILTEST2
    if (Sound != 0)
      mysound(200,Sound);
  #endif
}


int static near LIBENTRY FOSSIL_initializedriver(int PortNum) {
  _AH = 4;                              //  asm mov Ah,04h
  _DX = PortNum;                        //  asm mov Dx,PortNum
  _BX = 0;                              //  asm xor Bx,Bx
  geninterrupt (0x14);                  //  asm int 14h
  _AX -= 0x1954;                        //  asm sub Ax,1954h
  #ifdef _MSC_VER
    return;  /* return value is in AX */
  #else
    return(_AX);
  #endif
}


int static near LIBENTRY FOSSIL_getdriverinfo(void) {
  #ifdef _MSC_VER
    int reg;
    reg = sizeof(Fossil);
    asm mov cx,reg
  #else
    _CX = sizeof(Fossil);
  #endif

  _ES = FP_SEG ((void far *) &Fossil);  //  asm mov ax, seg Fossil
                                        //  asm mov es, ax
  _DI = FP_OFF ((void far *) &Fossil);  //  asm mov di, offset Fossil
  _DX = Port;                           //  asm mov dx, Port
  _AH = 0x1b;                           //  asm mov ah, 1Bh
  geninterrupt (0x14);                  //  asm int 14h

/*
  #ifdef _MSC_VER
    asm mov reg,ax
    if (reg != sizeof(Fossil))
      return(-1);
  #else
    if (_AX != sizeof(Fossil))
      return(-1);
  #endif
*/

  OutBufSize = Fossil.OutBufSize;
  return(0);
}

void LIBENTRY FOSSIL_disconnectmodem(void) {
  if (FosBuffer != NULL) {
    bfree(FosBuffer);
    FosBuffer = NULL;
  }

  #ifdef FOSSILTEST
    dosfclose(&FossilDump);
  #endif

  ModemOpened = FALSE;
  FosInBytes = FosHeadPtr = FosTailPtr = 0;
}


void LIBENTRY FOSSIL_openmodem(showtype Show) {
  int  Count;
  char Str[80];

  if (Asy.ComPortNumber == 0 || ! ModemFixupsDone)
    return;

  if ((FosBuffer = (char *) bmalloc(FOSINBUFSIZE)) == NULL) {
    sprintf(Str,"insufficient memory for comm buffers: %u : %ld",FOSINBUFSIZE,coreleft());
    errorexittodos(Str);
    return;
  }

  FosInBytes = FosHeadPtr = FosTailPtr = 0;
  #ifdef DEBUG
    memset(FosBuffer,0,FOSINBUFSIZE);
  #endif

  Port = Asy.ComPortNumber - 1;

  if (FOSSIL_initializedriver(Port) != 0) {
    errorexittodos("Invalid comm port - FOSSIL driver not found");
    return;
  }

  if (FOSSIL_getdriverinfo() != 0) {
    errorexittodos("Error obtaining FOSSIL information");
    return;
  }

  if (! PcbData.DisableCTS) {
    _AX = 0x0f02;                 /* enable CTS/RTS flow control */
    _DX = Port;
    geninterrupt (0x14);
  }

  _AX = 0x1000;               /* disable Ctrl-C/K check in FOSSIL driver */
  _DX = Port;
  geninterrupt (0x14);

  setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);

/*
  if (OutBufSize < 2048) {
    errorexittodos("Output buffer must be configured for at least 2048 bytes");
    return;
  }
*/

  OutBufSize -= 128;  /* provide a little 'slack' in buffer */

  #ifdef FOSSILTEST
    if (dosfopen("FOSSIL.DMP",OPEN_WRIT|OPEN_DENYRDWR,&FossilDump) == -1)
      errorexittodos("Unable to open/create FOSSIL.DMP");
    dossetbuf(&FossilDump,4096);
  #endif

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

    if (initializemodem(Show))
      break;
  }
  tickdelay((PcbData.ModemDelay*HALFSECOND)+QUARTERSECOND);
}


void LIBENTRY FOSSIL_reopenport(void) {
  FOSSIL_openmodem(HIDE);
}

void LIBENTRY FOSSIL_dofixups(void) {
  ringdetect      = FOSSIL_ringdetect;
  ctsokay         = FOSSIL_ctsokay;
  online          = FOSSIL_online;
  cdstillup       = FOSSIL_cdstillup;
  bauddivisor     = FOSSIL_bauddivisor;
  setport         = FOSSIL_setport;
  inbytes         = FOSSIL_inbytes;
  outbytes        = FOSSIL_outbytes;
  framingerrors   = FOSSIL_framingerrors;
  overrunerrors   = FOSSIL_overrunerrors;
  parityerrors    = FOSSIL_parityerrors;
  turnoffdtr      = FOSSIL_turnoffdtr;
  turnondtr       = FOSSIL_turnondtr;
  turnoffrts      = FOSSIL_turnoffrts;
  turnonrts       = FOSSIL_turnonrts;
  turnonxmit      = FOSSIL_turnonxmit;
  clearoutbuf     = FOSSIL_clearoutbuf;
  clearinbuf      = FOSSIL_clearinbuf;
  commgo          = FOSSIL_commgo;
  commstop        = FOSSIL_commstop;
  commpause       = FOSSIL_commpause;
  checkcomm       = FOSSIL_checkcomm;
  comminkey       = FOSSIL_comminkey;
  cgetstr         = FOSSIL_cgetstr;
  cgetbuf         = FOSSIL_cgetbuf;
  csendbyte       = FOSSIL_csendbyte;
  csendstr        = FOSSIL_csendstr;
  disconnectmodem = FOSSIL_disconnectmodem;
  reopenport      = FOSSIL_reopenport;

#ifndef LIB
  reportcom       = FOSSIL_reportcom;
#endif
}
#endif
