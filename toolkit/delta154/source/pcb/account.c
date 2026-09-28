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


#include <mem.h>
#include "dosfunc.h"
#include "pcb.h"
#include "misc.h"
#include "account.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

accountratetype AccountRates;

int LIBENTRY readaccountrates(void) {
  int File;
  int BytesRead;

  memset(&AccountRates,0,sizeof(accountratetype));

  if (fileexist(PcbData.AccountConfig) == 255)
    return(-1);

  if ((File = dosopencheck(PcbData.AccountConfig,OPEN_READ|OPEN_DENYNONE)) == -1)
    return(-1);

  BytesRead = readcheck(File,&AccountRates,sizeof(accountratetype));
  dosclose(File);

  if (BytesRead != sizeof(accountratetype))
    return(-1);

  return(0);
}


void LIBENTRY writeaccountrates(void) {
  int File;

  if (fileexist(PcbData.AccountConfig) == 255)
    File = doscreatecheck(PcbData.AccountConfig,OPEN_WRIT|OPEN_DENYWRIT,OPEN_NORMAL);
  else
    File = dosopencheck(PcbData.AccountConfig,OPEN_WRIT|OPEN_DENYWRIT);

  if (File == -1)
    return;

  writecheck(File,&AccountRates,sizeof(accountratetype));
  dosclose(File);
}
