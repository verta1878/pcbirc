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
#include <misc.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

enum { MONTH, DAY, YEAR };

unsigned LIBENTRY ctod(char *Date) {
  short  Temp[3];
  short *p;       /*  p  will be pointed to Temp  */
  char  *q;       /*  q  will be pointed to Date  */
  char done;

  switch (Date[0]) {
    case '0': if (memcmp(&Date[1],"0-00-00",7) == 0)   return(1 + (1 << 5) + 0);  /* change to 01-01-80 */
              break;
    case 'A': if (memcmp(&Date[1],"RCH",3) == 0)     return(-1);
              break;
    case 'D': if (memcmp(&Date[1],"ELETE",5) == 0)   return(-2);
              break;
    case 'O': if (memcmp(&Date[1],"FF-LINE",7) == 0) return(-1);
              break;
  }

  p = Temp;
  *(p  ) = 0;        /*  initialize all of Temp[] to 0  */
  *(p+1) = 0;
  *(p+2) = 0;

  for (q=Date, done = 0; p < &Temp[3]; ) {
    if (*q < '0' || *q > '9') {  /*  if the character is NOT a digit      */
      if (*q == 0)               /*    if the character is 0              */
        break;                   /*      then we're OUTTA here!           */
      q++;                       /*    else advance to next character     */
      p++;                       /*    and  advance to next Temp[] slot   */
      done = 0;                  /*    and indicate 0 characters done     */
      continue;                  /*    then loop back to the top!         */
    }                            /*                                       */
    if (done > 1) {              /*  if we've done more than 1 char       */
      done = 0;                  /*    then indicate 0 characters done    */
      p++;                       /*    and  advance to next Temp[] slot   */
      if (p >= &Temp[3])         /*    if we went too far                 */
        break;                   /*      then get OUTTA here!             */
    }                            /*                                       */
    if (done > 0)                /*  if we've done any chars already      */
      *p *= (short) 10;          /*    then multiply it by 10             */
    *p += (short) (*q-'0');      /*  add numeric value of char to Temp[x] */
    q++;                         /*  advance to next character            */
    done++;                      /*  indicate we did another character    */
  }
  /*  at this point Temp[MONTH], Temp[DAY] and Temp[YEAR] are set up  */

  if (Temp[MONTH] < 1 || Temp[MONTH] > 12)
    return(0);

  if (Temp[DAY] < 1 || Temp[DAY] > 31)
    return(0);

  if (Temp[YEAR] < 0 || Temp[YEAR] > 99)
    return(0);

  if (Temp[YEAR] < 80 && Temp[YEAR] >= 0)
    Temp[YEAR] += (short) 100;

  return(Temp[DAY] + (Temp[MONTH] << 5) + ((Temp[YEAR]-80) << 9));
}
