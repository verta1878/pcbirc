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


#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
 *
 *  Function:  stripall()
 *
 *  Strips all character Ch's from the Str[] no matter where they are found
 */

char * LIBENTRY stripall(char *Str, char Ch) {
  char *p;
  char *q;

  for (p = q = Str; *q; q++) {
    if (*q != Ch) {
      *p = *q;
      p++;
    }
  }
  *p = 0;

  return(Str);
}


#ifdef TEST
#include <stdio.h>
#include <string.h>
void main(void) {
  char Str[80];

  strcpy(Str,".......Test.......");
  printf("[%s]\n",Str);

  stripall(Str,'.');
  printf("[%s]\n",Str);

  strcpy(Str,".......Test.......");
  stripall(Str,' ');
  printf("[%s]\n",Str);
}
#endif
