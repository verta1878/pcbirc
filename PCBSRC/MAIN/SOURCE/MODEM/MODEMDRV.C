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


#if defined(COMM) && defined(MULTIPORT) && defined(COMMDRV)

#include "project.h"
#pragma hdrstop

#ifdef _MSC_VER
  #include <borland.h>
#else
//  #pragma inline
#endif


/******************************************************************************
  COMM-DRV SUPPORT FUNCTIONS
******************************************************************************/

/*  NOTES:  There is an OLDARNET conditional compilation setting that can     */
/*          be used.  It is believed that is setting is not generally needed. */
/*          The following is some history on the subject:                     */
/*                                                                            */
/*  The Digiboard COM/Xi card apparently does not interrupt COMM-DRV often    */
/*  enough to keep the pcb.opcb->inbuf_count and pcb.opcb->outbuf_count       */
/*  variables up to date.  Rather than ASSUME that when the counter is zero,  */
/*  the source code had to be modified to check to see if a DIGCXI card was   */
/*  in use, and if so, make an extra call to get the incoming bytes.  This    */
/*  would refresh the counter at the same time.                               */
/*                                                                            */
/*  The ARNET card, on the other hand, has not required this extra call.  And */
/*  since a function call is far more expensive than a simple memory          */
/*  comparison, it was chosen to perform the function call ONLY for Digiboard */
/*  systems.                                                                  */
/*                                                                            */
/*  On 2/9/93, however, Gary Hedberg called in with a problem on his 16-port  */
/*  ARNET SmartPort.  This was NOT a SmartPort Plus, just a regular           */
/*  SmartPort.  The card was old.  And it would not see the incoming data     */
/*  without modifying the source code to work the same way that the Digiboard */
/*  worked.                                                                   */
/*                                                                            */
/*  So, by defining OLDARNET and recompiling this module, that method can     */
/*  be used even by ARNET cards (or any other cards for that matter).         */

#define OLDARNET 1         /* also found a digiboard worked better */


#define  COMMDRV_DRIVER
#include <comm.h>

static struct port_param pcb;
static int Port;
static unsigned short savecardtype = 0;
static bool COMMDRV_SuccessfulSetup = FALSE;

extern int OutBufSize;

#ifdef COMMDRVDEBUG
DOSFILE CommDrvFile;
typedef enum {COMMDRVNONE, COMMDRVIN, COMMDRVOUT} commdrvdir;

void LIBENTRY writecommdrvdebug(void *Buf, int Len, commdrvdir Direction) {
  static commdrvdir LastDirection;

  if (Direction != LastDirection) {
    dosfwrite("\r\n",2,&CommDrvFile);
    dosfwrite(Direction == COMMDRVIN ? "IN :" : "OUT:",4,&CommDrvFile);
    LastDirection = Direction;
  }

  dosfwrite(Buf,Len,&CommDrvFile);
}
#endif



int LIBENTRY COMMDRV_ringdetect(void) {
#ifdef OLDARNET
  unsigned char Stat[10];
  ser_rs232_getpacket(Port,0,Stat);   /*lint !e534 force it to refresh buffers by reading the card */
#endif
  return(pcb.opcb->msr_reg & 0x04);
}

int LIBENTRY COMMDRV_ctsokay(void) {
#ifdef OLDARNET
  unsigned char Stat[10];
  ser_rs232_getpacket(Port,0,Stat);   /*lint !e534 force it to refresh buffers by reading the card */
#endif
  return(pcb.opcb->msr_reg & 0x10);
}

int LIBENTRY COMMDRV_online(void) {
#ifdef OLDARNET
  unsigned char Stat[10];
  ser_rs232_getpacket(Port,0,Stat);   /*lint !e534 force it to refresh buffers by reading the card */
#endif
  return(CDokay = (pcb.opcb->msr_reg & 0x80));
}

int LIBENTRY COMMDRV_cdstillup(void) {
  int X;
#ifdef OLDARNET
  unsigned char Stat[10];
#endif

  if (VerifyCDLoss) {
    for (X = 0; X < 15; X++) {
#ifdef OLDARNET
      ser_rs232_getpacket(Port,0,Stat);   /*lint !e534 force it to refresh buffers by reading the card */
#endif
      if (pcb.opcb->msr_reg & 0x80)
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
  ser_rs232_flush(Port,2);   /*lint !e534 */
  return(FALSE);
}

#ifndef LIB
#pragma warn -par
void LIBENTRY COMMDRV_reportcom(long Speed) {
}
#pragma warn +par
#endif

int LIBENTRY COMMDRV_bauddivisor(long PortSpeed) {
  int CPS;

  CPS = (int) (PortSpeed/10);
  switch (CPS) {
    case    30:  return(BAUD300);
    case   120:  return(BAUD1200);
    case   240:  return(BAUD2400);
    case   480:  return(BAUD4800);
    case   960:  return(BAUD9600);
    case  1920:  return(BAUD19200);
    case  3840:  return(BAUD38400);
    case  5760:  return(BAUD57600);
    case 11520:  return(BAUD115200);
    default   :  /* Invalid Port Speed - set it to 1200 */
                 return(BAUD1200);
  }
}


void LIBENTRY COMMDRV_setup(void) {
  int  RetVal;
  int  X;
  char Str[80];

  if ((RetVal = ser_rs232_setup(Port,&pcb)) != RS232ERR_NONE) {
    /* was the port just BUSY? If so, let's delay a moment */
    /* and then try it again before calling it an error    */
    for (X = 0; X < 20; X++) {
      tickdelay(HALFSECOND);
      RetVal = ser_rs232_setup(Port,&pcb);
      if (RetVal == RS232ERR_NONE)
        break;
      if (RetVal != RS232ERR_BUSY) {
        sprintf(Str,"Error initializing port: (%d,%d,%d,%d)",RetVal,Port,pcb.cardtype,pcb.error);
        errorexittodos(Str);
        return;
      }
    }
  }

  COMMDRV_SuccessfulSetup = TRUE;
  savecardtype = pcb.cardtype;
}


void LIBENTRY COMMDRV_setport(int BaudDivisor,int DataBits) {
  pcb.baud  = BaudDivisor;
  if (DataBits == 8) {
    pcb.lngth  = LENGTH_8;
    pcb.parity = PARITY_NONE;
  } else {
    pcb.lngth  = LENGTH_7;
    pcb.parity = PARITY_EVEN;
  }
  COMMDRV_setup();  /* put pcb changes into effect */
}


int LIBENTRY COMMDRV_inbytes(void) {
#ifdef OLDARNET
  ser_rs232_getpacket(Port,32767,NULL);     /*lint !e534 */
#else
  if (pcb.cardtype == CARD_DIGCXI)
    ser_rs232_getpacket(Port,32767,NULL);   /*lint !e534 */
#endif
  return(pcb.opcb->inbuf_count);
}

int LIBENTRY COMMDRV_outbytes(void) {
  return(pcb.opcb->outbuf_count);
}

int LIBENTRY COMMDRV_framingerrors(void) {
  return((int) pcb.auxpcb->aux_frmint);
}

int LIBENTRY COMMDRV_overrunerrors(void) {
  return((int) pcb.auxpcb->aux_ovrint);
}

int LIBENTRY COMMDRV_parityerrors(void) {
  return((int) pcb.auxpcb->aux_parint);
}

void LIBENTRY COMMDRV_turnoffdtr(void) {
  ser_rs232_dtr_off(Port);   /*lint !e534 */
}

void LIBENTRY COMMDRV_turnondtr(void) {
  ser_rs232_dtr_on(Port);    /*lint !e534 */
}

void LIBENTRY COMMDRV_turnoffrts(void) {
  ser_rs232_rts_off(Port);   /*lint !e534 */
}

void LIBENTRY COMMDRV_turnonrts(void) {
  ser_rs232_rts_on(Port);    /*lint !e534 */
}

void LIBENTRY COMMDRV_turnonxmit(void) {
  ser_rs232_putpacket(Port,0,NULL);    /*lint !e534 */
}

void LIBENTRY COMMDRV_clearoutbuf(void) {
  ser_rs232_flush(Port,1);   /*lint !e534 */
}

void LIBENTRY COMMDRV_clearinbuf(void) {
  ser_rs232_flush(Port,0);   /*lint !e534 */
}

void LIBENTRY COMMDRV_commgo(void) {
/*pcb.opcb->flag &= ~XMTOFF_STATE;*/
  asm mov Ax,1000h
  asm mov Dx,Port
  asm int 14h
}

void LIBENTRY COMMDRV_commstop(void) {
/*pcb.opcb->flag |= XMTOFF_STATE;*/
  asm mov Ax,1002h
  asm mov Dx,Port
  asm int 14h
}

void LIBENTRY COMMDRV_commpause(void) {
/*pcb.opcb->flag |= XMTOFF_STATE;*/
}

int LIBENTRY COMMDRV_checkcomm(void) {
  unsigned char Buf[128];
  int  NumBytes;

  NumBytes = InBytes;

  if (NumBytes == 0)
    return(0);
  if (NumBytes > 127)
    NumBytes = 127;

  ser_rs232_viewpacket(Port,NumBytes,Buf);  /*lint !e534 */
  if (memchr(Buf,19,NumBytes) != NULL)
    return(19);
  if (memchr(Buf,24,NumBytes) != NULL)
    return(24);
  if (memchr(Buf,11,NumBytes) != NULL)
    return(11);
  return(0);
}

int LIBENTRY COMMDRV_comminkey(void) {
  unsigned char Byte[2];

#ifndef OLDARNET
  if (pcb.opcb->inbuf_count == 0) {
    if (pcb.cardtype != CARD_DIGCXI)
      return(-1);
  }
#endif

  Byte[1] = 0;
  if (ser_rs232_getpacket(Port,1,Byte) == RS232ERR_NONE) {

    #ifdef COMMDRVDEBUG
      writecommdrvdebug(&Byte,1,COMMDRVIN);
    #endif

    return(Byte[0]);
  }
  return(-1);
}

int LIBENTRY COMMDRV_cgetstr(char *Str, int StrLen) {
  int  NumBytes;

#ifdef OLDARNET
  if ((NumBytes = inbytes()) == 0)
    return(0);
#else
  NumBytes = pcb.opcb->inbuf_count;
  if (NumBytes == 0) {
    if (pcb.cardtype == CARD_DIGCXI) {
      if ((NumBytes = inbytes()) == 0)
        return(0);
    } else
      return(0);
  }
#endif

  StrLen--;  /* make room for the NULL terminator */
  if (NumBytes > StrLen)
    NumBytes = StrLen;

  if (NumBytes == 1) {
    ser_rs232_getbyte(Port,(unsigned char *) Str);  /*lint !e534 */
    Str[1] = 0;
  } else {
    ser_rs232_getpacket(Port,NumBytes,(unsigned char *) Str);  /*lint !e534 */
    Str[NumBytes] = 0;
  }

  #ifdef COMMDRVDEBUG
    writecommdrvdebug(Str,NumBytes,COMMDRVIN);
  #endif

  return(NumBytes);
}

int LIBENTRY COMMDRV_cgetbuf(char *Buf, int BufLen) {
  int  NumBytes;

#ifdef OLDARNET
  if ((NumBytes = inbytes()) == 0)
    return(0);
#else
  NumBytes = pcb.opcb->inbuf_count;
  if (NumBytes == 0) {
    if (pcb.cardtype == CARD_DIGCXI) {
      if ((NumBytes = inbytes()) == 0)
        return(0);
    } else
      return(0);
  }
#endif

  if (NumBytes > BufLen)
    NumBytes = BufLen;
  if (NumBytes == 1) {
    ser_rs232_getbyte(Port,(unsigned char *) Buf);        /*lint !e534 */
  } else {
    ser_rs232_getpacket(Port,NumBytes,(unsigned char *) Buf);  /*lint !e534 */
  }

  #ifdef COMMDRVDEBUG
    writecommdrvdebug(Buf,NumBytes,COMMDRVIN);
  #endif

  return(NumBytes);
}

void LIBENTRY COMMDRV_csendstr(char *Str, int StrLen) {
  ser_rs232_putpacket(Port,StrLen,(unsigned char *) Str);          /*lint !e534 */
  #ifdef COMMDRVDEBUG
    writecommdrvdebug(Str,StrLen,COMMDRVOUT);
  #endif
}

void LIBENTRY COMMDRV_csendbyte(unsigned char Byte) {
  ser_rs232_putbyte(Port,&Byte);       /*lint !e534 */
  #ifdef COMMDRVDEBUG
    writecommdrvdebug(&Byte,1,COMMDRVOUT);
  #endif
}

void LIBENTRY COMMDRV_disconnectmodem(void) {
  #ifdef COMMDRVDEBUG
    dosfclose(&CommDrvFile);
  #endif
}

void LIBENTRY COMMDRV_reopenport(void) {
  ModemOpened = TRUE;
}


void LIBENTRY COMMDRV_openmodem(showtype Show) {
  int  RetVal;
  int  Count;
  char Str[80];

  if (Asy.ComPortNumber == 0 || ! ModemFixupsDone)
    return;

  Port = Asy.ComPortNumber - 1;

  if (! COMMDRV_SuccessfulSetup) {
    if ((RetVal = ser_rs232_init()) != RS232ERR_NONE) {
      sprintf(Str,"(%d) %s",RetVal,"Invalid comm port - COMMDRV access not found");
      errorexittodos(Str);
      return;
    }
  }

  ser_rs232_getport(Port,&pcb);   /*lint !e534 */

  #ifndef COMMDRV_MONITOR
    if (COMMDRV_SuccessfulSetup) {
      pcb.cardtype = savecardtype;
      pcb.opcb->cardtype = savecardtype;
    }
  #endif

  if (! PcbData.DisableCTS)
    pcb.protocol = PROT_RTSRTS;

  pcb.block[1] = 0;
  pcb.baud     = bauddivisor(Asy.ModemSpeed);
  pcb.lngth    = (Asy.DataBits == 8 ? LENGTH_8 : LENGTH_7);
  pcb.parity   = (Asy.DataBits == 8 ? PARITY_NONE : PARITY_EVEN);
  OutBufSize   = pcb.outbuf_len;

/*
  if (OutBufSize < 2048) {
    errorexittodos("Output buffer must be configured for at least 2048 bytes");
    return;
  }
*/

  OutBufSize -= 128;  /* provide a little 'slack' in buffer */

  COMMDRV_setup();  /* put pcb changes into effect */

  #ifdef COMMDRVDEBUG
    dosfopen("COMMDRV.TXT",OPEN_RDWR,&CommDrvFile);
    dossetbuf(&CommDrvFile,8192);
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


void LIBENTRY COMMDRV_dofixups(void) {
  ringdetect      = COMMDRV_ringdetect;
  ctsokay         = COMMDRV_ctsokay;
  online          = COMMDRV_online;
  cdstillup       = COMMDRV_cdstillup;
  bauddivisor     = COMMDRV_bauddivisor;
  setport         = COMMDRV_setport;
  inbytes         = COMMDRV_inbytes;
  outbytes        = COMMDRV_outbytes;
  framingerrors   = COMMDRV_framingerrors;
  overrunerrors   = COMMDRV_overrunerrors;
  parityerrors    = COMMDRV_parityerrors;
  turnoffdtr      = COMMDRV_turnoffdtr;
  turnondtr       = COMMDRV_turnondtr;
  turnoffrts      = COMMDRV_turnoffrts;
  turnonrts       = COMMDRV_turnonrts;
  turnonxmit      = COMMDRV_turnonxmit;
  clearoutbuf     = COMMDRV_clearoutbuf;
  clearinbuf      = COMMDRV_clearinbuf;
  commgo          = COMMDRV_commgo;
  commstop        = COMMDRV_commstop;
  commpause       = COMMDRV_commpause;
  checkcomm       = COMMDRV_checkcomm;
  comminkey       = COMMDRV_comminkey;
  cgetstr         = COMMDRV_cgetstr;
  cgetbuf         = COMMDRV_cgetbuf;
  csendbyte       = COMMDRV_csendbyte;
  csendstr        = COMMDRV_csendstr;
  disconnectmodem = COMMDRV_disconnectmodem;
  reopenport      = COMMDRV_reopenport;

#ifndef LIB
  reportcom       = COMMDRV_reportcom;
#endif
}

#endif  /* COMM && MULTIPORT && COMMDRV */
