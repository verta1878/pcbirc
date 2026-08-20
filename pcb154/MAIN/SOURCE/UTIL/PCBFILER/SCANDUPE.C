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


#include <io.h>
#include <dir.h>
#include <dos.h>
#include <alloc.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

extern struct ffblk DTA;   /* really declared as "char DTA[45]" in exist.c */

typedef enum { DIRNUM, DISKPATH, INDEX } storage;

typedef struct {
  unsigned ConfNum;       // Conference number
  unsigned DirNum;        // Directory number
} dirloc;

typedef struct {
  ulong    FileNum;       // File Number inside of DLPATH.LST
  ulong    PathNum;       // Path Number inside of IDX file, if Type==INDEX
} physloc;

typedef struct {
  VMAVLInfo Reserved;
  storage   Type;
  char      FileName[13];
  unsigned  FileDate;
  long      FileSize;
  long      Pos;          // used in dupes - it gives Pos of original
  union {
    physloc Physical;
    dirloc  Coord;
  } Loc;
} scantype;

static DOSFILE    Analysis;

static VMDataSet     OnDisk;           // files found on disk or in index
static VMAVLControl  OnDiskControl;    // AVL Control for OnDisk dataset
static VMAVLTree     OnDiskTree;       // AVL Tree for OnDisk dataset
static VMDataSet     OnDiskDupes;      // Duplicate files, not added to OnDisk
static long          DiskTotal;        // total number of files found on disk
static long          DiskDupeTotal;    // total number of disk duplicates
static long          DiskNonDupeTotal; // total number of non-dupe files

static VMDataSet     InList;           // files found inside of DIR files
static VMAVLControl  InListControl;    // AVL Control for InList dataset
static VMAVLTree     InListTree;       // AVL Tree for InList dataset
static VMDataSet     InListDupes;      // Duplicate files, not added to OnDisk
static long          ListTotal;        // total number of files found in list
static long          ListDupeTotal;    // total number of file duplicates
static long          ListNonDupeTotal; // total number of non-dupe files

static ulong    SaveFileNum = 0xFFFFFFFFL;
static ulong    SavePathNum = 0xFFFFFFFFL;
static char     SavePath[66];


/********************************************************************
*
*  Function:  noworktodo()
*
*  Desc    :  A simple pop-up message to tell the sysop that, according to the
*             current configuration, there is nothing to do (nothing to scan).
*/

void  noworktodo(char *Str) {
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Msg1    = "Error!  I have no work to do!";
  MsgData.Line1   = 15;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Msg2    = "Return to the Main Menu, select Defaults Page 2 and enter a";
  MsgData.Line2   = 17;
  MsgData.Color2  = Colors[ANSWER];
  MsgData.Msg3    = Str;
  MsgData.Line3   = 18;
  MsgData.Color3  = Colors[ANSWER];
  showmessage();
}


/********************************************************************
*
*  Function:  makepathlist()
*
*  Desc    :  Checks a bit array to see which conferences should be scanned.
*
*             Then, using the unique class, it keeps track of only those
*             DLPATH.LST files, paths and index files that are not duplicates.
*             It reads each of the paths and indexes into memory and prints
*             them in the analysis file at the same time.
*
*  Returns :  -1 if an error occured writing to the analysis file or if the
*             user aborted the process, otherwise it returns 0.
*/

static int near  makepathlist(char *BitArray, uniqueunlimited & DlName, uniqueunlimited & DlList) {
  unsigned    Board;
  DOSFILE     PathFile;
  char        Temp[66];
  pcbconftype Conf;

  fastprint(3,5,"Scanning configuration...",Colors[DISPLAY]);

  for (Board = 0; Board < PcbData.NumAreas; Board++) {
    if (isset(BitArray,Board)) {
      getconfrecord(Board,&Conf);

      DlName.putinlist(Conf.PubUpldLoc);  //lint !e534
      DlName.putinlist(Conf.PrvUpldLoc);  //lint !e534

      if (Conf.PthNameLoc[0] > 0) {
        if (! DlList.foundinlist(Conf.PthNameLoc)) {
          if (fileexist(Conf.PthNameLoc) != 255 && dosfopen(Conf.PthNameLoc,OPEN_READ|OPEN_DENYNONE,&PathFile) != -1) {
            dossetbuf(&PathFile,4096);
            while (dosfgets(Temp,sizeof(Temp)-1,&PathFile) != -1) {
              Temp[31] = 0;                    /* make sure it is null terminated with a max of 31 characters */
              stripright(Temp,' ');
              DlName.putinlist(Temp);     //lint !e534
              if (userabort()) {
                dosfclose(&PathFile);
                return(-1);
              }
              showaction();
            }
            dosfclose(&PathFile);
          }
        }
      }
    }
  }

  return(0);
}


/********************************************************************
*
*  Function:  addfilesfromindex()
*
*  Desc    :  Reads an index file and adds all of the filenames from it into
*             the OnDisk dataset.
*
*             If any filenames are duplicates of names already in the OnDisk
*             dataset, then they are stored in the OnDiskDupes dataset instead.
*
*             The Counter value is used to record the number of Index File
*             that is being read so that later, if an entry needs to be
*             printed, it can locate the Index File and then, utilizing the
*             PathNum, locate the path from the index file.
*
*  Returns :  -1 if the user aborts the process, 0 otherwise
*/

static int near  addfilesfromindex(char *IndexName, int Counter) {
  VMBool      Added;
  int         RecSize;
  long        NumFiles;
  scantype   *p;
  nametype    Name;
  scantype    DiskFile;
  DOSFILE     Index;
  idxhdrtype  Hdr;

  if (fileexist(IndexName) != 255 && dosfopen(IndexName,OPEN_READ|OPEN_DENYNONE,&Index) == -1)
    return(0);

  dosfread(&Hdr,sizeof(idxhdrtype),&Index); //lint !e534

  if (Hdr.Type1.Id == 0) {
    RecSize  = sizeof(nametype1);
    NumFiles = Hdr.Type1.NumFiles;
  } else {
    RecSize  = sizeof(nametype2);
    NumFiles = Hdr.Type2.NumFiles;
  }

  for (; NumFiles > 0; NumFiles--) {
    if (dosfread(&Name,RecSize,&Index) == -1)
      break;

    DiskFile.Type = INDEX;
    DiskFile.Loc.Physical.FileNum = Counter;

    if (Hdr.Type1.Id == 0)
      DiskFile.Loc.Physical.PathNum = Name.Type1.PathNum;
    else
      DiskFile.Loc.Physical.PathNum = Name.Type2.PathNum;

    convertspectoname(DiskFile.FileName,&Name);

    // simultaneously add the record into OnDisk AND into the AVL tree
    DiskFile.Pos = VMAVLSearch(&OnDiskTree,&OnDiskControl,&DiskFile,VM_TRUE,&Added);

    if (! Added) {
      // the AVL insertion failed - that means it's a duplicate, so
      // manually add it to the OnDiskDupes dataset.
      p = (scantype *) VMRecordCreate(&OnDiskDupes,sizeof(scantype),NULL,NULL);
      *p = DiskFile;
      DiskDupeTotal++;
    }

    DiskTotal++;
    showaction();
    if (userabort()) {
      dosfclose(&Index);
      return(-1);
    }
  }

  dosfclose(&Index);
  return(0);
}


/********************************************************************
*
*  Function:  addfilesfrompath()
*
*  Desc    :  Using a path, it scans all files in that directory and stores
*             them in the OnDisk dataset.
*
*             If any filenames are duplicates of names already in the OnDisk
*             dataset, then they are stored in the OnDiskDupes dataset instead.
*
*             The Counter value is used to record the number of Path in DLPATH
*             that is being read so that later, if an entry needs to be
*             printed, it can locate the Path.
*
*  Returns :  -1 if the user aborts the process, 0 otherwise
*/

static int near  addfilesfrompath(char *Path, int Counter) {
  VMBool    Added;
  scantype *p;
  scantype  DiskFile;
  find_t    Find;

  strcat(Path,"*.*");
  if (_dos_findfirst(Path,FA_ARCH,&Find) == 0) {
    do {
      if (Find.attrib != FA_DIREC) {
        DiskFile.Type = DISKPATH;
        DiskFile.FileDate = Find.wr_date;
        DiskFile.FileSize = Find.size;
        DiskFile.Loc.Physical.FileNum = Counter;
        strcpy(DiskFile.FileName,Find.name);

        // simultaneously add the record into OnDisk AND into the AVL tree
        DiskFile.Pos = VMAVLSearch(&OnDiskTree,&OnDiskControl,&DiskFile,VM_TRUE,&Added);

        if (! Added) {
          // the AVL insertion failed - that means it's a duplicate, so
          // manually add it to the OnDiskDupes dataset.
          p = (scantype *) VMRecordCreate(&OnDiskDupes,sizeof(scantype),NULL,NULL);
          *p = DiskFile;
          DiskDupeTotal++;
        }

        DiskTotal++;
        showaction();
        if (userabort())
          return(-1);
      }
    } while (_dos_findnext(&Find) == 0);
  }
  return(0);
}


/********************************************************************
*
*  Function:  getdiskentries()
*
*  Desc    :  Records the name of the path being scanned in the analysis file.
*
*             Then scans that path (or index file) for files and adds them
*             into the OnDisk and OnDiskDupes datasets.
*
*  Returns :  -1 if an error occurs writing to the analysis file, or if the
*             user aborts the process, otherwise 0.
*/

static int near  getdiskentries(uniqueunlimited & DlName) {
  char        Path[40];
  int         Counter;

  fastprint(3,6,"Scanning download paths...",Colors[DISPLAY]);

  if (dosfputs("\r\nDisk directories scanned\r\n========================\r\n",&Analysis) == -1)
    return(-1);

  DiskTotal = 0;

  for (Counter = 0; Counter < DlName.numitemsinlist(); Counter++) {
    strcpy(Path,DlName[Counter]);
    if (dosfputs(Path,&Analysis) == -1 || dosfputs("\r\n",&Analysis) == -1)
      return(-1);
    if (Path[0] == '%') {
      stripright(Path,'\\');   /* remove trailing backslash, if any */
      if (addfilesfromindex(&Path[1],Counter) == -1)
        return(-1);
    } else {
      if (addfilesfrompath(Path,Counter) == -1)
        return(-1);
    }
  }

  if (dosfputs("\r\n",&Analysis) == -1)
    return(-1);
  return(0);
}


/********************************************************************
*
*  Function:  getdirentries()
*
*  Desc    :  Using a bit array, scans only the specified conferences.
*
*             Using the unique class, it keeps track of the DIR.LST and DIR
*             files that have been scanned so that the files are scanned only
*             once.
*
*             Then it reads in the unique DIR files and stores all of the
*             filenames in the InList dataset.  Any duplicate filenames that
*             are found are stored in the InListDupes dataset instead.
*
*  Returns :  -1 if an error occurs writing to the analysis file, or if the
*             user aborts the process, otherwise 0.
*/

static int near  getdirentries(char *BitArray, uniqueunlimited & DirName, uniqueunlimited & DirList) {
  VMBool       Added;
  unsigned     BoardNumber;
  unsigned     HighNum;
  unsigned     Num;
  unsigned     NumTextDirs;
  int          TestValue;
  int          DirLstFile;
  scantype    *p;
  scantype     DirFile;
  DOSFILE      InFile;
  DirListType  Rec;
  char         LineIn[128];
  pcbconftype  Conf;

  fastprint(3,7,"Scanning DIR files...",Colors[DISPLAY]);

  DirLstFile  = -1;
  if (dosfputs("DIR files scanned\r\n=================\r\n",&Analysis) == -1)
    goto abort;

//NumNonDupes = 0;

  for (BoardNumber = 0; BoardNumber < PcbData.NumAreas; BoardNumber++) {
    if (isset(BitArray,BoardNumber)) {
      getconfrecord(BoardNumber,&Conf);
      if (! DirList.foundinlist(Conf.DirNameLoc) && fileexist(Conf.DirNameLoc) != 255) {
        DirLstFile = dosopencheck(Conf.DirNameLoc,OPEN_READ|OPEN_DENYNONE);
        NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));

        HighNum = NumTextDirs;
        if (Conf.UpldDir[0] != 0)
          HighNum++;

        for (Num = 0; Num <= HighNum; Num++) {
          if (fastfinddirpath(Num,&Rec,&Conf,NumTextDirs,DirLstFile) != -1) {
            if (! DirName.foundinlist(Rec.DirPath)) {
              if (Rec.DirPath[0] > 0 && fileexist(Rec.DirPath) != 255 && dosfopen(Rec.DirPath,OPEN_READ|OPEN_DENYNONE,&InFile) != -1) {
                dossetbuf(&InFile,16384);
                if (dosfputs(Rec.DirPath,&Analysis) == -1 || dosfputs("\r\n",&Analysis) == -1) {
                  dosfclose(&InFile);
                  goto abort;
                }

                while (dosfgets(LineIn,sizeof(LineIn)-1,&InFile) != -1) {
                  LineIn[31] = 0;
                  if (LineIn[0] > ' ' && strlen(LineIn) >= 30 && (TestValue = ctod2(&LineIn[23])) != 0) {
                    DirFile.Type = DIRNUM;
                    memcpy(DirFile.FileName,LineIn,12); DirFile.FileName[12] = 0;
                    stripright(DirFile.FileName,' ');
                    DirFile.FileDate = TestValue;
                    LineIn[21] = 0;
                    DirFile.FileSize = atol(&LineIn[12]);
                    DirFile.Loc.Coord.ConfNum = BoardNumber;
                    DirFile.Loc.Coord.DirNum  = Num;

                    // simultaneously into InList AND into the AVL tree
                    DirFile.Pos = VMAVLSearch(&InListTree,&InListControl,&DirFile,VM_TRUE,&Added);

                    if (! Added) {
                      // the AVL insertion failed - that means it's a duplicate, so
                      // manually add it to the OnDiskDupes dataset.
                      p = (scantype *) VMRecordCreate(&InListDupes,sizeof(scantype),NULL,NULL);
                      *p = DirFile;
                      ListDupeTotal++;
                    }

                    ListTotal++;
                    showaction();
                    if (userabort()) {
                      dosfclose(&InFile);
                      goto abort;
                    }
                  }
                }
                dosfclose(&InFile);
              }
            }
          }
        }
        if (DirLstFile != -1)
          dosclose(DirLstFile);
      }
    }
  }

  if (dosfputs("\r\n",&Analysis) == -1)
    return(-1);
  return(0);

abort:
  if (DirLstFile != -1)
    dosclose(DirLstFile);
  return(-1);
}


/********************************************************************
*
*  Function:  comparenames()
*
*  Desc    :  Used for the OnDisk and InList AVL trees, and also when sorting
*             the OnDiskDupes and InListDupes datasets.
*
*  Returns :  an integer to indicate which filename should sort higher
*/

static VM_SHINT comparenames(const void *a, const void *b) {
  return(strcmp( ((scantype *) a)->FileName, ((scantype *) b)->FileName));
}



static VM_SHINT similarnames(const void *a, const void *b) {
  int   Len;
  char *p = ((scantype *) a)->FileName;
  char *q = ((scantype *) b)->FileName;

  // scan for equivalent letters, Len will be equal to the number of matches
  // found.  If the entire base of the filename (ignoring the extension) is
  // found to be identical, as signified by encountering a NUL or Period,
  // then it returns a 0 to indicate equality.

  // NOTE:  for the purposes of equality, two characters are considered to be
  // equal if the are either equal to each other, or if a "-" and "_" are
  // being compared.

  for (Len = 0; (*p == *q) || (*p == '_' && *q == '-') || (*p == '-' && *q == '_'); p++, q++, Len++) {
    // reaching the period (or end of filename) and still being equal
    // means that the filenames are either the same or else the same minus
    // the extension so check for that condition and return now if it is true
    if (*p == '.' || *p == 0)
      return(0);
  }

  // if there were 2 or more matching letters, then check to see if the rest of
  // the letters are digits (indicating possible revisional changes).

  if (Len > 1) {
    if (*p <= '9' && *q <= '9' && *p >= '0' && *q <= '0') {
      return(0);
    } else if (Len > 2) {
      p--;
      q--;
      if ((*p <= '9' && *p >= '0') || *p == '-' || *p == '_')
        if (*q != 0 && *q != '.')
          return(0);
    }
  }

  return(strcmp(p,q));
}




/********************************************************************
*
*  Function:  filesindirnotdisk()
*
*  Desc    :  Scans for files that are listed in the DIR files but which are
*             not found on disk.  It does this by scanning the non-duplicate
*             files in the InList dataset, in order, while using the AVL tree
*             to search for files in the OnDisk dataset.
*
*  Returns :  -1 if an error occurs writing to the analysis file, or if the
*             user aborts the process, otherwise 0.
*/

static int near  filesindirnotdisk(void) {
  long        Count;
  scantype   *p;
  char        Temp[10];
  char        Temp2[100];
  pcbconftype Conf;

  fastprint(3,10,"Scanning for files listed in DIRs but missing from disk...",Colors[DISPLAY]);
  if (dosfputs("\r\n\r\nFiles listed in DIR files but missing from Disk\r\n-----------------------------------------------\r\n",&Analysis) == -1)
    goto abort;

  // Loop thru each file found in DIRs until all files have been examined.
  for (Count = 1; Count <= ListNonDupeTotal; Count++) {

    // Get filename found in DIRs
    p = (scantype *) VMRecordGetByIndex(&InList,Count,NULL);

    // Search for the same filename inside of the DISK listing
    if (VMAVLSearch(&OnDiskTree,&OnDiskControl,p,VM_FALSE,NULL) == VM_INVALID_POS) {

      // No match was found, show this file as missing from the DISK
      getconfrecord(p->Loc.Coord.ConfNum,&Conf);
      sprintf(Temp2,"%-12.12s%9ld  %s  %-15.15sDIR: %d\r\n",
              p->FileName,
              p->FileSize,
              countrydate(dtoc(p->FileDate,Temp)),
              Conf.Name,
              p->Loc.Coord.DirNum);
      if (dosfputs(Temp2,&Analysis) == -1)
        goto abort;
    }

    showaction();
    if (userabort())
      goto abort;
  }
  return(0);

abort:
  return(-1);
}


/********************************************************************
*
*  Function:  getpathfromindex()
*
*  Desc    :  Reads an IDX file and, using a previously recorded Path Number,
*             jumps a stored path within the file.
*
*  Returns :  The Path string is filled with the path from the index.
*/

static void near  getpathfromindex(char *IdxName, long PathNum, char *Path) {
  int        File;
  int        RecSize;
  long       NumFiles;
  long       Offset;
  idxhdrtype Hdr;

  Path[0] = 0;

  if ((File = dosopencheck(IdxName,OPEN_READ|OPEN_DENYNONE)) == -1)
    return;

  if (readcheck(File,&Hdr,sizeof(Hdr)) != (unsigned) -1) {
    if (Hdr.Type1.Id == 0) {
      RecSize  = sizeof(nametype1);
      NumFiles = Hdr.Type1.NumFiles;
    } else {
      RecSize  = sizeof(nametype2);
      NumFiles = Hdr.Type2.NumFiles;
    }
    Offset  = sizeof(Hdr) + ((long) NumFiles * RecSize);
    Offset += (long) PathNum * sizeof(pathtype);
    doslseek(File,Offset,SEEK_SET);
    readcheck(File,Path,sizeof(pathtype));  //lint !e534
  }
  dosclose(File);
}


static bool near  slowdrive(char DriveLetter) {
  char *p;

  DriveLetter = toupper(DriveLetter);

  for (p = PcbData.SlowDrives; *p > 0; p++) {
    if (*p > ' ') {
      if (*p == DriveLetter)
        return(TRUE);

      if (*p == '-' && p != PcbData.SlowDrives) {
        if (*(p-1) < DriveLetter && *(p+1) >= DriveLetter)
          return(TRUE);
      }
    }
  }
  return(FALSE);
}


/********************************************************************
*
*  Function:  getdisklocation()
*
*  Desc    :  If the file being checked was read in from an index, then this
*             function has to call getpathfromindex() to get the path.  Then
*             it checks for the file's existence to obtain the file's date
*             and size.
*
*             NOTE:  If the drive letter is from the Slow Drive list, then
*                    PCBFILER will avoid loading the CD into the CD-Changer
*                    and will record the date/time as 0xFFFF.  The reporting
*                    function will then show the date/time as being "CD-ROM".
*
*             If the file being checked was read in from a path, then it
*             simply uses the recorded path.
*
*             To save time, this function remembers the last path found in an
*             index so that, if the same index and path number are requested
*             again, it won't have to re-open and re-read the index file.
*
*  Returns :  Path is filled with the path that corresponds with the file.
*/

static void near  getdisklocation(char *Path, scantype *Buf, uniqueunlimited & DlName) {
  char Temp[66];

  if (Buf->Type == INDEX) {
    if (SaveFileNum == Buf->Loc.Physical.FileNum && SavePathNum == Buf->Loc.Physical.PathNum)
      strcpy(Path,SavePath);
    else {
      getpathfromindex(&DlName[Buf->Loc.Physical.FileNum][1],Buf->Loc.Physical.PathNum,Path);
      SaveFileNum = Buf->Loc.Physical.FileNum;
      SavePathNum = Buf->Loc.Physical.PathNum;
      strcpy(SavePath,Path);
    }

    if (Path[1] == ':' && slowdrive(Path[0])) {
      Buf->FileSize = 0xFFFFFFFFL;
      Buf->FileDate = 0xFFFF;
      return;
    }

    strcpy(Temp,Path);
    strcat(Temp,Buf->FileName);
    if (fileexist(Temp) != 255) {
      Buf->FileSize = DTA.ff_fsize;
      Buf->FileDate = DTA.ff_fdate;
    } else {
      Buf->FileSize = 0;
      Buf->FileDate = 0;
    }
  } else
    strcpy(Path,DlName[Buf->Loc.Physical.FileNum]);
}


/********************************************************************
*
*  Function:  printdiskfile()
*
*  Desc    :  Prints a file, with its associated date, size and path.
*
*  Returns :  -1 if an error occured writing to the analysis file, 0 otherwise
*/

static int near  printdiskfile(scantype *Buf, char *Path) {
  char DateStr[10];
  char Str[128];

  sprintf(Str,"%-12.12s",Buf->FileName);
  if (dosfputs(Str,&Analysis) == -1)
    return(-1);

  if (Buf->FileDate == 0xFFFF && Buf->FileSize == 0xFFFFFFFFL)
    sprintf(Str,"   -- SLOW DRIVE --  %s\r\n",Path);
  else
    sprintf(Str,"%9ld  %s  %s\r\n",
            Buf->FileSize,
            countrydate(dtoc(Buf->FileDate,DateStr)),
            Path);

  return(dosfputs(Str,&Analysis));
}


/********************************************************************
*
*  Function:  dupfilesondisk()
*
*  Desc    :  Sorts the OnDiskDupes dataset, which is a list of duplicates.
*
*             Then it displays, in order, those duplicates that exist, together
*             with the original (first file encountered).
*
*  Returns :  -1 if an error occurs writing to the analysis file, or if the
*             user aborts the process, otherwise 0.
*/

static int near  dupfilesondisk(uniqueunlimited & DlName) {
  scantype  *p1;
  scantype  *p2;
  long       Count;
  char       SaveName[13];
  char       Path1[66];
  char       Path2[66];
  scantype   SortBuf[128];

  fastprint(3,11,"Scanning for duplicate files found on disk...",Colors[DISPLAY]);
  if (dosfputs("\r\n\r\nDuplicate Files found in Disk subdirectories\r\n--------------------------------------------",&Analysis) == -1)
    return(-1);

  if (DiskDupeTotal == 0)
    goto end;

  // Sort the list so that we can group multiple files together without
  // putting a blank line in between them.

  VMSort(&OnDiskDupes,sizeof(scantype),1,DiskDupeTotal,VM_FALSE,comparenames,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));

  // NOTE: Down below, it is assumed the filenames are the same because
  // these are all duplicates.  The Pos field points to the original file
  // (i.e. the first one found).
  //
  // Also note that there is a strcmp() call which checks the path of the
  // two files.  This is because the file may have been encountered twice,
  // by two different sources (directory & index), which means it's not a
  // duplicate at all.  If the paths are the same, then don't print it.

  for (Count = 1, SaveName[0] = 0, Path2[0] = 0; Count <= DiskDupeTotal; Count++) {
    showaction();
    if (userabort())
      return(-1);

    p1 = (scantype *) VMRecordGetByIndex(&OnDiskDupes,Count,NULL);
    getdisklocation(Path1,p1,DlName);

    // If the current filename is different from the last filename, then
    // move down to the next line in the file and print the original
    // filename from the OnDisk dataset followed by the current filename.

    if (strcmp(p1->FileName,SaveName) != 0) {
      strcpy(SaveName,p1->FileName);
      p2 = (scantype *) VMRecordGetByPos(&OnDisk,p1->Pos);
      getdisklocation(Path2,p2,DlName);
      if (strcmp(Path1,Path2) != 0) {
        if (dosfputs("\r\n",&Analysis) == -1 || printdiskfile(p2,Path2) == -1 || printdiskfile(p1,Path1) == -1)
          return(-1);
        continue;       // continue back up to get the next filename
      }
    }

    // This filename is the same as the last one, so check the path to make
    // sure it's not in the same path as the original file and then print
    // this one.

    if (strcmp(Path1,Path2) != 0) {
      if (printdiskfile(p1,Path1) == -1)
        return(-1);
    }

  }

end:
  return(dosfputs("\r\n",&Analysis));
}


/********************************************************************
*
*  Function:  filesondisknotdir()
*
*  Desc    :  Scans for files that are on disk, but which are not listed in
*             the DIR files.  It does this by scanning the non-duplicate
*             files in the OnDisk dataset, in order, while using the AVL tree
*             to search for files in the InList dataset.
*
*  Returns :  -1 if an error occurs writing to the analysis file, or if the
*             user aborts the process, otherwise 0.
*/

static int near  filesondisknotdir(uniqueunlimited & DlName) {
  long      Count;
  scantype *p;
  char      Path[66];


  fastprint(3,12,"Scanning for files on disk but missing from DIRs...",Colors[DISPLAY]);
  if (dosfputs("\r\n\r\nFiles found on Disk but missing from DIR files\r\n----------------------------------------------\r\n",&Analysis) == -1)
    return(-1);

  // Loop thru each file found on disk until all files have been examined.
  for (Count = 1; Count <= DiskNonDupeTotal; Count++) {

    // Get filename found on disk
    p = (scantype *) VMRecordGetByIndex(&OnDisk,Count,NULL);

    // Search for the same filename inside of the DIR file listing
    if (VMAVLSearch(&InListTree,&InListControl,p,VM_FALSE,NULL) == VM_INVALID_POS) {

      // No match was found, show this file as missing from the DIR files
      getdisklocation(Path,p,DlName);
      if (printdiskfile(p,Path) == -1 || userabort())
        return(-1);
    }

    showaction();
    if (userabort())
      return(-1);
  }
  return(0);
}


/********************************************************************
*
*  Function:  printdirfile()
*
*  Desc    :  Prints a file, with its associated date, size, conference name
*             and directory number.
*
*  Returns :  -1 if an error occured writing to the analysis file, 0 otherwise
*/

static int near  printdirfile(scantype *Buf, pcbconftype *Conf) {
  char DateStr[10];
  char Str[128];

  sprintf(Str,"%-12s%9ld  %s  %-15.15sDIR: %d\r\n",
          Buf->FileName,
          Buf->FileSize,
          countrydate(dtoc(Buf->FileDate,DateStr)),
          Conf->Name,
          Buf->Loc.Coord.DirNum);
  return(dosfputs(Str,&Analysis));
}


/********************************************************************
*
*  Function:  dupfilesindir()
*
*  Desc    :  Sorts the InListDupes dataset, which is a list of duplicates.
*
*             Then it displays, in order, those duplicates that exist, together
*             with the original (first file encountered).
*
*  Returns :  -1 if an error occurs writing to the analysis file, or if the
*             user aborts the process, otherwise 0.
*/

static int near  dupfilesindir(void) {
  scantype     *p1;
  scantype     *p2;
  long          Count;
  char          SaveName[13];
  pcbconftype   Conf1;
  pcbconftype   Conf2;
  scantype      SortBuf[128];

  fastprint(3,13,"Scanning for duplicate files found in DIR files...",Colors[DISPLAY]);
  if (dosfputs("\r\n\r\nDuplicate Files found in DIR files\r\n----------------------------------",&Analysis) == -1)
    return(-1);

  if (ListDupeTotal == 0)
    goto end;

  // Sort the list so that we can group multiple files together without
  // putting a blank line in between them.

  VMSort(&InListDupes,sizeof(scantype),1,ListDupeTotal,VM_FALSE,comparenames,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));

  // NOTE: Down below, it is assumed the filenames are the same because
  // these are all duplicates.  The Pos field points to the original file
  // (i.e. the first one found).

  for (Count = 1, SaveName[0] = 0; Count <= ListDupeTotal; Count++) {
    showaction();
    if (userabort())
      return(-1);

    p1 = (scantype *) VMRecordGetByIndex(&InListDupes,Count,NULL);
    getconfrecord(p1->Loc.Coord.ConfNum,&Conf1);

    // If the current filename is different from the last filename, then
    // move down to the next line in the file and print the original
    // filename from the InList dataset followed by the current filename.

    if (strcmp(p1->FileName,SaveName) != 0) {
      strcpy(SaveName,p1->FileName);
      p2 = (scantype *) VMRecordGetByPos(&InList,p1->Pos);

      // avoid re-getting conf info if p1 & p2 are in the same conf
      if (p1->Loc.Coord.ConfNum == p2->Loc.Coord.ConfNum)
        Conf2 = Conf1;
      else
        getconfrecord(p2->Loc.Coord.ConfNum,&Conf2);

      if (dosfputs("\r\n",&Analysis) == -1 || printdirfile(p2,&Conf2) == -1 || printdirfile(p1,&Conf1) == -1)
        return(-1);
      continue;       // continue back up to get the next filename
    }

    // This filename is the same as the last one, print it without a
    // preceeding blank line

    if (printdirfile(p1,&Conf1) == -1)
      return(-1);
  }

end:
  return(dosfputs("\r\n",&Analysis));
}



static int near  similarfilesindir(void) {
  unsigned        ConfNum;
  long            Count;
  char            Save[13];
  scantype        Rec1;
  scantype        Rec2;
  pcbconftype     Conf;
  scantype        SortBuf[128];

  fastprint(3,14,"Scanning for similar files found in DIR files...",Colors[DISPLAY]);
  if (dosfputs("\r\n\r\nSimilar Files found in DIR files\r\n----------------------------------",&Analysis) == -1)
    return(-1);

  if (ListNonDupeTotal < 2)
    return(0);

  // NOTE:  I believe this will destroy the AVL tree's usefulness!

  VMSort(&InList,sizeof(scantype),1,ListNonDupeTotal,VM_FALSE,similarnames,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));

  Rec1 = *(scantype *) VMRecordGetByIndex(&InList,1,NULL);

  for (Count = 2, ConfNum = 0xFFFF, Save[0] = 0; Count <= ListNonDupeTotal; Count++) {
    showaction();
    if (userabort())
      return(-1);

    Rec2 = *(scantype *) VMRecordGetByIndex(&InList,Count,NULL);

    if (similarnames(&Rec1,&Rec2) == 0) {
      if (strcmp(Save,Rec1.FileName) != 0) {
        if (Rec1.Loc.Coord.ConfNum != ConfNum) {
          ConfNum = Rec1.Loc.Coord.ConfNum;
          getconfrecord(ConfNum,&Conf);
        }
        if (dosfputs("\r\n",&Analysis) == -1)
          return(-1);
        if (printdirfile(&Rec1,&Conf) == -1)
          return(-1);
        strcpy(Save,Rec1.FileName);
      }

      // avoid re-getting conf info if p1 & p2 are in the same conf
      if (Rec2.Loc.Coord.ConfNum != ConfNum) {
        ConfNum = Rec2.Loc.Coord.ConfNum;
        getconfrecord(ConfNum,&Conf);
      }

      if (printdirfile(&Rec2,&Conf) == -1)
        return(-1);
    } else {
      Rec1 = Rec2;
    }
  }

  return(dosfputs("\r\n",&Analysis));
}


/********************************************************************
*
*  Function:  scandupe()
*
*  Desc    :  Using a bit array, this function scans the conferences that are
*             selected and fills several datasets with the names of files that
*             are on disk and in the DIR files.  The datasets used are as
*             follows:
*
*                OnDisk      - Non-duplicate files found on disk
*                OnDiskDupes - Duplicate files found on disk
*
*                InList      - Non-duplicate files found in the DIR files
*                InListDupes - Duplicate files found in the DIR files
*
*             Finally, it analyzes and prints a report showing which files are
*             missing and which files are duplicates.
*/

void  scandupe(void) {
  char           *BitArray;
  uniqueunlimited DirList(DIRLIST_LENGTH);
  uniqueunlimited DlList(DIRLIST_LENGTH);
  uniqueunlimited DirName(DIRNAME_LENGTH);
  uniqueunlimited DlName(DLPATH_LENGTH);
  char            List[80];
  char            Temp[160];
  char            OnDiskBuffer[1024];
  char            InListBuffer[1024];

  if (! (DirList.Allocated && DirName.Allocated && DlList.Allocated && DlName.Allocated))
    return;

  if (BatchMode && strlen(CommandLine) > 4 && CommandLine[4] == ':')
    strcpy(List,&CommandLine[5]);
  else
    strcpy(List,Work.ScanList);

  if ((BitArray = makebitarray(List)) == NULL) {
    if (! BatchMode)
      noworktodo("list of conferences to scan for duplicate or missing files.");
    return;
  }

  if (dosfopen("ANALYSIS.RPT",OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&Analysis) == -1)
    goto end;

  dossetbuf(&Analysis,8192);

  VMInitRec(&OnDisk,OnDiskBuffer,sizeof(OnDiskBuffer),sizeof(scantype));
  VMAVLControlInit(&OnDisk,&OnDiskControl,comparenames,0);
  VMAVLTreeInit(&OnDiskTree,1);

  VMInitRec(&InList,InListBuffer,sizeof(InListBuffer),sizeof(scantype));
  VMAVLControlInit(&InList,&InListControl,comparenames,0);
  VMAVLTreeInit(&InListTree,1);

  VMInitRec(&OnDiskDupes,NULL,0,sizeof(scantype));
  VMInitRec(&InListDupes,NULL,0,sizeof(scantype));

  VMAccessAttrSet(&OnDisk,VM_RANDOM);
  VMAccessAttrSet(&InList,VM_RANDOM);
  VMAccessAttrSet(&OnDiskDupes,VM_SEQUENTIAL);
  VMAccessAttrSet(&InListDupes,VM_SEQUENTIAL);

  sprintf(Temp,"%19.19sPCBoard Duplicate/Missing File Analysis\r\n"
               "%19.19s---------------------------------------\r\n","","");
  dosfputs(Temp,&Analysis);  //lint !e534

  sprintf(Temp,"\r\nConferences Included in Scan: %s\r\n",List);
  dosfputs(Temp,&Analysis);  //lint !e534

  clscolor(Colors[DISPLAY]);
  generalscreen("Scan for Duplicate/Missing Files","");
  fastprintmove(19,20,"Working, please wait (ESC to abort) ",Colors[QUESTION]);
  startaction('.',TRUE,32);

  ListTotal = DiskTotal = DiskDupeTotal = ListDupeTotal = 0;

  ShowSwapMessage = TRUE;

  if (makepathlist(BitArray,DlName,DlList) == -1)
    goto abort;

  if (getdiskentries(DlName) == -1)
    goto abort;

  if (getdirentries(BitArray,DirName,DirList) == -1)
    goto abort;

  if (userabort())
    goto abort;

  DiskNonDupeTotal = DiskTotal - DiskDupeTotal;
  ListNonDupeTotal = ListTotal - ListDupeTotal;

  VMSizeLock(&OnDisk);
  VMSizeLock(&InList);

  resetaction(ActionChar,FALSE,128);
  if (filesindirnotdisk() == -1)
    goto abort;

  resetaction(ActionChar,FALSE,128);
  if (dupfilesondisk(DlName) == -1)
    goto abort;

  resetaction(ActionChar,FALSE,128);
  if (filesondisknotdir(DlName) == -1)
    goto abort;

  resetaction(ActionChar,FALSE,128);
  if (dupfilesindir() == -1)
    goto abort;

  resetaction(ActionChar,FALSE,128);
  if (similarfilesindir() == -1)
    goto abort;

abort:
  if (DiskTotal > ListTotal)
    ListTotal = DiskTotal;

  dosfclose(&Analysis);
  VMDone(&OnDisk);
  VMDone(&OnDiskDupes);
  VMDone(&InList);
  VMDone(&InListDupes);

end:
  free(BitArray);
  ShowSwapMessage = FALSE;
}
