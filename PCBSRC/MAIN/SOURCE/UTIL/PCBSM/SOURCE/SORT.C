#include "pcbsm_externs.h"
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
#include <stdio.h>
#include <dos.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <newdata.h>
#include <misc.h>
#include <pcb.h>
#include <dosfunc.h>
#include <help.h>
#define VMDATA_NEED_RECORDS
#define VMDATA_NEED_SEQFINALPASS
#include <vmdata.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#include <stdio.h>
#ifdef __WATCOMC__
extern hdrtype Header;
extern apptype AppHeader;
extern addresstypez Address;
extern addresstype OldAddress;
#endif
#endif

static URead    *Buffer;
static URead    *BufPtr;
static unsigned NumBuffers;

static VMDataSet Array;


/*****************************************************************************

the defines listed below are used to determine the size of the arrays that are
used for sorting the users file

******************************************************************************/

#define sv01size sizeof(UsersRead.Name)
#define sv02size sizeof(UsersRead.Password)
#define sv03size sizeof(UsersRead.BusDataPhone)
#define sv04size sizeof(UsersRead.HomeVoicePhone)
#define sv05size sizeof(UsersRead.RegExpDate)
#define sv06size sizeof(UsersRead.UserComment)
#define sv07size sizeof(UsersRead.SysopComment)
#define sv08size sizeof(UsersRead.City)
#define sv09size sizeof(int)  + sizeof(UsersRead.Name)
#define sv10size sizeof(long) + sizeof(UsersRead.Name)


/*****************************************************************************

the following ENUM declaration creates values used in several switch()'s

*****************************************************************************/

enum {SECURITY,TIMESON,DOWNLOAD,UPLOAD,RATIO,BYTEDNLD,BYTEUPLD,BYTERATIO};

/*****************************************************************************

The following are global variables for the sort procedure

*****************************************************************************/

static long Recs;
static int  svsize;
static int  SortSize;
static bool SortReverse;
static bool SortReverse2;
static bool Aborted;
static int  Additional;

static void *_Cdecl (*sub)(void *dest, const void *src, size_t maxlen);


/*****************************************************************************

The following structures allow easier access to the fields in the arrays that
are defined for the sort procedures.

******************************************************************************/

typedef struct {
  long DiskOffset;
  char SortValue[80];  /* pick the largest possible value */
} s00;

typedef struct {
  long DiskOffset;
  int  Num;
  char SortValue[80];  /* pick the largest possible value */
} s01;

typedef struct {
  long DiskOffset;
  long Num;
  char SortValue[80];  /* pick the largest possible value */
} s02;


/********************************************************************
*
*  Function: lasttofirst()
*
*  Desc    : Copies the users Last Name + First Name to the string Dest[]
*/

static void *_Cdecl lasttofirst(void *Dest, const void *Srce, size_t Num) {
  int  Len;
  int  Len2;
  char Str[30];
  char *ptr;

  memcpy(Str,Srce,Num);
  Str[Num] = 0;
  stripright(Str,' ');
  if ((ptr = strrchr(Str,' ')) != NULL) {
    ptr++;
    Len = strlen(ptr);
    memcpy(Dest,ptr,Len);
    ((char *) Dest)[Len] = ',';
    Len2 = (int) (ptr-Str-1);
    memcpy(&((char*) Dest)[Len+1],Str,Len2);
    ((char *) Dest)[Len+Len2+1] = 0;
    return(Dest);
  } else {
    memcpy(Dest,Srce,Num);
    return(Dest);
  }
}


/********************************************************************
*
*  Function: udratio()
*
*  Desc    : Determines the Upload/Download ratio.  If a number is equal to 0
*            it is incremented to 1.
*
*  Returns : If Ups > Dns then the number returned is positive ratio, if
*            Dns > Ups then it is negative.
*/

/*
int pascal udratio(unsigned NumUps, unsigned NumDns) {
  if (NumUps > NumDns) {
    if (NumDns == 0)
      NumDns++;
    return(NumUps * 10 / NumDns);
  } else {
    if (NumDns > NumUps) {
      if (NumUps == 0)
        NumUps++;
      return(-(int)(NumDns * 10 / NumUps));
    } else {
      return(0);
    }
  }
}
*/

long pascal ludratio(long NumUps, long NumDns) {
  dlong Temp;

  if (NumUps > NumDns) {
    if (NumDns == 0)
      NumDns++;

    dmul(NumUps,10,&Temp);          // (NumUps*10) / NumDns
    return(ddiv(&Temp,NumDns));
  } else {
    if (NumDns > NumUps) {
      if (NumUps == 0)
        NumUps++;

      dmul(NumDns,10,&Temp);         // -(NumDns*10) / NumUps
      return(-(ddiv(&Temp,NumUps)));
    }
  }
  return(0);
}

long pascal ddratio(double NumUps, double NumDns) {
  if (NumUps > NumDns) {
    if (NumDns == 0)
      NumDns = 1;
    return((long) ((NumUps*10)/NumDns));
  } else {
    if (NumDns > NumUps) {
      if (NumUps == 0)
        NumUps = 1;
      return((long) -((NumDns*10)/NumUps));
    }
  }
  return(0);
}

/********************************************************************
*
*  Function: comp01()
*
*  Desc    : Sort only on one field (in string format).
*
*  Returns : if Rec1<Rec2 then <0, if Rec1>Rec2 then >0, if Rec1=Rec2 then 0
*/

static VM_SHINT comp01(const void *Rec1, const void *Rec2) {
  s00 *A = (s00 *) Rec1;
  s00 *B = (s00 *) Rec2;
  int CompRet;

  CompRet = memcmp(A->SortValue,B->SortValue,svsize);
  if (SortReverse)
    CompRet = -CompRet;
  return(CompRet);
}



/********************************************************************
*
*  Function: comp02()
*
*  Desc    : Sort on an integer then by name
*
*  Returns : if Rec1<Rec2 then <0, if Rec1>Rec2 then >0, if Rec1=Rec2 then 0
*/

static VM_SHINT comp02(const void *Rec1, const void *Rec2) {
  s01 *A = (s01 *) Rec1;
  s01 *B = (s01 *) Rec2;
  int CompRet;

  CompRet = 0;
  if (A->Num > B->Num)
    CompRet = 1;
  else if (A->Num < B->Num)
    CompRet = -1;

  if (CompRet) {
    if (SortReverse)
      return(-CompRet);
    else
      return(CompRet);
  }

  CompRet = memcmp(A->SortValue,B->SortValue,svsize);
  if (SortReverse2)
    return(-CompRet);
  else
    return(CompRet);
}



/********************************************************************
*
*  Function: comp03()
*
*  Desc    : Sort on a long then by name
*
*  Returns : if Rec1<Rec2 then <0, if Rec1>Rec2 then >0, if Rec1=Rec2 then 0
*/

static VM_SHINT comp03(const void *Rec1, const void *Rec2) {
  s02 *A = (s02 *) Rec1;
  s02 *B = (s02 *) Rec2;
  int CompRet;

  CompRet = 0;
  if (A->Num > B->Num)
    CompRet = 1;
  else if (A->Num < B->Num)
    CompRet = -1;

  if (CompRet) {
    if (SortReverse)
      return(-CompRet);
    else
      return(CompRet);
  }

  CompRet = memcmp(A->SortValue,B->SortValue,svsize);
  if (SortReverse2)
    return(-CompRet);
  else
    return(CompRet);
}



/********************************************************************
*
*  Function: sort01()
*
*  Desc    : Sort users file by Name
*
*/

static void near pascal sort01(int Offset) {
  unsigned  BuffSize;
  URead    *Ending;
  s00      *p;
  long      Counter;
  long      DiskOffset;
  char      SortBuf[16834];

  for (Counter = 1, DiskOffset = sizeof(URead); Counter <= Recs; Counter++, DiskOffset += sizeof(URead)) {
    if (readdosusersrecord() == -1) {
      Aborted = TRUE;
      return;
    }
    p = (s00 *) VMRecordCreate(&Array,SortSize,NULL,NULL);
    sub(p->SortValue,UsersRead.Name + Offset,svsize);
    p->DiskOffset = DiskOffset;
  }

  VMSizeLock(&Array);

  fastprintmove(36,12,"   Sorting...",Colors[DISPLAY]);
  VMSort(&Array,SortSize,1,Recs,VM_FALSE,comp01,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));
  fastprintmove(47,12,"   Writing...",Colors[DISPLAY]);

  VMSeqFinalPass(&Array);

  if (Buffer != NULL) {
    BuffSize = NumBuffers * sizeof(URead);
    Ending = &Buffer[NumBuffers];

    for (Counter = 1; Counter <= Recs; ) {
      if (Recs-Counter < NumBuffers) {
        NumBuffers = (unsigned) (Recs - Counter + 1);
        BuffSize = NumBuffers * sizeof(URead);
        Ending = &Buffer[NumBuffers];
      }

      for (BufPtr = Buffer;  BufPtr < Ending; BufPtr++, Counter++) {
        p = (s00 *) VMRecordGetByIndex(&Array,Counter,NULL);
        doslseek(DosUsersFile.handle,p->DiskOffset,SEEK_SET);
        if (readcheck(DosUsersFile.handle,BufPtr,sizeof(URead)) == (unsigned) -1) {
          Aborted = TRUE;
          return;
        }
      }

      if (writecheck(TempFile,Buffer,BuffSize) == (unsigned) -1 || userabort()) {
        Aborted = TRUE;
        return;
      }
    }
  } else {
    for (Counter = 1;  Counter <= Recs;  Counter++) {
      p = (s00 *) VMRecordGetByIndex(&Array,Counter,NULL);
      doslseek(DosUsersFile.handle,p->DiskOffset,SEEK_SET);
      if (readcheck(DosUsersFile.handle,&UsersRead,sizeof(URead)) == (unsigned) -1) {
        Aborted = TRUE;
        return;
      }
      if (writecheck(TempFile,&UsersRead,sizeof(URead)) == (unsigned) -1) {
        Aborted = TRUE;
        return;
      }
    }
  }
  if (userabort()) {
    Aborted = TRUE;
    return;
  }
}


/********************************************************************
*
*  Function: sort02()
*
*  Desc    : Sort users file by Name
*
*/

static void near pascal sort02(int Type) {
  unsigned  BuffSize;
  URead    *Ending;
  s01      *p;
  long      Counter;
  long      DiskOffset;
  char      SortBuf[16384];

  for (Counter = 1, DiskOffset = sizeof(URead); Counter <= Recs; Counter++, DiskOffset += sizeof(URead)) {
    if (readdosusersrecord() == -1) {
      Aborted = TRUE;
      return;
    }

    p = (s01 *) VMRecordCreate(&Array,SortSize,NULL,NULL);
    p->DiskOffset = DiskOffset;

    switch (Type) {
      case SECURITY: p->Num = UsersRead.SecurityLevel;
                     lasttofirst(p->SortValue,UsersRead.Name,sv01size);   //lint !e534
                     break;
      case TIMESON : p->Num = UsersRead.NumTimesOn;
                     lasttofirst(p->SortValue,UsersRead.Name,sv01size);   //lint !e534
                     break;
      case DOWNLOAD: p->Num = UsersRead.NumDownloads;
                     lasttofirst(p->SortValue,UsersRead.Name,sv01size);   //lint !e534
                     break;
      case UPLOAD  : p->Num = UsersRead.NumUploads;
                     lasttofirst(p->SortValue,UsersRead.Name,sv01size);   //lint !e534
                     break;
      case RATIO   : p->Num = (int) ludratio(UsersRead.NumUploads, UsersRead.NumDownloads);
                     lasttofirst(p->SortValue,UsersRead.Name,sv01size);   //lint !e534
                     break;
    }
  }

  VMSizeLock(&Array);

  fastprintmove(36,12,"   Sorting...",Colors[DISPLAY]);
  VMSort(&Array,SortSize,1,Recs,VM_FALSE,comp02,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));
  fastprintmove(47,12,"   Writing...",Colors[DISPLAY]);

  VMSeqFinalPass(&Array);

  if (Buffer != NULL) {
    BuffSize = NumBuffers * sizeof(URead);
    Ending = &Buffer[NumBuffers];

    for (Counter = 1; Counter <= Recs; ) {
      if (Recs-Counter < NumBuffers) {
        NumBuffers = (unsigned) (Recs - Counter + 1);
        BuffSize = NumBuffers * sizeof(URead);
        Ending = &Buffer[NumBuffers];
      }

      for (BufPtr = Buffer;  BufPtr < Ending; BufPtr++, Counter++) {
        p = (s01 *) VMRecordGetByIndex(&Array,Counter,NULL);
        doslseek(DosUsersFile.handle,p->DiskOffset,SEEK_SET);
        if (readcheck(DosUsersFile.handle,BufPtr,sizeof(URead)) == (unsigned) -1) {
          Aborted = TRUE;
          return;
        }
      }

      if (writecheck(TempFile,Buffer,BuffSize) == (unsigned) -1 || userabort()) {
        Aborted = TRUE;
        return;
      }
    }
  } else {
    for (Counter = 1;  Counter <= Recs;  Counter++) {
      p = (s01 *) VMRecordGetByIndex(&Array,Counter,NULL);
      doslseek(DosUsersFile.handle,p->DiskOffset,SEEK_SET);
      if (readcheck(DosUsersFile.handle,&UsersRead,sizeof(URead)) == (unsigned) -1) {
        Aborted = TRUE;
        return;
      }
      if (writecheck(TempFile,&UsersRead,sizeof(URead)) == (unsigned) -1) {
        Aborted = TRUE;
        return;
      }
    }
  }
  if (userabort()) {
    Aborted = TRUE;
    return;
  }
}


/********************************************************************
*
*  Function: sort03()
*
*  Desc    : Sort users file by Name
*
*/

static void near pascal sort03(int Type) {
  unsigned  BuffSize;
  URead    *Ending;
  s02      *p;
  long      Counter;
  long      DiskOffset;
  char      SortBuf[16384];

  for (Counter = 1, DiskOffset = sizeof(URead); Counter <= Recs; Counter++, DiskOffset += sizeof(URead)) {
    if (readdosusersrecord() == -1) {
      Aborted = TRUE;
      return;
    }

    p = (s02 *) VMRecordCreate(&Array,SortSize,NULL,NULL);
    p->DiskOffset = DiskOffset;

    switch (Type) {
      case BYTEDNLD : p->Num = basdbletolong(UsersRead.TotDnldBytes);
                      lasttofirst(p->SortValue,UsersRead.Name,sv01size);  //lint !e534
                      break;
      case BYTEUPLD : p->Num = basdbletolong(UsersRead.TotUpldBytes);
                      lasttofirst(p->SortValue,UsersRead.Name,sv01size);  //lint !e534
                      break;
      case BYTERATIO: p->Num = ddratio(basdbletodouble(UsersRead.TotDnldBytes),basdbletodouble(UsersRead.TotUpldBytes));
                      lasttofirst(p->SortValue,UsersRead.Name,sv01size);  //lint !e534
                      break;
    }
  }

  VMSizeLock(&Array);

  fastprintmove(36,12,"   Sorting...",Colors[DISPLAY]);
  VMSort(&Array,SortSize,1,Recs,VM_FALSE,comp03,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));
  fastprintmove(47,12,"   Writing...",Colors[DISPLAY]);

  VMSeqFinalPass(&Array);

  if (Buffer != NULL) {
    BuffSize = NumBuffers * sizeof(URead);
    Ending = &Buffer[NumBuffers];

    for (Counter = 1; Counter <= Recs; ) {
      if (Recs-Counter < NumBuffers) {
        NumBuffers = (unsigned) (Recs - Counter + 1);
        BuffSize = NumBuffers * sizeof(URead);
        Ending = &Buffer[NumBuffers];
      }

      for (BufPtr = Buffer;  BufPtr < Ending; BufPtr++, Counter++) {
        p = (s02 *) VMRecordGetByIndex(&Array,Counter,NULL);
        doslseek(DosUsersFile.handle,p->DiskOffset,SEEK_SET);
        if (readcheck(DosUsersFile.handle,BufPtr,sizeof(URead)) == (unsigned) -1) {
          Aborted = TRUE;
          return;
        }
      }

      if (writecheck(TempFile,Buffer,BuffSize) == (unsigned) -1 || userabort()) {
        Aborted = TRUE;
        return;
      }
    }
  } else {
    for (Counter = 1;  Counter <= Recs;  Counter++) {
      p = (s02 *) VMRecordGetByIndex(&Array,Counter,NULL);
      doslseek(DosUsersFile.handle,p->DiskOffset,SEEK_SET);
      if (readcheck(DosUsersFile.handle,&UsersRead,sizeof(URead)) == (unsigned) -1) {
        Aborted = TRUE;
        return;
      }
      if (writecheck(TempFile,&UsersRead,sizeof(URead)) == (unsigned) -1) {
        Aborted = TRUE;
        return;
      }
    }
  }
  if (userabort()) {
    Aborted = TRUE;
    return;
  }
}


/********************************************************************
*
*  Function: sortall()
*
*  Desc    : The main sort procedure.  It sets up the arrays and global sort
*            variables and calls the corresponding sort routine according to
*            the number passed to it in MenuSelection.
*
*/

void pascal sortall(void) {
                              /* 1   2   3   4   5   6   7   8   9   10  12  12  13  14  15  16  17  18  19  */
  static char BatchCommands[] = "NAMEPASSDATAHOMEEXPDUSERSYSOCITYSECUTIMEDLFIUPFIFRATDLBYUPBYBRATREVEPRIRSECR";
  bool  Success;
  bool  SortSelected;
  int   Command;
  char *p;
  char  Desc[81];

  SortReverse  = FALSE;
  SortReverse2 = FALSE;

  if (BatchMode) {
    MenuSelection = 0;
    SortSelected = FALSE;
    while ((p = parsepaths(NULL)) != NULL) {
      Command = findfour(BatchCommands,p);
      if ((!SortSelected) && Command >= 1 && Command <= 16) {
        MenuSelection = Command-1;
        SortSelected = TRUE;
      } else
        switch(Command) {
          case 17:
          case 18: SortReverse  = TRUE; break;
          case 19: SortReverse2 = TRUE; break;
        }
    }
  } else {
    MenuSelection += Additional;
    if (Additional != 0) {
      boxcls(17,16,61,22,Colors[MENUBOX],SINGLE);
      inputnum(19,18,1,"Sort PRIMARY Key in Reverse Order",&SortReverse,vBOOL,0);
      if (KeyFlags == ESC)
        return;
      inputnum(19,20,1,"Sort SECONDARY Key in Reverse Order",&SortReverse2,vBOOL,0);
      if (KeyFlags == ESC)
        return;
    } else {
      boxcls(19,18,59,22,Colors[MENUBOX],SINGLE);
      inputnum(26,20,1,"Sort in Reverse Order",&SortReverse,vBOOL,0);
      if (KeyFlags == ESC)
        return;
    }
  }

  Recs = lockusersfile("Sort Users File",BUFFERED);
  if (Recs == -1)
    return;
  Recs--;
  if (Recs == 0) {
    closeusersfile(TRUE,BUFFERED,FALSE);
    return;
  }


  strcpy(Desc,"Sorting by ");

  if (MenuSelection == 0)
    sub = lasttofirst;       //lint !e64    assignment is okay
  else
    sub = (void * (__cdecl *)(void *, const void *, unsigned))memcpy;


  switch (MenuSelection) {
    case  0: svsize = sv01size;
             strcat(Desc,"Name:");
             break;
    case  1: svsize = sv02size;
             strcat(Desc,"Password:");
             break;
    case  2: svsize = sv03size;
             strcat(Desc,"Business/Data Phone:");
             break;
    case  3: svsize = sv04size;
             strcat(Desc,"Home/Voice Phone:");
             break;
    case  4: svsize = sv05size;
             strcat(Desc,"Registration Expiration Date:");
             break;
    case  5: svsize = sv06size;
             strcat(Desc,"Comment Number 1:");
             break;
    case  6: svsize = sv07size;
             strcat(Desc,"Comment Number 2:");
             break;
    case  7: svsize = sv08size;
             strcat(Desc,"User Location:");
             break;
    case  8: svsize = sv09size;
             strcat(Desc,"Security Level, then by Name:");
             break;
    case  9: svsize = sv09size;
             strcat(Desc,"Number of Times On, then by Name:");
             break;
    case 10: svsize = sv09size;
             strcat(Desc,"Number of Files Downloaded, then by Name:");
             break;
    case 11: svsize = sv09size;
             strcat(Desc,"Number of Files Uploaded, then by Name:");
             break;
    case 12: svsize = sv09size;
             strcat(Desc,"Ratio of Files Upload:Download, then by Name:");
             break;
    case 13: svsize = sv10size;
             strcat(Desc,"Number of Bytes Downloaded, then by Name:");
             break;
    case 14: svsize = sv10size;
             strcat(Desc,"Number of Bytes Uploaded, then by Name:");
             break;
    case 15: svsize = sv10size;
             strcat(Desc,"Ratio of Bytes Upload:Download, then by Name:");
             break;
  }
  setcursor(CUR_NORMAL);
  fastprint(3,9,Desc,Colors[STATUS]);
  fastprintmove(3,12,"Loading user records into memory...",Colors[DISPLAY]);

  Aborted = TRUE;
  Success = FALSE;
  if (createtemp(PREALLOC,NONBUFFERED,FALSE) != -1) {
    Aborted = FALSE;
    if (copysysoprecord(BUFFERED,NONBUFFERED) != -1) {
      // set SortSize to the size of the structure to be sorted, plus the
      // 4-byte long integer used to store the disk offset
      // NOTE:  on 9/27/95 I added "+2" to this equation because I found that
      // if I sorted exactly 64 records (i.e. sysop record + 64 more records
      // were in the users file) that the system would lock up or at least
      // screwup the data in the DataSet.  It would mess up during the sort
      // operation.  Adding +2 to the size of the record appears to have
      // cured the problem.
      SortSize = svsize+sizeof(long)+2;
      VMInitRec(&Array,NULL,0,SortSize);
      ShowSwapMessage = TRUE;

      NumBuffers = (coreleft() > 0xFFFF ? 0xFFFF : (int) coreleft()) / sizeof(URead);
      if (NumBuffers > 16)
        NumBuffers &= 0xFFF0;

      if (NumBuffers > 0) {
        if ((Buffer = (URead *) malloc(NumBuffers * sizeof(URead))) == NULL)
          NumBuffers = 0;
      } else {
        Buffer = NULL;
        NumBuffers = 0;
      }

      switch (MenuSelection) {
        case  0: sort01((int) (UsersRead.Name           - (char*) &UsersRead)); break;
        case  1: sort01((int) (UsersRead.Password       - (char*) &UsersRead)); break;
        case  2: sort01((int) (UsersRead.BusDataPhone   - (char*) &UsersRead)); break;
        case  3: sort01((int) (UsersRead.HomeVoicePhone - (char*) &UsersRead)); break;
        case  4: sort01((int) (UsersRead.RegExpDate     - (char*) &UsersRead)); break;
        case  5: sort01((int) (UsersRead.UserComment    - (char*) &UsersRead)); break;
        case  6: sort01((int) (UsersRead.SysopComment   - (char*) &UsersRead)); break;
        case  7: sort01((int) (UsersRead.City           - (char*) &UsersRead)); break;
        case  8: sort02(SECURITY);   break;
        case  9: sort02(TIMESON);    break;
        case 10: sort02(DOWNLOAD);   break;
        case 11: sort02(UPLOAD);     break;
        case 12: sort02(RATIO);      break;
        case 13: sort03(BYTEDNLD);   break;
        case 14: sort03(BYTEUPLD);   break;
        case 15: sort03(BYTERATIO);  break;
      }

      if (Buffer != NULL)
        free(Buffer);
      VMDone(&Array);
      ShowSwapMessage = FALSE;
    }
    dosclose(TempFile);

    /* copy the USERS.INF file so that UNDO will work */

    if (copyfile(PcbData.InfFile,TempInfFileName,FALSE) != 0)
      Aborted = TRUE;

    if (Aborted)
      unlink(TempFileName);
    else
      Success = TRUE;
  }

  closeusersfile(Aborted,BUFFERED,FALSE);

  /* now change the USERS.INF temp file to USERS.BAK so that UNDO will work */
  if (! Aborted) {
    unlink(BackInfFileName);
    rename(PcbData.InfFile,BackInfFileName);  //lint !e534
    rename(TempInfFileName,PcbData.InfFile);  //lint !e534
  }

  if (Success)
    createallindexes();
}


void pascal sortprimary(void) {
  Additional = 0;
  menusel(4,SORTMENU2,SMALL,0,TRUE);
}

void pascal sortsecondary(void) {
  Additional = 8;
  menusel(5,SORTMENU3,SMALL,0,TRUE);
}


/********************************************************************
*
*  Function: sortmenu()
*
*  Desc    : Allows the user to choose which type of sort he wants to perform.
*
*  Calls   : exitmenusel(), menusel(), sortall()
*/

void pascal sortmenu(void) {
  menusel(3,SORTMENU1,SMALL,0,TRUE);
}
