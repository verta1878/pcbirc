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


#include <ctype.h>
#include <string.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function:  ctod2()
*
*  Desc    :  converts a text date of the form "mm-dd-yy" into an unsigned
*             integer
*
*  Notes   :  This function is *NOT* identical in behaviour to the function
*             in misc.lib that is called ctod().  Both functions return
*             the same values - but this function is limited in scope to only
*             accept dates in the form "mm-dd-yy" while the ctod() function
*             in misc.lib allows more flexible forms such as "m/d/yy".
*
*  Returns :  An unsigned integer of where the first 7 bits are the year-80,
*             the next 4 bits are the month and the last 5 bits are the day.
*
*             A value of 0xFFFF is returned on text string of "==-==-==".
*/

unsigned pascal ctod2(char *Date) {
  int Month;
  int Day;
  int Year;

  switch(Date[0]) {
    case '=': if (memcmp(&Date[1],"=-==-==",7) == 0) return(0xFFFF);
              break;
    case 'A': if (memcmp(&Date[1],"RCH",3) == 0)     return(0xFFFF);
              break;
    case 'D': if (memcmp(&Date[1],"ELETE",5) == 0)   return(0xFFFE);
              break;
    case 'O': if (memcmp(&Date[1],"FF-LINE",7) == 0) return(0xFFFF);
              break;
  }

  if (Date[2] != '-' && Date[2] != '/')
    return(0);

  if (Date[5] != '-' && Date[5] != '/')
    return(0);

  if (Date[0] < '0' || Date[0] > '1' ||
      Date[1] < '0' || Date[1] > '9' ||
      Date[3] < '0' || Date[3] > '3' ||
      Date[4] < '0' || Date[4] > '9' ||
      Date[6] < '0' || Date[6] > '9' ||
      Date[7] < '0' || Date[7] > '9')
    return(0);

  Month = (Date[0]-'0')*10 + (Date[1]-'0');
  if (Month == 0)
    Month = 1;
  else if (Month > 12)
    return(0);

  Day = (Date[3]-'0')*10 + (Date[4]-'0');
  if (Day == 0)
    Day = 1;
  else if (Day > 31)
    return(0);

  Year = (Date[6]-'0')*10 + (Date[7]-'0');
  if (Year < 80)
    Year += 100;

  return(Day + (Month << 5) + ((Year-80) << 9));
}


