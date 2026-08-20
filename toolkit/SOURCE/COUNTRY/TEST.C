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
#include <screen.h>
#include <country.h>

#define NUMELEMENTS 9
#define LENGTH      10


char Array[NUMELEMENTS][LENGTH] = {
  "ögly",
  "Äycle",
  "êchî 1",
  "Apple",
  "Ugly",
  "Echì 2",
  "Cycle",
  "épple",
  "Echo 3"
};

char Sorted[NUMELEMENTS][LENGTH];

char BmTable[256];


int compare(const void *a, const void *b) {
  return(memfcmp(a,b,LENGTH));
}


void main(int argc, char **argv) {
  char Str[80];
  char Upper[80];
  char Lower[80];
  int  X;
  int  Y;

  getcountryspecs(atoi(argv[1]),atoi(argv[2]));

  printf("Country %d - Code Page %d\n\n",CurrentCountry,CodePage);

/*strcpy(Str,"abcdefghijklmnopqrstuvwxyz ÅÇÉÑÖÜáàâäãåçìîïñóòõ†°¢£§∆–’‰ÁÏ");*/
  strcpy(Str,"ÅÇÉÑÖÜáàâäãåçìîïñóòõ†°¢£§∆–’‰ÁÏ˝˛");
  puts(Str);

  strcpy(Upper,Str);
  puts(strupr(Upper));

  strcpy(Lower,Upper);
  puts(strlwr(Lower));

  if (stricmp(Str,Lower) != 0)
    puts("stricmp(): Str != Lower");

  if (stricmp(Str,Upper) != 0)
    puts("stricmp(): Str != Upper");

  if (stricmp(Upper,Lower) != 0)
    puts("stricmp(): Upper != Lower");

  if (strnicmp(Str,Lower,strlen(Str)) != 0)
    puts("strnicmp(): Str != Lower");

  if (strnicmp(Str,Upper,strlen(Str)) != 0)
    puts("strnicmp(): Str != Upper");

  if (strnicmp(Upper,Lower,strlen(Str)) != 0)
    puts("strnicmp(): Upper != Lower");

  if (memicmp(Str,Lower,strlen(Str)) != 0)
    puts("memicmp(): Str != Lower");

  if (memicmp(Str,Upper,strlen(Str)) != 0)
    puts("memicmp(): Str != Upper");

  if (memicmp(Upper,Lower,strlen(Str)) != 0)
    puts("memicmp(): Upper != Lower");

  printf("\n%s\n",comma(Str,123456789L));

  for (X = 0; X < NUMELEMENTS; X++)
    strcpy(Sorted[X],Array[X]);

  qsort(Sorted,NUMELEMENTS,LENGTH,compare);

  printf("\n"
         "Unsorted           Sorted\n"
         "--------           ------\n");

  for (X = 0; X < NUMELEMENTS; X++)
    printf("%-19s%s\n",Array[X],Sorted[X]);

  printf("\n"
         "Testing boyer-moore search routines\n");

  for (X = 0; X < NUMELEMENTS; X++) {
    strupr(Array[X]);
    maketable(BmTable,Array[X],(char) strlen(Array[X]));
    printf("Searching for \"%s\" found:%*s",Array[X],LENGTH-strlen(Array[X]),"");
    for (Y = 0; ((Y = bmisearch((char *) Sorted + Y,NUMELEMENTS*LENGTH,BmTable,Array[X],(char) strlen(Array[X]))) != 0); )
//      printf((char *) Sorted + Y - strlen(Array[X]));
      printf((char *) Sorted + Y - 1);
    printf("\n");
  }

  datestr(Str);
  countrydate(Str);
  printf("\nToday's date is %s (format %d)\n",Str,Country.DateFormat);
}
