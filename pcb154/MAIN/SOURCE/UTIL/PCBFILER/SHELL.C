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
#include "pcbfiler.h"
#include "pcbfiler.ext"

static const int TOP     =  7;
static const int BOTTOM  = 20;
static const int VIEWARC =  0;
static const int VIEWZIP =  1;

static int  ArcLine;     /* used by the arcview routines */

static int near  findprogram(char *ProgName) {
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


static int near  runprogram(char *ExecStr, char *ParamStr) {
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
  getcurdir(0,&OldPath[1]);

  cls();
  gotoxy(0,0);
  doswrite(1,"\r\n",2); //lint !e534
  uninstallhandlers();
  VMEMSStateSave();     //lint !e534

  if (PcbData.Swap) {
    Tmp = 0;
    if (swapenv(ComSpec,Str,&RetVal,"PCBFILER.$$$") != SWAP_OK) {
      PcbData.Swap = FALSE;  /* it failed!  Don't let it try again */
      goto noswap;
    }
    unlink("PCBFILER.$$$");
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


static void near  pause(void) {
  char Ch;
  fastprintmove(2,20,"press any key to continue",Colors[DISPLAY]);
  Ch = inkey(&Ch,CLOCK);
  if (Ch == 27)
    Aborted = TRUE;
}


/********************************************************************
*
*  Function: PRINTARCLINE()
*
*  Desc    : Called by the assembly language arcv() routine to print each
*            line found in an arc file.
*
*/

extern "C" int  _PRINTARCLINE(char *str) {
  if (ArcLine == BOTTOM) {
    pause();
    gotoxy(0,BOTTOM);
    clsbox(2,TOP,72,BOTTOM,Colors[QUESTION]);
    ArcLine = TOP;
  }
  fastprint(2,ArcLine,str,Colors[QUESTION]);
  ArcLine++;
  return(Aborted);
}



static void near  internalview(char *FilePathAndName, int Which) {
  Aborted = FALSE;
  clsbox(1,3,78, 3,Colors[ANSWER]);
  boxcls(0,4,79,21,Colors[ANSWER],SINGLE);
  fastprint(2,3,SrchDskPath,Colors[HEADING]);
  fastprint(2,5,"  Filename       Length     Method    SF    Size Now     Date     Time" ,Colors[ANSWER]);
  fastprint(2,6,"------------   ----------  --------  ----  ----------  ---------  -----",Colors[ANSWER]);
  ArcLine = 7;
//ScrollFlag = FALSE;

  switch (Which) {
    case VIEWARC: ARCV(FilePathAndName);         break;  //lint !e534
    case VIEWZIP: ListZipFile(FilePathAndName);  break;  //lint !e534
  }

  if (! Aborted)
    pause();
}


int  findfile(char *FileName, char *DskPath, char *PathListName) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox   = TRUE;
  MsgData.Save      = TRUE;
  MsgData.Msg1      = FileName;
  MsgData.Line1     = Scrn_BottomRow - 8;
  MsgData.Color1    = Colors[HEADING];
  MsgData.Msg2      = "file not found";
  MsgData.Line2     = Scrn_BottomRow - 6;
  MsgData.Color2    = Colors[HEADING];
  MsgData.Answer[0] = 'Y';

  if (DskPath[0] != 0) {
    if (verifyexist('Y',DskPath,PathListName,FileName) == 255) {
      MsgData.Quest     = "Search directories in download path";
      MsgData.QuestLine = Scrn_BottomRow - 4;
      MsgData.Answer[1] = 0;
      MsgData.Mask      = YESNO;
      showmessage();
      if (KeyFlags == ESC || MsgData.Answer[0] != 'Y') {
        KeyFlags = NOTHING;
        return(-1);
      }
    } else
      return(0);
  }

  if (verifyexist('A',DskPath,PathListName,FileName) == 255) {
    MsgData.QuestLine = 0;
    showmessage();
    KeyFlags = NOTHING;
    return(-1);
  }

  return(0);
}



/********************************************************************
*
*  Function: fileview()
*
*  Desc    :
*
*/

void  fileview(char *FileName) {
  char          Ch;
  bool          Temp;
  int           RetVal;
  fonttype      SaveFont;
  char         *p;
  ViewType     *q;
  char          ParmStr[60];
  char          ExecStr[60];
  savescrntype  ScrnBuf;

  SaveFont = getfont();
  savescreen(&ScrnBuf);
  setfont(BIGFONT);


  if ((p = strrchr(FileName,'.')) != NULL)
    p++;

  for (q = &Work.View[0]; q <= &Work.View[5]; q++) {
    if (stricmp(q->Ext,p) == 0 || strcmp(q->Ext,"???") == 0) {
      if (strcmp(q->Cmd,"INTERNAL") == 0) {
        if (stricmp(q->Ext,"ARC") == 0) {
          internalview(FileName,VIEWARC);
          break;
        }
        if (stricmp(q->Ext,"ZIP") == 0) {
          internalview(FileName,VIEWZIP);
          break;
        }
      } else {
        strcpy(ExecStr,q->Cmd);

        if ((p = strchr(ExecStr,' ')) != NULL) {
          *p = 0;
          strcpy(ParmStr,p+1);
          strcat(ParmStr," ");
          strcat(ParmStr,FileName);
          RetVal = runprogram(ExecStr,ParmStr);
        } else {
          RetVal = runprogram(ExecStr,FileName);
        }

        if (RetVal != -1) {
          fastprintmove(0,wherey(),"View completed.  Press any key to continue...",Colors[DISPLAY]);
          Temp = UpdateKbdStatus;
          UpdateKbdStatus = FALSE;
          Ch = inkey(&Ch,NOCLOCK);
          UpdateKbdStatus = Temp;
        }
      }
      break;
    }
  }

  setfont(SaveFont);
  restorescreen(&ScrnBuf);
  KeyFlags = NOTHING;
}


void  shelltodos(void) {
  fonttype      SaveFont;
  savescrntype  ScrnBuf;

  if (DisableShellToDos)
    return;

  SaveFont = getfont();
  savescreen(&ScrnBuf);
  setfont(BIGFONT);

  runprogram(ComSpec,"");  //lint !e534

  setfont(SaveFont);
  restorescreen(&ScrnBuf);
}


void  shelltoprogram(char *Program, char *Param) {
  fonttype      SaveFont;
  savescrntype  ScrnBuf;

  SaveFont = getfont();
  savescreen(&ScrnBuf);
  setfont(BIGFONT);

  runprogram(Program,Param); //lint !e534

  setfont(SaveFont);
  restorescreen(&ScrnBuf);
}


