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
/*                                                                                                                                                        */
/*                                                                 DBASE.CPP                                                              */
/*                                                                                                                                                        */
/*----------------------------------------------------------------------------*/
/*                                                                                                                                                        */
/*                      Data and functions for accessing dBASE compatible files.                  */
/*                                                                                                                                                        */
/*============================================================================*/
/*                                                                            */
/*                                               Written by Scott Dale Robison*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                                Copyright (C) 1994 Clark Development Company*/
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

#ifdef DBASE

// Include Files

#include        <ctype.h>
// #include     <values.h>
#include  "types.hpp"

#ifdef LIB
#include    "pcbtools.h"
#else
#include    "pcbmisc.hpp"
#endif

#include        "dbase.hpp"
#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

#define NUL                     char(0)

#define FALSE                   0
#define TRUE                    1

#define FREENULL(s)     if (s) free(s); s = NULL

/******************************************************************************/

// External Variable Initializations

//extern unsigned _stklen = 0x4000;

/******************************************************************************/

// cFLDSTRUCT Member Functions

void LIBENTRY cFLDSTRUCT::clrNode(FIELD4INFO & info)
{
        FREENULL(info.name);
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cFLDSTRUCT::cvtInfo(char * info)
{
    if ((info == NULL) || (strlen(info) == 0)) return;

        char       nameF4I [ 10 + 1 ];
        FIELD4INFO infoF4I = { nameF4I, 0, 0, 0 };

        char * infoDUP = strdup(info);
        if (infoDUP == NULL) return;

        strupr(infoDUP);

        char * cp = infoDUP;
        while (*cp != NUL)
        {
                if (isascii(*cp) && (isalnum(*cp) || (*cp == '_') || (*cp == ',')))
                        ++cp;
                else
                        strcpy(cp,cp+1);
        }

    memcpy(nameF4I,infoDUP,sizeof(nameF4I));
    nameF4I[sizeof(nameF4I)-1] = NUL;
    cp = strchr(nameF4I,',');
    if (cp != NULL) *cp = NUL;
    if (!isalpha(nameF4I[0])) nameF4I[0] = 'F';

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    infoF4I.type = *infoDUP;

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    infoF4I.len = atoi(infoDUP);

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    infoF4I.dec = atoi(infoDUP);

        free(infoDUP);

        infoF4I.name = strdup(nameF4I);
        if (infoF4I.name == NULL) return;

        switch (infoF4I.type)
        {
                case 'C': case 'D': case 'F': case 'L': case 'M': case 'N':
                        break;

                default:
                        infoF4I.type = 'C';
                        break;
        }

        switch (infoF4I.type)
        {
                case 'C':
                        if (infoF4I.len == 0) infoF4I.len = 1;
                        if (unsigned(infoF4I.len) > 65533U) infoF4I.len = 65533U;
                        infoF4I.dec = 0;
                        break;

                case 'D':
                        infoF4I.len =  8;
                        infoF4I.dec =  0;
                        break;

                case 'F':
                case 'N':
                        if (infoF4I.len <  1) infoF4I.len =  1;
                        if (infoF4I.len > 19) infoF4I.len = 19;
                        if (infoF4I.dec > (infoF4I.len-2)) infoF4I.dec = infoF4I.len-2;
                        if (infoF4I.dec > infoF4I.len) infoF4I.dec =  0;
                        break;

                case 'L':
                        infoF4I.len =  1;
                        infoF4I.dec =  0;
                        break;

                case 'M':
                        infoF4I.len = 10;
                        infoF4I.dec =  0;
                        break;
        }

    addInfo(infoF4I);
}

/******************************************************************************/

#if 0

// cTAGSTRUCT Member Functions

void LIBENTRY cTAGSTRUCT::clrNode(TAG4INFO & info)
{
        FREENULL(info.name);
        FREENULL(info.expression);
        FREENULL(info.filter);
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cTAGSTRUCT::cvtInfo(char * info)
{
    if ((info == NULL) || (strlen(info) == 0)) return;

        char     nameT4I [      10 + 1 ];
        char     exprT4I [ 256 + 1 ];
        char     filtT4I [ 256 + 1 ];
        TAG4INFO infoT4I = { nameT4I, exprT4I, filtT4I, 0, 0 };

        char * infoDUP = strdup(info);
        if (infoDUP == NULL) return;

        strupr(infoDUP);

        char * cp = infoDUP;

    memcpy(nameT4I,infoDUP,sizeof(nameT4I));
    nameT4I[sizeof(nameT4I)-1] = NUL;
    cp = strchr(nameT4I,',');
    if (cp != NULL) *cp = NUL;
    if (!isalpha(nameT4I[0])) nameT4I[0] = 'T';

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    memcpy(exprT4I,infoDUP,sizeof(exprT4I));
    exprT4I[sizeof(exprT4I)-1] = NUL;
    cp = strchr(exprT4I,',');
    if (cp != NULL) *cp = NUL;

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    memcpy(filtT4I,infoDUP,sizeof(filtT4I));
    filtT4I[sizeof(filtT4I)-1] = NUL;
    cp = strchr(filtT4I,',');
    if (cp != NULL) *cp = NUL;

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    infoT4I.unique = atoi(infoDUP);

    cp = strchr(infoDUP,',');
    strcpy(infoDUP,cp ? cp+1 : "");
    infoT4I.descending = atoi(infoDUP);

        free(infoDUP);

        cp = nameT4I;
        while (*cp != NUL)
        {
                if (isascii(*cp) && (isalnum(*cp) || (*cp == '_')))
                        ++cp;
                else
                        strcpy(cp,cp+1);
        }

        infoT4I.name = strdup(nameT4I);
        if (infoT4I.name == NULL) return;

        infoT4I.expression = strdup(exprT4I);
        if (infoT4I.expression == NULL) return;

        infoT4I.filter = strdup(filtT4I);
        if (infoT4I.filter == NULL) return;

        switch (infoT4I.unique)
        {
                case 0: infoT4I.unique = 0;                             break;
                case 1: infoT4I.unique = r4unique_continue; break;
                case 2: infoT4I.unique = e4unique;                      break;
                case 3: infoT4I.unique = r4unique;                      break;
        }

        switch (infoT4I.descending)
        {
                case 0: infoT4I.descending = 0;                         break;
                case 1: infoT4I.descending = r4descending;      break;
        }

    addInfo(infoT4I);
}

#endif

/******************************************************************************/

// cGRPSTRUCT Member Functions

#ifdef ___USE_VAR___

#pragma argsused
void LIBENTRY cGRPSTRUCT::clrNode(long & info)
{
}

#endif

        /*--------------------------------------------------------------------*/

#ifdef ___USE_VAR___

void LIBENTRY cGRPSTRUCT::cvtInfo(long info)
{
    addInfo(info);
}

#endif

/******************************************************************************/

// cCODEBASE Member Data

CODE4 cCODEBASE::c4;

int   cCODEBASE::cnt = 0;

/******************************************************************************/

// cDBF Member Functions

//#define inUseErr(r)    if ( d4) { err = TRUE; r; }
int LIBENTRY cDBF::inUseErr(void)
{
    char buf[80];
    err = FALSE;
    if(d4)
    {

       #ifndef PCBNLC
       if(Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL])
       {
            int cc = curcolor();
            sprintf(buf,"%s %d %s","Warning! The DBase channel #",myChannel," is already in use.");
            printcolor(PCB_RED);
            println(buf);
          //  moreprompt(PRESSENTER);
            writelog(buf,SPACERIGHT);
            printcolor(cc);
       }
       #endif
        err = TRUE;
    }
    return err;
}

        /*--------------------------------------------------------------------*/

//#define notInUseErr(r) if (!d4) { err = TRUE; r; }
int LIBENTRY cDBF::notInUseErr(void)
{
    char buf[80];
    err = FALSE;
    if(!d4)
    {

       #ifndef PCBNLC
       if(Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL])
       {
            int cc = curcolor();
            printcolor(PCB_RED);
            sprintf(buf,"%s %d %s","Warning! The DBase channel #",myChannel," is not in use.");
          //  moreprompt(PRESSENTER);
            println(buf);
            writelog(buf,SPACERIGHT);
            printcolor(cc);
       }
       #endif
        err = TRUE;
    }
    return err;
}


        /*--------------------------------------------------------------------*/

/*
#define dbfCreateTemplate(t)                                                                                              \
void LIBENTRY cDBF::dbfCreatePrototype(t)                                                                         \
{                                                                                                                                                         \
    if(inUseErr() == TRUE) return;                                                         \
    cFLDSTRUCT fstruct; fstruct.bldInfo(finfo);                               \
    c4.exclusive = TRUE;                                                      \
    d4 = d4create(&c4,name,fstruct.getInfo(),NULL);                           \
    if (!d4)                                                                  \
        err = TRUE;                                                           \
    else                                                                      \
    {                                                                         \
        dbfClose();                                                           \
        if (!err) dbfOpen(name,exclusive);                                    \
        if (!err) inUse = TRUE;                                               \
    }                                                                         \
}
*/

        /*--------------------------------------------------------------------*/

#if 0

#define ndxCreateTemplate(t)                                                                                              \
void LIBENTRY cDBF::ndxCreatePrototype(t)                                                                         \
{                                                                                                                                                         \
    if(notInUseErr() == TRUE) return;                                                      \
        cTAGSTRUCT tstruct; tstruct.bldInfo(tinfo);                                                       \
    if (!t4create(d4,tstruct.getInfo(),NULL)) err = TRUE;                     \
 /* if (!i4create(d4, *name ? name : NULL, tstruct.getInfo())) */             \
 /*     err = TRUE;                                            */             \
}

#endif

        /*--------------------------------------------------------------------*/

cDBF::cDBF(void) :
        d4(NULL),
    err(FALSE),
    inUse(FALSE),
    statusFlag(0)
{
}

        /*--------------------------------------------------------------------*/

cDBF::~cDBF(void)
{
    if (inUse) dbfClose();
}

        /*--------------------------------------------------------------------*/

/* dbfCreateTemplate(char * *); */
void LIBENTRY cDBF::dbfCreatePrototype(char * *)                                                                         \
{                                                                                                                                                         \
    if(inUseErr() == TRUE) return;
    cFLDSTRUCT fstruct; fstruct.bldInfo(finfo);
    c4.exclusive = TRUE;
    d4 = d4create(&c4,name,fstruct.getInfo(),NULL);
    if (!d4)
        err = TRUE;
    else
    {
        dbfClose();
        if (!err) dbfOpen(name,exclusive);
        if (!err) inUse = TRUE;
    }
}

        /*--------------------------------------------------------------------*/

#ifdef ___USE_VAR___

/* dbfCreateTemplate(cVAR &); */
void LIBENTRY cDBF::dbfCreatePrototype(cVAR &)                                                                         \
{                                                                                                                                                         \
    if(inUseErr() == TRUE) return;
    cFLDSTRUCT fstruct; fstruct.bldInfo(finfo);
    c4.exclusive = TRUE;
    d4 = d4create(&c4,name,fstruct.getInfo(),NULL);
    if (!d4)
        err = TRUE;
    else
    {
        dbfClose();
        if (!err) dbfOpen(name,exclusive);
        if (!err) inUse = TRUE;
    }
}

#endif

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::dbfOpen(char * name, int exclusive)
{
    if(inUseErr() == TRUE) return;
    err = FALSE;
    c4.exclusive = exclusive;
    c4.auto_open = FALSE;
        d4 = d4open(&c4,name);
    if(d4 == NULL)
    {
      err = TRUE;
    }
    else
    {
      inUse = TRUE;
    }
    statusFlag = c4.error_code;

}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::dbfClose(void)
{
    if(!d4) return;
    d4close(d4);
        d4 = NULL;
    inUse = FALSE;
    statusFlag = c4.error_code;
}

    /*--------------------------------------------------------------------*/
char * LIBENTRY cDBF::dbfGetAlias()
{
    if(notInUseErr() == TRUE) return NULL;
    char * _alias = d4alias(d4);
    statusFlag = c4.error_code;
    return _alias;
}
        /*--------------------------------------------------------------------*/

char * LIBENTRY cDBF::dbfAlias(char * alias)
{
    if(notInUseErr() == TRUE) return NULL;
        char * _alias = d4alias(d4);
    if (alias) d4alias_set(d4,alias);
    statusFlag = c4.error_code;
        return _alias;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::pack(void)
{
    if(notInUseErr() == TRUE) return;
    err = ((d4pack(d4) != 0) || (d4memo_compress(d4) != 0) || err);
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::lock(void)
{
    if(notInUseErr() == TRUE) return;
    unlock();
    if (d4lock_all(d4) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::lock(long recNo)
{
    if(notInUseErr() == TRUE) return;
    unlock();
    if (d4lock(d4,recNo) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::lock(long * recs, int count)
{
    if(notInUseErr() == TRUE) return;
    unlock();
    if (d4lock_group(d4,recs,count) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

#ifdef ___USE_VAR___

void LIBENTRY cDBF::lock(cVAR & recs, int count)
{
    if(notInUseErr() == TRUE) return;
    unlock();
    cGRPSTRUCT gstruct; gstruct.bldInfo(recs);
    if (d4lock_group(d4,gstruct.getInfo(),count) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

#endif

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::unlock(void)
{
    if(notInUseErr() == TRUE) return;
    if (d4unlock(d4) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

#if 0

ndxCreateTemplate(char * *);

#endif

        /*--------------------------------------------------------------------*/

#if 0

#ifdef ___USE_VAR___

ndxCreateTemplate(cVAR &);

#endif

#endif

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::ndxCreate(char * name, char * expr)
{
    if(notInUseErr() == TRUE) return;
    TAG4INFO tag4info = { name, expr, NULL, 0, 0 };
    if (!t4create(d4,&tag4info,NULL)) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::ndxOpen(char * name)
{
    if(notInUseErr() == TRUE) return;
    if (!t4open(d4,NULL,name)) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::ndxClose(char * name)
{
    if(notInUseErr() == TRUE) return;
    if ((i4close(d4index(d4,name)))) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::ndxCloseAll(void)
{
    if(notInUseErr() == TRUE) return;

    TAG4 * tag;

    do {

        tag = d4tag_next(d4,NULL);
        if (tag)
            if (i4close(tag->index) != 0)
                err = TRUE;

    } while (tag);
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::start(void)
{
    if(notInUseErr() == TRUE) return;
    if (d4append_start(d4,FALSE) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::add(void)
{
    if(notInUseErr() == TRUE) return;
    if (d4append(d4) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::append(void)
{
    if(notInUseErr() == TRUE) return;
    if (d4append_blank(d4) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::top(void)
{
    if(notInUseErr() == TRUE) return;
    if (d4top(d4) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::go(long recNo)
{
    err = FALSE;
    if(notInUseErr() == TRUE) return;
    if (d4go(d4,recNo) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::bottom(void)
{
    if(notInUseErr() == TRUE) return;
    if (d4bottom(d4) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

long LIBENTRY cDBF::reccount(void)
{
  long count = 0;
    if(notInUseErr() == TRUE) return 0L;
    count = d4reccount(d4);
    statusFlag = c4.error_code;
    return  count;
}

        /*--------------------------------------------------------------------*/

long LIBENTRY cDBF::recno(void)
{
  long num;
    if(notInUseErr() == TRUE) return 0L;
    num = d4recno(d4);
    statusFlag = c4.error_code;
    return num;

}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::skip(long recs)
{
    if(notInUseErr() == TRUE) return;
    if (d4skip(d4,recs) != 0) err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::bof(void)
{
  int bof;
    if(notInUseErr() == TRUE) return FALSE;
    bof = d4bof(d4);
    statusFlag = c4.error_code;
    return bof;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::eof(void)
{
  int eof;
    if(notInUseErr() == TRUE) return FALSE;
    eof = d4eof(d4);
    statusFlag = c4.error_code;
    return eof;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::blank(void)
{
    if(notInUseErr() == TRUE) return;
    d4blank(d4);
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::kill(void)
{
    if(notInUseErr() == TRUE) return;
        d4delete(d4);
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::resurrect(void)
{
    if(notInUseErr() == TRUE) return;
        d4recall(d4);
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::killed(void)
{
  int killed;
    if(notInUseErr() == TRUE) return FALSE;
    killed = d4deleted(d4);
    statusFlag = c4.error_code;
    return killed;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::changed(int flag)
{
  int changed;
    if(notInUseErr() == TRUE ) return FALSE;
    changed = d4changed(d4,flag);
    statusFlag = c4.error_code;
    return changed;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::tag(char * name)
{
    if(notInUseErr() == TRUE) return;
        d4tag_select(d4,name ? d4tag(d4,name) : NULL);
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/
int  LIBENTRY cDBF::returnStatus(void)
{
    return statusFlag;
    // 0 = r4success
    // 1 = r4found  Search key was found
    // 2 = r4after  Search unsuccessful, file placed after search key
    // 3 = r4eof    end of file
    // 4 = r4bof    beg of file
    // 5 = r4entry  record or tag key is missing
    // 10 = r4decending  tag should be in decending order
    // 20 = r4unique tag key not unique
    // 25 =
    // 40 = r4ignore record should be ingnored
    // 45 = r4keep   record should be kept
    // 50 = r4locked part of file is locked by another user
    // 60 = r4no_create file could not be created
    // 70 = r4no_open file could not be opened
    // 80 = r4no_tag  tag name could not be found
    // 90 = r4terminate slave record was not located during lookup.
}
        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::seek(char * key)
{
    if(notInUseErr() == TRUE) return;
        int rc = d4seek(d4,key);
    if ((rc != 0) && (rc != r4after) && (rc != r4eof) && (rc != r4bof))
    {
        err = TRUE;
        statusFlag = c4.error_code;
    }
    else
        statusFlag = rc;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::seek(double key)
{
    if(notInUseErr() == TRUE) return;
        int rc = d4seek_double(d4,key);
        if ((rc != 0) && (rc != r4after) && (rc != r4eof)) err = TRUE;
    //statusFlag = rc;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::fields(void)
{
  int num;
    if(notInUseErr() == TRUE) return 0;
    num = d4num_fields(d4);
    statusFlag = c4.error_code;
    return num;
}

        /*--------------------------------------------------------------------*/

char * LIBENTRY cDBF::name(int n)
{
    if(notInUseErr() == TRUE) return NULL;

        char * p = NULL;

        if ((n < 1) || (n > d4num_fields(d4)))
                err = TRUE;
        else
        {
                FIELD4 * f4 = d4field_j(d4,n);

                if (f4)
                        p = f4name(f4);
                else
                        err = TRUE;

        //statusFlag = err;
        }
    statusFlag = c4.error_code;

        return p;
}

        /*--------------------------------------------------------------------*/

char LIBENTRY cDBF::type(char * name)
{
    if(notInUseErr() == TRUE) return ' ';

        FIELD4 * f4 = d4field(d4,name);

        char t = ' ';

        if (f4)
                t = f4type(f4);
        else
                err = TRUE;

    statusFlag = c4.error_code;

        return t;
}

        /*--------------------------------------------------------------------*/

unsigned LIBENTRY cDBF::length(char * name)
{
    if(notInUseErr() == TRUE) return 0;

        FIELD4 * f4 = d4field(d4,name);

        unsigned len = 0;

        if (f4)
                len = f4len(f4);
        else
                err = TRUE;


    statusFlag = c4.error_code;
        return len;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::decimals(char * name)
{
    if(notInUseErr() == TRUE) return 0;

        FIELD4 * f4 = d4field(d4,name);

        int dec = 0;

        if (f4)
                dec = f4decimals(f4);
        else
                err = TRUE;


    statusFlag = c4.error_code;
        return dec;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::blank(char * name)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                f4blank(f4);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::get(char * name, char * val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
    {
        if(f4type(f4) == 'M')
          strcpy(val,f4memo_str(f4));
        else
          strcpy(val,f4str(f4));
    }
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::get(char * name, char & val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                val = f4char(f4);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::get(char * name, int & val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                val = f4int(f4);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::get(char * name, long & val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                val = f4long(f4);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::get(char * name, double & val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                val = f4double(f4);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::put(char * name, char * val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
    {
        if(f4type(f4) == 'M')
          f4memo_assign(f4,val);
        else
          f4assign(f4,val);
    }
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::put(char * name, char val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                f4assign_char(f4,val);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::put(char * name, int val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                f4assign_int(f4,val);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::put(char * name, long val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                f4assign_long(f4,val);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::put(char * name, double val)
{
    if(notInUseErr() == TRUE) return;
        FIELD4 * f4 = d4field(d4,name);
        if (f4)
                f4assign_double(f4,val);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

void LIBENTRY cDBF::copy(char * dest, cDBF & dbf, char * src)
{
    if(notInUseErr() == TRUE) return;

    if (dbf.d4 == NULL)
    {
        err = TRUE;
        statusFlag = c4.error_code;
        return;
    }

        FIELD4 * df4 = d4field(d4,dest);
    FIELD4 * sf4 = d4field(dbf.d4,src);

    if (df4 && sf4)
                f4assign_field(df4,sf4);
        else
                err = TRUE;
    statusFlag = c4.error_code;
}

        /*--------------------------------------------------------------------*/

int LIBENTRY cDBF::error(void)
{
        int t = err;
        err = FALSE;
        return t;
}

/******************************************************************************/

// CodeBase Support Functions

//extern "C" void e4hook(CODE4 * code_base, int err_code,
//        char * desc1, char * desc2, char * desc3);

#pragma argsused
void e4hook(CODE4 * code_base, int err_code, char * desc1, char * desc2,
        char * desc3)
{
   #ifdef ___EXEC___

    char buf [ 80 + 1 ];
   if(Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL])
   {
        #ifndef PCBNLC
        int cc = curcolor();
        printcolor(PCB_RED);
        sprintf(buf,"CodeBase Error Code %d (%s)",err_code,e4text(err_code));
        println(buf);
        if (desc1) println(desc1);
        if (desc2) println(desc2);
        if (desc3) println(desc3);
        //moreprompt(PRESSENTER);
        printcolor(cc);
        #endif
    }
    #ifndef PCBNLC
    writelog(buf,SPACERIGHT);
    if(desc1)  writelog(desc1,SPACERIGHT);
    if(desc2)  writelog(desc2,SPACERIGHT);
    if(desc3)  writelog(desc3,SPACERIGHT);
    #endif
    code_base->error_code = err_code;
   #endif
}

        /*--------------------------------------------------------------------*/

//extern "C" int S4FUNCTION file4lock_hook(CODE4 S4PTR * cb,
//        char S4PTR * file_name, long offset, long n)

#pragma argsused
int S4FUNCTION file4lock_hook(CODE4 S4PTR * cb, char S4PTR * file_name,
        long offset, long n, int num_tries)
{
        return e4lock;
}

char * LIBENTRY cDBF::errText(int code)
{
    return e4text(code);
}
/******************************************************************************/

// Functions

#ifdef ___DBF_TEST___

int main(void)
{
        char * fstrs [] =
        {
                "NAME,C,40,0",
                "BDAY,D,0,0",
                "WT,F,6,2",
                NULL
        };

//      cFLDSTRUCT fs(fstrs);

        char * tstrs [] =
        {
                "TNAME,NAME,,0,0",
                "TBDAY,BDAY,,0,0",
                "TWT,WT,,0,0",
                NULL
        };

//      cTAGSTRUCT ts(tstrs);

        cDBF dbf;

    dbf.dbfCreate("DBF",TRUE,fstrs);

    dbf.ndxCreate("TNAME","NAME");
    dbf.ndxCreate("TBDAY","BDAY");
    dbf.ndxCreate("TWT",  "WT");

        return 0;
}

#endif

#endif /* ifdef DBASE */
/******************************************************************************/

