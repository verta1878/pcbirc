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
#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <swap.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


int pascal externaledit(edittype Edit) {
  int Len;
  int ExtLen;

  if ((Edit & EDIT_CHECK7) > 0) {
    if (checkfiles7(FALSE))
      return(-1);
  } else if ((Edit & EDIT_CHECK8) > 0) {
   if (checkfiles8(FALSE))
     return(-1);
  }

  stripright(SaveFldPtr->Answer,' ');
  if (strlen(SaveFldPtr->Answer) != 0) {
    if (KeyFlags == FLAG3)
      editor(SaveFldPtr->Answer,FALSE);
    else if (KeyFlags == FLAG4 && (Edit & EDIT_GRAPHICS) > 0) {
      findfilename(&Len,&ExtLen);
      if (ExtLen == -1)
        editor(SaveFldPtr->Answer,TRUE);
    }
  }

  return(0);
}


void pascal showeditmsg(int Before, char *Msg) {
  char Str[80];

  if (Before) {
    sprintf(Str," Press F2 to edit %s file ",Msg);
    fastcenter(23,Str,Colors[DESC]);
  } else {
    clsbox(1,23,78,23,Colors[OUTBOX]);
  }
}


void pascal showeditmsg2(int Before, char *Msg) {
  char Str[80];

  if (Before)
    sprintf(Str,"the %s file or F3 to edit the %sG",Msg,Msg);
  showeditmsg(Before,Str);
}


int pascal numrandrecords(char *FileName, int RecSize) {
  struct ffblk rec;
  long   NumRecs;

  if (findfirst(FileName,&rec,FA_ARCH) == -1)
    return(0);

  NumRecs = rec.ff_fsize / RecSize;
  return((int) NumRecs);
}


int pascal numtextrecords(char *FileName) {
  char    Buffer[256];
  DOSFILE Input;
  int     NumRecs;

  if (fileexist(FileName) == 255)
    return(0);

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&Input) == -1)
    return(0);

  NumRecs = 0;
  while (dosfgets(Buffer,256,&Input) != -1)
    NumRecs++;
  dosfclose(&Input);

  return(NumRecs);
}


void pascal editor(char *FileName, bool Graphics) {
  char         RetVal;
  int          Tmp;
  fonttype     SaveFont;
  char        *p;
  char         ErrorStr[40];
  char         TempName[66];
  char         ProgName[66];
  char         Str[128];
  savescrntype ScreenBuf;

  strcpy(TempName,FileName);
  stripright(TempName,' ');

  if (Graphics) {
    addchar(TempName,'G');
    strcpy(ProgName,smConfig.GrphEdit);
  } else {
    strcpy(ProgName,smConfig.TextEdit);
  }

  if (strchr(TempName,'.') == NULL)
    addchar(TempName,'.');

  stripright(ProgName,' ');

  if (ProgName[0] == 0) {
    sprintf(Str,"The %s Editor is undefined.",(Graphics ? "Graphics" : "Text"));
    beep();
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save    = TRUE;
    MsgData.AutoBox = TRUE;
    MsgData.Line1   = 16;
    MsgData.Msg1    = Str;
    MsgData.Color1  = Colors[HEADING];
    MsgData.Line2   = 18;
    MsgData.Msg2    = "Run PCBSM and select Define Editors";
    MsgData.Color2  = Colors[HELPKEY];
    showmessage();
    return;
  }

  if (srchpath(ProgName) == -1) {
    if (strchr(ProgName,'.') == NULL) {
      p = &ProgName[strlen(ProgName)];
      strcpy(p,".COM");
      if (srchpath(ProgName) == -1) {
        strcpy(p,".EXE");
        if (srchpath(ProgName) == -1) {
          strcpy(p,".BAT");
          if (srchpath(ProgName) == -1) {
            beep();
            memset(&MsgData,0,sizeof(MsgData));
            MsgData.Save    = TRUE;
            MsgData.AutoBox = TRUE;
            MsgData.Line1   = 18;
            MsgData.Msg1    = "Unable to locate the defined editor in path.";
            MsgData.Color1  = Colors[HEADING];
            showmessage();
            return;
          }
        }
      }
    } else {
      beep();
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.Save    = TRUE;
      MsgData.AutoBox = TRUE;
      MsgData.Line1   = 18;
      MsgData.Msg1    = "Unable to locate the defined editor in path.";
      MsgData.Color1  = Colors[HEADING];
      showmessage();
      return;
    }
  }

  SaveFont = getfont();
  savescreen(&ScreenBuf);
  if (SaveFont != BIGFONT)
    setfont(BIGFONT);

  if (strstr(ProgName,".BAT") != NULL)
    sprintf(Str,"/c %s %s",ProgName,TempName);
  else
    Str[0] = 0;

  #ifdef TEST
  {
    char Line2[80];

    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save    = TRUE;
    MsgData.AutoBox = TRUE;
    MsgData.Line1   = 17;
    MsgData.Line2   = 18;
    MsgData.Color1  = Colors[QUESTION];
    MsgData.Color2  = Colors[QUESTION];
    MsgData.Msg1    = ComSpec;
    if (Str[0] != 0)
      MsgData.Msg2  = Str;
    else {
      sprintf(Line2,"\"%s\" \"%s\"",ProgName,TempName);
      MsgData.Msg2  = Line2;
    }
    showmessage();
  }
  #endif

  if (PcbData.Swap) {
    if (Str[0] != 0) {
      if (swapenv(ComSpec,Str,&RetVal,"PCBSETUP.$$$") != SWAP_OK) {
        sprintf(ErrorStr,"Swap Failed (%d)",RetVal);
        beep();
        memset(&MsgData,0,sizeof(MsgData));
        MsgData.Save    = TRUE;
        MsgData.AutoBox = TRUE;
        MsgData.Line1   = 18;
        MsgData.Msg1    = ErrorStr;
        MsgData.Color1  = Colors[HEADING];
        showmessage();
        goto noswap;
      }
    } else {
      if (swapenv(ProgName,TempName,&RetVal,"PCBSETUP.$$$") != SWAP_OK) {
        sprintf(ErrorStr,"Swap Failed (%d)",RetVal);
        beep();
        memset(&MsgData,0,sizeof(MsgData));
        MsgData.Save    = TRUE;
        MsgData.AutoBox = TRUE;
        MsgData.Line1   = 18;
        MsgData.Msg1    = ErrorStr;
        MsgData.Color1  = Colors[HEADING];
        showmessage();
        goto noswap;
      }
    }
    unlink("PCBSETUP.$$$");
  } else {
noswap:
    if (Str[0] != 0)
      Tmp = spawnl(P_WAIT,ComSpec,ComSpec,Str,NULL);
    else
      Tmp = spawnl(P_WAIT,ProgName,ProgName,TempName,NULL);

    if (Tmp == -1) {
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.Save    = TRUE;
      MsgData.AutoBox = TRUE;
      MsgData.Line1   = 18;
      MsgData.Color1  = Colors[HEADING];
      switch (errno) {
        case ENOENT: MsgData.Msg1 = "Unable to find or execute defined editor."; break;
        case ENOMEM: MsgData.Msg1 = "Insufficient memory to load editor.";       break;
        #ifdef TEST
        default    : sprintf(Str,"spawnl() errno = %d",errno);
                     MsgData.Msg1 = Str;
                     break;
        #else
        default    : goto done;
        #endif
      }
      beep();
      showmessage();
    }
  }

done:
  setfont(SaveFont);
  restorescreen(&ScreenBuf);
}
