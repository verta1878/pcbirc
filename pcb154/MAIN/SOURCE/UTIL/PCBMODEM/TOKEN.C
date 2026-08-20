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

#define TOKENSEPARATOR  1
#define TOKENEND        0

char static *TokenPointer;


static void near pascal removeextraspaces(char *Str) {
  char *p;

  while (1) {
    if ((p = strstr(Str,"  ")) == NULL)
      break;
    Str = p;
    while (*(++p) == ' ');
    strcpy(Str,p-1);
  }
}


static void near pascal inserttokens(char *Command) {
  char *p;

  removeextraspaces(Command);
  strupr(Command);
  for (p = Command; *p != TOKENEND; p++) {
    if ((*p == ' ' || *p == ';')) {
      if (*(p+1) != TOKENEND) {
        *p = TOKENSEPARATOR;
      } else *p = TOKENEND;
    }
  }
}


static int near pascal counttokens(char *Command) {
  int  Count;
  char *p;

  for (Count = 1, p = Command; *p != TOKENEND; p++)
    if (*p == TOKENSEPARATOR)
      Count++;

  return(Count);
}


int pascal tokenizestr(char *Command) {
  TokenPointer = Command;
  if (*Command == 0)
    return(0);

  inserttokens(Command);
  return(counttokens(Command));
}


char * pascal getnexttoken(void) {
  char *p;
  char *q;

  for (p = q = TokenPointer; *p != TOKENSEPARATOR; p++)
    if (*p == TOKENEND)
      return(q);

  *p = TOKENEND;
  TokenPointer = p+1;
  return(q);
}
