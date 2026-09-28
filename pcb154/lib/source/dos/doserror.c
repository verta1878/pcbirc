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


#ifdef _MSC_VER
#include <memory.h>
#else
#include <mem.h>
#endif

#include <string.h>
#include <stdio.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMBER_OF_ERROR_STRINGS 89

char * DosExtendedErrorStr[NUMBER_OF_ERROR_STRINGS] = {
/*  0 */ "Error 0",
/*  1 */ "Invalid function number",
/*  2 */ "File not found",
/*  3 */ "Path not found",
/*  4 */ "Too many open files",
/*  5 */ "Access denied",
/*  6 */ "Bad file number",
/*  7 */ "Memory arena trashed",
/*  8 */ "Not enough memory",
/*  9 */ "Invalid memory block address",
/* 10 */ "Invalid environment",
/* 11 */ "Invalid format",
/* 12 */ "Invalid access code",
/* 13 */ "Invalid data",
/* 14 */ "",
/* 15 */ "Invalid drive",
/* 16 */ "Attempted to remove current directory",
/* 17 */ "Not same device",
/* 18 */ "No more files",
/* 19 */ "Write protect fault",
/* 20 */ "Unknown unit",
/* 21 */ "Drive not ready",
/* 22 */ "Unknown command",
/* 23 */ "CRC error",
/* 24 */ "Bad request",
/* 25 */ "Seek error",
/* 26 */ "Unknown media type",
/* 27 */ "Sector not found",
/* 28 */ "Printer out of paper",
/* 29 */ "Write fault",
/* 30 */ "Read fault",
/* 31 */ "General failure",
/* 32 */ "Sharing violation",
/* 33 */ "Lock violation",
/* 34 */ "Invalid disk change",
/* 35 */ "FCB unavailable",
/* 36 */ "Sharing buffer overflow",
/* 37 */ "unknown error",
/* 38 */ "Unable to complete file operation",
/* 39 */ "Possible disk full condition",             /* I made this one up! */
/* 40 */ "Possible read past eof",                   /* I made this one up! */
/* 41 */ "Insufficient memory for file operation",   /* I made this one up! */
/* 42 */ "",
/* 43 */ "",
/* 44 */ "",
/* 45 */ "",
/* 46 */ "",
/* 47 */ "",
/* 48 */ "",
/* 49 */ "",
/* 50 */ "Network Error",
/* 51 */ "Network Error",
/* 52 */ "Network Error",
/* 53 */ "Network Error",
/* 54 */ "Network Error",
/* 55 */ "Network Error",
/* 56 */ "Network Error",
/* 57 */ "Network Error",
/* 58 */ "Network Error",
/* 59 */ "Network Error",
/* 60 */ "Network Error",
/* 61 */ "Network Error",
/* 62 */ "Network Error",
/* 63 */ "Network Error",
/* 64 */ "Network Error",
/* 65 */ "Network Error",
/* 66 */ "Network Error",
/* 67 */ "Network Error",
/* 68 */ "Network Error",
/* 69 */ "Network Error",
/* 70 */ "Network Error",
/* 71 */ "Network Error",
/* 72 */ "Network Error",
/* 73 */ "",
/* 74 */ "",
/* 75 */ "",
/* 76 */ "",
/* 77 */ "",
/* 78 */ "",
/* 79 */ "",
/* 80 */ "File Exists",
/* 81 */ "",
/* 82 */ "Cannot make directory entry",
/* 83 */ "Fail on INT 24h",
/* 84 */ "Network Error",
/* 85 */ "Network Error",
/* 86 */ "Network Error",
/* 87 */ "Network Error",
/* 88 */ "Network Error"};


/********************************************************************
*
*  Function:  doserror()
*
*  Desc    :  Should be called after an error has occured where the
*             ExtendedError variable has been set - such as dosopen().
*
*  Returns :  if Str is blank it simply returns the Extended Error message
*             otherwise it returns Str+": "+Extended Error message
*/

char * LIBENTRY doserror(char *Str, int ExtendedError) {
  static char Buffer[80];

  if (Str != NULL) {
    strcpy(Buffer,Str);
    strcat(Buffer,": ");
    if (ExtendedError < NUMBER_OF_ERROR_STRINGS)
      strcat(Buffer,DosExtendedErrorStr[ExtendedError]);
    else
      strcat(Buffer,"unknown error");
    return(Buffer);
  } else if (ExtendedError < NUMBER_OF_ERROR_STRINGS)
    return(DosExtendedErrorStr[ExtendedError]);
  else
    return("unknown error");
}
