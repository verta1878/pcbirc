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
#include <malloc.h>
#else
#include <alloc.h>
#endif

#include <string.h>
#include "pcbtools.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

/* Some modems can't handle data in command mode as fast as they can when */
/* in data mode.  This function will insert a small 3/18th second delay   */
/* in between each character sent to the modem.                           */

void LIBENTRY slowsendtomodem(char *Str) {
  for (; *Str != 0; Str++) {          /* step thru each byte in the string  */
    sendbyte(*Str);                   /* send the byte to the modem         */
    tickdelay(2);                     /* insert delay in between each byte  */
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


int LIBENTRY showmodem(char *InBuf, int BufSize, modemverifytype Verify) {
  int  InByte;
  char Str[2];

  while ((InByte = comminkey()) != -1) {
    Str[0] = (char) InByte;
    Str[1] = 0;
    ansi(Str);
    if (Verify == WAITFOROKAY)
     if (modemokay(InBuf,BufSize,(char) InByte))
       return(0);
  }
  if ((InByte = kbdinkey()) == 27) {
    ansi("\r\nESC Abort!\r\n");
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
      ModemStatus = showmodem(InBuf,sizeof(InBuf),Verify);
      if (ModemStatus != 1)
        return(ModemStatus);
    }
    if (Verify != WAITFOROKAY)
      break;
  }
  return(-1);
}
