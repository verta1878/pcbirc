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


#ifndef HPP_SEMAPHORE
#define HPP_SEMAPHORE

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif

const int SEM_WAIT_NOTIMEOUT = -1;  /* was SEM_UPDATE_ONDEMAND */
const int SEM_TIMEDOUT       = 0;  /* was SEM_UPDATE_TIMEDOUT */
const int SEM_INTERRUPTED    = 1;  /* was SEM_UPDATE_DEMANDED */

/****************************************************************************/

#pragma pack(4)
class CSemaphore {
  public:
    CSemaphore();
    virtual ~CSemaphore();

    bool   LIBENTRY create(const char *SemName);
    bool   LIBENTRY createunique(const char *SemName);
    bool   LIBENTRY open(const char *SemName);
    void   LIBENTRY close(void);
    int    LIBENTRY waitforevent(int Timeout);
    void   LIBENTRY postevent(void);
    int    LIBENTRY reset(void);
    char * LIBENTRY getname(void);

    inline bool     opened(void) { return Opened; }

  protected:
    long  Semaphore;
    bool  Opened;

  private:
    char  Name[32];
};
#pragma pack()

/****************************************************************************/

#pragma pack(4)
class CUpdateSemaphore : public CSemaphore {
  public:
    CUpdateSemaphore();
    ~CUpdateSemaphore();

    bool LIBENTRY create(const char *SemName);
    bool LIBENTRY createunique(const char *SemName);
    bool LIBENTRY open(const char *SemName);
    void LIBENTRY close(void);
    void LIBENTRY enabletimer(const int Time);
    void LIBENTRY disabletimer(void);
    int  LIBENTRY waitforevent(void);

    inline bool isenabled(void) { return(Enabled); }

  private:
    int   Interval;
    bool  Enabled;
};
#pragma pack(1)

/****************************************************************************/

#pragma pack(4)
class CTimeoutSemaphore : public CSemaphore{
  public:
    CTimeoutSemaphore();
    ~CTimeoutSemaphore();

    bool LIBENTRY open(const char *SemName);
    void LIBENTRY close(void);
    void LIBENTRY settimer(const int Time);
    bool LIBENTRY timerexpired(const int PauseTime);
    void LIBENTRY stoptimer(void);

  private:
    void *Timer;
};
#pragma pack(1)


/****************************************************************************/

#pragma pack(4)
class CMutexSemaphore {
  public:
    CMutexSemaphore();
    ~CMutexSemaphore();

    bool LIBENTRY create(const char *SemName);
    bool LIBENTRY createunique(const char *SemName);
    bool LIBENTRY open(const char *SemName);
    void LIBENTRY close(void);
    void LIBENTRY acquire(void);
    void LIBENTRY release(void);

  private:
    long  Semaphore;
    bool  Opened;
    char  Name[32];
};
#pragma pack()

/****************************************************************************/

#endif  /* #ifndef HPP_SEMAPHORE */
