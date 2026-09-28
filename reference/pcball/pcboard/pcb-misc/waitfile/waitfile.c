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
#include <system.h>
#include <screen.h>
#include <dosfunc.h>
#include <misc.h>

#ifndef __OS2__
extern "C" {
void LIBENTRY settimer(int Which, long Hundredths);
long LIBENTRY gettimer(int Which);
}
#else
void LIBENTRY settimer(int Which, int  Hundredths);
int  LIBENTRY gettimer(int Which);
#endif

#ifndef __OS2__
  #define FIVESECONDS     ((5 * 182) / 10)
#endif


void LIBENTRY say(char *Str) {
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif
  doswrite(1,Str,strlen(Str) POS2ERROR);
}


bool pascal checkforfile(char *FileName, bool WaitForExist) {
  say("\r\nChecking for ");
  say(FileName);
  if (fileexist(FileName) == 255) {
    if (WaitForExist) {
      say(" ... not there yet ... waiting five more seconds");
    } else {
      say(" ... has been removed .. returning errorlevel 1\r\n");
      return(TRUE);
    }
  } else {
    if (WaitForExist) {
      say(" ... found ... returning errorlevel 1\r\n");
      return(TRUE);
    } else {
      say(" ... still there ... waiting five more seconds");
    }
  }
  return(FALSE);
}


int main(int argc, char **argv) {
  bool WaitForExist;

  if (argc < 3) {
    say("\r\n"
        "usage:  WAITFILE filename seconds (remove)\r\n"
        "\r\n"
        "WAITFILE checks for the existence of 'filename' once every five seconds\r\n"
        "and continues to retry the operation for as many 'seconds' as specified.\r\n"
        "\r\n"
        "The 'remove' keyword is optional and, if used, indicates that WAITFILE is\r\n"
        "waiting for 'filename' to be removed instead of created.\r\n"
        "\r\n"
        "When filename exists (or is removed), WAITFILE exits with an errorlevel of 1.\r\n"
        "\r\n"
        "After seconds expires, WAITFILE exits when an errorlevel of 0.\r\n");
    return(99);
  }

  if (argc == 4)
    WaitForExist = (stricmp(argv[3],"REMOVE") == 0 ? FALSE : TRUE);
  else
    WaitForExist = TRUE;

  #ifndef __OS2__
    DisableGiveup = FALSE;
    char *p = getenv("PCB");
    if (p != NULL) {
      strupr(p);
      if (strstr(p,"/NOGIVEUP") != NULL) {
        DisableGiveup = TRUE;
        say("\r\nNo time slice giveup being used\r\n");
      }
    }

    checkmultitaskers();
    settimer(2,0);
  #endif

  #ifdef __OS2__
    settimer(1,atol(argv[2]) * 1000);
  #else
    settimer(1,(atol(argv[2]) * 182L) / 10L);
  #endif

  do {
    #ifdef __OS2__
      if (checkforfile(argv[1],WaitForExist))
        return(1);
      mydelay(500);
    #else
      if (gettimer(2) <= 0) {
        if (checkforfile(argv[1],WaitForExist))
          return(1);
        settimer(2,FIVESECONDS);
      }
      if (DisableGiveup) {
        bgetkey(1);             // signify that we're not busy by polling kbd
      } else {
        giveup();
        giveup();
      }
    #endif
  } while (gettimer(1) > 0);

  say("\r\n\n");
  say(argv[2]);
  say(" seconds have expired ... returning errorlevel 0\r\n");
  return(0);
}
