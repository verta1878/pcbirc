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


#include <process.h>
#include "threads.h"

#ifdef DEBUG
  #include <memcheck.h>
#endif

#ifdef __WATCOMC__
  #include <malloc.h>
#endif

#define  INCL_DOSPROCESS
#include <os2.h>


unsigned long LIBENTRY startthread(void (THREADFUNC *func)(void *), int StackSize, void *Param) {
  #ifdef __BORLANDC__
    return(_beginthread(func,StackSize,Param));
  #else
    // under Watcom go with at least 4K stack plus the requested stack size
    return(_beginthread(func,malloc(StackSize+4096),StackSize+4096,Param));
  #endif
}


void LIBENTRY waitthread(unsigned long ThreadId) {
  DosWaitThread(&ThreadId,DCWW_WAIT);
}


void LIBENTRY killthread(unsigned long ThreadId, bool Wait) {
  DosKillThread(ThreadId);
  if (Wait)
    waitthread(ThreadId);
}

