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
#include <misc.h>
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
  if (Decimal[1] != 0)
    strcat(Buffer,Decimal);

  return(Buffer);
}

#ifdef TEST
void main(void) {
  char Str[80];

  puts(dcomma(Str,1.));
  puts(dcomma(Str,12.));
  puts(dcomma(Str,123.));
  puts(dcomma(Str,1234.));
  puts(dcomma(Str,12345.));
  puts(dcomma(Str,123456.));
  puts(dcomma(Str,1234567.));
  puts(dcomma(Str,12345678.));
  puts(dcomma(Str,123456789.));
  puts(dcomma(Str,1234567890.));

  puts(dcomma(Str,-1.));
  puts(dcomma(Str,-12.));
  puts(dcomma(Str,-123.));
  puts(dcomma(Str,-1234.));
  puts(dcomma(Str,-12345.));
  puts(dcomma(Str,-123456.));
  puts(dcomma(Str,-1234567.));
  puts(dcomma(Str,-12345678.));
  puts(dcomma(Str,-123456789.));
  puts(dcomma(Str,-1234567890.));

  puts(dcomma(Str,-9.87654321));
  puts(dcomma(Str,-98.7654321));
  puts(dcomma(Str,-987.654321));
  puts(dcomma(Str,-9876.54321));
  puts(dcomma(Str,-98765.4321));
  puts(dcomma(Str,-987654.321));
  puts(dcomma(Str,-9876543.21));
  puts(dcomma(Str,-98765432.1));
  puts(dcomma(Str,-987654321.));

}
#endif
