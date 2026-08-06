/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* WILDCARD.C — Wildcard filename expansion                                  */
/* Clark Development Company, Inc. (C) 1996. All Rights Reserved.            */
/* Watcom C conversion by pcbrevival (GPL v3.0 for our additions)            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <string.h>
#include <stdio.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

static void near pascal wildcard(char *Field, int Len) {
#if defined(__OS2__) || defined(__WATCOMC__)
  int slen = (int)strlen(Field);
  int i;
  if (slen == 0) return;
  if (Field[slen-1] == '*') {
    for (i = slen-1; i < Len; i++)
      Field[i] = '?';
  } else {
    for (i = slen; i < Len; i++)
      Field[i] = ' ';
  }
  Field[Len] = 0;
#else
  asm  Push Es
  asm  Xor  Si,Si
  asm  Les  Bx,Field
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
  asm  Cmp  byte ptr Es:[Bx+Di],'*'
  asm  Jne  J1
  asm  Mov  Al,'?'
  asm  Jmp  J2
J1:
  asm  Inc  Di
  asm  Mov  Al,' '
J2:
L1:
  asm  Cmp  Di,Len
  asm  Jg   J3
  asm  Mov  Es:[Bx+Di],Al
  asm  Inc  Di
  asm  Jmp  L1
J3:
  asm  Add  Bx,Len
  asm  Mov  byte ptr Es:[Bx],0
J4:
  asm  Pop  Es
#endif
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
