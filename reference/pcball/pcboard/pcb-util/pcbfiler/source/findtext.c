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
#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <country.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "unique.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define MAXLINES   60
typedef enum { LN_FILE, LN_DUPE, LN_HEAD, LN_TEXT, LN_SPEC } dirlinetype;

#define movestr(dest,srce,size) memcpy(dest,srce,size); dest[size] = 0;

typedef struct {
  char        Str[81];          /* DIR text read in from disk */
} dirtype;

typedef struct {
  dirlinetype Type;              /* type of text */
  int         StrLen;            /* length of the text in dirtype */
  unsigned    Date;              /* date of file if it's a file listing */
} linetype;

// int  NumNonDupes;

char SrchCriteria[1024];
char SrchText[1024];
static bool ShowTemp;
static char SrchForm[13];
static char SearchInput[80];

void pascal massagesrchcriteria(void) {
  int  Len;
  char *p;
  char *LastChar;

  stripright(SrchCriteria,' ');
  Len = strlen(SrchCriteria);
  if (Len > 0 && Len < 12) {
    LastChar = &SrchCriteria[Len-1];
    if ((p = strchr(SrchCriteria,'.')) == NULL) {
      if (Len < 8 && *LastChar != '*')
        addchar(LastChar,'*');
      strcat(LastChar,".*");
    } else if (strlen(p) < 4 && *LastChar != '.' && *LastChar != '*' && *LastChar != '?')
      addchar(LastChar,'*');
  }
}


int pascal showline(char *Str, int ColorNum) {
  char Ch;
  bool Ext;

  LineCounter++;
  Str[76] = 0;   /* make sure it's not longer than 75 characters */
  if (LineCounter >= 20 && ! Work.NonStop) {
    fastprintmove(2,22,"press any key to continue (ESC exits)",Colors[HELPKEY]);
    Ch = inkey(&Ext,CLOCK);
    LineCounter = 1;
    clsbox(2,22,57,22,Colors[DISPLAY]);
  } else {
    Ch = 0;
    Ext = 0;
  }

  if (ShowTemp) {
    clsbox(2,22,77,22,Colors[DISPLAY]);
    ShowTemp = FALSE;
  }

  if (strlen(Str) > 75)
    Str[75] = 0;

  fastprint(2,22,Str,ColorNum);
  scrollup(2,3,77,22,Colors[DISPLAY]);
  gotoxy(2,22);
  if (Ch == 27 && Ext == 0)
    return(-1);
  return(0);
}


static int near pascal showtemp(char *Str, int ColorNum) {
  ShowTemp = TRUE;
  Str[76] = 0;   /* make sure it's not longer than 75 characters */
  fastprint(2,22,Str,ColorNum);
  return(userabort() ? -1 : 0);
}


/*
bool pascal foundinlist(char *DirPath, char *List) {
  int  Count;

  if (DirPath[0] == 0)
    return(TRUE);

  for (Count = 0; Count < NumNonDupes; Count++, List += DIRNAME_LENGTH) {
    if (strcmp(List,DirPath) == 0)
      return(TRUE);
  }

  if (NumNonDupes < NUM_NONDUPE_DIRS) {
    NumNonDupes++;
    strcpy(List,DirPath);
  }

  return(FALSE);
}
*/


static dirlinetype near pascal getlinetype(dirtype *Buffer, linetype *Line) {
  char OldChar;
  char *p;

  if (Buffer->Str[0] == ' ') {                                 /* is the filename blank? */
    if ((p = strnchr(Buffer->Str,'|',Line->StrLen)) != NULL) { /* is it a DUPE line? */
      *p = ' ';
      return(LN_DUPE);
    }
    return(LN_TEXT);
  }

  if (Line->StrLen >= 30) {
    OldChar = Buffer->Str[31];
    Line->Date = ctod2(&Buffer->Str[23]);
    Buffer->Str[31] = OldChar;
    if (Line->Date != 0)          /* is it a file listing? */
      return(LN_FILE);
    else switch (Buffer->Str[23]) {
       case 'A': if (memcmp(&Buffer->Str[24],"RCH",3) == 0)     return(LN_FILE);
                 break;
       case 'D': if (memcmp(&Buffer->Str[24],"ELETE",5) == 0)   return(LN_FILE);
                 break;
       case 'O': if (memcmp(&Buffer->Str[24],"FF-LINE",7) == 0) return(LN_FILE);
                 break;
    }
  }
  return(LN_TEXT);
}


static int near pascal displaydirline(dirtype *Buffer, linetype *Line, int NumLines) {
  char     Temp[20];
  unsigned TestValue;

  removecodes(Buffer->Str);
  if (Buffer->Str[0] != ' ' && strlen(Buffer->Str) >= 30) {
    memcpy(Temp,&Buffer->Str[23],8); Temp[8] = 0;
    switch(TestValue = ctod2(Temp)) {
      case      0:
      case 0xFFFE:
      case 0xFFFF: break;
      default    : countrydate(dtoc(TestValue,Temp));
                   memcpy(&Buffer->Str[23],Temp,8);
                   break;
    }
  }

  for (; NumLines; NumLines--, Buffer++) {
    if (showline(Buffer->Str,Colors[ANSWER]) == -1 || userabort())
      return(-1);
  }
  return(0);
}


static int near pascal filescanline(dirtype *Buffer, linetype *Line, int NumLines) {
  int  Len;
  char Temp[13];
  char *p;

  movestr(Temp,Buffer,12);
  stripright(Temp,' ');
  Len = strlen(Temp);
  p = strnchr(Temp,'.',Len);
  if (p == NULL) {           /* if there wasn't a period in the name then */
    if (Len > 8)             /*   is the name less than 8 characters?     */
      return(0);
  } else {                  /* is the name longer than 8 or the extension longer than 3 characters? */
    if ((p > &Temp[8]) || (&Temp[Len] - p > 4))
      return(0);
  }
  strupr(Temp);
  formatwild(Temp);
  if (equalwilds(SrchForm,Temp))
    return(displaydirline(Buffer,Line,NumLines));

  return(0);
}


static int near pascal zippyscanline(dirtype *Buffer, linetype *Line, int NumLines) {
  if (parsersearch((char *)Buffer,sizeof(dirtype) * NumLines,SearchInput,FALSE,0))
    return(displaydirline(Buffer,Line,NumLines));
  return(0);
}


static int near pascal displaydirfile(DirListType *Rec, char *Str) {
  int       NumLines;
  int       RetVal;
  dirtype  *p;
  linetype *q;
  DOSFILE   InFile;
  int near  pascal (*func)(dirtype *Buffer, linetype *Line, int NumLines);
  linetype  Lines[MAXLINES];
  dirtype   Buffer[MAXLINES];

  func = (MenuSelection == 5 ? filescanline : zippyscanline);
  RetVal = 0;

  if (fileexist(Rec->DirPath) == 255 || dosfopen(Rec->DirPath,OPEN_READ|OPEN_DENYNONE,&InFile) == -1) {
    strcat(Str,"unable to open file");
    if (showline(Str,Colors[HEADING]) == -1)
      return(0);
  } else {
    strcat(Str,Rec->DirDesc);
    if (showline(Str,Colors[QUESTION]) == -1) {
      dosfclose(&InFile);
      return(-1);
    }

    dossetbuf(&InFile,8192);
    NumLines = 0;
    RetVal   = 0;
    p = Buffer;
    q = Lines;

    while (1) {
      memset(p->Str,0,sizeof(p->Str)); /* set it all to zeroes because parsersearch() checks the whole thing */
      if (dosfgets(p->Str,sizeof(p->Str),&InFile) == -1)
        break;

      q->StrLen = strlen(p->Str);
      q->Type   = getlinetype(p,q);
      if (p != Buffer && Lines[0].Type == LN_FILE) {
        switch (q->Type) {
          case LN_SPEC:
          case LN_TEXT:
          case LN_HEAD:  RetVal = func(Buffer,Lines,NumLines);
                         /* func(p,q,1); */
                         NumLines = 0;
                         p = Buffer;
                         q = Lines;
                         break;
          case LN_FILE:  RetVal = func(Buffer,Lines,NumLines);
                         Buffer[0] = *p;
                         Lines[0] = *q;
                         NumLines = 1;
                         p = &Buffer[1];
                         q = &Lines[1];
                         break;
          case LN_DUPE:  if (NumLines < MAXLINES-1) {
                           NumLines++;
                           p++;
                           q++;
                         }
                         break;
        }
      } else {
        switch (Lines[0].Type) {
          case LN_SPEC:
          case LN_TEXT:
          case LN_HEAD:  /* func(Buffer,Lines,1); */
                         break;
          case LN_DUPE:  RetVal = func(Buffer,Lines,1);
                         break;
          case LN_FILE:  NumLines++;
                         p++;
                         q++;
                         break;
        }
      }
      if (RetVal == -1)
        break;
    }

    if (p != Buffer && RetVal != -1)
      func(Buffer,Lines,NumLines);

    dosfclose(&InFile);
  }
  return(RetVal);
}



void pascal scandirs(void) {
  bool            OldNonStop;
  unsigned        BoardNumber;
  unsigned        HighNum;
  unsigned        Num;
  unsigned        NumTextDirs;
  int             DirLstFile;
  uniquelist      DirList(DIRLIST_LENGTH,NUM_NONDUPE_LSTS);
  uniqueunlimited DirName(DIRNAME_LENGTH);
  char            Temp[81];
  DirListType     Rec;
  pcbconftype     Conf;

  if (! DirList.Allocated)
    return;
  if (! DirName.Allocated)
    return;

  boxcls(2,14,77,23,Colors[OUTBOX],SINGLE);
  fastprint(4,21,"NOTE:  The search scans through all of the DIR files on the system.",Colors[ANSWER]);
  setcursor(CUR_NORMAL);

  strcpy(Temp,"Scan DIR files for ");
  if (MenuSelection == 5) {
    fastprint(4,16,"You may locate any file within the listings by entering a single"     ,Colors[QUESTION]);
    fastprint(4,17,"file name or by using DOS wild card conventions (e.g. *.TXT, A?.T?T).",Colors[QUESTION]);
    inputstr(4,19,26,"Enter File Search Specification",SrchCriteria,SrchCriteria,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
    massagesrchcriteria();
    strcpy(SrchForm,SrchCriteria);
    formatwild(SrchForm);
    if (KeyFlags == ESC || SrchCriteria[0] == 0)
      return;
    strcpy(&Temp[19],SrchCriteria);
  } else {
    fastprint(4,16,"You may locate any text within the listings by typing in text as you",Colors[QUESTION]);
    fastprint(4,17,"would for a PCBoard Zippy Dir Scan (including & and | characters).",Colors[QUESTION]);
    inputstr(4,19,52,"Enter Search Text",SrchText,SrchText,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
    stripright(SrchText,' ');
    if (KeyFlags == ESC || SrchText[0] == 0)
      return;
    strcpy(&Temp[19],SrchText);
    if (tokenscan(SrchText,SearchInput,FALSE) <= 0)
      return;
  }

  clscolor(Colors[OUTBOX]);
  generalscreen(Temp,"");

  OldNonStop   = Work.NonStop;
  Work.NonStop = FALSE;
  LineCounter  = 0;
//NumNonDupes  = 0;
  DirLstFile   = -1;

  for (BoardNumber = 0; BoardNumber < PcbData.NumAreas; BoardNumber++) {
    getconfrecord(BoardNumber,&Conf);
    if (! DirList.foundinlist(Conf.DirNameLoc) && fileexist(Conf.DirNameLoc) != 255) {
      DirLstFile = dosopencheck(Conf.DirNameLoc,OPEN_READ|OPEN_DENYWRIT);
      NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
      HighNum = NumTextDirs;
      if (Conf.UpldDir[0] != 0)
        HighNum++;
      for (Num = 1; Num <= HighNum; Num++) {
        if (fastfinddirpath(Num,&Rec,&Conf,NumTextDirs,DirLstFile) != -1) {
          if (! DirName.foundinlist(Rec.DirPath)) {
            sprintf(Temp,"Conf: %-4u Num: %-4u ",BoardNumber,Num);
            if (displaydirfile(&Rec,Temp) == -1)
              goto abort;
          }
        }
      }
      if (DirLstFile != -1)
        dosclose(DirLstFile);
    }
  }
  LineCounter = 99;
  showline("",0);       //lint !e534

abort:
  if (DirLstFile != -1)
    dosclose(DirLstFile);
  Work.NonStop = OldNonStop;
  if (MenuSelection != 5)
    stopsearch();
}


void pascal scandisk(void) {
  bool            OldNonStop;
  unsigned        BoardNumber;
  DOSFILE         PathFile;
  uniquelist      PthList(DIRLIST_LENGTH,NUM_NONDUPE_LSTS);
  uniqueunlimited PthName(DIRNAME_LENGTH);
  char            Temp[128];
  char            Show[128];
  pcbconftype     Conf;

  if (! PthList.Allocated)
    return;
  if (! PthName.Allocated)
    return;

  boxcls(2,14,77,23,Colors[OUTBOX],SINGLE);
  fastprint(4,16,"You may locate any file in the Download Paths by entering a single"   ,Colors[QUESTION]);
  fastprint(4,17,"file name or by using DOS wild card conventions (e.g. *.TXT, A?.T?T).",Colors[QUESTION]);
  fastprint(4,21,"NOTE:  The search scans through all of the Download Paths defined."   ,Colors[ANSWER]);
  setcursor(CUR_NORMAL);
  inputstr(4,19,26,"Enter File Search Specification",SrchCriteria,SrchCriteria,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
  massagesrchcriteria();

  if (KeyFlags == ESC || SrchCriteria[0] == 0)
    return;

  strcpy(Temp,"Scan Download Paths for ");
  strcpy(&Temp[24],SrchCriteria);

  clscolor(Colors[OUTBOX]);
  generalscreen(Temp,"");

  OldNonStop   = Work.NonStop;
  Work.NonStop = FALSE;
  LineCounter  = 0;
//NumNonDupes  = 0;

  for (BoardNumber = 0; BoardNumber < PcbData.NumAreas; BoardNumber++) {
    getconfrecord(BoardNumber,&Conf);
    if (! PthList.foundinlist(Conf.PthNameLoc)) {
      sprintf(Temp,"%s conference downloads",Conf.Name);
      if (showline(Temp,Colors[HEADING]) == -1)
        goto abort;
      if (dosfopen(Conf.PthNameLoc,OPEN_READ|OPEN_DENYNONE,&PathFile) != -1) {
        dossetbuf(&PathFile,2048);
        while (dosfgets(Show,80,&PathFile) != -1) {
          if (showtemp(Show,Colors[QUESTION]) == -1) {
            dosfclose(&PathFile);
            goto abort;
          }

          if (! PthName.foundinlist(Show)) {
            if (findfileindlpathrecord(Show,SrchCriteria) == -1) {
              dosfclose(&PathFile);
              goto abort;
            }
          }
        }
        dosfclose(&PathFile);
      }
    }
  }

  LineCounter = 99;
  showline("",0);       //lint !e534
abort:
  Work.NonStop = OldNonStop;
}
