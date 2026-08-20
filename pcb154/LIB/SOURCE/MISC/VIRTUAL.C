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
#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include "misc.h"
#include "virtual.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define  BLOCKSIZE  16       /* NOTE: this MUST be a power of 2 */
#define  CACHESIZE  32       /* NOTE: this MUST be a power of 2 */

#define copytolist(Offset,Buffer) farmemcpy(&CacheList[Offset],Buffer,Vir.Size)

typedef struct {
  long     DiskOffset;
  unsigned CacheOffset;
} cachetype;

struct {
  VirType   huge *LastRecAdr;
  long      LastRecNum;
  int       File;
  int       DelFile;
  long      Offset;
  unsigned  Size;
  unsigned  AllocSize;
  unsigned  DelRecs;
  char      huge *MainMem;
  unsigned  MainMemOffset;
  long      CoreCheck;
  unsigned  MainMemCheck;
} Vir;


VirType  huge *Virtual;

cachetype static Cache[CACHESIZE];
char      static far *CacheList;
unsigned  static LastOffset;


void virtualerror(void) {
  memset(&MsgData,0,sizeof(MsgData));
  beep();
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Line1   = 18;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Msg1    = "Error: reached maximum number of records";
  showmessage();
}


static unsigned pascal findincache(long Offset) {
  cachetype *p;

  for (p = Cache; p < &Cache[CACHESIZE]; p++)
    if (Offset == p->DiskOffset)
      return(p->CacheOffset * Vir.Size);
  return(0xFFFF);
}


static void pascal insertincache(void *Buffer, long Offset) {
  memmove(&Cache[1],Cache,(CACHESIZE-1)*sizeof(cachetype));
  copytolist(LastOffset*Vir.Size,Buffer);
  Cache[0].DiskOffset  = Offset;
  Cache[0].CacheOffset = LastOffset;
  LastOffset++;
  LastOffset &= (CACHESIZE-1);  /* wrap around to zero if necessary */
}


void pascal getvirtualrec(const VirType huge *p, void *Buffer) {
  unsigned CacheOffset;
#ifdef DEBUG
  switch (p->Rec) {
    case DISKBUF:
    case MEMBUFA:
    case MEMBUFM: break;
    default     : printf("\nRecord %ld at address %p is not valid (%d)",p-Virtual,p,p->Rec);
                 exit(99);
  }
#endif

  if (p->Rec != DISKBUF) {
    farmemcpy(Buffer,(void far *)p->Loc.Buf,Vir.Size);
  } else {
    if ((CacheOffset = findincache(p->Loc.Offset)) != 0xFFFF) {
      farmemcpy(Buffer,&CacheList[CacheOffset],Vir.Size);
    } else {
      doslseek(Vir.File,p->Loc.Offset,SEEK_SET);
      dosread(Vir.File,Buffer,Vir.Size);
      insertincache(Buffer,p->Loc.Offset);
    }
  }
}


int pascal putvirtualrec(VirType huge *p, void *Buffer) {
  unsigned CacheOffset;

  if (p > Vir.LastRecAdr) {
    virtualerror();
    return(VIRPASTEND);
  }

  if (Vir.MainMemOffset == Vir.MainMemCheck) {
    if (farcoreleft() > Vir.CoreCheck) {
      Vir.MainMemOffset = 0;
      Vir.MainMem = (char huge *) farmalloc(Vir.AllocSize);
      p->Loc.Buf  = Vir.MainMem;
      p->Rec = MEMBUFA;
      farmemcpy((void far *)p->Loc.Buf,Buffer,Vir.Size);
      return(VIROKAY);
    } else {
      if ((CacheOffset = findincache(Vir.Offset)) == 0xFFFF)
        insertincache(Buffer,Vir.Offset);
      else
        copytolist(CacheOffset,Buffer);

      p->Rec = DISKBUF;
      p->Loc.Offset = Vir.Offset;
      doslseek(Vir.File,Vir.Offset,SEEK_SET);
      Vir.Offset += Vir.Size;
      if (writecheck(Vir.File,Buffer,Vir.Size) == (unsigned) -1)
        return(VIRDISKERR);
      else
        return(VIROKAY);
    }
  } else {
    Vir.MainMemOffset += Vir.Size;
    p->Loc.Buf = &Vir.MainMem[Vir.MainMemOffset];
    p->Rec = MEMBUFM;
    farmemcpy((void far *)p->Loc.Buf,Buffer,Vir.Size);
    return(VIROKAY);
  }
}


void pascal updvirtualrec(const VirType huge *p, void *Buffer) {
  unsigned CacheOffset;

  if (p->Rec != DISKBUF) {
    farmemcpy((void far *)p->Loc.Buf,Buffer,Vir.Size);
  } else {
    if ((CacheOffset = findincache(p->Loc.Offset)) == 0xFFFF)
      insertincache(Buffer,p->Loc.Offset);
    else
      copytolist(CacheOffset,Buffer);

    doslseek(Vir.File,p->Loc.Offset,SEEK_SET);
    writecheck(Vir.File,Buffer,Vir.Size);
  }
}


int pascal insvirtualrec(void *EmptyBuffer, long RecNum, long TotRecs) {
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
    virtualerror();
    return(VIRPASTEND);
  }

  Code = putvirtualrec(&Virtual[TotRecs],EmptyBuffer);
  if (Code)
    return(Code);

  VirRec = Virtual[TotRecs];
  /* this probably won't work if "(TotRecs-RecNum) * sizeof(VirType)" is > 64K */
  memmove((void *) &Virtual[RecNum+1],(void *) &Virtual[RecNum],(unsigned) ((TotRecs-RecNum) * sizeof(VirType)));
  Virtual[RecNum] = VirRec;
  return(VIROKAY);
}


int pascal delvirtualrec(long RecNum, long TotRecs) {
  if (writecheck(Vir.DelFile,(void *) &Virtual[RecNum],sizeof(VirType)) == (unsigned) -1) {
    return(VIRDISKERR);
  }
  Vir.DelRecs++;

  /* this probably won't work if "(TotRecs-RecNum) * sizeof(VirType)" is > 64K */
  memmove((void *) &Virtual[RecNum],(void *) &Virtual[RecNum+1],(unsigned) ((TotRecs-RecNum) * sizeof(VirType)));
  return(VIROKAY);
}


void pascal openvirtual(long MaxRecs, unsigned Size) {
  long NumBytes;

  if ((CacheList = (char far *) farmalloc(Size * CACHESIZE)) == NULL)
    errorexit("virtual","insufficient memory for virtual memory table",__FILE__,__LINE__);

  fmemset(CacheList,0,Size * CACHESIZE);
  memset(Cache,0xFFFF,sizeof(Cache));
  LastOffset = 0;

  Vir.Size          = Size;
  Vir.CoreCheck     = Size * (BLOCKSIZE * 2) + 1024;  /* reserve 2 times the block size plus 1k */
  Vir.AllocSize     = Size *  BLOCKSIZE;
  Vir.MainMemCheck  = Size * (BLOCKSIZE - 1);
  Vir.MainMemOffset = Vir.MainMemCheck;
  Vir.Offset        = 0;

  NumBytes   = MaxRecs * sizeof(VirType);
  Vir.DelRecs = 0;

  if ((Virtual = (VirType huge *) farmalloc(NumBytes)) == NULL)
    errorexit("virtual","insufficient memory for virtual memory table",__FILE__,__LINE__);

  fmemset(Virtual,0,NumBytes);

  Vir.LastRecAdr = &Virtual[MaxRecs-1];
  Vir.LastRecNum = MaxRecs;

  if ((Vir.File = doscreatecheck("virtdisk.$$$",OPEN_RDWR,OPEN_NORMAL)) == -1)
    exit(99);
  if ((Vir.DelFile = doscreatecheck("virtdlte.$$$",OPEN_RDWR,OPEN_NORMAL)) == -1)
    exit(99);
}


void pascal freevirtual(long NumRecs) {
  VirType huge *p;
  VirType VirRec;
  long    Counter;

  for (Counter = 0, p = &Virtual[NumRecs-1]; Counter < NumRecs; Counter++, p--) {
    if (p->Rec == MEMBUFA)
      farfree((void far *)p->Loc.Buf);
  }

  if (Vir.DelRecs) {
    doslseek(Vir.DelFile,0,SEEK_SET);
    for (; Vir.DelRecs; Vir.DelRecs--) {
      dosread(Vir.DelFile,&VirRec,sizeof(VirType));
      if (VirRec.Rec == MEMBUFA)
        farfree((void far *)VirRec.Loc.Buf);
    }
  }

  farfree((void far *)Virtual);
  farfree(CacheList);
  dosclose(Vir.File);
  dosclose(Vir.DelFile);
  unlink("virtdisk.$$$");
  unlink("virtdlte.$$$");
  memset(&Vir,0,sizeof(Vir));
}

