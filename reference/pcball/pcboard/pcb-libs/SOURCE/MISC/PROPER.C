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


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <misc.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY proper(char *Str) {
  char Ch;

  strlwr(Str);
  while (*Str == ' ') Str++;
  *Str++ = (char) toupper(*Str);

  while (1) {
    Ch = *Str;
    if (Ch == 0)
      break;

    if (Ch == ' ') {
      /* check for the word " de " in the name like Lex de Azevedo */
      if (Str[1] == 'd' && Str[2] == 'e' && Str[3] == ' ') {
        Str += 3;
        continue;
      }

      /* check for the word " von " in the name like John von Neumann */
      if (Str[1] == 'v' && Str[2] == 'o' && Str[3] == 'n' && Str[4] == ' ') {
        Str += 4;
        continue;
      }

      /* check for either "ii" or "iii" as in John Henry II or John Henry III */
      if (Str[1] == 'i' && Str[2] == 'i') {
        if (Str[3] == 0 || Str[3] == ' ') {
          Str[1] = 'I';
          Str[2] = 'I';
          Str += 3;
          continue;
        }
        if (Str[3] == 'i' && (Str[4] == 0 || Str[4] == ' ')) {
          Str[1] = 'I';
          Str[2] = 'I';
          Str[3] = 'I';
          Str += 4;
          continue;
        }
      }

      /* check for "mc" as in McDonald */
      if ((Str[1] == 'm' && Str[2] == 'c'))
        Str[3] = (char) toupper(Str[3]);
      Str[1] = (char) toupper(Str[1]);
    } else if (Ch == '\'' || Ch == '-' || Ch == '.')
      Str[1] = (char) toupper(Str[1]);
    Str++;
  }
}

#ifdef TEST
void main(void) {
  char Str[80];

  gets(Str);
  proper(Str);
  puts(Str);
}
#endif
