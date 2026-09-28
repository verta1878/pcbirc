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

int MaxHandle = 0;

#ifndef LIB
static char * _NEAR_ LIBENTRY flags(int OpenFlags) {
  static char FlagStr[6];
  char *p = FlagStr;
  int  Temp;

  Temp = OpenFlags & 0x03;
  switch (Temp) {
    case 0: *(p++) = 'R'; break;
    case 2: *(p++) = 'R'; /* fall thru */
    case 1: *(p++) = 'W'; break;
  }
  *(p++) = '/';
  *(p++) = 'D';

  Temp = (OpenFlags & 0x70) >> 4;
  switch (Temp) {
    case 1: *(p++) = 'A'; break;
    case 2: *(p++) = 'W'; break;
    case 3: *(p++) = 'R'; break;
    case 4: *(p++) = 'N'; break;
  }
  *p = 0;

  return(FlagStr);
}


static void LIBENTRY dosopencallback(char *FileName,int OpenFlags,int Handle,int ExtendedError) {
  char Str[128];

  if (Handle == -1) {
    if (DebugLevel >= 2) {
      sprintf(Str,"OPEN FAILED: [%s] %d %s",FileName,ExtendedError,flags(OpenFlags));
      writedebugrecord(Str);
    }
  } else {
    MaxHandle = Handle;
    if (DebugLevel >= 3) {
      sprintf(Str,"OPENED: [%s] %d %s",FileName,Handle,flags(OpenFlags));
      writedebugrecord(Str);
    }
  }
}


#pragma argsused
static void LIBENTRY doscreatecallback(char *FileName,int OpenFlags,int Attributes,int Handle,int ExtendedError) {
  char Str[128];
  if (Handle == -1) {
    if (DebugLevel >= 2) {
      sprintf(Str,"CREATE FAILED: [%s] %d %s",FileName,ExtendedError,flags(OpenFlags));
      writedebugrecord(Str);
    }
  } else {
    if (DebugLevel >= 3) {
      sprintf(Str,"CREATED: [%s] %d %s",FileName,Handle,flags(OpenFlags));
      writedebugrecord(Str);
    }
  }
}


static void LIBENTRY dosdupcallback(char *FileName,int NewHandle,int OldHandle,int ExtendedError) {
  char Str[128];
  if (NewHandle == -1) {
    if (DebugLevel >= 2) {
      sprintf(Str,"DUPE FAILED: [%s] %d %d",FileName,OldHandle,ExtendedError);
      writedebugrecord(Str);
    }
  } else {
    if (DebugLevel >= 3) {
      sprintf(Str,"DUPED: [%s] %d (was %d)",FileName,NewHandle,OldHandle);
      writedebugrecord(Str);
    }
  }
}

void LIBENTRY registerdosopencb(void) {
  setdosopencallback(dosopencallback);
}

void LIBENTRY deregisterdosopencb(void) {
  setdosopencallback(NULL);
}

void LIBENTRY registerdosdupcb(void) {
  setdosdupcallback(dosdupcallback);
}

void LIBENTRY deregisterdosdupcb(void) {
  setdosdupcallback(NULL);
}

void LIBENTRY registerdoscreatecb(void) {
  setdoscreatecallback(doscreatecallback);
}

void LIBENTRY deregisterdoscreatecb(void) {
  setdoscreatecallback(NULL);
}
#endif
