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


#include <misc.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

int LIBENTRY hextoint(char *Str) {
  int Result;

#if defined(_MSC_VER) || defined(__WATCOMC__)
  for (Result = 0; *Str >= '0'; Str++) {
    if (*Str <= '9') {
      Result <<= 4;
      Result += *Str - '0';
    } else {
      *Str &= 223;  /* convert to uppercase */
      if (*Str <= 'F' && *Str >= 'A') {
        Result <<= 4;
        Result += *Str - 'A' + 10;
      } else
        break;
    }
  }
#else
  Result = 0;
  while (1) {
    _AX = *(Str++);
    if (_AX < '0')
      break;
    if (_AX <= '9') {
      Result <<= 4;
      Result += _AX - '0';
    } else {
      _AX &= 223;  /* convert to uppercase */
      if (_AX <= 'F' && _AX >= 'A') {
        Result <<= 4;
        Result += _AX - 'A' + 10;
      } else
        break;
    }
  }
#endif
  return(Result);
}


#ifdef TEST

#include <stdio.h>
char *Str = "b800";

void main(void) {
  int x;

  x = hextoint(Str);
  printf("%s = %u\n",Str,x);
}
#endif
