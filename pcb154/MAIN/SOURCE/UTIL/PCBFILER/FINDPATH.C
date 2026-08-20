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
#include <string.h>
#include <stdlib.h>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

static void near  substitutepath(char *Path) {
  char Temp[60];

  if (SubstLen == 0)
    return;

  if (memcmp(Path,Work.SubstOld,SubstLen) == 0) {
    strcpy(Temp,Work.SubstNew);
    strcat(Temp,&Path[strlen(Work.SubstOld)]);
    strcpy(Path,Temp);
  }
}


int  fastfinddirpath(unsigned DirNum, DirListType *Rec, pcbconftype *Conf, unsigned NumTextDirs, int DirLstFile) {
  DirListType2 Buffer;

  if (DirNum == 0) {
    strcpy(Rec->DskPath,Conf->PrvUpldLoc);
    strcpy(Rec->DirPath,Conf->PrivDir);
    maxstrcpy(Rec->DirDesc,Conf->Name,sizeof(Rec->DirDesc)-16);
    strcat(Rec->DirDesc," PRIVATE upload");
    Rec->SortType = Conf->PrvUpldSort;
    goto end1;
  } else if (DirNum > NumTextDirs) {
    strcpy(Rec->DskPath,Conf->PubUpldLoc);
    strcpy(Rec->DirPath,Conf->UpldDir);
    maxstrcpy(Rec->DirDesc,Conf->Name,sizeof(Rec->DirDesc)-7);
    strcat(Rec->DirDesc," upload");
    Rec->SortType = Conf->PubUpldSort;
    goto end1;
  }

  if (DirLstFile == -1)
    return(-1);

  if (doslseek(DirLstFile,(long) (DirNum-1)*sizeof(Buffer),SEEK_SET) == -1L)
    return(-1);

  if (readcheck(DirLstFile,&Buffer,sizeof(Buffer)) == (unsigned) -1)
    return(-1);

  memcpy(Rec->DirPath,Buffer.DirPath,sizeof(Buffer.DirPath));  Rec->DirPath[sizeof(Buffer.DirPath)] = 0;
  memcpy(Rec->DskPath,Buffer.DskPath,sizeof(Buffer.DskPath));  Rec->DskPath[sizeof(Buffer.DskPath)] = 0;
  memcpy(Rec->DirDesc,Buffer.DirDesc,sizeof(Buffer.DirDesc));  Rec->DirDesc[sizeof(Buffer.DirDesc)] = 0;
  Rec->SortType = Buffer.SortType;

end1:
  stripright(Rec->DirPath,' ');
  stripright(Rec->DskPath,' ');
  stripright(Rec->DirDesc,' ');
  substitutepath(Rec->DirPath);
  return(0);
}


int  finddirpath(unsigned BoardNum, unsigned DirNum, DirListType *Rec) {
  pcbconftype  Conf;
  DirListType2 Buffer;
  int          DirLstFile;
  int          BytesRead;
  unsigned     NumTextDirs;

  if (BoardNum == 0xFFFF) {
    Rec->DirPath[0] = 0;
    strcpy(Rec->DskPath,Work.OffLine);
    stripright(Rec->DskPath,' ');
    addbackslash(Rec->DskPath,sizeof(Rec->DskPath));
    strcpy(Rec->DirDesc,"OFF-LINE directory");
    Rec->SortType = 0;
    goto end1;
  }

  getconfrecord(BoardNum,&Conf);
  NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));

  if (DirNum == 0) {
    strcpy(Rec->DskPath,Conf.PrvUpldLoc);
    strcpy(Rec->DirPath,Conf.PrivDir);
    maxstrcpy(Rec->DirDesc,Conf.Name,sizeof(Rec->DirDesc)-16);
    strcat(Rec->DirDesc," PRIVATE upload");
    Rec->SortType = Conf.PrvUpldSort;
    goto end1;
  } else if (DirNum > NumTextDirs) {
    strcpy(Rec->DskPath,Conf.PubUpldLoc);
    strcpy(Rec->DirPath,Conf.UpldDir);
    maxstrcpy(Rec->DirDesc,Conf.Name,sizeof(Rec->DirDesc)-7);
    strcat(Rec->DirDesc," upload");
    Rec->SortType = Conf.PubUpldSort;
    goto end1;
  }

  if ((DirLstFile = dosopencheck(Conf.DirNameLoc,OPEN_READ|OPEN_DENYWRIT)) == -1)
    return(-1);
  else {
    if (doslseek(DirLstFile,(long) (DirNum-1)*sizeof(Buffer),SEEK_SET) == -1L) {
      dosclose(DirLstFile);
      return(-1);
    }
    BytesRead = readcheck(DirLstFile,&Buffer,sizeof(Buffer));
    dosclose(DirLstFile);
    if (BytesRead == -1)
      return(-1);
    memcpy(Rec->DirPath,Buffer.DirPath,sizeof(Buffer.DirPath));  Rec->DirPath[sizeof(Buffer.DirPath)] = 0;
    memcpy(Rec->DskPath,Buffer.DskPath,sizeof(Buffer.DskPath));  Rec->DskPath[sizeof(Buffer.DskPath)] = 0;
    memcpy(Rec->DirDesc,Buffer.DirDesc,sizeof(Buffer.DirDesc));  Rec->DirDesc[sizeof(Buffer.DirDesc)] = 0;
    Rec->SortType = Buffer.SortType;
  }

end1:
  stripright(Rec->DirPath,' ');
  stripright(Rec->DskPath,' ');
  stripright(Rec->DirDesc,' ');
  substitutepath(Rec->DirPath);
  return(0);
}
