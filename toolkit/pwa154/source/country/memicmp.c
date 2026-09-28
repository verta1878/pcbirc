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


#ifdef _MSC_VER
#include <memory.h>
#else
#include <mem.h>
#endif

#include <country.h>

EXTERN_C int memicmp(const void *s1, const void *s2, size_t n) {
  int dif;
  const unsigned char *p1 = (unsigned char *) s1;
  const unsigned char *p2 = (unsigned char *) s2;

  for (; n-- >0; p1++, p2++ ) {
    dif = UpperCase[*p1] - UpperCase[*p2];
    if (dif != 0)
      return(dif);
  }
  return(0);
}
