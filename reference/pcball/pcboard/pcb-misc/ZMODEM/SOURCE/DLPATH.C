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

#if __BORLANDC__ < 0x500 || ! defined(__BORLANDC__)
#pragma inline
#endif

#include <stdlib.h>
#include <string.h>
#include <pcbtools.h>
#include <misc.h>
#include <zmodem.h>

#ifdef MEMCHECK
#include <memcheck.h>
#endif


#ifdef _MSC_VER
#include <direct.h>
#else
#include <dir.h>
#endif

#include <model.h>

extern struct ffblk DTA;  /* declared in exist.c */

#define NAMESIZE   8
#define EXTSIZE    3
#define FILESIZE   NAMESIZE + EXTSIZE
#define NUM_TYPES  9

#define min(a,b)    (((a) < (b)) ? (a) : (b))

typedef struct {
  unsigned NumFiles;
  unsigned RecOffset[26];
  char     Reserved[74];
} idxhdrtype;

typedef struct {
  char     Name[8];
  char     Ext[3];
  unsigned PathNum;
} nametype;

typedef struct {
  char Path[64];
} pathtype;


static bool CheckPublic;
static bool CheckPrivate;

static DOSFILE     Idx;
static nametype    FileSpec;
static unsigned    Start;
static unsigned    End;


/********************************************************************
*
*  Function:  convertspectoname()
*
*  Desc    :  converts a filespec (i.e. "FILENAMEEXT") to the regular
*             file format (i.e. FILENAME.EXT).
*
*  Returns :  returns the filename in the array pointed to by Name
*/

static void near pascal convertspectoname(char *Name, nametype *File) {
  memcpy(Name,File,8);  Name[8] = 0;
  if (File->Ext[0] != ' ') {
    Name[8] = '.';
    memcpy(&Name[9],File->Ext,3);
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
*  Returns :  0 if the file exists, -1 if it does not, or if it is a device
*/

static bool near pascal pathfileexist(pathtype *Buf, nametype *File, unsigned NumFiles) {
  char Name[13];
  char *p;
  char Exist;

  dosfseek(&Idx,sizeof(idxhdrtype)+(sizeof(nametype)*(long)NumFiles)+(sizeof(pathtype)*(long)File->PathNum),SEEK_SET);
  if (Idx.offset == 0)
    dossetbuf(&Idx,sizeof(pathtype));
  dosfread(Buf,sizeof(pathtype),&Idx);
  p = &Buf->Path[strlen(Buf->Path)];

  convertspectoname(Name,File);
  strcat(Buf->Path,Name);
  Exist = fileexist(Buf->Path);
  *p = 0;

  if (Exist == 255)
    return(-1);
  if ((Exist & (16|64)) != 0)
    return(-1);
  return(0);
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

static int near pascal compare(unsigned RecNum, nametype *File) {
  dosfseek(&Idx,((long)RecNum * sizeof(nametype)) + sizeof(idxhdrtype),SEEK_SET);
  dosfread(File,sizeof(nametype),&Idx);
  return(memcmp(&FileSpec,File,FILESIZE));
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

static int near pascal binarysearch(pathtype *Buf, unsigned NumFiles) {
  nametype File;
  unsigned Rec;
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
  return(pathfileexist(Buf,&File,NumFiles));
}


/********************************************************************
*
*  Function:  setsearchlimits()
*
*  Desc    :  Based on the name of the file being searched for a top and bottom
*             limit in the index will be set to avoid searching in areas of the
*             file where it is known that the file will not be found.
*/

static void near pascal setsearchlimits(idxhdrtype *Hdr) {
  int Letter;
  int Size;

  Letter = FileSpec.Name[0];

  if (Letter != '?') {
    /* Force the letter usage in the range A-Z *except* when it's a wildcard */
    /* Set Start point based on letter, force it to start at beginning if    */
    /* before 'A', force it to start at 'Z' offset if after 'Z'              */

    if (Letter > 'Z') {
      Letter = 'Z'-'A';
      Start  = Hdr->RecOffset['Z'-'A'];     /* force to start at Z */
    } else if (Letter < 'A') {
      Letter = 'A'-'A';
      Start  = 0;                       /* force to start at beginning (may NOT be 'A') */
    } else {
      Letter -= 'A';
      Start  = Hdr->RecOffset[Letter];  /* start at proper offset for filename's first letter */
    }

    /* default End to end of file, but then scan to see if it can be */
    /* moved in any closer to shorten the binary search              */

    for (End = Hdr->NumFiles, Letter++; Letter <= 25; Letter++) {
      if (Hdr->RecOffset[Letter] != Start) {
        End = Hdr->RecOffset[Letter];
        break;
      }
    }
  } else {
    /* The first character is a wildcard, so search the ENTIRE index */
    Start = 0;
    End = Hdr->NumFiles;
  }

done:
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

static void near pascal makefilespec(char *Name) {
  int  NameLen;
  int  Len;
  char *p;

  strupr(Name);
  memset(&FileSpec,' ',FILESIZE);
  NameLen = min(strlen(Name),FILESIZE+1);

  if ((p = strnchr(Name,'.',NameLen)) != NULL) {
    p++;
    Len = ((int) (p - Name));
    memcpy(FileSpec.Ext,p,NameLen - Len);
    NameLen = Len - 1;
  }
  memcpy(FileSpec.Name,Name,NameLen);
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

static int near pascal searchidxforfiles(char *IdxName, char *FileName) {
  int        RetVal;
  pathtype   Buf;
  idxhdrtype Hdr;

  stripright(IdxName,'\\');
  if (dosfopen(IdxName,OPEN_READ|OPEN_DENYNONE,&Idx) == -1)
    return(-2);

  dossetbuf(&Idx,128);
  if (dosfread(&Hdr,sizeof(idxhdrtype),&Idx) != sizeof(idxhdrtype)) {
    dosfclose(&Idx);
    return(-2);
  }

  makefilespec(FileName);
  setsearchlimits(&Hdr);

  RetVal = binarysearch(&Buf,Hdr.NumFiles);

  dosfclose(&Idx);
  return(RetVal);
}


/********************************************************************
*
*  Function:  findfile()
*
*  Desc    :  Uses findfirst to locate the file.
*
*  Returns :  -1 if the file was not found or if the file that was found is
*             a subdirectory or an internal DOS name (such as COM1 or LPT1)
*
*             0 if the file was found and is good.
*/

static int near pascal findfile(char *Name) {
  int  RetVal;

  if ((RetVal = fileexist(Name)) == 255)
    return(-1);

  if ((RetVal & (16|64)) != 0)      // directories or device names
    return(-1);

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

static int near pascal searchpathforfiles(char *Path, char *FileName) {
  char Temp[66];

  buildstr(Temp,Path,FileName,NULL);
  return(findfile(Temp));
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

static int near pascal getnextpath(char *Path, unsigned PathSize, DOSFILE *List) {
  while (dosfgets(Path,PathSize,List) != -1) {
    if (strcmp(Path,CurConf.PubUpldLoc) == 0)
      if (! CheckPublic)
        continue;
      else
        CheckPublic = FALSE;
    if (strcmp(Path,CurConf.PrvUpldLoc) == 0) {
      if (! CheckPrivate)
        continue;
      else
        CheckPrivate = FALSE;
    }
    return(0);
  }

  if (CheckPublic) {
    CheckPublic = FALSE;
    if (CurConf.PubUpldLoc[0] != 0) {
      strcpy(Path,CurConf.PubUpldLoc);
      if (strcmp(Path,CurConf.PrvUpldLoc) == 0) {
        if (! CheckPrivate)  /* if ignore private and public is the same */
          return(-1);        /* then indicate we are done                */
        CheckPrivate = FALSE;
      }
      return(0);
    }
  }

  if (CheckPrivate) {
    CheckPrivate = FALSE;
    if (CurConf.PrvUpldLoc[0] != 0) {
      strcpy(Path,CurConf.PrvUpldLoc);
      return(0);
    }
  }

  return(-1);
}


/********************************************************************
*
*  Function:  scandlpaths()
*
*  Desc    :  Scans the download and upload paths to see if the filename
*             specified exists.
*
*  Returns :   TRUE    means matching files were found
*              FALSE   means no matching files were found
*/


bool pascal scandlpaths(char *FileName) {
  int      RetVal;
  char     *p;
  char     Name[13];
  DOSFILE  List;
  char     PathList[66];
  char     Path[66];

  if (PcbData.AllFilesList[0] != 0 && fileexist(PcbData.AllFilesList) != 255)
    strcpy(PathList,PcbData.AllFilesList);
  else {
    if (UseConfInfo) {
      if (CurConf.PthNameLoc[0] == 0 || fileexist(CurConf.PthNameLoc) == 255)
        return(FALSE);
      strcpy(PathList,CurConf.PthNameLoc);
    } else
      return(FALSE);
  }

  if (dosfopen(PathList,OPEN_READ|OPEN_DENYNONE,&List) == -1)
    return(FALSE);

  for (p = FileName; *p != 0; p++)      /* skip over drive letter and path */
    if (*p == '\\' || *p == ':')
      maxstrcpy(Name,p + 1,sizeof(Name));

  RetVal       = -1;
  CheckPublic  = UseConfInfo;   // default to FALSE if conf info not available
  CheckPrivate = UseConfInfo;   // default to FALSE if conf info not available

  RetVal = -1;
  dosrewind(&List);
  while (getnextpath(Path,sizeof(Path)-13,&List) != -1) {
    if (Path[0] == 0)  /* don't allow searching the default directory */
      continue;

    if (Path[0] == '%')
      RetVal = searchidxforfiles(&Path[1],Name);
    else
      RetVal = searchpathforfiles(Path,Name);

    if (RetVal == 0)  // 0 means a file was found!
      break;
  }

  dosfclose(&List);
  return(RetVal == 0 ? TRUE : FALSE); // 0 means a file was found, return TRUE!
}
