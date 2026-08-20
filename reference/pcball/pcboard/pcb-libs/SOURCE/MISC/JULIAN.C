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


#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <direct.h>
  #include <dos.h>
#else
  #include <dir.h>
#endif

#include <stdio.h>
#include <string.h>
#include <system.h>
#include "misc.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif


#define MONTH 0
#define DAY   1
#define YEAR  2

#if defined(__BORLANDC__) || defined(__TURBOC__)
  extern struct ffblk DTA;   /* really declared as "char DTA[45]" in exist.c */
#elif defined(__WATCOMC__)
  extern struct find_t DTA;
  #define ff_name   name
  #define ff_ftime  wr_time
  #define ff_fdate  wr_date
  #define ff_fsize  size
  #define ff_attrib attrib
#endif

short Days[2][12] = {{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
                     {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};


/********************************************************************
*
*  function datetojulian()
*
*    parameters: a pointer to a date string delimited by anything non-numeric
*                i.e the following are valid:  01-01-87
*                                              2/19/64
*                                              3 5 86
*
*                if the year is less than 100 then 1900 is added to it to
*                ensure that leap year is dealt with properly.
*
*    return:     a integeter value that arithmetic can be performed on for
*                calculations such as "days between dates" and arithmetic
*                sorting of dates so that 2/1/86 -DOES- come before 1/1/87.
*
*   **** NOTE:   when I originally wrote this I had it returning a LONG value
*                rather than an INT.  The LONG is necessary in order to
*                use dates before the year 1900 and after the year 1999.  For
*                my purposes that wasn't necessary and I didn't like using up
*                4 bytes when I could us up only 2 (I store a lot of dates!).
*
*                You can modify the routine to return a LONG by removing the
*                line below that says "SEE NOTE ABOVE" and then changing all
*                references to iretval to lretval and changing the function
*                declaration to "long" instead of "int".
*
*********************************************************************/

void LIBENTRY breakdate(unsigned short *Numbers, char *DateStr) {
  unsigned short register *p;  /*  p  will be pointed to *Numbers  */
  char           register *q;  /*  q  will be pointed to *DateStr  */
  char           done;
  char           limit;
  int            which;

  p = Numbers;

  *(p  ) = 0;        /*  initialize all Numbers[] to 0  */
  *(p+1) = 0;
  *(p+2) = 0;

  limit = 2;       /* limit the first two fields to 2 digits each */
  which = 1;

  /* done is how many digits in a field we have done */
  /* which is which field we are working on (1=month,2=day,3=year) */
  /* limit is the limit of digits within the field */

  for (q=DateStr, done = 0; which <= 3; q++) {
    if (*q < '0' || *q > '9') {  /*  if the character is NOT a digit      */
      if (*q == 0)               /*    if the character is 0              */
        break;                   /*      then we're OUTTA here!           */
      which++;                   /*    increment to the next field        */
      if (which >= 3) {          /*    if it's the YEAR slot,             */
        limit = 4;               /*      then set the limit to 4 digits   */
        if (which > 3)           /*    if we went too far                 */
          break;                 /*      then get OUTTA here!             */
      }
      p++;                       /*    and  advance to next Temp[] slot   */
      done = 0;                  /*    and indicate 0 characters done     */
      continue;                  /*    then loop back to the top!         */
    }                            /*                                       */
    if (done == limit) {         /*  if we've done up to the limit        */
      which++;                   /*    increment which field we're doing  */
      if (which >= 3) {          /*    if it's the YEAR slot,             */
        limit = 4;               /*      then set the limit to 4 digits   */
        if (which > 3)           /*    if we went too far                 */
          break;                 /*      then get OUTTA here!             */
      }
      done = 0;                  /*    then indicate 0 characters done    */
      p++;                       /*    and  advance to next Temp[] slot   */
    }                            /*                                       */
    if (done > 0)                /*  if we've done any chars already      */
      *p *= (short) 10;          /*    then multiply it by 10             */
    *p += (short) (*q-'0');      /*  add numeric value of char to Temp[x] */
    done++;                      /*  indicate we did another character    */
  }

  /* at this point Numbers[MONTH], Numbers[DAY] and Numbers[YEAR] are set up */
}


unsigned short LIBENTRY datetojulian(char *DateStr) {
  unsigned short Temp[3];
  long           lretval;
  unsigned short iretval;

  breakdate(Temp,DateStr);
  if (Temp[MONTH] == 0 || Temp[DAY] == 0)
    return(0);

  /* only 2-digits are checked for, so verify that the date is >= 79    */
  /* indicating 1979 to 1999, or less than 79 indicating 2000 to 2078   */
  /*                                                                    */
  /* WARNING!  The date range of 1979-1999 and 2000-2078 was chosen     */
  /*           because dates beyond 6/6/2079 are invalid due to the use */
  /*           of unsigned integers (6/5/2079 equals 65535).            */

  if (Temp[YEAR] <= 99) {
    if (Temp[YEAR] < 79)
      Temp[YEAR] += (short) 2000;
    else
      Temp[YEAR] += (short) 1900;
  }

  lretval = 36525L * Temp[YEAR];
  if ((lretval % 100) == 0 && Temp[MONTH] < 3)
    lretval--;

  iretval = (unsigned short) ((lretval - (1900 * 36525L)) / 100);    /****  SEE NOTE ABOVE ****/
  iretval += (unsigned short) (Temp[DAY] + Days[0][Temp[MONTH]-1]);
  return(iretval);
}


/********************************************************************
*
*  function juliantodate()
*
*    parameters: an integer in the form of a julian date as determined by
*                the above function.  NOTE: the above function is returning
*                an INT which gives a range of 00-99 (or 1900 to 1999).  The
*                above routine may be changed to return a LONG and this
*                routine may be modified to take a LONG as its parameter.
*
*    return:     a pointer to a string in the form "0X-0X-0X".
*
*   **** NOTE:   To modify this routine to use LONG integers and thus use the
*                full range of dates, simply change the declaration of JD to
*                a LONG and remove the "1900" from the sprintf statement.
*
*********************************************************************/

char * LIBENTRY juliantodate(unsigned short JD) {
  char static   ReturnStr[14];
  bool          Leap;
  unsigned long Temp;
  unsigned int  Counter;
  unsigned int  Year;
  unsigned int  Month;
  unsigned int  Day;

  if (JD == 0) {
    strcpy(ReturnStr,"00-00-00");
    return(ReturnStr);
  }

  Year  = (unsigned short) ((long) 100*JD/36525L);   /* compute the year */
  Temp  = (long) Year * 36525L;
  JD   -= (unsigned short) ((long) Temp/100);  /* remove the year from the julian date */

  if (Year == 0 || Year == 1900)       /* 1900 is not a "leap century */
    Leap = 0;
  else
    Leap  = (bool) (Temp % 100 == 0 ? 1 : 0); /* find out if it is a leap year */

  if (Leap)                            /* if so, add 1 to the julian date */
    JD++;

  for (Month = Counter = 0; Counter < 12; Counter++)
    if (Days[Leap][Counter] < JD)
      Month = Counter;                 /* scan through Days[][] searching */
                                       /* for the last "complete" month   */

  Day = JD - Days[Leap][Month];        /* subtract the days for that month */

  if (Year >= 100)
    Year -= 100;

  sprintf(ReturnStr,"%02d-%02d-%02d",Month+1,Day,Year);
  ReturnStr[8] = 0;
  return(ReturnStr);
}


unsigned short LIBENTRY getjuliandate(void) {
  sysdatetype    SysDate;
  long           lretval;
  unsigned short iretval;

  getsysdate(&SysDate);

  lretval = 36525L * SysDate.Year;
  if ((lretval % 100) == 0 && SysDate.Month < 3)
    lretval--;

  iretval  = (unsigned short) ((lretval - (1900 * 36525L)) / 100);
  iretval += (unsigned short) (SysDate.Day + Days[0][SysDate.Month-1]);
  return(iretval);
}


#ifndef LIB
long LIBENTRY getfiledatetime(char *FileName) {
  unsigned Day;
  unsigned Year;
  unsigned register Temp;
  long     lretval;

  if (fileexist(FileName) == 255)
    return(0);

  Temp  = DTA.ff_fdate;
  Year  = ((Temp & 0xFE00) >> 9) + 1980;     /* year = 0 means 1980 */
  Day   =   Temp & 0x001F;                   /* get date portion    */
  Temp  =  (Temp & 0x01E0) >> 5;             /* get month portion   */

  lretval = 36525L * Year;
  if ((lretval % 100) == 0 && Temp < 3)  /* remember Temp = Month right now */
    lretval--;

  lretval = (lretval - (1900 * 36525L)) / 100;
  lretval += Day + Days[0][Temp-1];  /* lretval now equals the julian date */

  lretval *= 2048;      /* make room for the time (hours & minutes)  */

  Temp = DTA.ff_ftime;

  return(lretval + (Temp >> 11)*60 + ((Temp >> 5) & 0x003F));
}
#endif


#ifdef TEST
void main(int argc, char **argv) {
  unsigned JD;

  if (argc < 2)
    return;

  JD = datetojulian(argv[1]);
  printf("%s:  Julian Value = %u  Converted Back = %s",argv[1],JD,juliantodate(JD));
}
#endif
