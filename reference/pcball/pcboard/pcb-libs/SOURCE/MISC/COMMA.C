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
#ifdef DEBUG
#include <memcheck.h>
#endif

char * LIBENTRY commastr(char *Buffer, bool Negative) {
  int   x;
  char *p;
  char *q;

  x = strlen(Buffer);

  /* point p to the end of the buffer */
  p = &Buffer[x];

  if (Negative)  /* is the number negative? */
    x--;         /* yes, make number of digits one less ('-' was counted) */

  /* point q to the end of the buffer plus enough bytes to hold the commas */
  q = p + (x-1) / 3;

  *q-- = *p--;          /* copy the NULL terminator */

  while (p != q) {
    *q-- = *p--;        /* copy the next three bytes, one byte at a time */
    *q-- = *p--;
    *q-- = *p--;
    *q-- = ',';         /* now add the comma into the string */
  }

  return(Buffer);
}


char * LIBENTRY comma(char *Buffer, long Num) {
  lascii(Buffer,Num);
  return(commastr(Buffer,(bool) (Num < 0)));
}


char * LIBENTRY ucomma(char *Buffer, unsigned long Num) {
  ultoa(Num,Buffer,10);
  return(commastr(Buffer,FALSE));
}


#ifdef TEST
void main(void) {
  char Str[80];

  puts(comma(Str,1L));
  puts(comma(Str,12L));
  puts(comma(Str,123L));
  puts(comma(Str,1234L));
  puts(comma(Str,12345L));
  puts(comma(Str,123456L));
  puts(comma(Str,1234567L));
  puts(comma(Str,12345678L));
  puts(comma(Str,123456789L));
  puts(comma(Str,1234567890L));

  puts(comma(Str,-1L));
  puts(comma(Str,-12L));
  puts(comma(Str,-123L));
  puts(comma(Str,-1234L));
  puts(comma(Str,-12345L));
  puts(comma(Str,-123456L));
  puts(comma(Str,-1234567L));
  puts(comma(Str,-12345678L));
  puts(comma(Str,-123456789L));
  puts(comma(Str,-1234567890L));
}
#endif
