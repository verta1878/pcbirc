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
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include <country.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

char * LIBENTRY dcomma(char *Buffer, double Num) {
  char *p;
  char  Decimal[10];

  sprintf(Buffer,"%lf",Num);
  p = strchr(Buffer,'.');
  if (p != NULL) {
    stripright(p,'0');
    maxstrcpy(Decimal,p,sizeof(Decimal));
    *p = 0;
  }

  commastr(Buffer,(bool) (Num < 0));
  if (Decimal[1] != 0) {
    Decimal[0] = Country.FractionSep[0];
    strcat(Buffer,Decimal);
  }

  return(Buffer);
}
