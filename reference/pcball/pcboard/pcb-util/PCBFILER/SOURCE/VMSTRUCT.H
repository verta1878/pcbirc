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


#ifndef __IDXCLASS__
#include "idx.hpp"
#endif

#ifndef VMDATA_INCLUDED
#include <vmdata.h>
#endif

typedef struct {
  ExistType  Exist;
  char       FName[13];
  long       FSize;
  unsigned   FDate;
  char       FDesc[46];
  char       NName[13];
  unsigned   NDate;
  unsigned   NCnfNum;
  unsigned   NDirNum;
  unsigned   Days;
  KeepType   Keep;
  long       FirstChildPos;
} parentfieldstype;

typedef struct {
  LineType         Line;
  union {
  parentfieldstype Fields;
  char             Str[79];
  };
} parenttype;

typedef struct {
  char Str[79];
  long ParentPos;
  long NextChildPos;
  long PrevChildPos;
} childtype;

typedef struct {
  long Pos;
} parentsonlytype;

typedef struct {
  LineType Line;
  long     Pos;
} combinedtype;

typedef struct {
  VMAVLInfo Reserved;
  char      FName[13];
  long      ParentPos;
} dupestype;

// variables declared in INIT.C

extern bool     ShowSwapMessage;
extern bool     ShowSwapQuick;

// variables declared in READDIR.C

extern VMDataSet     Parents;
extern VMDataSet     Children;
extern VMDataSet     ParentsOnly;
extern VMDataSet     Combined;
extern VMDataSet     Dupes;
extern VMAVLControl  DupesControl;
extern VMAVLTree     DupesTree;

// prototype in READDIR.C

VM_SHINT comparedupes(const void *Rec1, const void *Rec2);

// prototype in SAVEDIR.C

int pascal writeparent(DOSFILE *file, diridxclass &IdxFile, parenttype *parent, bool UpdateIdx, bool UploadDir);
