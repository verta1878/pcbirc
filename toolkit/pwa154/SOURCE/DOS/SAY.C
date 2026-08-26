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


#include <stdio.h>
#include <string.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY say(char *Str) {
  #ifdef __OS2__
/*   os2errtype Os2Error; */
  #endif

  #ifdef __OS2__
    /* THIS MAY BE ONLY A TEMPORARY WORK AROUND */
    printf("%s\n",Str);
  #else
    doswrite(1,Str,strlen(Str) POS2ERROR);
    doswrite(1,"\r\n",2 POS2ERROR);
  #endif
}
