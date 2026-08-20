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


#include <stdio.h>
#include <stdlib.h>
#include <screen.h>

#ifdef DEBUG
#include <memcheck.h>
#endif

char    *errorlist[] = {
        "Error 0",
        "Invalid function number",
        "No such file or directory",
        "Path not found",
        "Too many open files",
        "Permission denied",
        "Bad file number",
        "Memory arena trashed",
        "Not enough memory",
        "Invalid memory block address",
        "Invalid environment",
        "Invalid format",
        "Invalid access code",
        "Invalid data",
        0,
        "No such device",
        "Attempted to remove current directory",
        "Not same device",
        "No more files",
        "Invalid argument",
        "Arg list too big",
        "Exec format error",
        "Cross-device link",
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        "Math argument",
        "Result too large",
        "File already exists"
};


void LIBENTRY errorexit(char *A, char *B, char *File, int Line) {
  char Str[128];

  cls();
  gotoxy(0,3);
  sprintf(Str,"FILE: %s, LINE: %d",File,Line);
  fastprint(0,0,Str,0x0F);

  if (A[0] != 0 && B[0] != 0) {
    sprintf(Str,"(%s) %s:%s",A,B,errorlist[errno]);
    fastprint(0,1,Str,0x07);
  } else if (A[0] != 0) {
    fastprint(0,1,A,0x07);
  } else {
    sprintf(Str,"%s:%s",B,errorlist[errno]);
    fastprint(0,1,Str,0x07);
  }

  exit(99);
}
