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
#include <string.h>
#include <pcbtools.h>

#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY inputfieldreqstr(char *Buffer, char *Prompt, int Color, int MaxLen, int MinLen, DISPLAYTYPE Display, int HelpNum, char *Mask) {
  char Str[80];

  while (1) {
    padstr(Buffer,'_',MinLen);
    inputfieldstr(Buffer,Prompt,Color,MaxLen,Display,HelpNum,Mask);
    stripright(Buffer,'_');
    if (strlen(Buffer) >= MinLen)
      break;

    printcolor(PCB_RED);
    sprintf(Str,"At least %d characters of input required!",MinLen);
    println(Str);
  }
}

/*****************************************************************************
 remove comment delimiters around "#define TEST" to test this function
*****************************************************************************/
/* #define TEST */

#ifdef TEST

#define ADDRLEN  25          /* maximum address length */
#define ADDRMIN   6          /* minimum address length */

#define OPENSETTINGS   WATCHKBD|WATCHSESSION|READUSERSYS|SHOWSTATUSLINE
#define INPUTSETTINGS  FIELDLEN|GUIDE|NOCLEAR|NEWLINE|LFBEFORE|LFAFTER

void main(void) {
  char Address[ADDRLEN+1];  /* don't forget the NULL terminator */

  if (initdoor("TestDoor",0,0,OPENSETTINGS) == -1)
    return;

  println("This is a test");

  Address[0] = 0;
  inputfieldreqstr(Address,"Street Address",PCB_GREEN,ADDRLEN,ADDRMIN,
                   INPUTSETTINGS,NOHELP,mask_alphanum);

  closedoor(FALSE);
}
#endif
