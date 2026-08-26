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


/******************************************************************************

EVENT.DAT File Layout

BITMAPSIZE = ((MAXEVENTS + 7) / 8)
RECSIZE    = sizeof(EventType)         (currently equal to 33)


Offset              Type               Description
------              ----               -----------
  0                 unsigned int       Maximum number of events (currently 64)
  2                 dynamic bitmap     Active event list; 1 bit per event
  2+BITMAPSIZE      event records      Event name, time, days, etc.
  2+BITMAPSIZE+
  MAXEVENTS*RECSIZE unsigned int       Last Date Run (unique to each node)

The complete size of the Last Date Run field is computed by multiplying 2 (the
size of an integer) times the number of nodes times the number of events.
Example:  A 10 node system would use 2*10*64 bytes (=1280 bytes) to hold the
entire Last Date Run field.

******************************************************************************/

#ifndef LIB
#ifndef EVENT_H
#define EVENT_H

#define MAX_EVENTS      64  /* This is _THE_ maximum number of events */
/* MaxEvents is a variable which is used */
/* instead of MAX_EVENTS when reading in */
/* the file because it might not be 64 */

#ifdef  PCB_MAXNODES  /* test to see if compiling PCBoard */
#define MAXNODES        PCB_MAXNODES  /* MAXNODES is a constant */
#else  /* nope, compiling PCBSetup instead */
#define MAXNODES        MaxNodes  /* MAXNODES is a variable in PCBSetup */
#endif


#define MAXEVENTOFFSET       0
#define MAXEVENTSIZE         (sizeof(unsigned short))
#define ACTIVEEVENTOFFSET    (MAXEVENTOFFSET + MAXEVENTSIZE)
#define ACTIVEEVENTSIZE      ((MaxEvents + 7) / 8)
#define MAXACTIVEEVENTSIZE   ((MAX_EVENTS + 7) / 8)
#define EVENTRECOFFSET       (ACTIVEEVENTOFFSET + ACTIVEEVENTSIZE)
#define EVENTRECSIZE         ((char) MaxEvents * (int) sizeof(EventType))
#define LASTDATEOFFSET       ((long) EVENTRECOFFSET + (long) EVENTRECSIZE)
#define LASTDATERECSIZE      ((unsigned) MAXNODES * sizeof(unsigned short))
#define LASTDATESIZE         ((unsigned) MaxEvents * LASTDATERECSIZE)
#define TOTALEVENTSIZE       (LASTDATEOFFSET + LASTDATESIZE)


/* This is the offset of the Last Date Record ... When accessing an Array of */
/* Last Date values, use it directly.  When accessing the disk, it needs to be */
/* multiplied by the size of the Last Date record which is sizeof(unsigned) */
 /* */
/* NOTE:  EventNum is a 0-based number (i.e. the first event is #0) */
/*        However, Node Number is a 1-based number (i.e. node #1 is 1) */

#define LASTDATERECPOS(EventNum,NodeNum)  (((EventNum) * (long) MAXNODES) + ((long) (NodeNum) - 1))


/* This is the offset of the Event Record on disk. */
 /* */
/* NOTE: EventNum is a 0-based number (i.e. the first event is #0) */

#define EVENTRECPOS(EventNum)   ((long) (EventNum) * sizeof(EventType))


/* This macro calculates the EXACT date by multiplying a julian date value */
/* by the number of minutes in a day and then adding the hour and minute values */
/* to it. */
 /* */
/* NOTE:  For some reason Louis subtracted one from the julian date, I don't */
/*        no why, but I've copied it into the macro verbatim */

#define MINUTESPERHOUR    60
#define HOURSPERDAY       24
#define MINUTESPERDAY     (MINUTESPERHOUR * HOURSPERDAY)

#define DATELENGTH        8
#define EVENTTIMER        7
#define FNLENGTH          10
#define TIMELENGTH        5

#define WC_YEAR           0x04
#define WC_MONTH          0x02
#define WC_DAY            0x01

#define WILDCARD_XX       0x5858
#define WILDCARD_00       0x3030
#define WILDCARD_SPACES   0x2020


typedef enum { EV_EXPEDITE = 'E', EV_IDLE = 'I', EV_NONE = 'N', EV_SLIDE = 'S', EV_FIDO = 'F', EV_MAIL = 'M' } EventMode;
typedef enum { SUNDAY = 1, MONDAY = 2, TUESDAY = 4, WEDNESDAY = 8, THURSDAY = 16, FRIDAY = 32, SATURDAY = 64 } weekdays;

#pragma pack(1)
typedef struct  {
    char           Mode;  /* holds values of type "EventMode" */
    char           Os2;   /* true means OS/2-specific event */
    unsigned short Node;
    char           Days[1];
    char           BeginTime[TIMELENGTH];
    char           EndTime[TIMELENGTH];
    char           Date[DATELENGTH];
    char           BatchFile[FNLENGTH];
} EventType;
#pragma pack()

/* extern  int     EventNum; */
extern  int     EventMinutes;
extern  int     SuspendMinutes;

extern  int     LIBENTRY  checkforevent(void);
extern  int     LIBENTRY  prepevent(void);
extern  int     LIBENTRY  timeforevent(void);
extern  void    LIBENTRY  performevent(void);
extern  void    LIBENTRY  nextevent(char *);
extern  bool    LIBENTRY  checkfidowindow(void);

#ifdef __OS2__
  void LIBENTRY destroyeventthread(void);
#else
  #define destroyeventthread()
#endif

#endif
#endif
