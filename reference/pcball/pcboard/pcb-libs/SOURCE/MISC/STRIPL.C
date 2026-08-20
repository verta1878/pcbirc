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
#include <misc.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
 *
 *  Function:  stripleft()
 *
 *  Strips character Ch's from the left side of Str[]
 */

char * LIBENTRY stripleft(char *Str, char Ch) {
  char *p;

  p = Str;

  if (*p == Ch) {
    for (p++; *p != 0; p++)
      if (*p != Ch)
        break;

    if (*p != 0)
      strcpy(Str,p);
    else
      Str[0] = 0;
  }

  return(Str);
}


#ifdef TEST
#include <stdio.h>
void main(void) {
  char Str[80];

  strcpy(Str,".......Test.......");    /* show initial value of Str */
  printf("[%s]\n",Str);
  stripleft(Str,'.');                  /* strip left dots */
  printf("[%s]\n",Str);

  strcpy(Str,"..................");    /* test a string of all dots */
  stripleft(Str,'.');
  printf("[%s]\n",Str);

  strcpy(Str,"       Test       ");    /* test a string with no dots */
  stripleft(Str,'.');
  printf("[%s]\n",Str);
}
#endif
