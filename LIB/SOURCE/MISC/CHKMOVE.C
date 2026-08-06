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


#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <direct.h>
#include <io.h>
#ifdef __WATCOMC__
struct ffblk { char ff_reserved[21]; char ff_attrib; unsigned short ff_ftime; unsigned short ff_fdate; long ff_fsize; char ff_name[256]; };
#endif
#else
  #include <dir.h>
#endif

#include <mem.h>
#include <stdio.h>
#include <string.h>
#include "screen.h"
#include "scrnio.h"
#include "scrnio.ext"
#include "validate.h"
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


extern struct ffblk DTA;   /* really declared as "char DTA[45]" in exist.c */


static void _NEAR_ LIBENTRY prepmessage(char *Msg1, char *Msg2) {
  beep();
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Line1   = 16;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Msg1    = Msg1;
  MsgData.Line2   = 18;
  MsgData.Color2  = Colors[HEADING];
  MsgData.Msg2    = Msg2;
}


int LIBENTRY checkmovefile(char *Srce, char *Dest) {
  int   Code;
  char *Msg1;
  char  Date[10];
  char  Temp[80];
  char  Path[2][80];

#ifndef __OS2__   /* this code needs to be finished for OS/2 */
  Code = validatepath(NULL,Srce,Path[0],VALIDATE_FILE);
  switch (Code) {
    case ERRNOTFILE     : Msg1 = "Error, found a DIRECTORY not a file!";
                          break;
    case ERRDIRNOTFOUND :
    case ERRNOTFOUND    : Msg1 = "Error, File not found!";
                          break;
    case ERRDISKINVALID : Msg1 = "Invalid disk drive!";
                          break;
  }

  if (Code) {
    prepmessage(Msg1,Srce);
    showmessage();
    return(-1);
  }

  Code = validatepath(NULL,Dest,Path[1],VALIDATE_FILE);
  switch (Code) {
    case 0              : fileexist(Dest); /* this fills DTA with info */
                          dtoc(DTA.ff_fdate,Date);
                          sprintf(Temp,"%s  %ld  %s",Dest,DTA.ff_fsize,Date);
                          prepmessage("File already exists!",Temp);
                          MsgData.Quest     = "Overwrite with new file";
                          MsgData.QuestLine = 20;
                          MsgData.Answer[0] = 'N';
                          MsgData.Answer[1] = 0;
                          MsgData.Mask      = YESNO;
                          showmessage();
                          if (KeyFlags == ESC || MsgData.Answer[0] != 'Y')
                            return(-1);
                          unlink(Dest);
                          break;
    case ERRDISKINVALID : prepmessage("Invalid disk drive!",Path[1]);
                          showmessage();
                          return(-1);
  }
#endif

  return(movefile(Path[0],Dest));
}
