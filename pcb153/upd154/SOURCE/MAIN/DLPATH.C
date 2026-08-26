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


#if !defined(__OS2__) && !defined(__WATCOMC__)
  #pragma inline
//#include <model.h>
#endif

#include "project.h"
#pragma hdrstop

#include "wild.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <direct.h>
  #include <borland.h>
#else
//  #include <dir.h>
#endif

extern struct ffblk DTA;  // pulled in from MISC.LIB from fileexist()
static struct ffblk FileInfo;

#define NAMESIZE       8
#define EXTSIZE        3
#define FILESIZE       NAMESIZE + EXTSIZE
#define NUM_FILETYPES  9

#if defined(__cplusplus)
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#pragma pack(1)
typedef struct {
  uint     NumFiles;
  uint     RecOffset[26];
  char     Reserved[73];
  char     Id;
} idxhdrtype1;
typedef struct {
  ulong    NumFiles;
  ulong    RecOffset[26];
  char     Reserved[19];
  char     Id;
} idxhdrtype2;
typedef union {
  idxhdrtype1 Type1;
  idxhdrtype2 Type2;
} idxhdrtype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char     Name[8];
  char     Ext[3];
  uint     PathNum;
} nametype1;
typedef struct {
  char     Name[8];
  char     Ext[3];
  ulong    PathNum;
  ulong    FileSize;
} nametype2;
typedef union {
  nametype1 Type1;
  nametype2 Type2;
} nametype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char Path[64];
} pathtype;
#pragma pack()


char LastPathFound[66];

static bool Extended;
static bool CheckPublic;
static bool CheckPrivate;
static int  NumFound;
static int  NumAdded;
static bool WildUpload;
static char WildUploadName[13];

static DOSFILE     Idx;
static nametype    FileSpec;
static ulong       Start;
static ulong       End;


/********************************************************************
*
*  Function:  convertspectoname()
*
*  Desc    :  converts a filespec (i.e. "FILENAMEEXT") to the regular
*             file format (i.e. FILENAME.EXT).
*
*  Returns :  returns the filename in the array pointed to by Name
*/

static void _NEAR_ LIBENTRY convertspectoname(char *Name, nametype *File) {
  memcpy(Name,File,8);  Name[8] = 0;
  if (File->Type1.Ext[0] != ' ') {
    Name[8] = '.';
    memcpy(&Name[9],File->Type1.Ext,3);
    Name[12] = 0;
  }
  stripall(Name,' ');
}


/********************************************************************
*
*  Function:  pathfileexist()
*
*  Desc    :  Reads the PATH location from the index and adds the filename
*             to it then checks to see if the combined path\filename exists
*             and is not an invalid file type.
*
*  Returns :  TRUE if file exists, otherwise FALSE.
*/

static bool _NEAR_ LIBENTRY pathfileexist(pathtype *Buf, nametype *File, ulong NumFilesInIdx) {
  char Name[13];
  char *p;
  char Exist;

  if (Extended)
    dosfseek(&Idx,sizeof(idxhdrtype)+(sizeof(nametype2)*(long)NumFilesInIdx)+(sizeof(pathtype)*(long)File->Type2.PathNum),SEEK_SET);
  else
    dosfseek(&Idx,sizeof(idxhdrtype)+(sizeof(nametype1)*(long)NumFilesInIdx)+(sizeof(pathtype)*(long)File->Type1.PathNum),SEEK_SET);

  if (Idx.offset == 0)
    dossetbuf(&Idx,sizeof(pathtype));
  if (dosfread(Buf,sizeof(pathtype),&Idx) == -1)
    return(FALSE);

  p = &Buf->Path[strlen(Buf->Path)];
  convertspectoname(Name,File);
  strcat(Buf->Path,Name);

  if (Extended) {
    memset(&FileInfo,0,sizeof(FileInfo));
    strcpy(FileInfo.ff_name,Name);
    FileInfo.ff_fsize = File->Type2.FileSize;
    *p = 0;
    return(TRUE);
  }

  Exist = fileexist(Buf->Path);
  FileInfo = DTA;
  strupr(FileInfo.ff_name);
  *p = 0;
  return((bool) (Exist != 255 && (Exist & (16|64)) == 0));
}


/********************************************************************
*
*  Function:  compare()
*
*  Desc    :  Reads the index record pointed to by RecNum and compares the
*             search FileSpec against the filespec found in the index.
*
*  Returns :  0 if FileSpec and index record are equal, negative or postive
*             values to indicate if FileSpec is bigger or smaller.
*/

static int _NEAR_ LIBENTRY compare(ulong RecNum, nametype *File) {
  if (Extended) {
    dosfseek(&Idx,((long)RecNum * sizeof(nametype2)) + sizeof(idxhdrtype),SEEK_SET);
    dosfread(File,sizeof(nametype2),&Idx);     /*lint !e534 */
  } else {
    dosfseek(&Idx,((long)RecNum * sizeof(nametype1)) + sizeof(idxhdrtype),SEEK_SET);
    dosfread(File,sizeof(nametype1),&Idx);     /*lint !e534 */
  }
  return(memcmp(&FileSpec,File,FILESIZE));
}



/********************************************************************
*
*  Function:  checkfiletype()
*
*  Desc    :  Checks to see if the filename passed to the function is one of
*             the types of files that we want to prevent from getting
*             duplicates regardless of extension.
*
*             For example, if TEST.ZIP already exists, we want to prevent an
*             upload of TEST.ARJ, but an upload of TEST.TXT is okay.
*
*  Returns :  TRUE if the file is a special type, FALSE if it is generic
*/

static bool _NEAR_ LIBENTRY checkfiletype(char *FileName) {
  static char *FileTypes[NUM_FILETYPES] = { ".ZIP",".ARJ",".LZH",".LHA",".ARC",".PAK",".ZOO",".DWC",".EXE" };
  int    X;

  for (X = 0; X < NUM_FILETYPES; X++)
    if (strstr(FileName,FileTypes[X]) != NULL)
      return(TRUE);

  return(FALSE);
}


/********************************************************************
*
*  Function:  foundfile()
*
*  Desc    :  Processes the newly found file - adding one to the counter and
*             storing it in the file array.
*
*  Returns :  -2 if the file cannot be added to the list (file was a zero byte
*             file or file was a duplicate in the batch).
*
*             -3 if a serious error (such as not allocating the file list)
*             occurs
*
*             0 if all goes well and the file is added
*/

static int _NEAR_ LIBENTRY foundfile(char *Path, SCANDLTYPE Flags) {
  int      AddRetVal;
  struct   ffblk FoundInfo;
  spectype File;

  FoundInfo = FileInfo;  // don't trust the information to remain intact
                         // save it to FoundInfo and use it instead

  // Are we doing a wildcard (FILENAME.*) search prior to allowing an upload?
  // If so, check to see if the file that was already found matches the FULL
  // NAME of the file searched for and, if not, check to see if the file found
  // is one of the file types that we want to prevent from duplicating.

  if (WildUpload)
    if (strcmp(FoundInfo.ff_name,WildUploadName) != 0)
      if (! checkfiletype(FoundInfo.ff_name))
        return(0);

  NumFound++;
  if (Flags & REPORTONLY) {
    strcpy(LastPathFound,Path);
    return(0);
  }

  if (FoundInfo.ff_fsize == 0 && Flags & CHECKDNLDS) {
    displaypcbtext(TXT_FILESIZEISZERO,NEWLINE|BELL);
    return(-2);
  }

  if ((AddRetVal = addfiletolist(FoundInfo.ff_name,Path,FoundInfo.ff_fsize,FALSE,(bool) (Flags & SHOWDUPES))) < 0)
    return(AddRetVal);  /* returns -2 if not serious, -3 if serious */

  if (getfilespec(AddRetVal,&File) == -1)
    return(-3);

  File.Found = TRUE;
  putfilespec(AddRetVal,&File);
  NumAdded++;
  return(0);
}


/********************************************************************
*
*  Function:  binarysearch()
*
*  Desc    :  Performs a binary search on the index file looking for a SINGLE
*             match to the FileSpec criteria.
*
*  Returns :  If the file is found, the path is found to exist, and no errors
*             occur in adding the file to the list (if necessary) then it
*             returns 0.
*
*             -1 if the file was not found
*             -2 if the found file was not (and should not be) added
*             -3 if an error occured forcing the file to not be added
*/

static int _NEAR_ LIBENTRY binarysearch(pathtype *Buf, SCANDLTYPE Flags, ulong NumFilesInIdx) {
  nametype File;
  ulong    Rec;
  int      RetVal;

  if ((RetVal = compare(Start,&File)) == 0)
    goto found;

  if (RetVal < 0)
    return(-1);

  while (1) {
    if (End <= Start + 1) return(-1);
    Rec = Start + ((End - Start) >> 1);
    if ((RetVal = compare(Rec,&File)) == 0)
      goto found;
    if (RetVal < 0)
      End = Rec;
    else
      Start = Rec;
  }

found:
  if (pathfileexist(Buf,&File,NumFilesInIdx))
    return(foundfile(Buf->Path,Flags));
  return(-1);
}


/********************************************************************
*
*  Function:  comparewildcards()
*
*  Desc    :  Compares *Wild (of the form "TEST????ZIP") to *String (of the
*             form "TEST    ZIP") to check for a match.
*
*  Returns :  TRUE if *Wild matches *String.
*/

#if !defined(__OS2__) && !defined(__WATCOMC__)   // this function was rewritten in WILD.C
bool LIBENTRY comparewildcards(char *Wild, char *String) {
#ifdef LDATA
asm      Push Ds
asm      Lds  Si,Wild     /* DS:SI points to wildcarded file spec */
asm      Les  Di,String   /* ES:DI points to filespec to compare to it */
#else
asm      Mov  Si,Wild     /* DS:SI points to wildcarded file spec */
asm      Mov  Ax,Ds
asm      Mov  Es,Ax
asm      Mov  Di,String   /* ES:DI points to filespec to compare to it */
#endif

asm      Cld
asm      Mov  Al,1        /* default to TRUE which means a MATCH was found */
asm      Mov  Cx,11       /* scan all eleven formatted positions */
asm      Mov  Bl,'?'      /* watch out for wildcards */

top:
asm      Rep  Cmpsb
asm      Je   done        /* if it's equal get out of here now */
asm      Cmp  [Si-1],Bl
asm      Je   top         /* if it was a wildcard go compare some more */
asm      Xor  Ax,Ax       /* return FALSE - no match found */

done:
#ifdef LDATA
asm      Pop  Ds
#endif
  return(_AL);
}  /*lint !e563 */
#endif


/********************************************************************
*
*  Function:  wildfindfiles()
*
*  Desc    :  Searches for MULTIPLE matches to the FileSpec criteria.
*
*  Returns :   0 if file(s) found and successfully added to the list
*             -1 if no file was found
*             -2 if a file was not (and should not be) added
*             -3 if an error occured forcing a file to not be added
*
*  NOTE    :  The MAXIMUM number of matches on a single wildcard search is
*             50 based on the size of the File[] array.
*/

static int _NEAR_ LIBENTRY wildfindfiles(pathtype *Buf, SCANDLTYPE Flags, ulong NumFilesInIdx) {
  int        X;
  int        RetVal;
  ulong      Rec;
  nametype1 *p1;
  nametype2 *p2;
  nametype1  File1[50];
  nametype2  File2[50];

  if (Extended) {
    dosfseek(&Idx,((long)Start * sizeof(nametype2)) + sizeof(idxhdrtype),SEEK_SET);
    for (Rec = Start, p2 = File2; Rec < End && p2 < File2+50; Rec++) {
      if (dosfread(p2,sizeof(nametype2),&Idx) != sizeof(nametype2))
        break;
      if (comparewildcards(FileSpec.Type2.Name,p2->Name))
        p2++;
    }
    X = (int) (p2 - File2);
    if (X == 0)
      return(-1);
    for (p2 = File2; X; X--, p2++) {

      // need to get rid of the pathfileexist() call or else supplement it

      if (pathfileexist(Buf,(nametype *) p2,NumFilesInIdx))
        if ((RetVal = foundfile(Buf->Path,Flags)) == -3)
          return(RetVal);
    }
  } else {
    dosfseek(&Idx,((long)Start * sizeof(nametype1)) + sizeof(idxhdrtype),SEEK_SET);
    for (Rec = Start, p1 = File1; Rec < End && p1 < &File1[50]; Rec++) {
      if (dosfread(p1,sizeof(nametype1),&Idx) != sizeof(nametype1))
        break;
      if (comparewildcards(FileSpec.Type1.Name,p1->Name))
        p1++;
    }
    X = (int) (p1 - File1);
    if (X == 0)
      return(-1);
    for (p1 = File1; X; X--, p1++) {
      if (pathfileexist(Buf,(nametype *) p1,NumFilesInIdx))
        if ((RetVal = foundfile(Buf->Path,Flags)) == -3)
          return(RetVal);
    }
  }

  return(0);
}


/********************************************************************
*
*  Function:  setsearchlimits()
*
*  Desc    :  Based on the name of the file being searched for a top and bottom
*             limit in the index will be set to avoid searching in areas of the
*             file where it is known that the file will not be found.
*/

static void _NEAR_ LIBENTRY setsearchlimits(idxhdrtype *IdxHdr) {
  int Letter;
  int Size;

  Letter = FileSpec.Type1.Name[0];

  if (Letter != '?') {
    /* Force the letter usage in the range A-Z *except* when it's a wildcard */
    /* Set Start point based on letter, force it to start at beginning if    */
    /* before 'A', force it to start at 'Z' offset if after 'Z'              */

    if (Letter > 'Z') {
      Letter = 'Z'-'A';
      if (Extended)
        Start  = IdxHdr->Type2.RecOffset['Z'-'A'];   // force to start at Z
      else
        Start  = IdxHdr->Type1.RecOffset['Z'-'A'];   // force to start at Z
    } else if (Letter < 'A') {
      Letter = 'A'-'A';
      Start  = 0;                       /* force to start at beginning (may NOT be 'A') */
    } else {
      Letter -= 'A';
      if (Extended)
        Start  = IdxHdr->Type2.RecOffset[Letter];  // start at proper offset for filename's first letter
      else
        Start  = IdxHdr->Type1.RecOffset[Letter];  // start at proper offset for filename's first letter
    }

    /* default End to end of file, but then scan to see if it can be */
    /* moved in any closer to shorten the binary search              */

    if (Extended) {
      for (End = IdxHdr->Type2.NumFiles, Letter++; Letter <= 25; Letter++) {
        if (IdxHdr->Type2.RecOffset[Letter] != Start) {
          End = IdxHdr->Type2.RecOffset[Letter];
          break;
        }
      }
    } else {
      for (End = IdxHdr->Type1.NumFiles, Letter++; Letter <= 25; Letter++) {
        if (IdxHdr->Type1.RecOffset[Letter] != Start) {
          End = IdxHdr->Type1.RecOffset[Letter];
          break;
        }
      }
    }
  } else {
    /* The first character is a wildcard, so search the ENTIRE index */
    Start = 0;
    End = (Extended ? IdxHdr->Type2.NumFiles : IdxHdr->Type1.NumFiles);
  }

  Size = (int) ((((End - Start) >> 1) + 1) * sizeof(nametype));
  if (Size >= 4096)
    Size = 4096;
  else if (Size <= 1024)
    Size = 1024;
  else Size = 2048;
  dossetbuf(&Idx,Size);
}


/********************************************************************
*
*  Function:  makefilespec()
*
*  Desc    :  Takes *Name (of the form "TEST.ZIP") and turns it into the
*             FileSpec criteria (of the form "TEST    ZIP").
*/

static void _NEAR_ LIBENTRY makefilespec(char *Name) {
  int  NameLen;
  int  Len;
  char *p;

  strupr(Name);
  memset(&FileSpec,' ',FILESIZE);
  NameLen = strlen(Name);
  if (NameLen > FILESIZE+1)
    NameLen = FILESIZE+1;

  if ((p = strnchr(Name,'.',NameLen)) != NULL) {
    p++;
    Len = ((int) (p - Name));
    memcpy(FileSpec.Type1.Ext,p,NameLen - Len);
    if ((p = strnchr(FileSpec.Type1.Ext,'*',3)) != NULL)
      memset(p,'?',3 - ((int) (p-FileSpec.Type1.Ext)));
    NameLen = Len - 1;
  }
  memcpy(FileSpec.Type1.Name,Name,NameLen);
  if ((p = strnchr(FileSpec.Type1.Name,'*',8)) != NULL)
    memset(p,'?',8 - ((int) (p-FileSpec.Type1.Name)));
}


/********************************************************************
*
*  Function:  searchidxforfiles()
*
*  Desc    :  Searches the index file rather than the DOS PATH to find files.
*
*  Returns :   0 if file(s) found and successfully added to the list
*             -1 if a file was not (and should not be) added
*             -2 if an error occured forcing a file to not be added
*/

static int _NEAR_ LIBENTRY searchidxforfiles(char *IdxName, char *Token, SCANDLTYPE Flags, bool WildCards) {
  int         RetVal;
  pathtype    Buf;
  idxhdrtype  IdxHdr;

  stripright(IdxName,'\\');
  if (dosfopen(IdxName,OPEN_READ|OPEN_DENYNONE,&Idx) == -1)
    return(-2);

  dossetbuf(&Idx,128);
  if (dosfread(&IdxHdr,sizeof(idxhdrtype),&Idx) != sizeof(idxhdrtype)) {
    dosfclose(&Idx);
    return(-2);
  }

  Extended = (bool) (IdxHdr.Type2.Id != 0);

  makefilespec(Token);
  setsearchlimits(&IdxHdr);

  if (WildCards)
    RetVal = wildfindfiles(&Buf,Flags,(Extended ? IdxHdr.Type2.NumFiles : IdxHdr.Type1.NumFiles));
  else
    RetVal = binarysearch(&Buf,Flags,(Extended ? IdxHdr.Type2.NumFiles : IdxHdr.Type1.NumFiles));

  dosfclose(&Idx);
  return(RetVal);
}


/********************************************************************
*
*  Function:  findfile()
*
*  Desc    :  Uses either findfirst or findnext DOS search calls to locate a
*             file.
*
*  Returns :  -1 if the file was not found or if the file that was found is
*             a subdirectory or an internal DOS name (such as COM1 or LPT1)
*
*             0 if the file was found and is good.
*/

static int _NEAR_ LIBENTRY findfile(char *Name, bool FindFirst) {
  int  Count;
  #if defined(__OS2__) || defined(__WATCOMC__)
#ifndef SYSTEMDIRHANDLE
#define SYSTEMDIRHANDLE 1
#endif
  int  DirHandle = SYSTEMDIRHANDLE;
  #endif

  Count = 1;

  if (FindFirst) {
    if (dosfindfirst(Name,&FileInfo,FA_ARCH,&Count PDIRHANDLE) == -1)
      return(-1);
  } else {
next:
    if (dosfindnext(&FileInfo,&Count PDIRHANDLE2) == -1)
      return(-1);
  }

  if (FileInfo.ff_attrib == 16 || FileInfo.ff_attrib == 64)
    goto next;     // skip over directories and device names

  strupr(FileInfo.ff_name);
  return(0);
}


/********************************************************************
*
*  Function:  searchpathforfiles()
*
*  Desc    :  Searches the DOS PATH for files.
*
*  Returns :   0 if file(s) found and successfully added to the list
*             -1 if no file was found
*             -2 if a file was not (and should not be) added
*             -3 if an error occured forcing a file to not be added
*/

static int _NEAR_ LIBENTRY searchpathforfiles(char *Path, char *Token, SCANDLTYPE Flags, bool WildCards) {
  bool FindFirst;
  int  RetVal;
  char Temp[66];

  buildstr(Temp,Path,Token,NULL);
  FindFirst = TRUE;

  while (findfile(Temp,FindFirst) == 0) {
    if ((RetVal = foundfile(Path,Flags)) < 0)
      if ((! WildCards) || RetVal == -3)
        return(RetVal);

    if (FindFirst) {
      if (WildCards)
        FindFirst = FALSE;  /* multi-file search, we'll loop back for more */
      else
        return(0);   /* SINGLE FILE search, file was found, so get out now */
    }
  }

  return(NumAdded == 0 ? -1 : 0);
}


/********************************************************************
*
*  Function:  getnextpath()
*
*  Desc    :  This function basically mimics the dosfgets() function except
*             that it will avoid returning a path read in using dosfgets()
*             which matches the upload path and when dosfgets() finds no more
*             records to read it then returns the upload paths.
*
*  Returns :  -1 if an error occurs, else 0
*/

static int LIBENTRY getnextpath(char *Path, unsigned PathSize, DOSFILE *List) {
  if (! Status.FileAttach) {
    while (dosfgets(Path,PathSize,List) != -1) {
      if (strcmp(Path,Status.CurConf.PubUpldLoc) == 0)
        CheckPublic = FALSE;
      if (strcmp(Path,Status.CurConf.PrvUpldLoc) == 0) {
        if (! CheckPrivate)
          continue;
        else
          CheckPrivate = FALSE;
      }
      return(0);
    }

    if (CheckPublic) {
      CheckPublic = FALSE;
      if (Status.CurConf.PubUpldLoc[0] != 0) {
        strcpy(Path,Status.CurConf.PubUpldLoc);
        if (strcmp(Path,Status.CurConf.PrvUpldLoc) == 0) {
          if (! CheckPrivate)  /* if ignore private and public is the same */
            return(-1);        /* then indicate we are done                */
          CheckPrivate = FALSE;
        }
        return(0);
      }
    }
  }

  if (CheckPrivate) {
    CheckPrivate = FALSE;
    if (Status.CurConf.PrvUpldLoc[0] != 0) {
      strcpy(Path,Status.CurConf.PrvUpldLoc);
      return(0);
    }
  }

  return(-1);
}


void static _NEAR_ LIBENTRY filenotfound(char *Str, bool CheckDnlds) {
  if (CheckDnlds) {
    maxstrcpy(Status.DisplayText,Str,sizeof(Status.DisplayText));
    displaypcbtext(TXT_NOTFOUNDONDISK,NEWLINE|LOGIT|BELL);
    UsersData.Stats.NumFileNotFound++;
  }
}


/********************************************************************
*
*  Function:  scandlpaths()
*
*  Desc    :  Scans all of the download paths and optionally the upload path
*             as well to see if the file or files specified by *FileNames
*             exists.
*
*  Returns :   0 if no matching filenames were found
*             -1 if unable to add filenames to the File List
*             otherwise a positive value representing the number of files added
*/


int LIBENTRY scandlpaths(SCANDLTYPE Flags, char *FileNames) {
  bool     WildCards;
  bool     CheckDnlds;
  bool     CheckUplds;
  int      NumTokens;
  int      RetVal;
  char     *p;
  int LIBENTRY (*getnext)(char *Str, unsigned MaxChars, DOSFILE *file);
  char     *OldPointer;
  char     Ext[10];
  char     Token[13];
  DOSFILE  List;
  char     PathList[66];
  char     Path[66];

  CheckDnlds = (bool) (Flags & CHECKDNLDS);
  CheckUplds = (bool) (Flags & CHECKUPLDS);

  if (CheckUplds && PcbData.AllFilesList[0] != 0 && fileexist(PcbData.AllFilesList) != 255)
    strcpy(PathList,PcbData.AllFilesList);
  else
    if (checkforalternatelist(PathList,Status.CurConf.PthNameLoc,0) == -1) {
      filenotfound(FileNames,CheckDnlds);
      return(0);
    }

  if (dosfopen(PathList,OPEN_READ|OPEN_DENYNONE,&List) == -1) {
    filenotfound(FileNames,CheckDnlds);
    return(0);
  }

  RetVal = 0;
  NumFound = 0;
  NumAdded = 0;
  savetokenpointer(&OldPointer);

  /* choose the function we want for getting paths based on    */
  /* whether or not we will be checking the upload directories */
  getnext = (CheckUplds ? getnextpath : dosfgets);

  if (CheckDnlds && PcbData.ViewExt[0] != 0) {
    Ext[0] = '.';                       /* we need the period */
    strcpy(&Ext[1],PcbData.ViewExt);    /* and then extension */
  } else
    Ext[0] = 0;                         /* don't check extension */

  for (NumTokens = tokenize(FileNames); NumTokens; NumTokens--) {
    maxstrcpy(Token,getnexttoken(),sizeof(Token));

    p = NULL;                           // default to NO EXTENSION added
    if (Ext[0] != 0 && strchr(Token,'.') == NULL) {
      int Temp = strlen(Token);
      if (Temp <= 8) {
        p = &Token[Temp];              /* point p to end of token */
        strcpy(p,Ext);                 /* add extension           */
      }
    }

    /* if we aren't checking uploads and the filename has a wildcard in it  */
    /* then record the fact that we may be ADDING new filenames to the list */
    WildCards  = (bool) (CheckDnlds && (strpbrk(Token,"*?") != NULL));
    WildUpload = FALSE;

    if (CheckUplds) {
      if ((p = strrchr(Token,'.')) != NULL && (strlen(Token) - (int) (p-Token)) <= 4 && checkfiletype(Token)) {
        strcpy(WildUploadName,Token);
        p++;
        *p++ = '*';
        *p++ = 0;
        WildCards = TRUE;
        WildUpload = TRUE;
      }
      CheckPublic  = TRUE;
      CheckPrivate = (bool) (Flags & IGNOREPRIVATE ? FALSE : TRUE);
    }

retry:
    dosrewind(&List);
    while (getnext(Path,sizeof(Path)-13,&List) != -1) {
      if (Path[0] == 0)  /* don't allow searching the default directory */
        continue;

      if (Path[0] == '%')
        RetVal = searchidxforfiles(&Path[1],Token,Flags,WildCards);
      else
        RetVal = searchpathforfiles(Path,Token,Flags,WildCards);

      if (RetVal == -2)
        goto forloop;
      if (RetVal == -3)
        goto exit;
      if (NumFound == 1 && ! WildCards)
        break;
    }

    if (NumFound == 0) {
      if (p != NULL) {
        *p = 0;              /* remove the extension    */
        p = NULL;            /* indicate NO extension   */
        goto retry;          /* retry without extension */
      }
      filenotfound(Token,CheckDnlds);
    }
forloop:;
  }

exit:
  restoretokenpointer(&OldPointer);
  dosfclose(&List);
  return(RetVal == -3 ? -1 : (Flags & REPORTONLY ? NumFound : NumAdded));
}



void LIBENTRY flagfiles(int NumTokens) {
  int  OldNumFiles;
  int  OpenStat;
  char *p;
  char *OldPointer;
  char Str[256];

  if (Status.CurConf.PthNameLoc[0] == 0) {
    displaypcbtext(TXT_NODIRSAVAILABLE,NEWLINE|LFBEFORE|BELL);
    return;
  }

  NumFound = 0;
  NumAdded = 0;
  savetokenpointer(&OldPointer);

  if (NumTokens == 0) {
    Str[0] = 0;
    inputfield(Str,TXT_FLAGFORDOWNLOAD,sizeof(Str)-1,NEWLINE|UPCASE|STACKED,HLP_FLAG,mask_wildname);
    if (Str[0] == 0)
      return;
    NumTokens = tokenize(Str);
  }

  displaypcbtext(TXT_CHECKINGFILEXFER,NEWLINE);

  /* allocate the file list if it's not already allocted */
  if ((OpenStat = allocatefilelist(DOWNLOAD)) == -1)
    return;                /* serious error - cannot create list */

  if (OpenStat == 2)       /* if allocatefilelist() found files then we need */
    checkdlfiles(0);       /* to scan them to set up the time/byte counters  */

  for (OldNumFiles = NumFiles; NumTokens; NumTokens--) {
    p = getnexttoken();
    if (! invalidfilename(p,ALLOWWILDCARDS|INFORM))
      scandlpaths(CHECKDNLDS|ADDFILE|SHOWDUPES,p);    //lint !e534
  }

  checkdlfiles(OldNumFiles);
  showbatchnames(OldNumFiles,1024);
  restoretokenpointer(&OldPointer);
  Status.LastView[0] = 0;
}


void LIBENTRY flagnamedfile(char *Path, char *NameOnDisk, char *NewName) {
  int      OldNumFiles;
  int      OpenStat;
  char     FullPath[66];
  spectype File;

  NumFound = 0;
  NumAdded = 0;

  displaypcbtext(TXT_CHECKINGFILEXFER,NEWLINE);

  /* allocate the file list if it's not already allocted */
  if ((OpenStat = allocatefilelist(DOWNLOAD)) == -1)
    return;                /* serious error - cannot create list */

  if (OpenStat == 2)       /* if allocatefilelist() found files then we need */
    checkdlfiles(0);       /* to scan them to set up the time/byte counters  */

  OldNumFiles = NumFiles;

  buildstr(FullPath,Path,NameOnDisk,NULL);
  if (fileexist(FullPath) == 255) {
    displaypcbtext(TXT_ATTACHMENTMISSING,NEWLINE|LFBEFORE|LOGIT);
    return;
  }

  if (addfiletolist(NameOnDisk,Path,DTA.ff_fsize,FALSE,TRUE) < 0)
    return;

  getfilespec(OldNumFiles,&File);      //lint !e534

  if (strcmp(NameOnDisk,NewName) != 0) {
    strcpy(File.OldName,File.Name);
    strcpy(File.Name,NewName);
    putfilespec(OldNumFiles,&File);
  }

  checkdlfiles(OldNumFiles);
  showbatchnames(OldNumFiles,1024);
}


bool LIBENTRY filesflagged(void) {
  return((bool) (Status.NumTaggedFiles != 0 || NumFiles != 0));
}
