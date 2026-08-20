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


#include "project.h"
#pragma hdrstop

// #include <process.h>
// #include <system.h>
#include "event.h"

#ifdef __BORLANDC__
//  #include <dir.h>
#else
  #include <io.h>
  #include <direct.h>
#endif

#ifdef __OS2__
  #include <kbd.hpp>
  #include <threads.h>
#endif

#ifndef __OS2__
  #define bgetkey(c) bgetkey2(c)
#endif

#ifdef FIDO
  #include "defines.h"
  #include "structs.h"
  /*#include "fidoque.hpp"*/
  #include "prototyp.h"
#endif

#define    EVENTDAT    PcbData.EventDatFile
#define    EVENTDIR    PcbData.EventFiles

typedef union {
    int  W;
    struct {
        char LoB,HiB;
    } B;
} keytype;

static  int             EventNum = 0;
static  int             MaxEvents;
static  unsigned        CurrentEventDate = 0;
// static  unsigned        LastEvDate = 0;
static  unsigned long   BeginEvent = 0L,
                        EndEvent = 0L;
static  EventType       CurrentEvent;

static  int LIBENTRY _NEAR_ eventminutes(char *Time, unsigned CurrentDate);


// returns a value expressed in minutes which represents both the JULIAN DAY
// as well as the current minute at the same time.  So for instance, if the
// JULIAN DAY is 1000, and the current time is 00:01, the return value would
// be ((1000 * 1440) + 1) which is the date times the number of minutes per
// day plus the current minute
static long LIBENTRY _NEAR_ exactdate(unsigned JulianDate, int Time) {
 return((((JulianDate) - 1) * (long) MINUTESPERDAY) + ((long) Time));
}


/***************************************************************************
 ***************************************************************************/
void LIBENTRY performevent(void) {
  bool        ScreenOff;
  int         RightNow;
  int         Code;
  keytype     Key;
  char        Time[9];
  char        Date[9];
  char        Str[80];
  pcbtexttype Buffer;

  if (!PcbData.EventActive)
    return;

  setcursor(CUR_NORMAL);
  agotoxy(0,0);

  ScreenOff = Status.ForceScreenOff;
  if (!Control.Screen)
    turndisplayon(FALSE);

  #ifdef COMM
    modemoffhook();
    PcbData.LeaveDtrUp = TRUE;
    PcbData.OffHook    = FALSE;
    PcbData.ResetModem = FALSE;
  #endif

  sprintf(Str, "#%d - %s", EventNum, CurrentEvent.BatchFile);
  writeusernetstatus(RUNNINGEVENT, Str);
  strcpy(Status.DisplayText, Str);

  RightNow = strtominutes(timestr2(Str));
  if (RightNow >= Status.SuspendMinute && RightNow < Status.EventMinute) {
    cls();
    agotoxy(0, 0);
    displaypcbtext(TXT_WAITINGFOREVENT, DEFAULTS);

    #ifdef __OS2__
      CTimeoutSemaphore Wait;
      Wait.createunique("ET");    //lint !e534
      Wait.settimer((Status.EventMinute - RightNow) * ONEMINUTE);
      kbdregistersemaphore(Wait); //lint !e534
      while (! Wait.timerexpired(SEM_WAIT_NOTIMEOUT)) {
        Key.W = kbdgetkeyfrombuffer();
        if (Key.B.LoB == 27 || (Key.B.LoB == 0 && Key.B.HiB == 68))
          break;
      }
      kbdunregistersemaphore(Wait);
      Wait.close();
    #else
      settimer(4,(long) (Status.EventMinute - RightNow) * ONEMINUTE);
      while (! timerexpired(4)) {
        if (bgetkey(1)) {
          Key.W = bgetkey(0);
          if (Key.B.LoB == 27 || (Key.B.LoB == 0 && Key.B.HiB == 68))
            break;
        }
        giveup();
      }
    #endif
  }

  if (ScreenOff) {                   /* was the screen togged off? */
    turndisplayoff(FALSE);         /* if yes, turn it back off   */
    Status.ForceScreenOff = TRUE;  /* and set it so that makepcboardsys() will write it out */
  }

  getpcbtext(TXT_EVENTRAN, &Buffer);
  sprintf(Str, "%s%s %s", Buffer.Str, datestr(Date), timestr1(Time));

  if (PcbData.Network)
    sprintf(&Str[strlen(Str)], " Node %d", PcbData.NodeNum);

  openlog();                                 /* log is closed at CW screen */
  writelog(Str, SPACERIGHT);                 /* open it now, write the     */
  writelog(LogDivider, LEFTJUSTIFY);         /* "ran event" message to it  */

  if ((Code = prepevent()) == -1)
    writelog("Error in 'prepevent()'", SPACERIGHT);

  closecallerlog();                          /* and then close it up again */

  // code = 0 if it's a regular event where we need to exit to dos
  // code = 1 if it's the fido mail hour in which case we just recycle

  if (Code == 0) {
    Status.ErrorLevel = EXIT_EVENT;
    Status.Logoff = RUNEVENT;
    PcbData.ExitToDos = TRUE;
  } else if (Code == 1) {
    // Override the ExitToDos flag otherwise we'll get caught in a loop
    // loading/exiting/loading/exiting since the fido mail hour does not
    // actually run an event it is always active...
    PcbData.ExitToDos = FALSE;
  }

  recycle();
}


/***************************************************************************
 *** Expand any wildcards in a date.  Return TRUE if wildcards found. ******/
static int _NEAR_ LIBENTRY expanddate(char *Query) {
  char  Today[DATELENGTH + 1];
  int   ReturnValue = FALSE;

  stripright(Query, ' ');
  if (Query[0] == 0) {
    datestr(Query);
    return WC_YEAR|WC_MONTH|WC_DAY;
  }

  datestr(Today);

  /*** check for year wildcard *******************************************/
  if ((memcmp(Query + 6, "XX", 2) == 0) ||
      (memcmp(Query + 6, "  ", 2) == 0) ||
      (memcmp(Query + 6, "00", 2) == 0)) {
    memcpy(Query + 6, Today + 6, 2);
    ReturnValue |= WC_YEAR;
  }
  /*** check for date wildcard *******************************************/
  if ((memcmp(Query + 3, "XX", 2) == 0) ||
      (memcmp(Query + 3, "  ", 2) == 0) ||
      (memcmp(Query + 3, "00", 2) == 0)) {
    memcpy(Query + 3, Today + 3, 3);
    ReturnValue |= WC_MONTH;
  }
  /*** check for month wildcard ******************************************/
  if ((memcmp(Query, "XX", 2) == 0) ||
      (memcmp(Query, "  ", 2) == 0) ||
      (memcmp(Query, "00", 2) == 0)) {
    memcpy(Query, Today, 3);
    ReturnValue |= WC_DAY;
  }

  return ReturnValue;
}


/***************************************************************************
 ***************************************************************************/
void LIBENTRY nextevent(char *EventTime) {
  int EventMinute;

  // The main event processor sets up BeginEvent.  But if we are front-ended,
  // or the /EVENT switch is used, then Status.EventMinutes holds the next
  // event time.

  if (PcbData.EventActive && (BeginEvent != 0 || Status.EventMinute != 0)) {
    EventMinute = (int) ((BeginEvent != 0 ? BeginEvent : Status.EventMinute) % MINUTESPERDAY);
    sprintf(EventTime, "%02d:%02d", EventMinute / 60, EventMinute % 60);
  } else
    strcpy(EventTime,"00:00");
}


#ifdef FIDO
static bool _NEAR_ LIBENTRY getverbstatus(char *Buf, bool CurrentStatus) {
  if (strstr(Buf,"YES") != NULL || strstr(Buf,"ON") != NULL)
    return(TRUE);

  if (strstr(Buf,"NO") != NULL || strstr(Buf,"OFF") != NULL)
    return(FALSE);

  return(CurrentStatus);
}


static bool _NEAR_ LIBENTRY processfido(char *FileName) {
  bool      Opened;
  DOSFILE   In;
  DOSFILE   Out;
  char      Buf[128];

  Opened = FALSE;
  memset(&Out,0,sizeof(DOSFILE));

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    return(FALSE);

  while (dosfgets(Buf,sizeof(Buf),&In) != -1) {
    if (memicmp(Buf,"ALLOW-HUMAN-CALLERS",19) == 0) {
      Status.AllowHumanCallers = getverbstatus(Buf,Status.AllowHumanCallers);
      continue;
    }

    if (memicmp(Buf,"ALLOW-FILE-REQUESTS",19) == 0) {
      Status.AllowFileRequests = getverbstatus(Buf,Status.AllowFileRequests);
      continue;
    }

    if (! Opened) {
      Opened = (dosfopen(".\\PCBFIDO.STS",OPEN_RDWR|OPEN_DENYRDWR,&Out) != -1);
      dosfseek(&Out,0,SEEK_END);
    }

    if (Opened) {
      dosfputs(Buf,&Out);       //lint !e534
      dosfputs("\r\n",&Out);    //lint !e534
    }
  }

  dosfclose(&In);
  dosfclose(&Out);
  return(Opened);  // if the PCBFIDO.STS file was OPENED, then we added something to it
}
#endif


/***************************************************************************
 *** unblock OS/2 thread ***************************************************/

#ifdef __OS2__
static CSemaphore EventSemaphore;
static bool       InEventThread;

#pragma argsused
static void THREADFUNC eventthread(void *Ignore) {
  EventSemaphore.reset();
  InEventThread = TRUE;

  // get the current date/minute
  int now = exactdate(getjuliandate(),currentminute());

  // calculate the number of minutes until the event runs .. note that the
  // time resolution is in minutes, so if the event is 15 seconds away it
  // will be calculated as a full minute left to the event
  int MinutesToEvent = (BeginEvent - now);

  // if the suspend minute has been calculated as non-zero, then we need to
  // subtract out the number of suspension minutes
  if (Status.SuspendMinute != 0)
    MinutesToEvent -= PcbData.EventSuspend;

  // calculate the number of seconds until the event runs, based on the
  // number of minutes that we've calculated to be remaining
  int SecondsToEvent = (MinutesToEvent * 60);

  // now we need to normalize the seconds (fix the "minute resolution" problem
  // so that we can run the event at the top of the minute) - we do this by
  // subtracting out the current second of the current minute.  For example,
  // if the event runs at 10:01 and it is now 10:00:45, we will have
  // calculated MinutesToEvent to be 1 minute (i.e. 10:01 minus 10:00), so
  // subtract the 45 seconds to make the seconds to event only 15.

  SecondsToEvent -= currentsecond();

  // if the seconds to event has not gone negative, then we need to wait
  // for the event time

  if (SecondsToEvent > 0) {
    // set up an asynchronous timer to jar the WaitKeySem semaphore at
    // approximately the time that the event should be run
    if (EventSemaphore.waitforevent(SecondsToEvent * ONESECOND) == SEM_INTERRUPTED) {
      InEventThread = FALSE;
      return;
    }
  }

  // if we get this far, then it may be time for the event
  releasekbdblock(0);
  mydelay(25);
  releasekbdblock(0);  // do it again for good measure
  InEventThread = FALSE;
}

static unsigned long EventThreadId;

void LIBENTRY destroyeventthread(void) {
  if (EventSemaphore.opened()) {
    // if the event semaphore is already opened then we may already have
    // a thread running, if so, post an event to the semaphore and that will
    // cause it to exit immediately afterwards
    if (InEventThread) {
//    EventSemaphore.postevent();
//    waitthread(EventThreadId);
      killthread(EventThreadId,TRUE);
      InEventThread = FALSE;
    }
  }
}

static void LIBENTRY createeventthread(void) {
  if (BeginEvent == 0)
    return;

  if (EventSemaphore.opened()) {
    // destroy the event thread, but leave the event semaphore open
    // so that we can use it again later
    destroyeventthread();
  } else {
    // otherwise, we'll create the event semaphore right now
    EventSemaphore.createunique("EVENT");   //lint !e534
  }

  EventThreadId = startthread(eventthread,8192,NULL);
}
#endif


/***************************************************************************
 *** check for the closest pending event ***********************************/

int LIBENTRY checkforevent(void) {
  unsigned short LastDate;
  unsigned short CurrentDateJulian;
  unsigned short YesterdayJulian;
  unsigned short NodeNumber;
  unsigned short EventDateJulian;
  int            i;
  int            WildCard;
  long           position;
  #ifdef FIDO
  unsigned long  now;
  bool           WasFidoMailHour;
  bool           AddedFido;
  #endif
  EventType      EventRec;
  DOSFILE        File;
  char           ActiveEventList[MAXACTIVEEVENTSIZE],
                 CurrentTime[TIMELENGTH + 1],
                 BeginEventTime[TIMELENGTH + 1],
                 EndEventTime[TIMELENGTH + 1];
  char           CurrentDate[DATELENGTH + 1],
                 EventDate[DATELENGTH + 1],
                 LastEventDate[DATELENGTH + 1],
                 NextEventDate[DATELENGTH + 1],
                 EventBatchFile[65];
  char           tmpStr[80];

  // If fido is enabled, then we may encounter some fido events which set
  // various behaviors.  Some of these should be recorded and remembered for
  // later processing.  This is done by storing the information in PCFIDO.STS
  // in the current directory.

  #ifdef FIDO
    WasFidoMailHour = FALSE;
    AddedFido       = FALSE;
    if (PcbData.EnableFido) {
      WasFidoMailHour          = FidoMailHour;
      FidoMailHour             = FALSE;
      Status.AllowHumanCallers = TRUE;    // default to true unless a Fido-verb overrides it
      Status.AllowFileRequests = FALSE;   // default to false
      Status.FidoEndEvent  = 0xFFFFFFFFL; // default to a large value, when a
      Status.FidoNextEvent = 0xFFFFFFFFL; // valid value is found, will be less
      unlink("PCBFIDO.STS");
      now = exactdate(getjuliandate(),currentminute());
    }
  #endif

  EventNum = 0;

  if (EVENTDAT[0] == 0 || fileexist(EVENTDAT) == 255) {
    PcbData.EventActive = FALSE;
    return FALSE;
  }

  if (dosfopen(EVENTDAT, OPEN_READ|OPEN_DENYNONE, &File) == -1)
    return FALSE;

  dossetbuf(&File, 8192);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(&MaxEvents, sizeof(unsigned short), &File) != sizeof(unsigned short)) {
    dosfclose(&File);
    return FALSE;
  }

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(ActiveEventList,ACTIVEEVENTSIZE,&File) != ACTIVEEVENTSIZE) {
    dosfclose(&File);
    return FALSE;
  }

  datestr(CurrentDate);
  timestr2(CurrentTime);
  CurrentDateJulian = datetojulian(CurrentDate);
  YesterdayJulian = (unsigned short) (CurrentDateJulian - 1);

  memset(&EventRec, 0, sizeof(EventType));
  memset(&CurrentEvent, 0, sizeof(EventType));

  memset(NextEventDate, 0, DATELENGTH + 1);
  strcpy(NextEventDate, "12-31-78");

  LastEventDate[0] = 0;

  for (i = 0; i < MaxEvents; i++) {
    if (isset(ActiveEventList, i)) {
      position = dosfseek(&File,(long) EVENTRECOFFSET + EVENTRECPOS(i),SEEK_SET);

      #ifdef DEBUG
        mc_check_buffers();
      #endif

      if (dosfread(&EventRec, sizeof(EventType), &File) != sizeof(EventType))
        break;

      if (DebugLevel > 0) {
        sprintf(tmpStr, "Event %d: %ld", i, position);
        writelog(tmpStr, SPACERIGHT);
      }

      // if this is NOT the OS/2 version, then IGNORE any OS/2-specific events
      #ifndef __OS2__
        if (EventRec.Os2)
          continue;
      #endif

      NodeNumber = (unsigned short) (PcbData.Network ? PcbData.NodeNum : 1);

      // if this is a FIDO event, then the LAST RAN DATES do not matter so
      // skip over the processing below

      if (EventRec.Mode == EV_FIDO) {
        LastEventDate[0] = 0;
        LastDate = 0;

        // if this node is not configured for FIDO then we may as well skip
        // this whole event record because it does not pertain to us

        if (! PcbData.EnableFido)
          continue;
      } else {
        position = (long) LASTDATEOFFSET + ((long) LASTDATERECPOS(i,NodeNumber) * (long) sizeof(unsigned short));
        dosfseek(&File,position,SEEK_SET);

        #ifdef DEBUG
          mc_check_buffers();
        #endif

        if (dosfread(&LastDate, sizeof(unsigned short), &File) != sizeof(unsigned short))
          LastDate = 0;  // when changing models (e.g. /2 to /10) the offsets of
                         // the LastDate fields change ... if we can't read one
                         // in then just assume an upgrade has occured and
                         // initialize the value to 0

        // if there is corruption in the file, and the date is "in the future",
        // then set the last date ran to 0 so that this event will be considered
        // to be "runable"
        if (LastDate > CurrentDateJulian)
          LastDate = 0;

        if (DebugLevel > 0) {
          sprintf(tmpStr, "Event %d: %ld", i, position);
          writelog(tmpStr, SPACERIGHT);
        }

        if (LastDate)
          strcpy(LastEventDate, juliantodate(LastDate));
        else
          LastEventDate[0] = 0;
      }

      memcpy(EventDate, EventRec.Date, DATELENGTH); EventDate[DATELENGTH] = 0;
      EventDateJulian = datetojulian(EventDate);

      memcpy(BeginEventTime, EventRec.BeginTime, TIMELENGTH); BeginEventTime[TIMELENGTH] = 0;
      memcpy(EndEventTime, EventRec.EndTime, TIMELENGTH);     EndEventTime[TIMELENGTH] = 0;

      if (memcmp(EndEventTime, "     ", TIMELENGTH) == 0)
        memcpy(EndEventTime, BeginEventTime, TIMELENGTH);

      // if the BEGIN time is later in the day than the END time, then that
      // means that the event is set to cross midnight.  First, this is not
      // supported because the "Last Date" field does not record which day
      // it ran on together with the time.  Second, they may have made a
      // mistake typing something like 12:30 and 01:30 as the begin/end times
      // thinking that 12:30 was 30 minutes after midnight.  This could result
      // in an unexpected long distance call in the middle of the day which
      // could be quite expensive.  So if the Begin time is later than the
      // end time we will simply consider this event as invalid and skip
      // over it.
      //
      // NOTE:  HOWEVER, for the purposes of FIDO, last dates do NOT matter
      //        and therefore events CAN cross midnight, so do not ignore
      //        events that cross midnight if they are fido!


      if (EventRec.Mode != EV_FIDO && memcmp(BeginEventTime,EndEventTime,TIMELENGTH) > 0)
        continue;

      if (LastEventDate[0] == 0)
        strcpy(LastEventDate, juliantodate(YesterdayJulian));

      /*** replace any wildcards found in date ***********************/
      if ((WildCard = expanddate(EventDate)) != 0) {
        int         Day,
                    Month,
                    Today,
                    Tomorrow = 0,
                    Year;
        int         j;

        EventDateJulian = datetojulian(EventDate);
        Month = atoi(EventDate);
        Day = atoi(EventDate + 3);
        Year = atoi(EventDate + 6);

        if (WildCard & WC_DAY) {
          Today = dayofweek(Month, Day, Year);
          if (EventDateJulian <= datetojulian(LastEventDate))
            Tomorrow++;

TryTomorrow:
          for (j = Today + Tomorrow; j < (Today + Tomorrow + 7); j++) {
            if (isset(EventRec.Days, j % 7)) {
              EventDateJulian += (unsigned short) (j - Today);
              strcpy(EventDate, juliantodate(EventDateJulian));
              break;
            }
          }

          if (EventDateJulian == CurrentDateJulian) {
            if (memcmp(BeginEventTime, EndEventTime, TIMELENGTH) > 0) {
              int Hour;
              int Minute;

              Hour = atoi(EndEventTime);
              Minute = atoi(strchr(EndEventTime, ':') + 1);  //lint !e613 this code will fail if the ':' is not present
              sprintf(EndEventTime, "%02d:%02d", Hour + 24, Minute);
            }
            if (memcmp(EndEventTime, CurrentTime, TIMELENGTH) < 0) {
              if (Tomorrow++ > 7)
                continue;
              goto TryTomorrow;
            }
          }
        } else if (WildCard & WC_MONTH) {
/* original code from louis:
          sprintf(EventDate, "%02d-%02d-%02d", (++Month == 13) ? 1 : Month, Day, (Month == 1) ? ++Year : Year);
** changed to the TWO lines you see below: */
          sprintf(EventDate, "%02d-%02d-%02d", (Month+1 == 13) ? 1 : Month, Day, (Month == 1) ? ++Year : Year);
          Month++;

          EventDateJulian = datetojulian(EventDate);
        } else if (WildCard & WC_YEAR) {
          sprintf(EventDate, "%02d-%02d-%02d", Month, Day, ++Year);
          EventDateJulian = datetojulian(EventDate);
        }
      }


      // has the event already run?
      // is the last event date more recent (a higher number) than the event
      // date that we are evaluating?
      if (EventDateJulian <= datetojulian(LastEventDate))
        continue;

      // is the event we are evaluating previous to today?
      if (EventDateJulian < CurrentDateJulian)
        continue;

      // is the event we are evaluating beyond NextEventDate?
      if (EventDateJulian > datetojulian(NextEventDate))
        continue;

      // is the event we are evaluating equal to NextEventDate?
      if (EventDateJulian == datetojulian(NextEventDate)) {
        if (memcmp(BeginEventTime, EndEventTime, TIMELENGTH) > 0) {
          int Hour;
          int Minute;

          Hour = atoi(EndEventTime);
          Minute = atoi(strchr(EndEventTime, ':') + 1);
          sprintf(EndEventTime, "%02d:%02d", Hour + 24, Minute);
        }
/*
        if (memcmp(EndEventTime, CurrentTime, TIMELENGTH) < 0)
          continue;
*/
        if (memcmp(BeginEventTime, CurrentEvent.BeginTime, TIMELENGTH) >= 0)
          continue;
      }

      // if we were ALREADY in the fido mail hour, and this event is set to
      // go off, then avoid running it again and just re-enable the fido
      // mail hour.  Also, set the fido end event time so that we'll be able
      // to turn the fido mail hour off when it's time.  then just loop around
      // and check for other events that might need to run

      #ifdef FIDO
        if (EventRec.Mode == EV_MAIL && WasFidoMailHour) {
          Status.AllowHumanCallers = TRUE;
          BeginEvent = exactdate(EventDateJulian,strtominutes(EventRec.BeginTime));
          EndEvent   = exactdate(EventDateJulian,strtominutes(EventRec.EndTime));
          // should we "renew" the fido mail hour?
          if (now >= BeginEvent && now <= EndEvent) {  //lint !e644 now is initialized if PcbData.EnableFido
            FidoMailHour = TRUE;                 // yes the mail hour continues
            Status.AllowHumanCallers = FALSE;
            if (EndEvent < Status.FidoEndEvent)
              Status.FidoEndEvent = EndEvent;
          }
          continue;
        }
      #endif

      stripright(EventRec.BatchFile, ' ');
      stripright(EVENTDIR, ' ');
      sprintf(EventBatchFile, "%s%s.%03d", EVENTDIR, EventRec.BatchFile, NodeNumber);
      if (fileexist(EventBatchFile) == 255) {
        buildstr(EventBatchFile, EVENTDIR, EventRec.BatchFile, NULL);
        if (fileexist(EventBatchFile) == 255) {
          if (DebugLevel > 0) {
            sprintf(tmpStr, "Event %d: %s is MISSING", i, EventBatchFile);
            writedebugrecord(tmpStr);
          }
          continue;
        }
      }

      // FIDO events are not really events at all!  Instead, PCBoard should
      // read in the FIDO settings and store them somewhere.  No further
      // processing (as far as events are concerned) should be performed, so
      // at the end of this check we'll loop back up for more event processing
      // instead of setting up for the event to run.

      #ifdef FIDO
        if (PcbData.EnableFido && EventRec.Mode == EV_FIDO) {
          // check to see if we are inside the "fido window"
          BeginEvent = exactdate(EventDateJulian,strtominutes(EventRec.BeginTime));
          EndEvent   = exactdate(EventDateJulian,strtominutes(EventRec.EndTime));
          if (now >= BeginEvent && now <= EndEvent) {  //lint !e644 now is initialized if PcbData.EnableFido
            if (DebugLevel > 0) {
              sprintf(tmpStr, "Event %d: %s ACTIVE", i, EventRec.BatchFile);
              writedebugrecord(tmpStr);
            }

            // yes, we're in the window, now go read the fido state file
            AddedFido |= processfido(EventBatchFile);

            // We want to keep track of the "end event" time that is CLOSEST
            // to being "next" as far as FIDO is concerned so that the
            // call waiting screen will now when to re-process the event.dat
            // file.  If Status.FidoEndEvent is zero, then it hasn't been
            // initialized yet.  If EndEvent is less than the current value of
            // Status.FidoEndEvent then we've found one that is "closer" to
            // needing to be run, so record it.

            if (EndEvent < Status.FidoEndEvent)
              Status.FidoEndEvent = EndEvent;
          } else {
            // we weren't inside the window, but we need to keep track of
            // when the next fido event is, so if the BeginEvent time is
            // closer than any previously recorded BeginEvent, then record it.
            if (BeginEvent < Status.FidoNextEvent)
              Status.FidoNextEvent = BeginEvent;
          }
          continue;
        }
      #endif


      // set up an event for running

      memcpy(NextEventDate, EventDate, DATELENGTH);
      CurrentEventDate = EventDateJulian;
      CurrentEvent = EventRec;
//    LastEvDate = LastDate;
      EventNum = i + 1;
    }
  }

  if (EventNum) {
    if ((Status.EventMinute = (short) eventminutes(CurrentEvent.BeginTime,CurrentDateJulian)) == -1) {
      Status.EventMinute    = 0;
      Status.SuspendMinute  = 0;
      Status.EndEventMinute = 0;
    } else {
      Status.SuspendMinute  = (short) (Status.EventMinute - PcbData.EventSuspend);
      Status.EndEventMinute = (short) eventminutes(CurrentEvent.EndTime,CurrentDateJulian);

      // an event time of 00:00 MIDNIGHT cannot be detected because it is
      // equal to zero, so we'll "fudge" a little and make it 12:01
      if (Status.EventMinute == 0)
        Status.EventMinute = 1;
      else if (Status.SuspendMinute < 0)                // did we cross midnight going backwards?
        Status.SuspendMinute += (short) MINUTESPERDAY;  // if so add MINUTESPERDAY minutes to the number
    }

//  memcpy(BeginEventTime, CurrentEvent.BeginTime, TIMELENGTH); BeginEventTime[TIMELENGTH] = 0;
//  memcpy(EndEventTime, CurrentEvent.EndTime, TIMELENGTH);     EndEventTime[TIMELENGTH] = 0;

    BeginEvent = exactdate(CurrentEventDate,strtominutes(CurrentEvent.BeginTime));
    EndEvent   = exactdate(CurrentEventDate,strtominutes(CurrentEvent.EndTime));
  }

/* End: */
  dosfclose(&File);

  if (DebugLevel > 0 && EventNum > 0) {
    sprintf(tmpStr, "Event #%d to be executed at %d", EventNum, Status.EventMinute);
    writelog(tmpStr, SPACERIGHT);
  }

  #ifdef FIDO
    if (PcbData.EnableFido && AddedFido) {
      // don't bother checking the queue unless this node handles fido mail
      // either importing, exporting or dialing out
      if (PcbData.FidoDialOut || PcbData.FidoProcessOut || PcbData.FidoProcessIn) {
        // only check the queue if there is no caller online and no other node
        // is currently handling mail
        if (Asy.Online == OFFLINE && okaytohandlemail()) {
          scanQueue();
          writeusernetstatus(NOCALLER,NULL);  // reset status - okaytohandlemai() had set it to handling mail
        }
      }
    }
  #endif

  return EventNum;
}


/***************************************************************************
    prepevent() returns:

    -1 on error
     0 to run an event
     1 for the fido mail hour

 ***************************************************************************/

int LIBENTRY prepevent(void) {
  unsigned  LastDate;
  unsigned  NodeNumber;
  char     *p;
  char      CurrentDate[DATELENGTH + 1];
  DOSFILE   File;
  char      EventBatchFile[65];

  if (EVENTDAT[0] == 0 || fileexist(EVENTDAT) == 255) {
    PcbData.EventActive = FALSE;
    return(-1);
  }

  if (dosfopen(EVENTDAT, OPEN_RDWR|OPEN_DENYNONE, &File) == -1)
    return(-1);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(&MaxEvents, sizeof(unsigned short), &File) != sizeof(unsigned short)) {
    dosfclose(&File);
    return(-1);
  }

  dosfseek(&File,EVENTRECOFFSET + EVENTRECPOS(EventNum-1),SEEK_SET);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(&CurrentEvent, sizeof(EventType), &File) != sizeof(EventType)) {
    dosfclose(&File);
    return(-1);
  }


  #ifdef FIDO
    // MAIL events don't run batch files, so just return now
    if (CurrentEvent.Mode == EV_MAIL && PcbData.EnableFido) {
      dosfclose(&File);
      FidoMailHour = TRUE;
      Status.AllowHumanCallers = FALSE;
      return(1);
    }
  #endif

  NodeNumber = (PcbData.Network ? PcbData.NodeNum : 1);
  LastDate = datetojulian(datestr(CurrentDate));

  dosrewind(&File);  // reset the file because we are going to write to it

  dosfseek(&File,LASTDATEOFFSET + ((long) LASTDATERECPOS(EventNum-1,NodeNumber) * sizeof(unsigned short)),SEEK_SET);
  dosfwrite(&LastDate, sizeof(unsigned short), &File);  //lint !e534
  dosfclose(&File);

  stripright(CurrentEvent.BatchFile, ' ');
  stripright(EVENTDIR, ' ');
  sprintf(EventBatchFile, "%s%s.%03d", EVENTDIR, CurrentEvent.BatchFile, NodeNumber);

  if (fileexist(EventBatchFile) == 255)
    buildstr(EventBatchFile, EVENTDIR, CurrentEvent.BatchFile, NULL);

  #ifdef __OS2__
    if (CurrentEvent.Os2)
      p = ".\\EVENT.CMD";
    else
  #endif
  p = ".\\EVENT.BAT";

  if (dosfopen(p,OPEN_CREATE|OPEN_RDWR|OPEN_DENYRDWR,&File) == -1) {
    #ifdef __OS2__
      writedebugrecord("Error Creating EVENT.CMD/EVENT.BAT");
    #else
      writedebugrecord("Error Creating EVENT.BAT");
    #endif
    return(-1);
  }

  setpcbenv(&File, FALSE);
  if (dosfclose(&File) == -1)
    return(-1);

  if (appendfile(EventBatchFile,p,TRUE) != 0) {
    #ifdef __OS2__
      writedebugrecord("Error Updating EVENT.CMD");
    #else
      writedebugrecord("Error Updating EVENT.BAT");
    #endif
    return(-1);
  }

/***************************************************************************
 Debugging information to aid in figuring out why some random nodes aren't
 executing events properly.
 ***************************************************************************/
  if (DebugLevel > 0) {
    char tmpStr[80];

    sprintf(tmpStr, "EVENT: %c %d %s", (char)CurrentEvent.Mode, EventNum, EventBatchFile);
    writelog(tmpStr, SPACERIGHT);
  }

  return(0);
}


/***************************************************************************
 ***************************************************************************/
static int _NEAR_ LIBENTRY eventminutes(char *Time, unsigned CurrentDate) {
  int TimeThen;

  if (EventNum != 0 && (CurrentEvent.Mode == EV_EXPEDITE || CurrentEvent.Mode == EV_MAIL)) {
    TimeThen = strtominutes(Time);

    if (CurrentDate < CurrentEventDate)
      TimeThen += (CurrentEventDate - CurrentDate) * MINUTESPERDAY;

    return(TimeThen);
  }
  return(-1);
}


/***************************************************************************
 ***************************************************************************/
int LIBENTRY timeforevent(void) {
  static unsigned Today = 0;
  unsigned int    Date;
  unsigned long   now;

  if (EventNum == 0)
    return 0;           /*** No event defined **************************/


  Date = getjuliandate();

  if (Today == 0)                    // Today is a static variable
    Today = Date;                    // if not initialized, set it now
  else if (Date != Today) {          // if we've crossed midnight
    Today = Date;                    //   then we need to re-load the
    #ifdef __OS2__                   //   event numbers, return -1 to signal it
      createeventthread();
    #endif
    return -1;
  }

  now = exactdate(Date,currentminute());

  if (now > (unsigned long)EndEvent)
    return -1;

  if (now < (unsigned long)BeginEvent) {
    if (CurrentEvent.Mode != EV_EXPEDITE && CurrentEvent.Mode != EV_MAIL)
      goto notyet;
    if (now < (unsigned long)(BeginEvent - PcbData.EventSuspend))
      goto notyet;
  }
  return(EventNum);

notyet:
  #ifdef __OS2__
    createeventthread();
  #endif
  return(0);
}


#ifdef FIDO
bool LIBENTRY checkfidowindow(void) {
  bool     WasFidoMailHour;
  unsigned long now;

  // check to see if a "fido window" has passed out of scope and, if so,
  // re-process the event.dat file

  if (! PcbData.EnableFido || (Status.FidoEndEvent == 0xFFFF && Status.FidoNextEvent == 0xFFFF))
    return(FALSE);

  now = exactdate(getjuliandate(),currentminute());
  if (now > (unsigned long) Status.FidoEndEvent || now > (unsigned long) Status.FidoNextEvent) {
    WasFidoMailHour = FidoMailHour;
    checkforevent();
    return(WasFidoMailHour != FidoMailHour);
  }
  return(FALSE);
}
#endif
