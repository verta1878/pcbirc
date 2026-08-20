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

void LIBENTRY checkdisplaystatus(void) {
}

#pragma argsused
void LIBENTRY startdisplay(int Action) {
}

#pragma argsused
void LIBENTRY printcom(char *Str) {
}

void LIBENTRY bell(void) {
}

#pragma argsused
int LIBENTRY sendtoprinter(char *Str, int StrLen) {
  return(-1);
}

#pragma argsused
void LIBENTRY print(char *Str) {
}

void LIBENTRY printcls(void) {
}

#pragma argsused
void LIBENTRY moreprompt(int Type) {
}

void LIBENTRY newline(void) {
}

void LIBENTRY freshline(void) {
}

#pragma argsused
void LIBENTRY println(char *Str) {
}

#pragma argsused
void LIBENTRY printcolor(int ColorNum) {
}

void LIBENTRY printdefcolor(void) {
}

#pragma argsused
void LIBENTRY backupdestructive(int NumColumns) {
}

#pragma argsused
void LIBENTRY backupcleareol(int NumColumns) {
}

#pragma argsused
void LIBENTRY backup(int NumColumns) {
}

#pragma argsused
void LIBENTRY forward(int NumColumns) {
}

void LIBENTRY cleareol(void) {
}

#pragma argsused
void LIBENTRY showmessage(char *Msg, char Color) {
}

void LIBENTRY checkforansi(void) {
}

#pragma argsused
void LIBENTRY movecursor(int X, int Y) {
}
