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



/******************************************************************************/
/*                                                                            */
/* VERY IMPORTANT NOTE!!!                                                     */
/* ~~~~~~~~~~~~~~~~~~~~~~                                                     */
/* This module is contains conditional code which is enabled by the           */
/* ___USE_CI_CACHE___ macro being defined.  This code is not desirable for    */
/* use by PCBoard, and as such, is not included in the PCB library.  However, */
/* it is very useful for other applications, particularly where many          */
/* CNAMES.IDX accesses will occur in a relatively short period of time.  For  */
/* this reason, any changes to this module will affect both the PCB library   */
/* and any program which recompiles this library for cached access.           */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

#include    <stdio.h>
#include    <string.h>
#include    <dosfunc.h>
#include    <misc.h>
#include    <newdata.h>
#include    <cnameidx.h>

#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

#define NUL '\0'

/******************************************************************************/

       int           IdxHand = 0;
       long          IdxRecs = 0L;

static DOSFILE       IdxFile = { 0, NULL, 0, 0, 0, 0, 0L, 0 };

static cnamesidxtype IdxRec;

/******************************************************************************/

#ifdef ___USE_CI_CACHE___

typedef struct {
    long          Rec;
    cnamesidxtype Buf;
} cicachetype;

#define CI_CACHE_SIZE   64

cicachetype CICache    [ CI_CACHE_SIZE ];

int         CICacheIdx [ CI_CACHE_SIZE ];

unsigned    CICacheHit;
unsigned    CICacheMiss;

    /*--------------------------------------------------------------------*/

static void _NEAR_ LIBENTRY initcicache(void)
{
    int i;
    memset(CICache,0xFF,sizeof(CICache));
    for (i = 0; i < CI_CACHE_SIZE; ++i) CICacheIdx[i] = i;
    CICacheHit = CICacheMiss = 0;
}

    /*--------------------------------------------------------------------*/

static int _NEAR_ LIBENTRY findcicacherec(unsigned Rec, cnamesidxtype * Buf)
{
    int i;

    for (i = 0; (i < CI_CACHE_SIZE) && (CICache[CICacheIdx[i]].Rec != -1L);
        ++i)
        if (Rec == CICache[CICacheIdx[i]].Rec)
        {
            int Tmp;

            Tmp = CICacheIdx[i];
            *Buf = CICache[Tmp].Buf;
            if (i > 0) memmove(&CICacheIdx[1],&CICacheIdx[0],i*sizeof(int));
            CICacheIdx[0] = Tmp;

            ++CICacheHit;

            return 0;
        }

    ++CICacheMiss;

    return -1;
}

    /*--------------------------------------------------------------------*/

static void _NEAR_ LIBENTRY addcicacherec(unsigned Rec, cnamesidxtype * Buf)
{
    int Tmp;

    Tmp = CICacheIdx[CI_CACHE_SIZE-1];
    memmove(&CICacheIdx[1],&CICacheIdx[0],sizeof(CICacheIdx)-sizeof(int));
    CICacheIdx[0] = Tmp;

    CICache[Tmp].Rec = Rec;
    CICache[Tmp].Buf = *Buf;
}

#endif

    /*====================================================================*/

void LIBENTRY opencnamesidx(bool Create)
{
    long Pos;
    char Name [ 128 + 1 ];

    closecnamesidx();

    /* Need to build the cnames.idx filename */
    strcpy(Name,PcbData.CnfFile);
    strcat(Name,".IDX");

    if (Create)
    {
        IdxHand = doscreatecheck(Name,OPEN_WRIT|OPEN_DENYRDWR,
            OPEN_NORMAL);
        IdxRecs = 0L;
    }
    else
    {
        IdxHand = dosopencheck(Name,OPEN_READ|OPEN_DENYWRIT);
        Pos = doslseek(IdxHand,0,SEEK_END);
        IdxRecs = (unsigned) (Pos / sizeof(cnamesidxtype));
    }

   #ifdef ___USE_CI_CACHE___
    initcicache();
   #endif
}

    /*--------------------------------------------------------------------*/

void LIBENTRY closecnamesidx(void)
{
    if (IdxHand > 0)
    {
        dosclose(IdxHand);
        IdxHand = 0;
        IdxRecs = 0L;
    }
}

    /*====================================================================*/

static int _NEAR_ LIBENTRY readidxrec(unsigned Rec, cnamesidxtype * Buf)
{
   #ifdef ___USE_CI_CACHE___
    if (findcicacherec(Rec,Buf) == 0) return 0;
   #endif

    doslseek(IdxHand,(long) Rec * (long) sizeof(cnamesidxtype),SEEK_SET);

    if (readcheck(IdxHand,Buf,sizeof(*Buf)) != sizeof(*Buf))
        return -1;

   #ifdef ___USE_CI_CACHE___
    addcicacherec(Rec,Buf);
   #endif

    return 0;
}

    /*--------------------------------------------------------------------*/

long LIBENTRY getconfbyname(char * Name)
{
    int           Cmp;
    unsigned      Start, Cur, End;
    long          Pos;
    cnamesidxtype Rec;
    char          Buf [ 60 + 1 ];

    memcpy(Buf,Name,sizeof(Buf));
    Buf[sizeof(Buf)-1] = NUL;

    memset(&Rec,0,sizeof(Rec));

    if (IdxHand > 0)
    {
        Start = 0;
        End   = (unsigned) IdxRecs;
        Pos   = 0L;

        while (Start < End)
        {
            Cur = (unsigned) (((long) Start + (long) End) / 2);

            Pos = readidxrec(Cur,&Rec);
            if (Pos == -1) break;

            Cmp = stricmp(Rec.Name,Buf);
            if      (Cmp <  0)
                Start = Cur + 1;
            else if (Cmp == 0)
            {
                Start = End = Cur;
                Pos = readidxrec(Cur,&Rec);
            }
            else if (Cmp >  0)
                End   = Cur;
        }

        if ((Pos != -1L) && (stricmp(Rec.Name,Buf) == 0))
            Pos = Rec.Num;
        else
            Pos = -1L;
    }

    return Pos;
}

    /*====================================================================*/

unsigned LIBENTRY findconfbyname(char * Name, cnamesidxtype * Rec)
{
    int           Cmp;
    int           Len;
    unsigned      Start, Cur, End;
    long          Pos;
    char          Buf [ 60 + 1 ];

    Len = strlen(Name);
    memcpy(Buf,Name,sizeof(Buf));
    Buf[sizeof(Buf)-1] = NUL;

    memset(Rec,0,sizeof(*Rec));

    if (IdxHand > 0)
    {
        Start = 0;
        End   = (unsigned) IdxRecs;
        Pos   = 0L;

        while (Start < End)
        {
            Cur = (unsigned) (((long) Start + (long) End) / 2);

            Pos = readidxrec(Cur,Rec);
            if (Pos == -1) break;

            Cmp = strnicmp(Rec->Name,Name,Len);
            if      (Cmp <  0)
                Start = Cur + 1;
            else if (Cmp == 0)
            {
                Start = End = Cur;
                Pos = readidxrec(Cur,Rec);
            }
            else if (Cmp >  0)
                End   = Cur;
        }

        if (Pos != -1L) {
          // check for an exact match first
          if (stricmp(Rec->Name,Buf) == 0)
            return(Cur);
          // then back up until the matches stop (then go forward +1)
          while (readidxrec(--Cur,Rec) != -1 && strnicmp(Rec->Name,Name,Len) == 0);
          readidxrec(++Cur,Rec);
          if (strnicmp(Rec->Name,Name,Len) == 0)
            return(Cur);
        }
    }

    return((unsigned) -1);
}

    /*====================================================================*/

int LIBENTRY getconfbyrec(unsigned RecNum, cnamesidxtype * Rec) {
  return(readidxrec(RecNum,Rec));
}

    /*====================================================================*/

void LIBENTRY fopencnamesidx(void)
{
    char Name [ 128 + 1 ];

    fclosecnamesidx();

    /* Need to build the cnames.idx filename */
    strcpy(Name,PcbData.CnfFile);
    strcat(Name,".IDX");

    dosfopen(Name,OPEN_READ|OPEN_DENYWRIT,&IdxFile);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY fclosecnamesidx(void)
{
    dosfclose(&IdxFile);
}

    /*====================================================================*/

static int _NEAR_ LIBENTRY freadidxrec(cnamesidxtype * Buf)
{
    int Bytes;
    Bytes = dosfread(Buf,sizeof(*Buf),&IdxFile);
    return ((Bytes == sizeof(*Buf)) ? 0 : -1);
}

    /*--------------------------------------------------------------------*/

cnamesidxtype * LIBENTRY findconfbystr(char * Str)
{
    char Buf [ 60 + 1 ];

    do {

        /* If there is an error reading the record */
        if (freadidxrec(&IdxRec) == -1)
        {
            /* Clear the record buffer and break out of the loop */
            memset(&IdxRec,0,sizeof(IdxRec));
            break;
        }

        /* Make a copy of the name & force uppercase for case insensitivity */
        maxstrcpy(Buf,IdxRec.Name,sizeof(Buf));
        strupr(Buf);

    /* While we haven't found a match */
    } while (strstr(Buf,Str) == NULL);

    /* If a name is defined, return a pointer to the record buffer, else NULL */
    return ((IdxRec.Name[0] != NUL) ? &IdxRec : NULL);
}

/******************************************************************************/

