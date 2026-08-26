#ifdef __WATCOMC__
#include <direct.h>
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


#include <dir.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <errno.h>
#include <swap.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <dosfunc.h>
#include <vmdata.h>
#include "setup.h"
#include "setup.ext"

static int near pascal findprogram(char *ProgName) {
  char *Ext[4] = {".EXE",".COM",".BAT",""};
  char *p;
  char Save;
  int  X;

  X = (strchr(ProgName,'.') != NULL ? 3 : 0);
  p = &ProgName[strlen(ProgName)];
  Save = *p;

  for (; X < 4; X++) {
    strcpy(p,Ext[X]);
    if (srchpath(ProgName) != -1)
      return(0);
  }
  *p = Save;
  return(-1);
}


static int near pascal runprogram(char *ExecStr, char *ParamStr) {
  char OldDrive;
  char RetVal;
  int  Tmp;
  char ProgName[66];
  char OldPath[66];
  char Str[128];

  strcpy(ProgName,ExecStr);
  if (findprogram(ProgName) == -1) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save    = TRUE;
    MsgData.AutoBox = TRUE;
    MsgData.Line1   = Scrn_BottomRow - 8;
    MsgData.Color1  = Colors[HEADING];
    MsgData.Msg1    = "Unable to find executable.";
    MsgData.Line2   = Scrn_BottomRow - 6;
    MsgData.Color2  = Colors[HEADING];
    MsgData.Msg2    = ExecStr;
    beep();
    showmessage();
    return(-1);
  }

  sprintf(Str,"/c %s %s",ProgName,ParamStr);

  OldDrive = getdisk();
  OldPath[0] = '\\';
  #ifdef __WATCOMC__
  _getdcwd(0, OldPath, sizeof(OldPath));
#else
  getcurdir(0,&OldPath[1]);
#endif

  cls();
  gotoxy(0,0);
  doswrite(1,"\r\n",2); //lint !e534
  uninstallhandlers();
  VMEMSStateSave();     //lint !e534

  if (PcbData.Swap) {
    Tmp = 0;
    if (swapenv(ComSpec,Str,&RetVal,"PCBSETUP.$$$") != SWAP_OK) {
      PcbData.Swap = FALSE;  /* it failed!  Don't let it try again */
      goto noswap;
    }
    unlink("PCBSETUP.$$$");
  } else {
noswap:
    Tmp = spawnl(P_WAIT,ComSpec,ComSpec,Str,NULL);

    if (Tmp == -1) {
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.Save    = TRUE;
      MsgData.AutoBox = TRUE;
      MsgData.Line1   = 18;
      MsgData.Color1  = Colors[HEADING];
      switch (errno) {
        case ENOENT: MsgData.Msg1 = "Unable to find or execute defined editor."; break;
        case ENOMEM: MsgData.Msg1 = "Insufficient convential memory.";           break;
        default    : goto done;
      }
      beep();
      showmessage();
    }
  }

done:
  VMEMSStateRestore();  //lint !e534
  reinstallhandlers();
  setdisk(OldDrive);
  chdir(OldPath);
  return(Tmp);
}


void pascal shelltoprogram(char *Program, char *Param) {
  fonttype      SaveFont;
  savescrntype  ScrnBuf;

  SaveFont = getfont();
  savescreen(&ScrnBuf);
  setfont(BIGFONT);

  runprogram(Program,Param); //lint !e534

  setfont(SaveFont);
  restorescreen(&ScrnBuf);
}


