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

#include <io.h>
#include <ctype.h>

#if defined(__BORLANDC__) || defined(__TURBOC__)
//  #include <dir.h>
#else
  #include <direct.h>
  #include <borland.h>
#endif

/* DTA is declared in exist.c from the MISC library */
extern struct ffblk DTA;

#define MAXDESCLEN  48

int NumFiles;       /* variable also used in DLPATH.C and TRANSFER.C */
int CurBatchLimit;  /* variable also used in TRANSFER.C */

static DOSFILE  FileList;
static DOSFILE  FileDesc;
static int      DescSize;
static char *   DescBuffer;
static bool     MovedIntoWork;  // have files been copied into Work dir?


/********************************************************************
*
*  Function:  getfilespec()
*
*  Desc    :  retrieves from disk the numbered file specification
*
*  Returns :  -1 on error, otherwise the number of bytes read in
*/

int LIBENTRY getfilespec(int Num, spectype *File) {
  if (FileList.handle != 0) {
    dosfseek(&FileList,(long) Num * sizeof(spectype),SEEK_SET);
    return(dosfread(File,sizeof(spectype),&FileList));
  }
  return(-1);
}


/********************************************************************
*
*  Function:  putfilespec()
*
*  Desc    :  puts a numbered file specification out to disk
*
*  Returns :  -1 on error, otherwise 0
*
*  NOTE    :  On slave card systems it may be necessary to perform a "flush"
*             after the write to disk in order for getfilespec() to see the
*             newly written data.
*/

int LIBENTRY putfilespec(int Num, spectype *File) {
  if (FileList.handle != 0) {
    dosfseek(&FileList,(long) Num * sizeof(spectype),SEEK_SET);
    return(dosfwrite(File,sizeof(spectype),&FileList));
  }
  return(-1);
}


static void _NEAR_ LIBENTRY deletefile(char *Name) {
  // I'm not so sure we'll be copying file attributes under Watcom so
  // we may not need to worry about un-setting them either
  #ifdef __BORLANDC__
    _chmod(Name,1,0);  //lint !e534 reset to normal in case copied file was READ ONLY
  #endif
  unlink(Name);      // now delete the file
}


/********************************************************************
*
*  Function:  removefilesfromlist()
*
*  Desc    :  This function removes filespecs from the FileList if the files
*             are not cleared by the FSEC file (checked by seeing that they
*             are neither FREE nor cleared with the FSECOKAY setting).
*/

void LIBENTRY removefilesfromlist(void) {
  int      X;
  int      Y;
  int      Total;
  spectype File;

  if (NumFiles < 1 || FileList.handle == 0)
    return;

  for (X = Y = 0, Total = NumFiles; X < Total; X++) {
    if (getfilespec(X,&File) == -1)
      break;

    if (! File.FsecOkay) {
      NumFiles--;
      if (File.MovedToWork)
        deletefile(File.FullPath);
    } else
      if (putfilespec(Y++,&File) == -1)
        break;
  }

  /* WARNING:  be sure that the following method of truncating the file will */
  /*           function properly with slave cards before releasing           */

  dosflush(&FileList);
  dosftrunc(&FileList,(long) NumFiles * sizeof(spectype));  //lint !e534
/*
  doslseek(FileList.handle,(long) NumFiles * sizeof(spectype),SEEK_SET);
  doswrite(FileList.handle,NULL,0 POS2ERROR);    //lint !e534
  dosflush(&FileList);
*/
}


char * LIBENTRY slowdrive(char *SlowDrives, char DriveLetter) {
  char *p;

  for (p = SlowDrives; *p > 0; p++) {
    if (*p > ' ') {
      if (*p == DriveLetter)
        return(p);

      if (*p == '-' && p != SlowDrives) {
        if (*(p-1) < DriveLetter && *(p+1) >= DriveLetter)
          return(p-1);
      }
    }
  }
  return(NULL);
}


/********************************************************************
*
*  Function:  movefilesinlist()
*
*  Desc    :  This function moves files that are in the list, which are found
*             to be stored on slower-access drives (CD-ROM), down to a local
*             drive.  This is done before a file transfer begins so that the
*             files can be pulled off of a cd-rom-changer for ready access
*             when the transfer begins.
*/

void LIBENTRY movefilesinlist(void) {
  bool           FirstTime;
  bool           Skipped;
  int            DriveLetter;
  int            CurDrive;
  int            X;
  int            CopyStatus;
  char          *p;
  unsigned long  FreeSpace;
  bool           DriveList[26];
  char           LockDrives[27];
  char           NewPath[66];
  spectype       File;

  if (NumFiles < 1 || FileList.handle == 0)
    return;

  if ((FreeSpace = diskfreespace(PcbData.TmpLoc)) < (100 * 1024L))
    return;

  CurDrive = dosgetcurdrive() - 1;  // subtract 1 so that 0 = A, 1 = B, 2 = C, etc.

  // scan the filespecs to see which drive letters are in use so that we can
  // sort the access the file list in sorted-drive-letter order in order to
  // reduce swapping on a CD-ROM changer

  memset(DriveList,0,sizeof(DriveList));
  for (X = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;

    if (File.FullPath[1] != ':')
      DriveList[CurDrive] = TRUE;  // if no ":" then use current drive
    else if (slowdrive(PcbData.SlowDrives,File.FullPath[0]) != NULL || File.OldName[0] != 0)
      DriveList[toupper(File.FullPath[0]) - 'A'] = TRUE;
  }

  FirstTime = TRUE;

  // loop through each drive letter to see if there are any files to be
  // moved from that drive

top:
  for (DriveLetter = 'A' - 'A', Skipped = FALSE; DriveLetter <= 'Z' - 'A'; DriveLetter++) {
    // if there are no files for this drive letter then loop back around
    if (! DriveList[DriveLetter])
      continue;

    // scan the file list for any files matching the current drive letter
    for (X = 0; X < NumFiles; X++) {
      if (getfilespec(X,&File) == -1)
        break;

      // if it doesn't match the current drive letter then skip it
      if (File.FullPath[1] == ':' && File.FullPath[0] != DriveLetter + 'A')
        continue;

      if ((File.FullPath[1] == ':' && (p = slowdrive(PcbData.SlowDrives,File.FullPath[0])) != NULL) || File.OldName[0] != 0) {

        // Don't let the file run the work drive out of disk space, make sure
        // there is at least as many bytes free, plus 100K, to hold the
        // file that is about to be copied.

        if (fileexist(File.FullPath) != 255)
          if ((unsigned long) DTA.ff_fsize > FreeSpace - (100 * 1024L))
            continue;

        if (FirstTime) {
          displaypcbtext(TXT_ANNOUNCECOPY,DEFAULTS);
          showactivity(ACTBEGIN);
          FirstTime = FALSE;
        }
        showactivity(ACTSHOW);

        if (File.OldName[0] != 0) {
          buildstr(NewPath,PcbData.TmpLoc,File.Name,NULL);
          memset(File.OldName,0,sizeof(File.OldName));
        } else
          buildstr(NewPath,PcbData.TmpLoc,File.Name,NULL);

        if (stricmp(File.FullPath,NewPath) == 0) {  // paths are identical
          if (PcbData.SlowDriveBat[0] != 0) {       // run slowdrive bat if it
            runslowdrivebat(File.FullPath);         // exist, but don't bother
            if (fileexist(File.FullPath) != 255) {  // copying the file
              File.Size = DTA.ff_fsize;
              putfilespec(X,&File);
            }
          }
          continue;
        }

        if (p == NULL)
          LockDrives[0] = 0;
        else {
          LockDrives[0] = *p;
          if (*(++p) == '-') {
            LockDrives[1] = '-';
            LockDrives[2] = *(++p);
            LockDrives[3] = 0;
          } else
            LockDrives[1] = 0;
        }

        CopyStatus = pcbcopyfile(File.FullPath,NewPath,FALSE,TRUE,LockDrives,FALSE);

        // If the drive was in use, record that a file has been skipped and
        // try it again later, otherwise, if the copy was okay, then mark the
        // file as having been successfully copied.
        if (CopyStatus == COPYFILE_DRIVEINUSE)
          Skipped = TRUE;
        else if (CopyStatus == 0) {
          strcpy(File.FullPath,NewPath);
          File.MovedToWork = TRUE;
          MovedIntoWork = TRUE;
          if (PcbData.SlowDriveBat[0] != 0) {
            runslowdrivebat(File.FullPath);
            if (fileexist(File.FullPath) != 255)
              File.Size = DTA.ff_fsize;
          }
          putfilespec(X,&File);
        }
      }
    }
  }

  // if any files were skipped because a locked drive was in use, then go
  // back and try to copy the file again.
  if (Skipped)
    goto top;

  showactivity(ACTEND);
  if (! FirstTime)
    freshline();
}


/********************************************************************
*
*  Function:  removefinishedfilesfromlist()
*
*  Desc    :  This function removes filespecs from the FileList if the files
*             are have already been transferred (checked by seeing that they
*             are Success variable is not equal to FAILED).
*/

void LIBENTRY removefinishedfilesfromlist(void) {
  int      X;
  int      Y;
  int      Total;
  spectype File;

  if (NumFiles < 1)
    return;

  for (X = Y = 0, Total = NumFiles; X < Total; X++) {
    if (getfilespec(X,&File) == -1)
      break;

    if (File.Success != FAILED) {
      if (File.MovedToWork)
        deletefile(File.FullPath);
      NumFiles--;
    } else
      if (putfilespec(Y++,&File) == -1)
        break;
  }

  /* WARNING:  be sure that the following method of truncating the file will */
  /*           function properly with slave cards before releasing           */

  dosflush(&FileList);
  dosftrunc(&FileList,(long) NumFiles * sizeof(spectype));   //lint !e534

/*
  doslseek(FileList.handle,(long) NumFiles * sizeof(spectype),SEEK_SET);
  doswrite(FileList.handle,NULL,0);    //lint !e534
  dosflush(&FileList);
*/

  if (MovedIntoWork && NumFiles == 0)
    MovedIntoWork = FALSE;
}


/********************************************************************
*
*  Function:  filelistname()
*
*  Desc    :  Builds the name of the file that will hold a list of filenames
*             to be used for file transfers.  The name is built upon the work
*             location, the name "FLIST" and an extension which is built from
*             the node number.
*
*  Returns :  The FileName filled with the name created by the function.
*/

static void _NEAR_ LIBENTRY filelistname(char *FileName) {
  strcpy(FileName,"FLIST.");
  if (PcbData.Network)
    ascii(&FileName[strlen(FileName)],PcbData.NodeNum);
}



/********************************************************************
*
*  Function:  allocatefilelist()
*
*  Desc    :  This function used to allocate memory.  Now it creates a file
*             instead so that filenames won't be lost when dropping to DOS.
*
*             CurBatchLimit is set based on whether uploads or downloads
*             will be performed.
*
*  Returns  : -1 indicates an error
*              0 indicates no action was required (list already open)
*              1 indicates a newly created list
*              2 indicates a re-opened list
*/

int LIBENTRY allocatefilelist(xfertype Mode) {
  char FileName[66];

  filelistname(FileName);

  if (Mode == UPLOAD) {
    if (PcbData.NoBatchUp || Status.FileAttach)
      CurBatchLimit = 1;
    else
      CurBatchLimit = 32000;       // get rid of batch limit on uploads
    deallocatefilelist(THROWAWAY);
    MovedIntoWork = FALSE;
  } else
    CurBatchLimit = (Status.CurSecLevel >= PcbData.UserLevels[SEC_BATCH] ? Status.BatchLimit : 1);

  if (FileList.handle == 0) {
    NumFiles = 0;
    if (fileexist(FileName) == 255) {
      if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE|OPEN_CREATE,&FileList) == -1)
        return(-1);
      MovedIntoWork = FALSE;
      return(1);
    } else {
      if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE,&FileList) == -1)
        return(-1);
      if (Mode == DOWNLOAD) {
        NumFiles = (int) (dosfseek(&FileList,0,SEEK_END) / sizeof(spectype));
        removefinishedfilesfromlist();
        dosrewind(&FileList);
        return(2);
      }
    }
  }

  return(0);
}


void LIBENTRY reloadfilelist(void) {
  int  OpenStat;
  char FileName[66];

  filelistname(FileName);
  if (fileexist(FileName) == 255)
    return;

  if ((OpenStat = allocatefilelist(DOWNLOAD)) == -1)
    return;                /* serious error - cannot create list */

  if (OpenStat == 2)       /* if allocatefilelist() found files then we need */
    checkdlfiles(0);       /* to scan them to set up the time/byte counters  */

  if (NumFiles == 0)
    deallocatefilelist(THROWAWAY);
}


/********************************************************************
*
*  Function:  deallocatefilelist()
*
*  Desc    :  This function used to deallocate memory.  Now it erases the
*             file that holds the file list and sets NumFiles to zero.
*
*             Prior to removing the file it checks to see if any files have
*             been moved into the work directory.  If they have, it will
*             delete them from the work directory.
*/

void LIBENTRY deallocatefilelist(savekeep Action) {
  int      X;
  char     FileName[66];
  spectype File;

  // if we have files in the work directory, but the file list is
  // currently closed, we're going to need to re-open it in order
  // to remove all of the files from the work directory

  if (FileList.handle == 0 && MovedIntoWork && Action == THROWAWAY)
    allocatefilelist(DOWNLOAD);   //lint !e534

  if (FileList.handle != 0) {
    if (MovedIntoWork && Action == THROWAWAY) {
      for (X = 0; X < NumFiles; X++) {
        if (getfilespec(X,&File) == -1)
          break;

        if (File.MovedToWork)
          deletefile(File.FullPath);
      }
      MovedIntoWork = FALSE;
    }
    dosfclose(&FileList);
  }

  if (Action == THROWAWAY) {
    filelistname(FileName);
    if (fileexist(FileName) != 255)
      unlink(FileName);
    NumFiles = 0;
  }
}


/********************************************************************
*
*  Function:  findfilename()
*
*  Desc    :  scans the list of file specs on disk searching for the name
*             passed (no path, just filename)
*
*  Returns :  the index (into the file list) number of the file found or -1 if
*             the filename was not found
*/

int LIBENTRY findfilename(char *Name) {
  int      X;
  spectype File;

  for (X = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (strcmp(Name,File.Name) == 0)
      return(X);
  }
  return(-1);
}


/********************************************************************
*
*  Function:  addfiletolist()
*
*  Desc    :  Adds a file spec to the file list on disk if the file spec is not
*             a duplicate of another already in the list and if the batch limit
*             has not been reached.
*
*             If ForceIntoList then it means the file has ALREADY been
*             received so force it into the list regardless of the batch
*             limit so that we don't lose the received file.
*
*  Returns :  -3 to indicate a serious error (can't create list or add spec)
*             -2 to indicate the file is a duplicate but not so serious
*             otherwise the "index" to the file added is returned
*/

int LIBENTRY addfiletolist(char *Name, char *Path, long Size, bool ForceIntoList, bool ShowDupe) {
  spectype File;

  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  if (NumFiles < CurBatchLimit || ForceIntoList) {
    if (findfilename(Name) == -1) {
      memset(&File,0,sizeof(spectype));
      maxstrcpy(File.Name,Name,sizeof(File.Name));
      buildstr(File.FullPath,Path,Name,NULL);
      File.Size = Size;
      if (putfilespec(NumFiles,&File) == -1)
        return(-3);  /* serious error - cannot add any more to the list */

      NumFiles++;
      if (NumFiles > 1)
        Status.Batch = TRUE;
      return(NumFiles-1);
    }

    if (ShowDupe)
      displaypcbtext(TXT_DUPLICATEBATCHFILE,NEWLINE);

    return(-2);  /* not a serious error */
  }

  displaypcbtext(TXT_BATCHLIMITREACHED,NEWLINE);
  return(-3);  /* serious error - limit has been reach, do not try any more */
}


/********************************************************************
*
*  Function:  averagecps()
*
*  Desc    :  This function scans the file list for all completed file
*             transfers of the type specified and averages the CPS rate.
*
*  Returns :  An integer specifying the averages CPS for the entire group.
*/

// total number of files flagged so far
unsigned LIBENTRY numfilesflagged(void) {
  bool     HadToOpen;
  int      X;
  unsigned TotalFiles;
  spectype File;

  if (NumFiles == 0)
    return(0);

  if (FileList.handle == 0) {
    if (allocatefilelist(DOWNLOAD) == -1)
      return(0);
    HadToOpen = TRUE;
  } else
    HadToOpen = FALSE;

  for (X = 0, TotalFiles = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    TotalFiles++;
  }

  if (HadToOpen)
    deallocatefilelist(KEEP);

  return(TotalFiles);
}


// total number of bytes flagged so far
long LIBENTRY numbytesflagged(void) {
  bool     HadToOpen;
  int      X;
  long     TotalBytes;
  spectype File;

  if (NumFiles == 0)
    return(0);

  if (FileList.handle == 0) {
    if (allocatefilelist(DOWNLOAD) == -1)
      return(0);
    HadToOpen = TRUE;
  } else
    HadToOpen = FALSE;

  for (X = 0, TotalBytes = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    TotalBytes += File.Size;
  }

  if (HadToOpen)
    deallocatefilelist(KEEP);

  return(TotalBytes);
}



static void _NEAR_ LIBENTRY filedescname(char *FileName) {
  strcpy(FileName,"FDESC.");
  if (PcbData.Network)
    ascii(&FileName[strlen(FileName)],PcbData.NodeNum);
}


char * LIBENTRY allocatefiledesc(void) {
  char FileName[66];

  filedescname(FileName);

  if (FileDesc.handle == 0) {
    if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE|OPEN_CREATE,&FileDesc) == -1)
      return(NULL);
  }

  if (DescBuffer == NULL) {
    DescSize = PcbData.NumDescLines * MAXDESCLEN;
    if ((DescBuffer = (char *) checkmalloc(DescSize,"DESC BUFFER")) == NULL) {
      deallocatefiledesc();
      return(NULL);
    }
  }

  memset(DescBuffer,0,DescSize);
  return(DescBuffer);
}


void LIBENTRY deallocatefiledesc(void) {
  char FileName[66];

  if (FileDesc.handle != 0)
    dosfclose(&FileDesc);

  if (DescBuffer != NULL)
    bfree(DescBuffer);
  DescBuffer = NULL;

  filedescname(FileName);
  if (fileexist(FileName) != 255)
    unlink(FileName);
}


int LIBENTRY getfiledesc(int Num, char *Desc) {
  if (FileDesc.handle != 0) {
    dosfseek(&FileDesc,(long) Num * DescSize,SEEK_SET);
    return(dosfread(Desc,DescSize,&FileDesc));
  }
  return(-1);
}


int LIBENTRY putfiledesc(int Num, char *Desc) {
  if (FileDesc.handle != 0) {
    dosfseek(&FileDesc,(long) Num * DescSize,SEEK_SET);
    return(dosfwrite(Desc,DescSize,&FileDesc));
  }
  return(-1);
}
