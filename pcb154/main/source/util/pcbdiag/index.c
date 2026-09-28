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


#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMHINTS 100

typedef struct {
  long RecNum;
  char Name[25];
} hinttype;

static long      Low;
static long      High;
static int       TotalRecs;
static DOSFILE   SingleIndex;
IndexType Index;
static hinttype  Hints[NUMHINTS];


/********************************************************************
*
*  Function: compareindex()
*
*/

static int near  compareindex(long RecNum, char *SrchName) {
  dosfseek(&SingleIndex,(RecNum-1) * sizeof(IndexType),SEEK_SET);
  dosfread(&Index,sizeof(IndexType),&SingleIndex);
  return(memcmp(SrchName,Index.UserName,25));
}


static void near  setrange(char *SrchName) {
  int RetVal;
  int Rec;
  int register LowHint;
  int register HighHint;

  LowHint = 0;
  HighHint = NUMHINTS-1;

  while (TRUE) {
    if (HighHint <= LowHint+1) {
      if (HighHint == LowHint+1)
        Rec = LowHint = HighHint;
      else
        return;
    } else
      Rec = LowHint + ((HighHint - LowHint) >> 1);
    if ((RetVal = memcmp(SrchName,Hints[Rec].Name,25)) == 0) {
      Low = High = Hints[Rec].RecNum;
      return;
    }
    if (RetVal < 0) {
      High = Hints[Rec].RecNum;
      HighHint = Rec;
    } else {
      Low = Hints[Rec].RecNum;
      LowHint = Rec;
    }
  }
}


/********************************************************************
*
*  Function: multiplereads()
*
*  Desc    : Finds a user record by doing multiple reads on the users file in
*            a "binary search" pattern
*/

static bool near  multiplereads(char *SrchName) {
  long Rec;
  int  RetVal;

  Low  = 1;
  High = TotalRecs;

  if (TotalRecs > NUMHINTS+2)
    setrange(SrchName);

  if ((RetVal = compareindex(Low,SrchName)) == 0)
    return(TRUE);
  else
    if (RetVal < 0)
      return(FALSE);

  if ((RetVal = compareindex(High,SrchName)) == 0)
    return(TRUE);
  else
    if (RetVal > 0)
      return(FALSE);

  while (TRUE) {
    if (High <= Low + 1) return(FALSE);
    Rec = Low + ((High - Low) >> 1);
    if ((RetVal = compareindex(Rec,SrchName)) == 0)
      return(TRUE);
    else
      if (RetVal < 0)
        High = Rec;
      else
        Low  = Rec;
  }
//  return(FALSE);  unreachable
}


/********************************************************************
*
*  Function: finduser()
*
*  Desc    : Locates a user name specified by SrchName by using a binary
*            search thru the index files.
*
*  Returns : The record number of the user if found, -1 otherwise.
*
*  Note    :  a modified version of finduser() is in PCBDIAG's ANALYZE.C
*/

long  finduser(char SrchName[]) {
  if (multiplereads(SrchName))
    return(Index.UserRec);
  return(-1);
}


static void near  getrecord(long Rec, int X) {
  dosfseek(&SingleIndex,(Rec-1) * sizeof(IndexType),SEEK_SET);
  dosfread(&Index,sizeof(IndexType),&SingleIndex);
  Hints[X].RecNum = Rec;
  memcpy(Hints[X].Name,Index.UserName,25);
}

static void near  makehints(void) {
  long Step;
  long Rec;
  int  X;

  if (TotalRecs > NUMHINTS+2) {
    Step = TotalRecs / (NUMHINTS+1);

    for (X = 0, Rec = Step; X < NUMHINTS-1; X++, Rec += Step)
      getrecord(Rec,X);
    getrecord(TotalRecs,NUMHINTS-1);
  }
}


void  createsingleindex(void) {
  char static FileName[] = "pcbndx.A";
  char IndexFileName[40];
  char *p;
  char Ch;

  strcpy(IndexFileName,PcbData.NdxLoc);
  strcat(IndexFileName,FileName);
  p = &IndexFileName[strlen(IndexFileName)-1];

  copyfile(IndexFileName,"TMPINDEX.$$$",FALSE);
  for (Ch = 'B'; Ch <= 'Z'; Ch++) {
    *p = Ch;
    appendfile(IndexFileName,"TMPINDEX.$$$",FALSE);
  }

  dosfopen("TMPINDEX.$$$",OPEN_READ|OPEN_DENYRDWR,&SingleIndex);
  TotalRecs = (int) numrecs(SingleIndex.handle,sizeof(Index));
  dosrewind(&SingleIndex);
  dossetbuf(&SingleIndex,32768U);
  makehints();
}

void  closesingleindex(void) {
  dosfclose(&SingleIndex);
  unlink("TMPINDEX.$$$");
}
