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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <newdata.h>
#include <pcb.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "idx.hpp"
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

static char        SortType;
static int         CompareType;
static parenttype *A;
static parenttype *B;
static char       *SortDesc[4] = { "Name", "Date", "A", "De" };


/***********************************************************************

CompareType values:
  0 :  DIRLINE  - DIRLINE
  1 :  TEXTLINE - DIRLINE
  2 :  DIRLINE  - TEXTLINE
  3 :  TEXTLINE - TEXTLINE

  4 :  DIRLINE  - DUPELINE
  5 :  TEXTLINE - DUPELINE
  6 :  DUPELINE - DIRLINE
  7 :  DUPELINE - TEXTLINE
  8 :  DUPELINE - DUPELINE

***********************************************************************/


static int near pascal sortnameup(void) {
  int CompRet;

  switch (CompareType) {
    case 0: CompRet = strcmp(A->Fields.NName,B->Fields.NName);
            break;
/*
    case 1: CompRet = memcmp(A->Str,B->Fields.NName,strlen(B->Fields.NName));
            break;
    case 2: CompRet = memcmp(A->Fields.NName,B->Str,strlen(A->Fields.NName));
            break;
*/
    case 1: CompRet = 1;
            break;
    case 2: CompRet = -1;
            break;
    case 3: CompRet = memcmp(A->Str,B->Str,12);
            break;
  }
  return(CompRet);  //lint !e644 CompRet is initialized
}

static int near pascal sortnamedn(void) {
  int CompRet;

  switch (CompareType) {
    case 0: CompRet = -strcmp(A->Fields.NName,B->Fields.NName);
            break;
/*
    case 1: CompRet = -memcmp(A->Str,B->Fields.NName,strlen(B->Fields.NName));
            break;
    case 2: CompRet = -memcmp(A->Fields.NName,B->Str,strlen(A->Fields.NName));
            break;
*/
    case 1: CompRet = 1;
            break;
    case 2: CompRet = -1;
            break;
    case 3: CompRet = -memcmp(A->Str,B->Str,12);
            break;
  }
  return(CompRet);  //lint !e644 CompRet is initialized
}


static int near pascal sortdateup(void) {
  int CompRet;
  int Date1;
  int Date2;

#pragma warn -sus
  switch (CompareType) {
    case 0: if ((CompRet = (A->Fields.NDate - B->Fields.NDate)) == 0)
              CompRet = sortnameup();
            break;
/*
    case 1: if (strlen(A->Str) > 24) {
              A->Str[31] = 0;
              Date1 = ctod2(&A->Str[23]);
              if ((CompRet = (Date1 - B->Fields.NDate)) == 0)
                CompRet = sortnameup();
            } else CompRet = sortnameup();
            break;
    case 2: if (strlen(B->Str) > 24) {
              B->Str[31] = 0;
              Date2 = ctod2(&B->Str[23]);
              if ((CompRet = (A->Fields.NDate - Date2)) == 0)
                CompRet = sortnameup();
            } else CompRet = sortnameup();
            break;
*/
    case 1: CompRet = 1;
            break;
    case 2: CompRet = -1;
            break;
    case 3: if (strlen(A->Str) > 24) {
              A->Str[31] = 0;
              Date1 = ctod2(&A->Str[23]);
            } else {
              CompRet = sortnameup();
              break;
            }
            if (strlen(B->Str) > 24) {
              B->Str[31] = 0;
              Date2 = ctod2(&B->Str[23]);
            } else {
              CompRet = sortnameup();
              break;
            }
            if ((CompRet = (Date1 - Date2)) == 0)
              CompRet = sortnameup();
            break;
  }
  return(CompRet);  //lint !e644 CompRet is initialized
#pragma warn +sus
}

static int near pascal sortdatedn(void) {
  int CompRet;
  int Date1;
  int Date2;

#pragma warn -sus
  switch (CompareType) {
    case 0: if ((CompRet = - (int) (A->Fields.NDate - B->Fields.NDate)) == 0)
              CompRet = sortnameup();
            break;
/*
    case 1: if (strlen(A->Str) > 24) {
              A->Str[31] = 0;
              Date1 = ctod2(&A->Str[23]);
              if ((CompRet = - (int) (Date1 - B->Fields.NDate)) == 0)
                CompRet = sortnameup();
            } else CompRet = sortnameup();
            break;
    case 2: if (strlen(B->Str) > 24) {
              B->Str[31] = 0;
              Date2 = ctod2(&B->Str[23]);
              if ((CompRet = - (int) (A->Fields.NDate - Date2)) == 0)
                CompRet = sortnameup();
            } else CompRet = sortnameup();
            break;
*/
    case 1: CompRet = -1;
            break;
    case 2: CompRet = 1;
            break;
    case 3: if (strlen(A->Str) > 24) {
              A->Str[31] = 0;
              Date1 = ctod2(&A->Str[23]);
            } else {
              CompRet = sortnameup();
              break;
            }
            if (strlen(B->Str) > 24) {
              B->Str[31] = 0;
              Date2 = ctod2(&B->Str[23]);
            } else {
              CompRet = sortnameup();
              break;
            }
            if ((CompRet = -(Date1 - Date2)) == 0)
              CompRet = sortnameup();
            break;
  }
  return(CompRet);  //lint !e644 CompRet is initialized
#pragma warn +sus
}


static VM_SHINT compare(const void *Rec1, const void *Rec2) {
  int              CompRet;
  parentsonlytype *a = (parentsonlytype *) Rec1;
  parentsonlytype *b = (parentsonlytype *) Rec2;
#ifdef DEBUGSORT
  char             Temp[10];
#endif

  A = (parenttype *)VMRecordGetByPos(&Parents,a->Pos);
  B = (parenttype *)VMRecordGetByPos(&Parents,b->Pos);

  switch (A->Line) {
    case DIRLINE : switch (B->Line) {
                     case DIRLINE : CompareType = 0; break; /* DIRLINE - DIRLINE  */
                     case TEXTLINE: CompareType = 2; break; /* DIRLINE - TEXTLINE */
                   }
                   break;
    case TEXTLINE: switch (B->Line) {
                     case DIRLINE : CompareType = 1; break; /* TEXTLINE - DIRLINE  */
                     case TEXTLINE: CompareType = 3; break; /* TEXTLINE - TEXTLINE */
                   }
                   break;
  }

#ifdef DEBUGSORT
  printf("\n");
  if (A->Line == TEXTLINE)
    printf("T]%s\n",A->Str);
  else
    printf("D]%-12.12s%9ld  %s  %s\n",A->Fields.NName,A->Fields.FSize,dtoc(A->Fields.NDate,Temp),A->Fields.FDesc);
  if (B->Line == TEXTLINE)
    printf("T]%s\n",B->Str);
  else
    printf("D]%-12.12s%9ld  %s  %s\n",A->Fields.NName,B->Fields.FSize,dtoc(B->Fields.NDate,Temp),B->Fields.FDesc);
#endif

  switch (SortType) {
    case 1: CompRet = sortnameup(); break;
    case 2: CompRet = sortdateup(); break;
    case 3: CompRet = sortnamedn(); break;
    case 4: CompRet = sortdatedn(); break;
  }
#ifdef DEBUGSORT
  printf("Return Value = %d\n",CompRet);
#endif
  showaction();
  return(CompRet);  //lint !e644 CompRet is initialized
}


void pascal writedirfile(char *FileName) {
  bool             UpdateIdx;
  parentsonlytype *parentsonly;
  parenttype      *parent;
  long             Count;
  DOSFILE          OutFile;
  diridxclass      IdxFile;

  unlink(FileName);
  if (dosfopen(FileName,OPEN_WRIT|OPEN_DENYRDWR,&OutFile) == -1)
    return;

  dossetbuf(&OutFile,16384);
  UpdateIdx = IdxFile.create(FileName);

  for (Count = 1; Count <= TotalParents; Count++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,Count,NULL);
    parent = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);

    // skip over files that were dupes (see dupes.c) that should be deleted
    if (parent->Line == DIRLINE && parent->Fields.Keep == DELETE)
      continue;

    writeparent(&OutFile,IdxFile,parent,UpdateIdx,TRUE);  //lint !e534   always UploadDir=TRUE to avoid stripping Uploaded By lines
    showaction();
  }

  dosfclose(&OutFile);
  if (UpdateIdx)
    IdxFile.close();
}


static long near pascal countheadlines(void) {
  parentsonlytype *parentsonly;
  parenttype      *parent;
  long             Count;

  for (Count = 1; Count <= TotalParents; Count++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,Count,NULL);
    parent = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);
    if (parent->Line != TEXTLINE)
      break;
  }
  return(Count-1);
}


static long near pascal counttaillines(void) {
  parentsonlytype *parentsonly;
  parenttype      *parent;
  long             Count;

  for (Count = TotalParents; Count >= 1; Count--) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,Count,NULL);
    parent = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);
    if (parent->Line != TEXTLINE)
      break;
  }
  return(TotalParents - Count);
}


void pascal sortchanged(void) {
  int             Len;
  long            TotalHeadLines;
  long            TotalTailLines;
  long            TotalSortLines;
  char            Temp[80];
  char            Buffer[128];
  parentsonlytype SortBuf[1024];

  if (openchangeread() == -1)
    return;

  while (dosfgets(Buffer,sizeof(Buffer),&DosChangedList) != -1) {
    if (Buffer[0] > 0 && Buffer[0] != '-' && Buffer[0] != '=') {
      Len = strlen(Buffer);
      SortType = Buffer[Len-1] - '0';
      Buffer[Len-2] = 0;

      if (SortType >= 1 && SortType <= 4) {  /* do we need to sort it? */
        sprintf(Temp,"Sorting   : %s by %s (%sscending)",
                Buffer,SortDesc[(SortType-1) & 1],SortDesc[(SortType < 3 ? 2 : 3)]);
        printscroll(Temp,Colors[QUESTION]);

        VMInitRec(&Parents,NULL,0,sizeof(parenttype));
        VMInitRec(&Children,NULL,0,sizeof(childtype));
        VMInitRec(&ParentsOnly,NULL,0,sizeof(parentsonlytype));

        VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
        VMAccessAttrSet(&Children,VM_SEQUENTIAL);
        VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

        fastprintmove(2,19,"Reading",Colors[ANSWER]);
        startaction('.',FALSE,32);

        if (readdirfile(Buffer,0,0,SHOWACTION) != -1) {
          VMSizeLock(&Parents);
          VMSizeLock(&Children);
          VMSizeLock(&ParentsOnly);

          TotalHeadLines = countheadlines();
          // if TotalHeadLines=TotalParents then EVERYTHING has already been counted
          TotalTailLines = (TotalHeadLines < TotalParents ? counttaillines() : 0);
          TotalSortLines = TotalParents - (TotalHeadLines + TotalTailLines);

          if (TotalSortLines > 0) {
            clsbox(2,19,45,19,Colors[DISPLAY]);
            fastprintmove(2,19,"Sorting",Colors[ANSWER]);
            startaction('.',FALSE,256);

            VMAccessAttrSet(&ParentsOnly,VM_RANDOM);
            VMAccessAttrSet(&Children,VM_RANDOM);
            VMAccessAttrSet(&Parents,VM_RANDOM);

            VMSort(&ParentsOnly,sizeof(parentsonlytype),TotalHeadLines+1,TotalSortLines,VM_FALSE,compare,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));

            VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
            VMAccessAttrSet(&Children,VM_SEQUENTIAL);
            VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

            clsbox(2,19,45,19,Colors[DISPLAY]);
            fastprintmove(2,19,"Writing",Colors[ANSWER]);
            startaction('.',FALSE,32);
            writedirfile(Buffer);
          }
        }

        VMDone(&Parents);
        VMDone(&Children);
        VMDone(&ParentsOnly);
      }
    }
    if (checkuserabort()) {
      Aborted = TRUE;
      break;
    }
  }
  dosfclose(&DosChangedList);
}


static void near pascal reinitcombined(void) {
  parentsonlytype *parentsonly;
  combinedtype    *combined;
  parenttype      *parent;
  childtype       *child;
  long             Count;
  long             ChildPos;

  VMInitRec(&Combined,NULL,0,sizeof(combinedtype));
  VMAccessAttrSet(&Combined,VM_SEQUENTIAL);

  for (Count = 1; Count <= TotalParents; Count++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,Count,NULL);
    parent = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);

    combined = (combinedtype *) VMRecordCreate(&Combined,sizeof(combinedtype),NULL,NULL);
    combined->Pos  = parentsonly->Pos;
    combined->Line = parent->Line;

    if (parent->Line == DIRLINE) {
      ChildPos = parent->Fields.FirstChildPos;
      while (ChildPos != VM_INVALID_POS) {
        combined = (combinedtype *) VMRecordCreate(&Combined,sizeof(combinedtype),NULL,NULL);
        combined->Line = DUPELINE;
        combined->Pos  = ChildPos;
        child = (childtype *) VMRecordGetByPos(&Children,ChildPos);
        ChildPos = child->NextChildPos;
      }
    }
  }
}


int pascal sortinplace(char Order, bool UpdateCombined) {
  long            TotalHeadLines;
  long            TotalTailLines;
  long            TotalSortLines;
  parentsonlytype SortBuf[1024];

  TotalHeadLines = countheadlines();
  // if TotalHeadLines=TotalParents then EVERYTHING has already been counted
  TotalTailLines = (TotalHeadLines < TotalParents ? counttaillines() : 0);
  TotalSortLines = TotalParents - (TotalHeadLines + TotalTailLines);

  if (TotalSortLines > 0) {
    SortType = Order;
    clsbox(1,5,78,19,Colors[DISPLAY]);
    fastprintmove(33,12,"Sorting ",Colors[DISPLAY]);
    startaction('.',FALSE,256);

    // release combined memory
    VMDone(&Combined);

    // set to random access to improve sorting speed
    VMAccessAttrSet(&ParentsOnly,VM_RANDOM);
    VMAccessAttrSet(&Children,VM_RANDOM);
    VMAccessAttrSet(&Parents,VM_RANDOM);

    VMSort(&ParentsOnly,sizeof(parentsonlytype),TotalHeadLines+1,TotalSortLines,VM_FALSE,compare,(VMSortFunc *)qsort,SortBuf,sizeof(SortBuf));

    // set back to sequential access
    VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
    VMAccessAttrSet(&Children,VM_SEQUENTIAL);
    VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

    // re-allocate combined memory, and re-read the lines into the index
    if (UpdateCombined)
      reinitcombined();

    clsbox(1,5,78,19,Colors[DISPLAY]);
    return(-1);
  }
  return(0);
}
