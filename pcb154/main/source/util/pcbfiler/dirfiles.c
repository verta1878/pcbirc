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
#include <string.h>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static int near  existinlist(char *Name) {
  long        Counter;
  parenttype *p;

  for (Counter = 1; Counter <= TotalParents; Counter++) {
    // throughout the rest of PCBFiler, the Parent dataset is always accessed
    // via the ParentsOnly index.  However, in this case, we don't care what
    // order the records are encountered in, so we'll just access them
    // sequentially, one at a time, via the built-in index.
    p = (parenttype *) VMRecordGetByIndex(&Parents,Counter,NULL);

    if (p->Line == DIRLINE) {
      if (strcmp(p->Fields.NName,Name) == 0)
        return(TRUE);
    }
  }
  return(FALSE);
}


int  includedirfiles(char *DskPath, int ConfNum, int DirNum, loadflagstype Flags) {
  bool             UpdateCombined = (Flags & UPDATECOMBINED);
  long             NewFiles;
  long             ParentPos;
  parenttype      *parent;
  parentsonlytype *parentsonly;
  combinedtype    *combined;
  find_t           Find;
  char             Directory[66];

  strcpy(Directory,DskPath);
  stripright(Directory,' ');
  addbackslash(Directory,sizeof(Directory));
  strcat(Directory,"*.*");
  NewFiles = 0;

  if (_dos_findfirst(Directory,FA_ARCH,&Find) == 0) {
    do {
      showaction();
      if (! existinlist(Find.name)) {
        parent = (parenttype *) VMRecordCreate(&Parents,sizeof(parenttype),&ParentPos,NULL);
        parent->Line                 = DIRLINE;
        parent->Fields.NCnfNum       = ConfNum;
        parent->Fields.NDirNum       = DirNum;
        parent->Fields.Exist         = DIRECT;
        parent->Fields.Keep          = KEEP;
        parent->Fields.FSize         = Find.size;
        parent->Fields.FDesc[0]      = 0;
        parent->Fields.Days          = 0;
        parent->Fields.FDate         = Find.wr_date;
        parent->Fields.NDate         = Find.wr_date;
        parent->Fields.FirstChildPos = VM_INVALID_POS;
        strcpy(parent->Fields.FName,Find.name);
        strcpy(parent->Fields.NName,Find.name);

        parentsonly = (parentsonlytype *) VMRecordCreate(&ParentsOnly,sizeof(parentsonlytype),NULL,NULL);
        parentsonly->Pos = ParentPos;

        if (UpdateCombined) {
          combined = (combinedtype *) VMRecordCreate(&Combined,sizeof(combinedtype),NULL,NULL);
          combined->Pos = ParentPos;
          combined->Line = DIRLINE;
        }

        NewFiles++;
      }
    } while (_dos_findnext(&Find) == 0);

    TotalLines   += NewFiles;
    TotalParents += NewFiles;
  }
  return(0);
}
