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
#include <dos.h>
#include <stdio.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <pcbfiles.h>
#include <pcbfiles.ext>

#include <vmdata.h>

#ifdef DEBUG
#include <memcheck.h>
#endif



char static *FileName = "pcbndx.-";
bool        NeedToIndex[26];
long static Highest;
long static NumAliases;

static long FoundRecNum;
static char FoundName[25];

static VMDataSet Array;


static int near pascal createsemaphore(char *IndexFileName, char *Semaphore) {
  int  Handle;

  /* build "semaphore" filename - just a NEW file to create which will be */
  /* "locked" while the indexing is proceeding                            */
  strcpy(Semaphore,IndexFileName);
  strcat(Semaphore,"!!");

  while ((Handle = doscreate(Semaphore,OPEN_RDWR|OPEN_DENYRDWR,OPEN_NORMAL)) == -1);
  return(Handle);
}


static void near pascal removesemaphore(char *Semaphore, int Handle) {
  dosclose(Handle);
  dosunlinkcheck(Semaphore);    /*lint !e534 */
}



static void near pascal getindexname(char *IndexFileName, char Letter) {
  if (Letter <= 'A')
    FileName[7] = 'A';
  else if (Letter >= 'Z')
    FileName[7] = 'Z';
  else
    FileName[7] = Letter;

  strcpy(IndexFileName,PcbData.NdxLoc);
  strcat(IndexFileName,FileName);
}


#ifdef PCBSM
/********************************************************************
*
*  Function: loadarrayfromusers()
*
*  Desc    : Loads the index array from the users file storing only a record
*            number and user name in the array.
*
*/

static int near pascal loadarrayfromusers(void) {
  IndexType2 *p;
  long        Offset;
  long        Count;
  char        Name[26];
  URead       Record;

  for (NumAliases = 0, Count = 1; Count <= Highest; Count++) {
    /* read entire record but use only the name portion in IndexType */

    if (dosfread((char *)&Record,sizeof(URead),&DosUsersFile) != sizeof(URead))
      return(-1);

    p = (IndexType2 *) VMRecordCreate(&Array,sizeof(IndexType2),NULL,NULL);
    p->UserRec = Count;
    memcpy(p->UserName,Record.Name,25);

    if (AliasSupport) {
      Offset = ((Record.RecNum-1) * Header.TotalRecSize) + InfHeaderSize + AliasOffset;
      doslseek(DosUsersInfFile.handle,Offset,SEEK_SET);
      if (readcheck(DosUsersInfFile.handle,Record.Name,25) == (unsigned) -1)
        return(-1);

      memcpy(Name,Record.Name,25); Name[25] = 0; stripright(Name,' ');
      if (Name[0] != 0) {
        p = (IndexType2 *) VMRecordCreate(&Array,sizeof(IndexType2),NULL,NULL);
        p->UserRec = Count;
        memcpy(p->UserName,Record.Name,25);
        NumAliases++;
      }
    }

  }

  dosfclose(&DosUsersFile);
  return(0);
}


/********************************************************************
*
*  Function: comparenames()
*
*  Desc    : Compares the names found in Rec1 and Rec2, used by zsort()
*
*  Returns : <0 if Rec1<Rec2, 0 if Rec1=Rec2, >0 if Rec1>Rec2
*/

static VM_SHINT comparenames(const void *A, const void *B) {
  IndexType2 *Rec1 = (IndexType2 *) A;
  IndexType2 *Rec2 = (IndexType2 *) B;

  return(memcmp(Rec1->UserName,Rec2->UserName,25));
}


/********************************************************************
*
*  Function: createindex()
*
*  Desc    : Creates a single index file as specified by Letter.
*
*/

static long StartRec;

static int near pascal createindex(char Letter, const long Total) {
  bool       Make;
  unsigned   uRecNo;
  long       Count;
  IndexType2 *p;
  DOSFILE    IndexFile;
  char       IndexFileName[40];

  getindexname(IndexFileName,Letter);

  dosunlinkcheck(IndexFileName);
  if (dosfopen(IndexFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&IndexFile) == -1)
    return(-1);

  dossetbuf(&IndexFile,8192);

  if (Total > 0) {
    Make = FALSE;
    for (Count = StartRec; Count <= Total; Count++) {
      p = (IndexType2 *) VMRecordGetByIndex(&Array,Count,NULL);
      if (p->UserName[0] == Letter || (Letter == 'A' && p->UserName[0] < 'A') || (Letter == 'Z' && p->UserName[0] > 'Z')) {
        Make = TRUE;                  /* MAKE when first one found */
        #ifdef BIGNDX
          if (BigNdx) {
            if (dosfwrite(&p->UserRec,sizeof(long),&IndexFile) == -1) {
              dosfclose(&IndexFile);
              return(-1);
            }
          } else {
            uRecNo = (unsigned) p->UserRec;
            if (dosfwrite(&uRecNo,sizeof(unsigned),&IndexFile) == -1) {
              dosfclose(&IndexFile);
              return(-1);
            }
          }
        #else
          uRecNo = (unsigned) p->UserRec;
          if (dosfwrite(&uRecNo,sizeof(unsigned),&IndexFile) == -1)
            dosfclose(&IndexFile);
            return(-1);
          }
        #endif
        if (dosfwrite(p->UserName,sizeof(p->UserName),&IndexFile) == -1) {
          dosfclose(&IndexFile);
          return(-1);
        }
      } else if (Make) {
        StartRec = Count-1;   /* store new starting record */
        goto goback;
      }
    }
  }

goback:
  dosfclose(&IndexFile);
  return(0);
}


/********************************************************************
*
*  Function: createindexfiles()
*
*  Desc    : Creates all index files that are specified as needing to be made
*            by the NeedToIndex[] array.
*
*/

void pascal createindexfiles(void) {
  char       NdxLtr[2];
  int        Xloc;
  int        Counter;
  IndexType2 Buffer[200];

  if (dosfopen(PcbData.UsrFile,OPEN_READ|OPEN_DENYNONE,&DosUsersFile) != -1) {
    if (openusersinffile(PcbData.InfFile) == -1) {
      dosfclose(&DosUsersFile);
      return;
    }

    Highest = numrecs(DosUsersFile.handle,sizeof(URead));
    doslseek(DosUsersFile.handle,0,SEEK_SET);

    VMInitRec(&Array,NULL,0,sizeof(IndexType2));
    ShowSwapMessage = TRUE;

    clscolor(Colors[OUTBOX]);
    generalscreen(MainHead1,"Create Users File Indexes");

    fastprintmove(3,8,"Loading user names into memory...",Colors[DISPLAY]);
    if (loadarrayfromusers() == 0) {
      VMSizeLock(&Array);
      fastprintmove(34,8,"   Sorting...",Colors[DISPLAY]);
      VMSort(&Array,sizeof(IndexType2),1,Highest+NumAliases,VM_FALSE,comparenames,(VMSortFunc *)qsort,Buffer,sizeof(Buffer));
      fastprintmove(45,8,"   Writing...",Colors[DISPLAY]);
      fastprintmove(3,10,"Creating Users File Indexes: ",Colors[DISPLAY]);
      Xloc = 32;
      NdxLtr[1] = 0;
      StartRec = 1;
      for (Counter = 0, NdxLtr[0] = 'A'; Counter < 26; Counter++, NdxLtr[0]++) {
        if (NeedToIndex[Counter]) {
          fastprintmove(Xloc,10,NdxLtr,Colors[DISPLAY]);
          Xloc++;
          if (createindex(NdxLtr[0],Highest+NumAliases) == -1)
            break;  /* abort */
        }
      }
    }

    VMDone(&Array);
    ShowSwapMessage = FALSE;
    dosfclose(&DosUsersInfFile);
    dosfclose(&DosUsersFile);
  }
}


/********************************************************************
*
*  Function: createallindexes()
*
*/

void pascal createallindexes(void) {
  memset(NeedToIndex,TRUE,26);
  createindexfiles();
}

void pascal createallindexesprompt(void) {
  bool Okay;

  Okay = FALSE;

  boxcls(21,18,55,22,Colors[MENUBOX],SINGLE);
  inputnum(25,20,1,"Create User Index Files",&Okay,vBOOL,0);
  if (KeyFlags == ESC || ! Okay)
    return;

  createallindexes();
}
#endif


/********************************************************************
*
*  Function: readindex()
*
*  Desc    : Reads the index record pointed to by RecNum
*
*  Returns : -1 on error, 0 otherwise
*/

static int near pascal readindex(const int Handle, long RecNo) {
  unsigned   Size;
  IndexType  SmallRec;
  #ifdef BIGNDX
  IndexType2 BigRec;
  #endif

  Size = sizeof(IndexType);
  #ifdef BIGNDX
    if (BigNdx)
      Size = sizeof(IndexType2);
  #endif

  RecNo--;
  doslseek(Handle,RecNo * Size,SEEK_SET);

  #ifdef BIGNDX
    if (BigNdx) {
      if (readcheck(Handle,&BigRec,sizeof(IndexType2)) == (unsigned) -1)
        return(-1);
      FoundRecNum = BigRec.UserRec;
      memcpy(FoundName,BigRec.UserName,25);
    } else {
      if (readcheck(Handle,&SmallRec,sizeof(IndexType)) == (unsigned) -1)
        return(-1);
      FoundRecNum = SmallRec.UserRec;
      memcpy(FoundName,SmallRec.UserName,25);
    }
  #else
    if (readcheck(Handle,&SmallRec,sizeof(IndexType)) == -1)
      return(-1);
    FoundRecNum = SmallRec.UserRec;
    memcpy(FoundName,SmallRec.UserName,25);
  #endif
  return(0);
}


/********************************************************************
*
*  Function: padname()
*
*  Desc    : Pads the right end of the string Name[] with spaces to fill out
*            a length of 25 as used by PCBoard.
*/

static void near pascal padname(char Name[]) {
  char *p;

  for (p = &Name[strlen(Name)]; p < &Name[25]; p++)
    *p = ' ';
}


/********************************************************************
*
*  Function: singlereads()
*
*  Desc    : Finds a user record by doing a single read on the users file and
*            then calling the bsearch() routine.
*
*  Return  : -1 if error, TRUE if found, FALSE if not found.
*/

#ifdef __BORLANDC__
#pragma option -Oi
#endif

static int bsearchcomp(const void *a, const void *b) {
  const IndexType *bRec;
  bRec = (IndexType *) b;
  return(memcmp(a,bRec->UserName,sizeof(bRec->UserName)));
}

static int bsearchcomp2(const void *a, const void *b) {
  const IndexType2 *bRec;
  bRec = (IndexType2 *) b;
  return(memcmp(a,bRec->UserName,sizeof(bRec->UserName)));
}

#ifdef __BORLANDC__
#pragma option -O-i
#endif

static int near pascal singleread(int IndexFile, long NumRecs, char *SearchName) {
  bool      Found;
  unsigned  Size;
  long      Len;
  char      *Buffer;
  IndexType *p;
  #ifdef BIGNDX
  IndexType2 *q;
  #endif

  Size = sizeof(IndexType);
  #ifdef BIGNDX
    if (BigNdx)
      Size = sizeof(IndexType2);
  #endif

  Len = NumRecs * Size;
  if (Len > 60000L || Len > (long) (coreleft()-2048))
    return(-1);

  if ((Buffer = (char *) malloc((unsigned) Len)) == NULL) return(-1);   /* don't use mallochk() here cuz we want to return to caller if insufficient memory */

  doslseek(IndexFile,0,SEEK_SET);
  if (readcheck(IndexFile,Buffer,(unsigned) Len) == (unsigned) -1) {
    free(Buffer);
    return(-1);
  }

  #ifdef BIGNDX
  if ((p = (IndexType *) bsearch(SearchName,Buffer,(unsigned) NumRecs,Size,(BigNdx ? bsearchcomp2 : bsearchcomp))) != NULL) {
  #else
  if ((p = (IndexType *) bsearch(SearchName,Buffer,(unsigned) NumRecs,Size,bsearchcomp)) != NULL) {
  #endif
    Found = TRUE;
    FoundRecNum = p->UserRec;
    memcpy(FoundName,p->UserName,25);
    #ifdef BIGNDX
      if (BigNdx) {
        q = (IndexType2 *) p;
        FoundRecNum = q->UserRec;
        memcpy(FoundName,q->UserName,25);
      }
    #endif
  } else Found = FALSE;

  free(Buffer);
  return(Found);
}

/********************************************************************
*
*  Function: multiplereads()
*
*  Desc    : Finds a user record by doing multiple reads on the users file in
*            a "binary search" pattern
*/

#ifdef __BORLANDC__
#pragma option -Oi
#endif

static bool near pascal multiplereads(int IndexFile, long High, char *SearchName) {
  long Low;
  long Rec;
  int  RetVal;

  Low = 1;
  if (readindex(IndexFile,Low) == -1) return(FALSE);

  if ((RetVal = memcmp(SearchName,FoundName,25)) == 0)
    return(TRUE);
  else
    if (RetVal < 0)
      return(FALSE);

  if (readindex(IndexFile,High) == -1) return(FALSE);

  if ((RetVal = memcmp(SearchName,FoundName,25)) == 0)
    return(TRUE);
  else
    if (RetVal > 0)
      return(FALSE);

  while (TRUE) {
    if (High <= Low + 1) return(FALSE);
    Rec = Low + ((High - Low) >> 1);
    if (readindex(IndexFile,Rec) == -1) return(FALSE);
    if ((RetVal = memcmp(SearchName,FoundName,25)) == 0)
      return(TRUE);
    else
      if (RetVal < 0)
        High = Rec;
      else
        Low  = Rec;
  }
}

#ifdef __BORLANDC__
#pragma option -O-i
#endif


static long near pascal numndxrecs(int Handle) {
  #ifdef BIGNDX
    if (BigNdx)
      return(numrecs(Handle,sizeof(IndexType2)));
    else
      return(numrecs(Handle,sizeof(IndexType)));
  #else
    return(numrecs(Handle,sizeof(IndexType)));
  #endif
}




#ifdef PCBSM    /* note:  a modified version of finduser() is in PCBDIAG's ANALYZE.C */
/********************************************************************
*
*  Function: finduser()
*
*  Desc    : Locates a user name specified by SearchName by using a binary
*            search thru the index files.
*
*  Returns : The record number of the user if found, -1 otherwise.
*
*  Note    :  a modified version of finduser() is in PCBDIAG's ANALYZE.C
*/

long pascal finduser(char *SearchName) {
  int  Found;
  int  IndexFile;
  long High;
  char IndexFileName[40];

  Found = FALSE;

  getindexname(IndexFileName,SearchName[0]);

  /* NOTE:  MsgData already filled from inside verifyadd() in EDIT.C */
  MsgData.Msg2   = IndexFileName;

  if ((IndexFile = dosopencheck(IndexFileName,OPEN_READ|OPEN_DENYWRIT)) == -1)
    return(-1);

  if ((High = numndxrecs(IndexFile)) == 0) /* not found because */
    goto returnstatus;                     /* the file is empty */

  padname(SearchName);

  if ((Found = singleread(IndexFile,High,SearchName)) == -1)
    Found = multiplereads(IndexFile,High,SearchName);

returnstatus:
  dosclose(IndexFile);
  if (Found)
    return(FoundRecNum);
  else
    return(-1);
}


#ifndef DEMO
#ifdef __BORLANDC__
#pragma option -Oi
#endif

static int near pascal slowadd(char *Name, long RecNo, long NumRecs) {
  unsigned    Size;
  unsigned    uRecNo;
  long        Counter;
  IndexType  *SmallNdx;
  char       *pName;
  #ifdef BIGNDX
  IndexType2 *WideNdx;
  #endif
  char        Buffer[sizeof(IndexType2)];
  DOSFILE     In;
  DOSFILE     Out;
  char        IndexFileName[40];
  char        NewFileName[40];

  uRecNo = (unsigned) RecNo;
  Size = sizeof(IndexType);
  SmallNdx = (IndexType *) Buffer;
  pName = SmallNdx->UserName;
  #ifdef BIGNDX
    if (BigNdx) {
      Size = sizeof(IndexType2);
      WideNdx = (IndexType2 *) Buffer;
      pName = WideNdx->UserName;
    }
  #endif


  getindexname(IndexFileName,Name[0]);

  strcpy(NewFileName,IndexFileName);
  strcat(NewFileName,"{}");

  /* continually loop trying to open the proper files */

  while (dosfopen(NewFileName,OPEN_RDWR|OPEN_CREATE,&Out) == -1);
  while (dosfopen(IndexFileName,OPEN_READ|OPEN_DENYNONE,&In) == -1);

  dossetbuf(&Out,16384);
  dossetbuf(&In,16384);

  /* copy all names UP TO AND INCLUDING names that are of less than */
  /* or equal value to the name to be inserted                      */

  for (Counter = NumRecs; Counter; Counter--) {
    if (dosfread(Buffer,Size,&In) != Size)
      goto error;
    if (memcmp(pName,Name,25) > 0)
      break;
    if (dosfwrite(Buffer,Size,&Out) == -1)
      goto error;
  }

  /* now write the new record */

  #ifdef BIGNDX
    if (BigNdx) {
      if (dosfwrite(&RecNo,sizeof(long),&Out) == -1)
        goto error;
    } else {
      if (dosfwrite(&uRecNo,sizeof(int),&Out) == -1)
        goto error;
    }
  #else
    if (dosfwrite(&uRecNo,sizeof(int),&Out) == -1)
      goto error;
  #endif

  if (dosfwrite(Name,25,&Out) == -1)
    goto error;

  /* if Counter != 0 then we stopped in the middle so write the record that */
  /* was read in but not yet written out and then continue copying records  */

  if (Counter) {
    if (dosfwrite(Buffer,Size,&Out) == -1)
      goto error;

    for (Counter--; Counter; Counter--)
      if (dosfread(Buffer,Size,&In) != Size || dosfwrite(Buffer,Size,&Out) == -1)
        goto error;
  }

  dosfclose(&In);
  while (dosunlinkcheck(IndexFileName) == -1);   /* remove old file          */
  dosfclose(&Out);                               /* close the semaphore      */
  rename(NewFileName,IndexFileName);             /* make temp file permanent */  //lint !e534
  return(0);

error:
  dosfclose(&In);
  dosfclose(&Out);
  unlink(NewFileName);
  return(-1);
}

#ifdef __BORLANDC__
#pragma option -O-i
#endif
#endif


/********************************************************************
*
*  Function: adduser()
*
*  Desc    :
*
*  Returns :  0 if okay, -1 if unable to add
*/

#ifdef __BORLANDC__
#pragma option -Oi
#endif

int pascal adduser(char *AddName, long AddNum) {
  unsigned  Size;
  unsigned  uRecNo;
  unsigned  RecNumSize;
  int       Handle;
  int       RetVal;
  int       IndexFile;
  long      Len;
  unsigned  LenBefore;
  unsigned  LenAfter;
  long      NumRecs;
  char      *Buffer;
  char      *p;
  char      *q;
  char      IndexFileName[40];
  char      Semaphore[40];

  uRecNo = (unsigned) AddNum;
  Size = sizeof(IndexType);
  RecNumSize = sizeof(int);
  #ifdef BIGNDX
    if (BigNdx) {
      Size = sizeof(IndexType2);
      RecNumSize = sizeof(long);
    }
  #endif

  getindexname(IndexFileName,AddName[0]);

  if ((Handle = createsemaphore(IndexFileName,Semaphore)) == -1)
    return(-1);

  /* NOTE:  MsgData already filled from inside verifyadd() in EDIT.C */
  MsgData.Msg2   = IndexFileName;

  if ((IndexFile = dosopencheck(IndexFileName,OPEN_RDWR|OPEN_DENYRDWR)) == -1) {
    removesemaphore(Semaphore,Handle);
    return(-1);
  }

  padname(AddName);
  Len = doslseek(IndexFile,0,SEEK_END);
  NumRecs = Len / Size;

  if ((long) NumRecs * sizeof(IndexType) > 32000L) {
    dosclose(IndexFile);
      #ifdef DEMO
        RetVal = -1;
      #else
        RetVal = slowadd(AddName,AddNum,NumRecs);
      #endif
    removesemaphore(Semaphore,Handle);
    return(RetVal);
  }

  LenBefore = (unsigned) Len;
  LenAfter  = LenBefore + Size;
  if ((Buffer = (char *) malloc(LenAfter)) == NULL) { /* don't use mallochk() here cuz we want to drop down if insufficient memory */
    dosclose(IndexFile);
    #ifdef DEMO
      RetVal = -1;
    #else
      RetVal = slowadd(AddName,AddNum,NumRecs);
    #endif
    removesemaphore(Semaphore,Handle);
    return(RetVal);
  }

  doslseek(IndexFile,0,SEEK_SET);
  if (readcheck(IndexFile,Buffer,LenBefore) == (unsigned) -1) {
    free(Buffer);
    dosclose(IndexFile);
    removesemaphore(Semaphore,Handle);
    return(-1);
  }

  q = &Buffer[(unsigned) (NumRecs * Size)];

  for (p = Buffer; p < q; p += Size) {
    if (memcmp(p+RecNumSize,AddName,25) > 0) {
      memmove(p+Size,p,(int) (q - p));
      break;
    }
  }

  #ifdef BIGNDX
    if (BigNdx)
      memcpy(p,&AddNum,sizeof(long));
    else
      memcpy(p,&uRecNo,sizeof(int));
  #else
    memcpy(p,&uRecNo,sizeof(int));
  #endif
  memcpy(p+RecNumSize,AddName,25);

  doslseek(IndexFile,0,SEEK_SET);
  if (writecheck(IndexFile,Buffer,LenAfter) == (unsigned) -1) {
    free(Buffer);
    dosclose(IndexFile);
    removesemaphore(Semaphore,Handle);
    return(-1);
  }

  free(Buffer);
  dosclose(IndexFile);
  removesemaphore(Semaphore,Handle);
  return(0);
}

#ifdef __BORLANDC__
#pragma option -O-i
#endif


/********************************************************************
*
*  Function: findusersoundex()
*
*  Desc    : Locates a user name specified by SearchName by using a soundex
*            routine to find names that "sound alike".  It begins it's
*            search inside the index file for the first letter of the name
*            to search for at the record number pointed to by StartRecord
*            which is 0-based.
*
*  Returns : The record number of the user if found, 0 if not found and
*            -1 if there was an error.
*/

static char SrchTemp[26];

long pascal findusersoundex(char *SearchName, char *FoundStr, long *StartRecord) {
  bool        Found;
  int         IndexFile;
  unsigned    Size;
  long        High;
  long        RecNo;
  char       *p;
  char       *pName;
  IndexType  *SmallNdx;
  #ifdef BIGNDX
  IndexType2 *WideNdx;
  #endif
  char        SrchFirst[5];
  char        SrchLast[5];
  char        First[5];
  char        Last[5];
  char        Buffer[sizeof(IndexType2)+1];
  char        IndexFileName[40];

  Size = sizeof(IndexType);
  pName = Buffer + sizeof(int);
  #ifdef BIGNDX
    if (BigNdx) {
      Size = sizeof(IndexType2);
      pName = Buffer + sizeof(long);
    }
  #endif

  Found = FALSE;

  getindexname(IndexFileName,SearchName[0]);

  if ((IndexFile = dosopencheck(IndexFileName,OPEN_READ|OPEN_DENYWRIT)) == -1)
    return(-1);

  High = numndxrecs(IndexFile);

  SrchLast[0] = 0;
  Last[0]     = 0;

  strcpy(SrchTemp,SearchName);
  if ((p = strchr(SrchTemp,' ')) != NULL) {
    *p = 0;
    soundex(SrchLast,p+1);
  }
  soundex(SrchFirst,SrchTemp);

  doslseek(IndexFile,*StartRecord * Size,SEEK_SET);

  for (RecNo = *StartRecord; RecNo < High; RecNo++) {
    if (readcheck(IndexFile,Buffer,Size) == (unsigned) -1)
      goto error;

    Buffer[Size] = 0;
    if (SrchLast[0] != 0) {
      if ((p = strchr(pName,' ')) != NULL) {
        *p = 0;
        soundex(Last,p+1);
      } else {
        Last[0] = 0;
      }
    }
    soundex(First,pName);

    if (strcmp(SrchFirst,First) == 0 && strcmp(SrchLast,Last) == 0) {
      Found = TRUE;
      *StartRecord = RecNo + 1;
      break;
    }
  }

  dosclose(IndexFile);

  if (Found) {
    if (p != NULL)
      *p = ' ';
    memcpy(FoundStr,pName,25);
    FoundStr[25] = 0;
    stripright(FoundStr,' ');
    #ifdef BIGNDX
      if (BigNdx) {
        WideNdx = (IndexType2 *) Buffer;
        return(WideNdx->UserRec);
      } else {
        SmallNdx = (IndexType *) Buffer;
        return(SmallNdx->UserRec);
      }
    #else
      SmallNdx = (IndexType *) Buffer;
      return(SmallNdx->UserRec);
    #endif
  } else {
    FoundStr[0] = 0;
    if (RecNo >= High)
      *StartRecord = -1;
    return(0);
  }

error:
  dosclose(IndexFile);
  return(-1);
}
#endif


#ifdef __BORLANDC__
#pragma option -Oi
#endif

void pascal removeuserfromindex(char *Name) {
  unsigned  Size;
  int       Handle;
  long      Counter;
  long      NumRecs;
  IndexType *SmallNdx;
  char      *p;
  #ifdef BIGNDX
  IndexType2 *WideNdx;
  #endif
  DOSFILE   In;
  DOSFILE   Out;
  char      Buffer[sizeof(IndexType2)];
  char      IndexFileName[40];
  char      NewFileName[40];
  char      Semaphore[40];

  Size = sizeof(IndexType);
  SmallNdx = (IndexType *) Buffer;
  p = SmallNdx->UserName;
  #ifdef BIGNDX
    if (BigNdx) {
      Size = sizeof(IndexType2);
      WideNdx = (IndexType2 *) Buffer;
      p = WideNdx->UserName;
    }
  #endif

  getindexname(IndexFileName,Name[0]);

  if ((Handle = createsemaphore(IndexFileName,Semaphore)) == -1)
    return;

  strcpy(NewFileName,IndexFileName);
  strcat(NewFileName,"{}");

  /* continually loop trying to open the proper files */

  while (dosfopen(NewFileName,OPEN_RDWR|OPEN_CREATE,&Out) == -1);
  while (dosfopen(IndexFileName,OPEN_READ|OPEN_DENYNONE,&In) == -1);

  NumRecs = dosfseek(&In,0,SEEK_END) / Size;
  dosrewind(&In);

  dossetbuf(&Out,16384);
  dossetbuf(&In,16384);

  /* copy all names until the Name is found, then break out */

  for (Counter = NumRecs; Counter; Counter--) {
    if (dosfread(Buffer,Size,&In) != Size)
      goto error;
    if (memcmp(p,Name,25) == 0)
      break;
    if (dosfwrite(Buffer,Size,&Out) == -1)
      goto error;
  }

  /* if Counter!=0 then we stopped in the middle so finish writing the rest */

  if (Counter) {
    for (Counter--; Counter; Counter--)
      if (dosfread(Buffer,Size,&In) != Size || dosfwrite(Buffer,Size,&Out) == -1)
        goto error;
  }

  dosfclose(&In);
  while (dosunlinkcheck(IndexFileName) == -1);   /* remove old file          */
  dosfclose(&Out);                               /* close the semaphore      */
  rename(NewFileName,IndexFileName);             /* make temp file permanent */  //lint !e534
  removesemaphore(Semaphore,Handle);
  return;

error:
  dosfclose(&In);
  dosfclose(&Out);
  dosunlinkcheck(NewFileName);    //lint !e534
  removesemaphore(Semaphore,Handle);
}

#ifdef __BORLANDC__
#pragma option -O-i
#endif


void pascal updateindex(char *Name, long NewRecNum) {
  int         Size;
  int         IndexFile;
  unsigned    uRecNum;
  long        High;
  long        Pos;
  char        IndexFileName[40];

  Size = sizeof(IndexType);
  #ifdef BIGNDX
    if (BigNdx)
      Size = sizeof(IndexType2);
  #endif

  getindexname(IndexFileName,Name[0]);

  if ((IndexFile = dosopencheck(IndexFileName,OPEN_RDWR|OPEN_DENYWRIT)) == -1)
    return;

  if ((High = numndxrecs(IndexFile)) == 0) /* not found because */
    goto exit;                             /* the file is empty */

  if (multiplereads(IndexFile,High,Name)) {          /* if record was found */
    Pos = doslseek(IndexFile,0,SEEK_CUR);            /* where was it?       */
    Pos -= Size;                                     /* get start of record */
    doslseek(IndexFile,Pos,SEEK_SET);                /* go back to record   */
    if (doslockcheck(IndexFile,Pos,Size) != -1) {
      /* update just the record number */
      #ifdef BIGNDX
        if (BigNdx)
          writecheck(IndexFile,&NewRecNum,sizeof(long));   //lint !e534
        else {
          uRecNum = (unsigned) NewRecNum;
          writecheck(IndexFile,&uRecNum,sizeof(unsigned)); //lint !e534
        }
      #else
        uRecNum = (unsigned) NewRecNum;
        writecheck(IndexFile,&uRecNum,sizeof(unsigned));   //lint !e534
      #endif

      unlock(IndexFile,Pos,Size);
    }
  }

exit:
  dosclose(IndexFile);
}
