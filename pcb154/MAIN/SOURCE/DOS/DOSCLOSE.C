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


#include "project.h"
#pragma hdrstop

#ifndef LIB
static void LIBENTRY dosclosecallback(char *FileName,int Handle,int ExtendedError) {
  char Str[80];

  if (ExtendedError != 0) {
    sprintf(Str,"Error Closing: [%s] %d %d",FileName,Handle,ExtendedError);
    writedebugrecord(Str);
  } else {
    if (DebugLevel >= 4) {
      sprintf(Str,"CLOSED: [%s] %d %d",FileName,Handle,ExtendedError);
      writedebugrecord(Str);
    }
  }
}

void LIBENTRY registerdosclosecb(void) {
  setdosclosecallback(dosclosecallback);
}

void LIBENTRY deregisterdosclosecb(void) {
  setdosclosecallback(NULL);
}
#endif
