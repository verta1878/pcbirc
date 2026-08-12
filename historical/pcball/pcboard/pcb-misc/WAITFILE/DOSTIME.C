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

#ifdef __OS2__
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

  #pragma inline

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


#ifdef __OS2__
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
  #ifdef __OS2__
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
