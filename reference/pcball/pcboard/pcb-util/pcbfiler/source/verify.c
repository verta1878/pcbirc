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
#include <stdlib.h>
#include <string.h>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#pragma inline

extern struct ffblk DTA;   /* really declared as "char DTA[45]" in exist.c */

       char     SrchDskPath[66];
static char     *SrchOffset;
static DOSFILE  PathListFile;
static DOSFILE  Idx;
static nametype1 FileSpec;
static ulong     Start;
static ulong     End;
static int       RecSize;
static bool      Extended;
static bool      CheckDiskForFiles = TRUE;


/********************************************************************
*
*  Function:  convertspectoname()
*
*  Desc    :  converts a filespec (i.e. "FILENAMEEXT") to the regular
*             file format (i.e. FILENAME.EXT).
*
*  Returns :  returns the filename in the array pointed to by Name
*/

void pascal convertspectoname(char *Name, nametype *File) {
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
*             If CheckDiskForFiles is FALSE then it skips checking the disk
*             and assumes that since the file was in the index it must really
*             be there and returns a file attribute of 0.
*
*  Returns :  File attribute of file (if it exists) or 255
*/

static bool near pascal pathfileexist(nametype *File, ulong NumFiles) {
  char Name[13];

  if (Extended)
    dosfseek(&Idx,sizeof(idxhdrtype)+((long) RecSize*NumFiles)+((long) sizeof(pathtype)*File->Type2.PathNum),SEEK_SET);
  else
    dosfseek(&Idx,sizeof(idxhdrtype)+((long) RecSize*NumFiles)+((long) sizeof(pathtype)*File->Type1.PathNum),SEEK_SET);

  if (Idx.offset == 0)
    dossetbuf(&Idx,sizeof(pathtype));
  dosfread(SrchDskPath,sizeof(pathtype),&Idx);   //lint !e534
  convertspectoname(Name,File);
  strcat(SrchDskPath,Name);
  return(CheckDiskForFiles ? fileexist(SrchDskPath) : 0);
}


/********************************************************************
*
*  Function:  comparespec()
*
*  Desc    :  Reads the index record pointed to by RecNum and compares the
*             search FileSpec against the filespec found in the index.
*
*  Returns :  0 if FileSpec and index record are equal, negative or postive
*             values to indicate if FileSpec is bigger or smaller.
*/

static int near pascal comparespec(ulong RecNum, nametype *File) {
  dosfseek(&Idx,(RecNum * RecSize) + sizeof(idxhdrtype),SEEK_SET);
  dosfread(File,RecSize,&Idx);     //lint !e534
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
*             returns the file attribute.  Otherwise it returns 255.
*/

static int near pascal binarysearch(ulong NumFiles) {
  nametype File;
  ulong    Rec;
  int      RetVal;

  if ((RetVal = comparespec(Start,&File)) == 0)
    return(pathfileexist(&File,NumFiles));
  if (RetVal < 0)
    return(255);

  while (1) {
    if (End <= Start + 1) return(255);
    Rec = Start + ((End - Start) >> 1);
    if ((RetVal = comparespec(Rec,&File)) == 0)
      return(pathfileexist(&File,NumFiles));
    if (RetVal < 0)
      End = Rec;
    else
      Start  = Rec;
  }
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

static bool near pascal comparewildcards(char *Wild, char *String) {
asm      Mov  Dx,Ds
asm      Lds  Si,Wild     /* DS:SI points to wildcarded file spec */
asm      Mov  Ax,Ds
asm      Mov  Es,Ax
asm      Les  Di,String   /* ES:DI points to filespec to compare to it */

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
asm      Mov  Ds,Dx
  return(_AL);
}                         //lint !e563


/********************************************************************
*
*  Function:  wildfindfiles()
*
*  Desc    :  Searches for MULTIPLE matches to the FileSpec criteria.
*
*  Returns :  -1 if the user asked to abort, otherwise 0
*/

static int near pascal wildfindfiles(ulong NumFiles) {
  nametype File[99];
  nametype *p;
  ulong    Rec;
  int      X;

  dosfseek(&Idx,((long)Start * RecSize) + sizeof(idxhdrtype),SEEK_SET);

  for (Rec = Start, p = File; Rec < End && p < &File[99]; Rec++) {
    dosfread(p,RecSize,&Idx);  //lint !e534
    if (comparewildcards(FileSpec.Name,((nametype1 *) p)->Name))
      p++;
  }

  if (p == File)
    return(0);

  X = (int) (p - File);
  for (p = File; X; X--, p++) {
    if (pathfileexist(p,NumFiles) != 255)
      if (showline(SrchDskPath,Colors[ANSWER]) == -1)
        return(-1);
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

static void near pascal setsearchlimits(idxhdrtype *Hdr) {
  int  Letter;
  int  Size;

  Extended = Hdr->Type1.Id;

  if (Extended)
    RecSize = sizeof(nametype2);
  else
    RecSize = sizeof(nametype1);

  Letter = FileSpec.Name[0];

  if (Letter >= 'A') {
    if (Letter <= 'Z') {
      Letter -= 'A';
      if (Extended)
        Start = Hdr->Type2.RecOffset[Letter];
      else
        Start = Hdr->Type1.RecOffset[Letter];
      for (Letter++; Letter <= 25; Letter++) {
        if (Extended) {
          if (Hdr->Type2.RecOffset[Letter] != Start) {
            End = Hdr->Type2.RecOffset[Letter];
            goto done;
          }
        } else {
          if (Hdr->Type1.RecOffset[Letter] != Start) {
            End = Hdr->Type1.RecOffset[Letter];
            goto done;
          }
        }
      }
      if (Extended)
        End = Hdr->Type2.NumFiles;
      else
        End = Hdr->Type1.NumFiles;
    } else {
      if (Extended) {
        Start = Hdr->Type2.RecOffset['Z'-'A'];
        End   = Hdr->Type2.NumFiles;
      } else {
        Start = Hdr->Type1.RecOffset['Z'-'A'];
        End   = Hdr->Type1.NumFiles;
      }
    }
  } else {
    Start = 0;
    if (Extended)
      End = (Letter == '?' ? Hdr->Type2.NumFiles : Hdr->Type2.RecOffset['A'-'A']);
    else
      End = (Letter == '?' ? Hdr->Type1.NumFiles : Hdr->Type1.RecOffset['A'-'A']);
  }

done:
  Size = (int) ((((End - Start) >> 1) + 1) * RecSize);
  Size = (Size + 1024) & 0xFC00;  /* AND 0xFC00 rounds to nearest 1024 */
  if (Size > 8192)
    Size = 8192;
  else if (Size < 1024)
    Size = 1024;
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
  memset(FileSpec.Name,' ',FILESIZE);
  NameLen = strlen(Name);
  if (NameLen > FILESIZE+1)
    NameLen = FILESIZE+1;

  if ((p = strnchr(Name,'.',NameLen)) != NULL) {
    p++;
    Len = (int) (p - Name);
    memcpy(FileSpec.Ext,p,NameLen - Len);
    if ((p = strnchr(FileSpec.Ext,'*',3)) != NULL)
      memset(p,'?',3 - (int) (p-FileSpec.Ext));
    NameLen = Len - 1;
  }
  memcpy(FileSpec.Name,Name,NameLen);
  if ((p = strnchr(FileSpec.Name,'*',8)) != NULL)
    memset(p,'?',8 - (int) (p-FileSpec.Name));
}


/********************************************************************
*
*  Function:  searchidxforfiles()
*
*  Desc    :  Searches the index file rather than the DOS PATH to find files.
*
*  Returns :  File attribute of file (if exists) or 255 otherwise
*/

static int near pascal searchidxforfiles(char *IdxName, char *Token) {
  idxhdrtype Hdr;
  int        RetVal;

  stripright(IdxName,'\\');
  if (dosfopen(IdxName,OPEN_READ|OPEN_DENYNONE,&Idx) == -1)
    return(255);

  dossetbuf(&Idx,128);
  dosfread(&Hdr,sizeof(idxhdrtype),&Idx);  //lint !e534

  makefilespec(Token);
  setsearchlimits(&Hdr);

  RetVal = binarysearch(Hdr.Type1.Id == 0 ? Hdr.Type1.NumFiles : Hdr.Type2.NumFiles);

  dosfclose(&Idx);
  return(RetVal);
}


static int near pascal existinpath(char *Name) {
  int Exist;

  dosrewind(&PathListFile);
  while (1) {
    if (dosfgets(SrchDskPath,66,&PathListFile) == -1)
      return(255);
    stripright(SrchDskPath,' ');
    if (SrchDskPath[0] == '%') {
      if ((Exist = searchidxforfiles(&SrchDskPath[1],Name)) != 255) /* this fills DTA too */
        return(Exist);
    } else {
      addbackslash(SrchDskPath,sizeof(SrchDskPath));
      strcat(SrchDskPath,Name);
      if ((Exist = fileexist(SrchDskPath)) != 255)   /* this fills DTA too */
        return(Exist);
    }
  }
}


typedef struct {
  char     Name[13];
  long     Size;
  unsigned Date;
} filelisttype;

static VMDataSet Files;


static int comparerecs(const void *A, const void *B) {
  long         *a = (long *) A;
  long         *b = (long *) B;
  filelisttype *Rec1;
  filelisttype *Rec2;

  Rec1 = (filelisttype *) VMRecordGetByPos(&Files,*a);
  Rec2 = (filelisttype *) VMRecordGetByPos(&Files,*b);

  return(stricmp(Rec1->Name,Rec2->Name));
}

static int bscompare(const void *Name, const void *B) {
  long         *b = (long *) B;
  filelisttype *Rec2;

  Rec2 = (filelisttype *) VMRecordGetByPos(&Files,*b);
  return(stricmp((char *)Name,Rec2->Name));
}


static unsigned near pascal fixdate(int Date) {
  char Str[20];
  return(ctod2(dtoc(Date,Str)));
}



void pascal verifyfiles(char VerifyType, char *DskPath, char *PathListName) {
  int           Exist;
  unsigned      Total;
  unsigned      MaxEntries;
  long          FreeMem;
  long          Counter;
  long         *Index;
  long         *p;
  filelisttype *f;
  parenttype   *parent;
  filelisttype  FoundFile;
  find_t        Find;

  strcpy(SrchDskPath,DskPath);
  SrchOffset = &SrchDskPath[strlen(SrchDskPath)];

  if (VerifyType == 'A' || VerifyType == 'Q') {
    if (dosfopen(PathListName,OPEN_READ|OPEN_DENYNONE,&PathListFile) == -1)
      return;
    dossetbuf(&PathListFile,2048);
    if (VerifyType == 'Q')
      CheckDiskForFiles = FALSE;
  }

  VMInitRec(&Files,NULL,0,sizeof(filelisttype));

  if ((FreeMem = coreleft()) > 65535L)
    FreeMem = 65535L;

  // WARNING:  The method of preloading files from the attached directory, as
  // shown below, is limited to about 16000 entries.  That is because of the
  // 64K limit.  The 64K limit is applied only to the "index array" which is
  // allocated as conventional memory so that qsort() and, more importantly,
  // bsearch(), can be used on it.
  //
  // If someone has more than 16,000 entries in their directory, then those
  // entries after 16,000 are found via a findfirst call which is only
  // performed if Total >= MaxEntries.

  MaxEntries = (int) ((FreeMem - 1024) / sizeof(long));

  if (MaxEntries > 0) {
    if ((Index = (long *) malloc(MaxEntries * sizeof(long))) != NULL) {
      Total = 0;
      p = Index;
      strcpy(SrchOffset,"*.*");
      if (_dos_findfirst(SrchDskPath,FA_ARCH,&Find) == 0) {
        VMAccessAttrSet(&Files,VM_SEQUENTIAL);
        do {
          if (++Total > MaxEntries)
            break;
          f = (filelisttype *) VMRecordCreate(&Files,sizeof(filelisttype),p,NULL);
          strcpy(f->Name,Find.name);
          f->Size = Find.size;
          f->Date = Find.wr_date;
          p++;
        } while (_dos_findnext(&Find) == 0);
      }
      VMAccessAttrSet(&Files,VM_RANDOM);
      qsort(Index,Total,sizeof(long),comparerecs);
    } else {
      Index = NULL;
      Total = MaxEntries = 0;
      VMDone(&Files);
    }
  } else {
    Index = NULL;
    Total = MaxEntries = 0;
    VMDone(&Files);
  }

  FoundFile.Size = 0;
  FoundFile.Date = 0;

  for (Counter = 1; Counter <= TotalParents; Counter++) {
    // throughout the rest of PCBFiler, the Parent dataset is always accessed
    // via the ParentsOnly index.  However, in this case, we don't care what
    // order the records are encountered in, so we'll just access them
    // sequentially, one at a time, via the built-in index.
    parent = (parenttype *) VMRecordGetByIndex(&Parents,Counter,NULL);

    showaction();

    if (parent->Line == DIRLINE) {
      strcpy(SrchOffset,parent->Fields.NName);

      /* see if the file is in the pre-loaded list of files */

      if (Total > 0) {
        if ((p = (long *) bsearch(SrchOffset,Index,Total,sizeof(long),bscompare)) != NULL) {
          FoundFile = *(filelisttype *) VMRecordGetByPos(&Files,*p);
          Exist = 0;
        } else
          Exist = 255;
      } else
        Exist = 255;

      // if Total >= MaxEntries then there are probably other files on disk,
      // so if Exist equals 255 then lets go check the disk

      if (Exist == 255 && Total == MaxEntries) {
        strcpy(SrchDskPath,DskPath);
        strcat(SrchDskPath,parent->Fields.NName);
        if ((Exist = fileexist(SrchDskPath)) != 255) { // this fills DTA too
          strcpy(FoundFile.Name,DTA.ff_name);
          FoundFile.Size = DTA.ff_fsize;
          FoundFile.Date = DTA.ff_fdate;
        }
      }

      if (Exist == 255 && (VerifyType == 'A' || VerifyType == 'Q')) {
        if ((Exist = existinpath(parent->Fields.NName)) != 255) { // this fills DTA too
          strcpy(FoundFile.Name,DTA.ff_name);
          FoundFile.Size = DTA.ff_fsize;
          FoundFile.Date = DTA.ff_fdate;
        }
      }

      if (Exist != 255) {
        if (VerifyType != 'Q') {
          if (parent->Fields.FDate == 0xffff || parent->Fields.FDate == 0xfffe) {
            /* avoid updating SIZE and DATE information if the DATE field */
            /* is set to either OFFLINE (=0xffff) or DELETE (=0xfffe) */
            if (FoundFile.Size != 0) {
              parent->Fields.FSize = FoundFile.Size;
              parent->Fields.FDate = parent->Fields.NDate = fixdate(FoundFile.Date);
            }
          } else {
            parent->Fields.FSize = FoundFile.Size;
            if (Work.UseDiskDate) {
              parent->Fields.FDate = fixdate(FoundFile.Date);
              parent->Fields.NDate = fixdate(FoundFile.Date);
            }
          }
        }
      } else {
        parent->Fields.Exist = LISTING;
      }
      VMRecordChanged(&Parents);
    }
  }

  if (VerifyType == 'A' || VerifyType == 'Q') {
    dosfclose(&PathListFile);
    if (VerifyType == 'Q')
      CheckDiskForFiles = TRUE;
  }

  if (MaxEntries > 0) {
    if (Index != NULL)
      free(Index);
    VMDone(&Files);
  }
}


int pascal verifyexist(char VerifyType, char *DskPath, char *PathListName, char *Name) {
  int Exist;

  strcpy(SrchDskPath,DskPath);
  strcat(SrchDskPath,Name);
  Exist = fileexist(SrchDskPath);

  if (Exist == 255 && (VerifyType == 'A' || VerifyType == 'Q') && PathListName[0] != 0) {
    if (dosfopen(PathListName,OPEN_READ|OPEN_DENYNONE,&PathListFile) == -1)
      return(255);
    if (VerifyType == 'Q')
      CheckDiskForFiles = FALSE;
    dossetbuf(&PathListFile,2048);
    Exist = existinpath(Name);
    dosfclose(&PathListFile);
    if (VerifyType == 'Q')
      CheckDiskForFiles = TRUE;
  }

  return(Exist);
}


/********************************************************************
*
*  Function:  findfileindlpathrecord()
*
*  Desc    :  Tests to see if a file (can be wildcard) exists in a dlpath.lst
*             record - whether that record is a PATH or an INDEX.  Files that
*             are found are shown by calling showline() to display them on the
*             screen.
*
*  Returns :  -1 if the user requested the search be aborted, otherwise 0.
*/

int pascal findfileindlpathrecord(char *PathRec, char *Token) {
  idxhdrtype   Hdr;
  int          RetVal;
  char         *p;
  struct ffblk Rec;

  RetVal = 0;

  if (PathRec[0] == '%') {
    stripright(PathRec,'\\');
    if (dosfopen(&PathRec[1],OPEN_READ|OPEN_DENYNONE,&Idx) == -1)
      return(0);

    dossetbuf(&Idx,128);
    dosfread(&Hdr,sizeof(idxhdrtype),&Idx); //lint !e534

    makefilespec(Token);
    setsearchlimits(&Hdr);

    if ((strpbrk(Token,"*?") != NULL))
      RetVal = wildfindfiles(Hdr.Type1.Id == 0 ? Hdr.Type1.NumFiles : Hdr.Type2.NumFiles);
    else
      if (binarysearch(Hdr.Type1.Id == 0 ? Hdr.Type1.NumFiles : Hdr.Type2.NumFiles) != 255)
        RetVal = showline(SrchDskPath,Colors[ANSWER]);

    dosfclose(&Idx);
  } else {
    p = &PathRec[strlen(PathRec)];
    strcpy(p,Token);
    if (findfirst(PathRec,&Rec,FA_ARCH) == 0) {         //lint !e1066
      while (1) {
        strcpy(p,Rec.ff_name);
        if (showline(PathRec,Colors[ANSWER]) == -1)
          return(-1);
        if (findnext(&Rec) != 0)                        //lint !e1066
          break;
        if (userabort())
          return(-1);
      }
    }
  }

  return(RetVal);
}
