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
  #define INCL_DOSDATETIME
  #define INCL_DOSASYNCTIMER
  #define INCL_DOSSEMAPHORES
  #define INCL_DOSPROCESS
  #define INCL_ERRORS
  #include <os2.h>
#endif

#include <stdio.h>
#include <string.h>
#include <semafore.hpp>

#ifdef DEBUG
  #include <memcheck.h>
#endif


/****************************************************************************/


static char * LIBENTRY makename(char *Name, const char *SemName) {
  #ifdef __OS2__
    if (SemName == NULL ||  SemName[0] == 0) {
      Name[0] = 0;
      return(NULL);
    } else {
      strcpy(Name,"\\SEM32\\");
      strcat(Name,SemName);
    }
  #endif
  return(Name);
}


/****************************************************************************/


CSemaphore::CSemaphore(void) {
  Semaphore = 0;
  Opened    = FALSE;
}


CSemaphore::~CSemaphore(void) {
  close();
}


char * LIBENTRY CSemaphore::getname(void) {
  if (Name[0] == 0)
    return(NULL);

  #ifdef __OS2__
    return(Name + 7);  // return a pointer to the name after "\SEM32\"
  #endif
  return(Name);
}

bool LIBENTRY CSemaphore::create(const char *SemName) {
  #ifdef __OS2__
    if (DosCreateEventSem(makename(Name,SemName),(PHEV) &Semaphore,0,0) == 0)
      Opened = TRUE;
  #endif

  return(Opened);
}

bool LIBENTRY CSemaphore::createunique(const char *SemName) {
  char Name[66];

  for (int Attempt = 1; Attempt < 0x100; Attempt++) {
    sprintf(Name,"%s%x",SemName,Attempt);
    if (CSemaphore::create(Name))
      return(TRUE);
  }
  return(FALSE);
}

bool LIBENTRY CSemaphore::open(const char *SemName) {
  #ifdef __OS2__
    if (DosOpenEventSem(makename(Name,SemName),(PHEV) &Semaphore) == 0)
      Opened = TRUE;
  #endif

  return(Opened);
}


void LIBENTRY CSemaphore::close(void) {
  if (Opened) {
    #ifdef __OS2__
      DosCloseEventSem(Semaphore);
    #endif
    Semaphore = 0;
    Opened    = FALSE;
    Name[0]   = 0;
  }
}


void LIBENTRY CSemaphore::postevent(void) {
  #ifdef __OS2__
    DosPostEventSem(Semaphore);
  #endif
}


int LIBENTRY CSemaphore::waitforevent(int Timeout) {
  #ifdef __OS2__
    APIRET rc;
    rc = DosWaitEventSem(Semaphore,Timeout);
    return(rc == ERROR_TIMEOUT ? SEM_TIMEDOUT : SEM_INTERRUPTED);
  #endif
  return(0);
}


int LIBENTRY CSemaphore::reset(void) {
  long CountOfEvents;

  #ifdef __OS2__
    DosResetEventSem(Semaphore,(PULONG) &CountOfEvents);
  #endif
  return(CountOfEvents);
}


/****************************************************************************/


CUpdateSemaphore::CUpdateSemaphore(void) : CSemaphore() {
  Interval  = 0;
  Enabled   = FALSE;
}


CUpdateSemaphore::~CUpdateSemaphore(void) {
  close();
}


bool LIBENTRY CUpdateSemaphore::create(const char *SemName) {
  if (CSemaphore::create(SemName)) {
    Interval = SEM_WAIT_NOTIMEOUT;
    Enabled  = FALSE;
  }
  return(Opened);
}


bool LIBENTRY CUpdateSemaphore::createunique(const char *SemName) {
  if (CSemaphore::createunique(SemName)) {
    Interval = SEM_WAIT_NOTIMEOUT;
    Enabled  = FALSE;
  }
  return(Opened);
}


bool LIBENTRY CUpdateSemaphore::open(const char *SemName) {
  if (CSemaphore::open(SemName)) {
    Interval = SEM_WAIT_NOTIMEOUT;
    Enabled  = FALSE;
  }
  return(Opened);
}


void LIBENTRY CUpdateSemaphore::close(void) {
  CSemaphore::close();
  Interval = 0;
  Enabled  = FALSE;
}


void LIBENTRY CUpdateSemaphore::enabletimer(const int Time) {
  Interval = Time;
  Enabled  = TRUE;
  postevent();
}


void LIBENTRY CUpdateSemaphore::disabletimer(void) {
  Interval = SEM_WAIT_NOTIMEOUT;
  if (Enabled) {
    Enabled = FALSE;
    postevent();
  }
}


int LIBENTRY CUpdateSemaphore::waitforevent(void) {
  return(CSemaphore::waitforevent(Interval));
}


/****************************************************************************/


CTimeoutSemaphore::CTimeoutSemaphore(void) : CSemaphore() {
  Timer = 0;
}


CTimeoutSemaphore::~CTimeoutSemaphore(void) {
  CSemaphore::close();
  close();
}


bool LIBENTRY CTimeoutSemaphore::open(const char *SemName) {
  if (CSemaphore::open(SemName))
    return(TRUE);
  return(FALSE);
}


void LIBENTRY CTimeoutSemaphore::close(void) {
  CSemaphore::close();
  Timer = 0;
}


void LIBENTRY CTimeoutSemaphore::stoptimer(void) {
  CSemaphore::postevent();
  #ifdef __OS2__
    if (Timer != 0) {
      DosStopTimer((HTIMER) Timer);
      Timer = 0;
    }
  #endif
}


void LIBENTRY CTimeoutSemaphore::settimer(const int Time) {
  reset();
  #ifdef __OS2__
    Timer = 0;
    DosAsyncTimer(Time,(HSEM) Semaphore,(PHTIMER) &Timer);
  #endif
}


bool LIBENTRY CTimeoutSemaphore::timerexpired(const int Time) {
  // pause for "Time" period of milliseconds
  // during that "Time", an asynchronous event timer might trigger, if
  // it does, we will use it to abort this event and we'll return TRUE
  // to indicate that the "timer" has expired, otherwise FALSE indicates
  // that the timer has not expired, but control is returned after the
  // "Time" period of the pause has completed
  if (CSemaphore::waitforevent(Time) == SEM_TIMEDOUT)
    return(FALSE);
  return(TRUE);
}


/****************************************************************************/

CMutexSemaphore::CMutexSemaphore(void) {
  Semaphore = 0;
  Opened    = FALSE;
}


CMutexSemaphore::~CMutexSemaphore(void) {
  close();
}


bool LIBENTRY CMutexSemaphore::create(const char *SemName) {
  #ifdef __OS2__
    if (DosCreateMutexSem(makename(Name,SemName),(PHMTX) &Semaphore,0,0) == 0)
      Opened = TRUE;
  #endif

  return(Opened);
}


bool LIBENTRY CMutexSemaphore::open(const char *SemName) {
  #ifdef __OS2__
    if (DosOpenMutexSem(makename(Name,SemName),(PHMTX) &Semaphore) == 0)
      Opened = TRUE;
  #endif

  return(Opened);
}

bool LIBENTRY CMutexSemaphore::createunique(const char *SemName) {
  char Name[66];

  for (int Attempt = 1; Attempt < 0x100; Attempt++) {
    sprintf(Name,"%s%x",SemName,Attempt);
    if (CMutexSemaphore::create(Name))
      return(TRUE);
  }
  return(FALSE);
}


void LIBENTRY CMutexSemaphore::close(void) {
  if (Opened) {
    #ifdef __OS2__
      DosCloseMutexSem(Semaphore);
    #endif
    Semaphore = 0;
    Opened    = FALSE;
    Name[0]   = 0;
  }
}


void LIBENTRY CMutexSemaphore::acquire(void) {
  #ifdef __OS2__
    DosRequestMutexSem(Semaphore,-1);
  #endif
}


void LIBENTRY CMutexSemaphore::release(void) {
  #ifdef __OS2__
    DosReleaseMutexSem(Semaphore);
  #endif
}

/****************************************************************************/
