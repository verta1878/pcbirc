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


#include "wild.h"

// routine to create a filespec of the form "FFFFFFFFEEE" out of a filename,
// where no dot is present and spaces are used to pad the fields

void LIBENTRY makeidxname(char *FileName, char *IdxStr) {
  char *p = FileName;
  char *q = IdxStr;
  int   Len;

  for (Len = 8; Len > 0; Len--) {
    if (*p == '.') {
      do {              // fill the rest of the name field with spaces
        *q++ = ' ';
        Len--;
      } while (Len > 0);
      p++;
      goto extension;
    }
    if (*p == 0) {
      Len += 3;  // fill the rest of the name *plus* the extension with spaces
      do {
        *q++ = ' ';
        Len--;
      } while (Len > 0);
      return;
    }
    *q++ = *p++;
  }

  if (*p == '.')
    p++;

  extension:
  for (Len = 3; Len > 0; Len--) {
    if (*p == 0) {
      do {              // fill the rest of the extension with spaces
        *q++ = ' ';
        Len--;
      } while (Len > 0);
      return;
    }
    *q++ = *p++;
  }
}


// Create a criteria string of the form "FFFFFFFEEE" where wildcards are all
// represented by ?'s.  A string such as FILE*.* becomes FILE??????? with no
// dot or null terminator.

void LIBENTRY criteria(char *FileName, char *Criteria) {
  char *p;
  char *q;

  for (p = FileName, q = Criteria; *p && q < &Criteria[11]; p++) {
    switch(*p) {
      case '*': if (q < &Criteria[8]) {
                  for (; q < &Criteria[8]; q++)
                    *q = '?';
                } else if (q < &Criteria[11]) {
                  for (; q < &Criteria[11]; q++)
                    *q = '?';
                }
                break;
      case '.': while (q < &Criteria[8])
                  *q++ = ' ';
                break;
      default : *q = *p;
                q++;
                break;
    }
  }

  while (q < &Criteria[11])
    *q++ = ' ';
}


bool LIBENTRY wildmatch(char *FileName, char *Criteria) {
  char  WildName[12];
  char *p = Criteria;
  char *q = WildName;
  int   Len;

  criteria(FileName,WildName); // turn the filename into the wildcard format

  for (Len = 11; Len > 0; Len--, p++, q++) {
    if (*p == '?')
      continue;
    if (*p != *q)
      return(FALSE);
  }
  return(TRUE);
}


bool LIBENTRY comparewildcards(char *Criteria, char *FileName) {
  char *p = Criteria;
  char *q = FileName;
  int   Len;

  for (Len = 11; Len > 0; Len--, p++, q++) {
    if (*p == '?')
      continue;
    if (*p != *q)
      return(FALSE);
  }
  return(TRUE);
}


#ifdef TEST
#include <stdio.h>

void test(char *Str) {
  char IdxStr[12];
  char Criteria[12];

  IdxStr[11] = Criteria[11] = 0;
  makeidxname(Str,IdxStr);
  criteria(Str,Criteria);
  printf("%-12.12s = [%s] [%s]\n",Str,IdxStr,Criteria);
}


void test2(char *FileName, char *Match) {
  char Criteria[12];

  Criteria[11] = 0;
  criteria(Match,Criteria);
  printf("%-12.12s == %-12.12s ? %s\n",FileName,Match,(wildmatch(FileName,Criteria) ? "TRUE" : "FALSE"));
}


void main(void) {
  test("file");
  test("file.txt");
  test("1.2");
  test("1.22");
  test("1.222");
  test("11111111");
  test("11111111.");
  test("11111111.2");
  test("11111111.222");
  test("1.*");
  test("1*.*");
  test("???.*");
  test("1*.222");

  getch();

  test2("file","file*.*");
  test2("file.txt","file*.*");
  test2("file.txt","file*");
  test2("file.txt","????.txt");
  test2("filename.txt","????.txt");
}
#endif
