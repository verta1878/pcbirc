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


#include <string.h>
#include <process.h>
#include "screen.h"
#include "dosfunc.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY errorexittodos(char *Str) {
  mysound(1000,30);
  cls();
  fastprint(0,0,Str,0x0F);
  fastprint(0,1,"Exiting to DOS!",0x0F);
  gotoxy(0,3);
  setcursor(CUR_NORMAL);

  #ifndef LIB
  {
    int File;
    #ifdef __OS2__
      os2errtype Os2Error;
    #endif

    /* write ENDPCB to allow the system to drop out of BOARD.BAT */
    if ((File = doscreate("ENDPCB",OPEN_WRIT,OPEN_NORMAL POS2ERROR)) != -1) {
      writecheck(File,"ABNORMAL EXIT\r\n",15);
      writecheck(File,Str,strlen(Str));
      writecheck(File,"\r\n",2);
      dosclose(File);
    }
  }
  #endif
  mydelay(300);
  exit(99);
}
