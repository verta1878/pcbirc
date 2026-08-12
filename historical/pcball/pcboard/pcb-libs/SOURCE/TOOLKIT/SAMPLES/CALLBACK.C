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
#include "pcbtools.h"

#define CONNECTED         1
#define NOT_CONNECTED     0
#define SIXTYSECONDS      (60 * 182 / 10)
#define ONESECOND         9

#define DISPLAY           NEWLINE|LFBEFORE|UPCASE|ECHODOTS|FIELDLEN
#define TXT_PWRD          148


int DialNumber(void) {
  char InBuf[128];
  char Command[80];

  buildstr(Command,"ATDT",UserSys.HomeVoicePhone,"\r",NULL);

  tickdelay(ONESECOND);           /* Give modem 1 second to reset          */
  turnondtr();                    /* Get modem ready to receive command    */
  tickdelay(ONESECOND);           /* Wait another second for DTR           */

  if (modemcommand("AT\r",WAITFOROKAY) == -1) /* send AT to get attention  */
    return(NOT_CONNECTED);

  modemcommand(Command,DONTWAIT);  /* tell modem to dial, don't wait for OK */

  ansi("Wait 60 seconds...\r\n"); /* Let the sysop know we're waiting...   */

  settimer(4,SIXTYSECONDS);       /* Give it about 60 seconds to connect   */
  while (! online()) {            /* Watch for carrier detect to turn on   */
    if (showmodem(InBuf,sizeof(InBuf),DONTWAIT) == -1)
      return(NOT_CONNECTED);
    if (gettimer(4) < 0)          /* Check for 60 second timer expiring    */
      return(NOT_CONNECTED);      /* Indicate 60 second wait expired       */
  }

  tickdelay(ONESECOND);
  showmodem(InBuf,sizeof(InBuf),FALSE);
  return(CONNECTED);              /* Indicate caller is back online        */
}


void main(void) {
  char Str[80];
  char Pwrd[13];
  int  X;

  if (initdoor("CALLBACK DOOR",0,0,READUSERSYS|SHOWSTATUSLINE) == -1)
    return;

  /* Let user know that we'll be calling them back at the number */
  /* they used as their home number                              */

  print("Will be calling you back at ");
  println(UserSys.HomeVoicePhone);
  println("When the phone rings type ATA and press ENTER");

  waitforempty(60);         /* Wait for buffer to empty             */
  tickdelay(ONESECOND);     /* Wait another second for modem buffer */
  Asy.IgnoreCDLoss=TRUE;    /* Disable carrier loss detection       */
  turnoffdtr();             /* Hang up the line                     */

  settimer(4,SIXTYSECONDS);       /* Give it about 30 seconds to hang up   */
  while (cdstillup()) {           /* Watch for carrier detect to go off    */
    if (gettimer(4) < 0)          /* Don't let it stay in here forever     */
      break;
  }

  if ((DialNumber()) == CONNECTED) {
    Asy.IgnoreCDLoss=FALSE;      /* Enable carrier loss detection   */
    Asy.LostCarrier=FALSE;       /* Enable carrier                  */
    clearinbuf();                /* Real important to clear buffer! */

    /* Get User's Password for the BBS - give 3 tries */
    for (X = 0; X < 3; X++) {
      Pwrd[0] = 0;
      inputfield(Pwrd,TXT_PWRD,sizeof(Pwrd)-1,DISPLAY,NOHELP,mask_alphanum);

      /* If password is correct increase security level */
      if (stricmp(Pwrd,UserSys.Password) == 0){
        println("Thank You!  Security level increased to 50.");
        println("Returning you to PCBoard...");
        UserSys.SecurityLevel=50;
        writelog("Callback verified - new level: 50",SPACERIGHT);
        closedoor(TRUE);    /* return to PCBoard */
      } else {
        println("Wrong password!");
        sprintf(Str,"Wrong password: %s",Pwrd);
        writelog(Str,SPACERIGHT);
      }
    }
    println("Too many errors, hanging up now");
    goodbye(FALSE);            /* hang up phone and return to PCBoard */
  } else {
    println("Callback unsuccessful");
    writelog("Callback unsuccessful",SPACERIGHT);
  }

  closedoor(FALSE);   /* return to PCBoard */
}
