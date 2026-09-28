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


#ifndef HPP_SESSION
#define HPP_SESSION

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

enum {START_WINDOWED=0,START_MINIMIZED,START_FULLSCREEN,START_DEFAULT};

int LIBENTRY startsession(char *ProgName,
                          char *Params,
                          char *Title,
                          char *WorkDir,
                          int   Wait,  /* 0=no wait, -1=wait forever, else wait # of 1/1000 seconds */
                          char  StartType,
                          int  *SidReturn,
                          char *SettingsFile,
                          int   PriorityDelta,
                          bool  MinimizeSelf,
                          bool  MinimizeWindow);

void LIBENTRY setvisibility(bool Visible);
int  LIBENTRY isminimized(void);

#endif  /* HPP_SESSION */
