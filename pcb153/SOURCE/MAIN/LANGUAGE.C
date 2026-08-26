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

extern countrytype SystemCountry;
static char SystemLanguage;

static int _NEAR_ LIBENTRY processinput(char *Buf) {
  change(Buf,';','_');   /* change real semicolons to underlines          */
  change(Buf,' ','_');   /* change spaces to underlines for tokenizestr() */
  change(Buf,',',';');   /* change commas to semicolons for tokenizestr() */
  addchar(Buf,' ');      /* add a space in case last parameter is blank   */
  return(tokenizestr(Buf));
}


static int _NEAR_ LIBENTRY scanlangfile(DOSFILE *File, bool Show, int *Current) {
  int  Count;
  char *p;
  char Str[80];
  char Buf[80];

  #ifndef LIB
    if (Show) {
      displaycmdfile("PREML");

      if (displaycmdfile("ML") == 0)
        Show = FALSE;
      else {
        displaypcbtext(TXT_LANGAVAIL,NEWLINE|LFAFTER);
        printcolor(PCB_CYAN);
      }
    }
  #endif

  Count = 0;
  SystemLanguage = 0;
  *Current = 0;
  dosrewind(File);

  while (dosfgets(Str,sizeof(Str)-1,File) != -1) {
    Count++;

    strcpy(Buf,Str);
    if (processinput(Buf) == 0)
      break;

    if (Show) {
      if ((p = strchr(Str,',')) != NULL && p != Str)
        *p = 0;
      println(Str);
    }

    getnexttoken();      /* throw away language description    */
    p = getnexttoken();  /* get the language extension         */
    stripright(p,' ');   /* remove the space we added up above */

    if (SystemLanguage == 0 && *p == 0)  /* have we already found the default (no .EXT)? */
      SystemLanguage = (char) Count;     /* nope, this must be it so record it           */

    if (strcmp(p,Status.MultiLangExt) == 0)  /* does this match our current EXT */
      *Current = Count;                      /* yes, record this as current     */
  }

  return(Count);
}


static int _NEAR_ LIBENTRY openlanguagefile(DOSFILE *File, callfromtype CalledFrom) {
  if (PcbData.MultiLang[0] == 0 || fileexist(PcbData.MultiLang) == 255 || dosfopen(PcbData.MultiLang,OPEN_READ|OPEN_DENYNONE,File) == -1) {
    PcbData.MultiLingual = FALSE;
    if (CalledFrom == USERSELECT)
      displaypcbtext(TXT_LANGALTNOTAVAIL,NEWLINE|LFBEFORE);
    return(-1);
  }
  return(0);
}


static int _NEAR_ LIBENTRY readlanguagefile(DOSFILE *File, int LangNum, callfromtype CalledFrom) {
  int     Count;
  int     Cntry;
  int     CdPage;
  int     NumTokens;
  char    *p;
  char    Str[80];
  #ifndef LIB
  char    Debug[80];
  #endif

  Count = 0;
  dosfseek(File,0,SEEK_SET);

  /* loop until we find the right language to read, according to the */
  /* language NUMBER which was passed to us */

  for (Count = 0; Count != LangNum; Count++)
    if (dosfgets(Str,sizeof(Str),File) == -1)
      return(-1);

  if ((NumTokens = processinput(Str)) == 0) {
    #ifndef LIB
      if (DebugLevel > 0)
        writedebugrecord("LANG: No Tokens");
    #endif
    return(-1);
  }

  /* first parameter is a description of the language, throw it away */
  getnexttoken();

  if (--NumTokens == 0) {  /* we *need* an extension, error if one not there */
    #ifndef LIB
      if (DebugLevel > 0) {
        sprintf(Debug,"LANG: No Ext [%s]",Str);
        writedebugrecord(Debug);
      }
    #endif
    return(-1);
  }

  /* next parameter is the EXTENSION for the language */
  p = getnexttoken();
  stripright(p,' ');
  if (readpcbtextfile(p,CalledFrom) == -1) {
    #ifndef LIB
      if (DebugLevel > 0)
        writedebugrecord("LANG: Error reading pcbtext");
    #endif
    return(-1);
  }

  if (--NumTokens == 0)   /* it's NOT an error to not have the rest... */
    return(0);

  /* next parameter is the country code */
  Cntry = atoi(getnexttoken());

  if (--NumTokens == 0)
    return(0);

  /* next parameter is the code page */
  CdPage = atoi(getnexttoken());

  getcountryspecs(Cntry,CdPage);

  if (CalledFrom == USERSELECT && Asy.Online == LOCAL && Status.StatLine != NONE) {
    SystemCountry = Country;
    redisplaystatusline();
  }

  #ifndef LIB
    if (DebugLevel > 0) {
      sprintf(Debug,"Country %d, CodePage %d",CurrentCountry,CodePage);
      writedebugrecord(Debug);
    }
  #endif

  if (--NumTokens == 0)
    return(0);

  /* next parameter is the YES character for yes/no questions */
  YesChar = *getnexttoken();

  #ifndef LIB
    if (DebugLevel > 0) {
      sprintf(Debug,"YesChar = %c",YesChar);
      writedebugrecord(Debug);
    }
  #endif

  if (--NumTokens == 0)
    return(0);

  /* next parameter is the NO character for yes/no questions */
  NoChar = *getnexttoken();

  #ifndef LIB
    if (DebugLevel > 0) {
      sprintf(Debug,"NoChar = %c",NoChar);
      writedebugrecord(Debug);
    }
  #endif

  return(0);
}


#ifndef LIB
void LIBENTRY getlanguage(int NumTokens) {
  int     Picked;
  int     Current;
  int     Count;
  int     Len;
  char    *OldPointer;
  char    *p;
  char    *Mask;
  char    Str[15];
  DOSFILE File;

/*Status.Language = 0; */
  if (! PcbData.MultiLingual || openlanguagefile(&File,USERSELECT) == -1)
    return;

  savetokenpointer(&OldPointer);
  Count = scanlangfile(&File,FALSE,&Current);
  restoretokenpointer(&OldPointer);

  while (1) {
    if (NumTokens) {
      p = getnexttoken();
      NumTokens = 0;
    } else {
      if (Status.Language == 0 && SystemLanguage != 1) {
        readlanguagefile(&File,1,USERSELECT);    /*lint !e534 */
        Status.Language = 1;
      }
      scanlangfile(&File,TRUE,&Current);    /*lint !e534 */
      Str[0] = 0;

      Len  = 2;
      Mask = mask_numbers;
      #if defined(FIDO) && defined(COMM)
        if (PcbData.EnableFido) {
          Mask = mask_alphanum;
          Len = sizeof(Str)-1;
        }
      #endif

      inputfield(Str,TXT_LANGENTERNUMBER,Len,NEWLINE|LFBEFORE|UPCASE,HLP_LANG,Mask);

      #if defined(FIDO) && defined(COMM)
        checkforfidoresponse();
      #endif

      p = Str;
      if (*p == 0)
        break;
    }

    Picked = atoi(p);
    if (Picked < 0 || Picked > Count) {
      displaypcbtext(TXT_INVALIDSELECTION,NEWLINE|LFBEFORE|LFAFTER);
      continue;
    }

    if (Picked == 0)
      break;

    if (readlanguagefile(&File,Picked,USERSELECT) == -1) {
      displaypcbtext(TXT_INVALIDSELECTION,NEWLINE|LFBEFORE|LFAFTER);
    } else {
      displaypcbtext(TXT_LANGACTIVE,NEWLINE|LOGIT);
      Status.Language = (char) Picked;
      Status.CmdLst[0] = 0;
      if (Status.LoggingIn == NOTNOW)
        loadcmds();  /* while online, force it to re-load the CMD.LST file */
      break;
    }
  }

  dosfclose(&File);
}
#endif


int LIBENTRY getdefaultlanguage(void) {
  DOSFILE File;

  /* if it has already been read in then don't bother doing it again */
  if (Status.Language == 1)
    return(0);

  if (! PcbData.MultiLingual || openlanguagefile(&File,PROGRAM) == -1)
    return(-1);

  if (readlanguagefile(&File,1,PROGRAM) == -1) {
    dosfclose(&File);
    return(-1);
  }

  Status.Language = 1;
  dosfclose(&File);
  return(0);
}


int LIBENTRY getsystemlanguage(void) {
  int     Current;
  DOSFILE File;

  /* if it has already been read in then don't bother doing it again */
  if (SystemLanguage != 0 && Status.Language == SystemLanguage)
    return(0);

  if (! PcbData.MultiLingual || openlanguagefile(&File,PROGRAM) == -1)
    return(-1);

  scanlangfile(&File,FALSE,&Current);  /*lint !e534 */
  if (SystemLanguage == 0)
    errorexittodos("Error!  PCBML.DAT does not contain a default (blank extension) entry.");

  if (readlanguagefile(&File,SystemLanguage,PROGRAM) == -1) {
    dosfclose(&File);
    return(-1);
  }
  Status.Language = SystemLanguage;
  dosfclose(&File);
  return(0);
}


int LIBENTRY setlanguage(int LangNum) {
  int     Current;
  DOSFILE File;

  if (! PcbData.MultiLingual || openlanguagefile(&File,PROGRAM) == -1)
    return(-1);

  scanlangfile(&File,FALSE,&Current);  /*lint !e534 */
  if (Current != LangNum) {
    if (readlanguagefile(&File,LangNum,PROGRAM) == -1) {
      dosfclose(&File);
      return(-1);
    }
  }
  Status.Language = (char) LangNum;
  dosfclose(&File);
  return(0);
}


void LIBENTRY getlanguagespecifics(void) {
  DOSFILE File;

  if (! PcbData.MultiLingual || openlanguagefile(&File,PROGRAM) == -1)
    return;

  readlanguagefile(&File,Status.Language,USERSELECT);   /*lint !e534 */
  dosfclose(&File);
}
