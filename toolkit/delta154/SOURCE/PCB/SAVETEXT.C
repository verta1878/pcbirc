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


#include "screen.h"
#include "scrnio.h"
#include "scrnio.ext"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function: savetext()
*
*  Desc    : Asks the users if they want to save the file
*
*  Returns : TRUE if the user wants to save the file, FALSE otherwise
*/

bool LIBENTRY savetext(void) {
  bool Okay;

  Okay = TRUE;
  boxcls(28,18,52,22,Colors[MENUBOX],SINGLE);
  inputnum(30,20,1,"Save File to Disk",&Okay,vBOOL,0);

  if (KeyFlags == ESC)
    Okay = FALSE;

  if (Okay) {
    boxcls(28,18,52,22,Colors[MENUBOX],SINGLE);
    fastprint(35,20,"Saving....",Colors[ANSWER]);
  }
  return(Okay);
}
