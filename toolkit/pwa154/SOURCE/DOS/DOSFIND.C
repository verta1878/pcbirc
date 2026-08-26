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


#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #include <os2.h>
  #include <string.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
    #include <direct.h>
  #else
/*  #pragma inline */
    #include <dir.h>
    #include "model.h"
  #endif
#endif

#include <malloc.h>
#include <dos.h>
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


int LIBENTRY dosfindfirst(char *FileSpec, struct ffblk *f, int Attrib, int *FindCount DDIRHANDLE) {
  if (*FindCount == 0)
    return(-1);

  #ifdef __OS2__
    FILEFINDBUF3 *FindBuffer;
    FILEFINDBUF3 *p;
    APIRET        rc;
    int           X;
    int           Bytes;

    Bytes = *FindCount * sizeof(FILEFINDBUF3);

/*  if ((FindBuffer = (FILEFINDBUF3 *) malloc(Bytes)) == NULL) */
    if ((FindBuffer = (FILEFINDBUF3 *) alloca(Bytes)) == NULL)
      return(-1);

    rc = DosFindFirst(FileSpec,
                      (PHDIR)DirHandle,
                      Attrib,
                      (PFILEFINDBUF)FindBuffer,
                      Bytes,
                      (PULONG)FindCount,
                      FIL_STANDARD);

    if (rc != 0) {
/*    free(FindBuffer); */
      return(-1);
    }

    for (X = 0, p = FindBuffer; X < *FindCount; X++, f++) {
      strcpy(f->ff_name,p->achName);
      f->ff_ftime  = *(USHORT *)&p->ftimeLastWrite;
      f->ff_fdate  = *(USHORT *)&p->fdateLastWrite;
      f->ff_fsize  = p->cbFile;
      f->ff_attrib = p->attrFile;
      ((char *) p) += p->oNextEntryOffset;
    }
/*  free(FindBuffer); */
    return(0);

  #else
    int NumFound;
    struct ffblk Temp;

    if (findfirst(FileSpec,&Temp,Attrib) == -1)
      return(-1);

    NumFound = 1;
    while (1) {
      *f = Temp;
      if (NumFound >= *FindCount || findnext(&Temp) == -1)
        break;
      f++;
      NumFound++;
    }
    *FindCount = NumFound;
    return(0);

  #endif
}


int LIBENTRY dosfindnext(struct ffblk *f, int *FindCount DDIRHANDLE2) {
  if (*FindCount == 0)
    return(-1);

  #ifdef __OS2__
    FILEFINDBUF3 *FindBuffer;
    FILEFINDBUF3 *p;
    APIRET        rc;
    int           X;
    int           Bytes;

    Bytes = *FindCount * sizeof(FILEFINDBUF3);

/*  if ((FindBuffer = (FILEFINDBUF3 *) malloc(Bytes)) == NULL) */
    if ((FindBuffer = (FILEFINDBUF3 *) alloca(Bytes)) == NULL)
      return(-1);

    rc = DosFindNext((HDIR)DirHandle,
                     (PFILEFINDBUF)FindBuffer,
                     Bytes,
                     (PULONG)FindCount);

    if (rc != 0) {
/*    free(FindBuffer); */
      return(-1);
    }

    for (X = 0, p = FindBuffer; X < *FindCount; X++, f++) {
      strcpy(f->ff_name,p->achName);
      f->ff_ftime  = *(USHORT *)&p->ftimeLastWrite;
      f->ff_fdate  = *(USHORT *)&p->fdateLastWrite;
      f->ff_fsize  = p->cbFile;
      f->ff_attrib = p->attrFile;
      ((char *) p) += p->oNextEntryOffset;
    }
/*  free(FindBuffer); */
    return(0);

  #else

    int NumFound;
    struct ffblk Temp;

    NumFound = 0;
    Temp = *f;

    while (NumFound < *FindCount) {
      if (findnext(&Temp) == -1)
        break;
      *f = Temp;
      f++;
      NumFound++;
    }

    if (NumFound == 0)
      return(-1);

    *FindCount = NumFound;
    return(0);

  #endif
}


#ifdef __OS2__
void LIBENTRY dosclosedirhandle(int DirHandle) {
  DosFindClose(DirHandle);
}
#endif
