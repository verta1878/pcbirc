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


#include <pcbtools.h>

short LIBENTRY minutesleft(void) {
  return(0);
}

short LIBENTRY minutesonline(void) {
  return(0);
}

#pragma argsused
void LIBENTRY addtime(long Seconds, timereasontype Reason) {
}

#pragma argsused
void LIBENTRY subtime(long Seconds, timereasontype Reason) {
}

void LIBENTRY checkpagelen(void) {
}

#pragma argsused
int LIBENTRY openfile(char *Name) {
  return(0);
}

#pragma argsused
void LIBENTRY ascii(char *Str, int Num) {
}

#pragma argsused
void LIBENTRY asciispace(char *Str, int Num) {
}

#pragma argsused
void LIBENTRY lascii(char *Str, long Num) {
}

#pragma argsused
bool LIBENTRY substitute(char *Str, char *Search, char *Replace, int MaxLen) {
  return(FALSE);
}

#pragma argsused
void *LIBENTRY checkmalloc(unsigned Size, char *Str) {
  return(NULL);
}

#pragma argsused
void buildstr(char *Dest, ...) {
}

