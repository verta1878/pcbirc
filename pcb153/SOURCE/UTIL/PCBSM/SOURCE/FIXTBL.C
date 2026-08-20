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
#include <mem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <process.h>
#include <dosfunc.h>
#include <scrnio.h>
#include "pcbfiles.h"

typedef struct {
  int  Num;
  int  Sec;
} SecTableTyp1;

SecTableTyp1 OldTable[60];
SecTableType NewTable[60];

char *TableName = "PCBSM.TBL";
char *NewName   = "TABLE.NEW";


void pascal readtable(const int Num) {

  int Table;

  if ((Table = open(TableName,O_RDWR | O_DENYNONE)) == -1) {
    errorexit(TableName,"Unable to Read",__FILE__,__LINE__);
  }

  lseek(Table,sizeof(OldTable) * Num,0);
  if (read(Table,OldTable,sizeof(OldTable)) == 0)
    memset(OldTable,0,sizeof(OldTable));
  close(Table);
}


void pascal writetable(const int Num) {

  int Table;
  SecTableTyp1 *p;
  SecTableType *q;

  if ((Table = open(NewName,O_RDWR | O_CREAT, S_IREAD | S_IWRITE)) == -1) {
    errorexit(NewName,"Error Opening File",__FILE__,__LINE__);
  }

  memset(NewTable,0,sizeof(NewTable));

  for (p = OldTable, q = NewTable; p < &OldTable[60]; p++, q++) {
    q->Num = p->Num;
    q->Sec = p->Sec;
  }

  lseek(Table,sizeof(NewTable) * Num,0);
  write(Table,NewTable,sizeof(NewTable));
  close(Table);
}

void main(void) {

  _fmode = O_BINARY;
  readtable(0);
  writetable(0);

  readtable(1);
  writetable(1);
}
