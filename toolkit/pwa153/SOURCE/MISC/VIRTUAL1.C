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
#include <sys/stat.h>
#include <alloc.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include "virtual.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define  BLOCKSIZE  64       /* NOTE: this MUST be a power of 2 */

struct {
  VirType  *LastRecAdr;
  unsigned  LastRecNum;
  int       File;
  int       DelFile;
  long      Offset;
  unsigned  Size;
  unsigned  AllocSize;
  unsigned  DelRecs;
  char      *MainMem;
  unsigned  MainMemOffset;
  long      CoreCheck;
  unsigned  MainMemCheck;
} Vir;


VirType  *Virtual;

void virtualerror(int Code) {
  memset(&MsgData,0,sizeof(MsgData));
  beep();
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Line1   = 18;
  MsgData.Color1  = Colors[HEADING];
  switch (Code) {
    case VIRDISKERR : MsgData.Msg1 = "Error writing to disk (check disk space)"; break;
    case VIRPASTEND : MsgData.Msg1 = "Error: reached maximum number of records"; break;
  }
  showmessage();
}


void pascal getvirtualrec(const VirType *p, void *Buffer) {
  if (p->Rec != DISKBUF) {
    memcpy(Buffer,p->Loc.Buf,Vir.Size);
  } else {
    lseek(Vir.File,p->Loc.Offset,SEEK_SET);
    read(Vir.File,Buffer,Vir.Size);
  }
}


int pascal putvirtualrec(VirType *p, const void *Buffer) {
  if (p > Vir.LastRecAdr) {
    virtualerror(VIRPASTEND);
    return(VIRPASTEND);
  }

  if (Vir.MainMemOffset == Vir.MainMemCheck) {
    if (coreleft() > Vir.CoreCheck) {
      Vir.MainMemOffset = 0;
      p->Loc.Buf = Vir.MainMem = malloc(Vir.AllocSize);
      p->Rec = MEMBUFA;
      memcpy(p->Loc.Buf,Buffer,Vir.Size);
      return(VIROKAY);
    } else {
      p->Rec = DISKBUF;
      lseek(Vir.File,Vir.Offset,SEEK_SET);
      p->Loc.Offset = Vir.Offset;
      Vir.Offset += Vir.Size;
      if (write(Vir.File,Buffer,Vir.Size) != Vir.Size) {
        virtualerror(VIRDISKERR);
        return(VIRDISKERR);
      } else
        return(VIROKAY);
    }
  } else {
    Vir.MainMemOffset += Vir.Size;
    p->Loc.Buf = &Vir.MainMem[Vir.MainMemOffset];
    p->Rec = MEMBUFM;
    memcpy(p->Loc.Buf,Buffer,Vir.Size);
    return(VIROKAY);
  }
}


void pascal updvirtualrec(const VirType *p, void *Buffer) {
  if (p->Rec != DISKBUF) {
    memcpy(p->Loc.Buf,Buffer,Vir.Size);
  } else {
    lseek(Vir.File,p->Loc.Offset,SEEK_SET);
    write(Vir.File,Buffer,Vir.Size);
    if (write(Vir.File,Buffer,Vir.Size) != Vir.Size)
      virtualerror(VIRDISKERR);
  }
}


int pascal insvirtualrec(void *EmptyBuffer, unsigned RecNum, unsigned TotRecs) {
  int     Code;
  VirType VirRec;

/*   record 0            DIAGRAM OF SEQUENCE OF EVENTS
     record 1
     record 2 - current record
     record 3
     record 4 - last record (TotRecs = 5)

     record 0
     record 1
     record 2 - current record (offset = 2)
     record 3
     record 4 -
     record 5 - new record - also last record - saved in VirRec (offset = 5)

     record 0
     record 1
     record 5 - current record - also new record from VirRec (offset = 2)
     record 2
     record 3 -
     record 4 - new record - also last record - saved in VirRec */

  if (TotRecs+1 > Vir.LastRecNum) {
    virtualerror(VIRPASTEND);
    return(VIRPASTEND);
  }

  Code = putvirtualrec(&Virtual[TotRecs],EmptyBuffer);
  if (Code)
    return(Code);

  VirRec = Virtual[TotRecs];
  memmove(&Virtual[RecNum+1],&Virtual[RecNum],(TotRecs-RecNum) * sizeof(VirType));
  Virtual[RecNum] = VirRec;
  return(VIROKAY);
}


int pascal delvirtualrec(unsigned RecNum, unsigned TotRecs) {
  if (write(Vir.DelFile,&Virtual[RecNum],sizeof(VirType)) != sizeof(VirType)) {
    virtualerror(VIRDISKERR);
    return(VIRDISKERR);
  }
  Vir.DelRecs++;

  memmove(&Virtual[RecNum],&Virtual[RecNum+1],(TotRecs-RecNum) * sizeof(VirType));
  return(VIROKAY);
}


void pascal openvirtual(unsigned MaxRecs, unsigned Size) {

  unsigned NumBytes;

  Vir.Size          = Size;
  Vir.CoreCheck     = Size * (BLOCKSIZE * 2);
  Vir.AllocSize     = Size *  BLOCKSIZE;
  Vir.MainMemCheck  = Size * (BLOCKSIZE - 1);
  Vir.MainMemOffset = Vir.MainMemCheck;

  NumBytes   = MaxRecs * sizeof(VirType);
  Vir.DelRecs = 0;

  if ((Virtual = malloc(NumBytes)) == NULL) {
    errorexit("virtual","memory error",__FILE__,__LINE__);
  }
  Vir.LastRecAdr = &Virtual[MaxRecs-1];
  Vir.LastRecNum = MaxRecs;

  if ((Vir.File = open("virtdisk.$$$",O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE)) == -1) {
    errorexit("virtdisk.$$$","Error Opening File",__FILE__,__LINE__);
  }
  if ((Vir.DelFile = open("virtdlte.$$$",O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE)) == -1) {
    errorexit("virtdlte.$$$","Error Opening File",__FILE__,__LINE__);
  }
}

void pascal freevirtual(unsigned NumRecs) {
  unsigned Counter;
  VirType *p, VirRec;

  for (Counter = 0, p = &Virtual[NumRecs-1]; Counter < NumRecs; Counter++, p--) {
    if (p->Rec == MEMBUFA)
      free(p->Loc.Buf);
  }

  if (Vir.DelRecs) {
    lseek(Vir.DelFile,0,SEEK_SET);
    for (; Vir.DelRecs; Vir.DelRecs--) {
      read(Vir.DelFile,&VirRec,sizeof(VirType));
      if (VirRec.Rec == MEMBUFA)
        free(VirRec.Loc.Buf);
    }
  }

  free(Virtual);
  close(Vir.File);
  close(Vir.DelFile);
  unlink("virtdisk.$$$");
  unlink("virtdlte.$$$");
  memset(&Vir,0,sizeof(Vir));
}

