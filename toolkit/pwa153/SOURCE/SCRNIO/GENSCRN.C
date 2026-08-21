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
#include <screen.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function:  generalscreen()
*
*  - Checks to see if the box around the screen needs to be updated if so,
*    it draws the screen as well as the time and date at the bottom
*  - It also displays the appropriate headings for that screen
*/

void LIBENTRY generalscreen(char *Heading1, char *Heading2) {
  if (UpdateBox) {
    box(0,0,79,Scrn_BottomRow,Colors[OUTBOX],DOUBLE);
    showtime();
    showkeystatus();
    #ifdef __OS2__
      enableshowstatus();
    #endif
    if (HelpFile != 0)
      fastprint(32,Scrn_BottomRow," F1 Í help ",Colors[HELPKEY]);
  }
  fastcenter(1,Heading1,Colors[HEADING]);
  fastcenter(2,Heading2,Colors[HEADING]);
}
