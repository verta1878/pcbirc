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

void       *TPAstatic;
void       *TPAdynamic;
long _FAR_ *MsgReadPtr;
char _FAR_ *ConfReg;
char _FAR_ *ConfFlags;
unsigned    ConfByteLen;
char        SysName[66];

#pragma warn -par
int LIBENTRY readusersysfile(int OpenStatus) {
  return(-1);
}
#pragma warn +par

void LIBENTRY writeusersysfile(void) {
}
