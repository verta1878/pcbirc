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


#include <string.h>
#include <pcbtools.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

#define BUFSIZE 512

static char Buffer[BUFSIZE];
int SpoolBytes = 0;


void pascal zmFlushSpooler(void) {
  if (SpoolBytes == 0)
    return;

  if (Asy.LostCarrier || ! cdstillup()) {   // is CD is down, then get out now
    Asy.LostCarrier = TRUE;
    zmAbort = TRUE;
    if (PcbData.Packet)
      turnoffdtr();
    SpoolBytes = 0;
    return;
  }

  sendstr(Buffer,SpoolBytes);
  SpoolBytes = 0;
}


void pascal zmSpoolByte(char Byte) {
  if (1 >= BUFSIZE-SpoolBytes)
    zmFlushSpooler();

  Buffer[SpoolBytes++] = Byte;
}


void pascal zmSpoolStr(char *Str, int StrLen) {
  int NumToCopy;

  NumToCopy = StrLen;
  if (NumToCopy > BUFSIZE) {
    zmSpoolStr(Str,BUFSIZE);
    Str += BUFSIZE;
    NumToCopy -= StrLen;
  }

  if (NumToCopy >= BUFSIZE - SpoolBytes)
    zmFlushSpooler();

  memcpy(&Buffer[SpoolBytes],Str,NumToCopy);
  SpoolBytes += NumToCopy;
}
