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
#else
  #include <dir.h>
#endif

#include <io.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <validate.h>
#include <country.h>
#ifdef DEBUG
#include <memcheck.h>
#endif


extern struct ffblk DTA;   /* really declared as "char DTA[45]" in exist.c */

int LIBENTRY movefile(char *Srce, char *Dest) {
  int  Code;
  char Path[2][80];
  char Date[10];
  char Temp[80];

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Line1   = 16;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Line2   = 18;
  MsgData.Color2  = Colors[HEADING];

#if ! defined(__OS2__)  /* this needs to be finished for OS/2 */
  Code = validatepath(NULL,Srce,Path[0],VALIDATE_FILE);
  switch (Code) {
    case ERRNOTFILE     : MsgData.Msg1 = "Error, found a DIRECTORY not a file!";
                          break;
    case ERRDIRNOTFOUND :
    case ERRNOTFOUND    : MsgData.Msg1 = "Error, File not found!";
                          break;
    case ERRDISKINVALID : MsgData.Msg1 = "Invalid disk drive!";
                          break;
  }
  if (Code) {
    beep();
    MsgData.Msg2 = Srce;
    showmessage();
    return(-1);
  }

  Code = validatepath(NULL,Dest,Path[1],VALIDATE_FILE);
  switch (Code) {
    case 0              : fileexist(Dest); /* this fills DTA with info */
                          dtoc(DTA.ff_fdate,Date);
                          sprintf(Temp,"%s  %ld  %s",Dest,DTA.ff_fsize,countrydate(Date));

                          MsgData.Msg1      = "File already exists!";
                          MsgData.Msg2      = Temp;
                          MsgData.Quest     = "Overwrite with new file";
                          MsgData.QuestLine = 20;
                          MsgData.Answer[0] = 'N';
                          MsgData.Answer[1] = 0;
                          MsgData.Mask      = YESNO;
                          beep();
                          showmessage();
                          if (KeyFlags == ESC || MsgData.Answer[0] != 'Y')
                            return(-1);
                          unlink(Dest);
                          break;
    case ERRDISKINVALID : MsgData.Msg1 = "Invalid disk drive!";
                          MsgData.Msg2 = Path[1];
                          beep();
                          showmessage();
                          return(-1);
  }
#endif

  if (Path[0][0] == Path[1][0]) {
    if (strcmp(Path[0],Path[1]) == 0)
      return(0);
    Code = rename(Path[0],Dest);
    if (Code) {
      Code = copyfile(Path[0],Dest,FALSE);
      if (! Code)
        unlink(Path[0]);
    }
  } else {
    Code = copyfile(Path[0],Dest,FALSE);
    if (! Code)
      unlink(Path[0]);
  }
  return(Code);
}
