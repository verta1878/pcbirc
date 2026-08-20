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

#include "wild.h"
#include <ctype.h>

#ifdef __WATCOMC__
  #include <io.h>
  #include <borland.h>
#endif

#define MAXLINES   60
#define MAXDIRS  2048
#define MAXARRAY  256  // must be 256 because the index uses a BYTE

#define printpaddedline(s) printit(s)

typedef enum { LN_FILE, LN_DUPE, LN_HEAD, LN_TEXT, LN_SPEC } dirlinetype;
typedef enum { SCAN_ALL, SCAN_NEW, SCAN_ZIP, SCAN_LOC } scantype;

#define NAMESIZE   8
#define EXTSIZE    3

#pragma pack(1)
typedef struct {
  char           Name[NAMESIZE];
  char           Ext[EXTSIZE];
  unsigned short Date;
  unsigned short LoOffset;
  char           HiOffset;
} filetype;

typedef struct {
  long           FileSize;
  unsigned short FileTime;
  unsigned short FileDate;
  unsigned short Latest;
  char           Reserved[6];
} idxheadertype;
#pragma pack()

typedef struct {
  char        Str[121];          /* DIR text read in from disk */
} dirtype;

typedef struct {
  dirlinetype    Type;           /* type of text */
  int            StrLen;         /* length of the text in dirtype */
  unsigned short Date;           /* date of file if it's a file listing */
} linetype;

typedef struct {                 /* structure used INTERNALLY only */
  char DirPath[31];              /* location of the DIR file */
  char DskPath[31];              /* path associated with the DIR file */
  char DirDesc[60];              /* description of the DIR file */
  char SortType;                 /* sort method (used by PCBFiler */
} dirlisttype;

#pragma pack(1)
typedef struct {                 /* structure of FILE on disk */
  char DirPath[30];              /* location of the DIR file */
  char DskPath[30];              /* path associated with the DIR file */
  char DirDesc[35];              /* description of the DIR file */
  char SortType;                 /* sort method (used by PCBFiler */
} dirlisttype2;
#pragma pack()

typedef struct {
  char Name[13];
  int  DirNum;
  long Offset;
} dirarraytype;


unsigned short static NewDate;   // date used for new files scan
int      static NumSearches;     // number of search words/phrases
char     static NewIndicator;    // character to display indicating new files
char     static Criteria[12];    // formatted string for wildmatch() searches
bool     static Colorize;        // TRUE if we need to color the DIR output
bool     static PastHeader;      // TRUE means we're past the DIR header
bool     static ZippyScan;       // TRUE means show zippy scan matches
bool     static CompleteList;    // TRUE means show %FILESPEC files
bool     static FlaggingFiles;   // TRUE means we're flagging "found" files for download
DOSFILE  static DirInFile;       // buffered file for displaying DIR files
DOSFILE  static IdxFile;         // index file for DIR files
bool     static UseIdx;          // TRUE means we're using the IDX file
bool     static ShowActivity;    // TRUE means we're scanning and need to show activity
char     static CurPos;          // used to keep track of current index into FilesDisplayed[]
long     static PrevOffset;      // used with the SINGLE LINE display mode
int      static CurDirNum;       // used with the SINGLE LINE display mode

static idxheadertype  IdxHeader;
static dirarraytype  *FilesDisplayed;

/* DTA is declared in exist.c from the MISC library */
extern struct ffblk DTA;


/********************************************************************
*
*  Function:  ctod2()
*
*  Desc    :  converts a text date of the form "mm-dd-yy" into an unsigned
*             integer
*
*  Notes   :  This function is *NOT* identical in behaviour to the function
*             in misc.lib that is called ctod().  Both functions return
*             the same values - but this function is limited in scope to only
*             accept dates in the form "mm-dd-yy" while the ctod() function
*             in misc.lib allows more flexible forms such as "m/d/yy".  Also,
*             this function does not recognize ARCHIVE or OFF-LINE files.
*
*  Returns :  An unsigned integer of where the first 7 bits are the year-80,
*             the next 4 bits are the month and the last 5 bits are the day.
*
*             The value for 01/01/80 is returned if the YEAR is set to "00".
*
*             A value of 0xFFFF is returned on text string of "==-==-==".
*/

static unsigned _NEAR_ LIBENTRY ctod2(char *Date) {
  int Month;
  int Day;
  int Year;

  if (Date[0] == '=')
    if (memcmp(&Date[1],"=-==-==",7) == 0)
      return(0xFFFF);

  if (Date[2] != '-' && Date[5] != '-')
    return(0);

  if (memcmp(Date,"00-00-00",8) == 0)
    return(1 + (1 << 5) + ((80-80) << 9));  /* change 00-00-00 to 01-01-80 */

  Month = (Date[0]-'0')*10 + (Date[1]-'0');
  if (Month < 1 || Month > 12)
    return(0);

  Day   = (Date[3]-'0')*10 + (Date[4]-'0');
  if (Day < 1 || Day > 31)
    return(0);

  Year  = (Date[6]-'0')*10 + (Date[7]-'0');
  if (Year < 0 || Year > 99)
    return(0);

  if (Year < 80 && Year >= 0)
    Year += 100;

  return(Day + (Month << 5) + ((Year-80) << 9));
}


/********************************************************************
*
*  Function:  numdirs()
*
*  Desc    :  first it checks for a security-specific DIR.LST file
*
*             if no DIR.LST file is found then NumDirs = 0 else NumDirs is
*             set to the number of records found in the DIR.LST file
*
*  Returns :  Number of records in DIR.LST plus 1 if the system is not set
*             for PRIVATE-ONLY uploads
*/

int LIBENTRY numdirs(char *DirList) {
  int  NumDirs;

  if (checkforalternatelist(DirList,Status.CurConf.DirNameLoc,sizeof(dirlisttype2)) == -1)
    NumDirs = 0;
  else
    NumDirs = numrandrecords(DirList,sizeof(dirlisttype2));

  #ifdef PCB_DEMO
    NumDirs = min(NumDirs,2);  /* limit to 2 download directories / 1 upload */
  #endif

  // if the PUBLIC UPLOAD DIR FILE is defined, then we will indicate it as an
  // available directory regardless of whether or not uploads are forced
  // private

  if (Status.CurConf.UpldDir[0] == 0)
    return(NumDirs);
  return(NumDirs+1);
}


/********************************************************************
*
*  Function:  printit()
*
*  Desc    :  the routines below call printit() instead of print() or
*             printxlated() so that it can determine ahead of time if
*             we were searching for something.  If we were then we call
*             printfoundtext() to highlight it on the screen.
*/

static void _NEAR_ LIBENTRY printit(char *Buffer) {
  if (ZippyScan)
    printfoundtext(Buffer);
  else
    printxlated(Buffer);
}


#ifndef printpaddedline
static void _NEAR_ LIBENTRY printpaddedline(char *Buffer) {
  int Num;
  int Tabs;

  if (UseAnsi && ! Status.Capture) {
    for (Num = 0; *Buffer == ' '; Num++, Buffer++);
    Tabs = Num / 8;
    Num -= Tabs * 8;
    for (; Tabs; Tabs--)
      print("\t");
    for (; Num; Num--)
      Buffer--;
  }
  printit(Buffer);
}
#endif


/********************************************************************
*
*  Function:  getlinetype()
*
*  Desc    :  checks *Str against the rules for DIR files to determine if the
*             string is a file listing, a text line, a dupe line or a header.
*
*  Returns :  LN_FILE, LN_DUPE, LN_HEAD or LN_TEXT
*/

static dirlinetype _NEAR_ LIBENTRY getlinetype(dirtype *Buffer, linetype *Line) {
  char *p;

  if (Buffer->Str[0] == ' ') {                                 /* is the filename blank? */
    if ((p = strnchr(Buffer->Str,'|',Line->StrLen)) != NULL) { /* is it a DUPE line? */
      *p = ' ';
      return(LN_DUPE);
    } else
      return((PastHeader ? LN_TEXT : LN_HEAD));
  }

  if (Line->StrLen >= 30) {
    Line->Date = (unsigned short) ctod2(&Buffer->Str[23]);
    if (Line->Date != 0) {         /* is it a file listing? */
      countrydate(&Buffer->Str[23]);
      PastHeader = TRUE;
      return(LN_FILE);
    } else {
      switch (Buffer->Str[23]) {
        case 'A': if (memcmp(&Buffer->Str[24],"RCH",3) == 0)     return(LN_FILE);
                  break;
        case 'C': if (memcmp(&Buffer->Str[24],"D-ROM",5) == 0)   return(LN_FILE);
                  break;
        case 'D': if (memcmp(&Buffer->Str[24],"ELETE",5) == 0)   return(LN_FILE);
                  break;
        case 'O': if (memcmp(&Buffer->Str[24],"FF-LINE",7) == 0) return(LN_FILE);
                  break;
      }
    }
  }

  /* 1st char wasn't blank so check for %FILESPEC or !FILESPEC.PPE */

  if (CompleteList && (Buffer->Str[0] == '%' || Buffer->Str[0] == '!') && fileexist(&Buffer->Str[1]) != 255)
    return(LN_SPEC);

  return((PastHeader ? LN_TEXT : LN_HEAD));
}


static void _NEAR_ LIBENTRY filespec(char *FileName) {
  switch (FileName[0]) {
    case '%': displayfile(&FileName[1],GRAPHICS|SECURITY|LANGUAGE); break;
    case '!': runscriptwithparams(&FileName[1]); break;    //lint !e534
  }

  if (Display.WasAborted)
    Display.AbortPrintout = TRUE;
}


/********************************************************************
*
*  Function:  displaydirline()
*
*  Desc    :  Displays a line of text from a DIR file
*/

static void LIBENTRY displaydirline(dirtype *Buffer, linetype *Line, int NumLines) {
  char Ch;

  if (ShowActivity)
    showactivity(ACTSUSPEND);

  freshline();

  // if SingleLine display is enabled, force the routine to display only
  // 1 line and just throw away the rest.
  if (Status.UseSingleLines) {
    NumLines = 1;
    if (FilesDisplayed != NULL) {
      // copy the filename into the array, INCLUDING the
      // spaces, because the memcpy() function is fast,
      // and instead of stripping spaces now, we'll just
      // pad with spaces on the search criteria
      memcpy(FilesDisplayed[++CurPos].Name,Buffer->Str,12);
      FilesDisplayed[CurPos].DirNum = CurDirNum;
      FilesDisplayed[CurPos].Offset = PrevOffset;
    }
  }

  pagebreak(NumLines);

  for (; NumLines; NumLines--, Buffer++, Line++) {
    if (! Colorize) {
      if (Line->Type == LN_FILE) {
        if (Line->Date >= UsersData.DateLastDirRead) {
          Buffer->Str[31] = 0;
          printit(Buffer->Str);
          Buffer->Str[31] = NewIndicator;
          printit(&Buffer->Str[31]);
          if (Line->Date > Status.LatestDate && Line->Date <= (unsigned) Status.CtodLogonDate)
            Status.LatestDate = Line->Date;
        } else printit(Buffer->Str);
      } else if (Line->Type == LN_SPEC) {
        filespec(Buffer->Str);
      } else if (Line->Type == LN_DUPE) {
        printpaddedline(Buffer->Str);
      } else
        printit(Buffer->Str);
    } else {
      switch (Line->Type) {
        case LN_SPEC: filespec(Buffer->Str); break;
        case LN_DUPE: printcolor(Status.DirColors[DUPE]); printpaddedline(Buffer->Str); break;
        case LN_TEXT: printcolor(Status.DirColors[TEXT]); printit(Buffer->Str); break;
        case LN_HEAD: printcolor(Status.DirColors[HEAD]); printit(Buffer->Str); break;
        case LN_FILE: if ((curcolor() & 0xF0) > 0 && (Status.DirColors[FNAM] & 0xF0) == 0) {
                        printcolor(Status.DirColors[FNAM]);
                        cleareol();
                      } else
                        printcolor(Status.DirColors[FNAM]);
                      Ch = Buffer->Str[12]; Buffer->Str[12] = 0; printit(Buffer->Str); Buffer->Str[12] = Ch;
                      printcolor(Status.DirColors[FSIZ]);
                      Ch = Buffer->Str[23]; Buffer->Str[23] = 0; printit(&Buffer->Str[12]); Buffer->Str[23] = Ch;
                      switch (Buffer->Str[23]) {
                        case 'A':
                        case 'O': Ch = (char) Status.DirColors[OFLN]; break;
                        case 'D': Ch = (char) Status.DirColors[DLTD]; break;
                        default : Ch = (char) Status.DirColors[FDTE]; break;
                      }
                      printcolor(Ch);
                      Ch = Buffer->Str[31]; Buffer->Str[31] = 0;
                      printit(&Buffer->Str[23]);
                      printcolor(Status.DirColors[NEWF]);
                      Buffer->Str[31] = (Line->Date >= UsersData.DateLastDirRead ? NewIndicator : Ch);
                      Ch = Buffer->Str[32]; Buffer->Str[32] = 0; printit(&Buffer->Str[31]);
                      printcolor(Status.DirColors[FDSC]);
                      Buffer->Str[32] = Ch; printit(&Buffer->Str[32]);
                      if (Line->Date > Status.LatestDate && Line->Date <= (unsigned) Status.CtodLogonDate)
                        Status.LatestDate = Line->Date;
                      break;
      }
      if ((curcolor() & 0xF0) != 0) {
        cleareol();
        printcolor(0x07);
      }
    }
    newline();
    if (FlaggingFiles && Line->Type == LN_FILE) {
      Buffer->Str[12] = 0;
      flagfilefordownload(Buffer->Str);
    }
  }

  if (ShowActivity)
    showactivity(ACTRESUME);
}


/********************************************************************
*
*  Function:  zippyscanline()
*
*  Desc    :  Displays a DIR line if it matches the criteria
*/

static void LIBENTRY zippyscanline(dirtype *Buffer, linetype *Line, int NumLines) {
  if (Line->Type == LN_FILE) {
    if (NewDate != 0 && Line->Date < NewDate)
      return;
    if (parsersearch((char *)Buffer,sizeof(dirtype) * NumLines,Status.SearchInput,FALSE,0))
      displaydirline(Buffer,Line,NumLines);
    else
      showactivity(ACTSHOW);
  }
}


/********************************************************************
*
*  Function:  newfilescanline()
*
*  Desc    :  Displays a DIR line if the file is considered 'new'.
*/

static void LIBENTRY newfilescanline(dirtype *Buffer, linetype *Line, int NumLines) {
  if (UseIdx || (Line->Type == LN_FILE && Line->Date >= NewDate))
    displaydirline(Buffer,Line,NumLines);
  else
    showactivity(ACTSHOW);
}


/********************************************************************
*
*  Function:  filescanline()
*
*  Desc    :  Displays a DIR line if it matches the locate criteria
*/

static void LIBENTRY filescanline(dirtype *Buffer, linetype *Line, int NumLines) {
  int  Len;
  char Temp[13];
  char *p;

  if (Line->Type == LN_FILE) {
    if (NewDate != 0 && Line->Date < NewDate) {
      showactivity(ACTSHOW);
      return;
    }

    if (UseIdx)
      displaydirline(Buffer,Line,NumLines);
    else {
      movestr(Temp,Buffer->Str,12);
      stripright(Temp,' ');
      Len = strlen(Temp);
      p = strnchr(Temp,'.',Len);
      if (p == NULL) {           /* if there wasn't a period in the name then */
        if (Len > 8)             /*   is the name less than 8 characters?     */
          return;
      } else {                  /* is the name longer than 8 or the extension longer than 3 characters? */
        int RootLen = (int) (p-Temp);
        int ExtLen  = (Len-1) - RootLen;  // subtract 1 from len to cover the period
        if ((RootLen > 8) || (ExtLen > 3))
          return;
      }
      if (wildmatch(Buffer->Str,Criteria))
        displaydirline(Buffer,Line,NumLines);
      else
        showactivity(ACTSHOW);
    }
  }
}


static void LIBENTRY locatenextnew(void) {
  filetype File;

  while (dosfread(&File,sizeof(File),&IdxFile) == sizeof(File)) {
    if (File.Date >= NewDate) {
      dosfseek(&DirInFile,File.LoOffset + ((long) File.HiOffset << 16),SEEK_SET);
      return;
    }
  }
  dosfseek(&DirInFile,0,SEEK_END);
}


static void LIBENTRY locatenextmatch(void) {
  filetype File;

  while (dosfread(&File,sizeof(File),&IdxFile) == sizeof(File)) {
    if (comparewildcards(Criteria,File.Name)) {
      dosfseek(&DirInFile,File.LoOffset + ((long) File.HiOffset << 16),SEEK_SET);
      return;
    }
  }
  dosfseek(&DirInFile,0,SEEK_END);
}


static bool _NEAR_ LIBENTRY fillidxrecord(filetype *File, char *Name, char *Str, long Offset) {
  unsigned short Date;

  Date = (unsigned short) ctod2(&Str[23]);
  if (Date == 0)
    return(FALSE);

  makeidxname(Name,File->Name);

  File->Date     = Date;
  File->LoOffset = (unsigned short) Offset;
  File->HiOffset = (char) ((Offset & 0xFF0000L) >> 16);
  return(TRUE);
}


void LIBENTRY updatedirindex(char *FileName, DOSFILE *File, char *Name, char *Str, long Offset) {
  filetype  Rec;

  if (fillidxrecord(&Rec,Name,Str,Offset)) {
    dosflush(File);
    dosfseek(File,0,SEEK_END);
    if (dosfwrite(&Rec,sizeof(Rec),File) != -1) {
      dosrewind(File);
      fileexist(FileName);   //lint !e534
      IdxHeader.FileSize = DTA.ff_fsize;
      IdxHeader.FileTime = DTA.ff_ftime;
      IdxHeader.FileDate = DTA.ff_fdate;
      if (Rec.Date > IdxHeader.Latest)
        IdxHeader.Latest = Rec.Date;
      dosfwrite(&IdxHeader,sizeof(IdxHeader),File);  //lint !e534
    }
  }
}


static void _NEAR_ LIBENTRY createdirindex(char *FileName, char *IdxName, DOSFILE *InFile) {
  unsigned short Latest;
  long           Offset;
  DOSFILE        OutFile;
  filetype       File;
  char           Str[128];

  if (dosfopen(IdxName,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&OutFile) != -1) {
    if (! ShowActivity)
      showactivity(ACTBEGIN);

    dossetbuf(&OutFile,16384);
    memset(&IdxHeader,0,sizeof(IdxHeader));
    dosfwrite(&IdxHeader,sizeof(IdxHeader),&OutFile);  //lint !e534

    Latest = 0;

    while (1){
      showactivity(ACTSHOW);
      Offset = dosfseek(InFile,0,SEEK_CUR);
      if (dosfgets(Str,sizeof(Str),InFile) == -1)
        break;

      if (Str[0] != ' ' && strlen(Str) >= 30) {
        if (fillidxrecord(&File,Str,Str,Offset)) {
          if (dosfwrite(&File,sizeof(File),&OutFile) == -1)
            break;
          if (File.Date > Latest)
            Latest = File.Date;
        }
      }
    }

    dosrewind(&OutFile);
    fileexist(FileName);     //lint !e534
    IdxHeader.FileSize = DTA.ff_fsize;
    IdxHeader.FileTime = DTA.ff_ftime;
    IdxHeader.FileDate = DTA.ff_fdate;
    IdxHeader.Latest   = Latest;
    dosfwrite(&IdxHeader,sizeof(IdxHeader),&OutFile); //lint !e534
    dosfclose(&OutFile);
    dosrewind(InFile);
    if (! ShowActivity)
      showactivity(ACTEND);
  }
}


bool LIBENTRY needtoresyncdirindex(char *FileName, DOSFILE *File) {
  if (dosfread(&IdxHeader,sizeof(IdxHeader),File) != sizeof(IdxHeader))
    return(TRUE);

  fileexist(FileName);       //lint !e534
  if (IdxHeader.FileSize != DTA.ff_fsize ||
      IdxHeader.FileTime != DTA.ff_ftime ||
      IdxHeader.FileDate != DTA.ff_fdate)
    return(TRUE);
  return(FALSE);
}


static bool _NEAR_ LIBENTRY idxfilenameokay(char *IdxName, char *FileName) {
  char *p;
  char *q;
  char Temp[13];

  if (FileName[1] == ':') {
    if (slowdrive(ReadOnlyDrives,(char) toupper(FileName[0])) != NULL)
      return(FALSE);
  }

  if ((p = strrchr(FileName,'\\')) != NULL)
    p++;
  else if ((p = strrchr(FileName,':')) != NULL)
    p++;
  else
    p = FileName;

  // check to see if the DIR filename has a period in it
  if ((q = strchr(p,'.')) != NULL) {
    // if it does, is the filename less than 7.3?  if not, get out now
    if (((int) (q - p)) > 7 || strlen(q+1) > 3)
      return(FALSE);

    // otherwise, change the filename by substituting X's in all of the
    // blank spots in an 8.3 format.
    strcpy(Temp,"XXXXXXXX.XXX");
    memcpy(Temp,p,(int) (q - p));
    memcpy(&Temp[9],q+1,strlen(q+1));
    strcpy(IdxName,FileName);
    strcpy(&IdxName[(int) (p - FileName)],Temp);
  } else {
    strcpy(IdxName,FileName);
    strcat(IdxName,".IDX");
  }
  return(TRUE);
}


/********************************************************************
*
*  Function:  displaydirfile()
*
*  Desc    :  Displays a DIR file
*/

static void _NEAR_ LIBENTRY displaydirfile(int DirNum, char *Name, scantype Type, void LIBENTRY (*func)(dirtype *Buf, linetype *Line, int Num)) {
  int           NumLines;
  long          CurOffset;
  dirtype      *p;
  linetype     *q;
  char          FileName[66];
  char          IdxName[66];
  linetype      Lines[MAXLINES];
  dirtype       Buffer[MAXLINES];

  PrevOffset = 0;
  CurDirNum  = DirNum;

  Colorize = (bool) (Control.GraphicsMode && Status.ColorizeDirFiles);

  strcpy(FileName,Name);

  #ifdef THIS_IS_COMMENTED_OUT
    getalternatename(FileName,LANGUAGE,0);

    /* getalternatename() sets the FileName to blanks if both the original */
    /* and any alternates are missing.  However, we're going to copy the   */
    /* original Name back into FileName so that the error message that is  */
    /* produced will make sense to the sysop reading the callers log       */
    if (FileName[0] == 0)
      strcpy(FileName,Name);
  #endif

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&DirInFile) != -1) {
    dossetbuf(&DirInFile,8192);
    PastHeader = FALSE;
    NumLines = 0;
    p = Buffer;
    q = Lines;

    UseIdx = FALSE;

    if (Type == SCAN_NEW || Type == SCAN_LOC || (Type == SCAN_ZIP && NewDate != 0)) {
      if (idxfilenameokay(IdxName,FileName)) {
        if (fileexist(IdxName) == 255)
          createdirindex(FileName,IdxName,&DirInFile);

        if (dosfopen(IdxName,OPEN_READ|OPEN_DENYNONE,&IdxFile) != -1) {
          dossetbuf(&IdxFile,8192);

          if (needtoresyncdirindex(FileName,&IdxFile)) {
            dosfclose(&IdxFile);
            unlink(IdxName);
          } else {
            UseIdx = TRUE;
            dossetbuf(&DirInFile,4096);
          }
        }
      }
    }

top:
    if (UseIdx) {
      switch (Type) {
        case SCAN_NEW:
        case SCAN_ZIP: if (IdxHeader.Latest != 0 && IdxHeader.Latest < NewDate)
                         goto done;       // no new files, get out of here now
                       locatenextnew();
                       break;
        case SCAN_LOC: locatenextmatch();
                       break;
      }
    }

    while (1) {
      checkstatus();              // watch for CTRL-K/CTRL-X to abort the
      if (Display.AbortPrintout)  // search or display
        break;

      if (Type == SCAN_ZIP)
        memset(p->Str,0,sizeof(p->Str)); /* set it all to zeroes because parsersearch() checks the whole thing */

      CurOffset = dosfseek(&DirInFile,0,SEEK_CUR);
      if (dosfgets(p->Str,sizeof(p->Str),&DirInFile) == -1)
        break;

      q->StrLen = strlen(p->Str);
      q->Type   = getlinetype(p,q);
      if (p != Buffer && Lines[0].Type == LN_FILE) {
        switch (q->Type) {
          case LN_SPEC:
          case LN_TEXT:
          case LN_HEAD:  func(Buffer,Lines,NumLines);
                         func(p,q,1);
                         NumLines = 0;
                         p = Buffer;
                         q = Lines;
                         goto top;
          case LN_FILE:  func(Buffer,Lines,NumLines);
                         PrevOffset = CurOffset;
                         if (UseIdx) {
                           NumLines = 0;
                           p = Buffer;
                           q = Lines;
                           goto top;
                         }

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
          case LN_HEAD:
          case LN_DUPE:  func(Buffer,Lines,1);
                         goto top;
          case LN_FILE:  NumLines++;
                         p++;
                         q++;
                         break;
        }
      }
    }

    if (p != Buffer && ! Display.AbortPrintout)
      func(Buffer,Lines,NumLines);

done:
    dosfclose(&DirInFile);
    if (UseIdx)
      dosfclose(&IdxFile);
  }
}


/********************************************************************
*
*  Function:  readdirlistfile()
*
*  Desc    :  reads a specific entry from the DIR.LST file
*
*  Returns :  TRUE if okay, FALSE if an error occured
*/

static bool _NEAR_ LIBENTRY readdirlistfile(int Num, dirlisttype *Dir, DOSFILE *DirListFile, int MaxNum) {
  long         Offset;
  pcbtexttype  Text;
  dirlisttype2 Buffer;

  Status.CurDirName = Dir->DirDesc;
  Status.CurDirNum  = (short) Num;

  if (Num == 0) {
    getpcbtext(TXT_RECENTUPLOADS,&Text);
    strcpy(Dir->DirPath,Status.CurConf.PrivDir);
    strcpy(Dir->DskPath,Status.CurConf.PrvUpldLoc);
    strcpy(Dir->DirDesc,Text.Str);
    return(TRUE);
  } else if (Num == MaxNum && ! (Status.CurConf.UpldDir[0] == 0)) {
    getpcbtext(TXT_RECENTUPLOADS,&Text);
    strcpy(Dir->DirPath,Status.CurConf.UpldDir);
    strcpy(Dir->DskPath,Status.CurConf.PubUpldLoc);
    strcpy(Dir->DirDesc,Text.Str);
    return(TRUE);
  } else if (DirListFile->handle > 0) {
    Offset = sizeof(dirlisttype2) * (long) (Num-1);
    if (dosfseek(DirListFile,Offset,SEEK_SET) != Offset)
      return(FALSE);

    if (dosfread(&Buffer,sizeof(dirlisttype2),DirListFile) != sizeof(dirlisttype2))
      return(FALSE);

    memcpy(Dir->DirPath,Buffer.DirPath,sizeof(Buffer.DirPath));
    Dir->DirPath[sizeof(Buffer.DirPath)] = 0;
    stripright(Dir->DirPath,' ');
    memcpy(Dir->DskPath,Buffer.DskPath,sizeof(Buffer.DskPath));
    Dir->DskPath[sizeof(Buffer.DskPath)] = 0;
    stripright(Dir->DskPath,' ');
    memcpy(Dir->DirDesc,Buffer.DirDesc,sizeof(Buffer.DirDesc));
    Dir->DirDesc[sizeof(Buffer.DirDesc)] = 0;
    stripright(Dir->DirDesc,' ');
    return(TRUE);
  }

  Status.CurDirName = NULL;
  Status.CurDirNum  = 0;
  return(FALSE);
}


/********************************************************************
*
*  Function:  addnumbers()
*
*  Desc    :  Checks the validity of a DIR number and adds it to a list of
*             Numbers if it's valid.  If it is invalid it empties the list
*             and returns.
*
*             Any text given is placed in the Text field if the pointer to
*             Text is not equal to NULL.
*
*  Returns :  0 if the numbers are good, -1 if they are not
*/

static int _NEAR_ LIBENTRY addnumbers(int NumDirs, int *Numbers, int *NumDirsToShow, char *New, char *Text, char *Date) {
  bool AllDigits;
  int  BegNum;
  int  EndNum;
  char *p;
  char *Dash;

  for (AllDigits = TRUE, Dash = NULL, p = New; *p != 0; p++) {
    if (! isdigit(*p)) {
      if (*p == '-')
        Dash = p;
      else {
        AllDigits = FALSE;
        break;
      }
    }
  }

  if (AllDigits) {
    if (strlen(New) == 6 && Date != NULL && Dash == NULL) {
      strcpy(Date,New);
      uncountrydate2(Date);
    } else {
      BegNum = EndNum = atoi(New);
      if (BegNum < 1 || BegNum > NumDirs)
        goto badnum;
      if (Dash != NULL) {
        EndNum = atoi(Dash+1);
        if (EndNum < 1 || EndNum > NumDirs)
          goto badnum;
      }
      for (; BegNum <= EndNum; BegNum++) {
        if (*NumDirsToShow < MAXDIRS)
          Numbers[(*NumDirsToShow)++] = BegNum;
      }
    }
  } else if (Text != NULL)
    addtext(Text,New,sizeof(Status.SearchText));
  return(0);

badnum:
  displaypcbtext(TXT_INVALIDFILENUM,NEWLINE|LFBEFORE|LOGIT);
  *NumDirsToShow = 0;
  return(-1);
}


/********************************************************************
*
*  Function:  getdirnumbers()
*
*  Desc    :  gets a list of numbers from the tokenized input
*             - translates "A" to all directory numbers
*             - translates "U" to the upload directory number
*             - if *Text != NULL it copies any non-numeric text to *Text
*             - indicates invalid DIR numbers
*
*  Returns :  TRUE if 'N' was found on the command line signifying that a
*             "New Files Scan" should be performed, otherwise FALSE
*/

static bool _NEAR_ LIBENTRY getdirnumbers(int NumTokens, char *Text, char *Date, int *Numbers, int *NumDirsToShow, int NumDirs) {
  bool RetVal;
  int  Num;
  char *p;
  int  *q;

  RetVal = FALSE;
  *NumDirsToShow = 0;

  for (; NumTokens > 0; NumTokens--) {
    p = getnexttoken();
    if (*(p+1) == 0) {
      switch (*p) {
        default : if (addnumbers(NumDirs,Numbers,NumDirsToShow,p,Text,Date) == -1)
                    return(RetVal);
                  break;
        case 'A': for (Num = 1, q = Numbers; Num <= NumDirs && Num <= MAXDIRS; Num++, q++)
                    *q = Num;
                  *NumDirsToShow = Num - 1;
                  break;
        case 'D': FlaggingFiles = TRUE;
                  break;
        case 'N': RetVal = TRUE;
                  break;
        case '0':
        case 'P': if (seclevelokay(p,PcbData.SysopSec[SEC_VIEWPRIV]))
                    Numbers[(*NumDirsToShow)++] = 0;
                  else
                    *NumDirsToShow = 0;
                  break;
        case 'S': if (Date != NULL) {
                    Date[0] = 'S';
                    Date[1] = 0;
                    RetVal = TRUE;
                  }
                  break;
        case 'U': if (Status.CurConf.PrivUplds && Status.CurConf.UpldDir[0] == 0) {
                    displaypcbtext(TXT_UPLOADSAREPRIVATE,NEWLINE|LFBEFORE);
                    *NumDirsToShow = 0;
                  } else
                    Numbers[(*NumDirsToShow)++] = NumDirs;
                  break;
      }
    } else if (addnumbers(NumDirs,Numbers,NumDirsToShow,p,Text,Date) == -1)
      return(RetVal);
  }

  return(RetVal);
}


/********************************************************************
*
*  Function:  getnumdirs()
*
*  Desc    :  determines the number of DIR files available for viewing
*             and displays "no dirs available" if none available
*
*  Returns :  the number of viewable DIR files
*/

static int _NEAR_ LIBENTRY getnumdirs(char *DirList) {
  int NumDirs;

  NumDirs = numdirs(DirList);
  if (NumDirs == 0)
    displaypcbtext(TXT_NODIRSAVAILABLE,NEWLINE|LFBEFORE|BELL);
  return(NumDirs);
}


/********************************************************************
*
*  Function:  showdirmenu()
*
*  Desc    :  displays a menu of available DIR files
*/

static void _NEAR_ LIBENTRY showdirmenu(void) {
  startdisplay(NOCHANGE);

  // don't include SECURITY as an option because a security specific
  // menu might be called DIR20 and yet directory #20 might be called DIR20
  displayfile(Status.CurConf.DirMenu,GRAPHICS|LANGUAGE|RUNMENU|RUNPPL);
  Display.NumLinesPrinted = 0;
}


/********************************************************************
*
*  Function:  dirmenu()
*
*  Desc    :  displays a menu of available DIR files and lets the user choose
*             a set of DIR files to view
*/

void LIBENTRY dirmenu(int NumTokens) {
  wordtype static Options[3] = {{"DOWNLOAD"}, {"FLAG"}, {"UPLOAD"}};
  bool        ShowMenu;
  int         NumDirs;
  int         NumDirsToShow;
  int         *q;
  char        *p;
  DOSFILE     DirListFile;
  char        DirList[66];
  dirlisttype Dir;
  char        Command[128];
  char        PeekBuf[128];
  int         Numbers[MAXDIRS];

  if ((NumDirs = getnumdirs(DirList)) == 0)
    return;

  // don't worry about bmalloc() failure, if it fails, the feature will
  // simply be disabled.
  FilesDisplayed = (dirarraytype *) bmalloc(sizeof(*FilesDisplayed) * MAXARRAY);
  if (FilesDisplayed != NULL)
    memset(FilesDisplayed,0,sizeof(*FilesDisplayed) * MAXARRAY);

  NewIndicator = '*';
  ShowMenu = (bool) (NumTokens == 0);

  while (1) {
    if (NumTokens == 0) {
      if ((! ShowMenu) && Status.InFileListing && Display.NumLinesPrinted > 1 && checkscreenforfilenames(1,NULL)) {
        Status.InFileListingLastMore = TRUE;
        freshline();
        moreprompt(MOREPROMPT);
      }
      Status.InFileListing = FALSE;
      Status.InFileListingLastMore = FALSE;
      if (ShowMenu) {
        showdirmenu();
        ShowMenu = FALSE;
      }
      Command[0] = 0;
      inputfield(Command,(UsersData.ExpertMode ? TXT_FILELISTCMDEXPRT : TXT_FILELISTCOMMAND),sizeof(Command)-1,UPCASE|STACKED|NEWLINE|LFBEFORE|HIGHASCII,HLP_F,mask_command);
      NumTokens = tokenize(Command);
      if (NumTokens == 0) {
        Status.LastView[0] = 0; /* don't leave without clearing the lastview filename */
        Status.CurDirName = NULL;
        if (FilesDisplayed != NULL) {
          bfree(FilesDisplayed);
          FilesDisplayed = NULL;
        }
        return;
      }
    }

    peekatnexttoken(PeekBuf,sizeof(PeekBuf)-1);
    switch (PeekBuf[0]) {
      case 'B': if (PeekBuf[1] == 'Y' && (PeekBuf[2] == 'E' || PeekBuf[2] == 0))
                  loguseroff(NLOGOFF);
                break;
      case 'D': if (PeekBuf[1] == 'B' && PeekBuf[2] == 0) {
                  Status.Batch = TRUE;
                  p = getnexttoken();  /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_D],NumTokens,send);
                  NumTokens = 0;
                  continue;
                } else if (PeekBuf[1] == 0 || option(Options,PeekBuf,3) != -1) {
                  p = getnexttoken();  /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_D],NumTokens,send);
                  NumTokens = 0;
                  continue;
                }
                break;
      case 'F': if (PeekBuf[1] == 0 || option(Options,PeekBuf,3) != -1) {
                  /* we peeked - so pull it out now */
                  getnexttoken();
                  peekatnexttoken(PeekBuf,sizeof(PeekBuf)-1);
                  if (PeekBuf[0] == 'V' && PeekBuf[1] == 0) { /* check for "F V" command instead of FLAG command */
                    NumTokens--;
                    goto view;
                  }
                  dispatch("FLAG",PcbData.UserLevels[SEC_D],NumTokens,flagfiles);
                  NumTokens = 0;
                  continue;
                }
                break;
      case 'G': if (PeekBuf[1] == 0) {
                  getnexttoken();  /* we peeked, now get rid of it */
                  byecommand();
                }
                break;
      case '?':
      case 'H': displayhelpfile(HLP_F);
                NumTokens = 0;
                continue;
      case 'L': if (PeekBuf[1] == 0) {
                  p = getnexttoken();    /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_L],NumTokens,filescan);
                  NumTokens = 0;
                  continue;
                }
                if (strcmp(PeekBuf,"LONG") == 0) {
                  Status.UseSingleLines = FALSE;
                  displaypcbtext(TXT_LONGINEFFECT,NEWLINE|LFBEFORE|LFAFTER);
                  moreprompt(PRESSENTER);
                }
                break;
      case 'N': if (PeekBuf[1] == 0) {
                  p = getnexttoken();    /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_N],NumTokens,newfilescan);
                  NumTokens = 0;
                  continue;
                }
                break;
      case 'R': ShowMenu  = TRUE;
                NumTokens = 0;
                continue;
      case 'S': if (strcmp(PeekBuf,"SHORT") == 0) {
                  Status.UseSingleLines = TRUE;
                  displaypcbtext(TXT_SHORTINEFFECT,NEWLINE|LFBEFORE|LFAFTER);
                  moreprompt(PRESSENTER);
                  NumTokens = 0;
                  continue;
                }
                if (PeekBuf[1] == 0) {
                  p = getnexttoken();    /* we peeked, now let's get it */
                  showfile(NumTokens-1);
                  NumTokens = 0;
                  continue;
                }
                break;
      case 'T': if (PeekBuf[1] == 0) {
                  p = getnexttoken();    /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_T],NumTokens,setprotocol);
                  NumTokens = 0;
                  continue;
                }
                break;
      case 'U': if (option(Options,PeekBuf,3) != -1) {
                  p = getnexttoken();    /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_U],NumTokens,receive);
                  NumTokens = 0;
                  continue;
                }
                break;
/*    case 'A': */   /* the 'A' command for "Arc View" is no longer used */
      case 'V': if (PeekBuf[1] == 0) {
      view:       p = getnexttoken();    /* we peeked, now let's get it */
                  viewfile(NumTokens-1);
                  displaycmdfile("PREFILE");
                  NumTokens = 0;
                  continue;
                }
                break;
      case 'Z': if (PeekBuf[1] == 0) {
                  p = getnexttoken();    /* we peeked, now let's get it */
                  dispatch(p,PcbData.UserLevels[SEC_Z],NumTokens,zippyscan);
                  NumTokens = 0;
                  continue;
                }
                break;
    }

    Status.LastView[0] = 0;
    FlaggingFiles = FALSE;
    getdirnumbers(NumTokens,NULL,NULL,Numbers,&NumDirsToShow,NumDirs); //lint !e534

    DirListFile.handle = 0;
    if (DirList[0] != 0)
      dosfopen(DirList,OPEN_READ|OPEN_DENYNONE,&DirListFile);   //lint !e534

    if (UseAnsi && UsersData.PackedFlags.MsgClear && (! UsersData.PackedFlags.ScrollMsgBody) && ! Status.Capture)
      printcls();

    displaycmdfile("PREFILE");

    CompleteList = TRUE;
    ZippyScan = FALSE;
    Status.InFileListing = TRUE;
    startdisplay(NOCHANGE);
    for (q = Numbers; q < &Numbers[NumDirsToShow]; q++) {
      if (readdirlistfile(*q,&Dir,&DirListFile,NumDirs)) {
        displaydirfile(*q,Dir.DirPath,SCAN_ALL,displaydirline);
        if (Display.AbortPrintout)
          break;
      }
    }

    if (Display.CountLines && Display.NumLinesPrinted > 1 && (! Display.WasAborted) && checkscreenforfilenames(1,NULL)) {
      Status.InFileListingLastMore = TRUE;
      moreprompt(MOREPROMPT);
    }
    Status.InFileListing = FALSE;
    Status.InFileListingLastMore = FALSE;
    Display.CountLines = FALSE;  /* don't do more prompt after we're done */
    NumTokens = 0;
    dosfclose(&DirListFile);
  }
}


/********************************************************************
*
*  Function:  writetolog()
*
*  Desc    :  writes to the log the fact that the user scanned the DIR files
*/

static void _NEAR_ LIBENTRY writetolog(char *SearchDate) {
  char Str[128];

  if (Status.SearchText[0] == 0 && SearchDate[0] == 0)
    return;

  if (SearchDate[0] != 0) {
    strcpy(Str,SearchDate);
    if (Status.SearchText[0] != 0)
      addchar(Str,':');
  } else Str[0] = 0;

  if (Status.SearchText[0] != 0)
    strcat(Str,Status.SearchText);

  maxstrcpy(Status.DisplayText,Str,sizeof(Status.DisplayText));
  logsystext(TXT_DIRECTORYSCAN,SPACERIGHT);
}


/********************************************************************
*
*  Function:  getdatefromuser()
*
*  Desc    :  prompts the user for a date of the form MMDDYY
*             if an 'S' was passed to the function it returns *Date
*
*  Notes   :  this function uses ctod() instead of ctod2() because it must
*             accept a date of the form mmddyy while ctod2() will only
*             accept a date of the form mm-dd-yy.
*/

static void _NEAR_ LIBENTRY getdatefromuser(char *Str, char *Date) {
  while (1) {
    if (Str[0] == 'S' && Str[1] == 0) {
      strcpy(Str,Date);
      return;
    }
    if (Str[0] == 0 || strlen(Str) != 6 || ! alldigits(Str) || ctod(Str) == 0) {
      strcpy(Str,Date);
      countrydate2(Str);
      inputfield(Str,TXT_DATETOSEARCH,6,UPCASE|GUIDE|FIELDLEN|NEWLINE|LFBEFORE,NOHELP,mask_numbers);
      if (Str[0] == 0) {
        strcpy(Str,Date);
        return;
      }
      if (strcmp(Str,"000000") == 0) {
        Str[0] = 0;
        return;
      }
      uncountrydate2(Str);
    } else return;
  }
}


/********************************************************************
*
*  Function:  scandirfiles()
*
*  Desc    :  engine for scanning DIR files - N)ew files, L)ocate & Z)ippy
*/

static void _NEAR_ LIBENTRY scandirfiles(int NumTokens, scantype Type) {
  bool        OptionalNewScan;
  int         NumDirsToShow;
  int         InputNum;
  int         Num;
  int         NumDirs;
  int         Len;
  int         *p;
  void LIBENTRY (*scanfunc)(dirtype *Buf, linetype *Line, int Num);
  char        SearchDate[7];
  char        Date[7];
  DOSFILE     DirListFile;
  char        DirList[66];
  dirlisttype Dir;
  char        Str[128];
  int         Numbers[MAXDIRS];

  if ((NumDirs = getnumdirs(DirList)) == 0)
    return;

  // don't worry about bmalloc() failure, if it fails, the feature will
  // simply be disabled.
  FilesDisplayed = (dirarraytype *) bmalloc(sizeof(*FilesDisplayed) * MAXARRAY);
  if (FilesDisplayed != NULL)
    memset(FilesDisplayed,0,sizeof(*FilesDisplayed) * MAXARRAY);

  memset(&DirListFile,0,sizeof(DirListFile));

  Status.SearchText[0] = 0;
  SearchDate[0] = 0;
  NumDirsToShow = 0;
  NewDate       = 0;
  Colorize      = (bool) (Control.GraphicsMode && Status.ColorizeDirFiles);
  FlaggingFiles = FALSE;
  ZippyScan     = FALSE;
  CompleteList  = FALSE;

  OptionalNewScan = getdirnumbers(NumTokens,Status.SearchText,SearchDate,Numbers,&NumDirsToShow,NumDirs);

  switch (Type) {
    case SCAN_ZIP: InputNum = TXT_TEXTTOSCANFOR;
                   scanfunc = zippyscanline;
                   NewIndicator = '*';
                   ZippyScan = TRUE;
                   break;
    case SCAN_LOC: InputNum = TXT_SEARCHFILENAME;
                   scanfunc = filescanline;
                   NewIndicator = '*';
                   break;
    case SCAN_NEW: scanfunc = newfilescanline;
                   break;
    default      : if (FilesDisplayed != NULL) {
                     bfree(FilesDisplayed);
                     FilesDisplayed = NULL;
                   }
                   return;
  }

  if (Type == SCAN_NEW || OptionalNewScan) {
    Date[0] = UsersRead.DateLastDirRead[2];
    Date[1] = UsersRead.DateLastDirRead[3];
    Date[2] = UsersRead.DateLastDirRead[4];
    Date[3] = UsersRead.DateLastDirRead[5];
    Date[4] = UsersRead.DateLastDirRead[0];
    Date[5] = UsersRead.DateLastDirRead[1];
    Date[6] = 0;
    getdatefromuser(SearchDate,Date);
  }

  if (Status.SearchText[0] == 0 && (Type == SCAN_ZIP || Type == SCAN_LOC)) {
    inputfield(Status.SearchText,InputNum,sizeof(Status.SearchText)-1,UPCASE|NEWLINE|LFBEFORE|HIGHASCII,(Type == SCAN_ZIP ? HLP_SRCH : NOHELP),mask_alphanum);  //lint !e644 InputNum was initiliazed above
    if (Status.SearchText[0] == 0)
      goto exit;
  }

  if (NumDirsToShow == 0) {
    while (1) {
      Str[0] = 0;
      inputfield(Str,(UsersData.ExpertMode ? TXT_FILENUMEXPERT : TXT_FILENUMNOVICE),sizeof(Str)-1,UPCASE|STACKED|NEWLINE,NOHELP,mask_dirs);
      if (Str[0] == 0) {
        // before aborting, clear out these two fields so that PCBoard won't
        // log a directory scan that never occurred
        Status.SearchText[0] = 0;
        SearchDate[0] = 0;
        goto exit;
      }
      if (Str[0] == 'L')
        showdirmenu();
      else {
        NumTokens = tokenize(Str);
        getdirnumbers(NumTokens,NULL,NULL,Numbers,&NumDirsToShow,NumDirs);  //lint !e534
        break;
      }
    }
  }

  if (Status.SearchText[0] == 0 && NumDirsToShow == 0) {
    Status.CurDirName = NULL;
    if (FilesDisplayed != NULL) {
      bfree(FilesDisplayed);
      FilesDisplayed = NULL;
    }
    return;
  }

  // if the caller types *.* then he must mean to do an L search so fix it.
  if (Type == SCAN_ZIP && strstr(Status.SearchText,"*.*") != NULL) {
    Type = SCAN_LOC;
    scanfunc = filescanline;
  }

  switch (Type) {
    case SCAN_LOC: if (! invalidfilename(Status.SearchText,ALLOWWILDCARDS|ALLOWSTARDOTSTAR)) {
                     Len = strlen(Status.SearchText);
                     if (strchr(Status.SearchText,'.') == NULL) {
                       if (Len < 8 && Status.SearchText[Len-1] != '*')
                         addchar(Status.SearchText,'*');
                       strcat(Status.SearchText,".*");
                     } else if (Status.SearchText[Len-1] != '.' && Status.SearchText[Len-1] != '*')
                       addchar(Status.SearchText,'*');
                     criteria(Status.SearchText,Criteria);
                     break;
                   }

                   // if the text is not a filename then fall thru and do a ZIPPY search instead!

                   // but first, check to see if there are any spaces in the
                   // search request.  if there are, change them to & to force
                   // the zippy search to look for AND'd criteria instead of
                   // straight text (e.g. L THIS THAT becomes L THIS & THAT)

                   change(Status.SearchText,' ','&');
                   Type = SCAN_ZIP;
                   scanfunc = zippyscanline;
                   // fall thru

    case SCAN_ZIP: if ((NumSearches = tokenscan(Status.SearchText,Status.SearchInput,FALSE)) <= 0) {  //lint !e616
                     if (NumSearches == -1)
                       displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
                     if (FilesDisplayed != NULL) {
                       bfree(FilesDisplayed);
                       FilesDisplayed = NULL;
                     }
                     return;
                   }
                   break;
  }

  if (Type == SCAN_NEW || OptionalNewScan) {
    Str[0] = SearchDate[0];
    Str[1] = SearchDate[1];
    Str[2] = '-';
    Str[3] = SearchDate[2];
    Str[4] = SearchDate[3];
    Str[5] = '-';
    Str[6] = SearchDate[4];
    Str[7] = SearchDate[5];
    Str[8] = 0;
    NewDate = (unsigned short) ctod2(Str);
    NewIndicator = (NewDate < UsersData.DateLastDirRead ? '*' : ' ');
/*  strtoyymmdd(SearchDate,Str); */
  }

  if (DirList[0] != 0)
    dosfopen(DirList,OPEN_READ|OPEN_DENYNONE,&DirListFile);  //lint !e534

  if (UseAnsi && UsersData.PackedFlags.MsgClear && (! UsersData.PackedFlags.ScrollMsgBody) && ! Status.Capture)
    printcls();

  displaycmdfile("PREFILE");
  Status.InFileListing = TRUE;
  newline();
  startdisplay(NOCHANGE);

  // on 4/1/95 I discovered both of the ShowActivity assignments below
  // I commented out the first one figuring that the second one must have been
  // overriding the first one...
//ShowActivity = (bool)  (Type != SCAN_ALL || OptionalNewScan);
  ShowActivity = TRUE;
  showactivity(ACTBEGIN);

  for (p = Numbers /*,FirstTime = TRUE */; p < &Numbers[NumDirsToShow]; p++) {
    Num = *p;
    if (readdirlistfile(Num,&Dir,&DirListFile,NumDirs)) {
      if (ShowActivity)
        showactivity(ACTSUSPEND);

      if (UseAnsi)
        print("\r");

      displaypcbtext(TXT_SCANNINGDIRECTORY,DEFAULTS);
      sprintf(Str," %d",Num);
      if (Dir.DirDesc[0] != 0) {
        print(Str);
        printcolor(PCB_GREEN);
        sprintf(Str,"  (%s)",Dir.DirDesc);
      }
      printxlated(Str);
      if (UseAnsi)
        cleareol();
      else
        newline();
      if (! Colorize)
        printdefcolor();

      if (ShowActivity)
        showactivity(ACTRESUME);

      displaydirfile(Num,Dir.DirPath,Type,scanfunc);
      checkstatus();
      if (Display.AbortPrintout)
        goto exit;
    }
  }

exit:
  if (ShowActivity)
    showactivity(ACTEND);

  if (FilesDisplayed != NULL) {
    bfree(FilesDisplayed);
    FilesDisplayed = NULL;
  }

  dosfclose(&DirListFile);
  Display.CountLines = FALSE;  /* don't do more prompt after we're done */
  writetolog(SearchDate);

  if (Type == SCAN_ZIP) {
    stopsearch();
    Status.SearchText[0] = 0;
  }

  Status.CurDirName = NULL;
}


/********************************************************************
*
*  Function:  filescan()
*
*  Desc    :  scans the DIR files for files (can use wildcards)
*/

void LIBENTRY filescan(int NumTokens) {
  scandirfiles(NumTokens,SCAN_LOC);
}


/********************************************************************
*
*  Function:  newfilescan()
*
*  Desc    :  scans the DIR files for files considered to be 'new'.
*/

void LIBENTRY newfilescan(int NumTokens) {
  scandirfiles(NumTokens,SCAN_NEW);
}


/********************************************************************
*
*  Function:  zippyscan()
*
*  Desc    :  scans the DIR files for key words
*/

void LIBENTRY zippyscan(int NumTokens) {
  scandirfiles(NumTokens,SCAN_ZIP);
}


void LIBENTRY showfile(int NumTokens) {
  char         Index;
  int          NumDirs;
  int          NumLines;
  dirtype     *p;
  linetype    *q;
  DOSFILE      File;
  char         DirName[66];
  char         FileName[13];
  dirlisttype  Dir;
  linetype     Lines[MAXLINES];
  dirtype      Buffer[MAXLINES];

  // if the FilesDisplayed array could not be allocated then disable
  // this function
  if (FilesDisplayed == NULL)
    return;

  if (NumTokens == 0) {
    FileName[0] = 0;
    inputfield(FileName,TXT_ARCVIEWFILENAME,sizeof(FileName)-1,NEWLINE|UPCASE|FIELDLEN,NOHELP,mask_filename);
    if (FileName[0] == 0)
      return;
  } else {
    maxstrcpy(FileName,getnexttoken(),sizeof(FileName));
  }

  // The array is filled with filenames that are space-padded on the right,
  // for example, "MYFILE.ZIP" is stored as "MYFILE.ZIP  ".  The spaces could
  // have been stripped while the scanning was being performed, but to improve
  // efficiency, it is quicker to just pad the search name before searching.
  padstr(FileName,' ',12);

  // NOTE:  Both Index and CurPos are stored in a SINGLE BYTE so that they
  // both 'wrap around' at 256.  Therefore, they will never examine more than
  // 256 entries because on the 256th entry, Index will be equal to CurPos.

  for (Index = CurPos; Index != CurPos+1; Index--) {
    if (strcmp(FilesDisplayed[Index].Name,FileName) == 0) {
      NumDirs = numdirs(DirName);
      if (dosfopen(DirName,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
        if (! readdirlistfile(FilesDisplayed[Index].DirNum,&Dir,&File,NumDirs))
          continue;
        dosfclose(&File);

        if (dosfopen(Dir.DirPath,OPEN_READ|OPEN_DENYNONE,&File) == -1)
          continue;

        dosfseek(&File,FilesDisplayed[Index].Offset,SEEK_SET);
        memset(Buffer,0,sizeof(Buffer));
        p = Buffer;
        q = Lines;
        NumLines = 0;

        while (1) {
          if (dosfgets(p->Str,sizeof(p->Str),&File) == -1)
            break;

          q->StrLen = strlen(p->Str);
          q->Type   = getlinetype(p,q);
          if (NumLines != 0 && q->Type != LN_DUPE)
            break;

          NumLines++;
          p++;
          q++;
        }

        if (NumLines != 0) {
          char Save = Status.UseSingleLines;
          Status.UseSingleLines = FALSE;
          displaydirline(Buffer,Lines,NumLines);
          Status.UseSingleLines = Save;
        }

        dosfclose(&File);
        break;
      }
    }
  }
}
