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


#include "project.h"
#pragma hdrstop

#ifndef LIB
  #include "menu.h"
#endif

#define PCBVERSION    "15.3"
#ifdef LIB
#define PCBVERSION145 "14.5"
#endif

static int  PcbTextHandle      = 0;
static int  SystemHandle       = 0;

#ifndef LIB
static bool PcbTextLoaded      = FALSE;
#endif


#ifndef LIB
#if defined(_MSC_VER) || defined(__WATCOMC__)
typedef struct {
  char flag0: 1;
  char flag1: 1;
  char flag2: 1;
  char flag3: 1;
  char flag4: 1;
  char flag5: 1;
  char flag6: 1;
  char flag7: 1;
} bitflags;
#else
typedef struct {
  int flag0: 1;
  int flag1: 1;
  int flag2: 1;
  int flag3: 1;
  int flag4: 1;
  int flag5: 1;
  int flag6: 1;
  int flag7: 1;
} bitflags;
#endif


static void *PcbTextBuffer = NULL;
static pcbtexttype *PcbText[TXT_NUMPROMPTS+1];

#pragma warn -pin
static bitflags UseText[(TXT_NUMPROMPTS+8)/8] = {
/*   0 */ 0,1,1,1,1,1,1,1,1,1, /*  10 */ 1,1,1,1,1,1,1,1,1,1,
/*  20 */ 0,1,1,0,1,1,1,1,1,1, /*  30 */ 1,1,1,1,1,1,1,1,1,1,
/*  40 */ 1,1,1,1,0,1,1,0,0,1, /*  50 */ 1,1,1,1,1,1,1,1,1,1,
/*  60 */ 1,1,1,1,1,1,1,1,1,1, /*  70 */ 1,1,1,1,1,1,1,1,1,1,
/*  80 */ 1,0,1,1,1,1,1,1,1,1, /*  90 */ 1,0,1,1,1,1,1,1,1,1,
/* 100 */ 1,1,1,1,1,1,1,1,0,1, /* 110 */ 1,1,1,1,1,0,1,1,1,1,
/* 120 */ 1,1,0,1,0,1,1,1,1,1, /* 130 */ 1,1,1,1,1,1,1,1,1,1,
/* 140 */ 1,1,1,1,1,1,1,0,1,1, /* 150 */ 1,0,1,1,1,1,1,1,1,1,
/* 160 */ 1,1,1,1,1,1,1,1,1,1, /* 170 */ 1,1,0,1,0,1,1,1,1,1,
/* 180 */ 1,1,1,0,0,1,1,1,1,1, /* 190 */ 1,1,1,1,1,1,1,1,1,1,
/* 200 */ 1,0,0,1,1,0,0,0,1,1, /* 210 */ 1,1,1,1,1,1,1,1,1,1,
/* 220 */ 1,1,1,1,1,1,1,1,1,1, /* 230 */ 1,1,1,1,1,1,1,1,1,1,
/* 240 */ 1,1,1,1,1,1,1,1,1,1, /* 250 */ 1,1,1,1,1,1,1,1,1,1,
/* 260 */ 1,1,1,1,1,1,1,1,1,1, /* 270 */ 1,1,1,1,1,1,1,1,1,1,
/* 280 */ 1,1,1,1,1,1,1,1,1,1, /* 290 */ 1,1,1,1,1,1,1,1,1,1,
/* 300 */ 1,1,1,1,1,1,1,1,1,1, /* 310 */ 1,1,1,1,1,1,1,1,1,1,
/* 320 */ 1,1,1,1,1,1,1,1,1,1, /* 330 */ 1,1,1,1,1,1,1,1,1,1,
/* 340 */ 1,1,1,1,1,1,1,1,1,1, /* 350 */ 1,1,1,1,1,1,1,1,1,1,
/* 360 */ 1,1,0,1,0,1,1,1,1,1, /* 370 */ 1,1,1,1,1,1,1,1,1,1,
/* 380 */ 1,1,1,1,1,1,1,1,1,1, /* 390 */ 1,0,0,1,1,1,1,1,1,1,
/* 400 */ 1,1,1,1,1,1,1,1,1,1, /* 410 */ 0,1,1,1,1,1,1,1,1,1,
/* 420 */ 1,1,1,1,1,1,1,1,1,1, /* 430 */ 1,1,0,1,1,1,1,1,1,1,
/* 440 */ 1,1,1,1,1,1,1,1,1,1, /* 450 */ 1,1,1,1,1,1,0,1,1,1,
/* 460 */ 1,1,0,1,1,1,1,1,1,1, /* 470 */ 1,1,1,1,1,1,1,1,1,1,
/* 480 */ 1,1,1,1,1,1,1,1,1,1, /* 490 */ 1,1,1,1,1,1,1,1,1,1,
/* 500 */ 1,1,1,1,1,1,1,1,1,1, /* 510 */ 1,1,1,1,1,1,1,1,1,1,
/* 520 */ 1,1,1,1,1,1,1,1,1,1, /* 530 */ 1,1,1,1,1,1,1,1,1,1,
/* 540 */ 1,1,1,1,1,1,1,1,1,1, /* 550 */ 1,1,1,1,1,1,1,1,1,1,
/* 560 */ 1,1,1,1,1,1,1,1,1,1, /* 570 */ 1,1,1,1,1,1,1,1,1,1,
/* 580 */ 1,1,1,1,1,1,1,1,1,1, /* 590 */ 1,1,1,1,1,1,1,1,1,1,
/* 600 */ 1,1,1,1,1,1,1,1,1,1, /* 610 */ 1,1,1,1,1,1,1,1,1,1,
/* 620 */ 1,1,1,1,1,1,1,1,1,1, /* 630 */ 1,1,1,1,1,1,1,1,1,1,
/* 640 */ 1,1,1,1,1,1,1,1,1,1, /* 650 */ 1,1,1,1,1,1,1,1,1,1,
/* 660 */ 1,1,1,1,1,1,1,1,1,1, /* 670 */ 1,1,1,1,1,1,1,1,1,1,
/* 680 */ 1,1,1,1,1,1,1,1,1,1, /* 690 */ 1,1,1,1,1,1,1,1,1,1,
/* 700 */ 1,1,1,1,1,1,1,1,1,1, /* 710 */ 1,1,1,1,1,1,1,1,1,1,
/* 720 */ 1,1,1,1,1,1,1,1,1,1, /* 730 */ 1,1,1,1,1,1,1,1,1,1,
/* 740 */ 1,1,1,1,1,1,1
};
#pragma warn +pin
#endif


void LIBENTRY closepcbtext(void) {
  if (PcbTextHandle > 0) {
    dosclose(PcbTextHandle);
    if (PcbTextHandle == SystemHandle)  /* if we're sharing the file then */
      SystemHandle = 0;                 /* don't try to close it twice    */
    PcbTextHandle = 0;
  }
  if (SystemHandle > 0) {
    dosclose(SystemHandle);
    SystemHandle = 0;
  }
  #ifndef LIB
  if (PcbTextBuffer != NULL) {
    PcbTextLoaded = FALSE;
    free(PcbTextBuffer);
    PcbTextBuffer = NULL;
  }
  #endif
}


#pragma argsused
static int _NEAR_ LIBENTRY processtext(pcbtexttype *Buf, int TextNum) {
  #ifndef LIB
    if (isset(UseText,TextNum) == 0)
      return(0);
  #endif

/*
  switch (TextNum) {
    case TXT_FILESSHOWPROMPT: Buf->Color = 0x0E;
                              strcpy(Buf->Str,"(@TIMELEFT@ min left), (V)iew, (F)lag, (S)how, More");
                              break;
    case TXT_ENTERDIRCMD    : Buf->Color = 0x0E;
                              strcpy(Buf->Str,"Enter DIR Command");
                              break;
    case TXT_NOFILESFOUND   : Buf->Color = 0x0C;
                              strcpy(Buf->Str,"No files found.");
                              break;
    case TXT_DIRECTORYOF    : Buf->Color = 0x0D;
                              strcpy(Buf->Str,"Directory of @OPTEXT@");
                              break;
    case TXT_SHORTINEFFECT  : Buf->Color = 0x0F;
                              strcpy(Buf->Str,"Short Description Mode in effect");
                              break;
    case TXT_LONGINEFFECT   : Buf->Color = 0x0F;
                              strcpy(Buf->Str,"Long Description Mode in effect");
                              break;
    case TXT_SHOWLONGDESC   : Buf->Color = 0x0E;
                              strcpy(Buf->Str,"Show long description of what file");
                              break;
    case TXT_USESHORTDESC   : Buf->Color = 0x0E;
                              strcpy(Buf->Str,"Set default file description to SHORT (one-line) format");
                              break;
    default                 :
*/
                              Buf->Color = pcbcolors[Buf->Color-'0'];
                              Buf->Str[sizeof(Buf->Str)-1] = 0; /* place null terminator on string */
                              stripright(Buf->Str,' ');         /* get rid of all trailing spaces */
                              #ifdef LIB
                                change(Buf->Str,'~',' ');    /* change any ~ characters to spaces */
                              #else
                                if (PcbData.Foreign)
                                  substitute(Buf->Str,"~~"," ",sizeof(Buf->Str));  /* replace double ~~ */
                                else
                                  change(Buf->Str,'~',' ');    /* change any ~ characters to spaces */
                              #endif
/*
                              break;
  }
*/

  return(strlen(Buf->Str)+2);        /* add 2:  1 for color, 1 for NULL */
}


int LIBENTRY readpcbtextfile(char *Extension, callfromtype CalledFrom) {
  int         NewHandle;
  #ifndef LIB
  int         Num;
  int         Len;
  unsigned    ActualSize;
  char        *p;
  DOSFILE     PcbTextFile;
  #else
  long        Size;
  #endif
  char        MultiLangExt[sizeof(Status.MultiLangExt)];
  pcbtexttype Buf;
  char        FileName[128];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  #ifndef LIB
    if ((PcbTextBuffer != NULL || PcbTextHandle > 0) && strcmp(Status.MultiLangExt,Extension) == 0)
      return(0);

    memset(&PcbTextFile,0,sizeof(PcbTextFile));
  #endif

  NewHandle = 0;
  maxstrcpy(MultiLangExt,Extension,sizeof(Status.MultiLangExt));
  buildstr(FileName,PcbData.TxtLoc,"PCBTEXT",MultiLangExt,NULL);

top:
  #ifndef LIB
  if (! PcbData.FastText) {
  #endif
    if ((NewHandle = dosopen(FileName,OPEN_READ|OPEN_DENYNONE POS2ERROR)) == -1)
      goto cantopen;

    dosread(NewHandle,&Buf,sizeof(Buf)-1 POS2ERROR);  /*lint !e534 */

    #ifdef LIB
      Size = doslseek(NewHandle,0,SEEK_END);
      if (strstr(PcbData.Version,"15.0") != NULL) {
        if (strstr(Buf.Str,PCBVERSION) == NULL)
          goto wrongversion;
        if (Size < (TXT_NUMPROMPTS+1) * (sizeof(Buf)-1))
          goto needtoupgrade;
      } else {
        if (strstr(Buf.Str,PCBVERSION145) == NULL)
          goto wrongversion;
        if (Size < (TXT_NUM145PROMPTS+1) * (sizeof(Buf)-1))
          goto needtoupgrade;
      }
    #else
      if (strstr(Buf.Str,PCBVERSION) == NULL)
        goto wrongversion;

      if (doslseek(NewHandle,0,SEEK_END) < (TXT_NUMPROMPTS-TXT_NEWPROMPTS+1)*(sizeof(Buf)-1))
        goto needtoupgrade;
    #endif

    if (PcbTextHandle > 0)        /* if PCBTEXT was already open then */
      dosclose(PcbTextHandle);    /* close the original file handle   */

    PcbTextHandle = NewHandle;

    if (PcbData.MultiLingual) {
      /* if we are set for multilingual then use a SEPARATE file handle */
      /* for the system language */
      if (SystemHandle == 0 && CalledFrom == PROGRAM && Extension[0] == 0)
        if ((SystemHandle = dosopen(FileName,OPEN_READ|OPEN_DENYNONE POS2ERROR)) == -1)
          errorexittodos("Cannot open system PCBTEXT file");
    } else {
      /* otherwise, if not multilingual, then just share one file handle */
      /* between both the caller's pcbtext and the system's pcbtext file */
      SystemHandle = NewHandle;
    }

    maxstrcpy(Status.MultiLangExt,MultiLangExt,sizeof(Status.MultiLangExt));
    return(0);
  #ifndef LIB
  }

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&PcbTextFile) == -1)
    goto cantopen;

  if (dosfseek(&PcbTextFile,0,SEEK_END) < (TXT_NUMPROMPTS-TXT_NEWPROMPTS+1)*(sizeof(Buf)-1))
    goto needtoupgrade;

  dosrewind(&PcbTextFile);
  dossetbuf(&PcbTextFile,16384);

  #ifdef DEBUG
    mc_check_buffers();
  #endif

  if (dosfread(&Buf,sizeof(Buf)-1,&PcbTextFile) != sizeof(Buf)-1 || strstr(Buf.Str,PCBVERSION) == NULL)
    goto wrongversion;

  if (PcbData.MultiLingual) {
    /* if we are set for multilingual then use a SEPARATE file handle */
    /* for the system language */
    if (SystemHandle == 0 && CalledFrom == PROGRAM && Extension[0] == 0)
      if ((SystemHandle = dosopen(FileName,OPEN_READ|OPEN_DENYNONE POS2ERROR)) == -1)
        errorexittodos("Cannot open system PCBTEXT file");
  } else {
    /* otherwise, if not multilingual, then just share the preloaded pcbtext */
    /* file and we'll save both a file handle AND increase performance */
    SystemHandle = 0;
  }

  if (PcbTextBuffer != NULL) {
    PcbTextLoaded = FALSE;
    free(PcbTextBuffer);
    PcbTextBuffer = NULL;
  }

  /* add 1 for safety, but we'll realloc() to shrink it down below anyway */
  if ((PcbTextBuffer = malloc(sizeof(Buf) * (TXT_NUMPROMPTS+1))) == NULL)
    goto goslow;

  for (Num = 1, ActualSize = 0, p = (char *) PcbTextBuffer; Num <= TXT_NUMPROMPTS-TXT_NEWPROMPTS; Num++) {
    #ifdef DEBUG
      mc_check_buffers();
    #endif
    if (dosfread(&Buf,sizeof(Buf)-1,&PcbTextFile) != sizeof(Buf)-1)
      goto goslow;
    if ((Len = processtext(&Buf,Num)) != 0) {
      ActualSize += Len;
      memcpy(p,&Buf,Len);
      PcbText[Num] = (pcbtexttype *) p;
      p += Len;
    }
  }

  #if TXT_NEWPROMPTS > 0
    for (; Num <= TXT_NUMPROMPTS; Num++) {
      if ((Len = processtext(&Buf,Num)) != 0) {
        ActualSize += Len;
        memcpy(p,&Buf,Len);
        PcbText[Num] = (pcbtexttype *) p;
        p += Len;
      }
    }
  #endif

  /* shorten the buffer down to just what we need */

  if ((p = (char *) realloc(PcbTextBuffer,ActualSize)) == NULL)
    goto goslow;

  /* this should never happen, but in case it does, if the new buffer is in */
  /* a different location from the old buffer, then re-load PcbText[] */

  if (p != PcbTextBuffer) {
    for (PcbTextBuffer = p, Num = 1; Num <= TXT_NUMPROMPTS; Num++) {
      PcbText[Num] = (pcbtexttype *) p;
      if (isset(UseText,Num) != 0)
        p = strchr(p+1,0) + 1;  /*lint !e613 skip over color byte, search for NULL terminator, point to next character */
    }
  }

  PcbTextLoaded = TRUE;
  dosfclose(&PcbTextFile);
  maxstrcpy(Status.MultiLangExt,MultiLangExt,sizeof(Status.MultiLangExt));
  return(0);
  #endif

cantopen:
  #ifndef LIB
    if (CalledFrom == USERSELECT) {
      displaypcbtext(TXT_LANGNOTAVAIL,NEWLINE);
      return(-1);
    }
  #endif
  strcat(FileName," is unavailable!");
  errorexittodos(FileName);

wrongversion:
  #ifndef LIB
    dosfclose(&PcbTextFile);
  #endif
  if (NewHandle > 0)
    dosclose(NewHandle);
  #ifndef LIB
    if (CalledFrom == USERSELECT) {
      displaypcbtext(TXT_LANGNOTAVAIL,NEWLINE);
      return(-1);
    }
  #endif
  strcat(FileName," is the wrong version!   Run MKPCBTXT to convert it.");
  errorexittodos(FileName);

needtoupgrade:
  #ifndef LIB
    dosfclose(&PcbTextFile);
  #endif
  if (NewHandle > 0)
    dosclose(NewHandle);
  #ifndef LIB
    if (CalledFrom == USERSELECT) {
      displaypcbtext(TXT_LANGNOTAVAIL,NEWLINE);
      return(-1);
    }
  #endif
  strcat(FileName," needs to be upgraded!  Run MKPCBTXT.");
  errorexittodos(FileName);

goslow:;
  #ifndef LIB
    if (PcbTextBuffer != NULL) {
      PcbTextLoaded = FALSE;
      free(PcbTextBuffer);
      PcbTextBuffer = NULL;
    }
    PcbData.FastText = FALSE;
    dosfclose(&PcbTextFile);
    goto top;
  #endif
}


bool LIBENTRY getpcbtext(int PcbTextNum, pcbtexttype *Buffer) {
  #ifndef LIB
  #ifdef PCBCOMM
    int Num;
  #endif
  int Len;

  if (PcbData.FastText && PcbTextLoaded) {
    #ifdef DEBUG
      if (PcbText[PcbTextNum] == NULL) {
        char Str[80];
        sprintf(Str,"Error: Null Source for PCBTEXT #%d",PcbTextNum);
        writedebugrecord(Str);
        memset(Buffer,0,sizeof(pcbtexttype));
        return(FALSE);
      }
    #endif
    fmemcpy(Buffer,PcbText[PcbTextNum],sizeof(pcbtexttype));
  } else {
  #endif
    doslseek(PcbTextHandle,(long) PcbTextNum*(sizeof(pcbtexttype)-1),SEEK_SET);
    readcheck(PcbTextHandle,Buffer,sizeof(pcbtexttype)-1);  /*lint !e534 */
    processtext(Buffer,PcbTextNum);  /*lint !e534 */
  #ifndef LIB
  }

  if (Status.AppendText) {
    Len = strlen(Buffer->Str);
    maxstrcpy(&Buffer->Str[Len],Status.DisplayText,sizeof(Buffer->Str) - Len);
    Status.AppendText = FALSE;
    return(TRUE);
  }

  #ifdef PCBCOMM
  if (Status.TerseMode) {
    switch(PcbTextNum) {
      case TXT_FILELISTCOMMAND:
      case TXT_FILENUMEXPERT:
      case TXT_FILENUMNOVICE:
      case TXT_FILELISTCMDEXPRT:
             Num = numdirs(Status.DisplayText);  /* fil with the name of DIR.LST */
             comma(Status.DisplayText,Num);      /* Replace with number */
             return(TRUE);
      case TXT_BLTLISTCOMMAND:
      case TXT_BLTLISTCMDEXPERT:
             comma(Status.DisplayText,numblts());
             return(TRUE);
      case TXT_MSGSCANCOMMAND:
      case TXT_MSGREADCOMMAND:
      case TXT_MSGREADCMDEXPRT:
      case TXT_MSGSCANCMDEXPERT:
             substitute(Buffer->Str,"@OPTEXT@",Status.DisplayText,sizeof(Buffer->Str));
             return(TRUE);
      case TXT_JOINCONFNUM:
             ascii(Status.DisplayText,PcbData.NumConf);
             return(TRUE);
    }
  }
  #endif
  #endif

  return(substitute(Buffer->Str,"@OPTEXT@",Status.DisplayText,sizeof(Buffer->Str)));
}


bool LIBENTRY getsystext(int PcbTextNum, pcbtexttype *Buffer) {
  int Len;

  #ifndef LIB
  if (PcbData.FastText && PcbTextLoaded && !PcbData.MultiLingual) {
    /* if we preloaded PCBTEXT *and* we're not set for multilingual */
    /* operation, then we're just sharing the PCBTEXT buffer        */
    fmemcpy(Buffer,PcbText[PcbTextNum],sizeof(pcbtexttype));
  } else {
  #endif
    /* otherwise, the SystemHandle is pointing to the file we want to read */
    /* we don't care whether it is a shared file handle, it won't matter   */
    /* we just need to read in the PCBTEXT record and process it           */
    doslseek(SystemHandle,(long) PcbTextNum*(sizeof(pcbtexttype)-1),SEEK_SET);
    readcheck(SystemHandle,Buffer,sizeof(pcbtexttype)-1);  /*lint !e534 */
    processtext(Buffer,PcbTextNum);    /*lint !e534 */
  #ifndef LIB
  }
  #endif

  if (Status.AppendText) {
    Len = strlen(Buffer->Str);
    maxstrcpy(&Buffer->Str[Len],Status.DisplayText,sizeof(Buffer->Str) - Len);
    Status.AppendText = FALSE;
    return(TRUE);
  }

  return(substitute(Buffer->Str,"@OPTEXT@",Status.DisplayText,sizeof(Buffer->Str)));
}


#ifndef LIB
void LIBENTRY getpcbtextshowcolor(int PcbTextNum, pcbtexttype *Buffer) {
  getpcbtext(PcbTextNum,Buffer);
  if (Control.GraphicsMode && Buffer->Color != 0)
    printcolor(Buffer->Color);
}
#endif


void LIBENTRY logsystext(int PcbTextNum, padtype Pad) {
  pcbtexttype Old;
  pcbtexttype New;

  getsystext(PcbTextNum,&Old);
  xlatetext(New.Str,Old.Str);
  writelog(New.Str,Pad);
}


bool LIBENTRY displaypcbtext(int PcbTextNum, DISPLAYTYPE DispCtrl) {
  char         Save;
  bool         AppendText;
  bool         StopChar;
  int          StrLen;
  char        *p;
  char        *q;
  pcbtexttype  Buffer;
  #ifndef LIB
  char         Temp[66];
  #endif

  #ifdef PCBCOMM
    bool OpText;
    onlinetype SaveOnline;

    SaveOnline = Asy.Online;
    AppendText = Status.AppendText;
    OpText = getpcbtext(PcbTextNum,&Buffer);
  #else
    AppendText = Status.AppendText;
    getpcbtext(PcbTextNum,&Buffer);
  #endif

  if (Buffer.Str[0] == 0 && DispCtrl & NOTBLANK)
    return(FALSE);

  if (DispCtrl & BELL)
    bell();

  if (DispCtrl & LFBEFORE)
    newline();

  if (Control.GraphicsMode && Buffer.Color != 0)
    printcolor(Buffer.Color);

  for (p = Buffer.Str; *p == ' '; p++);

  #if defined(COMM) && defined(PCBCOMM)
  {
    char TerseText[10];

    if (Status.TerseMode) {
      sprintf(TerseText,"TXT%d",PcbTextNum);
      if (Asy.Online == REMOTE && ! Asy.LostCarrier) {
        /* the information here will be sent out the COMM port only */
        sendstr(TerseText,strlen(TerseText));
        if (OpText) {
          sendbyte('');
          sendstr(Status.DisplayText,strlen(Status.DisplayText));
        }
        sendbyte('');
        Asy.Online = LOCAL;                      /* force LOCAL ONLY mode  */
      } else if (Asy.Online == LOCAL) {
        print(TerseText);
        if (OpText) {
          print("");
          print(Status.DisplayText);
        }
        println("");
      }

/* if we're about to run a PPE or an MNU, then we need to restore */
/* the online status in case further interaction with the caller will */
/* be required by the PPE or MNU */
      if ((*p == '!' && *(p+1) != '|') || *p == '$') {
        Asy.Online = SaveOnline;  /* restore true online status */
        redisplaystatusline();
      }
    }
  }
  #endif


  StopChar = FALSE;
  if (Status.WatchForStopChar && (StrLen = strlen(Buffer.Str)) > 0 && Buffer.Str[StrLen-1] == TXT_STOPCHAR) {
    Buffer.Str[StrLen-1] = 0;
    StopChar = TRUE;
  }

  switch (*p) {
    case '%': if ((q = strpbrk(p+1," +")) != NULL) {
                Save = *q;
                *q = 0;
              }
              displayfile(p+1,GRAPHICS|SECURITY|LANGUAGE);
              if (q != NULL) {
                /* only display what follows if it was separated by a space */
                /* if separated by a "+" then log it to disk properly       */
                if (Save == ' ') {  /*lint !e644  Save won't get used unless q!=NULL */
                  if (Control.GraphicsMode && Buffer.Color != 0)
                    printcolor(Buffer.Color);
                  printxlated(q+1);
                }
                *q = Save;
              }
              break;
    #ifndef LIB
    case '!': if (*(p+1) != '|') {
                if (runscriptwithparams(p+1))
                  break;
                printxlated(Buffer.Str);
              } else
                printxlated(Buffer.Str);
              break;
    case '$': maxstrcpy(Temp,p+1,sizeof(Temp));
              if (checkenvfileexist(Temp,sizeof(Temp)) != 255)
                doMenu(Temp,0);   /*lint !e534 */
              else
                printxlated(Buffer.Str);
              break;
    #endif
    default : printxlated(Buffer.Str);
              break;
  }

  #if defined(COMM) && defined(PCBCOMM)
    if (Asy.Online == LOCAL && Asy.Online != SaveOnline) {
      Asy.Online = SaveOnline;              /* restore true online status */
      redisplaystatusline();
    }
  #endif

  if (DispCtrl & NEWLINE)
    newline();
  if (DispCtrl & LFAFTER)
    newline();

  if (DispCtrl & (LOGIT|LOGITLEFT)) {
    if (PcbTextNum == TXT_LANGACTIVE)
      writelog(Buffer.Str,(DispCtrl & LOGITLEFT ? LEFTJUSTIFY : SPACERIGHT));
    else {
      if (AppendText)
        Status.AppendText = TRUE;
      logsystext(PcbTextNum,(DispCtrl & LOGITLEFT ? LEFTJUSTIFY : SPACERIGHT));
    }
  }

  return(StopChar);
}


#ifndef LIB
void LIBENTRY substpcbtext(int PcbTextNum, DISPLAYTYPE DispCtrl, char *Search, char *Replace) {
  pcbtexttype Buffer;

  getpcbtext(PcbTextNum,&Buffer);

  if (DispCtrl & BELL)
    bell();

  if (DispCtrl & LFBEFORE)
    newline();

  if (Control.GraphicsMode && Buffer.Color != 0)
    printcolor(Buffer.Color);

  substitute(Buffer.Str,Search,Replace,sizeof(Buffer.Str));
  printxlated(Buffer.Str);

  if (DispCtrl & NEWLINE)
    newline();
  if (DispCtrl & LFAFTER)
    newline();
}
#endif


bool LIBENTRY pcbtextspaces(int TextNum) {
  int Len;
  pcbtexttype Spaces;

  getpcbtext(TextNum,&Spaces);
  Len = strlen(Spaces.Str) + 1;
  if (Spaces.Str[Len-2] == TXT_STOPCHAR)
    return(TRUE);

  memset(Spaces.Str,' ',Len);
  Spaces.Str[Len] = 0;
  print(Spaces.Str);
  return(FALSE);
}
