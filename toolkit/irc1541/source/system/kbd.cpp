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


#ifdef __OS2__
  #define INCL_DOSPROCESS
  #include <os2.h>

  #include <threads.h>
#endif

#ifdef __BORLANDC__
  #include <alloc.h>
#else
  #include <malloc.h>
#endif

#include <mem.h>
#include <process.h>
#include "kbd.hpp"
#include "system.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

const int SemMax  = 100;  // maximum number of semaphores to track
const int BufSize = 256;  // use 256 so that when it reaches 255 it wraps to 0

static uint           KbdBuf[BufSize];
static ubyte          KbdPutPtr;  // needs to be char so it will wrap
static ubyte          KbdGetPtr;  // needs to be char so it will wrap
static ubyte volatile KbdCount;   // count of bytes in buffer

static int            SemCount;
static CSemaphore   **SemList;
static int            ThreadId;

#pragma argsused
static void THREADFUNC kbdhandler(void *Ignore) {
  int  X;
  uint Key;

  // increase our priority so that nothing can interrupt us while we process
  // the keystrokes (we'll finish quickly and then block again)

  #ifdef __OS2__
    DosSetPriority(PRTYS_THREAD,PRTYC_TIMECRITICAL,PRTYD_MINIMUM,0);
  #endif

  while (1) {
    Key = bgetkey(0);

    // don't add the key unless it's non-zero
    if (Key != 0) {
      // if the buffer is already full, then see if giving up some time will
      // give the other threads a chance to catch up
      if (KbdCount == 255) {
        #ifdef __OS2__
          DosSleep(10);
        #endif
      }
      // only add the key if the buffer is no longer full
      if (KbdCount < 255) {
        KbdBuf[KbdPutPtr++] = Key;
        KbdCount++;
      }
    }

    // but whether or not it is non-zero, we need to post an event...

    // scan and post an event to all semaphores that are waiting
    for (X = 0; X < SemCount; X++)
      SemList[X]->postevent();
  }
}


int LIBENTRY kbdcount(void) {
  return(KbdCount);
}


int LIBENTRY kbdgetkeyfrombuffer(void) {
  if (KbdCount == 0)
    return(-1);

  KbdCount--;
  return(KbdBuf[KbdGetPtr++]);
}


int LIBENTRY kbdstartthread(void) {
  ThreadId = 0;

  if ((SemList = (CSemaphore **) malloc(SemMax * sizeof(CSemaphore *))) == NULL)
    return(-1);

//openkbdhandle();
  enablekbdmonitor();

  KbdPutPtr = 0;
  KbdGetPtr = 0;
  KbdCount  = 0;
  SemCount  = 0;
  ThreadId  = startthread(kbdhandler,8*1024,NULL);
  return(0);
}


void LIBENTRY kbdstopthread(void) {
  if (ThreadId != 0) {
    killthread(ThreadId,TRUE);
//  closekbdhandle();
  }
  if (SemList != NULL)
    free(SemList);
}


int LIBENTRY kbdregistersemaphore(CSemaphore &Semaphore) {
  if (SemCount >= SemMax)
    return(-1);

  DosEnterCritSec();
    SemList[SemCount++] = &Semaphore;
  DosExitCritSec();
  return(0);
}


void LIBENTRY kbdunregistersemaphore(CSemaphore &Semaphore) {
  int X;

  if (SemCount <= 0)
    return;

  for (X = 0; X < SemCount; X++) {
    if (SemList[X] == &Semaphore) {
      DosEnterCritSec();
        SemCount--;
        if (X < SemCount)
          memmove(&SemList[X],&SemList[X+1],SemCount-X);
      DosExitCritSec();
      return;
    }
  }
}


#ifdef TEST
#include <stdio.h>

void main(void) {
  CTimeoutSemaphore Wait;

  kbdstartthread();
  Wait.create(NULL);
  kbdregistersemaphore(Wait);

  printf("waiting 10 seconds - or until a key is pressed...");
  Wait.settimer(10000);
  while (! Wait.timerexpired(1000))
    printf(".");

  printf(" done.\n");

  if (kbdgetkeyfrombuffer() != -1)
    puts("a key was hit");

  kbdunregistersemaphore(Wait);
  kbdstopthread();
//setkbdstatus(0);
}
#endif
