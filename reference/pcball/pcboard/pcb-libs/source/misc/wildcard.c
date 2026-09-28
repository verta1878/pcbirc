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
#include <stdio.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

#pragma inline


static void near pascal wildcard(char far *Field, int Len) {
  asm  Push Es
  asm  Xor  Si,Si                 /* Determine length of Field[] and */
  asm  Les  Bx,Field              /* store in Si                     */
X1:
  asm  Cmp  byte ptr Es:[Bx+Si],0
  asm  Je   X2
  asm  Inc  Si
  asm  Jmp  Short X1
X2:

  asm  And  Si,Si
  asm  Jz   J4

  asm  Dec  Si
  asm  Mov  Di,Si
  asm  Cmp  byte ptr Es:[Bx+Di],'*'  /* if it's a "*" then convert it and */
  asm  Jne  J1                       /* the rest of the string to "?"     */
  asm  Mov  Al,'?'
  asm  Jmp  J2
J1:
  asm  Inc  Di                       /* otherwise move forward one and fill */
  asm  Mov  Al,' '                   /* the rest with spaces                */
J2:

L1:
  asm  Cmp  Di,Len
  asm  Jg   J3
  asm  Mov  Es:[Bx+Di],Al
  asm  Inc  Di
  asm  Jmp  L1
J3:
  asm  Add  Bx,Len
  asm  Mov  byte ptr Es:[Bx],0       /* make sure the last element is NULL */
J4:
  asm  Pop  Es
}


void pascal formatwild(char Name[]) {
  char F[13],E[4];
  char *p;
  int  Len;

  if ((Len = strlen(Name)) > 12)
    return;

  strcpy(F,Name);
  strupr(F);
  p = strchr(F,'.');

  if (p != NULL)
    Len = p - F;

  if (Len > 8)
    return;

  if (p == NULL || *(p+1) == 0)
    strcpy(E,"   ");
  else {
    memcpy(E,p+1,3);
    E[3] = 0;
  }

  F[Len] = 0;
  wildcard(F,8);
  wildcard(E,3);

  strcpy(Name,F);
  Name[8] = '.';
  strcpy(&Name[9],E);
}


int pascal equalwilds(char S1[], char S2[]) {
  int X,MisMatch;

  for (X = 0, MisMatch = 0; X < 13 && ! MisMatch; X++)
    if (S1[X] != '?')
      if (S1[X] != S2[X])
        MisMatch = 1;

  return(! MisMatch);
}
