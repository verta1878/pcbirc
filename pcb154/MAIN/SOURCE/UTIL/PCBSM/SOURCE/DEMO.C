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


#ifdef DEMO

#include <mem.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#ifdef DEBUG
#include <memcheck.h>
#endif

void pascal demoscreen(void) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Line1   = 18;
  MsgData.Msg1    = "This feature is DISABLED in the demo version";
  MsgData.Color1  = Colors[HEADING];
  showmessage();
}
#endif
