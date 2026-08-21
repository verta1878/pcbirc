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


#include <mem.h>
#include "scrnio.h"
#include "scrnio.ext"
#include "system.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: userabort()
*
*  Desc    : Monitors the keyboard to see if the ESC key is pressed
*
*  Calls   : keypressed(), inkey()
*
*  Returns : TRUE if the ESC key was pressed
*/

bool LIBENTRY userabort(void) {
  char Ext,Key;

  if (bgetkey(1)) {
    Key = inkey(&Ext,CLOCK);
    if (! Ext && Key == 27)
      return(TRUE);
  }
  return(FALSE);
}


bool LIBENTRY checkuserabort(void) {
  if (userabort()) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox   = TRUE;
    MsgData.Save      = TRUE;
    MsgData.Msg1      = "WARNING:  Changes will be lost if aborted!";
    MsgData.Line1     = 18;
    MsgData.Color1    = Colors[HEADING];
    MsgData.Quest     = "Do you want to abort the operation?";
    MsgData.QuestLine = 20;
    MsgData.Answer[0] = 'N';
    MsgData.Answer[1] = 0;
    MsgData.Mask      = YESNO;
    showmessage();
    if (MsgData.Answer[0] == 'Y')
      return(TRUE);
  }
  return(FALSE);
}
