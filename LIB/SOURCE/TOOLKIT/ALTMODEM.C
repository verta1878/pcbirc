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


#ifdef _MSC_VER
  #include <memory.h>
  #include <borland.h>
#else
  #include <mem.h>
#endif
#include <string.h>
#include "pcbtools.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif


statustype  Status;
pcbdattype  PcbData;
asytype     Asy;
disptype    Display;
controltype Control;
syshdrtype  UserSysHdr;
userrectype UserSys;


/* typedef enum {SHOW, HIDE } showtype; */


void LIBENTRY openmodem(showtype Show);
void LIBENTRY closemodem(bool TurnOffDTR);

static bool Error;
static void (LIBENTRY *show)(char *Str);

/*** vv  these functions are in here to 'stub' them out of the toolkit  vv ***/

#pragma warn -par
void LIBENTRY loguseroff(int Type) {
}
void LIBENTRY writelog(char *Str, padtype Pad) {
}
int LIBENTRY kbdinkey(void) {
  return(0);
}
void LIBENTRY insertbuffer(int Key) {
}
void LIBENTRY watchsystemfunctions(void) {
}
#pragma warn +par

/*** ^^  these functions are in here to 'stub' them out of the toolkit  ^^ ***/


void LIBENTRY tickdelay(int Ticks) {
  settimer(3,Ticks);
  while (gettimer(3) > 0) {
    turnonxmit();
    giveup();
  }
}


#pragma argsused
void LIBENTRY errorexittodos(char *Str) {
  /* show(Str); */
  Error = TRUE;
}

void LIBENTRY slowsendtomodem(char *Str) {
  for (; *Str != 0; Str++) {          /* step thru each byte in the string  */
    sendbyte(*Str);                   /* send the byte to the modem         */
    tickdelay(3);                     /* insert delay in between each byte  */
  }
}


static bool _NEAR_ LIBENTRY modemokay(char *Buf, int BufSize, char InByte) {
  int X;

   X = strlen(Buf);
   if (X > BufSize-2) {
     memmove(&Buf[0],&Buf[1],BufSize-1);
     X--;
   }

   Buf[X] = InByte;
   Buf[X+1] = 0;
   return((bool) (strstr(Buf,"OK\r") != NULL));
}


static int _NEAR_ LIBENTRY showmodemcommand(char *InBuf, int BufSize, modemverifytype Verify) {
  int  InByte;
  char Str[2];

  while ((InByte = comminkey()) != -1) {
    Str[0] = (char) InByte;
    Str[1] = 0;
    show(Str);
    if (Verify == WAITFOROKAY)
     if (modemokay(InBuf,BufSize,(char) InByte))
       return(0);
  }
  if ((InByte = kbdinkey()) == 27) {
    show("ESC Abort!");
    return(-1);
  }
  return(1);
}


int LIBENTRY modemcommand(char *Command, modemverifytype Verify) {
  char InBuf[128];
  int  Count;
  int  ModemStatus;

  memset(InBuf,0,sizeof(InBuf));

  for (Count = 0; Count < 4; Count++) {
    slowsendtomodem(Command);
    settimer(4,FIVESECONDS);
    while (gettimer(4) > 0) {
      ModemStatus = showmodemcommand(InBuf,sizeof(InBuf),Verify);
      if (ModemStatus != 1)
        return(ModemStatus);
    }
    if (Verify != WAITFOROKAY)
      break;
  }
  return(-1);
}


int LIBENTRY initmodem(char *ComPort, int Num, int Irq, int Addr, long Speed, void (LIBENTRY *pshow)(char *Str)) {
  show                = pshow;
  Asy.Online          = OFFLINE;
  Asy.DataBits        = 8;
  Asy.ComPortNumber   = (char) Num;
  Asy.ModemSpeed      = Speed;
  PcbData.IrqNum      = (char) Irq;
  PcbData.BaseAddress = (short) Addr;
  PcbData.DisableCTS  = (bool) (Speed <= 2400);
  PcbData.ShareIRQs   = FALSE;
  PcbData.ModemDelay  = 0;
  Error               = FALSE;

  strcpy(PcbData.ModemPort,ComPort);

  openmodem(HIDE);
  if (Error)
    return(-1);

  tickdelay(4);
  return(0);
}

#ifdef TEST
#include <stdio.h>

void LIBENTRY showoutput(char *Str) {
  printf(Str);
}

void main(void) {
  if (initmodem("COM2",2,3,0x2F8,38400L,showoutput) == -1)
    return;

  modemcommand("AT\r",WAITFOROKAY);
  closemodem(TRUE);
}
#endif
