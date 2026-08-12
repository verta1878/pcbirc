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
#include <stdio.h>
#include <string.h>
#include <misc.h>
#include <dosfunc.h>
#include <pcb.h>
#include <scrnio.h>
#include <scrnio.ext>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "idx.hpp"

#ifdef DEBUG
#include <memcheck.h>
#endif

extern struct ffblk DTA;  /* declared in exist.c */


static bool near  idxfilenameokay(char *IdxName, char *FileName) {
  char *p;
  char *q;
  char Temp[13];

  if ((p = strrchr(FileName,'\\')) != NULL)
    p++;
  else if ((p = strrchr(FileName,':')) != NULL)
    p++;
  else
    p = FileName;

  // check to see if the DIR filename has a period in it
  if ((q = strchr(p,'.')) != NULL) {
    // if it does, is the filename less than 7.3?  if not, get out now
    if (((int) (q - p)) > 7 || strlen(q+1) > 3)
      return(FALSE);

    // otherwise, change the filename by substituting X's in all of the
    // blank spots in an 8.3 format.
    strcpy(Temp,"XXXXXXXX.XXX");
    memcpy(Temp,p,(int) (q - p));
    memcpy(&Temp[9],q+1,strlen(q+1));
    strcpy(IdxName,FileName);
    strcpy(&IdxName[(int) (p - FileName)],Temp);
  } else {
    strcpy(IdxName,FileName);
    strcat(IdxName,".IDX");
  }
  return(TRUE);
}



diridxclass::diridxclass(void) {
  Opened = FALSE;
  memset(&IdxHeader,0,sizeof(IdxHeader));
  memset(&FileRec,0,sizeof(FileRec));
  memset(&IdxFile,0,sizeof(IdxFile));
  memset(DirName,0,sizeof(DirName));
}


bool  diridxclass::append(char *DirFileName) {
  if (! idxfilenameokay(IdxName,DirFileName))
    return(FALSE);

  strcpy(DirName,DirFileName);

  if (fileexist(IdxName) == 255)
    return(diridxclass::create(DirFileName));

  if (dosfopen(IdxName,OPEN_RDWR|OPEN_DENYWRIT,&IdxFile) == -1)
    return(FALSE);

  // read in IDX header, see if the file needs to be updated
  // if not, get out now without any further action (will rebuild later)
  if (needtoresync()) {
    dosfclose(&IdxFile);
    unlink(IdxName);
  } else {
    dossetbuf(&IdxFile,8192);
    dosfseek(&IdxFile,0,SEEK_END);
    Opened = TRUE;
    return(TRUE);
  }

  return(FALSE);
}


bool  diridxclass::create(char *DirFileName) {
  if (! idxfilenameokay(IdxName,DirFileName))
    return(FALSE);

  strcpy(DirName,DirFileName);

  if (dosfopen(IdxName,OPEN_RDWR|OPEN_DENYWRIT|OPEN_CREATE,&IdxFile) == -1)
    return(FALSE);

  dossetbuf(&IdxFile,16384);  // create-only - give it a bigger buffer
  memset(&IdxHeader,0,sizeof(IdxHeader));
  dosfwrite(&IdxHeader,sizeof(IdxHeader),&IdxFile);    //lint !e534
  Opened = TRUE;
  return(TRUE);
}


void  diridxclass::close(void) {
  if (! Opened)
    return;

  dosrewind(&IdxFile);

  // fill the DTA variable
  fileexist(DirName);   //lint !e534

  IdxHeader.FileSize = DTA.ff_fsize;
  IdxHeader.FileTime = DTA.ff_ftime;
  IdxHeader.FileDate = DTA.ff_fdate;
  memset(IdxHeader.Reserved,0,sizeof(IdxHeader.Reserved));

  dosfwrite(&IdxHeader,sizeof(IdxHeader),&IdxFile);    //lint !e534
  dosfclose(&IdxFile);
  Opened = FALSE;
}


int  diridxclass::write(void) {
  if (FileRec.Date > IdxHeader.Latest)
    IdxHeader.Latest = FileRec.Date;
  return(dosfwrite(&FileRec,sizeof(FileRec),&IdxFile));
}


bool  diridxclass::needtoresync(void) {
  if (dosfread(&IdxHeader,sizeof(IdxHeader),&IdxFile) == -1)
    return(TRUE);

  // load the DTR variable with this function call
  fileexist(DirName);  //lint !e534

  // if the header signature does not match the current file, then yes, we
  // need to resynchronize the file because someone else has updated the
  // DIR file -- perhaps with a text editor
  if (IdxHeader.FileSize != DTA.ff_fsize ||
      IdxHeader.FileTime != DTA.ff_ftime ||
      IdxHeader.FileDate != DTA.ff_fdate)
    return(TRUE);

  return(FALSE);
}


bool  diridxclass::fill(char *FileName, char *Str, long OffsetIntoDir) {
  unsigned Date;

  Date = ctod2(&Str[23]);
  if (Date == 0 || Date >= 0xFFFE)
    return(FALSE);

  makeidxname(FileName,FileRec.Name);

  FileRec.Date     = Date;
  FileRec.LoOffset = (unsigned) OffsetIntoDir;
  FileRec.HiOffset = (char) ((OffsetIntoDir & 0xFF0000L) >> 16);
  return(TRUE);
}


static void near  createdirindex(char *DirFileName) {
  long          Offset;
  DOSFILE       InFile;
  char          Str[128];
  diridxclass   Idx;

  if (dosfopen(DirFileName,OPEN_READ|OPEN_DENYNONE,&InFile) == -1)
    return;

  if (! Idx.create(DirFileName))
    return;

  while (1) {
    Offset = dosfseek(&InFile,0,SEEK_CUR);
    if (dosfgets(Str,sizeof(Str),&InFile) == -1)
      break;

    if (Str[0] != ' ' && strlen(Str) >= 30) {
      if (Idx.fill(Str,Str,Offset)) {
        if (Idx.write() == -1)
          break;
      }
    }
  }

  Idx.close();
  dosfclose(&InFile);
}


void  indexchanged(void) {
  char Temp[80];
  char Buffer[100];

  if (openchangeread() == -1)
    return;

  while (dosfgets(Buffer,sizeof(Buffer),&DosChangedList) != -1) {
    sprintf(Temp,"Indexing  : %s",Buffer);
    printscroll(Temp,Colors[QUESTION]);

    createdirindex(Buffer);

    if (checkuserabort()) {
      Aborted = TRUE;
      break;
    }
  }

  dosfclose(&DosChangedList);
}
