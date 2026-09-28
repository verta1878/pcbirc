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
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif


/***************************************************************************
 ***************************************************************************/
void pascal zmPutStr(char *Str) {
  int  count;
  char *p;

  for (p = Str, count = 0; *p; p++) {
    switch (*p) {
       case 0xDE : zmSpoolStr(Str,count);
                   count = 0;
                   Str = p + 1;
                   mydelay(100);
                   continue;
       case 0xDD : zmSpoolStr(Str,count);
                   count = 0;
                   Str = p + 1;
                   /*** Send BREAK -- Does nothing right now **************/
                   /*******************************************************
                    *** According to David, a BREAK isn't used by many ****
                    *** pieces of software. *******************************/
                   continue;
    }
    count++;
  }
  zmSpoolStr(Str,count);
}
