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

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <io.h>
  #include <direct.h>
  #include <malloc.h>
  #include <borland.h>
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

/* NOTE:  this file was split off from LOG.C so that the routines */
/* could be moved into the OVERLAY section of the executable     */

#define MAXLOGFILES   50
#define NUMTEXTCOLORS 27

typedef struct {
  short Num;
  char  Color;
} textcolortype;

static textcolortype Text[NUMTEXTCOLORS] = {
  {TXT_COMPLETEDUSING   ,PCB_WHITE },
  {TXT_COMMENTLEFT      ,PCB_YELLOW},
  {TXT_MESSAGELEFT      ,PCB_CYAN  },
  {TXT_ABORTEDUSING     ,PCB_RED   },
  {TXT_NOTFOUNDONDISK   ,PCB_RED   },
  {TXT_DIRECTORYSCAN    ,0x06      },
  {TXT_CARRIERLOST      ,PCB_RED   },
  {TXT_TIMELIMITEXCEEDED,PCB_RED   },
  {TXT_UNAUTHORIZEDNAME ,PCB_RED   },
  {TXT_REALNAMESONLY    ,PCB_RED   },
  {TXT_LOCKEDOUT        ,PCB_RED   },
  {TXT_DENIEDWRONGPWRD  ,PCB_RED   },
  {TXT_DENIEDPWRDFAILED ,PCB_RED   },
  {TXT_NAMEALREADYINUSE ,PCB_RED   },
  {TXT_AUTOMATICLOCKOUT ,PCB_RED   },
  {TXT_KBDTIMEEXPIRED   ,PCB_RED   },
  {TXT_BADDOWNLOADPWRD  ,PCB_RED   },
  {TXT_SECURITYVIOL     ,PCB_RED   },
  {TXT_NOTREGINCONF     ,PCB_RED   },
  {TXT_INSUFSECTOVIEW   ,PCB_RED   },
  {TXT_MENUSELUNAVAIL   ,PCB_RED   },
  {TXT_INSUFSECFORDOOR  ,PCB_RED   },
  {TXT_INSUFSECTODLFILE ,PCB_RED   },
  {TXT_NOTIMEFORXFER    ,PCB_RED   },
  {TXT_USERRECNUMISBAD  ,PCB_RED   },
  {TXT_OPENEDDOOR       ,PCB_GREEN },
  {TXT_NOMATCHINPWRD    ,PCB_RED   }
};


static void _NEAR_ LIBENTRY initlogtext(pcbtexttype *LogText) {
  int X;

  checkstack();
  /* keep the @OPTEXT@ macro from disappearing */
  strcpy(Status.DisplayText,"@OPTEXT@");

  for (X = 0; X < NUMTEXTCOLORS; X++, LogText++) {
    getsystext(Text[X].Num,LogText);
    stripright(LogText->Str,' ');
    stripleft(LogText->Str,' ');

    // change SYSTIME and USER macros to OPTEXT so that the equaltext()
    // function, below, will be able to ignore the SYSTIME macro
    substitute(LogText->Str,"@SYSTIME@","@OPTEXT@",sizeof(LogText->Str));
    substitute(LogText->Str,"@USER@","@OPTEXT@",sizeof(LogText->Str));
  }
}


static bool _NEAR_ LIBENTRY equaltext(char *Str, char *LogText) {
  checkstack();
  if (strstr(LogText,"@OPTEXT@") == NULL)
    return((bool) (strstr(Str,LogText) != NULL));

  for (; *Str; Str++, LogText++) {
    if (*LogText == '@' && memcmp(LogText,"@OPTEXT@",8) == 0) {
      LogText += 8;
      return((bool) (strstr(Str,LogText) != NULL));
    }
    if (*Str != *LogText)
      return(FALSE);
  }
  return(TRUE);
}


static char _NEAR_ LIBENTRY getlogcolor(char *Str, pcbtexttype *LogText) {
  int  X;

  checkstack();
  if (strstr(Str,"Batch ") != NULL) {
    if (Str[6] == 'U')
      return(PCB_WHITE);
    if (Str[6] == 'D')
      return(0x03);
  }

  if (strstr(Str,LogText->Str) != NULL) {  /* check file transfer first */
    if (Str[1] == 'U')
      return(PCB_WHITE);
    else
      return(0x03);
  }

  for (X = 1, LogText++; X < NUMTEXTCOLORS; X++, LogText++) {
    if (equaltext(Str,LogText->Str))
      return(Text[X].Color);
  }
  return(0);
}


static void _NEAR_ LIBENTRY displaylogline(logrectype *Rec, bool SearchActive, pcbtexttype *LogText) {
  char *p;
  char Color;

  checkstack();
  Rec->CrLf[0] = 0;
  stripright(Rec->Text,' ');

  if (! Control.GraphicsMode) {
    println(Rec->Text);
    return;
  }

  if (Rec->Text[0] == '*') {
    printcolor(Display.DefaultColor & 0xF0 ? 0x0D : 0x05);
    println(Rec->Text);
    return;
  }

  if (Rec->Text[0] != ' ') {
    printcolor(Display.DefaultColor & 0xF0 ? 0x0A : 0x02);
    printfoundtext(Rec->Text);
    newline();
    return;
  }

  p = &Rec->Text[6];
  if ((Color = getlogcolor(p,LogText)) != 0)
    printcolor(Color);
  else if (strchr(p,':') != NULL && strstr(p,"Error") != NULL)
    printcolor(PCB_RED);
  else printcolor(0x07);

  if (SearchActive) {
    printfoundtext(Rec->Text);
    newline();
  } else
    println(Rec->Text);
}


static void _NEAR_ LIBENTRY searchlog(char *Name, pcbtexttype *LogText) {
  bool        First;
  int         File;
  int         SessionRecs;
  int         MaxLogRecs;
  int         MaxSessionRecs;
  int         MaxRecs;
  int         BufSize;
  logrectype  *Buf;
  logrectype  *Session;
  logrectype  *p;
  logrectype  *q;
  long        FreeMem;
  long        NewPos;
  long        Offset;

  checkstack();
  if ((File = openfile(Name)) == -1)
    return;


  #ifdef __OS2__
    FreeMem = 128 * 1024;
    MaxRecs         = (int) (FreeMem / LOGRECLEN);
    MaxLogRecs      = MaxRecs - (MaxRecs / 3);
    MaxLogRecs     &= 0xFFFF0;  /* round it to a multiple of 16 */
  #else
    FreeMem = coreleft() - 4096;  // reserve at least 4K of memory
    if (FreeMem > 32768U)         // don't let it eat up more than 32K of memory
      FreeMem = 32768U;
    MaxRecs         = (int) (FreeMem / LOGRECLEN);
    MaxLogRecs      = MaxRecs - (MaxRecs / 3);
    MaxLogRecs     &= 0xFFF0;  /* round it to a multiple of 16 */
  #endif

  BufSize         = MaxLogRecs * LOGRECLEN;
  MaxSessionRecs  = MaxRecs - MaxLogRecs;

  if (MaxLogRecs <= 0 || MaxSessionRecs <= 0)
    return;

  if ((Buf = (logrectype *) bmalloc(BufSize)) == NULL)
    return;

  if ((Session = (logrectype *) bmalloc(MaxSessionRecs * LOGRECLEN)) == NULL) {
    bfree(Buf);
    return;
  }

  SessionRecs = 0;
  q = Session;
  First = TRUE;

  Offset = numrecs(File,sizeof(logrectype))-1;
  NewPos = ((Offset*sizeof(logrectype)) / BufSize) * BufSize;
  p = &Buf[(int) (Offset % MaxLogRecs)];
  doslseek(File,NewPos,SEEK_SET);

  if (readcheck(File,Buf,BufSize) != (unsigned) -1) {
    while (1) {
      showactivity(ACTSHOW);
      if ((int) p < (int) Buf) {       //lint !e507  ignore warning, this was needed
        /*checkstatus();*/
        NewPos -= BufSize;
        if (NewPos < 0)
          break;
        p = &Buf[MaxLogRecs-1];
        doslseek(File,NewPos,SEEK_SET);
        if (readcheck(File,Buf,BufSize) == (unsigned) -1)
          break;
      }
      if (p->Text[0] == '*') {
        if (parsersearch((char *)Session,SessionRecs*sizeof(logrectype),Status.SearchInput,FALSE,0) != 0) {
          showactivity(ACTSUSPEND);
          if (First) {
            newline();
            First = FALSE;
          }
          for (q = Session; SessionRecs; q++, SessionRecs--)
            displaylogline(q,TRUE,LogText);
          displaylogline(p,FALSE,LogText);
          showactivity(ACTRESUME);
        }
        SessionRecs = 0;
        q = Session;
      } else if (SessionRecs < MaxSessionRecs) {
        memcpy(q,p,sizeof(logrectype));
        q++;
        SessionRecs++;
      }
      p--;
      if (Display.AbortPrintout)
        break;
    }
  }

  bfree(Session);
  bfree(Buf);
  dosclose(File);
}


static void _NEAR_ LIBENTRY displaylog(char *Name, pcbtexttype *LogText) {
  int         File;
  int         MaxLogRecs;
  int         BufSize;
  logrectype  *Buf;
  logrectype  *p;
  long        NewPos;
  long        Offset;
  long        FreeMem;

  checkstack();
  if ((File = openfile(Name)) == -1)
    return;

  newline();

  #ifdef __OS2__
    FreeMem     = 128 * 1024;
    MaxLogRecs  = (int) (FreeMem / LOGRECLEN);
    MaxLogRecs &= 0xFFFF0;  /* round it to a multiple of 16 */
  #else
    FreeMem = coreleft() - 2048;
    if (FreeMem > 16384)  // don't let it eat up more than 16K of memory
      FreeMem = 16384;
    MaxLogRecs  = (int) (FreeMem / LOGRECLEN);
    MaxLogRecs &= 0xFFF0;  /* round it to a multiple of 16 */
  #endif

  BufSize = MaxLogRecs * LOGRECLEN;

  if (MaxLogRecs <= 0 || (Buf = (logrectype *) bmalloc(BufSize)) == NULL)
    return;

  Offset = numrecs(File,sizeof(logrectype))-1;
  NewPos = ((Offset*sizeof(logrectype)) / BufSize) * BufSize;
  p = &Buf[(int) (Offset % MaxLogRecs)];
  doslseek(File,NewPos,SEEK_SET);

  if (readcheck(File,Buf,BufSize) != (unsigned) -1) {
    while (1) {
      if ((int) p < (int) Buf) {       //lint !e507  ignore warning, this was needed
        /*checkstatus();*/
        NewPos -= BufSize;
        if (NewPos < 0)
          break;
        p = &Buf[MaxLogRecs-1];
        doslseek(File,NewPos,SEEK_SET);
        if (readcheck(File,Buf,BufSize) == (unsigned) -1)
          break;
      }
      displaylogline(p,FALSE,LogText);
      p--;
      if (Display.AbortPrintout)
        break;
    }
  }

  bfree(Buf);
  dosclose(File);
}


void LIBENTRY viewcallerslog(int NumTokens) {
  bool         Temp;
  char         Str[2];
  char        *p;
  char         Name[66];
  pcbtexttype  LogText[NUMTEXTCOLORS];

  checkstack();
  if (PcbData.Network)
    sprintf(Name,"%s%d",PcbData.ClrFile,PcbData.NodeNum);
  else
    strcpy(Name,PcbData.ClrFile);

  Status.SearchText[0] = 0;
  Temp = Status.Printer;

  if (NumTokens) {
    p = getnexttoken();
    NumTokens--;
  } else {
    Str[0] = 0;
    inputfield(Str,TXT_VIEWCALLERS,1,FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_viewlog);
    p = Str;
  }

  switch (*p) {
    case 'V': break;
    case 'P': Status.Printer = (bool) (prn > 0);
              startdisplay(FORCENONSTOP);
              break;
    case 'S': if (NumTokens == 0)
                inputfield(Status.SearchText,TXT_TEXTTOSCANFOR,sizeof(Status.SearchText)-1,UPCASE|NEWLINE|LFBEFORE|HIGHASCII,HLP_SRCH,mask_alphanum);
              else
                for (;  NumTokens; NumTokens--)
                  addtext(Status.SearchText,getnexttoken(),sizeof(Status.SearchText)-1);
              break;
    case 'D': Str[0] = NoChar;
              Str[1] = 0;
              inputfield(Str,TXT_DELETECALLERSLOG,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_yesno);
              if (Str[0] == YesChar) {
                #ifndef LIB
                  int SaveLevel = DebugLevel;
                  DebugLevel = 0;
                #endif
                closecallerlog();
                unlink(Name);
                openlog();
                writenametolog(LOGON);
                #ifndef LIB
                  DebugLevel = SaveLevel;
                #endif
              }
              return;
    default : return;
  }

  initlogtext(LogText);

  if (Status.SearchText[0] != 0) {
    showactivity(ACTBEGIN);
    if (tokenscan(Status.SearchText,Status.SearchInput,FALSE) == -1) {
      displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
      goto end;
    }
    searchlog(Name,LogText);
    stopsearch();
    Status.SearchText[0] = 0;
    showactivity(ACTEND);
  } else
    displaylog(Name,LogText);

end:
  logsystext(TXT_CALLERSLOGVIEWED,SPACERIGHT);
  Status.Printer = Temp;
}


static int _NEAR_ LIBENTRY countlogs(char *Name) {
  int  Count;
  int  NumFound;
  char Mask[66];
  struct ffblk blk[MAXLOGFILES];
  #ifdef __OS2__
  int  DirHandle = MAKEDIRHANDLE; //lint !e569
  #endif

  checkstack();
  strcpy(Mask,Name);
  strcat(Mask,"*.*");

  Count = 0;
  NumFound = MAXLOGFILES;

  if (dosfindfirst(Mask,blk,FA_ARCH,&NumFound PDIRHANDLE) == 0) {
    while (1) {
      Count += NumFound;

      // if the number found is less than the max then we're done
      if (NumFound < MAXLOGFILES)
        break;

      #ifndef __OS2__
        // the DOS version needs the file info in order to find the next one
        blk[0] = blk[NumFound-1];
      #endif

      if (dosfindnext(blk,&NumFound PDIRHANDLE2) == -1)
        break;
    }
  }

  #ifdef __OS2__
    dosclosedirhandle(DirHandle);
  #endif
  return(Count);
}


void LIBENTRY nodeviewcallerslog(int NumTokens) {
  bool        GotNum;
  bool        Search;
  bool        ScanAll;
  int         Num;
  int         Max;
  char       *p;
  char       *q;
  void _NEAR_ LIBENTRY (*viewlog)(char *Name, pcbtexttype *LogText);
  char        Str[10];
  char        Name[66];
  pcbtexttype LogText[NUMTEXTCOLORS];

  checkstack();
  Num     = PcbData.NodeNum;
  ScanAll = FALSE;
  GotNum  = FALSE;
  Search  = FALSE;
  Status.SearchText[0] = 0;
  strcpy(Name,PcbData.ClrFile);
  q = &Name[strlen(Name)];

top:
  if (NumTokens == 0) {
    displayusernet(0);
    Str[0] = 0;
    inputfield(Str,TXT_NODETOVIEW,sizeof(Str)-1,UPCASE|STACKED|NEWLINE|LFBEFORE,NOHELP,mask_nodeview);
    if (Str[0] == 0)
      return;
    NumTokens = tokenize(Str);
  }

  for (;  NumTokens; NumTokens--) {
    p = getnexttoken();
    if (alldigits(p)) {
      Num = atoi(p);
      if (Num >= 1 && Num <= PCB_MAXNODES)
        GotNum = TRUE;
    } else if (*p == 'S' && *(p+1) == 0) {
      Search = TRUE;
    } else if (*p == 'A' && *(p+1) == 0) {
      ScanAll = TRUE;
    } else
      addtext(Status.SearchText,p,sizeof(Status.SearchText)-1);
  }

  if (! (GotNum || ScanAll))
    goto top;

  if (Search && Status.SearchText[0] == 0)
    inputfield(Status.SearchText,TXT_TEXTTOSCANFOR,sizeof(Status.SearchText)-1,UPCASE|NEWLINE|LFBEFORE,HLP_SRCH,mask_alphanum);

  if (Status.SearchText[0] != 0) {
    showactivity(ACTBEGIN);
    if (tokenscan(Status.SearchText,Status.SearchInput,FALSE) == -1) {
      displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
      return;
    }
    viewlog = searchlog;
  } else
    viewlog = displaylog;

  initlogtext(LogText);

  if (ScanAll) {
    for (Num = 1, Max = countlogs(Name); Num <= PCB_MAXNODES && Max > 0 && ! Display.AbortPrintout; Num++) {
      ascii(q,Num);
      if (fileexist(Name) != 255) {
        viewlog(Name,LogText);
        Max--;
      }
    }
  } else {
    ascii(q,Num);
    viewlog(Name,LogText);
  }

  if (Status.SearchText[0] != 0) {
    stopsearch();
    Status.SearchText[0] = 0;
    showactivity(ACTEND);
  }
}


#ifdef PCB152
unsigned LIBENTRY searchlogfordoor(char *DoorName) {
  int         File;
  unsigned    RetVal;
  char       *p;
  long        RecNum;
  char        Temp[15];
  char        Buf[LOGRECLEN];
  pcbtexttype Rec;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  checkstack();
  if ((File = dosdup(CallersLog POS2ERROR)) == -1)
    return(0xFFFF);

  /* keep the @OPTEXT@ macro from disappearing */
  strcpy(Status.DisplayText,"@OPTEXT@");
  getsystext(TXT_OPENEDDOOR,&Rec);

  RetVal = 0xFFFF;
  RecNum = (doslseek(File,0,SEEK_END) / LOGRECLEN) - 1;
  while (--RecNum > 0) {
    doslseek(File,RecNum * LOGRECLEN,SEEK_SET);
    if (readcheck(File,Buf,LOGRECLEN) != (unsigned) -1) {
      if (Buf[0] > ' ') {
        RetVal = 0xFFFF;
        goto end;
      }

      Buf[LOGRECLEN-2] = 0;
      stripright(Buf,' ');
      if (equaltext(&Buf[6],Rec.Str)) {
        if ((p = strchr(Buf,':')) != NULL)
          RetVal = strtominutes(p-2);
        if ((p = strstr(Rec.Str,"@OPTEXT@")) != NULL) {
          maxstrcpy(Temp,Buf + 6 + (int) (p - Rec.Str),sizeof(Temp));
          if ((p = strstr(Temp,p+8)) != NULL)
            *p = 0;
          strcpy(DoorName,Temp);
        }
        break;
      }
    }
  }

end:
  doslseek(File,0,SEEK_END);
  dosclose(File);
  return(RetVal);
}
#endif
