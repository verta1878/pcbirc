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


/* this function is not currently being compiled for OS/2 */

#ifndef __OS2__

#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <validate.h>
#include "pcb.h"
#include <\proj\pcbsetup\source\setup.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

bool PerformValidation;
static bool AutoCreatePaths = FALSE;
extern char YNA[];

int LIBENTRY checkexistence(char *Path, char Choice, char *Reference, bool Display) {
  int  Code;
  int  X;
  int  Y;
  char ResultPath[66];
  char WorkPath[66];
  char Heading1[66];

  stripright(Path,' ');

  if (Path[0] == 0)
    return(0);

  strcpy(WorkPath,Path);

  if (Display) {
    X = wherex();
    Y = wherey();
    fastprint(X,Y,WorkPath,0x07);
  }

  Code = validatepath(NULL,Path,ResultPath,Choice);

errhandler:
  if (Code == 0 || Code == ERRNOTFOUND) {
    Code = 0;
    goto exit;
  }

  strcpy(Heading1,"ERROR IN PATH: ");
  strcat(Heading1,WorkPath);

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.Line1  = 15;
  MsgData.Msg1   = Heading1;
  MsgData.Color1 = Colors[HEADING];
  MsgData.Line3  = 18;
  MsgData.Msg3   = ResultPath;
  MsgData.Color3 = Colors[DISPLAY];
  MsgData.Save    = TRUE;
  MsgData.AutoBox = TRUE;

  if (Reference != NULL) {
    if ((MsgData.Msg2 = getcontext(Reference)) != NULL) {
      MsgData.Line2  = 16;
      MsgData.Color2 = Colors[HEADING];
    }
  }

  switch (Code) {
    case ERRDIRNOTFOUND: if (! AutoCreatePaths) {
                           MsgData.Answer[0] = 'Y';
                           MsgData.Answer[1] = 0;
                           MsgData.Mask      = YNA;
                           MsgData.Quest     = "Path above does not exist, create it now (Y,N,A=auto,ESC=stop checking)";
                           MsgData.QuestLine = 20;
                           showmessage();
                           if (KeyFlags == ESC) {
                             PerformValidation = FALSE;   /* turn off further validation checks! */
                             goto exit;
                           }
                           if (MsgData.Answer[0] == 'N')
                             goto exit;
                           if (MsgData.Answer[0] == 'A')
                             AutoCreatePaths = TRUE;
                         }
                         Code = validatepath(NULL,Path,ResultPath,(Choice==CHECK_DIR ? VALIDATE_DIR : VALIDATE_FILE));
                         goto errhandler;
    case ERRNOTFILE    : MsgData.Msg4   = "Expected to find a FILE but found a DIRECTORY instead!";
                         break;
    case ERRNOTDIR     : MsgData.Msg4   = "Expected to find a DIRECTORY but found a FILE instead!";
                         break;
    case ERRDISKINVALID: MsgData.Msg4   = "Disk drive specified is INVALID!";
                         break;
    case ERRPATHINVALID: MsgData.Msg4   = "Path specified is INVALID!";
                         break;
  }

  MsgData.Line4  = 20;
  MsgData.Color4 = Colors[HEADING];
  showmessage();

  if (KeyFlags == ESC)
    PerformValidation = FALSE;   /* turn off further validation checks! */

exit:
  if (Display) {
    memset(WorkPath,' ',strlen(WorkPath));
    fastprint(X,Y,WorkPath,0x07);
  }
  return(Code);
}

#endif
