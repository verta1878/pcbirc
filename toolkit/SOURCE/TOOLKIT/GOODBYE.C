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


#include <stdlib.h>
#include "pcbtools.h"

#define TXT_THANKSFORCALLING   166
typedef enum {LOGON, NLOGOFF, ALOGOFF} logtype;

void LIBENTRY writeusersysfile(void);
void LIBENTRY loguseroff(logtype Type);


static void _NEAR_ LIBENTRY createkbdstuff(void) {
  char FileName[66];
  int  Handle;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  buildstr(FileName,PcbDir,"PCBSTUFF.KBD",NULL);
  if ((Handle = doscreate(FileName,OPEN_WRIT,OPEN_NORMAL POS2ERROR)) != -1) {
    doswrite(Handle,"\r\n\r\nBYE\r\n",7 POS2ERROR);
    dosclose(Handle);
  }
}



void LIBENTRY goodbye(bool UpdateUserSys) {
  if (Status.UserRecNo > 1 && PcbData.LogOffScr[0] != 0 && PcbData.LogOffAns[0] == 0 && fileexist(PcbData.LogOffScr) != 255) {
    Asy.IgnoreCDLoss = TRUE;
    displayfile(PcbData.LogOffScr,GRAPHICS|SECURITY|LANGUAGE);
  }

  displaypcbtext(TXT_THANKSFORCALLING,LFBEFORE|NEWLINE);
  printcolor(7);  /* reset the color after logging off */

  if (UpdateUserSys)
    writeusersysfile();

  writelog("Logged off in door",SPACERIGHT);
  createkbdstuff();
  loguseroff(ALOGOFF);
}
