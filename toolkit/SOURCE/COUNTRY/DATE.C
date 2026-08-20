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


#include "country.h"

#define swapchar(a,b) { Tmp = a; a = b; b = Tmp; }

char * LIBENTRY countrydate(char *DateStr) {
  char Tmp;

  switch (Country.DateFormat) {
    case 1: swapchar(DateStr[0],DateStr[3]);  /* mm-dd-yy becomes dd-mm-yy */
            swapchar(DateStr[1],DateStr[4]);
            break;
    case 2: swapchar(DateStr[0],DateStr[3]);  /* mm-dd-yy becomes dd-mm-yy */
            swapchar(DateStr[1],DateStr[4]);
            swapchar(DateStr[0],DateStr[6]);  /* dd-mm-yy becomes yy-mm-dd */
            swapchar(DateStr[1],DateStr[7]);
            break;
  }
  DateStr[2] = Country.DateSep[0];
  DateStr[5] = Country.DateSep[0];
  return(DateStr);
}


char * LIBENTRY countrydate2(char *DateStr) {
  char Tmp;

  switch (Country.DateFormat) {
    case 1: swapchar(DateStr[0],DateStr[2]);  /* mmddyy becomes ddmmyy */
            swapchar(DateStr[1],DateStr[3]);
            break;
    case 2: swapchar(DateStr[0],DateStr[2]);  /* mmddyy becomes ddmmyy */
            swapchar(DateStr[1],DateStr[3]);
            swapchar(DateStr[0],DateStr[4]);  /* ddmmyy becomes yymmdd */
            swapchar(DateStr[1],DateStr[5]);
            break;
  }
  return(DateStr);
}


char * LIBENTRY dateformat(void) {
  static char *Date = "MM-DD-YY";
  return(countrydate(Date));
}


char * LIBENTRY dateformat2(void) {
  static char *Date = "MMDDYY";
  return(countrydate2(Date));
}


char * LIBENTRY uncountrydate(char *DateStr) {
  char Tmp;

  switch (Country.DateFormat) {
    case 1: swapchar(DateStr[0],DateStr[3]);  /* dd-mm-yy becomes mm-dd-yy */
            swapchar(DateStr[1],DateStr[4]);
            break;
    case 2: swapchar(DateStr[0],DateStr[6]);  /* yy-mm-dd becomes dd-mm-yy */
            swapchar(DateStr[1],DateStr[7]);
            swapchar(DateStr[0],DateStr[3]);  /* dd-mm-yy becomes mm-dd-yy */
            swapchar(DateStr[1],DateStr[4]);
            break;
  }
  DateStr[2] = '-';
  DateStr[5] = '-';
  return(DateStr);
}


char * LIBENTRY uncountrydate2(char *DateStr) {
  char Tmp;

  switch (Country.DateFormat) {
    case 1: swapchar(DateStr[0],DateStr[2]);  /* ddmmyy becomes mmddyy */
            swapchar(DateStr[1],DateStr[3]);
            break;
    case 2: swapchar(DateStr[0],DateStr[4]);  /* yymmdd becomes ddmmyy */
            swapchar(DateStr[1],DateStr[5]);
            swapchar(DateStr[0],DateStr[2]);  /* ddmmyy becomes yymmdd */
            swapchar(DateStr[1],DateStr[3]);
            break;
  }
  return(DateStr);
}
