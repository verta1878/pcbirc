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


#include <system.h>

#ifndef __OS2__
//#pragma inline
#endif


/*  NOTE:  This module exists because sometimes OS/2 does not update the  */
/*         timer clock ticks at address 0:46Ch.  Meanwhile, OS/2 *does*   */
/*         manage to keep the correct time.  So these functions allow you */
/*         to get the current time and compare it against another time to */
/*         determine how many hundredths of seconds have passed.          */


/********************************************************************
*
*  Function:  dosgetlongtime()
*
*  Desc    :  Asks DOS for the current time and returns the time expressed in
*             hundredths of a second.  Two times can be directly compared
*             to see if one is more recent than the other, however the
*             doselapsedtime() function is prefered so that it can watch for
*             crossing midnight and return an adjusted difference.
*/

long LIBENTRY dosgetlongtime(void) {
    systimetype SysTime;

    getsystime(&SysTime);
    return((long) (((long)  SysTime.Hours * 60 * 60 * 100) +
                   ((long)  SysTime.Minutes * 60 * 100)    +
                   ((long)  SysTime.Seconds * 100)         +
                            SysTime.Hundredths));
}


long LIBENTRY doselapsedtime(long StartTime) {
  long CurTime;

  CurTime = dosgetlongtime();

  /* have we crossed midnight? */
  if (StartTime > CurTime)
    CurTime += (long) (24L * 60L * 60L * 100L);  // add 1 day

  return(CurTime - StartTime);
}


#if defined(__OS2__) || defined(__WATCOMC__)
static int StartTime[13];
static int EndTime[13];


void LIBENTRY settimer(int WhichTimer, int Thousandths) {
  int Hundredths = Thousandths / 10;

  EndTime[WhichTimer] = (StartTime[WhichTimer] = dosgetlongtime()) + Hundredths;
//StartTime[WhichTimer] = EndTime[WhichTimer] = dosgetlongtime();
//EndTime[WhichTimer] += Hundredths;
}


int LIBENTRY gettimer(int WhichTimer) {
  int CurTime;

  CurTime = dosgetlongtime();
  if (CurTime < StartTime[WhichTimer])
    CurTime += (long) (24L * 60L * 60L * 100L);  // add 1 day

  // return time in thousands by multiplying the difference, which is in
  // hundredths by 10
  return((EndTime[WhichTimer] - CurTime) * 10);
}


bool LIBENTRY timerexpired(int WhichTimer) {
  return(gettimer(WhichTimer) <= 0 ? TRUE : FALSE);
}
#endif





#ifdef NOTNEEDED

this function is no longer used

#if defined(__OS2__) || defined(__WATCOMC__)
  typedef union {
    long Whole;
    struct {
      unsigned char Hundredths;
      unsigned char Seconds;
      unsigned char Minutes;
      unsigned char Hours;
    } Separate;
  } septimetype;

#else

  /* byte layout for the long integer values storing time values */
  /*                                                             */
  /* offset 0 = hundredths                                       */
  /* offset 1 = seconds                                          */
  /* offset 2 = minutes                                          */
  /* offset 3 = hours                                            */

  #define HUNDREDTHS  0
  #define SECONDS     1
  #define MINUTES     2
  #define HOURS       3
#endif

/********************************************************************
*
*  Function:  dosdifflongtime()
*
*  Desc    :  Takes two long integers, in the format returned from the
*             dosgetlongtime() function up above and subtracts the t2 value
*             from the t1 value and returns the result.
*
*  Returns :  The number of hundredths of seconds difference between t1 and t2.
*/

long LIBENTRY dosdifflongtime(long t1, long t2) {
  long        Diff;
  #if defined(__OS2__) || defined(__WATCOMC__)
    bool        Borrow;
    septimetype T1;
    septimetype T2;
  #endif

  if (t1 < t2)
    t1 += 0x18000000L;  /* add 24 hours */

  #if defined(__OS2__) || defined(__WATCOMC__)
    T1.Whole = t1;
    T2.Whole = t2;

    Borrow = FALSE;
    if (T1.Separate.Hundredths < T2.Separate.Hundredths) {
      T1.Separate.Hundredths += (char) 100;
      Borrow = TRUE;
    }
    Diff = T1.Separate.Hundredths - T2.Separate.Hundredths;

    if (Borrow) {
      if (T1.Separate.Seconds != 0) {
        T1.Separate.Seconds--;
        Borrow = FALSE;
      } else
        T1.Separate.Seconds = 60;
    }

    if (T1.Separate.Seconds < T2.Separate.Seconds) {
      T1.Separate.Seconds += (char) 60;
      Borrow = TRUE;
    }
    Diff += (T1.Separate.Seconds - T2.Separate.Seconds) * 100;

    if (Borrow) {
      if (T1.Separate.Minutes != 0) {
        T1.Separate.Minutes--;
        Borrow = FALSE;
      } else
        T1.Separate.Minutes = 60;
    }

    if (T1.Separate.Minutes < T2.Separate.Minutes) {
      T1.Separate.Minutes += (char) 60;
      Borrow = TRUE;
    }

    if (Borrow)
      T1.Separate.Hours--;

    Diff += ((T1.Separate.Minutes - T2.Separate.Minutes) * 60 * 100) +
            ((T1.Separate.Hours   - T2.Separate.Hours) * 60 * 60 * 100);

  #else  /* ifdef __OS2__ */

    Diff = 0;

    asm mov al, byte ptr t1 + HUNDREDTHS
    asm mov bl, byte ptr t2 + HUNDREDTHS

    asm sub al, bl                            /* subtract hundredths */
    asm jz  getseconds
    asm jnc storehund
    asm dec byte ptr t1 + SECONDS             /* borrow from seconds */
    asm sub al, 256-100                       /* adjust for 99 as maximum */

    storehund:
    Diff = _AL;

    getseconds:
    asm mov al, byte ptr t1 + SECONDS
    asm mov bl, byte ptr t2 + SECONDS

    asm cmp al, 0FFh
    asm jne subseconds
    asm mov al, 59
    asm sub byte ptr t1 + MINUTES, 1
    asm cmp byte ptr t1 + MINUTES, 0FFh
    asm jne subseconds
    asm mov byte ptr t1 + MINUTES, 59
    asm sub byte ptr t1 + HOURS, 1

    subseconds:
    asm sub al, bl                            /* subtract seconds */
    asm jz  getminutes
    asm jnc storesec
    asm dec byte ptr t1 + MINUTES             /* borrow from minutes */
    asm sub al, 256-60                        /* adjust for 59 as maximum */

    storesec:
    Diff += _AL * 100;

    getminutes:
    asm mov al, byte ptr t1 + MINUTES
    asm mov bl, byte ptr t2 + MINUTES

    asm cmp al, 0FFh
    asm jne subminutes
    asm mov al, 59
    asm sub byte ptr t1 + HOURS, 1

    subminutes:
    asm sub al, bl                            /* subtract minutes */
    asm jz  subhours
    asm jnc storemin
    asm dec byte ptr t1 + HOURS
    asm sub al, 256-60                        /* adjust for 59 as maximum */

    storemin:
    Diff += ((unsigned) _AL) * 100 * 60L;

    subhours:
    asm mov al, byte ptr t1 + HOURS
    asm mov bl, byte ptr t2 + HOURS
    asm sub al, bl                            /* subtract hours */
    asm jz  done

    Diff += ((long) _AL) * 100 * 60 * 60;

    done:
  #endif   /* ifdef __OS2__ */

  return(Diff);
}   /*lint !e563 */
#endif


#ifdef TEST
#include <screen.h>
#include <stdio.h>

void main(void) {
  long t1,t2;

  setdelay();
  t1 = dosgetlongtime();
  mydelay(550);
  t2 = dosgetlongtime();
  printf("t2 = %ld\nt1 = %ld\ndiff = %ld\n",t2,t1,dosdifflongtime(t2,t1));
  #if defined(__OS2__) || defined(__WATCOMC__)
    septimetype T1,T2;
    T1.Whole = t1;
    T2.Whole = t2;
    printf("T2.hours = %2d, T2.minutes=%2d, T2.seconds=%2d, T2.hundredths=%2d\n",
           T2.Separate.Hours,
           T2.Separate.Minutes,
           T2.Separate.Seconds,
           T2.Separate.Hundredths);
    printf("T1.hours = %2d, T1.minutes=%2d, T1.seconds=%2d, T1.hundredths=%2d\n",
           T1.Separate.Hours,
           T1.Separate.Minutes,
           T1.Separate.Seconds,
           T1.Separate.Hundredths);
  #endif
}
#endif
