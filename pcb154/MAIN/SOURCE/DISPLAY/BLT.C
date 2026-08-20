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

#ifdef __WATCOMC__
  #include <borland.h>
#endif

#define BLTBUFSIZE       16*1024

typedef struct {
  char NamePath[31];                   /* location of the BLT file */
} bltlisttype;

static bool NewFiles = FALSE;

/* DTA is declared in exist.c from the MISC library */
extern struct ffblk DTA;


int LIBENTRY numblts(void) {
  return(numrandrecords(Status.CurConf.BltNameLoc,sizeof(bltlisttype)-1));
}


static void _NEAR_ LIBENTRY displaybuffer(char *Buffer, int BytesInBuffer) {
  int StrLen;

  while (BytesInBuffer > 0) {
    printfoundtext(Buffer);
    newline();
    if (Display.AbortPrintout)
      break;

    StrLen = strlen(Buffer) + 1;
    Buffer += StrLen;
    BytesInBuffer -= StrLen;
  }
}


static void _NEAR_ LIBENTRY showbltheader(DOSFILE *InFile) {
  long SavePos;
  char Str[81];

  // save the current file pointer position
  SavePos = dosfseek(InFile,0,SEEK_CUR);
  // go back to the top of the file
  dosrewind(InFile);

  while (dosfgets(Str,sizeof(Str),InFile) != -1) {
    println(Str);

    // display only up to the separator, then get out
    if (Str[0] == '-' || Str[0] == '=' || Str[0] == '+')
      break;

    if (Display.AbortPrintout)
      break;
  }

  // restore the file pointer position
  dosfseek(InFile,SavePos,SEEK_SET);
}



/********************************************************************
*
*  Function:  searchfile()
*
*  Desc    :  Searches a file for text matching the input criteria using
*             an 8K buffer for AND/OR searches.  If a match is found anywhere
*             in the file then the entire file is displayed.
*
*  Returns :  -1 if the display was aborted
*             -2 if the file (or text) wasn't found
*              0 in all other cases
*/

static int _NEAR_ LIBENTRY searchfile(char *Name) {
  bool      EndOfFile;
  bool      UseSeparator;
  bool      DisplayedHeader;
  bool      ReadingHeader;
  int       BytesLeft;
  int       StrLen;
  int       RetVal;
  char     *p;
  char     *Buffer;
  DOSFILE   InFile;

  if (Name[0] == 0)
    return(-2);

  if (fileexist(Name) == 255)
    return(-2);

  if ((Buffer = (char *) checkmalloc(BLTBUFSIZE,"SEARCH FILE")) == NULL)
    return(-2);

  DisplayedHeader = FALSE;
  UseSeparator = FALSE;
  ReadingHeader = TRUE;
  RetVal = -2;     // default to indicate text was not found
  memset(Buffer,0,BLTBUFSIZE);

  if (dosfopen(Name,OPEN_READ|OPEN_DENYNONE,&InFile) != -1) {
    while (1) {
      for (BytesLeft = BLTBUFSIZE, EndOfFile = FALSE, p = Buffer;
           TRUE;              //lint !e506
           p += StrLen) {
        if (dosfgets(p,BytesLeft,&InFile) == -1) {
          EndOfFile = TRUE;
          break;
        }
        StrLen = strlen(p)+1;      // add 1 for NUL byte
        BytesLeft -= StrLen;
        if (BytesLeft < 80)
          break;
        if (p[0] == '-' || p[0] == '=' || p[0] == '+') {
          UseSeparator = TRUE;
          break;
        }
      }

      if (! UseSeparator)
        ReadingHeader = FALSE;

      showactivity(ACTSHOW);
      if (parsersearch(Buffer,BLTBUFSIZE-BytesLeft,Status.SearchInput,FALSE,0)) {
        RetVal = 0;           // indicate we found what we were looking for
        if (Status.SendBlts)  // if we're downloading,
          break;              //   then just exit with RetVal=0


        // If the bulletin is using separator lines, then the top portion of
        // the bulletin is the header so we need to display the header to the
        // caller before displaying what was found.  Do that by saving the
        // current location, go back to the top, display up to the header,
        // then return back to where we left off and resume.
        if (UseSeparator && ! DisplayedHeader) {
          showactivity(ACTSUSPEND);
          showbltheader(&InFile);
          showactivity(ACTRESUME);
          DisplayedHeader = TRUE;
        }

        // if we show that we're still reading the header, then the
        // showbltheader function must have displayed the information
        // already, so skip the call to displaybuffer()
        if (ReadingHeader) {
        } else {
          showactivity(ACTSUSPEND);
          displaybuffer(Buffer,BLTBUFSIZE-BytesLeft);
          showactivity(ACTRESUME);
        }

        if (Display.AbortPrintout) {
          RetVal = -1;
          break;
        }
      }

      ReadingHeader = FALSE;

      if (EndOfFile)
        break;
    }

    dosfclose(&InFile);
  }

  Status.OldStatLine = NONE;  /* force it to redisplay the status line */
  checkdisplaystatus();
  bfree(Buffer);
  return(RetVal);
}



void LIBENTRY checkbulletins(void) {
  int         Count;
  int         NumBlts;
  bool        Displayed;
  DOSFILE     In;
  bltlisttype Blt;
  char        Str[80];

  /* don't check for new bulletins if the user can't read them! */
  if (Status.CurSecLevel < PcbData.UserLevels[SEC_B])
    return;

  /* don't check for new bulletins if we've already checked it */
  if (previousconfmatchfound(Status.CurConf.BltNameLoc))
    return;

  if ((NumBlts = numblts()) > 0) {
    if (dosfopen(Status.CurConf.BltNameLoc,OPEN_READ|OPEN_DENYNONE,&In) != -1) {
      displaypcbtext(TXT_SCANNINGBLTS,LFBEFORE);
      showactivity(ACTBEGIN);
      for (Count = 1, Displayed = FALSE; Count <= NumBlts; Count++) {
        if (dosfread(Blt.NamePath,sizeof(bltlisttype)-1,&In) == sizeof(bltlisttype)-1) {
          showactivity(ACTSHOW);
          Blt.NamePath[sizeof(bltlisttype)-1] = 0;
          stripright(Blt.NamePath,' ');
          if (getfiledatetime(Blt.NamePath) >= Status.LastDateTime) {
            showactivity(ACTSUSPEND);
            if (! Displayed) {
              makeconfstr(Str,Status.Conference);
              backupcleareol(awherex());
              newline();
              printcolor(PCB_RED);
              print(Str);
              displaypcbtext(TXT_BULLETINSUPDATED,NEWLINE);
              displaypcbtext(TXT_NEWBULLETINS,DEFAULTS);
              Displayed = TRUE;
            }
            sprintf(Str," %d",Count);
            print(Str);
            showactivity(ACTRESUME);
          }
        }
        checkstatus();
        if (Display.AbortPrintout)
          break;
      }
      checkdisplaystatus();
      showactivity(ACTEND);
      newline();
      dosfclose(&In);
    }
  }
}


static bool _NEAR_ LIBENTRY readbltfile(int Num, bltlisttype *Blt) {
  int  File;
  long Offset;
  char FileName[66];
  char Str[80];

  if (checkforalternatelist(FileName,Status.CurConf.BltNameLoc,sizeof(bltlisttype)-1) == -1) {
    sprintf(Str,"%s not found or invalid size",Status.CurConf.BltNameLoc);
    writelog(Str,SPACERIGHT);
    return(FALSE);
  }

  if ((File = openfile(FileName)) == -1)
    return(FALSE);

  Offset = ((long) sizeof(bltlisttype)-1) * (Num-1);
  if (doslseek(File,Offset,SEEK_SET) != Offset) {
    dosclose(File);
    return(FALSE);
  }

  if (readcheck(File,Blt->NamePath,sizeof(bltlisttype)-1) == (unsigned) -1) {
    dosclose(File);
    return(FALSE);
  }

  Blt->NamePath[sizeof(bltlisttype)-1] = 0;
  stripright(Blt->NamePath,' ');
  dosclose(File);

  if (NewFiles && getfiledatetime(Blt->NamePath) < Status.LastDateTime)
    return(FALSE);
  return(TRUE);
}


static void _NEAR_ LIBENTRY getbltname(int Num) {
  char        *p;
  char        *q;
  int         AddRetVal;
  bltlisttype Blt;
  #if defined(__BORLANDC__) || defined(__TURBOC__)
    struct    ffblk FoundInfo;
  #else
    struct    find_t FoundInfo;
  #endif
  char        AltName[66];
  char        Str[80];
  pcbtexttype Buf;
  spectype    File;

  if (readbltfile(Num,&Blt)) {
    if (Status.NumTaggedFiles < Status.BatchLimit) {
      if (fileexist(Blt.NamePath) != 255) {
        strcpy(AltName,Blt.NamePath);
        getalternatename(AltName,LANGUAGE,0);
        if (AltName[0] == 0)
          strcpy(AltName,Blt.NamePath);

        if (Status.SearchText[0] == 0 || searchfile(AltName) != -2) {
          /* pick out only the FILENAME portion of the path+filename */
          if ((p = strrchr(AltName,'\\')) == NULL)
            p = AltName;
          else
            p++;

          if (PcbData.ViewExt[0] != 0 && strchr(p,'.') == NULL) {
            q = p + strlen(p);
            *q = '.';
            strcpy(q+1,PcbData.ViewExt);
            if (fileexist(AltName) == 255)
              *q = 0;
          }

          if (fileexist(AltName) != 255) {
            *p = 0;
            FoundInfo = DTA;  /* don't trust the DTA information to remain intact */
            if ((AddRetVal = addfiletolist(FoundInfo.ff_name,AltName,FoundInfo.ff_fsize,FALSE,FALSE)) >= 0) {
              if (getfilespec(AddRetVal,&File) != -1) {
                File.Found = TRUE;
                putfilespec(AddRetVal,&File);
                shownamesizeandtime(AddRetVal,1024);
                getsystext(TXT_BULLETINREAD,&Buf);
                sprintf(Str,"%s%s # %d",Buf.Str,Status.CurConf.Name,Num);
                writelog(Str,SPACERIGHT);
              }
            }
          }
        }
      }
    }
  }
}


static int _NEAR_ LIBENTRY displaybltfile(int Num) {
  int         RetVal;
  bltlisttype Blt;
  pcbtexttype Buf;
  char        Str[80];

  if (Num == 0 || Num > numblts()) {
    displaypcbtext(TXT_INVALIDBLTNUM,NEWLINE|LFBEFORE);
    return(-2);
  }

  RetVal = 0;
  if (readbltfile(Num,&Blt)) {
    if (Status.SearchText[0] != 0)
      RetVal = searchfile(Blt.NamePath);
    else
      RetVal = displayfile(Blt.NamePath,GRAPHICS|LANGUAGE);
    if (RetVal != -2) {
      getsystext(TXT_BULLETINREAD,&Buf);
      sprintf(Str,"%s%s # %d",Buf.Str,Status.CurConf.Name,Num);
      writelog(Str,SPACERIGHT);
    }
  }
  return(RetVal);
}


/********************************************************************
*
*  Function:  showblt()
*
*  Desc    :  Displays the specified bulletin
*
*             This function was added for script/menu functions.  It allows
*             the caller to view bulletins ONLY IF the caller has sufficient
*             security to do so.
*
*  Returns :   0 the bulletin was displayed in its entirety
*             -1 the bulletin was displayed but was aborted before finishing
*             -2 invalid bulletin number or file not found
*             -3 insufficient security to view bulletins
*/

int LIBENTRY showblt(int BltNum) {
  if (seclevelokay("B",PcbData.UserLevels[SEC_B]))
    return(displaybltfile(BltNum));
  return(-3);
}


void LIBENTRY bltmenu(int NumTokens) {
  int   NumBlts;
  int   Num;
  char  *p;
  bool  Search;
  bool  ShowMenu;
  bool  Prompted;
  char  Str[10];
  char  Command[128];
  char  PeekBuf[128];
  char  Numbers[2048];

  if ((NumBlts = numblts()) == 0) {
    displaypcbtext(TXT_NOBLTSAVAILABLE,NEWLINE|LFBEFORE|BELL);
    return;
  }

  Prompted = FALSE;
  ShowMenu = (bool) (NumTokens == 0);

  while (1) {
    if (NumTokens == 0) {
      if (ShowMenu) {
        startdisplay(NOCHANGE);
        displayfile(Status.CurConf.BltMenu,GRAPHICS|LANGUAGE|RUNMENU|RUNPPL);
        ShowMenu = FALSE;
      }
      Prompted = TRUE;
      Command[0] = 0;
      inputfield(Command,(UsersData.ExpertMode ? TXT_BLTLISTCMDEXPERT : TXT_BLTLISTCOMMAND),sizeof(Command)-1,UPCASE|STACKED|NEWLINE|LFBEFORE,HLP_B,mask_blts);
      NumTokens = tokenize(Command);
      if (NumTokens == 0)
        return;
    }

    peekatnexttoken(PeekBuf,sizeof(PeekBuf)-1);
    if (PeekBuf[1] == 0) {
      switch (PeekBuf[0]) {
        case 'G': getnexttoken();  /* we peeked, now get rid of it */
                  byecommand();
                  break;
        case 'R':
        case 'L': ShowMenu = TRUE;
                  NumTokens = 0;
                  continue;
        case '?':
        case 'H': displayhelpfile(HLP_B);
                  NumTokens = 0;
                  continue;
        case 'N': if (NumTokens == 1 && Prompted)
                    return;
      }
    }

    Search = FALSE;
    NewFiles = FALSE;
    Status.SearchText[0] = 0;
    Numbers[0] = 0;
    for (; NumTokens > 0; NumTokens--) {
      p = getnexttoken();
      if (alldigits(p)) {
        strcat(Numbers,p);
        addchar(Numbers,' ');
      } else if (*(p+1) == 0) {
        switch (*p) {
          case 'N': NewFiles = TRUE;
                    /* fall thru */
          case 'A': for (Num = 1; Num <= NumBlts; Num++) {
                      ascii(Str,Num);
                      strcat(Numbers,Str);
                      addchar(Numbers,' ');
                    }
                    break;
          case 'D': if (seclevelokay("D",PcbData.UserLevels[SEC_D])) {
                      allocatefilelist(DOWNLOAD);   //lint !e534
                      Status.SendBlts = TRUE;
                    }
                    break;
          case 'S': Search = TRUE;
                    break;
        }
      } else if (strcmp(p,"GB") == 0 || strcmp(p,"BYE") == 0)
        Status.CapBye = TRUE;
      else
        maxstrcpy(Status.SearchText,p,sizeof(Status.SearchText));
    }

    if (Search && Status.SearchText[0] == 0)
      inputfield(Status.SearchText,TXT_TEXTTOSCANFOR,sizeof(Status.SearchText)-1,UPCASE|NEWLINE|LFBEFORE|HIGHASCII,NOHELP,mask_alphanum);

    stripright(Numbers,' ');
    if (Search && Status.SearchText[0] != 0) {
      if (tokenscan(Status.SearchText,Status.SearchInput,FALSE) == -1) {
        displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
        goto end;
      }
      showactivity(ACTBEGIN);
      if (Numbers[0] == 0)
        for (Num = 1; Num <= NumBlts; Num++) {
          ascii(Str,Num);
          strcat(Numbers,Str);
          addchar(Numbers,' ');
        }
    } else Status.SearchText[0] = 0;

    NumTokens = tokenizestr(Numbers);

    startdisplay(NOCHANGE);

    for (; NumTokens > 0; NumTokens--) {
      p = getnexttoken();
      if (Status.SendBlts)
        getbltname(atoi(p));
      else if (displaybltfile(atoi(p)) == -1 && Status.SearchText[0] == 0)
        break;
    }


    if (Status.SearchText[0] != 0) {
      stopsearch();
      showactivity(ACTEND);
      Status.SearchText[0] = 0;
    }
    Display.CountLines = FALSE;  /* don't do more prompt after we're done */

    if (Status.SendBlts) {
      if (NumFiles != 0)
        send(0);
      else
        Status.SendBlts = FALSE;
      break;
    }

    NumTokens = 0;
  }

end:
  Status.SendBlts = FALSE;
}
