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

#pragma argsused
void LIBENTRY toggleoff(char _FAR_ *ScrnBuf) {
}
#pragma argsused
void LIBENTRY toggleon(char _FAR_ *ScrnBuf) {
}
char LIBENTRY curcolor(void) {
  return(0);
}
char LIBENTRY awherex(void) {
  return(0);
}
char LIBENTRY awherey(void) {
  return(0);
}
#pragma argsused
void LIBENTRY agotoxy(char X, char Y) {
}
#pragma argsused
void LIBENTRY asetcolor(char NewColor) {
}
#pragma argsused
int LIBENTRY colorstr(char *Str, char NewColor) {
  return(0);
}
void LIBENTRY doesccodes(void) {
}
void LIBENTRY noesccodes(void) {
}
#pragma argsused
void LIBENTRY ansi(char *Str) {
}
#pragma argsused
void LIBENTRY setlimits(char NumLines) {
}
#pragma argsused
void LIBENTRY scrollon(void _FAR_ *Buffer,void _FAR_ *Offset,int NumLines) {
}
void LIBENTRY scrolloff(void) {
}
void LIBENTRY reenablescroll(void) {
}
#pragma argsused
void LIBENTRY viewbuff(int TopLine) {
}
#pragma argsused
void LIBENTRY tagem(int X1, int Y1, int X2, int Y2, char Attr) {
}
#pragma argsused
void LIBENTRY gettagged(char _FAR_ *Str,int X1, int Y1, int X2, int Y2) {
}
