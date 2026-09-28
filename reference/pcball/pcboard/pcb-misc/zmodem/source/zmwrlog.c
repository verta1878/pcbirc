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
#include <stdio.h>
#include <stdlib.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

/***************************************************************************
 ***************************************************************************/
void pascal zmWriteLog(int SendRecv, int PassFail) {
  DOSFILE LogFile;
  char    tmpStr[128];

  sprintf(tmpStr, "%c %6ld %5ld bps %4ld cps %3d errors %5ld %4d %s %d\r\n",
          ((PassFail == ZM_SUCCESS) ?  ((SendRecv == ZM_SEND) ? 'z' : 'Z') : 'E'),
          FileSize,
          Asy.ConnectSpeed,
          CPS,
          Warnings,
          0L,
          BlockLength,
          FileName,
          -1);

  if (dosfopen(getenv(DSZLOG),OPEN_RDWR|OPEN_APPEND|OPEN_DENYNONE,&LogFile) != -1) {
    dosfputs(tmpStr,&LogFile);  //lint !e534
    dosfclose(&LogFile);        //lint !e534
  }
}
