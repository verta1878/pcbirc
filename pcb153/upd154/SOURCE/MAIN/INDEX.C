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


#include "project.h"
#pragma hdrstop

// #include <search.h>

#ifdef __OS2__
  #include <semafore.hpp>
  #define USERENTRY _USERENTRY
#else
  #define USERENTRY
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
  #define coreleft _memavl
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#ifdef LIB
  #define numrecs(Handle,RecSize) (doslseek(Handle,0,SEEK_END) / RecSize)
  #define bmalloc malloc
  #define bfree   free
#endif

static int  IndexFile = 0;

static long FoundRecNum;
static char FoundName[25];


static void _NEAR_ LIBENTRY getindexname(char FirstLetter, char *FileName) {
  char static *NdxMask = "PCBNDX.-";

  NdxMask[7] = (FirstLetter <= 'A' ? 'A' : FirstLetter >= 'Z' ? 'Z' : FirstLetter);
  buildstr(FileName,PcbData.NdxLoc,NdxMask,NULL);
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

static int bsearchcomp(const void *a, const void *b) {
  const IndexType *bRec = (IndexType *) b;
  return(memcmp(a,bRec->UserName,sizeof(bRec->UserName)));
}

#ifdef BIGNDX
static int bsearchcomp2(const void *a, const void *b) {
  const IndexType2 *bRec = (IndexType2 *) b;
  return(memcmp(a,bRec->UserName,sizeof(bRec->UserName)));
}
#endif


static int _NEAR_ LIBENTRY singleread(long NumRecs, char *SrchName) {
  unsigned  Size;
  int       RetVal;
  long      Len;
  char      *Buffer;
  IndexType *p;
  int (*comp)(const void *a, const void *b);
  #ifdef BIGNDX
  IndexType2 *q;
  #endif

  Size = sizeof(IndexType);
  #ifdef BIGNDX
    if (BigNdx)
      Size = sizeof(IndexType2);
  #endif

  Len = NumRecs * Size;
  #ifndef __OS2__
    if (Len > 60000L || Len > coreleft()-2048)     /*lint !e574 */
      return(-1);
  #endif

  if ((Buffer = (char *) bmalloc((unsigned) Len)) == NULL) return(-1);   /* don't use mallochk() here cuz we want to return to caller if insufficient memory */

  doslseek(IndexFile,0,SEEK_SET);
  if (readcheck(IndexFile,Buffer,(unsigned) Len) == (unsigned) -1) {
    bfree(Buffer);
    return(-1);
  }

  comp = bsearchcomp;
  #ifdef BIGNDX
    if (BigNdx)
      comp = bsearchcomp2;
  #endif

  if ((p = (IndexType *) bsearch(SrchName,Buffer,(unsigned) NumRecs,Size,comp)) != NULL) {
    RetVal = 0;  // means we found the record
    FoundRecNum = p->UserRec;
    memcpy(FoundName,p->UserName,25);
    #ifdef BIGNDX
      if (BigNdx) {
        q = (IndexType2 *) p;
        FoundRecNum = q->UserRec;
        memcpy(FoundName,q->UserName,25);
      }
    #endif

  } else
    RetVal = -1;  // means we did not find the record

  bfree(Buffer);
  return(RetVal);
}


#ifndef PCB_DEMO
/********************************************************************
*
*  Function: readindex()
*
*  Desc    : Reads the index record pointed to by RecNum
*
*/

#ifndef __OS2__
static int _NEAR_ LIBENTRY readindex(long IdxRecNum) {
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

  IdxRecNum--;
  doslseek(IndexFile,IdxRecNum * Size,SEEK_SET);

  #ifdef BIGNDX
    if (BigNdx) {
      if (readcheck(IndexFile,&BigRec,sizeof(IndexType2)) == (unsigned) -1)
        return(-1);
      FoundRecNum = BigRec.UserRec;
      memcpy(FoundName,BigRec.UserName,25);
    } else {
      if (readcheck(IndexFile,&SmallRec,sizeof(IndexType)) == (unsigned) -1)
        return(-1);
      FoundRecNum = SmallRec.UserRec;
      memcpy(FoundName,SmallRec.UserName,25);
    }
  #else
    if (readcheck(IndexFile,&SmallRec,sizeof(IndexType)) == (unsigned) -1)
      return(-1);
    FoundRecNum = SmallRec.UserRec;
    memcpy(FoundName,SmallRec.UserName,25);
  #endif
  return(0);
}
#endif  /* ifndef __OS2__ */


/********************************************************************
*
*  Function: multiplereads()
*
*  Desc    : Finds a user record by doing multiple reads on the users file in
*            a "binary search" pattern
*/
#pragma warn -rvl
/* #pragma option -Oi */
#ifndef __OS2__
static int _NEAR_ LIBENTRY multiplereads(long High, char *SrchName) {
  long Low;
  long Rec;
  int  RetVal;

  Low = 1;
  if (readindex(Low) == -1) return(FALSE);

  if ((RetVal = memcmp(SrchName,FoundName,25)) == 0)
    return(TRUE);
  else
    if (RetVal < 0)
      return(FALSE);

  if (readindex(High) == -1) return(FALSE);

  if ((RetVal = memcmp(SrchName,FoundName,25)) == 0)
    return(TRUE);
  else
    if (RetVal > 0)
      return(FALSE);

  while (TRUE) {
    if (High <= Low + 1) return(FALSE);
    Rec = Low + ((High - Low) >> 1);
    if (readindex(Rec) == -1) return(FALSE);
    if ((RetVal = memcmp(SrchName,FoundName,25)) == 0)
      return(TRUE);
    else
      if (RetVal < 0)
        High = Rec;
      else
        Low  = Rec;
  }
}
#endif  /* ifdef __OS2__ */
/* #pragma option -O-i */
#pragma warn +rvl
#endif


static long _NEAR_ LIBENTRY numndxrecs(int Handle) {
  #ifdef BIGNDX
    if (BigNdx)
      return(numrecs(Handle,sizeof(IndexType2)));
    else
      return(numrecs(Handle,sizeof(IndexType)));
  #else
    return(numrecs(Handle,sizeof(IndexType)));
  #endif
}


/********************************************************************
*
*  Function: openindexfile()
*
*  Desc    : Uses the name of the user to determine the name of the index file
*            to be opened.
*
*  Returns : The number of entries in the index file or -1 if an error occurs.
*/

static long _NEAR_ LIBENTRY openindexfile(char *SrchName, int OpenMode) {
  char IndexFileName[40];
  #ifndef LIB
  char Str[80];
  #endif

  getindexname(SrchName[0],IndexFileName);

  OpenMode |= (OpenMode == OPEN_RDWR ? OPEN_DENYRDWR : OPEN_DENYWRIT);

  /* just in case the file does not exist - due to slowadd() creating a */
  /* new one - wait up to 20 seconds for a new one to be created.       */

  settimer(2,TWENTYSECONDS);

  while (fileexist(IndexFileName) == 255) {
    if (timerexpired(2)) {
      #ifndef LIB
        printcolor(PCB_RED);
        freshline();
        sprintf(Str,"%s not found",IndexFileName);
        println(Str);
        writelog(Str,SPACERIGHT);
      #endif
      return(-1);  /* never found the file in 20 seconds, so get out */
    }
    tickdelay(ONESECOND);
  }

  /* the file at least WAS there - but now it may be in use so */
  /* let's give it up to 20 seconds to properly open the file  */

  settimer(2,TWENTYSECONDS);
  while ((IndexFile = dosopencheck(IndexFileName,OpenMode)) == -1) {
    if (timerexpired(2))
      return(-1);
    tickdelay(ONESECOND);
  }

  return(numndxrecs(IndexFile));
}


/********************************************************************
*
*  Function: finduser()
*
*  Desc    : Locates a user name specified by SrchName by using a binary
*            search thru the index files.
*
*  Returns : The record number of the user if found, -1 otherwise.
*/

long LIBENTRY finduser(char *SrchName) {
  int  Found;
  long High;
  char Name[26];

  Found = FALSE;
  if ((High = openindexfile(SrchName,OPEN_READ)) != -1) {
    if (High > 0) {
      maxstrcpy(Name,SrchName,sizeof(Name));
      padstr(Name,' ',25);
      if (singleread(High,Name) == 0) {
        Found = TRUE;
      } else {
        #ifndef __OS2__
          #ifndef PCB_DEMO
            Found = multiplereads(High,Name);
          #endif
        #endif
      }
    }
  }
  dosclose(IndexFile);
  if (Found)
    return(FoundRecNum);
  else
    return(-1);
}


#ifndef LIB
static int _NEAR_ LIBENTRY createsemaphore(char *IndexFileName, char *Semaphore) {
  int  Handle;
  #ifdef __OS2__
    os2errtype        Os2Error;
    CTimeoutSemaphore Wait;
  #endif

  /* build "semaphore" filename - just a NEW file to create which will be */
  /* "locked" while the indexing is proceeding                            */
  buildstr(Semaphore,IndexFileName,"!!",NULL);

  #ifdef __OS2__
    Wait.createunique("CR8SEM");
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while ((Handle = doscreate(Semaphore,OPEN_RDWR|OPEN_DENYRDWR,OPEN_NORMAL POS2ERROR)) == -1) {
    #ifdef __OS2__
    if (Wait.timerexpired(QUARTERSECOND)) {
    #else
    if (timerexpired(2)) {
    #endif
      writelog("ERROR: SEMAPHORE CREATE FAILED",SPACERIGHT);
      return(-1);
    }
  }

  return(Handle);
}


static void _NEAR_ LIBENTRY removesemaphore(char *Semaphore, int Handle) {
  dosclose(Handle);
  dosunlinkcheck(Semaphore);    /*lint !e534 */
}


/* #pragma option -Oi */
void LIBENTRY removeuser(char *SrchName) {
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
  char      Name[sizeof(UsersData.Name)];

  strcpy(Name,SrchName);
  padstr(Name,' ',25);

  getindexname(SrchName[0],IndexFileName);

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

  if ((Handle = createsemaphore(IndexFileName,Semaphore)) == -1)
    return;

  buildstr(NewFileName,IndexFileName,"{}",NULL);

  /* use TIMER #2 to watch the opening of the index files - continually loop */
  /* for up to 60 seconds each in an attempt to open the files for updating  */
  /* if the 60 second timer expires then exit with a failure message         */

  #ifdef __OS2__
    CTimeoutSemaphore Wait;
    Wait.createunique("RMVUSR");
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while (dosfopen(NewFileName,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&Out) == -1) {
    #ifdef __OS2__
    if (Wait.timerexpired(QUARTERSECOND)) {
    #else
    if (timerexpired(2)) {
    #endif
      writelog("ERROR: CREATE INDEX FAILURE",SPACERIGHT);
      removesemaphore(Semaphore,Handle);
      return;
    }
  }

  #ifdef __OS2__
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while (dosfopen(IndexFileName,OPEN_READ|OPEN_DENYNONE,&In) == -1) {
    #ifdef __OS2__
      if (Wait.timerexpired(QUARTERSECOND))
        goto error;
    #else
      if (timerexpired(2))
        goto error;
    #endif
  }

  NumRecs = numndxrecs(In.handle);
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

  /* Now delete the old index file - waiting up to 60 seconds in case it is */
  /* in use by another node.  Then remove the "semaphore" by closing the    */
  /* new file and renaming it to the permanent index filename.              */

  #ifdef __OS2__
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while (dosunlinkcheck(IndexFileName) == -1) {
    #ifdef __OS2__
    if (Wait.timerexpired(QUARTERSECOND)) {
    #else
    if (timerexpired(2)) {
    #endif
      dosfclose(&Out);
      goto error;
    }
  }
  dosfclose(&Out);
  rename(NewFileName,IndexFileName);   /*lint !e534 */
  removesemaphore(Semaphore,Handle);
  return;

error:
  dosfclose(&In);
  dosfclose(&Out);
  dosunlinkcheck(NewFileName);    /*lint !e534 */
  writelog("ERROR: REMOVE INDEX FAILURE",SPACERIGHT);
  removesemaphore(Semaphore,Handle);
  return;
}
/* #pragma option -O-i */


#ifndef PCB_DEMO
/* #pragma option -Oi */
static int _NEAR_ LIBENTRY slowadd(char *Name, long RecNo, long NumRecs) {
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

  getindexname(Name[0],IndexFileName);

  buildstr(NewFileName,IndexFileName,"{}",NULL);

  /* use TIMER #2 to watch the opening of the index files - continually loop */
  /* for up to 60 seconds each in an attempt to open the files for updating  */
  /* if the 60 second timer expires then exit with a failure message         */

  #ifdef __OS2__
    CTimeoutSemaphore Wait;
    Wait.createunique("RMVUSR");
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while (dosfopen(NewFileName,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&Out) == -1) {
    #ifdef __OS2__
    if (Wait.timerexpired(QUARTERSECOND)) {
    #else
    if (timerexpired(2)) {
    #endif
      writelog("ERROR: CREATE INDEX FAILURE",SPACERIGHT);
      return(-1);
    }
  }

  #ifdef __OS2__
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while (dosfopen(IndexFileName,OPEN_READ|OPEN_DENYNONE,&In) == -1) {
    #ifdef __OS2__
      if (Wait.timerexpired(QUARTERSECOND))
        goto error;
    #else
      if (timerexpired(2))
        goto error;
    #endif
  }

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
      if (dosfwrite(&uRecNo,sizeof(short),&Out) == -1)
        goto error;
    }
  #else
    if (dosfwrite(&uRecNo,sizeof(short),&Out) == -1)
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

  /* Now delete the old index file - waiting up to 60 seconds in case it is */
  /* in use by another node.  Then remove the "semaphore" by closing the    */
  /* new file and renaming it to the permanent index filename.              */

  #ifdef __OS2__
    Wait.settimer(SIXTYSECONDS);
  #else
    settimer(2,SIXTYSECONDS);
  #endif

  while (dosunlinkcheck(IndexFileName) == -1) {
    #ifdef __OS2__
    if (Wait.timerexpired(QUARTERSECOND)) {
    #else
    if (timerexpired(2)) {
    #endif
      dosfclose(&Out);
      goto error;
    }
  }
  dosfclose(&Out);
  rename(NewFileName,IndexFileName);   /*lint !e534 */
  return(0);

error:
  dosfclose(&In);
  dosfclose(&Out);
  dosunlinkcheck(NewFileName);         /*lint !e534 */
  writelog("ERROR: ADD INDEX FAILURE",SPACERIGHT);
  return(-1);
}
/* #pragma option -O-i */
#endif  /* ifndef DEMO */


/********************************************************************
*
*  Function: addusertoindex()
*
*  Desc    :
*
*  Returns :  0 if okay, -1 if error
*/

/* #pragma option -Oi */
int LIBENTRY addusertoindex(char *Name, long RecNo) {
  int       Handle;
  int       RetVal;
  unsigned  Size;
  unsigned  uRecNo;
  unsigned  RecNumSize;
  unsigned  LenBefore;
  unsigned  LenAfter;
  char      *Buffer;
  char      *p;
  char      *q;
  long      NumRecs;
  char      SrchName[sizeof(UsersData.Name)];
  char      IndexFileName[40];
  char      Semaphore[40];

  uRecNo = (unsigned) RecNo;
  Size = sizeof(IndexType);
  RecNumSize = sizeof(short);
  #ifdef BIGNDX
    if (BigNdx) {
      Size = sizeof(IndexType2);
      RecNumSize = sizeof(long);
    }
  #endif

  getindexname(Name[0],IndexFileName);

  if ((Handle = createsemaphore(IndexFileName,Semaphore)) == -1)
    return(-1);

  if ((NumRecs = openindexfile(Name,OPEN_RDWR)) != -1) {
    movestr(SrchName,Name,sizeof(UsersRead.Name));
    padstr(SrchName,' ',25);

    if ((long) NumRecs * Size > 60000L) {
      dosclose(IndexFile);
      #ifdef PCB_DEMO
        RetVal = -1;
      #else
        RetVal = slowadd(Name,RecNo,NumRecs);
      #endif
      removesemaphore(Semaphore,Handle);
      return(RetVal);
    }

    LenBefore = (unsigned) (NumRecs * Size);
    LenAfter  = LenBefore + Size;
    if ((Buffer = (char *) malloc(LenAfter)) == NULL) {
      dosclose(IndexFile);
      #ifdef PCB_DEMO
        RetVal = -1;
      #else
        RetVal = slowadd(Name,RecNo,NumRecs);
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
      if (memcmp(p+RecNumSize,Name,25) > 0) {
        memmove(p+Size,p,(int) (q - p));
        break;
      }
    }

    #ifdef BIGNDX
      if (BigNdx)
        memcpy(p,&RecNo,sizeof(long));
      else
        memcpy(p,&uRecNo,sizeof(short));
    #else
      memcpy(p,&uRecNo,sizeof(short));
    #endif
    memcpy(p+RecNumSize,SrchName,25);

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
  removesemaphore(Semaphore,Handle);
  return(-1);
}
/* #pragma option -O-i */


/********************************************************************
*
*  Function: findusersoundex()
*
*  Desc    : Locates a user name specified by SrchName by using a soundex
*            routine to find names that "sound alike".  It begins it's
*            search inside the index file for the first letter of the name
*            to search for at the record number pointed to by StartRecord
*            which is 0-based.
*
*  Returns : The record number of the user if found, 0 if not found and
*            -1 if there was an error.
*/

long LIBENTRY findusersoundex(char *SrchName, char *FoundStr, long *StartRecord) {
  char static SrchTemp[26];  /* static so we can re-use it */
  char static SrchFirst[5];  /* static so we can re-use it */
  char static SrchLast[5];   /* static so we can re-use it */
  bool        Found;
  unsigned    Size;
  long        High;
  long        RecNum;
  char       *p;
  char       *pName;
  IndexType  *SmallNdx;
  #ifdef BIGNDX
  IndexType2 *WideNdx;
  #endif
  char        First[5];
  char        Last[5];
  char        Buffer[sizeof(IndexType2)+1];

  Size = sizeof(IndexType);
  pName = Buffer + sizeof(short);
  #ifdef BIGNDX
    if (BigNdx) {
      Size = sizeof(IndexType2);
      pName = Buffer + sizeof(long);
    }
  #endif

  p = NULL;
  Found = FALSE;
  RecNum = *StartRecord;

  if ((High = openindexfile(SrchName,OPEN_READ)) != -1) {
    if (High != 0) {
      if (RecNum == 0) {     /* StartRecord */
        maxstrcpy(SrchTemp,SrchName,sizeof(SrchTemp));
        if ((p = strchr(SrchTemp,' ')) != NULL) {
          *p = 0;
          soundex(SrchLast,p+1);
        } else {
          SrchLast[0] = 0;
          Last[0]     = 0;
        }
        soundex(SrchFirst,SrchTemp);
      }

      /* at this point RecNum is still equal to *StartRecord */
      doslseek(IndexFile,RecNum * Size,SEEK_SET);
      for (; RecNum < High; RecNum++) {
        if (readcheck(IndexFile,Buffer,Size) == (unsigned) -1)
          goto error;

        Buffer[Size] = 0;
        if ((p = strnchr(pName,' ',25)) != NULL) {
          *p = 0;
          soundex(Last,p+1);
        } else
          Last[0] = 0;

        soundex(First,pName);

        if (strcmp(SrchFirst,First) == 0 && (SrchLast[0] == 0 || strcmp(SrchLast,Last) == 0)) {
          Found = TRUE;
          *StartRecord = RecNum + 1;
          break;
        }
      }
    }
    dosclose(IndexFile);
  }

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
    if (RecNum >= High)
      *StartRecord = -1;
    return(0);
  }

error:
  dosclose(IndexFile);
  return(-1);
}
#endif  /* ifndef LIB */
