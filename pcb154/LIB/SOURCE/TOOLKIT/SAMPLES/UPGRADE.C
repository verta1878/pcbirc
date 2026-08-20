#ifndef LIB
#define LIB
#endif
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


bool pascal verifyuser(void) {
  char Password[16];

  /* Display a file to the caller explaining what is about to happen. */
  /* Use a graphics version of the file if one is available.          */

  displayfile("EXPLAIN",GRAPHICS);

  /* Ask the caller for a password.  Make sure the default is blank by   */
  /* setting the first character to a NULL terminator.  Ask the question */
  /* in yellow.  Issue a line feed before the prompt in case the EXPLAIN */
  /* file did not end with a blank line.  Show "(     )" field length    */
  /* delimiters whether in graphics mode or not.  Automatically convert  */
  /* the answer given to uppercase letters.  Provide no help but allow   */
  /* any alpha numeric character to be typed for the password.           */

  Password[0] = 0;
  inputfieldstr(Password,"Enter password for verification",PCB_YELLOW,
                sizeof(Password)-1,NEWLINE|LFBEFORE|FIELDLEN|GUIDE|UPCASE,
                NOHELP,mask_alphanum);

  /* Check to see if the password entered is an exact match for UPGRADEME */
  /* and if so return a TRUE value, otherwise return a FALSE value.       */

  return(strcmp(Password,"UPGRADEME") == 0);
}


void pascal upgradeuser(void) {
  int Counter;

  /* Display a file to the caller thanking him for registering. */
  /* Use a graphics version of the file if one is available.    */

  displayfile("THANKS",GRAPHICS);

  /* Set his normal (unexpired) security level to 20 and his expired     */
  /* security level to 19.  Set his expiration date to 1 year (365 days) */
  /* from today's date.  When he expires PCBoard will automatically use  */
  /* the expired level rather than the normal level.                     */

  UserSys.SecurityLevel = 20;
  UserSys.ExpSecurityLevel = 19;
  UserSys.RegExpDate = getjuliandate() + 365;

  /* Set the 'R' (registered) flag in conferences 1 through 10.  Do not   */
  /* set the 'X' (expired) flag - this will allow him access to those     */
  /* conferences until after he expires.  Also, set the scan flag so that */
  /* the user will automatically be set to scan conferences 1-10.         */


  for (Counter = 1; Counter <= 10; Counter++) {
    setbit(&ConfReg[CNF_REG],Counter);
    setbit(&ConfReg[CNF_USR],Counter);
  }

  /* Record in the callers log the fact that he was upgraded. */

  writelog("Password Verified",SPACERIGHT);
}

#define OPENPARAMS  WATCHKBD|        /* watch for keyboard timeout */ \
                    WATCHSESSION|    /* watch session time limit   */ \
                    READUSERSYS|     /* read the user.sys file     */ \
                    SHOWSTATUSLINE   /* show pcb-style status line */

void main(void) {
  if (initdoor("Upgrade Door",0,0,(openstatus)(WATCHKBD|WATCHSESSION|READUSERSYS|SHOWSTATUSLINE)) == -1)
    return;

  if (verifyuser()) {    /* if the user passed the verification */
    upgradeuser();       /*   then upgrade his access           */
    closedoor(TRUE);     /*   and exit updating users.sys       */
  }

  printcolor(PCB_RED);
  println("Password incorrect.  Returning to PCBoard.");
  closedoor(FALSE);
}
