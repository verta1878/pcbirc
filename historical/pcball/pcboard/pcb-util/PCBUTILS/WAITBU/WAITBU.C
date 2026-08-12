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


#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <dosfunc.h>
#include <misc.h>

#pragma inline

static char CurSec;

static char *Taskers[6] = {
  "NONE",
  "DoubleDOS",
  "DESQview",
  "PC-MOS",
  "Windows",
  "OS/2"
};



void pascal say(char *Str) {
  doswrite(0,Str,strlen(Str));
}


int pascal waitasec(void) {
  do {
    /* give up some time */
    giveup();
    giveup();

    /* check for a keystroke, get out now if one is available */
    if (bgetkey(1)) {
      bgetkey(0);
      return(1);
    }

    /* give up some more time */
    giveup();
    giveup();

    /* check for the current second */
    asm Mov  Ah,2Ch
    asm Int  21h
    asm Mov  Al,Dh

    /* loop back up if it's the same second it was last time we checked */
  } while (_AL == CurSec);

  /* save the second for the next time waitasec() is called */
  CurSec = _AL;
  return(0);
}


void main(int argc, char **argv) {
  int  Minutes;
  int  Seconds;
  char Str[10];

  if (argc != 2 || (Minutes = atoi(argv[1])) < 1) {
    say("\r\n"
        "usage:  WAITBU num\r\n"
        "\r\n"
        "where:  'num' is the number of minutes to wait\r\n"
        "\r\n");
    return;
  }

  checkmultitaskers();

  say("\r\nWAITBU - Multitasking Friendly Count Down\r\n");
  say("Multitasker Detected: ");
  say(Taskers[Tasker/2]);
  say("\r\n\n");

  /* initialize the waitasec value */
  CurSec = 255;
  waitasec();

  for (Seconds = Minutes*60; Seconds >= 0; Seconds--) {
    say("\rT-minus: ");
    ascii(Str,Seconds);
    say(Str);
    say(" ");
    if (waitasec())
      break;
  }
}
