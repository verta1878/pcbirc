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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmdata.hpp"
/* VMData swap types - legacy, not in current headers */
typedef int VMDataSwapFailureAction;
typedef int VMDataSwapFailure;
#define VM_SWAP_OPEN_FAILED 1
#define VM_SWAP_FULL 2
#define VM_CREATE_NEW 3
#define VM_ABORT 4
#define VMDataSwapFailureHandlerSet(x)
#include <dosfunc.h>
#include <screen.h>
#include <misc.h>
#include <vmdata.h>


#ifdef DEBUG
#include <memcheck.h>
#endif

extern unsigned _heaplen = 0x3000;

/****************************************************************************
 *  file spec:
 *
 *  NOTE:  There are two different file headers, an old one, which is limited
 *         to 64K entries in the index, and a new one, which is limited to
 *         4G and includes file sizes in order that PCBoard can avoid doing
 *         a find-first on the filename to verify its existence and get the
 *         size of the file.
 *
 *  header1:    2 bytes for an integer indicating the number of files
 *             52 bytes for 26 integers indicating starting points for A thru Z
 *             73 bytes reserved space
 *              1 byte header-id (equal to 0 for backward compatibility)
 *
 *  header2:    4 bytes for an integer indicating the number of files
 *            104 bytes for 26 longs indicating starting points for A thru Z
 *             19 bytes reserved space
 *              1 byte header-id (equal to 1 for new format)
 *
 *  names:     11 bytes for a filename of the form:  nnnnnnnneee
 *             any character not used is filled with a space
 *
 *  paths:     64 bytes for a NULL terminated/padded path
 *
 *  Following the header is the list of filenames which are then followed by
 *  paths.  The reason for this order is so that file buffering can be used
 *  to speed up access to the file and to reduce file seeks on startup.
 *
 *  The header consists of 26 separate integer values which indicate the
 *  record number (offset 0) of the first filename starting with each of the
 *  letters A thru Z.  This reduces the number of file seeks performed by
 *  immediately narrowing down the search records.  It also increases the
 *  likelyhood that all file seeks will be performed within the memory buffer
 *  that is read in from disk.
 *
 *      NOTE:  For the new format, it's 26 separate LONG INTEGER values.
 *
 *  Files starting with letters before 'A' will be placed immediately following
 *  the header so no offset value is recorded there.  Files that start with
 *  letters after 'Z' will follow those filenames starting with 'Z'.
 *
 *  A binary search is then used to locate the filename.  Note the format of
 *  the filenames which makes it easy to use a wildcard matching technique.
 *
 */

#define NAMESIZE   8
#define EXTSIZE    3
#define FILESIZE   NAMESIZE + EXTSIZE

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

typedef struct {
  char     Name[NAMESIZE];
  char     Ext[EXTSIZE];
  uint     PathNum;
} nametype1;

typedef struct {
  char     Name[NAMESIZE];
  char     Ext[EXTSIZE];
  ulong    PathNum;
  ulong    FileSize;
} nametype2;

typedef struct {
  char Path[64];
} pathtype;


long        PathNum;
long        NumFiles;
idxhdrtype1 Hdr1;
idxhdrtype2 Hdr2;
DOSFILE     Idx;
bool        Abort;
bool        Extended = FALSE;

VMDataSet   Files;
VMDataSet   Paths;

#ifdef TEST
DOSFILE     DebugFile;
#endif


static void swapdisplay(char *Message) {
  char         Str[80];
  savescrntype Screen;

  strcpy(Str,Message);
  stripall(Str,10);
  stripall(Str,13);
  savescreen(&Screen);
  boxcls(2,19,77,23,0x01,SINGLE);
  fastcenter(21,Str,0x0C);
  mydelay(100);
  restorescreen(&Screen);
}


static VMDataSwapFailureAction swaphandler(VMDataSwapFailure Failure, char *NewName) {
  // note: this filename is different from our default which we are
  // placing in the current directory as VMDATA.$$$
  static char *path = "C:\\VMSWAP.TMP";

  if (Failure == VM_SWAP_OPEN_FAILED || Failure == VM_SWAP_FULL) {
    if (*path <= 'Z') {
      strcpy(NewName,path);
      (*path)++; // get ready for next call
      return(VM_CREATE_NEW);
    }
  }

  // handle remaining swap file exceptions by aborting
  return(VM_ABORT);
}



void pascal getnames(char *Path) {
  int        Len;
  int        NameLen;
  long       StorePathNum;
  char      *p;
  char      *q;
  pathtype  *pathrec;
  nametype1 *filerec1;
  nametype2 *filerec2;
  struct     ffblk DTA;
  pathtype   Buf;
  #ifdef TEST
  char       Str[128];
  int        RetVal;
  #endif

  clsbox(0,1,sizeof(pathtype),1,0x07);
  fastprint(0,1,Path,0x07);

  StorePathNum = PathNum;

  pathrec = (pathtype *) VMRecordCreate(&Paths,sizeof(pathtype),NULL,NULL);
  memset(pathrec,0,sizeof(pathtype));
  strcpy(pathrec->Path,Path);

  p = &Path[strlen(Path)];
  strcpy(p,"*.*");

  #ifdef TEST
    sprintf(Str,"Path=%s\r\n",Path);
    dosfputs(Str,&DebugFile);
  #endif

  if (findfirst(Path,&DTA,FA_DIREC|FA_ARCH|FA_RDONLY|FA_HIDDEN|FA_SYSTEM) == 0) {
    do {
      strupr(DTA.ff_name);
      #ifdef TEST
        sprintf(Str,"File=%s %d\r\n",DTA.ff_name,DTA.ff_attrib);
        dosfputs(Str,&DebugFile);
      #endif
      if (DTA.ff_name[0] != '.') {
        if ((DTA.ff_attrib & FA_DIREC) != 0) {
          strcpy(p,DTA.ff_name);
          addchar(p,'\\');
          memset(&Buf,0,sizeof(pathtype));
          strcpy(Buf.Path,Path);
          PathNum++;
          getnames(Buf.Path);
          *p = 0;
        } else {
          NameLen = strlen(DTA.ff_name);
          q = strnchr(DTA.ff_name,'.',NameLen);

          if (Extended) {
            filerec2 = (nametype2 *) VMRecordCreate(&Files,sizeof(nametype2),NULL,NULL);
            memset(filerec2,' ',FILESIZE);
            filerec2->PathNum = StorePathNum;
            filerec2->FileSize = DTA.ff_fsize;
            if (q != NULL) {
              q++;
              Len = (int) (q - DTA.ff_name);
              memcpy(filerec2->Ext,q,NameLen - Len);
              NameLen = Len - 1;
            }
            memcpy(filerec2->Name,DTA.ff_name,NameLen);
          } else {
            if ((! Extended) && NumFiles == 0xFFFF) {
              clsbox(0,1,79,1,0x07);
              fastprintmove(0,0,"FOUND 65535 FILES - SPLIT INDEX UP",0x8F);
              return;
            }
            filerec1 = (nametype1 *) VMRecordCreate(&Files,sizeof(nametype1),NULL,NULL);
            memset(filerec1,' ',FILESIZE);
            filerec1->PathNum = (unsigned) StorePathNum;
            if (q != NULL) {
              q++;
              Len = (int) (q - DTA.ff_name);
              memcpy(filerec1->Ext,q,NameLen - Len);
              NameLen = Len - 1;
            }
            memcpy(filerec1->Name,DTA.ff_name,NameLen);
          }
          NumFiles++;
        }
      }
    #ifdef TEST
    } while ((RetVal = findnext(&DTA)) == 0);
      sprintf(Str,"done (%s) %d\r\n",Path,RetVal);
      dosfputs(Str,&DebugFile);
    #else
    } while (findnext(&DTA) == 0);
    #endif
  }
}


void copypaths(void) {
  pathtype *p;
  long      X;

  for (X = 1; X <= PathNum; X++) {
    p = (pathtype *) VMRecordGetByIndex(&Paths,X,NULL);
    if (dosfwrite(p,sizeof(pathtype),&Idx) == -1) {
      Abort = TRUE;
      return;
    }
  }
}


static VM_SHINT compare1(const void *Rec1, const void *Rec2) {
  nametype1 *A = (nametype1 *) Rec1;
  nametype1 *B = (nametype1 *) Rec2;

  return(memcmp(A,B,FILESIZE));
}


static VM_SHINT compare2(const void *Rec1, const void *Rec2) {
  nametype2 *A = (nametype2 *) Rec1;
  nametype2 *B = (nametype2 *) Rec2;

  return(memcmp(A,B,FILESIZE));
}


void sortnames(void) {
  nametype1 *p1;
  nametype2 *p2;
  long       X;
  int        Letter;
  int        LastLetter;
  char       Str[80];
  char       SortBuf[2048];

  sprintf(Str,"Number of files: %ld",NumFiles);
  fastprint(0,5,Str,0x0F);

  if (NumFiles > 0) {
    if (Extended)
      VMSort(&Files,sizeof(nametype2),1,NumFiles,VM_FALSE,compare2,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));
    else
      VMSort(&Files,sizeof(nametype1),1,NumFiles,VM_FALSE,compare1,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));

    dosrewind(&Idx);
    dosfseek(&Idx,sizeof(idxhdrtype1),SEEK_SET);

    for (X = 1, Letter = -1, LastLetter = 0; X <= NumFiles; X++) {
      if (Extended) {
        p2 = (nametype2 *) VMRecordGetByIndex(&Files,X,NULL);
        if (dosfwrite(p2,sizeof(nametype2),&Idx) == -1) {
          Abort = TRUE;
          break;
        }
        Letter = p2->Name[0];
        if (Letter != LastLetter && Letter >= 'A' && Letter <= 'Z') {
          LastLetter = Letter;
          Hdr2.RecOffset[Letter - 'A'] = X-1;
        }
      } else {
        p1 = (nametype1 *) VMRecordGetByIndex(&Files,X,NULL);
        if (dosfwrite(p1,sizeof(nametype1),&Idx) == -1) {
          Abort = TRUE;
          break;
        }
        Letter = p1->Name[0];
        if (Letter != LastLetter && Letter >= 'A' && Letter <= 'Z') {
          LastLetter = Letter;
          Hdr1.RecOffset[Letter - 'A'] = (unsigned short) X-1;
        }
      }
    }
  }
}


void writeheader(void) {
  int      X;
  unsigned Previous1;
  long     Previous2;

  dosrewind(&Idx);

  if (Extended) {
    Hdr2.NumFiles = NumFiles;
    for (X = 0, Previous2 = 0; X < 26; X++) {
      if (Hdr2.RecOffset[X] == 0xFFFFFFFFL)
        Hdr2.RecOffset[X] = Previous2;
      Previous2 = Hdr2.RecOffset[X];
    }
    if (dosfwrite(&Hdr2,sizeof(Hdr2),&Idx) == -1)
      Abort = TRUE;
  } else {
    Hdr1.NumFiles = (unsigned) NumFiles;
    for (X = 0, Previous1 = 0; X < 26; X++) {
      if (Hdr1.RecOffset[X] == 0xFFFF)
        Hdr1.RecOffset[X] = Previous1;
      Previous1 = Hdr1.RecOffset[X];
    }
    if (dosfwrite(&Hdr1,sizeof(Hdr1),&Idx) == -1)
      Abort = TRUE;
  }
}


void pascal processpaths(char *Path) {
  pathtype  Buf;
  char     *p;
  int       Len;

  strupr(Path);
/*memset(&Buf,0,sizeof(pathtype)); */
/*strcpy(Buf.Path,Path);           */

  Len = strlen(Path);
  memcpy(Buf.Path,Path,Len+1);

  /* if Len is 1 or greater, and if the last character in the string is */
  /* not a backslash (i.e. C:\PCB\) and is not a colon (i.e. C:), then it */
  /* is a path that needs to have a backslash added (i.e. C:\PCB becomes */
  /* C:\PCB\ after adding the backslash */

  if (Len > 0) {
    p = &Buf.Path[Len-1];
    if (*p != '\\' && *p != ':') {
      *(p+1) = '\\';
      *(p+2) = 0;
    }
  }
  getnames(Buf.Path);
}


void pascal maketempname(char *Temp, char *Name) {
  char *p;

  strcpy(Temp,Name);
  p = strchr(Temp,'.') + 1;
  strcpy(p,"!!!");
}


void main(int argc, char **argv) {
  char    Str[128];
  char    TmpName[66];
  char    IdxName[66];
  char    PthName[66];
  char    Path[66];
  DOSFILE PathList;

  #ifdef DEBUG
    mc_startcheck(erf_standard);
  #endif

  Abort = FALSE;

  if (argc < 3 || argc > 4) {
     puts("Type 'MAKEIDX pathlist[.PTH] indexname[.IDX] [/E]'\n"
         "\n"
         "Where pathlist is an ASCII file (CR/LF delimited) listing paths that\n"
         "are to be scanned for files (including any subdirectories of the paths\n"
         "that are specified.\n"
         "\n"
         "And the indexname is the name of the index file to be created based\n"
         "on the files found in the paths specified by the pathlist file.\n"
         "\n"
         "NOTE:  The /E is optional and specifies an extended format IDX file\n"
         "which stores file sizes inside the file, and is also capable of holding\n"
         "more than the 64K limit for IDX files without the /E option.\n");
    return;
  }

  /*
  initscrnio();
  ShowClock       = FALSE;
  UpdateKbdStatus = FALSE;
  */
  getmode();

  VMDataStartUp("VMDATA.$$$",32,100,VM_FALSE);
  VMDataSwapDisplayFuncSet(swapdisplay);
  VMDataSwapFailureHandlerSet(swaphandler);
  #ifdef VM_DEBUG
    VMDebugOn(VM_SANITY_CHECK|VM_MURPHY);
  #endif

  cls();
  strcpy(PthName,argv[1]);
  if (strchr(PthName,'.') == NULL)
    strcat(PthName,".PTH");

  strcpy(IdxName,argv[2]);
  if (strchr(IdxName,'.') == NULL)
    strcat(IdxName,".IDX");

  if (argc == 4 && stricmp(argv[3],"/E") == 0)
    Extended = TRUE;

  VMInitRec(&Files,NULL,0,(Extended ? sizeof(nametype2) : sizeof(nametype1)));
  VMInitRec(&Paths,NULL,0,sizeof(pathtype));

  maketempname(TmpName,IdxName);

  sprintf(Str,"Creating %sindex file %s...",(Extended ? "extended format " : ""),IdxName);
  fastprintmove(0,0,Str,0x0F);

  if (dosfopen(TmpName,OPEN_RDWR|OPEN_CREATE,&Idx) == -1) {
    puts("\nunable to create index file");
    return;
  }

  if (dosfopen(PthName,OPEN_READ|OPEN_DENYNONE,&PathList) == -1) {
    puts("\nunable to open pathlist file");
    return;
  }

  if (Extended) {
    memset(&Hdr2,0,sizeof(Hdr2));
    memset(Hdr2.RecOffset,0xFF,sizeof(Hdr2.RecOffset));
    Hdr2.Id = 1;
    if (dosfwrite(&Hdr1,sizeof(Hdr1),&Idx) == -1) {
      Abort = TRUE;
      goto end;
    }
  } else {
    memset(&Hdr1,0,sizeof(Hdr1));
    memset(Hdr1.RecOffset,0xFF,sizeof(Hdr1.RecOffset));
    Hdr2.Id = 0;
    if (dosfwrite(&Hdr1,sizeof(Hdr1),&Idx) == -1) {
      Abort = TRUE;
      goto end;
    }
  }

  dossetbuf(&Idx,8192);
  NumFiles = 0;
  PathNum = 0;

  #ifdef TEST
    dosfopen("MAKEIDX.DBG",OPEN_RDWR|OPEN_CREATE,&DebugFile);
  #endif

  while (dosfgets(Path,sizeof(Path),&PathList) != -1) {
    stripright(Path,' ');
    if (Path[0] != 0) {
      processpaths(Path);
      PathNum++;
      if (Abort)
        goto end;
    }
  }

  #ifdef TEST
    dosfclose(&DebugFile);
  #endif

  clsbox(0,1,79,1,0x07);
  dosfclose(&PathList);

  fastprintmove(0,1,"Sorting filenames...",0x07);
  sortnames();
  fastprintmove(22,1,"Copying paths...",0x07);
  copypaths();
  writeheader();

  dosfclose(&Idx);

  if (fileexist(IdxName) != 255)
    dosunlinkcheck(IdxName);

end:
  VMDone(&Files);
  VMDone(&Paths);

  if (Abort) {
    unlink(TmpName);
    fastprint(40,1,"Aborted!",0x0F);
  } else {
    rename(TmpName,IdxName);
    fastprint(40,1,"Done!",0x07);
  }
  gotoxy(0,6);

#ifdef DEBUG
  mc_endcheck();
#endif
}
