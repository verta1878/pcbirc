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


#include "project.h"
#pragma hdrstop

#define TOKENSEPARATOR  1
#define TOKENEND        0

char static *TokenPointer;


static bool _NEAR_ LIBENTRY nonstop(void) {
  char *p;
  char *q;
  bool FirstChar;

  for (p = TokenPointer, FirstChar = TRUE; *p != TOKENEND; p++) {
    if (FirstChar) {
      q = p;
      if (*q++ == 'N')           /* is the first letter 'N'? */
        if (*q++ == 'S')         /* the second letter 'S'?   */
          switch(*q) {           /* and finally a separator? */
            case TOKENEND      : /* go here if "NS" was the last parameter */
                                 /* get rid of it by chopping off the NS   */
                                 if (p > TokenPointer)
                                   p--;
                                 *p = TOKENEND;
                                 return(TRUE);
            case TOKENSEPARATOR: /* go here if "NS" is in the middle */
                                 /* get rid of it by moving the rest down */
                                 /* if NS == TokenPointer then move q forward */
                                 /* one more - past the TOKENSEPARATOR, else */
                                 /* back p up by one to start the copy process */
                                 if (p > TokenPointer)
                                   p--;
                                 else
                                   q++;
                                 memcpy(p,q,strlen(q)+1);
                                 return(TRUE);
          }
    }
    /* next time around is a FirstChar if this character is a SEPARATOR */
    FirstChar = (bool) (*p == TOKENSEPARATOR);
  }
  return(FALSE);
}


static void _NEAR_ LIBENTRY removeextraspaces(char *Str) {
  char *p;

  while (1) {
    if ((p = strstr(Str,"  ")) == NULL)
      break;
    Str = p;
    while (*(++p) == ' ');
    strcpy(Str,p-1);
  }
}


static void _NEAR_ LIBENTRY inserttokens(char *Command) {
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


static int _NEAR_ LIBENTRY counttokens(char *Command) {
  int  Count;
  char *p;

  for (Count = 1, p = Command; *p != TOKENEND; p++)
    if (*p == TOKENSEPARATOR)
      Count++;

  return(Count);
}


int LIBENTRY tokenizestr(char *Command) {
  TokenPointer = Command;
  if (*Command == 0)
    return(0);

  inserttokens(Command);
  return(counttokens(Command));
}


int LIBENTRY tokenize(char *Command) {
  int Count;

  if ((Count = tokenizestr(Command)) == 0)
    return(0);

  /* The tokenize() function is used to scan answers to input functions which */
  /* may include the "NS" directive to perform a function in non-stop mode.   */
  /* Check for nonstop mode, and remove the "NS" if found, decrementing the   */
  /* number of tokens found by one.  Reset the line counters and display      */
  /* routines by caling startdisplay().                                       */

  if (nonstop()) {
    #ifndef LIB
      Display.NonStopDuringCmd = TRUE;
    #endif
    startdisplay(FORCENONSTOP);
    if (*Command == TOKENEND)  /* was "NS" the ONLY parameter? */
      return(0);
    return(Count-1);
  } else {
    #ifndef LIB
      Display.NonStopDuringCmd = FALSE;
    #endif
    startdisplay(NOCHANGE);
  }
  return(Count);
}


char * LIBENTRY getnexttoken(void) {
  char *p;
  char *q;

  for (p = q = TokenPointer; *p != TOKENSEPARATOR; p++)
    if (*p == TOKENEND)
      return(q);

  *p = TOKENEND;
  TokenPointer = p+1;
  return(q);
}


void LIBENTRY peekatnexttoken(char *Buffer, int MaxLen) {
  char *p;

  for (p = TokenPointer; *p != TOKENSEPARATOR && *p != TOKENEND && MaxLen; p++, Buffer++, MaxLen--)
    *Buffer = *p;
  *Buffer = TOKENEND;
}

void LIBENTRY savetokenpointer(char **Pointer) {
  *Pointer = TokenPointer;
}

void LIBENTRY restoretokenpointer(char **Pointer) {
  TokenPointer = *Pointer;
}
